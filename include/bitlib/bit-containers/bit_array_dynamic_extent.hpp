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

template <typename T, std::align_val_t V, typename W>
class bit_array<T, std::dynamic_extent, V, W> {
 public:
  using word_type = W;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = typename std::conditional<std::is_same_v<T, bit_value>, bit_reference<word_type>, T&>::type;
  using const_reference = typename std::conditional<std::is_same_v<T, bit_value>, const bit_reference<const word_type>, const T&>::type;
  using pointer = typename std::conditional<std::is_same_v<T, bit_value>, bit_pointer<word_type>, T&>::type;
  using const_pointer = const pointer;
  using iterator = typename std::conditional<std::is_same_v<T, bit_value>,
                                             bit_iterator<word_type*>,
                                             word_type*>::type;
  using const_iterator = typename std::conditional<std::is_same_v<T, bit_value>,
                                                   bit_iterator<const word_type*>,
                                                   const word_type*>::type;

 private:
  size_type m_size;
  std::unique_ptr<word_type[], decltype(&std::free)> storage;
  static constexpr std::size_t Words(std::size_t N) { return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>(); };

 public:
  /*
  * Constructors, copies and moves...
  */
  bit_array() = delete;
  constexpr bit_array(size_type size);
  constexpr bit_array(size_type size, value_type bit_val);
  constexpr bit_array(const bit_array<T, std::dynamic_extent, V, W>& other);
  constexpr bit_array(const bit_array<T, std::dynamic_extent, V, W>&& other);
  constexpr bit_array(const std::initializer_list<value_type> init)
    requires(!std::is_same_v<value_type, word_type>);
#if 0
  constexpr bit_array(const std::initializer_list<bool> init);
#endif
  constexpr bit_array(const std::initializer_list<word_type> init);
  constexpr bit_array(const std::string_view s)
    requires(std::is_same_v<value_type, bit_value>);

  ~bit_array() = default;
  /*
     * Assignment
     */
  constexpr bit_array<T, std::dynamic_extent, V, W>& operator=(const bit_array<T, std::dynamic_extent, V, W>& other);
  constexpr bit_array<T, std::dynamic_extent, V, W>& operator=(bit_array<T, std::dynamic_extent, V, W>&& other);

  constexpr bool operator==(const bit_array<T, std::dynamic_extent, V, W>& other) const noexcept;

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
  constexpr word_type* data() noexcept;
  constexpr const word_type* data() const noexcept;

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
  constexpr void swap(bit_array<T, std::dynamic_extent, V, W>& other) noexcept;
  //constexpr std::synth-three-way-result<bit_array> operator<=>() const noexcept;
};

static_assert(bit_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_sized_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_sized_range concept!");
#endif

/* Can't imagine any use for a default constructor
template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>::bit_array() noexcept
    : m_size(0), storage(std::unique_ptr<word_type[], decltype(&std::free)>(nullptr, &std::free)) {
}
*/

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>::bit_array(size_type size)
    : m_size(size),
      storage(std::unique_ptr<word_type[], decltype(&std::free)>(
          static_cast<word_type*>(std::aligned_alloc(static_cast<size_t>(V), sizeof(word_type) * Words(m_size))), &std::free)) {
  //std::uninitialized_fill_n(this->begin(), Words(m_size), word_type());
  for (std::size_t i = 0; i < Words(m_size); ++i) {
    new (storage.get() + i) word_type();
  }
}

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>::bit_array(size_type size, value_type bit_val)
    : m_size(size),
      storage(std::unique_ptr<word_type[], decltype(&std::free)>(
          static_cast<word_type*>(std::aligned_alloc(static_cast<size_t>(V), sizeof(word_type) * Words(m_size))), &std::free)) {
  if constexpr (std::is_same<value_type, word_type>::value) {
    for (std::size_t i = 0; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type(bit_val);
    }
  } else {
    for (std::size_t i = 0; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type();
    }
    fill(bit_val);
  }
}

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>::bit_array(const bit_array<T, std::dynamic_extent, V, W>& other)
    : m_size(other.size()),
      storage(std::unique_ptr<word_type[], decltype(&std::free)>(
          static_cast<word_type*>(std::aligned_alloc(static_cast<size_t>(V), sizeof(word_type) * Words(m_size))), &std::free)) {
  for (std::size_t i = 0; i < Words(m_size); ++i) {
    new (storage.get() + i) word_type(other.storage[i]);
  }
}

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>::bit_array(const bit_array<T, std::dynamic_extent, V, W>&& other)
    : m_size(other.size()),
      storage(std::unique_ptr<word_type[], decltype(&std::free)>(
          static_cast<word_type*>(std::aligned_alloc(static_cast<size_t>(V), sizeof(word_type) * Words(m_size))), &std::free)) {
  for (std::size_t i = 0; i < Words(m_size); ++i) {
    new (storage.get() + i) word_type(other.storage[i]);
  }
}

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>::bit_array(const std::initializer_list<value_type> init)
  requires(!std::is_same_v<value_type, word_type>)
    : m_size(init.size()),
      storage(std::unique_ptr<word_type[], decltype(&std::free)>(
          static_cast<word_type*>(std::aligned_alloc(static_cast<size_t>(V), sizeof(word_type) * Words(m_size))), &std::free)) {
  std::copy(init.begin(), init.end(), this->begin());
}

#if 0
No known conversion from bool to bit_value
bit_value has explicit constructor from bool to bit_value so this doesnt work
constexpr bit_array<std::dynamic_extent,W>::bit_array(const std::initializer_list<bool> init)
    : storage(std::make_unique<word_type[]>(Words(init.size()))),
      m_size(init.size()) {
  std::copy(init.begin(), init.end(), this->begin());
}
#endif

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>::bit_array(const std::initializer_list<word_type> init)
    : m_size(bitsof<word_type>() * init.size()),
      storage(std::unique_ptr<word_type[], decltype(&std::free)>(
          static_cast<word_type*>(std::aligned_alloc(static_cast<size_t>(V), sizeof(word_type) * Words(m_size))), &std::free)) {
  std::size_t i = 0;
  auto&& it = init.begin();
  for (; i < Words(m_size); ++i, ++it) {
    new (storage.get() + i) word_type(*it);
  }
}

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>::bit_array(const std::string_view s)
  requires(std::is_same_v<value_type, bit_value>)
    : m_size((std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))),
      storage(std::unique_ptr<word_type[], decltype(&std::free)>(
          static_cast<word_type*>(std::aligned_alloc(static_cast<size_t>(V), sizeof(word_type) * Words(m_size))), &std::free)) {
  size_type i = 0;
  for (char c : s) {
    if (c == '0') {
      begin()[i++] = bit0;
    } else if (c == '1') {
      begin()[i++] = bit1;
    }
  }
}

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>& bit_array<T, std::dynamic_extent, V, W>::operator=(const bit_array<T, std::dynamic_extent, V, W>& other) {
  if (nullptr == storage.get() || m_size != other.m_size) {
    throw std::invalid_argument("Cannot reassign bit_array<std::dynamic_extent,V,W> size");
  }
  std::copy(other.begin(), other.end(), this->begin());
  return *this;
}

