// ================================= BIT_ARRAY_REF =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array_ref.hpp
// Description: Implementation of bit_array_ref
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ARRAY_REF_HPP_INCLUDED
#define _BIT_ARRAY_REF_HPP_INCLUDED

#include <algorithm>
#include <cstddef>
#include <cstring>  // memcpy
#include <initializer_list>
#include <memory>
#include <new>
#include <span>  // std::dynamic_extent
#include <stdexcept>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_base.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_span.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

/**
 * @brief A non-owning reference to a bit array
 *
 * Similar to bit_array_dynamic_extent but does not allocate or deallocate memory.
 * The pointer and size are const class members and cannot be re-bound.
 * Assignment operations always copy content and can't rebind the pointer/size.
 *
 * @tparam T The value type (typically bit_value)
 * @tparam W The word type used for storage
 */
template <typename T = bit_value, typename W = std::uint8_t>
class bit_array_ref
    : public bit_array_base<bit_array_ref<T, W>, T, W, bit_iterator<W*>, bit_iterator<const W*>> {
 public:
  using base = bit_array_base<bit_array_ref<T, W>, T, W, bit_iterator<W*>, bit_iterator<const W*>>;
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

 private:
  const bit_pointer<word_type> m_storage;
  const size_type m_size;

 protected:
  static constexpr size_type Words(size_type N) {
    return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>();
  }

 public:
  // Constructors
  bit_array_ref() = delete;

  /**
   * @brief Constructs a non-owning reference to a bit array
   *
   * @param storage Pointer to the storage
   * @param size Number of bits
   */
  constexpr bit_array_ref(word_type* storage, size_type size)
      : m_storage(storage),
        m_size(size) {
  }

  /**
   * @brief Constructs a non-owning reference to a bit array using a bit_pointer
   *
   * @param storage bit_pointer to the storage
   * @param size Number of bits
   */
  constexpr bit_array_ref(bit_pointer<word_type> storage, size_type size)
      : m_storage(storage),
        m_size(size) {
  }

  /**
   * @brief Copy constructor
   */
  constexpr bit_array_ref(const bit_array_ref& other) = default;

  /**
   * @brief Move constructor
   */
  constexpr bit_array_ref(bit_array_ref&& other) = default;

  /**
   * @brief Assignment operator - copies content but doesn't rebind
   */
  constexpr bit_array_ref& operator=(const bit_array_ref& other) {
    if (this != &other) {
      if (m_size != other.m_size) {
        throw std::invalid_argument("Cannot assign from bit_array_ref of different size");
      }
      std::copy(other.begin(), other.end(), this->begin());
    }
    return *this;
  }

  /**
   * @brief Move assignment operator - copies content but doesn't rebind
   */
  constexpr bit_array_ref& operator=(bit_array_ref&& other) {
    if (this != &other) {
      if (m_size != other.m_size) {
        throw std::invalid_argument("Cannot assign from bit_array_ref of different size");
      }
      std::copy(other.begin(), other.end(), this->begin());
    }
    return *this;
  }

  /**
   * @brief No destructor needed as we don't own the memory
   */
  ~bit_array_ref() = default;

  /*
   * Element Access
   */
  constexpr word_type* data() noexcept {
    throw new std::invalid_argument("Not valid for this class. may be unaligned");
    return const_cast<word_type*>(m_storage.base());
  }

  constexpr const word_type* data() const noexcept {
    throw new std::invalid_argument("Not valid for this class. may be unaligned");
    return m_storage.base();
  }

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept {
    return iterator(m_storage);
  }

  constexpr iterator end() noexcept {
    return begin() + size();
  }

  constexpr const_iterator begin() const noexcept {
    return const_iterator(m_storage);
  }

  constexpr const_iterator end() const noexcept {
    return const_iterator(begin() + size());
  }

  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(begin());
  }

  constexpr const_iterator cend() const noexcept {
    return const_iterator(end());
  }

  /*
   * Capacity
   */
  constexpr size_type size() const noexcept {
    return m_size;
  }

  /*
   * Operations
   */
  constexpr void swap(bit_array_ref& other) noexcept {
    if (m_size != other.m_size) {
      throw std::invalid_argument("Cannot swap bit_array_ref of different sizes");
    }
    swap_ranges(begin(), end(), other.begin());
  }

  /**
   * @brief Slice operations - returns a bit_array_ref
   */
  constexpr auto operator()(size_type offset, size_type right) const noexcept {
    return bit_array_ref<T, W>(bit_pointer<word_type>(const_cast<word_type*>(m_storage.base()), m_storage.position()) + offset, right - offset);
  }

  constexpr auto operator()(size_type offset, size_type right) noexcept {
    return bit_array_ref<T, W>(bit_pointer<word_type>(m_storage.base(), m_storage.position()) + offset, right - offset);
  }

  /**
   * @brief Explicit conversion to integral types
   */
  template <std::integral U>
  explicit constexpr operator U() const noexcept {
    assert(size() <= bitsof<U>());
    U integral;
    bit_array_ref<> integral_ref(reinterpret_cast<uint8_t*>(&integral), bitsof<U>());
    copy(begin(), begin() + bitsof<U>(), integral_ref.begin());
    if constexpr (std::is_signed_v<U>) {
      fill(integral_ref.begin() + size(), integral_ref.end(), integral_ref[bitsof<U>() - 1]);
    } else {
      fill(integral_ref.begin() + size(), integral_ref.end(), bit0);
    }
    return integral;
  }
};

static_assert(bit_range<bit_array_ref<>>, "bit_array_ref<> does not satisfy bit_range concept!");
static_assert(bit_sized_range<bit_array_ref<>>, "bit_array_ref<> does not satisfy bit_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<bit_array_ref<>>, "bit_array_ref<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<bit_array_ref<>>, "bit_array_ref<> does not satisfy bit_contiguous_sized_range concept!");
#endif

// ========================================================================== //
}  // namespace bit

#endif  // _BIT_ARRAY_REF_HPP_INCLUDED