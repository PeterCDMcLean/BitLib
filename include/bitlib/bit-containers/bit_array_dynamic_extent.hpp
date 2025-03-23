// ================================= BIT_ARRAY =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array.hpp
// Description: Implementation of bit_array
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ARRAY_DYNAMIC_EXTENT_HPP_INCLUDED
#define _BIT_ARRAY_DYNAMIC_EXTENT_HPP_INCLUDED

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <span>  // std::dynamic_extent
#include <stdexcept>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

template <>
class bit_array<std::dynamic_extent> {
 public:
  using T = bit_value;
  using WordType = std::uint8_t;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = bit_reference<WordType>;
  using const_reference = const bit_reference<const WordType>;
  using pointer = bit_pointer<WordType>;
  using const_pointer = const pointer;
  using iterator = bit_iterator<WordType*>;
  using const_iterator = bit_iterator<const WordType*>;

 private:
  size_type m_size;
  std::unique_ptr<WordType[]> storage;
  static constexpr std::size_t Words(std::size_t N) { return (N * bitsof<T>() + bitsof<WordType>() - 1) / bitsof<WordType>(); };

 public:
  /*
  * Constructors, copies and moves...
  */
  constexpr bit_array() noexcept;
  constexpr bit_array(size_type size);
  constexpr bit_array(size_type size, value_type bit_val);
  constexpr bit_array(const bit_array<std::dynamic_extent>& other);
  constexpr bit_array(const bit_array<std::dynamic_extent>&& other);
  constexpr bit_array(const std::initializer_list<bit_value> init);
#if 0
  constexpr bit_array(const std::initializer_list<bool> init);
#endif
  constexpr bit_array(const std::initializer_list<WordType> init);
  constexpr bit_array(const std::string_view s);

  ~bit_array() = default;
  /*
     * Assignment
     */
  constexpr bit_array<std::dynamic_extent>& operator=(const bit_array<std::dynamic_extent>& other);
  constexpr bit_array<std::dynamic_extent>& operator=(bit_array<std::dynamic_extent>&& other);

  constexpr bool operator==(const bit_array<std::dynamic_extent>& other) const noexcept;

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
  constexpr void swap(bit_array<std::dynamic_extent>& other) noexcept;
  //constexpr std::synth-three-way-result<bit_array> operator<=>() const noexcept;
};

static_assert(bit_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_sized_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_sized_range concept!");
#endif

constexpr bit_array<std::dynamic_extent>::bit_array() noexcept
    : m_size(0), storage(std::unique_ptr<WordType[]>(nullptr)) {
}

constexpr bit_array<std::dynamic_extent>::bit_array(size_type size)
    : m_size(size),
      storage(std::make_unique<WordType[]>(Words(size))) {
}

constexpr bit_array<std::dynamic_extent>::bit_array(size_type size, value_type bit_val)
    : m_size(size),
      storage(std::make_unique<WordType[]>(Words(size))) {
  fill(bit_val);
}

constexpr bit_array<std::dynamic_extent>::bit_array(const bit_array<std::dynamic_extent>& other)
    : m_size(other.size()),
      storage(std::make_unique<WordType[]>(Words(m_size))) {
  std::copy(other.begin(), other.end(), this->begin());
}

constexpr bit_array<std::dynamic_extent>::bit_array(const bit_array<std::dynamic_extent>&& other)
    : m_size(other.size()),
      storage(std::make_unique<WordType[]>(Words(m_size))) {
  std::copy(other.begin(), other.end(), this->begin());
}

constexpr bit_array<std::dynamic_extent>::bit_array(const std::initializer_list<bit_value> init)
    : m_size(init.size()),
      storage(std::make_unique<WordType[]>(Words(Words(m_size)))) {
  std::copy(init.begin(), init.end(), this->begin());
}

#if 0
No known conversion from bool to bit_value
bit_value has explicit constructor from bool to bit_value so this doesnt work
constexpr bit_array<std::dynamic_extent>::bit_array(const std::initializer_list<bool> init)
    : storage(std::make_unique<WordType[]>(Words(init.size()))),
      m_size(init.size()) {
  std::copy(init.begin(), init.end(), this->begin());
}
#endif

constexpr bit_array<std::dynamic_extent>::bit_array(const std::initializer_list<WordType> init)
    : m_size(bitsof<WordType>() * init.size()),
      storage(std::make_unique<WordType[]>(init.size())) {
  std::copy(init.begin(), init.end(), &storage[0]);
}

constexpr bit_array<std::dynamic_extent>::bit_array(const std::string_view s)
    : m_size((std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))),
      storage(std::make_unique<WordType[]>(Words(m_size))) {
  size_type i = 0;
  for (char c : s) {
    if (c == '0') {
      begin()[i++] = bit0;
    } else if (c == '1') {
      begin()[i++] = bit1;
    }
  }
}

