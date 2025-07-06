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
#include <initializer_list>
#include <memory>
#include <span>  // std::dynamic_extent
#include <stdexcept>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_base.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

template <typename T = bit_value, size_t N = std::dynamic_extent, typename W = std::uintptr_t, typename Policy = policy::typical<std::remove_cvref_t<W>>>
class array_ref;

template <size_t N = std::dynamic_extent, typename W = std::uintptr_t, typename Policy = policy::typical<std::remove_cvref_t<W>>>
using bit_array_ref = array_ref<bit_value, N, W, Policy>;

namespace detail {
template <typename word_type>
struct array_ref_iterator_types {
  using iterator = bit_iterator<word_type*>;
  using const_iterator = bit_iterator<const word_type*>;
};
}  // namespace detail
/**
 * @brief A non-owning reference to a bit array
 *
 * Similar to array_dynamic_extent but does not allocate or deallocate memory.
 * The pointer and size are const class members and cannot be re-bound.
 * Assignment operations always copy content and can't rebind the pointer/size.
 *
 * @tparam T The value type (typically bit_value)
 * @tparam W The word type used for storage
 */
template <typename T, size_t N, typename W, typename Policy>
class array_ref
    : public array_base<array_ref<T, N, W, Policy>, T, N, W, false, Policy, detail::array_ref_iterator_types<W>> {
 public:
  using base = array_base<array_ref<T, N, W, Policy>, T, N, W, false, Policy, detail::array_ref_iterator_types<W>>;
  using base::end;
  using base::size;
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

 public:
  // Constructors
  array_ref() = delete;

  /**
   * @brief Constructs a non-owning reference to a bit array
   *
   * @param storage Pointer to the storage
   * @param extent Number of bits
   */
  constexpr array_ref(word_type* storage, size_type extent)
    requires(N == std::dynamic_extent)
      : base(extent), m_storage(storage) {
  }
  constexpr array_ref(word_type* storage)
    requires(N != std::dynamic_extent)
      : base(), m_storage(storage) {
  }

  /**
   * @brief Constructs a non-owning reference to a bit array using a bit_pointer
   *
   * @param storage bit_pointer to the storage
   * @param extent Number of bits
   */
  constexpr array_ref(bit_pointer<word_type> storage, size_type extent)
    requires(N == std::dynamic_extent)
      : base(extent), m_storage(storage) {
  }
  constexpr array_ref(bit_pointer<word_type> storage)
    requires(N != std::dynamic_extent)
      : base(), m_storage(storage) {
  }

  /**
   * @brief Constructs a non-owning reference to a bit array from a bit_sized_range
   *
   * @param other bit_sized_range
   */
  constexpr array_ref(bit_range auto& other, size_type extent)
    requires(N == std::dynamic_extent)
      : base(extent), m_storage(&(*other.begin())) {
    assert(extent <= (other.end() - other.begin()));
  }
  constexpr array_ref(bit_range auto& other)
    requires(N != std::dynamic_extent)
      : base(), m_storage(&(*other.begin())) {
    assert(N <= (other.end() - other.begin()));
  }

  /**
   * @brief Constructs a non-owning reference to a bit array from a bit_sized_range
   *
   * @param other bit_sized_range
   */
  constexpr array_ref(const bit_range auto& other, size_type extent)
    requires((N == std::dynamic_extent) && std::is_const_v<W>)
      : base(extent), m_storage(&(*other.begin())) {
    assert(extent <= (other.end() - other.begin()));
  }
  constexpr array_ref(const bit_range auto& other)
    requires((N != std::dynamic_extent) && std::is_const_v<W>)
      : base(), m_storage(&(*other.begin())) {
    assert(N <= (other.end() - other.begin()));
  }

  /**
   * @brief Copy constructor
   */
  constexpr array_ref(const array_ref& other) = default;

  /**
   * @brief Move constructor
   */
  constexpr array_ref(array_ref&& other) = default;

  /**
   * @brief Range Assignment operator - copies content but doesn't rebind
   */
  constexpr array_ref& operator=(const bit_sized_range auto& other) {
    if (size() != other.size()) {
      throw std::invalid_argument("Cannot assign from array_ref of different size");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
    return *this;
  }

  /**
   * @brief Copy Assignment operator - copies content but doesn't rebind
   */
  constexpr array_ref& operator=(const array_ref& other) {
    if (this != &other) {
      if (size() != other.size()) {
        throw std::invalid_argument("Cannot assign from array_ref of different size");
      }
      ::bit::copy(other.begin(), other.end(), this->begin());
    }
    return *this;
  }

  /**
   * @brief Move assignment operator - copies content but doesn't rebind
   */
  constexpr array_ref& operator=(array_ref&& other) {
    if (this != &other) {
      if (size() != other.size()) {
        throw std::invalid_argument("Cannot assign from array_ref of different size");
      }
      ::bit::copy(other.begin(), other.end(), this->begin());
    }
    return *this;
  }

  /**
   * @brief No destructor needed as we don't own the memory
   */
  ~array_ref() = default;

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
   * Operations
   */
  constexpr void swap(array_ref& other) {
    if (size() != other.size()) {
      throw std::invalid_argument("Cannot swap array_ref of different sizes");
    }
    swap_ranges(begin(), end(), other.begin());
  }
};

static_assert(bit_range<array_ref<>>, "array_ref<> does not satisfy bit_range concept!");
static_assert(bit_sized_range<array_ref<>>, "array_ref<> does not satisfy bit_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<array_ref<>>, "array_ref<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<array_ref<>>, "array_ref<> does not satisfy bit_contiguous_sized_range concept!");
#endif

// ========================================================================== //
}  // namespace bit

#endif  // _BIT_ARRAY_REF_HPP_INCLUDED