cmake_minimum_required(VERSION 3.20)
project(paral_bfs CXX)

include(FetchContent)

set(PARLAY_OPENMP ON CACHE BOOL "Use OpenMP scheduler" FORCE)
set(PARLAY_ELASTIC_PARALLELISM OFF CACHE BOOL "Disable elastic parallelism" FORCE)

FetchContent_Declare(
  parlaylib
  GIT_REPOSITORY https://github.com/cmuparlay/parlaylib.git
  GIT_TAG master
)
FetchContent_MakeAvailable(parlaylib)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE parlay)
target_compile_features(main PRIVATE cxx_std_20)

target_compile_options(main PRIVATE -O3 -DNDEBUG -march=native)
