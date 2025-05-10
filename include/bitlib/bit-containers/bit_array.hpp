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
#include <cstring>  // memcpy
#include <span>
#include <string>
#include <type_traits>
#include <vector>

// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_base.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_span.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {
// ========================================================================== //

namespace detail {

template <typename value_type, typename word_type, std::size_t N>
constexpr size_t Words() { return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>(); }

template <typename value_type, typename word_type, std::size_t N>
using bit_array_d_it = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                                 bit_iterator<typename std::array<word_type, Words<value_type, word_type, N>()>::iterator>,
                                                 typename std::array<word_type, Words<value_type, word_type, N>()>::iterator>::type;

template <typename value_type, typename word_type, std::size_t N>
using bit_array_d_cit = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                                  bit_iterator<typename std::array<word_type, Words<value_type, word_type, N>()>::const_iterator>,
                                                  typename std::array<const word_type, Words<value_type, word_type, N>()>::const_iterator>::type;
}  // namespace detail

template <typename T = bit_value,
          std::size_t N = std::dynamic_extent,
          std::align_val_t V = std::align_val_t(alignof(T)),
          typename W = std::conditional_t<std::is_same_v<T, bit_value>, uint8_t, T>>
class bit_array : public bit_array_base<bit_array<T, N, V, W>, T, W, detail::bit_array_d_it<T, W, N>, detail::bit_array_d_cit<T, W, N>> {
 public:
  using base = bit_array_base<bit_array<T, N, V, W>, T, W, detail::bit_array_d_it<T, W, N>, detail::bit_array_d_cit<T, W, N>>;
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
  static constexpr std::size_t Words_ = Words(N);
  static constexpr std::size_t AlignedWords = (((Words_ * sizeof(word_type) + static_cast<size_t>(V) - 1) & ~(static_cast<size_t>(V) - 1)) + sizeof(word_type) - 1) / sizeof(word_type);

  alignas(static_cast<size_t>(V)) std::array<word_type, AlignedWords> storage;

 public:
  /*
  * Constructors, copies and moves...
  */
  constexpr bit_array() noexcept : storage{} {}

  constexpr bit_array(value_type bit_val) : storage{} {
    this->fill(bit_val);
  }

  template <std::integral U>
  constexpr bit_array(const U& integral)
    requires(bitsof<U>() <= bits)
  {
    std::memcpy(&storage[0], &integral, sizeof(integral));

    bool sign_extend = false;
    if constexpr (std::is_signed_v<U>) {
      sign_extend = (integral & (1 << (bitsof<U>() - 1))) ? true : false;
    }
    if (sign_extend) {
      for (auto it = begin() + bitsof<U>(); it != end(); ++it) {
        *it = bit1;
      }
    } else {
      for (auto it = begin() + bitsof<U>(); it != end(); ++it) {
        *it = bit0;
      }
    }
  }

  constexpr bit_array(const bit_array<T, N, V, W>& other) = default;

  constexpr bit_array(const bit_array<T, N, V, W>&& other) noexcept
      : storage(other.storage) {}

  constexpr bit_array(const std::initializer_list<value_type> init)
    requires(!std::is_same_v<value_type, word_type>)
  {
    if (init.size() != bitsof(*this)) [[unlikely]] {
      throw std::invalid_argument("initialize_list contains an invalid number of bits for bit_array.");
    }
    std::copy(init.begin(), init.end(), this->begin());
  }

  constexpr bit_array(const std::initializer_list<bool> init) {
    if (init.size() != bitsof(*this)) [[unlikely]] {
      throw std::invalid_argument("initialize_list contains an invalid number of bits for bit_array.");
    }
    std::copy(init.begin(), init.end(), this->begin());
  }

  constexpr bit_array(const std::initializer_list<word_type> init) : storage{} {
    // Make sure we handle the case where init.size() != Words
    auto it = init.begin();
    for (size_type i = 0; i < std::min(AlignedWords, init.size()); ++i, ++it) {
      storage[i] = *it;
    }
  }

  constexpr bit_array(const std::string_view s)
    requires(std::is_same_v<value_type, bit_value>)
  {
    if (bitsof(*this) != static_cast<size_t>(std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))) [[unlikely]] {
      throw std::invalid_argument("String contains an invalid number of bits for bit_array.");
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

  ~bit_array() = default;
  /*
    * Assignment
    */
  constexpr bit_array& operator=(const bit_array<T, N, V, W>& other) = default;

  constexpr bit_array& operator=(bit_array<T, N, V, W>&& other) noexcept {
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

  constexpr iterator end() noexcept {
    return begin() + size();
  }

  constexpr const_iterator begin() const noexcept {
    return const_iterator(storage.begin());
  }

  constexpr const_iterator end() const noexcept {
    return const_iterator(storage.begin()) + size();
  }

  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(storage.begin());
  }

  constexpr const_iterator cend() const noexcept {
    return const_iterator(storage.begin()) + size();
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
  constexpr void swap(bit_array<T, N, V, W>& other) noexcept {
    std::swap(this->storage, other.storage);
  }

  template <std::integral U>
  explicit constexpr operator U() const noexcept
    requires(bitsof<U>() >= (bitsof<T>() * N))
  {
    return base::operator U();
  }
};

static_assert(bit_range<bit_array<bit_value, 11>>, "bit_array does not satisfy bit_range concept!");
static_assert(bit_sized_range<bit_array<bit_value, 11>>, "bit_array does not satisfy bit_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<bit_array<11>>, "bit_array does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<bit_array<11>>, "bit_array does not satisfy bit_contiguous_sized_range concept!");
#endif

#if 0
// Class Template Argument Deduction
// CTAD guide for the constructor taking a word_type&,
// deducing Extent as bitsof<word_type>().
template <typename word_type>
bit_array(word_type&) -> bit_array<word_type, bitsof<word_type>()>;
template <typename word_type>
bit_array(word_type*) -> bit_array<word_type, bitsof<word_type>()>;

// CTAD guide for the constructor taking a word_type* and a size,
// which should always be used when Extent is std::dynamic_extent.
template <typename word_type>
bit_array(word_type&, std::size_t) -> bit_array<word_type, std::dynamic_extent>;
template <typename word_type>
bit_array(word_type*, std::size_t) -> bit_array<word_type, std::dynamic_extent>;
#endif

}  // namespace bit

#endif  // _BIT_ARRAY_HPP_INCLUDED