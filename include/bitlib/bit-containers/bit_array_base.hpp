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
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

template <typename T, size_t N, typename W, std::size_t A, typename Policy>
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
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
class array_base : public detail::container_size_storage<std::size_t, resizable, N> {
 protected:
  constexpr Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  constexpr const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }

 public:
  using detail::container_size_storage<std::size_t, resizable, N>::size;
  using word_type = W;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = typename std::conditional<std::is_same_v<T, bit_value>, ::bit::reference<word_type&>, T&>::type;
  using const_reference = typename std::conditional<std::is_same_v<T, bit_value>, const ::bit::reference<const word_type&>, const T&>::type;
  using value_pointer = pointer<word_type, value_type>;
  using const_value_pointer = const value_pointer;
  using iterator = Iterators::iterator;
  using const_iterator = Iterators::const_iterator;

  constexpr array_base() noexcept : detail::container_size_storage<std::size_t, resizable, N>() {}
  constexpr array_base(const size_type& extent) noexcept
    requires(N == std::dynamic_extent)
      : detail::container_size_storage<std::size_t, resizable, N>(extent) {}

  // Element access
  constexpr reference operator[](size_type pos) {
    return derived().begin()[pos];
  }

  constexpr const_reference operator[](size_type pos) const {
    return derived().begin()[pos];
  }

  constexpr reference at(size_type pos) {
    if (pos < size()) {
      return derived().begin()[pos];
    } else {
      throw std::out_of_range("Position is out of range");
    }
  }

  constexpr const_reference at(size_type pos) const {
    if (pos < size()) {
      return derived().begin()[pos];
    } else {
      throw std::out_of_range("Position is out of range");
    }
  }

  constexpr reference front()
    requires(A == std::dynamic_extent)
  {
    return derived().begin()[0];
  }

  constexpr auto front()
    requires(A != std::dynamic_extent)
  {
    return ::bit::reference<word_type&, value_type, A>(*derived().data());
  }

  constexpr const_reference front() const
    requires(A == std::dynamic_extent)
  {
    return derived().begin()[0];
  }

  constexpr const auto front() const
    requires(A != std::dynamic_extent)
  {
    return ::bit::reference<const word_type&, value_type, A>(*derived().data());
  }

  constexpr reference back()
    requires(A == std::dynamic_extent || N == std::dynamic_extent)
  {
    return derived().begin()[size() - 1];
  }

  constexpr auto back()
    requires(!(A == std::dynamic_extent || N == std::dynamic_extent))
  {
    constexpr size_type back_index = (A + N * bitsof<value_type>() - 1) / bitsof<word_type>();
    constexpr size_type back_pos = (A + N * bitsof<value_type>() - 1) % bitsof<word_type>();
    return ::bit::reference<word_type&, value_type, back_pos>(derived().data()[back_index]);
  }

  constexpr const_reference back() const
    requires(A == std::dynamic_extent || N == std::dynamic_extent)
  {
    return derived().begin()[size() - 1];
  }

  constexpr const auto back() const
    requires(!(A == std::dynamic_extent || N == std::dynamic_extent))
  {
    constexpr size_type back_index = (A + N * bitsof<value_type>() - 1) / bitsof<word_type>();
    constexpr size_type back_pos = (A + N * bitsof<value_type>() - 1) % bitsof<word_type>();
    return ::bit::reference<const word_type&, value_type, back_pos>(derived().data()[back_index]);
  }

  constexpr iterator end() noexcept {
    return derived().begin() + size();
  }

  constexpr const_iterator end() const noexcept {
    return const_iterator(derived().begin()) + size();
  }

  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(derived().begin());
  }

  constexpr const_iterator cend() const noexcept {
    return const_iterator(derived().end());
  }

  // Capacity
  constexpr bool empty() const noexcept {
    return 0 == size();
  }

  constexpr size_type max_size() const noexcept {
    return size();
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
    return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(&this->at(offset), right - offset);
  }

  /**
   * @brief Slice operations - returns a array_ref
   */
  constexpr auto operator()(size_type offset, size_type right) noexcept {
    return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(&this->at(offset), right - offset);
  }

  /**
   * @brief Compile-time slice [Start, End) - returns a array_ref.
   *
   * Supports all combinations of Start/End being compile-time constants or
   * runtime values:
   *   slice<Start, End>()          - both compile-time constants
   *   slice<Start>(end)            - Start compile-time, End runtime
   *   slice<End>(start)            - Start runtime, End compile-time
   *
   * @tparam Start Start offset in bits
   * @tparam End End offset in bits
   */
  template <std::size_t Start, std::size_t End>
  constexpr auto slice() const noexcept
    requires(N != std::dynamic_extent && Start <= End && End <= N)
  {
    return array_ref<value_type, End - Start, const word_type, std::dynamic_extent, Policy>(&this->at(Start));
  }

  template <std::size_t Start, std::size_t End>
  constexpr auto slice() noexcept
    requires(N != std::dynamic_extent && Start <= End && End <= N)
  {
    return array_ref<value_type, End - Start, word_type, std::dynamic_extent, Policy>(&this->at(Start));
  }

  /**
   * @brief Compile-time Start, runtime End slice [Start, end) - returns a array_ref.
   *
   * @tparam Start Start offset in bits (compile-time constant)
   * @param end End offset in bits (runtime)
   */
  template <std::size_t Start>
  constexpr auto slice(size_t end) const noexcept
    requires(Start != std::dynamic_extent && N != std::dynamic_extent && Start <= N)
  {
    return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
        &this->at(Start), end - Start);
  }

  template <std::size_t Start>
  constexpr auto slice(size_t end) noexcept
    requires(Start != std::dynamic_extent && N != std::dynamic_extent && Start <= N)
  {
    return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
        &this->at(Start), end - Start);
  }

  /**
   * @brief Runtime Start, compile-time End slice [start, End) - returns a array_ref.
   *
   * Start=std::dynamic_extent means the start offset is a runtime argument.
   *
   * @tparam Start std::dynamic_extent (runtime start sentinel)
   * @tparam End End offset in bits (compile-time constant)
   * @param start Start offset in bits (runtime)
   */
  template <std::size_t Start = std::dynamic_extent, std::size_t End>
  constexpr auto slice(size_t start) const noexcept
    requires(Start == std::dynamic_extent && N != std::dynamic_extent)
  {
    return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
        &this->at(start), End - start);
  }

  template <std::size_t Start = std::dynamic_extent, std::size_t End>
  constexpr auto slice(size_t start) noexcept
    requires(Start == std::dynamic_extent && N != std::dynamic_extent)
  {
    return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
        &this->at(start), End - start);
  }

  /**
   * @brief Compile-time slice [Start, End) - returns a array_ref.
   *
   * Supports all combinations of Start/End being compile-time constants or
   * runtime values:
   *   slice<Start, End>()          - both compile-time constants
   *   slice<Start>(end)            - Start compile-time, End runtime
   *
   * @tparam Start Start offset in bits
   * @tparam End End offset in bits
   */
  template <std::size_t Start, std::size_t End>
  constexpr auto slice() const noexcept
    requires(N == std::dynamic_extent && Start <= End)
  {
    return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
        &this->at(Start), End - Start);
  }

  template <std::size_t Start, std::size_t End>
  constexpr auto slice() noexcept
    requires(N == std::dynamic_extent && Start <= End)
  {
    return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
        &this->at(Start), End - Start);
  }

  /**
   * @brief Compile-time Start, runtime End slice [Start, end) - returns a array_ref.
   *
   * @tparam Start Start offset in bits (compile-time constant)
   * @param end End offset in bits (runtime)
   */
  template <std::size_t Start>
  constexpr auto slice(size_t end) const noexcept
    requires(N == std::dynamic_extent)
  {
    return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
        &this->at(Start), end - Start);
  }

  template <std::size_t Start>
  constexpr auto slice(size_t end) noexcept
    requires(N == std::dynamic_extent)
  {
    return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
        &this->at(Start), end - Start);
  }

  /**
   * @brief Runtime Start, compile-time End slice [start, End) - returns a array_ref.
   *
   * Start=std::dynamic_extent means the start offset is a runtime argument.
   * End is a compile-time constant upper bound.
   *
   * @tparam Start std::dynamic_extent (runtime start)
   * @tparam End End offset in bits (compile-time constant)
   * @param start Start offset in bits (runtime)
   */
  template <std::size_t Start = std::dynamic_extent, std::size_t End>
  constexpr auto slice(size_t start) const noexcept
    requires(Start == std::dynamic_extent && N == std::dynamic_extent)
  {
    return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
        &this->at(start), End - start);
  }

  template <std::size_t Start = std::dynamic_extent, std::size_t End>
  constexpr auto slice(size_t start) noexcept
    requires(Start == std::dynamic_extent && N == std::dynamic_extent)
  {
    return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
        &this->at(start), End - start);
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
    assert(size() <= bitsof<U>());
    U integral;

    if constexpr (N == std::dynamic_extent) {
      if (size() > bitsof<U>()) {
        Policy::truncation::template to_integral<U, N>(derived(), integral);
      } else {
        ::bit::copy(derived().begin(), end(), &integral);
      }
      if (size() < bitsof<U>()) {
        Policy::extension::template to_integral<U, N>(detail::uninitialized, derived(), integral);
      }
    } else {
      if constexpr (N > bitsof<U>()) {
        Policy::truncation::template to_integral<U, N>(derived(), integral);
      } else {
        ::bit::copy(derived().begin(), end(), &integral);
      }
      if constexpr (N < bitsof<U>()) {
        Policy::extension::template to_integral<U, N>(detail::uninitialized, derived(), integral);
      }
    }

    return integral;
  }

  using compatible_bitarray = array<value_type, N, word_type, Policy>;

  constexpr compatible_bitarray operator~() {
    compatible_bitarray result(detail::uninitialized, size());
    transform(derived().begin(), derived().end(), result.begin(), [](const word_type& bits) -> word_type { return ~bits; });
    return result;
  }

  constexpr compatible_bitarray operator|(const bit_sized_range auto& other) const {
    assert(other.size() == size());
    compatible_bitarray result(detail::uninitialized, size());
    transform(derived().begin(), derived().end(), other.begin(), result.begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a | b; });
    return result;
  }
  constexpr Derived& operator|=(const bit_sized_range auto& other) {
    assert(other.size() == size());
    transform(derived().cbegin(), derived().cend(), other.begin(), derived().begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a | b; });
    return derived();
  }
  constexpr compatible_bitarray operator&(const bit_sized_range auto& other) const {
    assert(other.size() == size());
    compatible_bitarray result(detail::uninitialized, size());
    transform(derived().begin(), derived().end(), other.begin(), result.begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a & b; });
    return result;
  }
  constexpr Derived& operator&=(const bit_sized_range auto& other) {
    assert(other.size() == size());
    transform(derived().cbegin(), derived().cend(), other.begin(), derived().begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a & b; });
    return derived();
  }
  constexpr compatible_bitarray operator^(const bit_sized_range auto& other) const {
    assert(other.size() == size());
    compatible_bitarray result(detail::uninitialized, size());
    transform(derived().begin(), derived().end(), other.begin(), result.begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a ^ b; });
    return result;
  }
  constexpr Derived& operator^=(const bit_sized_range auto& other) {
    assert(other.size() == size());
    transform(derived().cbegin(), derived().cend(), other.begin(), derived().begin(),
              [](const word_type& a, const word_type& b) -> word_type { return a ^ b; });
    return derived();
  }

  friend std::ostream& operator<<(std::ostream& os, const array_base& cv) {
    // Save stream formatting settings
    std::ios_base::fmtflags flags = os.flags();
    char fill = os.fill();
    std::streamsize width = os.width();

    std::string content;
    switch (flags & std::ios_base::basefield) {
      case std::ios_base::hex:
        content = ::bit::to_string(cv.derived(), ::bit::string::typical(16));
        break;
      case std::ios_base::oct:
        content = ::bit::to_string(cv.derived(), ::bit::string::typical(8));
        break;
      case std::ios_base::dec:
        content = ::bit::to_string(cv.derived(), ::bit::string::typical(10));
        break;
      default:
        content = ::bit::to_string(cv.derived(), ::bit::string::typical(2));
        break;
    }

    // Clear width to avoid affecting next output
    os.width(0);

    // Apply padding manually if needed
    if (width > static_cast<std::streamsize>(content.size())) {
      std::streamsize pad = width - content.size();
      bool left = (flags & std::ios_base::adjustfield) == std::ios_base::left;

      if (!left) {
        os << std::string(pad, fill);
      }

      os << content;

      if (left) {
        os << std::string(pad, fill);
      }
    } else {
      os << content;
    }

    return os;
  }

 protected:
  template <typename U>
  constexpr void from_integral(const U& integral) {
    if constexpr (N == std::dynamic_extent) {
      if ((size() * bitsof<value_type>()) < bitsof<U>()) {
        Policy::truncation::template from_integral<U, N>(integral, derived());
      } else {
        ::bit::copy(&integral, &integral + 1, derived().begin());
      }
      if (bitsof<U>() < (size() * bitsof<value_type>())) {
        Policy::extension::template from_integral<U, N>(detail::uninitialized, integral, derived());
      }
    } else {
      if constexpr ((N * bitsof<value_type>()) < bitsof<U>()) {
        Policy::truncation::template from_integral<U, N>(integral, derived());
      } else {
        ::bit::copy(&integral, &integral + 1, derived().begin());
      }
      if constexpr (bitsof<U>() < (N * bitsof<value_type>())) {
        Policy::extension::template from_integral<U, N>(detail::uninitialized, integral, derived());
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