template <typename T, std::align_val_t V, typename W>
constexpr bit_array<T, std::dynamic_extent, V, W>& bit_array<T, std::dynamic_extent, V, W>::operator=(bit_array<T, std::dynamic_extent, V, W>&& other) {
  if (nullptr == storage.get() || m_size != other.m_size) {
    throw std::invalid_argument("Cannot reassign bit_array<std::dynamic_extent,V,W> size");
  }
  std::copy(other.begin(), other.end(), this->begin());
  return *this;
}

template <typename T, std::align_val_t V, typename W>
constexpr bool bit_array<T, std::dynamic_extent, V, W>::operator==(const bit_array<T, std::dynamic_extent, V, W>& other) const noexcept {
  return equal(begin(), end(), other.begin());
}

template <typename T, std::align_val_t V, typename W>
constexpr void bit_array<T, std::dynamic_extent, V, W>::swap(bit_array<T, std::dynamic_extent, V, W>& other) noexcept {
  std::swap(this->storage, other.storage);
  std::swap(this->m_size, other.m_size);
}

template <typename T, std::align_val_t V, typename W>
constexpr void bit_array<T, std::dynamic_extent, V, W>::fill(value_type bit_val) noexcept {
  std::fill(this->begin(), this->end(), bit_val);
}

// -------------------------------------------------------------------------- //
/*
    * Element Access
    */
template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::reference bit_array<T, std::dynamic_extent, V, W>::operator[](size_type pos) {
  return begin()[pos];
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::const_reference bit_array<T, std::dynamic_extent, V, W>::operator[](size_type pos) const {
  return begin()[pos];
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::reference bit_array<T, std::dynamic_extent, V, W>::at(size_type pos) {
  if (pos < size()) {
    return begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::const_reference bit_array<T, std::dynamic_extent, V, W>::at(size_type pos) const {
  if (pos < size()) {
    return begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::reference bit_array<T, std::dynamic_extent, V, W>::front() {
  return begin()[0];
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::const_reference bit_array<T, std::dynamic_extent, V, W>::front() const {
  return begin()[0];
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::reference bit_array<T, std::dynamic_extent, V, W>::back() {
  return begin()[size() - 1];
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::const_reference bit_array<T, std::dynamic_extent, V, W>::back() const {
  return begin()[size() - 1];
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::word_type* bit_array<T, std::dynamic_extent, V, W>::data() noexcept {
  return size() ? storage.get() : nullptr;
}

template <typename T, std::align_val_t V, typename W>
constexpr const typename bit_array<T, std::dynamic_extent, V, W>::word_type* bit_array<T, std::dynamic_extent, V, W>::data() const noexcept {
  return size() ? storage.get() : nullptr;
}

// -------------------------------------------------------------------------- //

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::size_type bit_array<T, std::dynamic_extent, V, W>::size() const noexcept { return m_size; }

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::size_type bit_array<T, std::dynamic_extent, V, W>::max_size() const noexcept { return size(); }

template <typename T, std::align_val_t V, typename W>
constexpr bool bit_array<T, std::dynamic_extent, V, W>::empty() const noexcept { return 0 == size(); }

// Iterators
// -------------------------------------------------------------------------- //

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::iterator bit_array<T, std::dynamic_extent, V, W>::begin() noexcept {
  return iterator(storage.get());
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::iterator bit_array<T, std::dynamic_extent, V, W>::end() noexcept {
  return begin() + size();
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::const_iterator bit_array<T, std::dynamic_extent, V, W>::begin() const noexcept {
  return const_iterator(storage.get());
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::const_iterator bit_array<T, std::dynamic_extent, V, W>::end() const noexcept {
  return const_iterator(storage.get()) + size();
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::const_iterator bit_array<T, std::dynamic_extent, V, W>::cbegin() const noexcept {
  return const_iterator(storage.get());
}

template <typename T, std::align_val_t V, typename W>
constexpr typename bit_array<T, std::dynamic_extent, V, W>::const_iterator bit_array<T, std::dynamic_extent, V, W>::cend() const noexcept {
  return const_iterator(storage.get()) + size();
}

// -------------------------------------------------------------------------- //

// ========================================================================== //
}  // namespace bit

#endif  // _BIT_ARRAY_DYNAMIC_EXTENT_HPP_INCLUDED
// ========================================================================== //
