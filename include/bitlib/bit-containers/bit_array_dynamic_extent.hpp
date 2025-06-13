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
template <typename T, typename W>
class bit_array<T, std::dynamic_extent, W>
    : public bit_array_base<bit_array<T, std::dynamic_extent, W>, T, std::dynamic_extent, W, detail::bit_array_it<T, W>, detail::bit_array_cit<T, W>> {
 public:
  using base = bit_array_base<bit_array<T, std::dynamic_extent, W>, T, std::dynamic_extent, W, detail::bit_array_it<T, W>, detail::bit_array_cit<T, W>>;
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
  using word_type_ptr = std::unique_ptr<word_type[]>;
  static const size_type FixedWords = sizeof(word_type_ptr) / sizeof(word_type);
  static const size_type FixedBits = FixedWords * bitsof<word_type>();

  const size_type m_size;

  union Storage {
    word_type_ptr pointer;
    word_type fixed[FixedWords];

    Storage(size_type words) {
      if (words > FixedWords) {
        new (&pointer) word_type_ptr(new word_type[words]);
      } else {
        for (size_type i = 0; i < words; ++i) {
          new (&fixed[i]) word_type();
        }
      }
    }

    Storage(size_type words, Storage&& other) {
      if (words > FixedWords) {
        new (&pointer) word_type_ptr(std::move(other.pointer));
        other.pointer = nullptr;  // Prevent double deletion
      } else {
        for (size_type i = 0; i < words; ++i) {
          new (&fixed[i]) word_type(std::move(other.fixed[i]));
        }
      }
    }

    Storage(size_type words, const Storage& other) {
      if (words > FixedWords) {
        new (&pointer) word_type_ptr(new word_type[words]);
        std::memcpy(pointer.get(), other.pointer.get(), words * sizeof(word_type));
      } else {
        for (size_type i = 0; i < words; ++i) {
          new (&fixed[i]) word_type(other.fixed[i]);
        }
      }
    }
    template <typename U>
    Storage(size_type words, const U& val) {
      if (words > FixedWords) {
        new (&pointer) word_type_ptr(new word_type[words](val));
      } else {
        for (size_type i = 0; i < words; ++i) {
          new (&fixed[i]) word_type(val);
        }
      }
    }
    Storage() = delete;
    ~Storage() {}
  } storage;

  static constexpr size_type Words(size_type N) {
    return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>();
  };
  /*
  constexpr size_type Words() const {
    return Words(m_size);
  }*/

 public:
  ~bit_array() {
    if (size() > FixedBits) {
      storage.pointer.~unique_ptr();
    } else {
      for (size_type i = 0; i < Words(size()); ++i) {
        storage.fixed[i].~word_type();
      }
    }
  }

  /*
  * Constructors, copies and moves...
  */
  bit_array() = delete;

  constexpr bit_array(const size_type size)
      : m_size(size), storage(Words(size)) {
  }

  template <std::integral U>
  constexpr bit_array(const size_type size, const U& integral)
      : bit_array(size) {
    assert(bitsof<U>() <= size);
    if (size() > FixedBits) {
      std::memcpy(storage.pointer, &integral, sizeof(integral));
    } else {
      std::memcpy(storage.fixed, &integral, sizeof(integral));
    }
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

  constexpr bit_array(const size_type size, const word_type val)
      : m_size(size), storage(Words(size), val) {
  }

  constexpr bit_array(const size_type size, const value_type bit_val)
    requires(!std::is_same<value_type, word_type>::value)
      : m_size(size), storage(Words(size)) {
    this->fill(bit_val);
  }

  constexpr bit_array(const bit_array<T, std::dynamic_extent, W>& other)
      : m_size(other.size()), storage(Words(size()), other.storage) {
  }

  constexpr bit_array(const bit_sized_range auto& other)
      : bit_array(other.size()) {
    ::bit::copy(other.begin(), other.end(), this->begin());
  }

  constexpr bit_array(bit_array<T, std::dynamic_extent, W>&& other)
      : m_size(other.size()), storage(Words(size()), std::move(other.storage)) {
  }

  constexpr bit_array(const std::initializer_list<value_type> init)
    requires(!std::is_same_v<value_type, word_type>)
      : bit_array(init.size()) {
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

  template <typename U>
  constexpr bit_array(const std::initializer_list<U> init)
      : bit_array(bitsof<U>() * init.size()) {
    std::copy(init.begin(), init.end(), data());
  }

  constexpr bit_array(const std::string_view s)
    requires(std::is_same_v<value_type, bit_value>)
      : bit_array((std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))) {
    size_type i = 0;
    for (char c : s) {
      if (c == '0') {
        begin()[i++] = bit0;
      } else if (c == '1') {
        begin()[i++] = bit1;
      }
    }
  }

  /*
   * Assignment
   */
  constexpr bit_array<T, std::dynamic_extent, W>& operator=(const bit_array<T, std::dynamic_extent, W>& other) {
    if (nullptr == data() || size() != other.size()) {
      throw std::invalid_argument("Cannot reassign bit_array<std::dynamic_extent,V,W> size");
    }
    if (this == &other) [[unlikely]] {
      return *this;
    }
    std::copy(other.begin(), other.end(), this->begin());
    return *this;
  }

  constexpr bit_array<T, std::dynamic_extent, W>& operator=(const bit_sized_range auto& other) {
    if (other.size() != this->size()) [[unlikely]] {
      throw std::invalid_argument("other bit_range contains an invalid number of bits for bit_array.");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
    return *this;
  };

  constexpr bit_array<T, std::dynamic_extent, W>& operator=(bit_array<T, std::dynamic_extent, W>&& other) {
    if (nullptr == data() || size() != other.size()) {
      throw std::invalid_argument("Cannot reassign bit_array<std::dynamic_extent,V,W> size");
    }
    bit_array<T, std::dynamic_extent, W> temp(std::move(other));
    swap(temp);
    return *this;
  }

  /*
   * Element Access
   */
  constexpr word_type* data() noexcept {
    if (size() > FixedBits) {
      return storage.pointer.get();
    } else {
      return storage.fixed;
    }
  }

  constexpr const word_type* data() const noexcept {
    if (size() > FixedBits) {
      return storage.pointer.get();
    } else {
      return storage.fixed;
    }
  }

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept {
    if (size() > FixedBits) {
      return iterator(storage.pointer.get());
    } else {
      return iterator(storage.fixed);
    }
  }

  constexpr const_iterator begin() const noexcept {
    if (size() > FixedBits) {
      return const_iterator(storage.pointer.get());
    } else {
      return const_iterator(storage.fixed);
    }
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
  constexpr void swap(bit_array<T, std::dynamic_extent, W>& other) noexcept {
    assert(size() == other.size());
    if (size() > FixedBits) {
      std::swap(this->storage.pointer, other.storage.pointer);
    } else {
      for (size_type i = 0; i < Words(size()); ++i) {
        std::swap(this->storage.fixed[i], other.storage.fixed[i]);
      }
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
