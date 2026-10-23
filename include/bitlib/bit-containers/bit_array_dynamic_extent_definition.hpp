#ifndef _BIT_ARRAY_DYNAMIC_EXTENT_DEFINITION_HPP_INCLUDED
#define _BIT_ARRAY_DYNAMIC_EXTENT_DEFINITION_HPP_INCLUDED

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_base_definition.hpp"
#include "bitlib/bit-containers/bit_array_dynamic_extent_declaration.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

template <typename T, typename W, typename Policy>
array<T, std::dynamic_extent, W, Policy>::Storage::Storage(
    size_type words,
    const Allocator& allocator,
    detail::uninitialized_t)
    : m_allocator(allocator) {
  if (words > FixedWords) {
    new (&pointer) word_type_ptr(m_allocator.allocate(words));
  }
}

template <typename T, typename W, typename Policy>
array<T, std::dynamic_extent, W, Policy>::Storage::Storage(
    size_type words,
    const Allocator& allocator)
    : m_allocator(allocator) {
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

template <typename T, typename W, typename Policy>
array<T, std::dynamic_extent, W, Policy>::Storage::Storage(size_type words, Storage&& other) {
  if (words > FixedWords) {
    new (&pointer) word_type_ptr(std::move(other.pointer));
    other.pointer = nullptr;  // Prevent double deletion
  } else {
    for (size_type i = 0; i < words; ++i) {
      new (&fixed[i]) word_type(std::move(other.fixed[i]));
    }
  }
}

// Qualify Storage: MSVC does not find the injected-class-name in this
// parameter list.
template <typename T, typename W, typename Policy>
array<T, std::dynamic_extent, W, Policy>::Storage::Storage(
    size_type words,
    const typename array<T, std::dynamic_extent, W, Policy>::Storage& other,
    const Allocator& allocator)
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

template <typename T, typename W, typename Policy>
template <typename U>
array<T, std::dynamic_extent, W, Policy>::Storage::Storage(
    size_type words,
    const U& val,
    const Allocator& allocator)
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

template <typename T, typename W, typename Policy>
array<T, std::dynamic_extent, W, Policy>::~array() {
  if (size() > FixedBits) {
    storage.m_allocator.deallocate(storage.pointer, Words(size()));
  } else if constexpr (!std::is_fundamental_v<T>) {
    for (size_type i = 0; i < Words(size()); ++i) {
      // W is 'word_type', but MSVC is unhappy with using an alias
      storage.fixed[i].~W();
    }
  }
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(const size_type extent, const Allocator& allocator)
    : base(extent), storage(Words(extent), allocator) {
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    detail::uninitialized_t,
    const size_type extent,
    const Allocator& allocator)
    : base(extent), storage(Words(extent), allocator, detail::uninitialized) {
}

template <typename T, typename W, typename Policy>
template <std::integral U>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    const size_type extent,
    const U& integral,
    const Allocator& allocator)
    : base(extent), storage(Words(extent), allocator, detail::uninitialized) {
  this->from_integral(integral);
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    const size_type extent,
    const word_type val,
    const Allocator& allocator)
    : base(extent), storage(Words(extent), val, allocator) {
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    const size_type extent,
    const value_type bit_val,
    const Allocator& allocator)
  requires(!std::is_same<value_type, word_type>::value)
    : base(extent), storage(Words(extent), allocator, detail::uninitialized) {
  this->fill(bit_val);
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(const array<T, std::dynamic_extent, W, Policy>& other)
    : base(other.size()), storage(Words(size()), other.storage) {
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    const array<T, std::dynamic_extent, W, Policy>& other,
    const Allocator& allocator)
    : base(other.size()), storage(Words(size()), other.storage, allocator) {
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    const bit_sized_range auto& other,
    const Allocator& allocator)
    : base(other.size()), storage(Words(size()), allocator, detail::uninitialized) {
  ::bit::copy(other.begin(), other.end(), this->begin());
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(array<T, std::dynamic_extent, W, Policy>&& other)
    : base(other.size()), storage(Words(size()), std::move(other.storage)) {
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    array<T, std::dynamic_extent, W, Policy>&& other,
    const Allocator& allocator)
    : base(other.size()), storage(Words(size()), std::move(other.storage), allocator) {
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    const std::initializer_list<value_type> init,
    const Allocator& allocator)
  requires(!std::is_same_v<value_type, word_type>)
    : base(init.size()), storage(Words(size()), allocator, detail::uninitialized) {
  std::copy(init.begin(), init.end(), this->begin());
}

template <typename T, typename W, typename Policy>
template <typename U>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    const std::initializer_list<U> init,
    const Allocator& allocator)
    : base(bitsof<U>() * init.size()), storage(Words(size()), allocator, detail::uninitialized) {
  std::copy(init.begin(), init.end(), data());
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>::array(
    const std::string_view s,
    const Allocator& allocator)
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

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>&
array<T, std::dynamic_extent, W, Policy>::operator=(const array<T, std::dynamic_extent, W, Policy>& other) {
  if (nullptr == data() || size() != other.size()) {
    throw std::invalid_argument("Cannot reassign array<std::dynamic_extent,V,W,Policy> extent");
  }
  if (this == &other) [[unlikely]] {
    return *this;
  }
  std::copy(other.begin(), other.end(), this->begin());
  return *this;
}

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>&
array<T, std::dynamic_extent, W, Policy>::operator=(const bit_sized_range auto& other) {
  if (other.size() != this->size()) [[unlikely]] {
    throw std::invalid_argument("other bit_range contains an invalid number of bits for array.");
  }
  ::bit::copy(other.begin(), other.end(), this->begin());
  return *this;
};

template <typename T, typename W, typename Policy>
constexpr array<T, std::dynamic_extent, W, Policy>&
array<T, std::dynamic_extent, W, Policy>::operator=(array<T, std::dynamic_extent, W, Policy>&& other) {
  if (nullptr == data() || size() != other.size()) {
    throw std::invalid_argument("Cannot reassign array<std::dynamic_extent,V,W,Policy> extent");
  }
  array<T, std::dynamic_extent, W, Policy> temp(std::move(other));
  swap(temp);
  return *this;
}

template <typename T, typename W, typename Policy>
constexpr typename array<T, std::dynamic_extent, W, Policy>::word_type*
array<T, std::dynamic_extent, W, Policy>::data() noexcept {
  if (size() > FixedBits) {
    return storage.pointer;
  } else {
    return storage.fixed;
  }
}

template <typename T, typename W, typename Policy>
constexpr const typename array<T, std::dynamic_extent, W, Policy>::word_type*
array<T, std::dynamic_extent, W, Policy>::data() const noexcept {
  if (size() > FixedBits) {
    return storage.pointer;
  } else {
    return storage.fixed;
  }
}

template <typename T, typename W, typename Policy>
constexpr typename array<T, std::dynamic_extent, W, Policy>::iterator
array<T, std::dynamic_extent, W, Policy>::begin() noexcept {
  if (size() > FixedBits) {
    return iterator(storage.pointer);
  } else {
    return iterator(storage.fixed);
  }
}

template <typename T, typename W, typename Policy>
constexpr typename array<T, std::dynamic_extent, W, Policy>::const_iterator
array<T, std::dynamic_extent, W, Policy>::begin() const noexcept {
  if (size() > FixedBits) {
    return const_iterator(storage.pointer);
  } else {
    return const_iterator(storage.fixed);
  }
}

template <typename T, typename W, typename Policy>
constexpr void array<T, std::dynamic_extent, W, Policy>::swap(array<T, std::dynamic_extent, W, Policy>& other) noexcept {
  assert(size() == other.size());
  if (size() > FixedBits) {
    std::swap(this->storage.pointer, other.storage.pointer);
  } else {
    for (size_type i = 0; i < Words(size()); ++i) {
      std::swap(this->storage.fixed[i], other.storage.fixed[i]);
    }
  }
}

}  // namespace bit

#endif  // _BIT_ARRAY_DYNAMIC_EXTENT_DEFINITION_HPP_INCLUDED
