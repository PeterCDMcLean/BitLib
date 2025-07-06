// ================================= BIT_ARRAY =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array_dynamic_extent.hpp
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
#include <new>
#include <span>  // std::dynamic_extent
#include <stdexcept>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_array_base.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {
namespace detail {

template <typename value_type, typename word_type>
struct array_dextent_iterator_types {
  using iterator = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                             bit_iterator<word_type*>,
                                             word_type*>::type;
  using const_iterator = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                                   bit_iterator<const word_type*>,
                                                   const word_type*>::type;
};
}  // namespace detail
template <typename T, typename W, typename Policy>
class array<T, std::dynamic_extent, W, Policy>
    : public array_base<array<T, std::dynamic_extent, W, Policy>, T, std::dynamic_extent, W, false, Policy, detail::array_dextent_iterator_types<T, W>> {
 public:
  using base = array_base<array<T, std::dynamic_extent, W, Policy>, T, std::dynamic_extent, W, false, Policy, detail::array_dextent_iterator_types<T, W>>;
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
  using Allocator = typename Policy::allocator;

 private:
  using word_type_ptr = word_type*;
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wsizeof-pointer-div"
  static const size_type FixedWords = sizeof(word_type_ptr) / sizeof(word_type);
  #pragma GCC diagnostic pop
  static const size_type FixedBits = FixedWords * bitsof<word_type>();

  struct Storage {
    union {
      word_type_ptr pointer;
      word_type fixed[FixedWords];
    };
    Allocator m_allocator;

    Storage(size_type words, const Allocator& allocator, detail::uninitialized_t) : m_allocator(allocator) {
      if (words > FixedWords) {
        new (&pointer) word_type_ptr(m_allocator.allocate(words));
      }
    }

    Storage(size_type words, const Allocator& allocator = Allocator()) : m_allocator(allocator) {
      if (words > FixedWords) {
        new (&pointer) word_type_ptr(m_allocator.allocate(words));
        for (size_type i = 0; i < words; ++i) {
          new (pointer + i) word_type();
        }
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

    Storage(size_type words, const Storage& other, const Allocator& allocator = Allocator())
        : m_allocator(allocator) {
      if (words > FixedWords) {
        new (&pointer) word_type_ptr(m_allocator.allocate(words));
        for (size_t i = 0; i < words; ++i) {
          new (pointer + i) word_type(other.pointer[i]);
        }
        std::copy_n(other.pointer, words, pointer);
      } else {
        for (size_type i = 0; i < words; ++i) {
          new (&fixed[i]) word_type(other.fixed[i]);
        }
      }
    }
    template <typename U>
    Storage(size_type words, const U& val, const Allocator& allocator = Allocator())
        : m_allocator(allocator) {
      if (words > FixedWords) {
        new (&pointer) word_type_ptr(m_allocator.allocate(words));
        for (size_t i = 0; i < words; ++i) {
          new (pointer + i) word_type(val);
        }
      } else {
        for (size_type i = 0; i < words; ++i) {
          new (&fixed[i]) word_type(val);
        }
      }
    }
    Storage() = delete;
    ~Storage() {};
  } storage;

  static constexpr size_type Words(size_type N) {
    return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>();
  };

 public:
  ~array() {
    if (size() > FixedBits) {
      storage.m_allocator.deallocate(storage.pointer, Words(size()));
    } else {
      for (size_type i = 0; i < Words(size()); ++i) {
        storage.fixed[i].~word_type();
      }
    }
  }

  /*
  * Constructors, copies and moves...
  */
  array() = delete;

  constexpr array(const size_type size, const Allocator& allocator = Allocator())
      : base(size), storage(Words(size), allocator) {
  }

  template <std::integral U>
  constexpr array(const size_type size, const U& integral, const Allocator& allocator = Allocator())
      : base(size), storage(Words(size), allocator, detail::uninitialized) {
    this->from_integral(integral);
  }

  constexpr array(const size_type size, const word_type val, const Allocator& allocator = Allocator())
      : base(size), storage(Words(size), val, allocator) {
  }

  constexpr array(const size_type size, const value_type bit_val, const Allocator& allocator = Allocator())
    requires(!std::is_same<value_type, word_type>::value)
      : base(size), storage(Words(size), allocator, detail::uninitialized) {
    this->fill(bit_val);
  }

  constexpr array(const array<T, std::dynamic_extent, W, Policy>& other)
      : base(other.size()), storage(Words(size()), other.storage) {
  }

  constexpr array(const array<T, std::dynamic_extent, W, Policy>& other, const Allocator& allocator)
      : base(other.size()), storage(Words(size()), other.storage, allocator) {
  }

  constexpr array(const bit_sized_range auto& other, const Allocator& allocator = Allocator())
      : base(other.size()), storage(Words(size()), allocator, detail::uninitialized) {
    ::bit::copy(other.begin(), other.end(), this->begin());
  }

  constexpr array(array<T, std::dynamic_extent, W, Policy>&& other)
      : base(other.size()), storage(Words(size()), std::move(other.storage)) {
  }

  constexpr array(array<T, std::dynamic_extent, W, Policy>&& other, const Allocator& allocator)
      : base(other.size()), storage(Words(size()), std::move(other.storage), allocator) {
  }

  constexpr array(const std::initializer_list<value_type> init, const Allocator& allocator = Allocator())
    requires(!std::is_same_v<value_type, word_type>)
      : base(init.size()), storage(Words(size()), allocator, detail::uninitialized) {
    std::copy(init.begin(), init.end(), this->begin());
  }

#if 0
  No known conversion from bool to bit_value
  bit_value has explicit constructor from bool to bit_value so this doesnt work
  constexpr array<std::dynamic_extent, W, Policy>::array(const std::initializer_list<bool> init)
      : storage(std::make_unique<word_type[]>(Words(init.size()))),
        base(init.size()) {
    std::copy(init.begin(), init.end(), this->begin());
  }
#endif

  template <typename U>
  constexpr array(const std::initializer_list<U> init, const Allocator& allocator = Allocator())
      : base(bitsof<U>() * init.size()), storage(Words(size()), allocator, detail::uninitialized) {
    std::copy(init.begin(), init.end(), data());
  }

  constexpr array(const std::string_view s, const Allocator& allocator = Allocator())
    requires(std::is_same_v<value_type, bit_value>)
      : base(std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1')), storage(Words(size()), allocator, detail::uninitialized) {
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
  constexpr array<T, std::dynamic_extent, W, Policy>& operator=(const array<T, std::dynamic_extent, W, Policy>& other) {
    if (nullptr == data() || size() != other.size()) {
      throw std::invalid_argument("Cannot reassign array<std::dynamic_extent,V,W,Policy> size");
    }
    if (this == &other) [[unlikely]] {
      return *this;
    }
    std::copy(other.begin(), other.end(), this->begin());
    return *this;
  }

  constexpr array<T, std::dynamic_extent, W, Policy>& operator=(const bit_sized_range auto& other) {
    if (other.size() != this->size()) [[unlikely]] {
      throw std::invalid_argument("other bit_range contains an invalid number of bits for array.");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
    return *this;
  };

  constexpr array<T, std::dynamic_extent, W, Policy>& operator=(array<T, std::dynamic_extent, W, Policy>&& other) {
    if (nullptr == data() || size() != other.size()) {
      throw std::invalid_argument("Cannot reassign array<std::dynamic_extent,V,W,Policy> size");
    }
    array<T, std::dynamic_extent, W, Policy> temp(std::move(other));
    swap(temp);
    return *this;
  }

  /*
   * Element Access
   */
  constexpr word_type* data() noexcept {
    if (size() > FixedBits) {
      return storage.pointer;
    } else {
      return storage.fixed;
    }
  }

  constexpr const word_type* data() const noexcept {
    if (size() > FixedBits) {
      return storage.pointer;
    } else {
      return storage.fixed;
    }
  }

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept {
    if (size() > FixedBits) {
      return iterator(storage.pointer);
    } else {
      return iterator(storage.fixed);
    }
  }

  constexpr const_iterator begin() const noexcept {
    if (size() > FixedBits) {
      return const_iterator(storage.pointer);
    } else {
      return const_iterator(storage.fixed);
    }
  }

  /*
   * Operations
   */
  constexpr void swap(array<T, std::dynamic_extent, W, Policy>& other) noexcept {
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

static_assert(bit_range<bit_array<>>, "array<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_sized_range<bit_array<>>, "array<> does not satisfy bit_contiguous_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<array<>>, "array<> does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<array<>>, "array<> does not satisfy bit_contiguous_sized_range concept!");
#endif

// ========================================================================== //
}  // namespace bit

#endif  // _BIT_ARRAY_DYNAMIC_EXTENT_HPP_INCLUDED
