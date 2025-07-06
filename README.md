# BitLib

![Actions](https://github.com/PeterCDMcLean/BitLib/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=master)[![Coverage Status](https://coveralls.io/repos/github/PeterCDMcLean/BitLib/badge.svg?branch=master)](https://coveralls.io/github/PeterCDMcLean/BitLib?branch=master)

# Overview

This project provides convenient and efficient stl-like containers and algorithms for bit access.

# Table of Contents

- [Contributors](#contributors)
- [Requirements](#requirements)
- [Example](#example)
- [CMake](#cmake)
- [Sized Literal](#literal)
- [Containers](#containers)
  - [bit_array](#bit_array)
  - [bit_vector](#bit_vector)
- [Views](#views)
  - [bit_array_ref](#bit_array_ref)
  - [bit_span](#bit_span)
  - [mdspan with bit_default_accessor](#mdspan)
- [Iterators and References](#iterators_and_references)
  - [bit_iterator](#bit_iterator)
  - [bit_reference](#bit_reference)
  - [bit_word_pointer_adapter](#bit_word_pointer_adapter)
  - [bit_word_reference_adapter](#bit_word_reference_adapter)
- [Algorithms](#algorithms)
- [Testing](#testing)
- [License](#license)

# Contributors <a href="#contributors"></a>
- Vincent Reverdy
- Bryce Kille
- Peter McLean

# Requirements <a href="#requirements"></a>

- _gcc_ or _clang_
- `C++23`
- cmake verison 3.20.
  * cmake version 3.31 if compiling bitlib's benchmark suite

# Example <a href="#example"></a>

# CMake <a href="#cmake"></a>
### options
bitlib provides the following cmake options and their defaults
```cmake
option(BITLIB_HWY "Build with google highway SIMD extensions" OFF)
option(BITLIB_BENCHMARK "Build bitlib benchmarks" OFF)
option(BITLIB_EXAMPLE "Build bitlib examples" OFF)
option(BITLIB_TEST "Build bitlib tests" OFF)
option(BITLIB_TEST_WERROR "Build bitlib tests with -Werror" OFF)
option(BITLIB_MDSPAN "Accessor Support for mdspan" ON)
```

Options can be set through command line switches:
```
cmake -G .. -DBITLIB_MDSPAN=OFF
```
or within cmake files before the bitlib targets have been added
```cmake
set(BITLIB_MDSPAN OFF CACHE BOOL "Disable mdspan" FORCE)
```

The bitlib targets are added through one of two ways:

### add_subdirectory
```cmake
add_subdirectory(..../bitlib)
add_executable(example)
target_sources(example PRIVATE example.cpp)
target_link_libraries(example PRIVATE bitlib::bitlib)
```

### FetchContent
```cmake
include(FetchContent)
FetchContent_Declare(
    bitlib
    GIT_REPOSITORY https://github.com/bkille/bitlib.git
    GIT_TAG origin/master
)
FetchContent_MakeAvailable(bitlib)

add_executable(example)
target_sources(example PRIVATE example.cpp)
target_link_libraries(example PRIVATE bitlib::bitlib)
```

# Literal <a href="#literal"></a>

This provides a sized literal for compile-time bit_array.

There are three components of the literal:
```
   [base_prefix][size']data
           |      |      |
           |--->  |   <--|
ex:             0xC'7B;
```
 - base prefix
 - size
   * before first apostrophe `'`
 - data

### Base
Similar to standard C++ literals, the base is optionally prefixed:

- default base 10
- `0b` binary base
- `0x` hexadecimal base
- `0` octal base

### Size
This user-defined literal repurposes the apostrophe `'` numeric separator in the C++ standard.
The digits of the numeric leading up to the first apostrophe are the number of bits in the literal.
All digits that follow the first apostrophe are digits in the actual numeric.
Any apostophes after the first are considered standard numeric separators.

> [!IMPORTANT]
> The size is expressed in the same base as the rest of the literal

### Data

Numeric literal in the base described

> [!CAUTION]
> This literal does not support negative literals

> [!CAUTION]
> This literal does not support literals above 64 bits

### Examples

```
#include "bitlib/bit.hpp"

auto dec_lit = 31'123456;  // 31 bit base 10 literal
auto hex_lit = 0x1F'1E240; // 31 bit hex literal
auto bin_lit = 0b11111'11110001001000000; //31 bit binary literal
auto oct_lit = 037'361100; // 31 bit octal literal
```

# Containers <a href="#containers"></a>
## bit_array <a href="#bit_array"></a>
## bit_vector <a href="#bit_vector"></a>
# Views <a href="#views"></a>
## bit_array_ref <a href="#bit_array_ref"></a>
## bit_span <a href="#bit_span"></a>
## mdspan with bit_default_accessor <a href="#mdspan"></a>
# Iterators and References <a href="#iterators_and_references"></a>
## bit_iterator <a href="#bit_iterator"></a>
## bit_reference <a href="#bit_reference"></a>
## bit_word_pointer_adapter <a href="#bit_word_pointer_adapter"></a>
## bit_word_reference_adapter <a href="#bit_word_reference_adapter"></a>
# Algorithms <a href="#algorithms"></a>
 - accumulate
 - copy_backward
 - copy
 - count
 - equal
 - fill
 - find
 - move
 - reverse
 - rotate
 - shift
 - swap_ranges
 - to_string
 - transform






**This repository acts as an efficient replacement of `std::vector<bool>`. It provides implementations of many of the functions in [`<algorithms>`](https://en.cppreference.com/w/cpp/algorithm) optimized for containers of bits, in addition to providing a `bit_vector` class which has roughly the same interface as `std::vector<bool>`**.

This project is built on "[bit iterators](https://github.com/vreverdy/bit)" developed by Vincent Reverdy and many of the implementations in `include/bit-algorithms` come from some of my previous work with Vincent [here](https://github.com/vreverdy/bit-algorithms).

# Example
The code below is from `example/src/example1.cpp`. While the type of word that the bitvector is built off of is templated and you can use any unsigned type, it is likely that you'll want to use `uint64_t` or another 64 bit unsigned type, as that will leverage the most bit-parallelism.
```cpp
#include <iostream>
#include "bitlib/bitlib.hpp"

int main() {
    bit::bit_vector<unsigned char> bv1 ("011111010010");
    std::cout << "Original bitvec:  " << bv1.debug_string() << std::endl;
    // Original bitvec:  01111101 0010

    // Same behavior as std::reverse
    bit::reverse(bv1.begin(), bv1.end());
    std::cout << "Reversed bitvec:  " << bv1.debug_string() << std::endl;
    // Reversed bitvec:  01001011 1110

    // Same behavior as std::rotate
    bit::rotate(bv1.begin(), bv1.begin() + 3, bv1.end());
    std::cout << "Rotated bitvec:   " << bv1.debug_string() << std::endl;
    // Rotated bitvec:   01011111 0010

    // Same behavior as the corresponding std::vector::push_back and std::vector::insert
    bv1.push_back(bit::bit0);
    bv1.insert(bv1.end(), 10, bit::bit1);
    std::cout << "Extended bitvec:  " << bv1.debug_string() << std::endl;
    // Extended bitvec:  01011111 00100111 1111111

    return 0;
}
```

# Installation
BitLib is a header-only libarary. Currently, the BitLib library requires at least `-std=c++17`.


## CMake
You can automatically fetch the library using Cmake's `FetchContent`.

```cmake
include(FetchContent)
FetchContent_Declare(
    bitlib
    GIT_REPOSITORY https://github.com/bkille/bitlib.git
    GIT_TAG origin/master
)
FetchContent_MakeAvailable(bitlib)

add_executable(example example.cpp)
target_link_libraries(example bitlib::bitlib)
```

## Manual include
Alternatively, you can copy the `include/bitlib` directory to somewhere in your include path.

## SIMD support, testing and benchmarking

SIMD support (enabled via Google's [highway](https://github.com/google/highway) library) can be enabled by defining `BITLIB_HWY`. For example, with `cmake`, you can run `cmake -DBITLIB_HWY=1`. Other options can be found in the `CMakeLists.txt` file:

```cmake
option(BITLIB_HWY "Build with google highway SIMD extensions" OFF)
option(BITLIB_BENCHMARK "Build bitlib benchmarks" OFF)
option(BITLIB_EXAMPLE "Build bitlib examples" OFF)
option(BITLIB_TEST "Build bitlib tests" OFF)
option(BITLIB_PROFILE "Buid simple example for profiling" OFF)
option(BITLIB_COVERAGE "Compute test coverage" OFF)
```

# Usage
The goal of BitLib is to be as similar to the C++ STL as possible. The interface of most functions and classes are the same as they are in the STL. Instead of the values being `bool`, we have `bit::bit_value`, which can take on either `bit::bit0` or `bit::bit1`.

## Containers
 Right now, the only container I have implemented is the bitvector. `bit::bit_vector<WordType>` is essentially a wrapper around `std::vector<WordType>`. The interfaces are nearly identical. In addition to the normal `vector` constructors, you can also provide a string to construct your bitvector:
```cpp
using WordType = uint64_t;
bit::bit_vector<WordType> bvec1 ("011111010010");
```

While the type of word that the bitvector is built off of is templated and you can use any unsigned type, it is likely that you'll want to use `uint64_t` or another 64 bit unsigned type, as that will leverage the most bit-parallelism.

## Algorithms
The algorithms again work in the same manner as the STL. The functions provided here have the same interface as those in the STL, however under the hood, they take advantage of bit-parallelism. It should be noted that if there is an STL algorithm that is not supported yet by BitLib, you can still use the STL implementation. For example:
```cpp
using WordType = uint64_t;
bit::bit_vector<WordType> bvec1 ("011111010010");
bit::bit_vector<WordType> bvec2 = bvec1;
bit::equal(bvec1.begin(), bvec1.end(), bvec2.begin(), bvec1.end());
std::equal(bvec1.begin(), bvec1.end(), bvec2.begin(), bvec1.end()); // Also works, but much slower as it works bit-by-bit
```

For algorithms which take a function (i.e. `bit::transform`), the function should have `WordType` as the input types as well as the return type. For example, to compute the intersection of two bitvectors:
```cpp
using WordType = uint64_t;
auto binary_op = std::bit_and<WordType>();

// Store the AND of bitvec1 and bitvec2 in bitvec3
auto bitret = bit::transform(
        bitvec1.begin(),
        bitvec1.end(),
        bitvec2.begin(),
        bitvec3.begin()
        binary_op);
```

## Iterators
The bit-iterators are the foundation of the library. In most cases, users will only need to work w/ the `bit::bit_vector::begin()` and `bit::bit_vector::end()` methods to obtain iterators. However, constructing a bit iterator from any address is also straightforward:
```cpp
using WordType = uint64_t;
std::array<WordType, 4> wordArr = {1,2,3,4};
bit::bit_iterator<WordType*>(&(wordArr[0])); // Constructs a bit iterator starting from the first bit from the first word of the vector
bit::bit_iterator<WordType*>(&(wordArr[0]), 1); // Constructs a bit iterator from the second bit (position 1) of the first word of the vector
```

In order to grab the underlying word that a bit pointed to by a bit_iterator comes from, you can use the `bit_iterator.base()` function.

It is worth noting that the "position" of a bit always increases from LSB to MSB. For those looking to create their own algorithms from bit_iterators, this can be a common "gotcha". For example, shifting a word to the right by `k` will eliminate the first `k` bits of the container. This is only important to those implementing their own algorithms. `bit::shift_*` works as described in the documentation i.e. `shift_right` shifts the container towards `end()` and `shift_left` shifts the container towards `begin()`.

```
       MSB|<-----|LSB
Position: 76543210
Value:    01010001 --> Sequence: 10001010


// bit::shift_right by 2
       MSB|<-----|LSB
Position: 76543210
Value:    01000100 --> Sequence: 00100010
```

# Documentation
Given that the majority of the library is focused on having the same interface as the C++ STL iterators, containers, and algorithms, users should use the official [STL documentation website](https://en.cppreference.com/). We do plan on adding our own documentation in the future, however.
