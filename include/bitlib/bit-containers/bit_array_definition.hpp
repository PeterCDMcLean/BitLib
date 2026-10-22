#ifndef _BIT_ARRAY_DEFINITION_HPP_INCLUDED
#define _BIT_ARRAY_DEFINITION_HPP_INCLUDED

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_declaration.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array() noexcept : storage{} {}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(value_type bit_val) {
  this->fill(bit_val);
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(detail::uninitialized_t, const size_t& size) {
  assert(size == N);
}

template <typename T, std::size_t N, typename W, typename Policy>
template <std::integral U>
constexpr array<T, N, W, Policy>::array(const U& integral) {
  this->from_integral(integral);
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(const array<T, N, W, Policy>& other) noexcept
    : base(), storage(other.storage) {}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(const array<T, N, W, Policy>&& other) noexcept
    : base(), storage(other.storage) {}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(const bit_sized_range auto& other) : base() {
  if (other.size() != this->size()) [[unlikely]] {
    throw std::invalid_argument("other bit_range contains an invalid number of bits for array.");
  }
  ::bit::copy(other.begin(), other.end(), this->begin());
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(const std::initializer_list<value_type> init)
  requires(!std::is_same_v<value_type, word_type>)
    : base() {
  if (init.size() != bitsof(*this)) [[unlikely]] {
    throw std::invalid_argument("initialize_list contains an invalid number of bits for array.");
  }
  std::copy(init.begin(), init.end(), this->begin());
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(const std::initializer_list<bool> init) : base() {
  if (init.size() != bitsof(*this)) [[unlikely]] {
    throw std::invalid_argument("initialize_list contains an invalid number of bits for array.");
  }
  std::copy(init.begin(), init.end(), this->begin());
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(const std::initializer_list<word_type> init) : base(), storage{} {
  // Make sure we handle the case where init.size() != Words
  auto it = init.begin();
  for (size_type i = 0; i < std::min(Words(N), init.size()); ++i, ++it) {
    storage[i] = *it;
  }
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::array(const std::string_view s)
  requires(std::is_same_v<value_type, bit_value>)
    : base() {
  if (bitsof(*this) != static_cast<size_t>(std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))) [[unlikely]] {
    throw std::invalid_argument("String contains an invalid number of bits for array.");
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

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>& array<T, N, W, Policy>::operator=(const bit_sized_range auto& other) {
  if (other.size() != this->size()) [[unlikely]] {
    throw std::invalid_argument("other bit_sized_range contains an invalid number of bits for array.");
  }
  ::bit::copy(other.begin(), other.end(), this->begin());
  return *this;
};

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>& array<T, N, W, Policy>::operator=(array<T, N, W, Policy>&& other) noexcept {
  std::copy(other.storage.begin(), other.storage.end(), storage.begin());
  return *this;
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr typename array<T, N, W, Policy>::word_type* array<T, N, W, Policy>::data() noexcept {
  return size() ? storage.data() : nullptr;
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr const typename array<T, N, W, Policy>::word_type* array<T, N, W, Policy>::data() const noexcept {
  return size() ? storage.data() : nullptr;
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr typename array<T, N, W, Policy>::iterator array<T, N, W, Policy>::begin() noexcept {
  return iterator(storage.begin());
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr typename array<T, N, W, Policy>::const_iterator array<T, N, W, Policy>::begin() const noexcept {
  return const_iterator(storage.begin());
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr typename array<T, N, W, Policy>::size_type array<T, N, W, Policy>::size() const noexcept {
  return N;
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr void array<T, N, W, Policy>::swap(array<T, N, W, Policy>& other) noexcept {
  std::swap(this->storage, other.storage);
}

template <typename T, std::size_t N, typename W, typename Policy>
constexpr array<T, N, W, Policy>::operator bit_value() const noexcept
  requires(std::is_same_v<value_type, bit_value> && N == 1)
{
  return this->begin()[0];
}

}  // namespace bit

#endif  // _BIT_ARRAY_DEFINITION_HPP_INCLUDED
