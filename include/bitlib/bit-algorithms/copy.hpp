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

// ---------------------------- Copy Algorithms ----------------------------- //
struct copy_impl {
  // Status: Does not work for Input/Output iterators due to distance call
  // Backward-compatible entry point used by with_bit_iterator_adapter
  template <typename RandomAccessIt1, typename RandomAccessIt2>
  constexpr bit_iterator<RandomAccessIt2> operator()(
      bit_iterator<RandomAccessIt1> first,
      bit_iterator<RandomAccessIt1> last,
      bit_iterator<RandomAccessIt2> d_first) {
    return run<std::dynamic_extent, std::dynamic_extent>(
        first, last, d_first);
  }

  // Entry point with compile-time alignment hints
  template <std::size_t SrcAlign, std::size_t DstAlign,
            typename RandomAccessIt1, typename RandomAccessIt2>
  static constexpr iterator<RandomAccessIt2, primitive<1>, std::dynamic_extent> run(
      iterator<RandomAccessIt1, primitive<1>, std::dynamic_extent> first,
      iterator<RandomAccessIt1, primitive<1>, std::dynamic_extent> last,
      iterator<RandomAccessIt2, primitive<1>, std::dynamic_extent> d_first) {
    // Types and constants
    using dst_word_type = typename bit_iterator<RandomAccessIt2>::word_type;
    using src_word_type = typename bit_iterator<RandomAccessIt1>::word_type;

    // This checks for differing word types and uses an unoptimized copy in that event
    static_assert(std::is_same<dst_word_type, src_word_type>::value, "Both types must be the same");
    //static_assert(SrcAlign != std::dynamic_extent, "SrcAlign must be zero for compile-time alignment optimization");

    using word_type = dst_word_type;
    using size_type = typename bit_iterator<RandomAccessIt2>::size_type;
    constexpr size_type digits = binary_digits<word_type>::value;

    // Assertions
    _assert_range_viability(first, last);
    if (first == last) {
      return d_first;
    }

    // Initialization
    size_type total_bits_to_copy = distance(first, last);
    size_type remaining_bits_to_copy = total_bits_to_copy;
    auto it = d_first.base();

    // If the destination Alignment is known at compile time, skip the
    // run-time position check entirely. DstAlign == 0 means d_first is already
    // word-aligned, so there is no prefix to write.
    bool consumed_source_prefix = false;
    if constexpr (DstAlign == 0) {
      // d_first is aligned: nothing to do.
    } else {
      size_type d_first_position;
      if constexpr (DstAlign == std::dynamic_extent) {
        d_first_position = d_first.position();
      } else {
        d_first_position = DstAlign;
      }
      if (d_first_position != 0) {
        size_type partial_bits_to_copy = ::std::min(
            remaining_bits_to_copy,
            digits - d_first_position);
        *it = _bitblend<word_type>(
            *it,
            static_cast<word_type>(
                get_word<word_type>(first, partial_bits_to_copy)
                << static_cast<word_type>(d_first_position)),
            d_first_position,
            partial_bits_to_copy);
        remaining_bits_to_copy -= partial_bits_to_copy;
        advance(first, partial_bits_to_copy);
        consumed_source_prefix = true;
        it++;
      }
    }

    if (remaining_bits_to_copy > 0) {
      // Use the compile-time SrcAlign hint where it still applies. If the
      // partial destination word above advanced the source, the hint may no
      // longer describe the iterator and the run-time position is consulted.
      bool is_first_aligned;
      if constexpr (SrcAlign == 0) {
        // Aligned on entry. A prefix write advances by less than one word, so
        // the source is unaligned afterwards.
        is_first_aligned = !consumed_source_prefix;
      } else if constexpr (SrcAlign == std::dynamic_extent) {
        is_first_aligned = (first.position() == 0);
      } else {
        // Known unaligned on entry. It only wraps back to a word boundary when
        // source and destination share the same fixed Alignment and the prefix
        // consumed the rest of the destination word.
        if constexpr (DstAlign == SrcAlign) {
          is_first_aligned = consumed_source_prefix;
        } else {
          is_first_aligned = consumed_source_prefix && (first.position() == 0);
        }
      }
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

// Front-end for bit iterators of any Alignment (including fixed, non-dynamic
// Alignment) and raw pointers.
//
// Fixed-alignment iterators are not incrementable, so the adapter converts
// every input to a mutable dynamic_extent iterator before dispatching to impl.
// It extracts SrcAlign/DstAlign from the input types via iterator_alignment_v
// and hints them to impl as template parameters for if constexpr optimization.
// Raw pointers denote word-aligned ranges and therefore have Alignment 0.
//
// first and last may have different Alignments (e.g. aligned_begin() and
// aligned_end() of a fixed-size container). When the word types match, the
// alignment hints are passed to impl. When they differ, the same hints are
// translated to the adapted word width inside with_bit_iterator_adapter.
template <typename SrcIt1, typename SrcIt2, typename DstIt>
constexpr copy_return_t<DstIt> copy(
    const SrcIt1& first,
    const SrcIt2& last,
    const DstIt& d_first) {
  static_assert(copy_iterator_like<SrcIt1> && copy_iterator_like<SrcIt2> &&
                    copy_iterator_like<DstIt>,
                "copy() requires bit iterators or raw pointers");
  static_assert(
      std::is_same_v<copy_iterator_type_t<SrcIt1>, copy_iterator_type_t<SrcIt2>>,
      "first and last must have the same underlying iterator type");
  return with_bit_iterator_adapter<copy_impl>(first, last, d_first);
}
// -------------------------------------------------------------------------- //

// ========================================================================== //
}  // namespace bit
#endif  // _COPY_HPP_INCLUDED \
        // ========================================================================== //