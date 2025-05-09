// ================================= BIT_ARRAY_BASE =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array_base.hpp
// Description: Base implementation for bit_array variants
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ARRAY_BASE_HPP_INCLUDED
#define _BIT_ARRAY_BASE_HPP_INCLUDED
// ========================================================================== //

// ================================ PREAMBLE ================================ //
// C++ standard library
#include <algorithm>
#include <bit>
#include <cmath>
#include <span>
#include <string>
#include <type_traits>
#include <vector>
#include <cstring> // memcpy

// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_span.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {
// ========================================================================== //

/**
 * @brief Common utility functions for bit array implementations
 */
namespace bit_array_utils {

/**
     * @brief Generate a string representation of a range of bits
     *
     * @tparam Iterator The iterator type
     * @param first Iterator to the first bit
     * @param last Iterator past the last bit
     * @param digits The number of bits in a word
     * @return A string representation of the bits
     */
template <typename Iterator>
constexpr std::string debug_string_impl(Iterator first, Iterator last, std::size_t digits) {
  std::string ret = "";
  auto position = 0;
  for (auto it = first; it != last; ++it) {
    if (position % digits == 0 && position != 0) {
      ret += " ";
    } else if (position % 8 == 0 && position != 0) {
      ret += '.';
    }
    ret += *it == bit1 ? '1' : '0';
    ++position;
  }
  return ret;
}

    /**
     * @brief Element access helper for at() method
     */
template <typename Container>
constexpr auto at_impl(Container& container, typename Container::size_type pos)
    -> decltype(container.begin()[pos]) {
  if (pos < container.size()) {
    return container.begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

    /**
     * @brief Equality comparison implementation
     */
    template <typename Container>
    constexpr bool equal_impl(const Container& lhs, const Container& rhs) {
        return equal(lhs.begin(), lhs.end(), rhs.begin());
    }

} // namespace bit_array_utils

/**
 * @brief Base class template for bit_array implementations
 *
 * This is a CRTP (Curiously Recurring Template Pattern) base class that provides
 * common functionality for bit_array variants.
 *
 * @tparam Derived The derived class (CRTP pattern)
 * @tparam T The value type (typically bit_value)
 * @tparam W The word type used for storage
 */
template <typename Derived, typename T, typename W, typename It, typename CIt>
class bit_array_base {
 public:
  using word_type = W;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = typename std::conditional<std::is_same_v<T, bit_value>, bit_reference<word_type>, T&>::type;
  using const_reference = typename std::conditional<std::is_same_v<T, bit_value>, const bit_reference<const word_type>, const T&>::type;
  using pointer = typename std::conditional<std::is_same_v<T, bit_value>, bit_pointer<word_type>, T&>::type;
  using const_pointer = const pointer;
  using iterator = It;
  using const_iterator = CIt;

  // Element access
  constexpr reference operator[](size_type pos) {
    return derived().begin()[pos];
  }

  constexpr const_reference operator[](size_type pos) const {
    return derived().begin()[pos];
  }

  constexpr reference at(size_type pos) {
    return bit_array_utils::at_impl(derived(), pos);
  }

  constexpr const_reference at(size_type pos) const {
    return bit_array_utils::at_impl(derived(), pos);
  }

  constexpr reference front() {
    return derived().begin()[0];
  }

  constexpr const_reference front() const {
    return derived().begin()[0];
  }

  constexpr reference back() {
    return derived().begin()[derived().size() - 1];
  }

  constexpr const_reference back() const {
    return derived().begin()[derived().size() - 1];
  }

  // Capacity
  constexpr bool empty() const noexcept {
    return 0 == derived().size();
  }

  constexpr size_type max_size() const noexcept {
    return derived().size();
  }

  // String representation
  constexpr std::string debug_string() const {
    return bit_array_utils::debug_string_impl(derived().begin(), derived().end(), bitsof<word_type>());
  }

  // Comparison
  constexpr bool operator==(const Derived& other) const noexcept {
    return bit_array_utils::equal_impl(derived(), other);
  }

 protected:
  constexpr Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  constexpr const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

} // namespace bit

#endif // _BIT_ARRAY_BASE_HPP_INCLUDED
// ========================================================================== //