#ifndef _BIT_ARRAY_REF_DEFINITION_HPP_INCLUDED
#define _BIT_ARRAY_REF_DEFINITION_HPP_INCLUDED

#include <cassert>
#include <stdexcept>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_base_definition.hpp"
#include "bitlib/bit-containers/bit_array_ref_declaration.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>::array_ref(word_type* storage, size_type extent)
  requires(N == std::dynamic_extent)
    : base(extent), m_storage(storage) {
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>::array_ref(word_type* storage)
  requires(N != std::dynamic_extent)
    : base(), m_storage(storage) {
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>::array_ref(bit_pointer<word_type> storage, size_type extent)
  requires(N == std::dynamic_extent)
    : base(extent), m_storage(storage) {
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>::array_ref(bit_pointer<word_type> storage)
  requires(N != std::dynamic_extent)
    : base(), m_storage(storage) {
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>::array_ref(bit_range auto& other, size_type extent)
  requires(N == std::dynamic_extent)
    : base(extent), m_storage(&(*other.begin())) {
  assert(extent <= static_cast<size_type>(other.end() - other.begin()));
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>::array_ref(bit_range auto& other)
  requires(N != std::dynamic_extent)
    : base(), m_storage(&(*other.begin())) {
  assert(N <= static_cast<size_type>(other.end() - other.begin()));
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>::array_ref(const bit_range auto& other, size_type extent)
  requires((N == std::dynamic_extent) && std::is_const_v<W>)
    : base(extent), m_storage(&(*other.begin())) {
  assert(extent <= (other.end() - other.begin()));
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>::array_ref(const bit_range auto& other)
  requires((N != std::dynamic_extent) && std::is_const_v<W>)
    : base(), m_storage(&(*other.begin())) {
  assert(N <= (other.end() - other.begin()));
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>&
array_ref<T, N, W, A, Policy>::operator=(const bit_sized_range auto& other) {
  if (size() != other.size()) {
    throw std::invalid_argument("Cannot assign from array_ref of different size");
  }
  ::bit::copy(other.begin(), other.end(), this->begin());
  return *this;
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>&
array_ref<T, N, W, A, Policy>::operator=(const array_ref& other) {
  if (this != &other) {
    if (size() != other.size()) {
      throw std::invalid_argument("Cannot assign from array_ref of different size");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
  }
  return *this;
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr array_ref<T, N, W, A, Policy>&
array_ref<T, N, W, A, Policy>::operator=(array_ref&& other) {
  if (this != &other) {
    if (size() != other.size()) {
      throw std::invalid_argument("Cannot assign from array_ref of different size");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
  }
  return *this;
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr typename array_ref<T, N, W, A, Policy>::iterator
array_ref<T, N, W, A, Policy>::begin() noexcept {
  return iterator(m_storage);
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr typename array_ref<T, N, W, A, Policy>::const_iterator
array_ref<T, N, W, A, Policy>::begin() const noexcept {
  return const_iterator(m_storage);
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr void array_ref<T, N, W, A, Policy>::swap(array_ref& other) {
  if (size() != other.size()) {
    throw std::invalid_argument("Cannot swap array_ref of different sizes");
  }
  swap_ranges(begin(), end(), other.begin());
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr typename array_ref<T, N, W, A, Policy>::word_type*
array_ref<T, N, W, A, Policy>::data() noexcept {
  return m_storage.base();
}

template <typename T, size_t N, typename W, size_t A, typename Policy>
constexpr const typename array_ref<T, N, W, A, Policy>::word_type*
array_ref<T, N, W, A, Policy>::data() const noexcept {
  return m_storage.base();
}

}  // namespace bit

#endif  // _BIT_ARRAY_REF_DEFINITION_HPP_INCLUDED
