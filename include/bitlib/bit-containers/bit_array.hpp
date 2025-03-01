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
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <type_traits>
// Project sources
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"

namespace bit {
// ========================================================================== //

template<std::size_t N>
class bit_array {
public:
static constexpr std::size_t bits = N;

private:
using T = bit_value;
using WordType = std::uint8_t;
static constexpr std::size_t Words = (N * bitsof<T>() + bitsof<WordType>() - 1) / bitsof<WordType>();

alignas(alignof(T)) std::array<WordType, Words> storage;

public:
using value_type             = bit_value;
using size_type              = std::size_t;
using difference_type        = std::ptrdiff_t;
using reference              = bit_reference<WordType>;
using const_reference        = const reference;
using pointer                = bit_pointer<WordType>;
using const_pointer          = const pointer;
using iterator               = bit_iterator<typename std::array<WordType, Words>::iterator>;
using const_iterator         = bit_iterator<typename std::array<WordType, Words>::const_iterator>;

/*
  * Constructors, copies and moves...
  */
constexpr bit_array() noexcept;
constexpr bit_array(value_type bit_val);
constexpr bit_array(const bit_array<N>& other) = default;
constexpr bit_array(const bit_array<N>&& other) noexcept;
constexpr bit_array(const std::initializer_list<bit_value> init);
constexpr bit_array(const std::initializer_list<bool> init);
constexpr bit_array(const std::initializer_list<WordType> init);
constexpr bit_array(const std::string_view s);

~bit_array() = default;

private:

public:

  /*
    * Assignment
    */
  constexpr bit_array& operator=(const bit_array<N>& other) = default;
  constexpr bit_array& operator=(bit_array<N>&& other) noexcept;

  constexpr bool operator==(const bit_array<N>& other) const noexcept;

  /*
    * Element Access
    */
  constexpr reference operator[](size_type pos);
  constexpr const_reference operator[](size_type pos) const;
  constexpr reference at(size_type pos);
  constexpr const_reference at(size_type pos) const;
  constexpr reference front();
  constexpr const_reference front() const;
  constexpr reference back();
  constexpr const_reference back() const;
  constexpr WordType* data() noexcept;
  constexpr const WordType* data() const noexcept;

  /*
    * Iterators
    */
  constexpr iterator begin() noexcept;
  constexpr iterator end() noexcept;
  constexpr const_iterator begin() const noexcept;
  constexpr const_iterator end() const noexcept;
  constexpr const_iterator cbegin() const noexcept;
  constexpr const_iterator cend() const noexcept;

  /*
    * Capacity
    */
  constexpr bool empty() const noexcept;
  constexpr size_type size() const noexcept;
  constexpr size_type max_size() const noexcept;

