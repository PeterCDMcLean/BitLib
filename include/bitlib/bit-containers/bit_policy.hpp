// ================================= BIT_ARRAY_REF =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array_ref.hpp
// Description: Implementation of bit_array_ref
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_POLICY_HPP_INCLUDED
#define _BIT_POLICY_HPP_INCLUDED

#include "bitlib/bit_concepts.hpp"
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"

namespace bit::policy {

struct truncate;
struct sign_extend;
template <typename W>
struct typical {
  using allocator = std::allocator<std::remove_const_t<W>>;
  using extension = sign_extend;
  using truncation = truncate;
};

struct truncate {
  template <std::integral U, std::size_t N>
  constexpr static void to_integral(const bit_sized_range auto& value, U& integral) noexcept {
    bit_pointer<U> integral_begin(&integral);
    ::bit::copy(value.begin(), value.begin() + bitsof<U>(), integral_begin);
  }
  template <std::integral U, std::size_t N>
  constexpr static void from_integral(bit_sized_range auto& value, const U& integral) noexcept {
    const bit_pointer<const U> integral_begin(&integral);
    if constexpr (N == std::dynamic_extent) {
      ::bit::copy(integral_begin, integral_begin + value.size(), value.begin());
    } else {
      ::bit::copy(integral_begin, integral_begin + N, value.begin());
    }
  }
};

struct sign_extend {
  template <std::integral U, std::size_t N>
  constexpr static void to_integral(const bit_sized_range auto& value, U& integral) noexcept {
    bit_pointer<U> integral_begin(&integral);
    if constexpr (N == std::dynamic_extent) {
      if constexpr (std::is_signed_v<U>) {
        if (value.last()[-1]) {
          ::bit::fill(integral_begin + value.size(), integral_begin + bitsof<U>(), bit1);
        }
      }
    } else {
      if constexpr (std::is_signed_v<U>) {
        if (value.begin()[N - 1]) {
          ::bit::fill(integral_begin + value.size(), integral_begin + bitsof<U>(), bit1);
        }
      }
    }
  }
  template <std::integral U, std::size_t N>
  constexpr static void to_integral(const bit_sized_range auto& value, U& integral, detail::uninitialized_t) noexcept {
    bit_pointer<U> integral_begin(&integral);
    if constexpr (N == std::dynamic_extent) {
      if constexpr (std::is_signed_v<U>) {
        ::bit::fill(integral_begin + value.size(), integral_begin + bitsof<U>(), value.end()[-1]);
      } else {
        ::bit::fill(integral_begin + value.size(), integral_begin + bitsof<U>(), bit0);
      }
    } else {
      if constexpr (std::is_signed_v<U>) {
        ::bit::fill(integral_begin + value.size(), integral_begin + bitsof<U>(), value.begin()[N - 1]);
      } else {
        ::bit::fill(integral_begin + value.size(), integral_begin + bitsof<U>(), bit0);
      }
    }
  }
  template <std::integral U, std::size_t N = std::dynamic_extent>
  constexpr static void from_integral(bit_sized_range auto& value, const U& integral) noexcept {
    if constexpr (N == std::dynamic_extent) {
      if constexpr (std::is_signed_v<U>) {
        if (integral < 0) {
          ::bit::fill(value.begin() + bitsof<U>(), value.end(), bit1);
        }
      }
    } else {
      if constexpr (std::is_signed_v<U>) {
        if (integral < 0) {
          ::bit::fill(value.begin() + N, value.end(), bit1);
        }
      }
    }
  }
  template <std::integral U, std::size_t N = std::dynamic_extent>
  constexpr static void from_integral(bit_sized_range auto& value, const U& integral, detail::uninitialized_t) noexcept {
    if constexpr (N == std::dynamic_extent) {
      ::bit::fill(value.begin() + bitsof<U>(), value.end(), (integral < 0) ? bit1 : bit0);
    } else {
      ::bit::fill(value.begin() + N, value.end(), (integral < 0) ? bit1 : bit0);
    }
  }
};

}  // namespace bit::policy

#endif  // _BIT_POLICY_HPP_INCLUDED