constexpr bit_array<std::dynamic_extent>& bit_array<std::dynamic_extent>::operator=(const bit_array<std::dynamic_extent>& other) {
  if (nullptr == storage.get() || m_size != other.m_size) {
    throw std::invalid_argument("Cannot reassign bit_array<std::dynamic_extent> size");
  }
  std::copy(other.begin(), other.end(), this->begin());
  return *this;
}
constexpr bit_array<std::dynamic_extent>& bit_array<std::dynamic_extent>::operator=(bit_array<std::dynamic_extent>&& other) {
  if (nullptr == storage.get() || m_size != other.m_size) {
    throw std::invalid_argument("Cannot reassign bit_array<std::dynamic_extent> size");
  }
  std::copy(other.begin(), other.end(), this->begin());
  return *this;
}

constexpr bool bit_array<std::dynamic_extent>::operator==(const bit_array<std::dynamic_extent>& other) const noexcept {
  return equal(begin(), end(), other.begin());
}

constexpr void bit_array<std::dynamic_extent>::swap(bit_array<std::dynamic_extent>& other) noexcept {
  std::swap(this->storage, other.storage);
  std::swap(this->m_size, other.m_size);
}

constexpr void bit_array<std::dynamic_extent>::fill(value_type bit_val) noexcept {
  if (bit_val) {
    std::fill(&storage[0], &storage[Words(m_size)], bit_array<std::dynamic_extent>::WordType(-1));
  } else {
    std::fill(&storage[0], &storage[Words(m_size)], bit_array<std::dynamic_extent>::WordType(0));
  }
}

// -------------------------------------------------------------------------- //
/*
    * Element Access
    */
constexpr typename bit_array<std::dynamic_extent>::reference bit_array<std::dynamic_extent>::operator[](size_type pos) {
  return begin()[pos];
}

constexpr typename bit_array<std::dynamic_extent>::const_reference bit_array<std::dynamic_extent>::operator[](size_type pos) const {
  return begin()[pos];
}

constexpr typename bit_array<std::dynamic_extent>::reference bit_array<std::dynamic_extent>::at(size_type pos) {
  if (pos < size()) {
    return begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

constexpr typename bit_array<std::dynamic_extent>::const_reference bit_array<std::dynamic_extent>::at(size_type pos) const {
  if (pos < size()) {
    return begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

constexpr typename bit_array<std::dynamic_extent>::reference bit_array<std::dynamic_extent>::front() {
  return begin()[0];
}

constexpr typename bit_array<std::dynamic_extent>::const_reference bit_array<std::dynamic_extent>::front() const {
  return begin()[0];
}

constexpr typename bit_array<std::dynamic_extent>::reference bit_array<std::dynamic_extent>::back() {
  return begin()[size() - 1];
}

constexpr typename bit_array<std::dynamic_extent>::const_reference bit_array<std::dynamic_extent>::back() const {
  return begin()[size() - 1];
}

constexpr typename bit_array<std::dynamic_extent>::WordType* bit_array<std::dynamic_extent>::data() noexcept {
  return size() ? storage.get() : nullptr;
}

constexpr const typename bit_array<std::dynamic_extent>::WordType* bit_array<std::dynamic_extent>::data() const noexcept {
  return size() ? storage.get() : nullptr;
}

// -------------------------------------------------------------------------- //

constexpr typename bit_array<std::dynamic_extent>::size_type bit_array<std::dynamic_extent>::size() const noexcept { return m_size; }

constexpr typename bit_array<std::dynamic_extent>::size_type bit_array<std::dynamic_extent>::max_size() const noexcept { return size(); }

constexpr bool bit_array<std::dynamic_extent>::empty() const noexcept { return 0 == size(); }

// Iterators
// -------------------------------------------------------------------------- //

constexpr typename bit_array<std::dynamic_extent>::iterator bit_array<std::dynamic_extent>::begin() noexcept {
  return iterator(storage.get());
}

constexpr typename bit_array<std::dynamic_extent>::iterator bit_array<std::dynamic_extent>::end() noexcept {
  return begin() + size();
}

constexpr typename bit_array<std::dynamic_extent>::const_iterator bit_array<std::dynamic_extent>::begin() const noexcept {
  return const_iterator(storage.get());
}

constexpr typename bit_array<std::dynamic_extent>::const_iterator bit_array<std::dynamic_extent>::end() const noexcept {
  return const_iterator(storage.get()) + size();
}

constexpr typename bit_array<std::dynamic_extent>::const_iterator bit_array<std::dynamic_extent>::cbegin() const noexcept {
  return const_iterator(storage.get());
}

constexpr typename bit_array<std::dynamic_extent>::const_iterator bit_array<std::dynamic_extent>::cend() const noexcept {
  return const_iterator(storage.get()) + size();
}

// -------------------------------------------------------------------------- //

// ========================================================================== //
}  // namespace bit
#endif  // _BIT_ARRAY_DYNAMIC_EXTENT_HPP_INCLUDED
// ========================================================================== //
