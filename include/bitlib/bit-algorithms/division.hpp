// ================================== MOVE ================================== //
// Project: The Experimental Bit Algorithms Library
// Name: division.hpp
// Description: bit range division
// Contributor(s):
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _DIVISION_HPP_INCLUDED
#define _DIVISION_HPP_INCLUDED
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <type_traits>
#include <math.h>
// Project sources
#include "bitlib/bit-algorithms/transform.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-iterator/bit_iterator.hpp"

namespace bit {

// 'Schoolbook' division by scalar
template <typename RandomAccessItIn, typename RandomAccessItOut, typename U>
  requires(
      (std::is_same_v<std::remove_cvref_t<std::iter_value_t<RandomAccessItIn>>,
                      std::remove_cvref_t<std::iter_value_t<RandomAccessItOut>>>) &&
      (is_static_castable_v<U, typename bit_iterator<RandomAccessItOut>::word_type>) &&
      (bitsof<U>() <= bitsof<typename bit_iterator<RandomAccessItIn>::word_type>()))
constexpr typename bit_iterator<RandomAccessItOut>::word_type division(
    const bit_iterator<RandomAccessItIn>& first,
    const bit_iterator<RandomAccessItIn>& last,
    const bit_iterator<RandomAccessItOut>& d_first,
    const U& integral_operand) {
  using word_type = typename bit_iterator<RandomAccessItOut>::word_type;
  auto long_div = [integral_operand](
      const word_type& remainder,
      const word_type& word,
      size_t bits = bitsof<word_type>()) {
    word_type next_remainder;
    word_type result_word = _divx(remainder, word, static_cast<word_type>(integral_operand), &next_remainder);
    return std::make_pair(result_word, next_remainder);
  };
  word_type remainder = 0;
  return transform_accumulate_backward(first, last, d_first, d_first + distance(first, last), remainder, long_div, long_div);
}

template <typename It, typename U>
  requires(
      (is_static_castable_v<U, typename bit_iterator<It>::word_type>) &&
      (bitsof<U>() <= bitsof<typename bit_iterator<It>::word_type>()))
constexpr typename bit_iterator<It>::word_type division(
    const bit_iterator<It>& first,
    const bit_iterator<It>& last,
    const U& integral_operand) {
  return division(first, last, first, integral_operand);
}

} // namespace bit

#endif  // _DIVISION_HPP_INCLUDED
