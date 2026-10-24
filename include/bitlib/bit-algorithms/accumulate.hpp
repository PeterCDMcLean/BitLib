// ================================= ACCUMULATE ================================= //
// Project:     The Experimental Bit Algorithms Library
// \file        accumulate.hpp
// Description: Implementation of accumulate
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //

#ifndef _BIT_ACCUMULATE_HPP_INCLUDED
#define _BIT_ACCUMULATE_HPP_INCLUDED

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <algorithm>
#include <tuple>
#include <utility>
// Project sources
#include "bitlib/bit-algorithms/bit_algorithm_details.hpp"
#include "bitlib/bit-algorithms/word_adapter.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_iterator.hpp"

namespace bit {

namespace policy {
struct AccumulateInitialSubword {
  static constexpr bool initial_sub_word = true;
};

struct AccumulateNoInitialSubword {
  static constexpr bool initial_sub_word = false;
};
}  // namespace policy

namespace detail {

template <bool forward, bool initial_sub_word, typename First, typename Last,
          typename T, typename BinaryOperation,
          typename BinaryOperationSubword>
constexpr auto accumulate_impl(const First& first_in, const Last& last_in,
                               T acc, BinaryOperation binary_op,
                               BinaryOperationSubword binary_op_subword) {
  auto first = make_mutable_bit_iterator(first_in);
  auto last = make_mutable_bit_iterator(last_in);
  using mutable_iterator = decltype(first);
  using word_type = typename mutable_iterator::word_type;
  using size_type = typename mutable_iterator::size_type;
  constexpr size_type digits = bitsof<word_type>();
  constexpr std::size_t FirstAlign = iterator_alignment_v<First>;
  constexpr std::size_t LastAlign = iterator_alignment_v<Last>;

  _assert_range_viability(first, last);
  size_type total_bits_to_op = distance(first, last);
  if (total_bits_to_op == 0) {
    return acc;
  }

  if constexpr (forward) {
    if constexpr (initial_sub_word) {
      size_type position;
      if constexpr (FirstAlign != std::dynamic_extent) {
        position = FirstAlign;
      } else {
        position = first.position();
      }
      const size_type sub_digits =
          std::min(total_bits_to_op, digits - position);
      if (sub_digits != 0) {
        acc = binary_op_subword(std::move(acc),
                                get_masked_word<word_type>(first, sub_digits),
                                sub_digits);
        advance(first, sub_digits);
        total_bits_to_op -= sub_digits;
      }
    }

    const size_type whole_words_to_op = total_bits_to_op / digits;
    const size_type remaining_bits_to_op = total_bits_to_op % digits;

    if (first.position() == 0) {
      // The prefix (if any) aligned the source. Read complete words directly
      // instead of spelling out get_word()/advance() for every iteration.
      auto it = first.base();
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        acc = binary_op(std::move(acc), *it++);
      }
      first = mutable_iterator(it);
    } else {
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        acc = binary_op(std::move(acc), get_word<word_type>(first));
        advance(first, digits);
      }
    }

    if (remaining_bits_to_op > 0) {
      acc = binary_op_subword(
          std::move(acc),
          get_masked_word<word_type>(first, remaining_bits_to_op),
          remaining_bits_to_op);
    }
  } else {
    if constexpr (initial_sub_word) {
      size_type position;
      if constexpr (LastAlign != std::dynamic_extent) {
        position = LastAlign;
      } else {
        position = last.position();
      }
      const size_type sub_digits = std::min(total_bits_to_op, position);
      if (sub_digits != 0) {
        reverse(last, sub_digits);
        acc = binary_op_subword(std::move(acc),
                                get_masked_word<word_type>(last, sub_digits),
                                sub_digits);
        total_bits_to_op -= sub_digits;
      }
    }

    const size_type whole_words_to_op = total_bits_to_op / digits;
    const size_type remaining_bits_to_op = total_bits_to_op % digits;

    if (last.position() == 0) {
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        reverse(last, digits);
        acc = binary_op(std::move(acc), *last.base());
      }
    } else {
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        reverse(last, digits);
        acc = binary_op(std::move(acc), get_word<word_type>(last));
      }
    }

    if (remaining_bits_to_op > 0) {
      reverse(last, remaining_bits_to_op);
      acc = binary_op_subword(
          std::move(acc),
          get_masked_word<word_type>(last, remaining_bits_to_op),
          remaining_bits_to_op);
    }
  }

  return acc;
}

template <bool forward, bool initial_sub_word, typename First, typename Last,
          typename T, typename BinaryOperation,
          typename BinaryOperationSubword>
