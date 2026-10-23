// ========================= TRANSFORM ACCUMULATE =========================== //
// Project:     The Experimental Bit Algorithms Library
// \file        transform_accumulate.hpp
// Description: Implementation of transform_accumulate
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //

#ifndef _BIT_TRANSFORM_ACCUMULATE_HPP_INCLUDED
#define _BIT_TRANSFORM_ACCUMULATE_HPP_INCLUDED

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>
// Project sources
#include "bitlib/bit-algorithms/bit_algorithm_details.hpp"
#include "bitlib/bit-algorithms/word_adapter.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_iterator.hpp"

namespace bit {
namespace detail {

template <bool forward, bool initial_sub_word, typename First, typename Last,
          typename DFirst, typename DLast, typename T,
          typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto transform_accumulate_impl(
    const First& first_in, const Last& last_in, const DFirst& d_first_in,
    const DLast& d_last_in, T acc, BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  auto first = make_mutable_bit_iterator(first_in);
  auto last = make_mutable_bit_iterator(last_in);
  auto d_first = make_mutable_bit_iterator(d_first_in);
  auto d_last = make_mutable_bit_iterator(d_last_in);

  using mutable_iterator = decltype(first);
  using word_type = typename mutable_iterator::word_type;
  using size_type = typename mutable_iterator::size_type;
  constexpr size_type digits = bitsof<word_type>();
  constexpr std::size_t DFirstAlign = iterator_alignment_v<DFirst>;
  constexpr std::size_t DLastAlign = iterator_alignment_v<DLast>;

  _assert_range_viability(first, last);
  size_type total_bits_to_op = distance(first, last);
  if (total_bits_to_op == 0) {
    return acc;
  }

  if constexpr (forward) {
    size_type d_first_position;
    if constexpr (DFirstAlign != std::dynamic_extent) {
      d_first_position = DFirstAlign;
    } else {
      d_first_position = d_first.position();
    }

    auto d_it = d_first.base();
    if (d_first_position != 0) {
      const size_type partial_bits_to_op = std::min(
          total_bits_to_op, digits - d_first_position);
      if (partial_bits_to_op != 0) {
        word_type word;
        std::tie(word, acc) = binary_op_subword(
            std::move(acc),
            get_masked_word<word_type>(first, partial_bits_to_op),
            partial_bits_to_op);
        *d_it = _bitblend(
            *d_it,
            static_cast<word_type>(word << d_first_position),
            d_first_position, partial_bits_to_op);
        total_bits_to_op -= partial_bits_to_op;
        advance(first, partial_bits_to_op);
        ++d_it;
      }
    }

    const size_type whole_words_to_op = total_bits_to_op / digits;
    const size_type remaining_bits_to_op = total_bits_to_op % digits;

    if (first.position() == 0) {
      auto in = first.base();
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        word_type word;
        std::tie(word, acc) = binary_op(std::move(acc), *in++);
        *d_it++ = word;
      }
      first = mutable_iterator(in);
    } else {
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        word_type word;
        std::tie(word, acc) =
            binary_op(std::move(acc), get_word<word_type>(first));
        *d_it++ = word;
        advance(first, digits);
      }
    }

