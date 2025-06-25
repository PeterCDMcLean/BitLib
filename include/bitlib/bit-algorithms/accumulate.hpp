// ================================= array_REF =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        accumulate.hpp
// Description: Implementation of accumulate
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //

#ifndef _BIT_ACCUMULATE_HPP_INCLUDED
#define _BIT_ACCUMULATE_HPP_INCLUDED

#include "bitlib/bit-iterator/bit_iterator.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"

namespace bit {

namespace policy {
struct AccumulateInitialSubword {
  static constexpr bool initial_sub_word = true;
};

struct AccumulateNoInitialSubword {
  static constexpr bool initial_sub_word = false;
};
}  // namespace policy

template <bool forward, bool initial_sub_word, typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto accumulate(
    bit_iterator<RandomAccessIt> first,
    bit_iterator<RandomAccessIt> last,
    T acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  using word_type = typename bit_iterator<RandomAccessIt>::word_type;
  using size_type = typename bit_iterator<RandomAccessIt>::size_type;
  constexpr size_type digits = bitsof<word_type>();


  size_type total_bits_to_op = distance(first, last);
  bool keep_going = true;;
  if constexpr (initial_sub_word) {
    size_type sub_digits;
    if constexpr (forward) {
      sub_digits = std::min(digits - first.position(), total_bits_to_op);
      if (sub_digits != 0) {
        acc = binary_op_subword(std::move(acc), get_masked_word<word_type>(first, sub_digits), sub_digits);
        advance(first, digits - first.position());
      }
    } else {
      sub_digits = std::min(last.position(), total_bits_to_op);
      if (sub_digits != 0) {
        advance(last, -sub_digits);
        acc = binary_op_subword(std::move(acc), get_masked_word<word_type>(last, sub_digits), sub_digits);
      }
    }
    total_bits_to_op -= sub_digits;
  }

  const size_type whole_words_to_op = total_bits_to_op / digits;
  const size_type remaining_bits_to_op = total_bits_to_op % digits;

  for (size_t i = 0; i < whole_words_to_op; i ++) {
    if constexpr (forward) {
      acc = binary_op(std::move(acc), get_word<word_type>(first));
      advance(first, digits);
    } else {
      advance(last, -digits);
      acc = binary_op(std::move(acc), get_word<word_type>(last));
    }
  }
  if (remaining_bits_to_op > 0) {
    if constexpr (forward) {
      acc = binary_op_subword(std::move(acc), get_masked_word<word_type>(first, remaining_bits_to_op), remaining_bits_to_op);
    } else {
      advance(last, -remaining_bits_to_op);
      acc = binary_op_subword(std::move(acc), get_masked_word<word_type>(last, remaining_bits_to_op), remaining_bits_to_op);
    }
  }

  return acc;
}

template <bool forward, bool initial_sub_word, typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto accumulate_while(
    bit_iterator<RandomAccessIt> first,
    bit_iterator<RandomAccessIt> last,
    T acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  using word_type = typename bit_iterator<RandomAccessIt>::word_type;
  using size_type = typename bit_iterator<RandomAccessIt>::size_type;
  constexpr size_type digits = bitsof<word_type>();

  size_type total_bits_to_op = distance(first, last);
  size_type whole_words_to_op = total_bits_to_op / digits;

  bool keep_going = true;;
  if constexpr (initial_sub_word) {
    if (whole_words_to_op > 1) {
      size_type sub_digits;
      if constexpr (forward) {
        sub_digits = std::min(digits - first.position(), total_bits_to_op);
        if (sub_digits != 0) {
          std::tie(keep_going, acc) = binary_op_subword(std::move(acc), get_masked_word<word_type>(first, sub_digits), sub_digits);
          advance(first, digits - first.position());
        }
      } else {
        sub_digits = std::min(last.position(), total_bits_to_op);
        if (sub_digits != 0) {
          advance(last, -sub_digits);
          std::tie(keep_going, acc) = binary_op_subword(std::move(acc), get_masked_word<word_type>(first, sub_digits), sub_digits);
        }
      }
      total_bits_to_op -= sub_digits;
      if (!keep_going) {
        return acc; // Stop early if the operation indicates to stop
      }
    }
  }

  whole_words_to_op = total_bits_to_op / digits;
  const size_type remaining_bits_to_op = total_bits_to_op % digits;

  for (size_type i = 0; i < whole_words_to_op; i ++) {
    if constexpr (forward) {
      std::tie(keep_going, acc) = binary_op(std::move(acc), get_word<word_type>(first));
      advance(first, digits);
    } else {
      advance(last, -digits);
      std::tie(keep_going, acc) = binary_op(std::move(acc), get_word<word_type>(last));
    }
    if (!keep_going) {
      return acc; // Stop early if the operation indicates to stop
    }
  }
  if (remaining_bits_to_op > 0) {
    if constexpr (forward) {
      std::tie(std::ignore, acc) = binary_op_subword(std::move(acc), get_masked_word<word_type>(first, remaining_bits_to_op), remaining_bits_to_op);
    } else {
      advance(last, -remaining_bits_to_op);
      std::tie(std::ignore, acc) = binary_op_subword(std::move(acc), get_masked_word<word_type>(last, remaining_bits_to_op), remaining_bits_to_op);
    }
  }

  return acc;
}

// Requires BinaryOperation to have a third but defaulted argument
template <typename RandomAccessIt, typename T, typename BinaryOperation>
constexpr auto accumulate(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op) {
  return accumulate<true, true>(first, last, acc, binary_op, binary_op);
}

template <typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto accumulate(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  return accumulate<true, true>(first, last, acc, binary_op, binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument
template <typename RandomAccessIt, typename T, typename BinaryOperation>
constexpr auto accumulate_while(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op) {
  return accumulate_while<true, true>(first, last, acc, binary_op, binary_op);
}

template <typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto accumulate_while(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  return accumulate_while<true, true>(first, last, acc, binary_op, binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument
template <typename RandomAccessIt, typename T, typename BinaryOperation>
constexpr auto accumulate_backward_while(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op) {
  return accumulate_while<false, true>(first, last, acc, binary_op, binary_op);
}

template <typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto accumulate_backward_while(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  return accumulate_while<false, true>(first, last, acc, binary_op, binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument
template <typename Policy, typename RandomAccessIt, typename T, typename BinaryOperation>
constexpr auto accumulate(
    Policy,
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op) {
  return accumulate<true, Policy::initial_sub_word>(first, last, acc, binary_op, binary_op);
}

template <typename Policy, typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto accumulate(
    Policy,
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  return accumulate<true, Policy::initial_sub_word>(first, last, acc, binary_op, binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument
template <typename Policy, typename RandomAccessIt, typename T, typename BinaryOperation>
constexpr auto accumulate_while(
    Policy,
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op) {
  return accumulate_while<true, Policy::initial_sub_word>(first, last, acc, binary_op, binary_op);
}

template <typename Policy, typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto accumulate_while(
    Policy,
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  return accumulate_while<true, Policy::initial_sub_word>(first, last, acc, binary_op, binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument
template <typename Policy, typename RandomAccessIt, typename T, typename BinaryOperation>
constexpr auto accumulate_backward_while(
    Policy,
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op) {
  return accumulate_while<false, Policy::initial_sub_word>(first, last, acc, binary_op, binary_op);
}

template <typename Policy, typename RandomAccessIt, typename T, typename BinaryOperation, typename BinaryOperationSubword>
constexpr auto accumulate_backward_while(
    Policy,
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const T& acc,
    BinaryOperation binary_op,
    BinaryOperationSubword binary_op_subword) {
  return accumulate_while<false, Policy::initial_sub_word>(first, last, acc, binary_op, binary_op_subword);
}

} // namespace bit

#endif  // _BIT_ACCUMULATE_HPP_INCLUDED
