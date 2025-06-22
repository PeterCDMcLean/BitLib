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

// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit-containers/bit_span.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

template <typename T, typename W, typename Policy>
class array_ref;

template <typename T,
          std::size_t N,
          typename W,
          typename Policy>
class array;

/**
 * @brief Base class template for array implementations
 *
 * This is a CRTP (Curiously Recurring Template Pattern) base class that provides
 * common functionality for array variants.
 *
 * @tparam Derived The derived class (CRTP pattern)
 * @tparam T The value type (typically bit_value)
 * @tparam W The word type used for storage
 * @tparam N The size of the bit array, or std::dynamic_extent for dynamic size
 * @tparam Policy The policy for integral conversion (default is typical)
 * @tparam Iterators A struct that provides iterator and const_iterator types
 */
template <typename Derived, typename T, size_t N, typename W, typename Policy, typename Iterators>
class array_base {
 protected:
  constexpr Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  constexpr const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }

 public:
  using word_type = W;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = typename std::conditional<std::is_same_v<T, bit_value>, bit_reference<word_type&>, T&>::type;
  using const_reference = typename std::conditional<std::is_same_v<T, bit_value>, const bit_reference<const word_type&>, const T&>::type;
  using pointer = typename std::conditional<std::is_same_v<T, bit_value>, bit_pointer<word_type>, T&>::type;
  using const_pointer = const pointer;
  using iterator = Iterators::iterator;
  using const_iterator = Iterators::const_iterator;

  // Element access
  constexpr reference operator[](size_type pos) {
    return derived().begin()[pos];
  }

  constexpr const_reference operator[](size_type pos) const {
    return derived().begin()[pos];
  }

  constexpr reference at(size_type pos) {
    if (pos < derived().size()) {
      return derived().begin()[pos];
    } else {
      throw std::out_of_range("Position is out of range");
    }
  }

  constexpr const_reference at(size_type pos) const {
    if (pos < derived().size()) {
      return derived().begin()[pos];
    } else {
      throw std::out_of_range("Position is out of range");
    }
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

  constexpr iterator end() noexcept {
    return derived().begin() + derived().size();
  }

  constexpr const_iterator end() const noexcept {
    return const_iterator(derived().begin()) + derived().size();
  }

  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(derived().begin());
  }

  constexpr const_iterator cend() const noexcept {
    return const_iterator(derived().end());
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
    return debug_string(derived().begin(), derived().end());
  }

  constexpr std::string debug_string(const_iterator first, const_iterator last) const {
    std::string ret = "";
    auto position = 0;
    for (auto it = first; it != last; ++it) {
      if (position % bitsof<word_type>() == 0 && position != 0) {
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
   * @brief Slice operations - returns a array_ref
   */
  constexpr auto operator()(size_type offset, size_type right) const noexcept {
    return array_ref<value_type, const word_type, Policy>(&this->at(offset), right - offset);
  }

  /**
   * @brief Slice operations - returns a array_ref
   */
  constexpr auto operator()(size_type offset, size_type right) noexcept {
    return array_ref<value_type, word_type, Policy>(&this->at(offset), right - offset);
  }

  // Common operations
  constexpr void fill(value_type bit_val) noexcept {
    std::fill(derived().begin(), derived().end(), bit_val);
  }

  /**
   * @brief Explicit conversion to integral types
   */
  template <std::integral U>
  explicit constexpr operator U() const noexcept {
    assert(derived().size() <= bitsof<U>());
    U integral;

    if constexpr (N == std::dynamic_extent) {
      if (derived().size() > bitsof<U>()) {
        Policy::truncation::template to_integral<U, N>(derived(), integral);
      } else {
        ::bit::copy(derived().begin(), end(), bit_pointer<U>(&integral));
      }
      if (derived().size() < bitsof<U>()) {
        Policy::extension::template to_integral<U, N>(derived(), integral);
      }
    } else {
      if constexpr (N > bitsof<U>()) {
        Policy::truncation::template to_integral<U, N>(derived(), integral);
      } else {
        ::bit::copy(derived().begin(), end(), bit_pointer<U>(&integral));
      }
      if constexpr (N < bitsof<U>()) {
        Policy::extension::template to_integral<U, N>(derived(), integral);
      }
    }

    return integral;
  }

  using compatible_bitarray = array<value_type, N, word_type, Policy>;

  constexpr compatible_bitarray operator~() {
    compatible_bitarray result(derived().size());
    transform(derived().begin(), derived().end(), result.begin(), [](const word_type& bits) -> word_type { return ~bits; });
    return result;
  }

  constexpr compatible_bitarray operator|(const bit_sized_range auto& other) const {
    assert(other.size() == derived().size());
    compatible_bitarray result(derived().size());
    transform(derived().begin(), derived().end(), other.begin(), result.begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a | b; });
    return result;
  }
  constexpr Derived& operator|=(bit_sized_range auto& other) {
    assert(other.size() == derived().size());
    transform(derived().begin(), derived().end(), other.begin(), derived().begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a | b; });
    return derived();
  }
  constexpr compatible_bitarray operator&(const bit_sized_range auto& other) const {
    assert(other.size() == derived().size());
    compatible_bitarray result(derived().size());
    transform(derived().begin(), derived().end(), other.begin(), result.begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a & b; });
    return result;
  }
  constexpr Derived& operator&=(bit_sized_range auto& other) {
    assert(other.size() == derived().size());
    transform(derived().begin(), derived().end(), other.begin(), derived().begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a & b; });
    return derived();
  }
  constexpr compatible_bitarray operator^(const bit_sized_range auto& other) const {
    assert(other.size() == derived().size());
    compatible_bitarray result(derived().size());
    transform(derived().begin(), derived().end(), other.begin(), result.begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a ^ b; });
    return result;
  }
  constexpr Derived& operator^=(bit_sized_range auto& other) {
    assert(other.size() == derived().size());
    transform(derived().begin(), derived().end(), other.begin(), derived().begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a ^ b; });
    return derived();
  }

 protected:
  template <typename U>
  constexpr void from_integral(const U& integral) {
    if constexpr (N == std::dynamic_extent) {
      if ((derived().size() * bitsof<value_type>()) < bitsof<U>()) {
        Policy::truncation::template from_integral<U, N>(derived(), integral);
      } else {
        bit_pointer<const U> integral_ptr(&integral);
        ::bit::copy(integral_ptr, integral_ptr + bitsof<U>(), derived().begin());
      }
      if (bitsof<U>() < (derived().size() * bitsof<value_type>())) {
        Policy::extension::template from_integral<U, N>(derived(), integral, detail::uninitialized);
      }
    } else {
      if constexpr ((N * bitsof<value_type>()) < bitsof<U>()) {
        Policy::truncation::template from_integral<U, N>(derived(), integral);
      } else {
        bit_pointer<const U> integral_ptr(&integral);
        ::bit::copy(integral_ptr, integral_ptr + bitsof<U>(), derived().begin());
      }
      if constexpr (bitsof<U>() < (N * bitsof<value_type>())) {
        Policy::extension::template from_integral<U, N>(derived(), integral, detail::uninitialized);
      }
    }
  }
};

constexpr bool operator==(const bit_sized_range auto& lhs, const bit_sized_range auto& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  return ::bit::equal(lhs.begin(), lhs.end(), rhs.begin());
}

}  // namespace bit

#endif  // _BIT_ARRAY_BASE_HPP_INCLUDED
