cmake_minimum_required(VERSION 3.20)

if (POLICY CMP0168)
  cmake_policy(SET CMP0168 NEW)
endif()
if (POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# set the project name
project(BitLib VERSION 1.0.0)

option(BITLIB_HWY "Build with google highway SIMD extensions" OFF)
option(BITLIB_BENCHMARK "Build bitlib benchmarks" OFF)
option(BITLIB_EXAMPLE "Build bitlib examples" OFF)
option(BITLIB_TEST "Build bitlib tests" OFF)
option(BITLIB_TEST_WERROR "Build bitlib tests with -Werror" OFF)
option(BITLIB_MDSPAN "Accessor Support for mdspan" ON)

add_library(bitlib INTERFACE)
add_library(bitlib::bitlib ALIAS bitlib)
target_include_directories(bitlib INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_sources(bitlib INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bitlib.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit_concepts.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/bit_algorithm_details.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/bit_algorithm.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/copy_backward.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/copy.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/count.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/debug_utils.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/equal.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/fill.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/find.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/libpopcnt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/move.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/reverse.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/rotate.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/shift.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/swap_ranges.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/transform.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-algorithms/type_traits.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-containers/bit_array_dynamic_extent.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-containers/bit_array.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-containers/bit_bitsof.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-containers/bit-containers.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-containers/bit_mdspan_accessor.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-containers/bit_span.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-containers/bit_vector.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-iterator/bit_details.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-iterator/bit.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-iterator/bit_iterator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-iterator/bit_reference.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bitlib/bit-iterator/bit_value.hpp
)

target_compile_features(bitlib INTERFACE cxx_std_23)
include(CheckCXXSourceCompiles)

if (BITLIB_MDSPAN)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(USING_LIBSTDCXX 1)
  endif()
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CODE "
    #include <string>
    #ifdef __GLIBCXX__
    int main() { return 0; }
    #else
    #error Not using libstdc++
    #endif
    ")

    check_cxx_source_compiles("${CODE}" USING_LIBSTDCXX)
  endif()
  if (USING_LIBSTDCXX)
    if (NOT TARGET std::mdspan)
      find_package(mdspan CONFIG)
      if (NOT mdspan_DIR OR "${mdspan_DIR}" STREQUAL "mdspan-NOTFOUND")
        include(FetchContent)

        FetchContent_Declare(
          mdspan
          GIT_REPOSITORY https://github.com/kokkos/mdspan.git
          GIT_TAG 414a5dccf5c775b2eecc13d408b256e94f23d1d2 # stable 2025/04/25
        )
        FetchContent_MakeAvailable(mdspan)
      endif()
    endif()
    target_compile_definitions(bitlib INTERFACE MDSPAN_IMPL_STANDARD_NAMESPACE=std MDSPAN_IMPL_PROPOSED_NAMESPACE=experimental)
    target_link_libraries(bitlib INTERFACE std::mdspan)
  endif()
endif()

if (BITLIB_HWY)
  target_compile_definitions(bitlib INTERFACE -DBITLIB_HWY)
endif()

if (BITLIB_BENCHMARK)
  enable_testing()
  add_subdirectory(benchmark)
endif()

if (BITLIB_EXAMPLE)
  add_subdirectory(example)
endif()

if (BITLIB_TEST)
  enable_testing()
  add_subdirectory(test)
endif()
