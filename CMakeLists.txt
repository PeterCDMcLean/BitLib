# specify the C++ standard
cmake_minimum_required(VERSION 3.14)

# set the project name
project(Bit-Vector VERSION 0.3.0)

# set output directory of builds
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set up linters/checkers
#set(CMAKE_CXX_CPPCHECK cppcheck;--std=c++17;--file-filter=*bitlib*)
#set(CMAKE_CXX_CPPLINT cpplint;--linelength=100;--filter=-whitespace;)
#set(CMAKE_CXX_CLANG_TIDY
  #clang-tidy;
  #-header-filter=include/;)

add_library(bitlib INTERFACE)
add_library(bitlib::bitlib ALIAS bitlib)
target_include_directories(bitlib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(bitlib INTERFACE cxx_std_23)

add_library(bitlib_utils INTERFACE)
target_include_directories(bitlib_utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/utils)
target_sources(bitlib_utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/utils/test_utils.hpp)


option(BITLIB_HWY "Build with google highway SIMD extensions" OFF)
option(BITLIB_BENCHMARK "Build bitlib benchmarks" OFF)
option(BITLIB_EXAMPLE "Build bitlib examples" OFF)
option(BITLIB_TEST "Build bitlib tests" OFF)
option(BITLIB_PROFILE "Buid simple example for profiling" OFF)
option(BITLIB_COVERAGE "Compute test coverage" OFF)
option(BITLIB_MDSPAN "Compute test coverage" ON)

if (BITLIB_MDSPAN)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        find_package(mdspan CONFIG REQUIRED)
        target_compile_definitions(bitlib INTERFACE MDSPAN_IMPL_STANDARD_NAMESPACE=std MDSPAN_IMPL_PROPOSED_NAMESPACE=experimental)
        target_link_libraries(bitlib INTERFACE std::mdspan)
    endif()
endif()

if (BITLIB_HWY)
    add_definitions(-DBITLIB_HWY)
endif()

if(BITLIB_BENCHMARK)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
    add_subdirectory(benchmark)
endif()
if(BITLIB_EXAMPLE)
    add_subdirectory(example)
endif()
if(BITLIB_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

if(BITLIB_PROFILE)
    add_subdirectory(profile)
endif()

