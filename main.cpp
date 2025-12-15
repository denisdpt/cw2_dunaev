#include <cstdint>
#include <vector>
#include <queue>
#include <string>
#include <iostream>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <cstdlib>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>

using NodeId = uint32_t;
using Dist   = int32_t;

using Clock = std::chrono::high_resolution_clock;

static inline void set_parlay_threads(int threads) {
  if (threads < 1) threads = 1;
  std::string s = std::to_string(threads);
  ::setenv("PARLAY_NUM_THREADS", s.c_str(), 1);
}

static inline NodeId id3(uint32_t x, uint32_t y, uint32_t z, uint32_t L) {
  return x + L * (y + L * z);
}

struct CsrGraph {
  uint32_t n = 0;
  std::vector<uint32_t> offsets;
  std::vector<NodeId>   edges;

  uint32_t num_vertices() const { return n; }
  uint32_t degree(NodeId v) const { return offsets[v + 1] - offsets[v]; }

  template <class F>
  void for_each_neighbor(NodeId v, F&& fn) const {
    const uint32_t b = offsets[v];
    const uint32_t e = offsets[v + 1];
    for (uint32_t i = b; i < e; ++i) fn(edges[i]);
  }
};

CsrGraph build_cube_csr(uint32_t side) {
  const uint64_t n64 = uint64_t(side) * side * side;
  CsrGraph g;
  g.n = (uint32_t)n64;

  parlay::sequence<uint32_t> deg(g.n);
  parlay::parallel_for(0u, g.n, [&](uint32_t v) {
    uint32_t x = v % side;
    uint32_t y = (v / side) % side;
    uint32_t z = v / (uint64_t(side) * side);

    uint32_t d = 0;
    d += (x > 0) + (x + 1 < side);
    d += (y > 0) + (y + 1 < side);
    d += (z > 0) + (z + 1 < side);
    deg[v] = d;
  });

  auto scanned = parlay::scan(deg);
  parlay::sequence<uint32_t> pref = std::move(scanned.first);
  const uint32_t m = scanned.second;

  g.offsets.resize((size_t)g.n + 1);
  for (uint32_t i = 0; i < g.n; ++i) g.offsets[i] = pref[i];
  g.offsets[g.n] = m;

  g.edges.resize(m);

  parlay::parallel_for(0u, g.n, [&](uint32_t v) {
    uint32_t x = v % side;
    uint32_t y = (v / side) % side;
    uint32_t z = v / (uint64_t(side) * side);

    uint32_t p = g.offsets[v];

    if (x > 0)       g.edges[p++] = id3(x - 1, y, z, side);
    if (x + 1 < side)g.edges[p++] = id3(x + 1, y, z, side);
    if (y > 0)       g.edges[p++] = id3(x, y - 1, z, side);
    if (y + 1 < side)g.edges[p++] = id3(x, y + 1, z, side);
    if (z > 0)       g.edges[p++] = id3(x, y, z - 1, side);
    if (z + 1 < side)g.edges[p++] = id3(x, y, z + 1, side);
  });

  return g;
}

// --- SEQ BFS ---
std::vector<Dist> bfs_seq(const CsrGraph& g, NodeId source) {
  const uint32_t n = g.num_vertices();
  if (n == 0 || source >= n) return {};

  std::vector<Dist> dist(n, -1);
  std::queue<NodeId> q;

  dist[source] = 0;
  q.push(source);

  while (!q.empty()) {
    NodeId v = q.front(); q.pop();
    Dist dv = dist[v];
    g.for_each_neighbor(v, [&](NodeId u) {
      if (dist[u] != -1) return;
      dist[u] = dv + 1;
      q.push(u);
    });
  }
  return dist;
}

