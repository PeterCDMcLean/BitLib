// ================================= EQUAL =================================== //
// Project:     The Experimental Bit Algorithms Library
// Name:        equal.hpp
// Contributor: Bryce Kille [2019]
//              Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _EQUAL_HPP_INCLUDED
#define _EQUAL_HPP_INCLUDED
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
struct equal_impl;

// Status: Does not work for Input/Output iterators due to distance call
template <typename RandomAccessIt1, typename RandomAccessIt2>
constexpr bool equal(
    const bit_iterator<RandomAccessIt1>& first,
    const bit_iterator<RandomAccessIt1>& last,
    const bit_iterator<RandomAccessIt2>& d_first) {
  return with_bit_iterator_adapter<equal_impl>(first, last, d_first);
}

// ---------------------------- Equal Algorithms ----------------------------- //

// Status: Does not work for Input/Output iterators due to distance call
struct equal_impl {
  // Backward-compatible entry point used by with_bit_iterator_adapter
  template <typename RandomAccessIt1, typename RandomAccessIt2>
  constexpr bool operator()(
      bit_iterator<RandomAccessIt1> first,
      bit_iterator<RandomAccessIt1> last,
      bit_iterator<RandomAccessIt2> d_first) {
    return run<std::dynamic_extent, std::dynamic_extent>(first, last, d_first);
  }

  // Entry point with compile-time alignment hints
  template <std::size_t SrcAlign, std::size_t DstAlign,
            typename RandomAccessIt1, typename RandomAccessIt2>
  static constexpr bool run(
      iterator<RandomAccessIt1, primitive<1>, std::dynamic_extent> first,
      iterator<RandomAccessIt1, primitive<1>, std::dynamic_extent> last,
      iterator<RandomAccessIt2, primitive<1>, std::dynamic_extent> d_first) {
    // Types and constants
    using dst_word_type = typename bit_iterator<RandomAccessIt2>::word_type;
    using src_word_type = typename bit_iterator<RandomAccessIt1>::word_type;
    static_assert(::std::is_same<dst_word_type, src_word_type>::value, "Underlying word types must be equal");
    using word_type = dst_word_type;
    using size_type = typename bit_iterator<RandomAccessIt2>::size_type;
    constexpr size_type digits = binary_digits<word_type>::value;

    // Assertions
    _assert_range_viability(first, last);
    if (first == last) {
      return true;
    }

    // Initialization
    const bool is_d_first_aligned = (DstAlign == std::dynamic_extent)
                                        ? (d_first.position() == 0)
                                        : false;
    size_type total_bits_to_check = distance(first, last);
    size_type remaining_bits_to_check = total_bits_to_check;
    auto it = d_first.base();

    if constexpr (DstAlign != 0) {
      // d_first is not known to be aligned at bit 0
      if (!is_d_first_aligned) {
        const size_type partial_bits_to_check = ::std::min(
            remaining_bits_to_check,
            digits - d_first.position());
        const word_type mask = _mask<word_type>(partial_bits_to_check, d_first.position());
        const word_type comp = static_cast<word_type>(
            get_word<word_type>(first, partial_bits_to_check)
            << d_first.position());
        if ((mask & *it) != (mask & comp)) {
          return false;
        }
        remaining_bits_to_check -= partial_bits_to_check;
        advance(first, partial_bits_to_check);
        it++;
      }
    }

    if (remaining_bits_to_check > 0) {
      const bool is_first_aligned = (SrcAlign == std::dynamic_extent)
                                        ? (first.position() == 0)
                                        : false;
      // d_first will be aligned at this point
      if constexpr (SrcAlign == 0) {
        // first is always aligned at bit 0
        if (remaining_bits_to_check >= digits) {
          auto N = ::std::distance(first.base(), last.base());
          bool found_mismatch = !::std::equal(first.base(), last.base(), it);
          if (found_mismatch) {
            return false;
          }
          it += N;
          first += digits * N;
          remaining_bits_to_check -= digits * N;
        }
      } else if (is_first_aligned && remaining_bits_to_check >= digits) {
        auto N = ::std::distance(first.base(), last.base());
        bool found_mismatch = !::std::equal(first.base(), last.base(), it);
        if (found_mismatch) {
          return false;
        }
        it += N;
        first += digits * N;
        remaining_bits_to_check -= digits * N;
      } else {
        // TODO benchmark if its faster to ::std::check the entire range then shift
        while (remaining_bits_to_check >= digits) {
          if (*it != get_word<word_type>(first, digits)) {
            return false;
          }
          remaining_bits_to_check -= digits;
          it++;
          advance(first, digits);
        }
      }
      if (remaining_bits_to_check > 0) {
        const word_type mask = _mask<word_type>(remaining_bits_to_check);
        const word_type comp = get_word<word_type>(first, remaining_bits_to_check);
        if ((mask & *it) != (mask & comp)) {
          return false;
        }
      }
    }
    return true;
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
constexpr bool equal(
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
    return equal_impl::run<SrcAlign, DstAlign>(
        mutable_first, mutable_last, mutable_d_first);
  } else {
    // Different word types: use adapter for width adaptation
    return with_bit_iterator_adapter<equal_impl>(
        mutable_first, mutable_last, mutable_d_first);
  }
}
// -------------------------------------------------------------------------- //

// ========================================================================== //
}  // namespace bit
#endif  // _EQUAL_HPP_INCLUDED
// ========================================================================== //
