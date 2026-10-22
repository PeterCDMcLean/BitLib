#ifndef _BIT_PRIMITIVE_DEFINITION_HPP_INCLUDED
#define _BIT_PRIMITIVE_DEFINITION_HPP_INCLUDED

#include <cassert>
#include <utility>

#include "bitlib/bit-containers/bit_array_definition.hpp"
#include "bitlib/bit-iterator/bit_primitive_declaration.hpp"

namespace bit {

template <size_t Bits>
/* explicit */ constexpr primitive<Bits>::primitive(bool b) noexcept {
  _data.fill(b ? bit_value(true) : bit_value(false));
}

template <size_t Bits>
/* explicit */ template <class WordType>
constexpr primitive<Bits>::primitive(WordType val) noexcept
  requires std::integral<WordType>
{
  static_assert(binary_digits<WordType>::value, "");
  for (size_type i = 0; i < Bits; ++i) {
    _data[i] = bit_value(static_cast<bool>((val >> i) & 1));
  }
}

template <size_t Bits>
template <class WordType>
constexpr primitive<Bits>::primitive(WordType val, size_type pos)
  requires std::integral<WordType>
{
  static_assert(binary_digits<WordType>::value, "");
  assert(pos + Bits <= binary_digits<WordType>::value);
  for (size_type i = 0; i < Bits; ++i) {
    _data[i] = bit_value(static_cast<bool>((val >> (pos + i)) & 1));
  }
}

template <size_t Bits>
constexpr typename primitive<Bits>::reference_type primitive<Bits>::operator[](size_type pos) {
  assert(pos < Bits);
  return _data[pos];
}

template <size_t Bits>
constexpr typename primitive<Bits>::const_reference_type primitive<Bits>::operator[](size_type pos) const {
  assert(pos < Bits);
  return _data[pos];
}

template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::operator=(const primitive& other) noexcept {
  _data = other._data;
  return *this;
}

template <size_t Bits>
template <class T>
constexpr primitive<Bits>& primitive<Bits>::operator=(reference<T> ref) noexcept {
  // Set bit 0 to ref's value, clear all others
  _data[0] = bit_value(static_cast<bool>(ref));
  for (size_type i = 1; i < Bits; ++i) {
    _data[i] = bit_value(false);
  }
  return *this;
}

template <size_t Bits>
template <class WordType>
constexpr primitive<Bits>& primitive<Bits>::assign(WordType val) noexcept
  requires std::integral<WordType>
{
  static_assert(binary_digits<WordType>::value, "");
  for (size_type i = 0; i < Bits; ++i) {
    _data[i] = bit_value(static_cast<bool>((val >> i) & 1));
  }
  return *this;
}

template <size_t Bits>
template <class WordType>
constexpr primitive<Bits>& primitive<Bits>::assign(WordType val, size_type pos)
  requires std::integral<WordType>
{
  static_assert(binary_digits<WordType>::value, "");
  assert(pos + Bits <= binary_digits<WordType>::value);
  for (size_type i = 0; i < Bits; ++i) {
    _data[i] = bit_value(static_cast<bool>((val >> (pos + i)) & 1));
  }
  return *this;
}

template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::operator&=(const primitive& other) noexcept {
  for (size_type i = 0; i < Bits; ++i) {
    _data[i] &= other._data[i];
  }
  return *this;
}

template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::operator|=(const primitive& other) noexcept {
  for (size_type i = 0; i < Bits; ++i) {
    _data[i] |= other._data[i];
  }
  return *this;
}

template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::operator^=(const primitive& other) noexcept {
  for (size_type i = 0; i < Bits; ++i) {
    _data[i] ^= other._data[i];
  }
  return *this;
}

template <size_t Bits>
constexpr primitive<Bits>::operator bool() const noexcept {
  for (size_type i = 0; i < Bits; ++i) {
    if (static_cast<bool>(_data[i])) {
      return true;
    }
  }
  return false;
}

template <size_t Bits>
constexpr typename primitive<Bits>::word_type primitive<Bits>::word_value() const noexcept {
  word_type result = 0;
  for (size_type i = 0; i < Bits; ++i) {
    if (static_cast<bool>(_data[i])) {
      result |= (word_type(1) << i);
    }
  }
  return result;
}

template <size_t Bits>
void primitive<Bits>::swap(primitive& other) noexcept {
  _data.swap(other._data);
}

template <size_t Bits>
template <class T>
void primitive<Bits>::swap(reference<T> other) noexcept {
  if (static_cast<bool>(_data[0]) != static_cast<bool>(other)) {
    bit_value temp = static_cast<bit_value>(_data[0]);
    _data[0] = static_cast<bit_value>(other);
    other = temp;
  }
}

}  // namespace bit

#endif  // _BIT_PRIMITIVE_DEFINITION_HPP_INCLUDED
