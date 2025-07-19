// ================================= array_REF =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        accumulate.hpp
// Description: Implementation of accumulate
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //

#ifndef _BIT_TRANSFORM_ACCUMULATE_HPP_INCLUDED
#define _BIT_TRANSFORM_ACCUMULATE_HPP_INCLUDED

#include "bitlib/bit-iterator/bit_iterator.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"

namespace bit {

template <bool forward, bool initial_sub_word, typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto transform_accumulate(
    bit_iterator<RandomAccessIt> first,
    bit_iterator<RandomAccessIt> last,
    bit_iterator<RandomAccessIt> d_first,
    bit_iterator<RandomAccessIt> d_last,
    T acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  using word_type = typename bit_iterator<RandomAccessIt>::word_type;
  using size_type = typename bit_iterator<RandomAccessIt>::size_type;
  constexpr size_type digits = bitsof<word_type>();

  size_type total_bits_to_op = distance(first, last);

  RandomAccessIt d_it;
  if constexpr (forward) {
    d_it = d_first.base();
  } else {
    d_it = d_last.base();
  }

  if constexpr (forward) {
    if (d_first.position() != 0) {
      size_type partial_bits_to_op = ::std::min(
          total_bits_to_op,
          digits - d_first.position());
      if (partial_bits_to_op != 0) {
        word_type word;
        std::tie(word, acc) = binary_op_subword(std::move(acc), get_masked_word<word_type>(first, partial_bits_to_op), partial_bits_to_op);
        *d_it = _bitblend(
            *d_it,
            word,
            static_cast<word_type>(d_first.position()),
            static_cast<word_type>(partial_bits_to_op));
        total_bits_to_op -= partial_bits_to_op;
        advance(first, partial_bits_to_op);
        advance(d_it, 1);
      }
    }
  } else {
    if (d_last.position() != 0) {
      size_type partial_bits_to_op = ::std::min(
          total_bits_to_op,
          d_last.position());
      if (partial_bits_to_op != 0) {
        advance(last, -partial_bits_to_op);
        word_type word;
        std::tie(word, acc) = binary_op_subword(std::move(acc), get_masked_word<word_type>(last, partial_bits_to_op), partial_bits_to_op);
        *d_it = _bitblend(
            *d_it,
            word,
            static_cast<word_type>(d_first.position()),
            static_cast<word_type>(partial_bits_to_op));
        total_bits_to_op -= partial_bits_to_op;
      }
    }
  }


  const size_type whole_words_to_op = total_bits_to_op / digits;
  const size_type remaining_bits_to_op = total_bits_to_op % digits;

  for (size_t i = 0; i < whole_words_to_op; i ++) {
    if constexpr (forward) {
      word_type word;
      std::tie(word, acc) = binary_op(std::move(acc), get_word<word_type>(first));
      *d_it = word;
      advance(first, digits);
      std::advance(d_it, 1);
    } else {
      advance(last, -digits);
      std::advance(d_it, -1);
      word_type word;
      std::tie(word, acc) = binary_op(std::move(acc), get_word<word_type>(last));
      *d_it = word;
    }
  }
  if (remaining_bits_to_op > 0) {
    if constexpr (forward) {
      word_type word;
      std::tie(word, acc) = binary_op_subword(std::move(acc), get_masked_word<word_type>(first, remaining_bits_to_op), remaining_bits_to_op);
      *d_it = _bitblend(
          *d_it,
          word,
          0,
          remaining_bits_to_op);

    } else {
      advance(last, -remaining_bits_to_op);
      std::advance(d_it, -1);
      word_type word;
      std::tie(word, acc) = binary_op_subword(std::move(acc), get_masked_word<word_type>(last, remaining_bits_to_op), remaining_bits_to_op);
      *d_it = _bitblend(
          *d_it,
          word,
          0,
          remaining_bits_to_op);
    }
  }

  return acc;
}

template <typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto transform_accumulate_backward(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const bit_iterator<RandomAccessIt>& d_first,
    const bit_iterator<RandomAccessIt>& d_last,
    const T& acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  return transform_accumulate<false, true>(first, last, d_first, d_last, acc, binary_op, binary_op_subword);
}

} // namespace bit

#endif // _BIT_TRANSFORM_ACCUMULATE_HPP_INCLUDED
