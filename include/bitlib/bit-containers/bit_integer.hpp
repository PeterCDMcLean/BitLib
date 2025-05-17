// ================================= BIT_ARRAY_BASE =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_integer.hpp
// Description: Base implementation for bit_array variants
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_INTEGER_HPP_INCLUDED
#define _BIT_INTEGER_HPP_INCLUDED

// C++ standard library
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>  // memcpy
#include <span>
#include <string>
#include <type_traits>
#include <vector>

// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

template <size_t N = std::dynamic_extent, bool issigned = true, typename W = uint8_t>
class bit_integer : public bit_array<bit_value, N, std::align_val_t(alignof(W)), W> {
 public:
  using base_class = bit_array<bit_value, N, std::align_val_t(alignof(W)), W>;

 private:
  // Hide bit_array methods specific to containerization
  // so that this type is focused on arithmetic and native-like look and feel.
  // The type can still be downcasted to get a bit_array;
  using base_class::at;
  using base_class::begin;
  using base_class::cbegin;
  using base_class::cend;
  using base_class::data;
  using base_class::empty;
  using base_class::end;
  using base_class::max_size;
  using base_class::size;
  using base_class::operator[];
  using base_class::operator();
  using base_class::debug_string;
  using base_class::fill;

 public:
  // Use select type
  using typename base_class::size_type;
  using typename base_class::word_type;

  // Use all of bit_array constructors
  using base_class::bit_array;
  // Must define some of our own constructors
  constexpr bit_integer(const base_class& other) : base_class(other) {}

  constexpr bool operator==(const base_class& other) const {
    return static_cast<base_class>(*this) == other;
  }

  constexpr bool operator==(const bit_sized_range auto& other) const {
    return static_cast<base_class>(*this) == other;
  }

  constexpr bit_integer& operator++() {
    unsigned char carry = 1;
    ::bit::transform(begin(), end(), begin(),
                     [&carry](const word_type& a) -> word_type {
                       word_type result;
                       carry = ::bit::add_carry(carry, a, static_cast<word_type>(0), &result);
                       return result;
                     });
    return *this;
  }

  constexpr bit_integer operator++(int) {
    bit_integer old = *this;
    ++(*this);
    return old;
  }

  constexpr bit_integer& operator--() {
    unsigned char carry = 1;
    ::bit::transform(begin(), end(), begin(),
                     [&carry](const word_type& a) -> word_type {
                       word_type result;
                       carry = ::bit::sub_borrow(carry, a, static_cast<word_type>(0), &result);
                       return result;
                     });
    return *this;
  }

  constexpr bit_integer operator--(int) {
    bit_integer old = *this;
    --(*this);
    return old;
  }

  constexpr bit_integer operator+(const bit_integer& other) const {
    bit_integer result;

    unsigned char carry = 0;
    ::bit::transform(begin(), end(), other.begin(), result.begin(),
                     [&carry](const word_type& a, const word_type& b) -> word_type {
                       word_type result;
                       carry = ::bit::add_carry(carry, a, b, &result);
                       return result;
                     });
    return result;
  }

  constexpr bit_integer& operator+=(const bit_integer& other) {
    unsigned char carry = 0;
    ::bit::transform(begin(), end(), other.begin(), begin(),
                     [&carry](const word_type& a, const word_type& b) -> word_type {
                       word_type result;
                       carry = ::bit::add_carry(carry, a, b, &result);
                       return result;
                     });
    return *this;
  }

  constexpr bit_integer& operator-(const bit_integer& other) const {
    bit_integer result;

    unsigned char carry = 0;
    ::bit::transform(begin(), end(), other.begin(), result.begin(),
                     [&carry](const word_type& a, const word_type& b) -> word_type {
                       word_type result;
                       carry = ::bit::sub_borrow(carry, a, b, &result);
                       return result;
                     });
    return result;
  }

  constexpr bit_integer& operator-=(const bit_integer& other) {
    unsigned char carry = 0;
    ::bit::transform(begin(), end(), other.begin(), begin(),
                     [&carry](const word_type& a, const word_type& b) -> word_type {
                       word_type result;
                       carry = ::bit::sub_borrow(carry, a, b, &result);
                       return result;
                     });
    return *this;
  }

  constexpr bit_integer operator*(const bit_integer& other) const { return {}; }
  constexpr bit_integer& operator*=(const bit_integer& other) { return *this; }
  constexpr bit_integer operator/(const bit_integer& other) const { return {}; }
  constexpr bit_integer& operator/=(const bit_integer& other) { return *this; }
  constexpr bit_integer operator%(const bit_integer& other) const { return {}; }
  constexpr bit_integer& operator%=(const bit_integer& other) { return *this; }

  constexpr bool operator>(const bit_integer& other) const { return false; }
  constexpr bool operator>=(const bit_integer& other) const { return false; }
  constexpr bool operator<(const bit_integer& other) const { return false; }
  constexpr bool operator<=(const bit_integer& other) const { return false; }

  constexpr bit_integer operator>>(const size_type shift) const { return {}; }
  constexpr bit_integer& operator>>=(const size_type shift) { return *this; }
  constexpr bit_integer operator<<(const size_type shift) const { return {}; }
  constexpr bit_integer& operator<<=(const size_type shift) { return *this; }
};

}  // namespace bit

#endif  // _BIT_INTEGER_HPP_INCLUDED
