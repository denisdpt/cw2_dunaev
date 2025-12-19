#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

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

struct AdjListGraph {
  std::vector<std::vector<NodeId>> adj;

  uint32_t num_vertices() const { return static_cast<uint32_t>(adj.size()); }

  uint32_t degree(NodeId v) const { return static_cast<uint32_t>(adj[v].size()); }

  template <class F>
  void for_each_neighbor(NodeId v, F&& fn) const {
    for (NodeId u : adj[v]) fn(u);
  }
};

// --- csr граф ---

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

  uint64_t m_directed() const { return edges.size(); }
};

static CsrGraph build_cube_csr(uint32_t side) {
  const uint64_t n64 = uint64_t(side) * side * side;
  if (n64 > std::numeric_limits<uint32_t>::max()) {
    throw std::runtime_error("n does not fit uint32_t");
  }

  CsrGraph g;
  g.n = static_cast<uint32_t>(n64);

  parlay::sequence<uint32_t> deg(g.n);
  parlay::parallel_for(0u, g.n, [&](uint32_t v) {
    const uint32_t x = v % side;
    const uint32_t y = (v / side) % side;
    const uint32_t z = v / (uint64_t(side) * side);

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
    const uint32_t x = v % side;
    const uint32_t y = (v / side) % side;
    const uint32_t z = v / (uint64_t(side) * side);

    uint32_t p = g.offsets[v];

    if (x > 0)        g.edges[p++] = id3(x - 1, y, z, side);
    if (x + 1 < side) g.edges[p++] = id3(x + 1, y, z, side);
    if (y > 0)        g.edges[p++] = id3(x, y - 1, z, side);
    if (y + 1 < side) g.edges[p++] = id3(x, y + 1, z, side);
    if (z > 0)        g.edges[p++] = id3(x, y, z - 1, side);
    if (z + 1 < side) g.edges[p++] = id3(x, y, z + 1, side);
  });

  return g;
}

static AdjListGraph build_cube_adjlist(uint32_t side) {
  const uint32_t n = side * side * side;
  AdjListGraph g;
  g.adj.resize(n);

  for (uint32_t z = 0; z < side; ++z) {
    for (uint32_t y = 0; y < side; ++y) {
      for (uint32_t x = 0; x < side; ++x) {
        NodeId v = id3(x, y, z, side);
        auto &a = g.adj[v];
        if (x > 0)        a.push_back(id3(x - 1, y, z, side));
        if (x + 1 < side) a.push_back(id3(x + 1, y, z, side));
        if (y > 0)        a.push_back(id3(x, y - 1, z, side));
        if (y + 1 < side) a.push_back(id3(x, y + 1, z, side));
        if (z > 0)        a.push_back(id3(x, y, z - 1, side));
        if (z + 1 < side) a.push_back(id3(x, y, z + 1, side));
      }
    }
  }
  return g;
}

// --- seq bfs ---

