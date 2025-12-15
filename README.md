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
PARLAY_NUM_THREADS=4 ./build/main
Building CSR cube (300^3) ...
CSR built: n=27000000, m=161460000

--- RUN 1 ---
SEQ: 4560.2 ms
PAR(4): 2546.33 ms
SPEEDUP: 1.79089

--- RUN 2 ---
SEQ: 4629.17 ms
PAR(4): 2486.97 ms
SPEEDUP: 1.86137

--- RUN 3 ---
SEQ: 5066.45 ms
PAR(4): 2560.66 ms
SPEEDUP: 1.97857

--- RUN 4 ---
SEQ: 5129.74 ms
PAR(4): 2534.32 ms
SPEEDUP: 2.02411

--- RUN 5 ---
SEQ: 4959.37 ms
PAR(4): 2506.89 ms
SPEEDUP: 1.9783

--------------------------
avg seq: 4868.99 ms
avg par: 2527.03 ms
avg speedup: 1.92676
--------------------------
denis@denis-MS-7C52:~/Documents/vs/paral_bfs$ 