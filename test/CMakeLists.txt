
###########
# Testing #
###########

include(FetchContent)
include(GoogleTest)

if (NOT TARGET GTest::gtest)
  find_package(GTest)
  if (NOT GTest_DIR OR "${GTest_DIR}" STREQUAL "GTest_DIR-NOTFOUND")
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG 6910c9d9165801d8827d628cb72eb7ea9dd538c5 # v1.16.x
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()
endif()

add_executable(bitlib-tests)

target_sources(bitlib-tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fixtures.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-array.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-bitwise.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-copy_backward.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-copy.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-count.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-equal.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-fill.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-find.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-iterator_adapter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-literal.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-move.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-reverse.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-rotate.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-shift.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-span.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-swap_ranges.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-transform.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-usecase.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test-array_ref.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vector_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/test_utils.hpp
)
if (BITLIB_MDSPAN)
  target_sources(bitlib-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/test-mdspan.cpp)
endif()

target_include_directories(bitlib-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

if (BITLIB_TEST_WERROR)
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(bitlib-tests PRIVATE -Werror)
  elseif (MSVC)
    target_compile_options(bitlib-tests PRIVATE /WX)
  endif()
endif()

# specify test-specific libraries
target_link_libraries(bitlib-tests PRIVATE bitlib GTest::gtest GTest::gtest_main)

if (NOT BITLIB_GTEST_REPEAT)
  set(BITLIB_GTEST_REPEAT 1)
endif()

gtest_discover_tests(
    bitlib-tests
    EXTRA_ARGS --gtest_repeat=${BITLIB_GTEST_REPEAT})
