// ============================= COPY BACKWARD ============================= //
// Project: The Experimental Bit Algorithms Library
// Name: copy_backward.hpp
// Description: bit_iterator overloads for std::copy_backward
// Contributor(s):
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _COPY_BACKWARD_HPP_INCLUDED
#define _COPY_BACKWARD_HPP_INCLUDED
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

struct copy_backward_impl;

template <class It1, class It2>
constexpr bit_iterator<It2> copy_backward(
    const bit_iterator<It1>& first, const bit_iterator<It1>& last,
    const bit_iterator<It2>& d_last) {
  return with_bit_iterator_adapter<copy_backward_impl>(first, last, d_last);
}

// Status: Does not work for Input/Output iterators due to distance call
struct copy_backward_impl {
  // Backward-compatible entry point used by with_bit_iterator_adapter
  template <class RandomAccessIt1, class RandomAccessIt2>
  constexpr bit_iterator<RandomAccessIt2> operator()(
      bit_iterator<RandomAccessIt1> first,
      bit_iterator<RandomAccessIt1> last,
      bit_iterator<RandomAccessIt2> d_last) {
    return run<std::dynamic_extent, std::dynamic_extent>(first, last, d_last);
  }

  // Entry point with compile-time alignment hints
  template <std::size_t SrcAlign, std::size_t DstAlign,
            class RandomAccessIt1, class RandomAccessIt2>
  static constexpr iterator<RandomAccessIt2, primitive<1>, std::dynamic_extent> run(
      iterator<RandomAccessIt1, primitive<1>, std::dynamic_extent> first,
      iterator<RandomAccessIt1, primitive<1>, std::dynamic_extent> last,
      iterator<RandomAccessIt2, primitive<1>, std::dynamic_extent> d_last) {
    // Types and constants
    using dst_word_type = typename bit_iterator<RandomAccessIt2>::word_type;
    using src_word_type = typename bit_iterator<RandomAccessIt1>::word_type;
    using word_type = dst_word_type;
    using size_type = typename bit_iterator<RandomAccessIt2>::size_type;
    constexpr size_type digits = binary_digits<word_type>::value;

    // Assertions
    _assert_range_viability(first, last);
    static_assert(::std::is_same<dst_word_type, src_word_type>::value, "Underlying word types must be equal");
    if (first == last) {
      return d_last;
    }

    // Initialization
    const bool is_d_last_aligned = (DstAlign == std::dynamic_extent)
                                       ? (d_last.position() == 0)
                                       : false;
    size_type total_bits_to_copy = distance(first, last);
    size_type remaining_bits_to_copy = total_bits_to_copy;
    auto it = d_last.base();

    if constexpr (DstAlign != 0) {
      // d_last is not known to be aligned at bit 0
      if (!is_d_last_aligned) {
        size_type partial_bits_to_copy = ::std::min(
            remaining_bits_to_copy,
            d_last.position());
        *it = _bitblend<word_type>(
            *it,
            static_cast<word_type>(
                get_word<word_type>(last - partial_bits_to_copy, partial_bits_to_copy)
                << (d_last.position() - partial_bits_to_copy)),
            d_last.position() - partial_bits_to_copy,
            partial_bits_to_copy);
        remaining_bits_to_copy -= partial_bits_to_copy;
        reverse(last, partial_bits_to_copy);
      }
    }

    if (remaining_bits_to_copy > 0) {
      const bool is_last_aligned = (SrcAlign == std::dynamic_extent)
                                       ? (last.position() == 0)
                                       : false;
      // d_last will be aligned at this point
      if constexpr (SrcAlign == 0) {
        // last is always aligned at bit 0
        if (remaining_bits_to_copy >= digits) {
          auto N = ::std::distance(first.base(), last.base()) - 1;
          it = ::std::copy_backward(first.base() + 1, last.base(), it);
          last -= digits * N;
          remaining_bits_to_copy -= digits * N;
        }
      } else if (is_last_aligned && remaining_bits_to_copy >= digits) {
        auto N = ::std::distance(first.base(), last.base()) - 1;
        it = ::std::copy_backward(first.base() + 1, last.base(), it);
        last -= digits * N;
        remaining_bits_to_copy -= digits * N;
      } else {
        // TODO benchmark if its faster to ::std::copy the entire range then shift
        while (remaining_bits_to_copy >= digits) {
          *(--it) = get_word<word_type>(last - digits, digits);
          remaining_bits_to_copy -= digits;
          reverse(last, digits);
        }
      }
      if (remaining_bits_to_copy > 0) {
        it--;
        *it = _bitblend<word_type>(
            *it,
            static_cast<word_type>(get_word<word_type>(last - remaining_bits_to_copy, remaining_bits_to_copy)
                                   << (digits - remaining_bits_to_copy)),
            digits - remaining_bits_to_copy,
            remaining_bits_to_copy);
        remaining_bits_to_copy = 0;
      }
    }
    return d_last - total_bits_to_copy;
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
constexpr iterator<typename DstIt::iterator_type, primitive<1>, std::dynamic_extent> copy_backward(
    const SrcIt& first,
    const SrcIt& last,
    const DstIt& d_last)
  requires(iterator_alignment_v<SrcIt> != std::dynamic_extent || iterator_alignment_v<DstIt> != std::dynamic_extent)
{
  constexpr std::size_t SrcAlign = iterator_alignment_v<SrcIt>;
  constexpr std::size_t DstAlign = iterator_alignment_v<DstIt>;
  // Convert immutable iterators to mutable dynamic_extent iterators
  using mutable_src_it = iterator<typename SrcIt::iterator_type, primitive<1>, std::dynamic_extent>;
  using mutable_dst_it = iterator<typename DstIt::iterator_type, primitive<1>, std::dynamic_extent>;
  auto mutable_first = mutable_src_it(first.base(), first.position());
  auto mutable_last = mutable_src_it(last.base(), last.position());
  auto mutable_d_last = mutable_dst_it(d_last.base(), d_last.position());
  if constexpr (std::is_same_v<typename SrcIt::word_type, typename DstIt::word_type>) {
    // Same word type: no width adaptation needed, use alignment hints
    return copy_backward_impl::run<SrcAlign, DstAlign>(
        mutable_first, mutable_last, mutable_d_last);
  } else {
    // Different word types: use adapter for width adaptation
    return with_bit_iterator_adapter<copy_backward_impl>(
        mutable_first, mutable_last, mutable_d_last);
  }
}
// ========================================================================== //
}  // namespace bit

#endif  // _COPY_BACKWARD_HPP_INCLUDED
// ========================================================================== //
