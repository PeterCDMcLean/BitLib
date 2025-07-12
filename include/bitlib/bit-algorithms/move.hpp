// ================================== MOVE ================================== //
// Project: The Experimental Bit Algorithms Library
// Name: move.hpp
// Description: bit_iterator overloads for std::move
// Contributor(s):
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _MOVE_HPP_INCLUDED
#define _MOVE_HPP_INCLUDED
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <type_traits>
#include <math.h>
// Project sources
#include "bitlib/bit-algorithms/copy.hpp"

namespace bit {
// ========================================================================== //

template <typename It1, typename It2>
constexpr bit_iterator<It2> move(
    const bit_iterator<It1>& first,
    const bit_iterator<It1>& last,
    const It2& d_first) {
  return copy(first, last, d_first);
}

template <typename It1, typename It2>
constexpr bit_iterator<It2> move(
    const It1& first,
    const It1& last,
    const bit_iterator<It2>& d_first) {
  return copy(first, last, d_first);
}

template <typename It1, typename It2>
constexpr bit_iterator<It2> move(
    const bit_iterator<It1>& first,
    const bit_iterator<It1>& last,
    const bit_iterator<It2>& d_first) {
  return copy(first, last, d_first);
}

// ========================================================================== //
} // namespace bit

#endif // _MOVE_HPP_INCLUDED
// ========================================================================== //
