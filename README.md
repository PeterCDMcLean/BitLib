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
- [Slice Operator](#slice-operator)
- [Policy](#policy)
- [Owning Containers](#owning-containers)
  - [bit_array](#bit_array)
  - [bit_vector](#bit_vector)
- [Non-owning Views](#non-owning-views)
  - [bit_array_ref](#bit_array_ref)
  - [bit_span](#bit_span)
  - [mdspan with bit_default_accessor](#mdspan-with-bit-accessors)
- [Iterators and References](#iterators-and-references)
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
```bash
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

There are four components of the literal:
```
   [base_prefix][size']data_b
           |      |      |  |
           |--->  |   <--|  |--- '_b' suffix
ex:             0xC'7B_b;
```
 - base prefix
 - size
   * before first apostrophe `'`
 - data
 - `_b` suffix

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

### Suffix

The literal must be followed by the user defined literal suffix: _b

### Examples

```c++
#include "bitlib/bit.hpp"

auto dec_lit = 31'123456_b;  // 31 bit base 10 literal
auto hex_lit = 0x1F'1E240_b; // 31 bit hex literal
auto bin_lit = 0b11111'11110001001000000_b; //31 bit binary literal
auto oct_lit = 037'361100_b; // 31 bit octal literal
```

# Slice Operator <a href="#slice-operator"></a>

All containers and views provide a slice operator which take a half-open range and
return a [mutable view](#bit_array_ref) of the given range.
Example:

```c++
auto lit = 0x1F'1E240_b;
auto ref = lit(4, 8); // reference the bits from [4,8) i.e. 4,5,6,7.
assert(ref == 0x4'4_b);
assert(ref == 0x4);
lit(4, 8) = 0xA;
assert(lit == 0x1F'1E2A0_b);
assert(ref(1,4) == 0x5); // array_ref can be sliced further
```

# Policy <a href="#policy"></a>

Template class controlling behavior such as expansion (aka sign-extension) and truncation
The containers can be specialized with a custom policy type to
throwing an exception on loss of data or clamping instead of truncation

The default policy truncates when necessary and sign extends for conversion to/from signed integrals

# Owning Containers <a href="#owning-containers"></a>

## bit_array <a href="#bit_array"></a>

Provides compile-time or construction time container for an array of bits.

### Compile-time:
Storage is on the stack. The number of bytes is the nearest power of two integral size.
```c++
bit_array<11> vec_11(0x123); // Will use a uint16_t to hold the data
bit_array<65> vec_65(); // Will use a uint64_t to hold the data
```
The storage word type can be specified:
```c++
bit_array<65, uint8_t> vec_65_bytes(); // 9 bytes on stack
```

### Construction-time
A non-resizable construction-time storage is used when the N (aka Extent)
is equal to std::dynamic_extent (similar to std::span).
Since the N template parameter is by default std::dynamic_extent this is the
default template specialization `bit_array<>`.

```c++
bit_array<> vec_11(11, 0x123); // 8 bytes on stack for data, 8 bytes for size
bit_array<> vec_64(65); // same stack size as above + 16 bytes in heap
```
The storage word size is by default uintptr_t.
The container will perform small buffer optimization
when the number of bits is equal or less than `bitsof<uintptr_t>()` typically 64.

## bit_vector <a href="#bit_vector"></a>
# Non-Owning Views <a href="#non-owning-views"></a>
## bit_array_ref <a href="#bit_array_ref"></a>
## bit_span <a href="#bit_span"></a>
## mdspan with bit accessors <a href="#mdspan-with-bit-accessors"></a>

The std::mdspan container (C++23) can be used with a custom accessors that
use proxy pointers and references. This makes it suitable for accessing multi-dimensional
bit dense data.

There are three flavours of accessors:
 - `bit_default_accessor` which povides individual `bit_value` access which behaves like a typical stl mdspan
 - `bit_word_accessor<N>` which provides compile-time bit array value type access
 - `bit_word_accessor<std::dynamic_extent>` which provides construction-time bit array value type access
> [!INFO]
> The dynamic_extent `bit_word_accessor` requires a non-default constructor to the accessor.
> The mdspan must use the mdspan constructor which takes the
> container pointer, extent instance and accessor instance.
bit_value:
```c++
bit_array<7*8*9> bits();
std::mdspan<
  bit::bit_value,
  std::extents<size_t, 7, 8, 9>,
  std::layout_right,
  bit::bit_default_accessor
> myspan (
  &bits[0]
);
myspan[6, 7, 8] = bit::bit1; // set last bit to one
assert(bits[7*8*9-1] == bit::bit1);
```

Compile-time:
```c++
bit_array<7*8*9> bits();
std::mdspan<
  bit::bit_word_accessor<7>::element_type,
  std::extents<size_t, 8, 9>,
  std::layout_right,
  bit::bit_word_accessor<7>
> myspan (
  &bits[0]
);
myspan[7, 8] = 0x7'7F_b; // set last 7 bit word to all ones.
assert(bits(7*8*9-7, 7*8*9) == 0x7F);
```

Construction-time:
```c++
bit_array<> bits(7*8*9);
std::mdspan<
  bit::bit_word_accessor<>::element_type,
  std::dextents<size_t, 2>,
  std::layout_right,
  bit::bit_word_accessor<>
> myspan (
  &bits[0],
  std::dextents<size_t, 2>{8, 9},
  bit::bit_word_accessor<>(7)
);
myspan[7, 8] = 0x7'7F_b; // set last 7 bit word to all ones.
assert(bits(7*8*9-7, 7*8*9) == 0x7F);
```

# Iterators and References <a href="#iterators-and-references"></a>
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
 - to_from_string
 - transform