template <class Graph>
std::vector<Dist> bfs_seq(const Graph& g, NodeId source) {
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

// --- par bfs ---

template <class Graph>
std::vector<Dist> bfs_par_fast(const Graph& g, NodeId source, int threads) {
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
  constexpr size_t BLOCK = 4096;

  while (!frontier.empty()) {
    const size_t fsz = frontier.size();
    const size_t nb  = (fsz + BLOCK - 1) / BLOCK;

    std::vector<std::vector<NodeId>> locals(nb);
    parlay::sequence<size_t> sizes(nb);

    parlay::parallel_for(0, nb, [&](size_t b) {
      const size_t l = b * BLOCK;
      const size_t r = std::min(fsz, (b + 1) * BLOCK);

      auto &buf = locals[b];
      buf.clear();
      buf.reserve((r - l) * 3);

      for (size_t i = l; i < r; ++i) {
        NodeId v = frontier[i];

        g.for_each_neighbor(v, [&](NodeId u) {
          Dist expected = -1;
          if (dist_a[u].compare_exchange_strong(
                expected, level,
                std::memory_order_relaxed,
                std::memory_order_relaxed)) {
            buf.push_back(u);
          }
        });
      }
      sizes[b] = buf.size();
    });

    auto scanned = parlay::scan(sizes);
    parlay::sequence<size_t> off = std::move(scanned.first);
    const size_t total = scanned.second;

    parlay::sequence<NodeId> next(total);

    parlay::parallel_for(0, nb, [&](size_t b) {
      size_t w = off[b];
      const auto &buf = locals[b];
      for (NodeId u : buf) next[w++] = u;
    });

    frontier = std::move(next);
    ++level;
  }

  std::vector<Dist> dist(n);
  parlay::parallel_for(0u, n, [&](uint32_t i) {
    dist[i] = dist_a[i].load(std::memory_order_relaxed);
  });
  return dist;
}

static uint64_t checksum(const std::vector<Dist>& dist) {
  uint64_t h = 1469598103934665603ull;
  for (Dist d : dist) {
    uint32_t x = (uint32_t)(d + 2); // -1 -> 1
    h ^= x;
    h *= 1099511628211ull;
  }
  return h;
}

// --- Тесты ---

static std::string vec2str(const std::vector<Dist>& v) {
  std::string s = "[";
  for (size_t i = 0; i < v.size(); ++i) {
    if (i) s += ", ";
    s += std::to_string(v[i]);
  }
  s += "]";
  return s;
}

static bool expect_eq(const std::string& name, const std::vector<Dist>& a, const std::vector<Dist>& b) {
  if (a == b) {
    std::cout << "[PASS] " << name << "\n";
    return true;
  }
  std::cout << "[FAIL] " << name << "\n";
  std::cout << "  seq=" << vec2str(a) << "\n";
  std::cout << "  par=" << vec2str(b) << "\n";
  return false;
}

static bool run_fixed_tests() {
  struct TC {
    std::string name;
    std::vector<std::vector<NodeId>> adj;
    NodeId src;
    std::vector<Dist> expected;
  };

  const std::vector<TC> tests = {
    {"empty_graph", {}, 0, {}},
    {"single_vertex", {{}}, 0, {0}},
    {"path_graph", {{1},{0,2},{1,3},{2,4},{3}}, 0, {0,1,2,3,4}},
    {"cycle_graph", {{1,3},{0,2},{1,3},{2,0}}, 0, {0,1,2,1}},
    {"star_graph", {{1,2,3,4},{0},{0},{0},{0}}, 0, {0,1,1,1,1}},
    {"two_components", {{1},{0},{3},{2}}, 0, {0,1,-1,-1}},
    {"self_loops", {{0,1},{0,1,2},{1,2}}, 0, {0,1,2}},
    {"complete_graph", {{1,2,3},{0,2,3},{0,1,3},{0,1,2}}, 2, {1,1,0,1}},
    {"isolated_vertex", {{1},{0},{}}, 0, {0,1,-1}},
    {"directed_like", {{1},{2},{},{}}, 0, {0,1,2,-1}},
  };

  for (const auto& tc : tests) {
    AdjListGraph g{tc.adj};

    auto seq = bfs_seq(g, tc.src);
    if (seq != tc.expected) {
      std::cout << "[FAIL][SEQ][expected] " << tc.name << "\n";
      std::cout << "  got=" << vec2str(seq) << "\n";
      std::cout << "  exp=" << vec2str(tc.expected) << "\n";
      return false;
    }

    auto par = bfs_par_fast(g, tc.src, 4);
    if (par != tc.expected) {
      std::cout << "[FAIL][PAR][expected] " << tc.name << "\n";
      std::cout << "  got=" << vec2str(par) << "\n";
      std::cout << "  exp=" << vec2str(tc.expected) << "\n";
      return false;
    }

    std::cout << "[PASS] " << tc.name << "\n";
  }

  std::cout << "All fixed tests passed.\n";
  return true;
}

static bool run_random_tests() { // случайные графы (фиксированный seed), проверяем seq == par
  std::mt19937 rng(123456u);

  auto gen_graph = [&](uint32_t n, uint32_t m, bool undirected) {
    AdjListGraph g;
    g.adj.assign(n, {});
    std::uniform_int_distribution<uint32_t> vd(0, n - 1);

    for (uint32_t i = 0; i < m; ++i) {
      uint32_t a = vd(rng);
      uint32_t b = vd(rng);
      if (a == b) continue;
      g.adj[a].push_back(b);
      if (undirected) g.adj[b].push_back(a);
    }
    return g;
  };

  for (int t = 0; t < 20; ++t) {
    uint32_t n = 10 + (t % 20);
    uint32_t m = 20 + (t * 7 % 80);
    bool undirected = (t % 2 == 0);

    auto g = gen_graph(n, m, undirected);

    std::uniform_int_distribution<uint32_t> sd(0, n - 1);
    NodeId src = sd(rng);

    auto seq = bfs_seq(g, src);
    auto par = bfs_par_fast(g, src, 4);

    if (!expect_eq("random_graph_" + std::to_string(t), seq, par)) return false;
  }

  std::cout << "All random tests passed.\n";
  return true;
}

static bool run_cube_consistency_test() { // проверкяем любой граф + сравнение csr и adjacency list на маленьком кубике
  const uint32_t side = 8;
  auto csr = build_cube_csr(side);
  auto adj = build_cube_adjlist(side);

  std::vector<NodeId> sources = {0, id3(3,3,3,side), id3(7,0,7,side)};

  for (NodeId s : sources) {
    auto seq_csr = bfs_seq(csr, s);
    auto par_csr = bfs_par_fast(csr, s, 4);
    if (!expect_eq("cube_csr_seq_vs_par_src=" + std::to_string(s), seq_csr, par_csr)) return false;

    auto seq_adj = bfs_seq(adj, s);
    auto par_adj = bfs_par_fast(adj, s, 4);
    if (!expect_eq("cube_adj_seq_vs_par_src=" + std::to_string(s), seq_adj, par_adj)) return false;

    // сравним CSR и adjacency list (должны совпасть)
    if (!expect_eq("cube_csr_vs_adj_seq_src=" + std::to_string(s), seq_csr, seq_adj)) return false;
  }

  std::cout << "Cube consistency tests passed.\n";
  return true;
}

static bool run_all_tests() {
  if (!run_fixed_tests()) return false;
  if (!run_random_tests()) return false;
  if (!run_cube_consistency_test()) return false;
  return true;
}

// --- Бенчмарки ---

struct Args {
  uint32_t side = 300;
  int runs = 5;
  int threads = 4;
  bool skip_bench = false;
};

static void usage(const char* p) {
  std::cerr << "Usage: " << p
            << " [--side 300] [--runs 5] [--threads 4] [--skip-bench]\n";
}

static Args parse(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    auto need = [&](const char* k) {
      if (i + 1 >= argc) { std::cerr << "Missing " << k << "\n"; std::exit(2); }
    };

    if (!std::strcmp(argv[i], "--side")) { need("--side"); a.side = (uint32_t)std::stoul(argv[++i]); }
    else if (!std::strcmp(argv[i], "--runs")) { need("--runs"); a.runs = std::stoi(argv[++i]); }
    else if (!std::strcmp(argv[i], "--threads")) { need("--threads"); a.threads = std::stoi(argv[++i]); }
    else if (!std::strcmp(argv[i], "--skip-bench")) { a.skip_bench = true; }
    else if (!std::strcmp(argv[i], "--help")) { usage(argv[0]); std::exit(0); }
    else { std::cerr << "Unknown arg: " << argv[i] << "\n"; usage(argv[0]); std::exit(2); }
  }

  if (a.runs < 1) a.runs = 1;
  if (a.threads < 1) a.threads = 1;
  return a;
}

