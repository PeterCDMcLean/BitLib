#ifndef _BIT_WORD_REFERENCE_ADAPTER_DEFINITION_HPP_INCLUDED
#define _BIT_WORD_REFERENCE_ADAPTER_DEFINITION_HPP_INCLUDED

#include <utility>

#include "bitlib/bit-iterator/bit_word_reference_adapter_declaration.hpp"

namespace bit {

template <typename target_word_ref, typename source_word_ref>
/* explicit */ constexpr bit_word_reference_adapter<target_word_ref, source_word_ref>::bit_word_reference_adapter(
    source_word_ref source)
  requires(is_small_to_big)
    : _source(source), _index(0) {
}

template <typename target_word_ref, typename source_word_ref>
constexpr bit_word_reference_adapter<target_word_ref, source_word_ref>::bit_word_reference_adapter(
    source_word_ref source,
    const size_type index)
  requires(is_big_to_small)
    : _source(source), _index(index) {
}

template <typename target_word_ref, typename source_word_ref>
/* explicit */ constexpr bit_word_reference_adapter<target_word_ref, source_word_ref>::bit_word_reference_adapter(
    const bit_word_reference_adapter& other)
    : _source(other._source), _index(other._index) {
}

template <typename target_word_ref, typename source_word_ref>
constexpr bit_word_reference_adapter<target_word_ref, source_word_ref>&
bit_word_reference_adapter<target_word_ref, source_word_ref>::operator=(
    const bit_word_reference_adapter& other) {
  if constexpr (is_small_to_big) {
    *reinterpret_cast<target_word*>(&_source) = *reinterpret_cast<const target_word*>(&other._source);
  } else {
    *(reinterpret_cast<target_word*>(&_source) + _index) = *(reinterpret_cast<const target_word*>(&other._source) + other._index);
  }
  return *this;
}

template <typename target_word_ref, typename source_word_ref>
constexpr typename bit_word_reference_adapter<target_word_ref, source_word_ref>::pointer
bit_word_reference_adapter<target_word_ref, source_word_ref>::operator&() const noexcept {
  if constexpr (is_small_to_big) {
    return pointer(&_source);
  } else {
    return pointer(&_source, _index);
  }
}

// Assignment from value type.
// NOT safe for big-endian systems
template <typename target_word_ref, typename source_word_ref>
constexpr bit_word_reference_adapter<target_word_ref, source_word_ref>&
bit_word_reference_adapter<target_word_ref, source_word_ref>::operator=(const target_word& value) noexcept {
  if constexpr (is_small_to_big) {
    *reinterpret_cast<target_word*>(&_source) = value;
  } else {
    *(reinterpret_cast<target_word*>(&_source) + _index) = value;
  }
  return *this;
}

// Implicit conversion to value type
// NOT safe for big-endian systems
template <typename target_word_ref, typename source_word_ref>
constexpr bit_word_reference_adapter<target_word_ref, source_word_ref>::operator target_word() const noexcept {
  if constexpr (is_small_to_big) {
    return *reinterpret_cast<const target_word*>(&_source);
  } else {
    return *(reinterpret_cast<const target_word*>(&_source) + _index);
  }
}

template <typename target_word_ref, typename source_word_ref>
void bit_word_reference_adapter<target_word_ref, source_word_ref>::swap(bit_word_reference_adapter& other) noexcept {
  std::swap(_source, other._source);
  std::swap(_index, other._index);
}

template <typename target_word_ref, typename source_word_ref>
void swap(bit_word_reference_adapter<target_word_ref, source_word_ref> a,
          bit_word_reference_adapter<target_word_ref, source_word_ref> b) {
  std::swap(a._source, b._source);
  std::swap(a._index, b._index);
}

}  // namespace bit

#endif  // _BIT_WORD_REFERENCE_ADAPTER_DEFINITION_HPP_INCLUDED