// --- PAR BFS ---
std::vector<Dist> bfs_par_fast(const CsrGraph& g, NodeId source, int threads) {
  set_parlay_threads(threads);

  const uint32_t n = g.num_vertices();
  if (n == 0 || source >= n) return {};

  std::vector<std::atomic<Dist>> dist_a(n);
  parlay::parallel_for(0u, n, [&](uint32_t i) {
    dist_a[i].store(-1, std::memory_order_relaxed);
  });
  dist_a[source].store(0, std::memory_order_relaxed);

  parlay::sequence<NodeId> frontier(1);
  frontier[0] = source;

  Dist level = 1;

  constexpr size_t BLOCK = 2048;

  while (frontier.size() != 0) {
    const size_t fsz = frontier.size();
    const size_t num_blocks = (fsz + BLOCK - 1) / BLOCK;

    parlay::sequence<parlay::sequence<NodeId>> blocks(num_blocks);

    parlay::parallel_for(0, num_blocks, [&](size_t b) {
      const size_t l = b * BLOCK;
      const size_t r = std::min(fsz, (b + 1) * BLOCK);

      std::vector<NodeId> local;
      local.reserve((r - l) * 3);

      for (size_t i = l; i < r; ++i) {
        const NodeId v = frontier[i];
        g.for_each_neighbor(v, [&](NodeId u) {
          Dist expected = -1;
          if (dist_a[u].compare_exchange_strong(
                  expected, level,
                  std::memory_order_relaxed,
                  std::memory_order_relaxed)) {
            local.push_back(u);
          }
        });
      }

      blocks[b] = parlay::to_sequence(std::move(local));
    });

    frontier = parlay::flatten(std::move(blocks));
    ++level;
  }

  std::vector<Dist> dist(n);
  parlay::parallel_for(0u, n, [&](uint32_t i) {
    dist[i] = dist_a[i].load(std::memory_order_relaxed);
  });
  return dist;
}

// --- BENCH ---
static uint64_t checksum(const std::vector<Dist>& dist) {
  uint64_t h = 1469598103934665603ull;
  for (Dist d : dist) {
    uint32_t x = (uint32_t)(d + 2);
    h ^= x;
    h *= 1099511628211ull;
  }
  return h;
}

int main(int argc, char** argv) {
  uint32_t side = 300;
  int runs = 5;
  int threads = 4;

  for (int i = 1; i < argc; ++i) {
    auto need = [&](const char* k){ if (i+1 >= argc) { std::cerr<<"Missing "<<k<<"\n"; std::exit(2);} };
    if (!std::strcmp(argv[i], "--side")) { need("--side"); side = (uint32_t)std::stoul(argv[++i]); }
    else if (!std::strcmp(argv[i], "--runs")) { need("--runs"); runs = std::stoi(argv[++i]); }
    else if (!std::strcmp(argv[i], "--threads")) { need("--threads"); threads = std::stoi(argv[++i]); }
  }

  std::cerr << "Building CSR cube (" << side << "^3) ...\n";
  auto g = build_cube_csr(side);
  std::cerr << "CSR built: n=" << g.n << ", m=" << g.edges.size() << "\n";

  const NodeId source = 0;

  double sum_seq = 0, sum_par = 0;
  uint64_t ref = 0;

  for (int r = 1; r <= runs; ++r) {
    std::cout << "\n--- RUN " << r << " ---\n";

    double t_seq, t_par;
    {
      auto t0 = Clock::now();
      auto dist = bfs_seq(g, source);
      auto t1 = Clock::now();
      t_seq = std::chrono::duration<double, std::milli>(t1 - t0).count();
      sum_seq += t_seq;

      uint64_t h = checksum(dist);
      if (r == 1) ref = h;
      if (h != ref) { std::cerr << "Checksum mismatch (seq)\n"; return 1; }
      std::cout << "SEQ: " << t_seq << " ms\n";
    }

    {
      auto t0 = Clock::now();
      auto dist = bfs_par_fast(g, source, threads);
      auto t1 = Clock::now();
      t_par = std::chrono::duration<double, std::milli>(t1 - t0).count();
      sum_par += t_par;

      uint64_t h = checksum(dist);
      if (h != ref) { std::cerr << "Checksum mismatch (par)\n"; return 1; }
      std::cout << "PAR(" << threads << "): " << t_par << " ms\n";
    }

    std::cout << "SPEEDUP: " << (t_seq / t_par) << "\n";
  }

  std::cout << "\n--------------------------\n";
  std::cout << "avg seq: " << (sum_seq / runs) << " ms\n";
  std::cout << "avg par: " << (sum_par / runs) << " ms\n";
  std::cout << "avg speedup: " << ((sum_seq / runs) / (sum_par / runs)) << "\n";
  std::cout << "--------------------------\n";
  return 0;
}
