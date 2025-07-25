// ================================= COPY =================================== //
// Project:     The Experimental Bit Algorithms Library
// Name:        copy.hpp
// Description: Implementation of copy, copy_if, copy_n and copy_backward
// Contributor: Bryce Kille [2019]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _COPY_HPP_INCLUDED
#define _COPY_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
#include <type_traits>
#include <math.h>
// Project sources
#include "bitlib/bit-iterator/bit.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {
// ========================================================================== //

struct copy_impl;

template <typename It1, typename It2>
constexpr bit_iterator<It2> copy(
    const bit_iterator<It1>& first,
    const bit_iterator<It1>& last,
    const It2& d_first) {
  return with_bit_iterator_adapter<copy_impl>(first, last, d_first);
}

template <typename It1, typename It2>
constexpr bit_iterator<It2> copy(
    const It1& first,
    const It1& last,
    const bit_iterator<It2>& d_first) {
  return with_bit_iterator_adapter<copy_impl>(first, last, d_first);
}

template <typename It1, typename It2>
constexpr bit_iterator<It2> copy(
    const bit_iterator<It1>& first,
    const bit_iterator<It1>& last,
    const bit_iterator<It2>& d_first) {
  return with_bit_iterator_adapter<copy_impl>(first, last, d_first);
}

// ---------------------------- Copy Algorithms ----------------------------- //
struct copy_impl {
  // Status: Does not work for Input/Output iterators due to distance call
  template <typename RandomAccessIt1, typename RandomAccessIt2>
  constexpr bit_iterator<RandomAccessIt2> operator()(
      bit_iterator<RandomAccessIt1> first,
      bit_iterator<RandomAccessIt1> last,
      bit_iterator<RandomAccessIt2> d_first) {
    // Types and constants
    using dst_word_type = typename bit_iterator<RandomAccessIt2>::word_type;
    using src_word_type = typename bit_iterator<RandomAccessIt1>::word_type;

    // This checks for differing word types and uses an unoptimized copy in that event
    static_assert(std::is_same<dst_word_type, src_word_type>::value, "Both types must be the same");

    using word_type = dst_word_type;
    using size_type = typename bit_iterator<RandomAccessIt2>::size_type;
    constexpr size_type digits = binary_digits<word_type>::value;

    // Assertions
    _assert_range_viability(first, last);
    if (first == last) {
      return d_first;
    }

    // Initialization
    const bool is_d_first_aligned = d_first.position() == 0;
    size_type total_bits_to_copy = distance(first, last);
    size_type remaining_bits_to_copy = total_bits_to_copy;
    auto it = d_first.base();

    // d_first is not aligned. Copy partial word to align it
    if (!is_d_first_aligned) {
      size_type partial_bits_to_copy = ::std::min(
          remaining_bits_to_copy,
          digits - d_first.position());
      *it = _bitblend<word_type>(
          *it,
          static_cast<word_type>(
              get_word<word_type>(first, partial_bits_to_copy)
              << static_cast<word_type>(d_first.position())),
          static_cast<word_type>(d_first.position()),
          static_cast<word_type>(partial_bits_to_copy));
      remaining_bits_to_copy -= partial_bits_to_copy;
      advance(first, partial_bits_to_copy);
      it++;
    }

    if (remaining_bits_to_copy > 0) {
        const bool is_first_aligned = first.position() == 0;
        //size_type words_to_copy = ::std::ceil(remaining_bits_to_copy / static_cast<float>(digits));
        // d_first will be aligned at this point
        if (is_first_aligned && remaining_bits_to_copy > digits) {
            auto N = ::std::distance(first.base(), last.base());
            it = ::std::copy(first.base(), last.base(), it);
            first += digits * N;
            remaining_bits_to_copy -= digits * N;
        } else {
            // TODO benchmark if its faster to ::std::copy the entire range then shift
            while (remaining_bits_to_copy >= digits) {
                *it = get_word<word_type>(first, digits);
                remaining_bits_to_copy -= digits;
                it++;
                advance(first, digits);
            }
        }
        if (remaining_bits_to_copy > 0) {
          *it = _bitblend<word_type>(
              *it,
              get_word<word_type>(first, remaining_bits_to_copy),
              _mask<word_type>(remaining_bits_to_copy));
        }
    }
    return d_first + total_bits_to_copy;
  }
};
// -------------------------------------------------------------------------- //



// ========================================================================== //
}  // namespace bit
#endif // _COPY_HPP_INCLUDED
// ========================================================================== //