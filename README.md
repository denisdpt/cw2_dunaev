denis@denis-MS-7C52:~/Documents/vs/paral_bfs$ make build
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
-- The CXX compiler identification is GNU 13.3.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- PARLAY VERSION 2.3.3
-- ---------------------------- General configuration -----------------------------
-- CMake Generator:                Unix Makefiles
-- Compiler:                       GNU 13.3.0
-- Build type:                     Release
-- CMAKE_CXX_FLAGS:                
-- CMAKE_CXX_FLAGS_DEBUG:          -g
-- CMAKE_CXX_FLAGS_RELEASE:        -O3 -DNDEBUG
-- CMAKE_CXX_FLAGS_RELWITHDEBINFO: -O2 -g -DNDEBUG -fno-omit-frame-pointer
-- CMAKE_EXE_LINKER_FLAGS          
-- CMAKE_INSTALL_PREFIX:           /usr/local
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- -------------------------------- Library options ------------------------------
-- Parlay OpenMP integration enabled
-- Found OpenMP_CXX: -fopenmp (found version "4.5") 
-- Found OpenMP: TRUE (found version "4.5")  
-- ------------------------------- Static Analysis --------------------------------
-- cppcheck:                     Disabled (enable with -DENABLE_CPPCHECK=On)
-- clang-tidy:                   Disabled (enable with -DENABLE_CLANG_TIDY=On)
-- include-what-you-use:         Disabled (enable with -DENABLE_IWYU=On)
-- ---------------------------------- Unit Tests ----------------------------------
-- testing: Disabled (enable with -DPARLAY_TEST=On)
-- ---------------------------------- Benchmarks ----------------------------------
-- benchmarks: Disabled (enable with -DPARLAY_BENCHMARK=On)
-- ----------------------------------- Examples -----------------------------------
-- examples: Disabled (enable with -DPARLAY_EXAMPLES=On)
-- example data: Off (add -DPARLAY_EXAMPLE_DATA=On to download)
-- Configuring done (3.5s)
-- Generating done (0.0s)
-- Build files have been written to: /home/denis/Documents/vs/paral_bfs/build
cmake --build build -j
gmake[1]: Entering directory '/home/denis/Documents/vs/paral_bfs/build'
gmake[2]: Entering directory '/home/denis/Documents/vs/paral_bfs/build'
gmake[3]: Entering directory '/home/denis/Documents/vs/paral_bfs/build'
gmake[3]: Leaving directory '/home/denis/Documents/vs/paral_bfs/build'
gmake[3]: Entering directory '/home/denis/Documents/vs/paral_bfs/build'
[ 50%] Building CXX object CMakeFiles/main.dir/main.cpp.o
[100%] Linking CXX executable main
gmake[3]: Leaving directory '/home/denis/Documents/vs/paral_bfs/build'
[100%] Built target main
gmake[2]: Leaving directory '/home/denis/Documents/vs/paral_bfs/build'
gmake[1]: Leaving directory '/home/denis/Documents/vs/paral_bfs/build'
ln -sf build/compile_commands.json compile_commands.json

denis@denis-MS-7C52:~/Documents/vs/paral_bfs$ make run
OMP_PROC_BIND=true OMP_PLACES=cores PARLAY_NUM_THREADS=4 \
./build/main --side 300 --runs 5 --threads 4
Running correctness tests...
[PASS] empty_graph
[PASS] single_vertex
[PASS] path_graph
[PASS] cycle_graph
[PASS] star_graph
[PASS] two_components
[PASS] self_loops
[PASS] complete_graph
[PASS] isolated_vertex
[PASS] directed_like
All fixed tests passed.
[PASS] random_graph_0
[PASS] random_graph_1
[PASS] random_graph_2
[PASS] random_graph_3
[PASS] random_graph_4
[PASS] random_graph_5
[PASS] random_graph_6
[PASS] random_graph_7
[PASS] random_graph_8
[PASS] random_graph_9
[PASS] random_graph_10
[PASS] random_graph_11
[PASS] random_graph_12
[PASS] random_graph_13
[PASS] random_graph_14
[PASS] random_graph_15
[PASS] random_graph_16
[PASS] random_graph_17
[PASS] random_graph_18
[PASS] random_graph_19
[PASS] large_star_n=200000
[PASS] large_two_components_path_n=200000
[PASS] large_random_sparse_undirected_n=150000_m=600000
All random tests passed.
[PASS] cube_csr_seq_vs_par_src=0
[PASS] cube_adj_seq_vs_par_src=0
[PASS] cube_csr_vs_adj_seq_src=0
[PASS] cube_csr_seq_vs_par_src=219
[PASS] cube_adj_seq_vs_par_src=219
[PASS] cube_csr_vs_adj_seq_src=219
[PASS] cube_csr_seq_vs_par_src=455
[PASS] cube_adj_seq_vs_par_src=455
[PASS] cube_csr_vs_adj_seq_src=455
Cube consistency tests passed.
All tests passed.
Building CSR cube (300^3) ...
CSR built: n=27000000, m(directed)=161460000

--- RUN 1 ---
SEQ: 5342.4 ms
PAR(4): 1855.82 ms
SPEEDUP: 2.87872

--- RUN 2 ---
SEQ: 4855.78 ms
PAR(4): 1660.04 ms
SPEEDUP: 2.92509

--- RUN 3 ---
SEQ: 5129.93 ms
PAR(4): 1705.95 ms
SPEEDUP: 3.00708

--- RUN 4 ---
SEQ: 5187.77 ms
PAR(4): 1612.62 ms
SPEEDUP: 3.21698

--- RUN 5 ---
SEQ: 4794.42 ms
PAR(4): 1549.5 ms
SPEEDUP: 3.09417

--------------------------
avg seq: 5062.06 ms
avg par: 1676.78 ms
avg speedup: 3.01891
--------------------------
denis@denis-MS-7C52:~/Documents/vs/paral_bfs$ 
