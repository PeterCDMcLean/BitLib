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

    size_type d_first_position;
    if constexpr (DstAlign == std::dynamic_extent) {
      d_first_position = d_first.position();
    } else {
      d_first_position = DstAlign;
    }

    const bool is_d_first_aligned = (d_first_position == 0);
    if (!is_d_first_aligned) {
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
      it++;
    }

    if (remaining_bits_to_copy > 0) {
      size_type first_position;
      if constexpr (SrcAlign == std::dynamic_extent) {
        first_position = first.position();
      } else {
        first_position = SrcAlign;
      }

      const bool is_first_aligned = (first_position == 0);
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

// Front-end for iterators with compile-time Alignment.
// Fixed-alignment iterators are not incrementable, so they are converted to
// mutable dynamic_extent iterators before being passed to the implementation.
// SrcAlign/DstAlign are deduced from the iterator types via iterator_alignment_v
// and hinted to impl as template parameters for if constexpr optimization.
// This overload only participates when at least one iterator has fixed Alignment.
// When word types match, alignment hints are passed to impl for if constexpr
// optimization. When word types differ, falls back to with_bit_iterator_adapter
// for cross-width word adaptation (alignment hints are lost in that case).
template <typename SrcIt, typename DstIt>
constexpr iterator<typename DstIt::iterator_type, primitive<1>, std::dynamic_extent> copy(
    const SrcIt& first,
    const SrcIt& last,
    const DstIt& d_first)
  requires(iterator_alignment_v<SrcIt> != std::dynamic_extent || iterator_alignment_v<DstIt> != std::dynamic_extent)
{
  constexpr std::size_t SrcAlign = iterator_alignment_v<SrcIt>;
  constexpr std::size_t DstAlign = iterator_alignment_v<DstIt>;
  // Convert immutable iterators to mutable dynamic_extent iterators
  using mutable_src_it = iterator<typename SrcIt::iterator_type, primitive<1>, std::dynamic_extent>;
  using mutable_dst_it = iterator<typename DstIt::iterator_type, primitive<1>, std::dynamic_extent>;
  auto mutable_first = mutable_src_it(first.base(), first.position());
  auto mutable_last = mutable_src_it(last.base(), last.position());
  auto mutable_d_first = mutable_dst_it(d_first.base(), d_first.position());
  if constexpr (std::is_same_v<typename SrcIt::word_type, typename DstIt::word_type>) {
    // Same word type: no width adaptation needed, use alignment hints
    return copy_impl::run<SrcAlign, DstAlign>(
        mutable_first, mutable_last, mutable_d_first);
  } else {
    // Different word types: use adapter for width adaptation
    return with_bit_iterator_adapter<copy_impl>(
        mutable_first, mutable_last, mutable_d_first);
  }
}
// -------------------------------------------------------------------------- //

// ========================================================================== //
}  // namespace bit
#endif  // _COPY_HPP_INCLUDED \
        // ========================================================================== //