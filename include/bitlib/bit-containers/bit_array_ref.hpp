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
#include "bitlib/bit_concepts.hpp"

namespace bit {

namespace detail {
template <typename word_type>
struct bit_array_ref_iterator_types {
  using iterator = bit_iterator<word_type*>;
  using const_iterator = bit_iterator<const word_type*>;
};
}  // namespace detail
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
template <typename T = bit_value, typename W = std::uintptr_t, typename Policy = policy::typical>
class bit_array_ref
    : public bit_array_base<bit_array_ref<T, W>, T, std::dynamic_extent, W, Policy, detail::bit_array_ref_iterator_types<W>> {
 public:
  using base = bit_array_base<bit_array_ref<T, W>, T, std::dynamic_extent, W, Policy, detail::bit_array_ref_iterator_types<W>>;
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

 private:
  const bit_pointer<word_type> m_storage;
  const size_type m_size;

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
   * @brief Constructs a non-owning reference to a bit array from a bit_sized_range
   *
   * @param other bit_sized_range
   */
  constexpr bit_array_ref(bit_range auto& other, size_type size)
      : m_storage(&(*other.begin())),
        m_size(size) {
    assert(size <= (other.end() - other.begin()));
  }

  /**
   * @brief Constructs a non-owning reference to a bit array from a bit_sized_range
   *
   * @param other bit_sized_range
   */
  constexpr bit_array_ref(const bit_range auto& other, size_type size)
    requires(std::is_const_v<W>)
      : m_storage(&(*other.begin())),
        m_size(size) {
    assert(size <= (other.end() - other.begin()));
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
   * @brief Range Assignment operator - copies content but doesn't rebind
   */
  constexpr bit_array_ref& operator=(const bit_sized_range auto& other) {
    if (m_size != other.size()) {
      throw std::invalid_argument("Cannot assign from bit_array_ref of different size");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
    return *this;
  }

  /**
   * @brief Copy Assignment operator - copies content but doesn't rebind
   */
  constexpr bit_array_ref& operator=(const bit_array_ref& other) {
    if (this != &other) {
      if (m_size != other.m_size) {
        throw std::invalid_argument("Cannot assign from bit_array_ref of different size");
      }
      ::bit::copy(other.begin(), other.end(), this->begin());
    }
    return *this;
  }

  /**
   * @brief Move assignment operator - copies content but doesn't rebind
   */
  constexpr bit_array_ref& operator=(bit_array_ref&& other) {
    if (this != &other) {
      if (m_size != other.size()) {
        throw std::invalid_argument("Cannot assign from bit_array_ref of different size");
      }
      ::bit::copy(other.begin(), other.end(), this->begin());
    }
    return *this;
  }

  /**
   * @brief No destructor needed as we don't own the memory
   */
  ~bit_array_ref() = default;

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept {
    return iterator(m_storage);
  }

  constexpr const_iterator begin() const noexcept {
    return const_iterator(m_storage);
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
  constexpr void swap(bit_array_ref& other) {
    if (m_size != other.m_size) {
      throw std::invalid_argument("Cannot swap bit_array_ref of different sizes");
    }
    swap_ranges(begin(), end(), other.begin());
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