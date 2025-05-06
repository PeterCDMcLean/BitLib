set(BENCHMARK_ENABLE_GTEST_TESTS OFF)

include(FetchContent)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)

find_package(benchmark CONFIG)
if (NOT benchmark_DIR OR "${benchmark_DIR}" STREQUAL "benchmark-NOTFOUND")
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG afa23b7699c17f1e26c88cbf95257b20d78d6247 #v1.9.2
  )
  FetchContent_MakeAvailable(benchmark)
endif()

find_package(sul-dynamic_bitset)
if (NOT sul-dynamic_bitset_DIR OR "${sul-dynamic_bitset_DIR}" STREQUAL "sul-dynamic_bitset-NOTFOUND")
  FetchContent_Declare(
    sul-dynamic_bitset
    GIT_REPOSITORY https://github.com/pinam45/dynamic_bitset.git
    GIT_TAG ef647f07a663a4cbbd3bf8c70848876e63b64ce1 #v1.3.2
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
  )
  FetchContent_MakeAvailable(sul-dynamic_bitset)
endif()

if (NOT TARGET itsy::bitsy)

  set(ITSY_BITSY_SINGLE OFF CACHE BOOL "" FORCE)
  set(ITSY_BITSY_TESTS OFF CACHE BOOL "" FORCE)

  find_package(itsy.bitsy)
  if (NOT itsy.bitsy_DIR OR "${itsy.bitsy_DIR}" STREQUAL "itsy.bitsy-NOTFOUND")
  FetchContent_Declare(
    itsy.bitsy
    GIT_REPOSITORY https://github.com/ThePhD/itsy_bitsy.git
    GIT_TAG d5b6bf9509bb2dff6235452d427f0b1c349d5f8b # main @ Aug 1, 2022
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
  )
  FetchContent_MakeAvailable(itsy.bitsy)
  endif()
endif()

FetchContent_Declare(
  BitArray
  GIT_REPOSITORY https://github.com/noporpoise/BitArray.git
  GIT_TAG 560ed785a0687b35301a8c23b8bb9fc06b69d827 # v2.0
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(BitArray)
add_library(BitArray SHARED EXCLUDE_FROM_ALL)
target_sources(BitArray PRIVATE
  ${CMAKE_BINARY_DIR}/_deps/bitarray-src/bit_array.c
  PUBLIC
  ${CMAKE_BINARY_DIR}/_deps/bitarray-src/bit_array.h
  ${CMAKE_BINARY_DIR}/_deps/bitarray-src/bit_macros.h
)
target_include_directories(BitArray PUBLIC
  ${CMAKE_BINARY_DIR}/_deps/bitarray-src
)

# Add targets
add_executable(bitlib-bench EXCLUDE_FROM_ALL)

target_sources(bitlib-bench PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark_main.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/copy_backward_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/copy_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/count_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/equal_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fill_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/find_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/move_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/reverse_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rotate_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rw_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/shift_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/swap_ranges-bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/transform_bench.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/benchmark_utils.hpp
)
target_include_directories(bitlib-bench PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(bitlib-bench PRIVATE
  bitlib
  benchmark::benchmark
  sul::dynamic_bitset
  BitArray)

add_test(
  NAME BenchmarkTest
  COMMAND $<TARGET_FILE:bitlib-bench> --benchmark_format=json --benchmark_out=benchmark_result.json
)
