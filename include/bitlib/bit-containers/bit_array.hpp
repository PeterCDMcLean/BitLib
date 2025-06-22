// ================================= BIT_ARRAY =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array.hpp
// Description: Implementation of bit_array
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ARRAY_HPP_INCLUDED
#define _BIT_ARRAY_HPP_INCLUDED
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
#include "bitlib/bit-containers/bit_array_base.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit-containers/bit_span.hpp"
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

template <typename T = bit_value,
          std::size_t N = std::dynamic_extent,
          typename W = std::conditional_t<
              std::is_same_v<T, bit_value>,
              std::conditional_t<
                  (N == std::dynamic_extent),
                  std::uintptr_t,
                  ceil_integral<N>>,
              T>,
          typename Policy = policy::typical<W>>
class array;

template <std::size_t N = std::dynamic_extent,
          typename W = std::conditional_t<
              (N == std::dynamic_extent),
              std::uintptr_t,
              ceil_integral<N>>,
          typename Policy = policy::typical<W>>
using bit_array = array<bit_value, N, W, Policy>;

namespace detail {

template <typename value_type, typename word_type, std::size_t N>
constexpr size_t Words() { return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>(); }

template <typename value_type, typename word_type, std::size_t N>
struct array_iterator_types {
  using iterator = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                             bit_iterator<typename std::array<word_type, Words<value_type, word_type, N>()>::iterator>,
                                             typename std::array<word_type, Words<value_type, word_type, N>()>::iterator>::type;

  using const_iterator = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                                   bit_iterator<typename std::array<word_type, Words<value_type, word_type, N>()>::const_iterator>,
                                                   typename std::array<const word_type, Words<value_type, word_type, N>()>::const_iterator>::type;
};
}  // namespace detail

template <typename T, std::size_t N, typename W, typename Policy>
class array : public array_base<array<T, N, W>, T, N, W, Policy, detail::array_iterator_types<T, W, N>> {
 public:
  using base = array_base<array<T, N, W>, T, N, W, Policy, detail::array_iterator_types<T, W, N>>;
  using base::end;
  using typename base::const_iterator;
  using typename base::const_pointer;
  using typename base::const_reference;
  using typename base::difference_type;
  using typename base::iterator;
  using typename base::pointer;
  using typename base::reference;
  using typename base::size_type;
  using typename base::value_type;
  using typename base::word_type;

  static constexpr std::size_t bits = N * bitsof<T>();

 protected:
  static constexpr std::size_t Words(std::size_t size_) {
    return (size_ * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>();
  }

 private:
  std::array<word_type, Words(N)> storage;

 public:
  /*
  * Constructors, copies and moves...
  */
  constexpr array() noexcept : storage{} {}

  constexpr array(value_type bit_val) {
    this->fill(bit_val);
  }

  template <std::integral U>
  constexpr array(const U& integral) {
    this->from_integral(integral);
  }

  constexpr array(const array<T, N, W>& other) noexcept
      : storage(other.storage) {}

  constexpr array(const array<T, N, W>&& other) noexcept
      : storage(other.storage) {}

  constexpr array(const bit_sized_range auto& other) {
    if (other.size() != this->size()) [[unlikely]] {
      throw std::invalid_argument("other bit_range contains an invalid number of bits for array.");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
  };

  constexpr array(const std::initializer_list<value_type> init)
    requires(!std::is_same_v<value_type, word_type>)
  {
    if (init.size() != bitsof(*this)) [[unlikely]] {
      throw std::invalid_argument("initialize_list contains an invalid number of bits for array.");
    }
    std::copy(init.begin(), init.end(), this->begin());
  }

  constexpr array(const std::initializer_list<bool> init) {
    if (init.size() != bitsof(*this)) [[unlikely]] {
      throw std::invalid_argument("initialize_list contains an invalid number of bits for array.");
    }
    std::copy(init.begin(), init.end(), this->begin());
  }

  constexpr array(const std::initializer_list<word_type> init) : storage{} {
    // Make sure we handle the case where init.size() != Words
    auto it = init.begin();
    for (size_type i = 0; i < std::min(Words(N), init.size()); ++i, ++it) {
      storage[i] = *it;
    }
  }

  constexpr array(const std::string_view s)
    requires(std::is_same_v<value_type, bit_value>)
  {
    if (bitsof(*this) != static_cast<size_t>(std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))) [[unlikely]] {
      throw std::invalid_argument("String contains an invalid number of bits for array.");
    };
    size_type i = 0;
    for (char c : s) {
      if (c == '0') {
        begin()[i++] = bit0;
      } else if (c == '1') {
        begin()[i++] = bit1;
      }
    }
  }

  ~array() = default;
  /*
    * Assignment
    */
  constexpr array& operator=(const array<T, N, W, Policy>& other) = default;

  constexpr array& operator=(const bit_sized_range auto& other) {
    if (other.size() != this->size()) [[unlikely]] {
      throw std::invalid_argument("other bit_sized_range contains an invalid number of bits for array.");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
    return *this;
  };

  constexpr array& operator=(array<T, N, W, Policy>&& other) noexcept {
    std::copy(other.storage.begin(), other.storage.end(), storage.begin());
    return *this;
  }

  constexpr word_type* data() noexcept {
    return size() ? storage.data() : nullptr;
  }

  constexpr const word_type* data() const noexcept {
    return size() ? storage.data() : nullptr;
  }

  /*
    * Iterators
    */
  constexpr iterator begin() noexcept {
    return iterator(storage.begin());
  }

  constexpr const_iterator begin() const noexcept {
    return const_iterator(storage.begin());
  }

  /*
    * Capacity
    */
  constexpr size_type size() const noexcept {
    return N;
  }

  /*
    * Operations
    */
  constexpr void swap(array<T, N, W>& other) noexcept {
    std::swap(this->storage, other.storage);
  }

  constexpr operator bit_value() const noexcept
    requires(std::is_same_v<value_type, bit_value> && N == 1)
  {
    return this->begin()[0];
  }
};

static_assert(bit_range<bit_array<11>>, "array does not satisfy bit_range concept!");
static_assert(bit_sized_range<bit_array<11>>, "array does not satisfy bit_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<array<11>>, "array does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<array<11>>, "array does not satisfy bit_contiguous_sized_range concept!");
#endif

#if 0
// Class Template Argument Deduction
// CTAD guide for the constructor taking a word_type&,
// deducing Extent as bitsof<word_type>().
template <typename word_type>
array(word_type&) -> array<word_type, bitsof<word_type>()>;
template <typename word_type>
array(word_type*) -> array<word_type, bitsof<word_type>()>;

// CTAD guide for the constructor taking a word_type* and a size,
// which should always be used when Extent is std::dynamic_extent.
template <typename word_type>
array(word_type&, std::size_t) -> array<word_type, std::dynamic_extent>;
template <typename word_type>
array(word_type*, std::size_t) -> array<word_type, std::dynamic_extent>;
#endif

}  // namespace bit

#endif  // _array_HPP_INCLUDED