  /*
    * Operations
    */
  constexpr void fill(value_type bit_val) noexcept;
  constexpr void swap(bit_array<N>& other) noexcept;
  //constexpr std::synth-three-way-result<bit_array> operator<=>() const noexcept;
};

template<std::size_t N>
constexpr bit_array<N>::bit_array() noexcept : storage{} {}

template<std::size_t N>
constexpr bit_array<N>::bit_array(bit_array<N>::value_type bit_val) : storage{} {
  fill(bit_val);
}

template<std::size_t N>
constexpr void bit_array<N>::fill(bit_array<N>::value_type bit_val) noexcept {
  if (bit_val) {
    std::fill(storage.begin(), storage.end(), bit_array<N>::WordType(-1));
  } else {
    std::fill(storage.begin(), storage.end(), bit_array<N>::WordType(0));
  }
}

template<std::size_t N>
constexpr bit_array<N>::bit_array(const std::initializer_list<bit_value> init) {
  if(init.size() != bitsof(*this)) [[unlikely]] {
    throw std::invalid_argument("initialize_list contains an invalid number of bits for bit_array.");
  }
  std::copy(init.begin(), init.end(), this->begin());
}

template<std::size_t N>
constexpr bit_array<N>::bit_array(const std::initializer_list<bool> init) {
  if(init.size() != bitsof(*this)) [[unlikely]] {
    throw std::invalid_argument("initialize_list contains an invalid number of bits for bit_array.");
  }
  std::copy(init.begin(), init.end(), this->begin());
}

template<std::size_t N>
constexpr bit_array<N>::bit_array(const std::initializer_list<WordType> init) : storage(init) {
  static_assert(init.size() == storage.size());
}

template<std::size_t N>
constexpr bit_array<N>::bit_array(const std::string_view s) {
  if(bitsof(*this) != (std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))) [[unlikely]] {
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

template<std::size_t N>
constexpr bit_array<N>::bit_array(const bit_array<N>&& other) noexcept
    : storage(other.storage) {}


template<std::size_t N>
constexpr bit_array<N>& bit_array<N>::operator=(bit_array<N>&& other) noexcept {
  std::copy(other.storage.begin(), other.storage.end(), storage.begin());
  return *this;
}

template<std::size_t N>
constexpr bool bit_array<N>::operator==(const bit_array<N>& other) const noexcept {
  return equal(begin(), end(), other.begin());
}

template<std::size_t N>
constexpr void bit_array<N>::swap(bit_array<N>& other) noexcept {
  std::swap(this->storage, other.storage);
}

// -------------------------------------------------------------------------- //
  /*
    * Element Access
    */
template<std::size_t N>
constexpr typename bit_array<N>::reference bit_array<N>::operator[](size_type pos) {
  return begin()[pos];
}

template<std::size_t N>
constexpr typename bit_array<N>::const_reference bit_array<N>::operator[](size_type pos) const {
  return begin()[pos];
}

template<std::size_t N>
constexpr typename bit_array<N>::reference bit_array<N>::at(size_type pos) {
  if (pos < size()) {
    return begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

template<std::size_t N>
constexpr typename bit_array<N>::const_reference bit_array<N>::at(size_type pos) const {
  if (pos < size()) {
    return begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

template<std::size_t N>
constexpr typename bit_array<N>::reference bit_array<N>::front() {
  return begin()[0];
}

template<std::size_t N>
constexpr typename bit_array<N>::const_reference bit_array<N>::front() const {
  return begin()[0];
}

template<std::size_t N>
constexpr typename bit_array<N>::reference bit_array<N>::back() {
  return begin()[size()-1];
}

template<std::size_t N>
constexpr typename bit_array<N>::const_reference bit_array<N>::back() const {
  return begin()[size()-1];
}

template<std::size_t N>
constexpr typename bit_array<N>::WordType* bit_array<N>::data() noexcept {
  return size() ? storage.data() : nullptr;
}

template<std::size_t N>
constexpr const typename bit_array<N>::WordType* bit_array<N>::data() const noexcept {
  return size() ? storage.data() : nullptr;
}

// -------------------------------------------------------------------------- //


template<std::size_t N>
constexpr typename bit_array<N>::size_type bit_array<N>::size() const noexcept {return N;}

template<std::size_t N>
constexpr typename bit_array<N>::size_type bit_array<N>::max_size() const noexcept {return size();}

template<std::size_t N>
constexpr bool bit_array<N>::empty() const noexcept {return 0 == size();}

// Iterators
// -------------------------------------------------------------------------- //

template<std::size_t N>
constexpr typename bit_array<N>::iterator bit_array<N>::begin() noexcept {
  return iterator(storage.begin());
}

template<std::size_t N>
constexpr typename bit_array<N>::iterator bit_array<N>::end() noexcept {
  return begin() + size();
}

template<std::size_t N>
constexpr typename bit_array<N>::const_iterator bit_array<N>::begin() const noexcept {
  return const_iterator(storage.begin());
}

template<std::size_t N>
constexpr typename bit_array<N>::const_iterator bit_array<N>::end() const noexcept {
  return const_iterator(storage.begin()) + size();
}

template<std::size_t N>
constexpr typename bit_array<N>::const_iterator bit_array<N>::cbegin() const noexcept {
  return const_iterator(storage.begin());
}

template<std::size_t N>
constexpr typename bit_array<N>::const_iterator bit_array<N>::cend() const noexcept {
  return const_iterator(storage.begin()) + size();
}

// -------------------------------------------------------------------------- //


// ========================================================================== //
} // namespace bit
#endif // _BIT_ARRAY_HPP_INCLUDED
// ========================================================================== //
