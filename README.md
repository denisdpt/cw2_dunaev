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
-- Using Parlay scheduler. Switch with -DPARLAY_{CILKPLUS,OPENCILK,OPENMP,TBB}=On
-- Elastic parallelism enabled. Disable with -DPARLAY_ELASTIC_PARALLELISM=Off
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
-- Configuring done (5.7s)
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
--------------------------

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
SEQ: 5726 ms
PAR(4): 1849.67 ms
SPEEDUP: 3.09569

--- RUN 2 ---
SEQ: 5278.48 ms
PAR(4): 1814.18 ms
SPEEDUP: 2.90957

--- RUN 3 ---
SEQ: 5209.37 ms
PAR(4): 1727.86 ms
SPEEDUP: 3.01493

--- RUN 4 ---
SEQ: 5348.74 ms
PAR(4): 1720.53 ms
SPEEDUP: 3.10879

--- RUN 5 ---
SEQ: 4975.24 ms
PAR(4): 1657.99 ms
SPEEDUP: 3.00076

--------------------------
avg seq: 5307.57 ms
avg par: 1754.05 ms
avg speedup: 3.02595
--------------------------
