// ================================== ADDITION ============================== //
// Project: The Experimental Bit Algorithms Library
// Name: addition.hpp
// Description: bit range addition
// Contributor(s):
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _ADDITION_HPP_INCLUDED
#define _ADDITION_HPP_INCLUDED
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>
// Project sources
#include "bitlib/bit-algorithms/transform_accumulate.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_iterator.hpp"

namespace bit {

// Adds integral_operand to the bit integer [first, last).  The result is
// written to d_first, which may overlap the input.  Returns a non-zero value
// when the addition overflows the range; the exact carry value is not
// represented because the return type is only an overflow flag.
template <typename It, typename U>
  requires(
      (is_static_castable_v<U, typename bit_iterator<It>::word_type>) &&
      (bitsof<U>() <= bitsof<typename bit_iterator<It>::word_type>()))
constexpr unsigned char addition(
    const bit_iterator<It>& first,
    const bit_iterator<It>& last,
    const bit_iterator<It>& d_first,
    const U& integral_operand) {
  using word_type = typename bit_iterator<It>::word_type;

  const auto total_bits = ::bit::distance(first, last);
  if (total_bits == 0) {
    return 0;
  }

  // The operand is the initial carry: it is added once to the least
  // significant chunk, and every later chunk consumes the carry produced by
  // the preceding chunk.
  auto add_op = [](word_type carry, word_type input_word,
                   std::size_t bits = bitsof<word_type>())
      -> std::pair<word_type, word_type> {
    using unsigned_word = std::make_unsigned_t<word_type>;
    using carry_type =
        std::conditional_t<(sizeof(unsigned_word) > 1), unsigned_word,
                           unsigned int>;
    constexpr std::size_t digits = bitsof<word_type>();
    assert(bits > 0 && bits <= digits);

    const unsigned_word mask =
        ::bit::_mask<unsigned_word, ::bit::_mask_len::unknown>(bits);
    unsigned_word low = 0;
    const carry_type high = ::bit::add_carry(
        static_cast<carry_type>(carry),
        static_cast<unsigned_word>(static_cast<unsigned_word>(input_word) &
                                   mask),
        static_cast<unsigned_word>(0), &low);
    const carry_type next_carry =
        bits == digits
            ? high
            : static_cast<carry_type>(
                  (low >> bits) |
                  (static_cast<unsigned_word>(high) << (digits - bits)));
    return {static_cast<word_type>(low),
            static_cast<word_type>(next_carry)};
  };

  const word_type carry = ::bit::transform_accumulate<true, true>(
      first, last, d_first, d_first + total_bits,
      static_cast<word_type>(integral_operand), add_op, add_op);
  return static_cast<unsigned char>(carry != 0);
}

template <typename It, typename U>
  requires(
      (is_static_castable_v<U, typename bit_iterator<It>::word_type>) &&
      (bitsof<U>() <= bitsof<typename bit_iterator<It>::word_type>()))
constexpr unsigned char addition(
    const bit_iterator<It>& first,
    const bit_iterator<It>& last,
    const U& integral_operand) {
  return addition(first, last, first, integral_operand);
}

}  // namespace bit

#endif  // _ADDITION_HPP_INCLUDED
