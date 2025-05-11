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
#include <cstring>  // memcpy
#include <initializer_list>
#include <memory>
#include <new>
#include <span>  // std::dynamic_extent
#include <stdexcept>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_array_base.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_span.hpp"
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {
namespace detail {
template <typename value_type, typename word_type>
using bit_array_it = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                               bit_iterator<word_type*>,
                                               word_type*>::type;
template <typename value_type, typename word_type>
using bit_array_cit = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                                bit_iterator<const word_type*>,
                                                const word_type*>::type;
}  // namespace detail
template <typename T, std::align_val_t V, typename W>
class bit_array<T, std::dynamic_extent, V, W>
    : public bit_array_base<bit_array<T, std::dynamic_extent, V, W>, T, W, detail::bit_array_it<T, W>, detail::bit_array_cit<T, W>> {
 public:
  using base = bit_array_base<bit_array<T, std::dynamic_extent, V, W>, T, W, detail::bit_array_it<T, W>, detail::bit_array_cit<T, W>>;
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
  struct deleter {
    size_type words;
    void operator()(word_type* const p) const {
      for (size_type i = 0; i < words; ++i) {
        (p + i)->~word_type();
      }
      ::operator delete(p, V);
    }
  };

  const size_type m_size;
  const std::unique_ptr<word_type[], deleter> storage;

  static constexpr size_type Words(size_type N) {
    return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>();
  };

  static constexpr size_t AlignedBytes(size_t N) {
    return (Words(N) * sizeof(word_type) + static_cast<size_t>(V) - 1) & ~(static_cast<size_t>(V) - 1);
  };

 public:
  /*
  * Constructors, copies and moves...
  */
  bit_array() = delete;

  constexpr bit_array(const size_type size)
      : m_size(size),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    //std::uninitialized_fill_n(this->begin(), Words(m_size), word_type());
    for (size_type i = 0; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type();
    }
  }

  template <std::integral U>
  constexpr bit_array(const size_type size, const U& integral)
      : m_size(size),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    assert(bitsof<U>() <= size);
    for (size_type i = 0; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type();
    }
    std::memcpy(storage.get(), &integral, sizeof(integral));
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

  constexpr bit_array(const size_type size, const value_type bit_val)
      : m_size(size),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    if constexpr (std::is_same<value_type, word_type>::value) {
      for (size_type i = 0; i < Words(m_size); ++i) {
        new (storage.get() + i) word_type(bit_val);
      }
    } else {
      for (size_type i = 0; i < Words(m_size); ++i) {
        new (storage.get() + i) word_type();
      }
      this->fill(bit_val);
    }
  }

  constexpr bit_array(const bit_array<T, std::dynamic_extent, V, W>& other)
      : m_size(other.size()),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    for (size_type i = 0; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type(*(other.storage.get() + i));
    }
  }

  constexpr bit_array(const bit_sized_range auto& other)
      : m_size(other.size()),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    ::bit::copy(other.begin(), other.end(), this->begin());
  }

  constexpr bit_array(const bit_array<T, std::dynamic_extent, V, W>&& other)
      : m_size(other.size()),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    for (size_type i = 0; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type(*(other.storage.get() + i));
    }
  }

  constexpr bit_array(const std::initializer_list<value_type> init)
    requires(!std::is_same_v<value_type, word_type>)
      : m_size(init.size()),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    for (size_type i = 0; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type();
    }
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

  constexpr bit_array(const std::initializer_list<word_type> init)
      : m_size(bitsof<word_type>() * init.size()),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    size_type i = 0;
    auto it = init.begin();
    for (; i < Words(m_size) && it != init.end(); ++i, ++it) {
      new (storage.get() + i) word_type(*it);
    }
    // Initialize remaining words if any
    for (; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type();
    }
  }

  constexpr bit_array(const std::string_view s)
    requires(std::is_same_v<value_type, bit_value>)
      : m_size((std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))),
        storage(static_cast<word_type*>(::operator new(AlignedBytes(m_size), V)), deleter{Words(m_size)}) {
    for (size_type i = 0; i < Words(m_size); ++i) {
      new (storage.get() + i) word_type();
    }
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
  constexpr bit_array<T, std::dynamic_extent, V, W>& operator=(const bit_array<T, std::dynamic_extent, V, W>& other) {
    if (nullptr == storage.get() || m_size != other.m_size) {
      throw std::invalid_argument("Cannot reassign bit_array<std::dynamic_extent,V,W> size");
    }
    std::copy(other.begin(), other.end(), this->begin());
    return *this;
  }

  constexpr bit_array<T, std::dynamic_extent, V, W>& operator=(const bit_sized_range auto& other) {
    if (other.size() != this->size()) [[unlikely]] {
      throw std::invalid_argument("other bit_range contains an invalid number of bits for bit_array.");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
    return *this;
  };

  constexpr bit_array<T, std::dynamic_extent, V, W>& operator=(bit_array<T, std::dynamic_extent, V, W>&& other) {
    if (nullptr == storage.get() || m_size != other.m_size) {
      throw std::invalid_argument("Cannot reassign bit_array<std::dynamic_extent,V,W> size");
    }
    std::copy(other.begin(), other.end(), this->begin());
    return *this;
  }

  /*
   * Element Access
   */
  constexpr word_type* data() noexcept {
    return size() ? storage.get() : nullptr;
  }

  constexpr const word_type* data() const noexcept {
    return size() ? storage.get() : nullptr;
  }

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept {
    return iterator(storage.get());
  }

  constexpr const_iterator begin() const noexcept {
    return const_iterator(storage.get());
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
  constexpr void swap(bit_array<T, std::dynamic_extent, V, W>& other) noexcept {
    assert(this->m_size == other.m_size);
    // Cannot just swap storage as it is const.
    W* it1 = this->storage.get();
    W* it2 = other.storage.get();
    for (size_type i = 0; i < Words(this->m_size); i++, it1++, it2++) {
      std::swap(*it1, *it2);
    }
  }
};

static_assert(bit_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_sized_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<bit_array<>>, "bit_array<> does not satisfy bit_contiguous_sized_range concept!");
#endif

// ========================================================================== //
}  // namespace bit

#endif  // _BIT_ARRAY_DYNAMIC_EXTENT_HPP_INCLUDED