constexpr auto accumulate_while_impl(const First& first_in,
                                     const Last& last_in, T acc,
                                     BinaryOperation binary_op,
                                     BinaryOperationSubword binary_op_subword) {
  auto first = make_mutable_bit_iterator(first_in);
  auto last = make_mutable_bit_iterator(last_in);
  using mutable_iterator = decltype(first);
  using word_type = typename mutable_iterator::word_type;
  using size_type = typename mutable_iterator::size_type;
  constexpr size_type digits = bitsof<word_type>();
  constexpr std::size_t FirstAlign = iterator_alignment_v<First>;
  constexpr std::size_t LastAlign = iterator_alignment_v<Last>;

  _assert_range_viability(first, last);
  size_type total_bits_to_op = distance(first, last);
  if (total_bits_to_op == 0) {
    return acc;
  }

  bool keep_going = true;

  if constexpr (forward) {
    if constexpr (initial_sub_word) {
      // Preserve the original guard: only peel a leading subword when there
      // is more than one complete word to process.
      if (total_bits_to_op / digits > 1) {
        size_type position;
        if constexpr (FirstAlign != std::dynamic_extent) {
          position = FirstAlign;
        } else {
          position = first.position();
        }
        const size_type sub_digits =
            std::min(total_bits_to_op, digits - position);
        if (sub_digits != 0) {
          std::tie(keep_going, acc) = binary_op_subword(
              std::move(acc),
              get_masked_word<word_type>(first, sub_digits), sub_digits);
          advance(first, sub_digits);
          total_bits_to_op -= sub_digits;
        }
        if (!keep_going) {
          return acc;
        }
      }
    }

    const size_type whole_words_to_op = total_bits_to_op / digits;
    const size_type remaining_bits_to_op = total_bits_to_op % digits;

    if (first.position() == 0) {
      auto it = first.base();
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        std::tie(keep_going, acc) = binary_op(std::move(acc), *it++);
        if (!keep_going) {
          return acc;
        }
      }
      first = mutable_iterator(it);
    } else {
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        std::tie(keep_going, acc) =
            binary_op(std::move(acc), get_word<word_type>(first));
        if (!keep_going) {
          return acc;
        }
        advance(first, digits);
      }
    }

    if (remaining_bits_to_op > 0) {
      std::tie(std::ignore, acc) = binary_op_subword(
          std::move(acc),
          get_masked_word<word_type>(first, remaining_bits_to_op),
          remaining_bits_to_op);
    }
  } else {
    if constexpr (initial_sub_word) {
      if (total_bits_to_op / digits > 1) {
        size_type position;
        if constexpr (LastAlign != std::dynamic_extent) {
          position = LastAlign;
        } else {
          position = last.position();
        }
        const size_type sub_digits = std::min(total_bits_to_op, position);
        if (sub_digits != 0) {
          reverse(last, sub_digits);
          std::tie(keep_going, acc) = binary_op_subword(
              std::move(acc),
              get_masked_word<word_type>(last, sub_digits), sub_digits);
          total_bits_to_op -= sub_digits;
        }
        if (!keep_going) {
          return acc;
        }
      }
    }

    const size_type whole_words_to_op = total_bits_to_op / digits;
    const size_type remaining_bits_to_op = total_bits_to_op % digits;

    if (last.position() == 0) {
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        reverse(last, digits);
        std::tie(keep_going, acc) =
            binary_op(std::move(acc), *last.base());
        if (!keep_going) {
          return acc;
        }
      }
    } else {
      for (size_type i = 0; i < whole_words_to_op; ++i) {
        reverse(last, digits);
        std::tie(keep_going, acc) =
            binary_op(std::move(acc), get_word<word_type>(last));
        if (!keep_going) {
          return acc;
        }
      }
    }

    if (remaining_bits_to_op > 0) {
      reverse(last, remaining_bits_to_op);
      std::tie(std::ignore, acc) = binary_op_subword(
          std::move(acc),
          get_masked_word<word_type>(last, remaining_bits_to_op),
          remaining_bits_to_op);
    }
  }

  return acc;
}

}  // namespace detail

// Forward/backward entry points used internally (and by callers that spell
// out the policy bools explicitly).
template <bool forward, bool initial_sub_word, typename First, typename Last,
          typename T, typename BinaryOperation,
          typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate(const First& first, const Last& last, T acc,
                          BinaryOperation binary_op,
                          BinaryOperationSubword binary_op_subword) {
  return detail::accumulate_impl<forward, initial_sub_word>(
      first, last, acc, binary_op, binary_op_subword);
}

