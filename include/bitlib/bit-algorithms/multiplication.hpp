// ================================== MOVE ================================== //
// Project: The Experimental Bit Algorithms Library
// Name: multiplication.hpp
// Description: bit range multiplication
// Contributor(s):
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _MULTIPLICATION_HPP_INCLUDED
#define _MULTIPLICATION_HPP_INCLUDED
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

template <typename It, typename U>
  requires(
      (is_static_castable_v<U, typename bit_iterator<It>::word_type>) &&
      (bitsof<exact_floor_integral_t<U>>() < bitsof<typename bit_iterator<It>::word_type>()))
constexpr typename bit_iterator<It>::word_type multiplication(
    const bit_iterator<It>& first,
    const bit_iterator<It>& last,
    const bit_iterator<It>& d_first,
    const U& integral_operand) {
  using word_type = typename bit_iterator<It>::word_type;
  word_type carry = 0;
  transform(first, last, d_first,
            [&carry, integral_operand](auto word) -> word_type {
              return (carry + _mulx(static_cast<word_type>(integral_operand), word, &carry));
            });
  return carry;
}

template <typename It, typename U>
  requires(
      (is_static_castable_v<U, typename bit_iterator<It>::word_type>) &&
      (bitsof<exact_floor_integral_t<U>>() < bitsof<typename bit_iterator<It>::word_type>()))
constexpr typename bit_iterator<It>::word_type multiplication(
    const bit_iterator<It>& first,
    const bit_iterator<It>& last,
    const U& integral_operand) {
  return multiplication(first, last, first, integral_operand);
}

} // namespace bit

#endif  // _MULTIPLICATION_HPP_INCLUDED