int main(int argc, char** argv) {
  Args a = parse(argc, argv);

  // 1) Tests (required by assignment)
  std::cout << "Running correctness tests...\n";
  if (!run_all_tests()) return 1;
  std::cout << "All tests passed.\n";

  if (a.skip_bench) return 0;

  // 2) Benchmark (required by assignment): cube 300^3, source (0,0,0) -> 0, runs=5
  std::cerr << "Building CSR cube (" << a.side << "^3) ...\n";
  auto g = build_cube_csr(a.side);
  std::cerr << "CSR built: n=" << g.n << ", m(directed)=" << g.m_directed() << "\n";

  const NodeId source = 0;

  double sum_seq = 0.0, sum_par = 0.0;
  uint64_t ref = 0;

  for (int r = 1; r <= a.runs; ++r) {
    std::cout << "\n--- RUN " << r << " ---\n";

    double t_seq = 0.0, t_par = 0.0;

    { // seq
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
    
    { // par
      auto t0 = Clock::now();
      auto dist = bfs_par_fast(g, source, a.threads);
      auto t1 = Clock::now();
      t_par = std::chrono::duration<double, std::milli>(t1 - t0).count();
      sum_par += t_par;

      uint64_t h = checksum(dist);
      if (h != ref) { std::cerr << "Checksum mismatch (par)\n"; return 1; }

      std::cout << "PAR(" << a.threads << "): " << t_par << " ms\n";
    }

    std::cout << "SPEEDUP: " << (t_seq / t_par) << "\n";
  }

  const double avg_seq = sum_seq / a.runs;
  const double avg_par = sum_par / a.runs;

  std::cout << "\n--------------------------\n";
  std::cout << "avg seq: " << avg_seq << " ms\n";
  std::cout << "avg par: " << avg_par << " ms\n";
  std::cout << "avg speedup: " << (avg_seq / avg_par) << "\n";
  std::cout << "--------------------------\n";

  return 0;
}