    if (remaining_bits_to_op > 0) {
      word_type word;
      std::tie(word, acc) = binary_op_subword(
          std::move(acc),
          get_masked_word<word_type>(first, remaining_bits_to_op),
          remaining_bits_to_op);
      *d_it = _bitblend(*d_it, word, 0, remaining_bits_to_op);
    }
  } else {
    size_type d_first_position;
    if constexpr (DFirstAlign != std::dynamic_extent) {
      d_first_position = DFirstAlign;
    } else {
      d_first_position = d_first.position();
    }

    size_type d_last_position;
    if constexpr (DLastAlign != std::dynamic_extent) {
      d_last_position = DLastAlign;
    } else {
      d_last_position = d_last.position();
    }

    auto d_it = d_last.base();

    // Process the final partial output word. If the whole range lives inside
    // one word, the partial starts at d_first_position; otherwise it is the
    // low d_last_position bits of the d_last word.
    size_type tail_bits = 0;
    size_type tail_offset = 0;
    if (d_last_position != 0) {
      if (total_bits_to_op < d_last_position) {
        tail_bits = total_bits_to_op;
        tail_offset = d_first_position;
      } else {
        tail_bits = d_last_position;
        tail_offset = 0;
      }
    }
    if (tail_bits != 0) {
      reverse(last, tail_bits);
      word_type word;
      std::tie(word, acc) = binary_op_subword(
          std::move(acc), get_masked_word<word_type>(last, tail_bits),
          tail_bits);
      *d_it = _bitblend(
          *d_it, static_cast<word_type>(word << tail_offset), tail_offset,
          tail_bits);
      total_bits_to_op -= tail_bits;
    }

    const size_type whole_words_to_op = total_bits_to_op / digits;
    const size_type remaining_bits_to_op = total_bits_to_op % digits;

    // The output tail above may have been longer or shorter than the input
    // tail, so the input cursor is not necessarily word-aligned even though
    // the output is. Consume complete output words and read the corresponding
    // input word with get_word() unless the input happens to be aligned.
    const bool input_aligned = last.position() == 0;
    for (size_type i = 0; i < whole_words_to_op; ++i) {
      reverse(last, digits);
      --d_it;
      word_type word;
      if (input_aligned) {
        std::tie(word, acc) = binary_op(std::move(acc), *last.base());
      } else {
        std::tie(word, acc) =
            binary_op(std::move(acc), get_word<word_type>(last));
      }
      *d_it = word;
    }

    if (remaining_bits_to_op > 0) {
      reverse(last, remaining_bits_to_op);
      --d_it;
      word_type word;
      std::tie(word, acc) = binary_op_subword(
          std::move(acc),
          get_masked_word<word_type>(last, remaining_bits_to_op),
          remaining_bits_to_op);
      *d_it = _bitblend(
          *d_it,
          static_cast<word_type>(word << d_first_position),
          d_first_position, remaining_bits_to_op);
    }
  }

  return acc;
}

}  // namespace detail

// Alignment- and word-size-aware transform_accumulate front-end. The input
// and output ranges are re-expressed in the smallest word width used by any
// of the four iterators, so mixed-width ranges work without changing the
// accumulator's callback contract.
template <bool forward, bool initial_sub_word, typename First, typename Last,
          typename DFirst, typename DLast, typename T,
          typename BinaryOperation, typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last> &&
           bit_iterator_like_c<DFirst> && bit_iterator_like_c<DLast>)
constexpr auto transform_accumulate(
    const First& first, const Last& last, const DFirst& d_first,
    const DLast& d_last, T acc, BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  static_assert(
      std::is_same_v<underlying_iterator_t<First>,
                     underlying_iterator_t<Last>>,
      "first and last must have the same underlying iterator type");
  static_assert(
      std::is_same_v<underlying_iterator_t<DFirst>,
                     underlying_iterator_t<DLast>>,
      "d_first and d_last must have the same underlying iterator type");

  auto first_probe = make_mutable_bit_iterator(first);
  auto last_probe = make_mutable_bit_iterator(last);
  auto d_first_probe = make_mutable_bit_iterator(d_first);
  auto d_last_probe = make_mutable_bit_iterator(d_last);
  constexpr std::size_t target_bits =
      detail::smallest_word_bits<decltype(first_probe), decltype(last_probe),
                                 decltype(d_first_probe),
                                 decltype(d_last_probe)>();
  using target_word = ceil_integral<target_bits>;

  auto adapted_first =
      detail::adapt_word_iterator<target_bits, target_word>(first);
  auto adapted_last =
      detail::adapt_word_iterator<target_bits, target_word>(last);
  auto adapted_d_first =
      detail::adapt_word_iterator<target_bits, target_word>(d_first);
  auto adapted_d_last =
      detail::adapt_word_iterator<target_bits, target_word>(d_last);

  return detail::transform_accumulate_impl<forward, initial_sub_word>(
      adapted_first, adapted_last, adapted_d_first, adapted_d_last, acc,
      binary_op, binary_op_subword);
}

template <typename First, typename Last, typename DFirst, typename DLast,
          typename T, typename BinaryOperation,
          typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last> &&
           bit_iterator_like_c<DFirst> && bit_iterator_like_c<DLast>)
constexpr auto transform_accumulate_backward(
    const First& first, const Last& last, const DFirst& d_first,
    const DLast& d_last, const T& acc, BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  return transform_accumulate<false, true>(
      first, last, d_first, d_last, acc, binary_op, binary_op_subword);
}

}  // namespace bit

#endif  // _BIT_TRANSFORM_ACCUMULATE_HPP_INCLUDED
