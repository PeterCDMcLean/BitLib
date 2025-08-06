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
  template <std::integral U, std::size_t N, typename RandomAccessIt>
  constexpr static void to_integral(const bit_iterator<RandomAccessIt>& first, U& integral) noexcept {
    bit_pointer<U> integral_begin(&integral);
    ::bit::copy(first, first + bitsof<U>(), integral_begin);
  }

  template <std::integral U, std::size_t N>
  constexpr static void to_integral(const bit_range auto& value, U& integral) noexcept {
    to_integral<U, N>(value.begin(), integral);
  }

  template <std::integral U, std::size_t N, typename RandomAccessIt>
  constexpr static void from_integral(
      const U& integral,
      const bit_iterator<RandomAccessIt>& first,
      const bit_iterator<RandomAccessIt>& last) noexcept {
    const bit_pointer<const U> integral_begin(&integral);
    if constexpr (N == std::dynamic_extent) {
      ::bit::copy(integral_begin, integral_begin + distance(first, last), first);
    } else {
      ::bit::copy(integral_begin, integral_begin + N, first);
    }
  }

  template <std::integral U, std::size_t N>
  constexpr static void from_integral(const U& integral, bit_sized_range auto& value) noexcept {
    const bit_pointer<const U> integral_begin(&integral);
    if constexpr (N == std::dynamic_extent) {
      ::bit::copy(integral_begin, integral_begin + value.size(), value.begin());
    } else {
      ::bit::copy(integral_begin, integral_begin + N, value.begin());
    }
  }
};

struct sign_extend {
  template <std::integral U, std::size_t N, typename RandomAccessIt>
  constexpr static void to_integral(
      const bit_iterator<RandomAccessIt>& first,
      const bit_iterator<RandomAccessIt>& last,
      U& integral) noexcept {
    bit_pointer<U> integral_begin(&integral);
    if constexpr (N == std::dynamic_extent) {
      if constexpr (std::is_signed_v<U>) {
        if (last[-1]) {
          ::bit::fill(integral_begin + distance(first, last), integral_begin + bitsof<U>(), bit1);
        }
      }
    } else {
      if constexpr (std::is_signed_v<U>) {
        if (first[N - 1]) {
          ::bit::fill(integral_begin + N, integral_begin + bitsof<U>(), bit1);
        }
      }
    }
  }

  template <std::integral U, std::size_t N>
  constexpr static void to_integral(const bit_sized_range auto& value, U& integral) noexcept {
    bit_pointer<U> integral_begin(&integral);
    if constexpr (N == std::dynamic_extent) {
      if constexpr (std::is_signed_v<U>) {
        if (value.end()[-1]) {
          ::bit::fill(integral_begin + value.size(), integral_begin + bitsof<U>(), bit1);
        }
      }
    } else {
      if constexpr (std::is_signed_v<U>) {
        if (value.begin()[N - 1]) {
          ::bit::fill(integral_begin + N, integral_begin + bitsof<U>(), bit1);
        }
      }
    }
  }

  template <std::integral U, std::size_t N, typename RandomAccessIt>
  constexpr static void to_integral(
      detail::uninitialized_t,
      const bit_iterator<RandomAccessIt>& first,
      const bit_iterator<RandomAccessIt>& last, U& integral) noexcept {
    bit_pointer<U> integral_begin(&integral);
    if constexpr (N == std::dynamic_extent) {
      if constexpr (std::is_signed_v<U>) {
        ::bit::fill(integral_begin + distance(first, last), integral_begin + bitsof<U>(), last[-1]);
      }
    } else {
      if constexpr (std::is_signed_v<U>) {
        ::bit::fill(integral_begin + N, integral_begin + bitsof<U>(), first[N - 1]);
      }
    }
  }

  template <std::integral U, std::size_t N>
  constexpr static void to_integral(
      detail::uninitialized_t,
      const bit_sized_range auto& value,
      U& integral) noexcept {
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

  template <std::integral U, std::size_t N = std::dynamic_extent, typename RandomAccessIt>
  constexpr static void from_integral(
      const U& integral,
      const bit_iterator<RandomAccessIt>& first,
      const bit_iterator<RandomAccessIt>& last) noexcept {
    if constexpr (N == std::dynamic_extent) {
      if constexpr (std::is_signed_v<U>) {
        if (integral < 0) {
          ::bit::fill(first + bitsof<U>(), last, bit1);
        }
      }
    } else {
      if constexpr (std::is_signed_v<U>) {
        if (integral < 0) {
          ::bit::fill(first + bitsof<U>(), last, bit1);
        }
      }
    }
  }
  template <std::integral U, std::size_t N = std::dynamic_extent>
  constexpr static void from_integral(const U& integral, bit_sized_range auto& value) noexcept {
    from_integral<U, N>(integral, value.begin(), value.end());
  }

  template <std::integral U, std::size_t N = std::dynamic_extent, typename RandomAccessIt>
  constexpr static void from_integral(
      detail::uninitialized_t,
      const U& integral,
      const bit_iterator<RandomAccessIt>& first,
      const bit_iterator<RandomAccessIt>& last) noexcept {
    if constexpr (N == std::dynamic_extent) {
      ::bit::fill(first + bitsof<U>(), last, (integral < 0) ? bit1 : bit0);
    } else {
      ::bit::fill(first + N, last, (integral < 0) ? bit1 : bit0);
    }
  }

  template <std::integral U, std::size_t N = std::dynamic_extent>
  constexpr static void from_integral(
      detail::uninitialized_t,
      const U& integral,
      bit_sized_range auto& value) noexcept {
    from_integral<U, N>(detail::uninitialized, integral, value.begin(), value.end());
  }
};

}  // namespace bit::policy

#endif  // _BIT_POLICY_HPP_INCLUDED