template <bool forward, bool initial_sub_word, typename First, typename Last,
          typename T, typename BinaryOperation,
          typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_while(const First& first, const Last& last, T acc,
                                BinaryOperation binary_op,
                                BinaryOperationSubword binary_op_subword) {
  return detail::accumulate_while_impl<forward, initial_sub_word>(
      first, last, acc, binary_op, binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument.
template <typename First, typename Last, typename T, typename BinaryOperation>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate(const First& first, const Last& last, const T& acc,
                          BinaryOperation binary_op) {
  return accumulate<true, true>(first, last, acc, binary_op, binary_op);
}

template <typename First, typename Last, typename T, typename BinaryOperation,
          typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate(const First& first, const Last& last, const T& acc,
                          BinaryOperation binary_op,
                          BinaryOperationSubword binary_op_subword) {
  return accumulate<true, true>(first, last, acc, binary_op,
                                binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument.
template <typename First, typename Last, typename T, typename BinaryOperation>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_while(const First& first, const Last& last,
                                const T& acc, BinaryOperation binary_op) {
  return accumulate_while<true, true>(first, last, acc, binary_op, binary_op);
}

template <typename First, typename Last, typename T, typename BinaryOperation,
          typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_while(const First& first, const Last& last,
                                const T& acc, BinaryOperation binary_op,
                                BinaryOperationSubword binary_op_subword) {
  return accumulate_while<true, true>(first, last, acc, binary_op,
                                      binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument.
template <typename First, typename Last, typename T, typename BinaryOperation>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_backward_while(const First& first, const Last& last,
                                         const T& acc,
                                         BinaryOperation binary_op) {
  return accumulate_while<false, true>(first, last, acc, binary_op,
                                       binary_op);
}

template <typename First, typename Last, typename T, typename BinaryOperation,
          typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_backward_while(
    const First& first, const Last& last, const T& acc,
    BinaryOperation binary_op, BinaryOperationSubword binary_op_subword) {
  return accumulate_while<false, true>(first, last, acc, binary_op,
                                       binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument.
template <typename Policy, typename First, typename Last, typename T,
          typename BinaryOperation>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate(Policy, const First& first, const Last& last,
                          const T& acc, BinaryOperation binary_op) {
  return accumulate<true, Policy::initial_sub_word>(first, last, acc,
                                                    binary_op, binary_op);
}

template <typename Policy, typename First, typename Last, typename T,
          typename BinaryOperation, typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate(Policy, const First& first, const Last& last,
                          const T& acc, BinaryOperation binary_op,
                          BinaryOperationSubword binary_op_subword) {
  return accumulate<true, Policy::initial_sub_word>(
      first, last, acc, binary_op, binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument.
template <typename Policy, typename First, typename Last, typename T,
          typename BinaryOperation>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_while(Policy, const First& first, const Last& last,
                                const T& acc, BinaryOperation binary_op) {
  return accumulate_while<true, Policy::initial_sub_word>(
      first, last, acc, binary_op, binary_op);
}

template <typename Policy, typename First, typename Last, typename T,
          typename BinaryOperation, typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_while(Policy, const First& first, const Last& last,
                                const T& acc, BinaryOperation binary_op,
                                BinaryOperationSubword binary_op_subword) {
  return accumulate_while<true, Policy::initial_sub_word>(
      first, last, acc, binary_op, binary_op_subword);
}

// Requires BinaryOperation to have a third but defaulted argument.
template <typename Policy, typename First, typename Last, typename T,
          typename BinaryOperation>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_backward_while(Policy, const First& first,
                                         const Last& last, const T& acc,
                                         BinaryOperation binary_op) {
  return accumulate_while<false, Policy::initial_sub_word>(
      first, last, acc, binary_op, binary_op);
}

template <typename Policy, typename First, typename Last, typename T,
          typename BinaryOperation, typename BinaryOperationSubword>
  requires(bit_iterator_like_c<First> && bit_iterator_like_c<Last>)
constexpr auto accumulate_backward_while(
    Policy, const First& first, const Last& last, const T& acc,
    BinaryOperation binary_op, BinaryOperationSubword binary_op_subword) {
  return accumulate_while<false, Policy::initial_sub_word>(
      first, last, acc, binary_op, binary_op_subword);
}

}  // namespace bit

#endif  // _BIT_ACCUMULATE_HPP_INCLUDED
