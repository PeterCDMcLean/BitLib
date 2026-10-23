#ifndef _BIT_REFERENCE_DEFINITION_HPP_INCLUDED
#define _BIT_REFERENCE_DEFINITION_HPP_INCLUDED

#include <bit>
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>

#include "bitlib/bit-algorithms/bit_algorithm_details.hpp"
#include "bitlib/bit-algorithms/bit_algorithm_forward.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-iterator/bit_reference_declaration.hpp"
#include "bitlib/bit-iterator/bit_value_definition.hpp"

namespace bit {

/* ********************* SINGLE-WORD, SINGLE-BIT REFERENCE ****************** */

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr typename reference<StorageRef, ValueType, Alignment>::state_type
reference<StorageRef, ValueType, Alignment>::make_state(size_type pos) noexcept {
  if constexpr (Alignment == std::dynamic_extent) {
    return state_type{pos < word_bits
                          ? static_cast<mask_type>(mask_type(1) << pos)
                          : mask_type(0)};
  } else {
    return state_type{};
  }
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr bool reference<StorageRef, ValueType, Alignment>::value() const noexcept {
  return (_ref & this->mask()) != mask_type(0);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr void reference<StorageRef, ValueType, Alignment>::assign(bool bit) const noexcept {
  if (bit) {
    _ref = static_cast<word_type>(_ref | this->mask());
  } else {
    _ref = static_cast<word_type>(_ref & ~this->mask());
  }
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::reference(const reference& other) noexcept
    : state_type(other), _ref(other._ref) {}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
/* explicit */ constexpr reference<StorageRef, ValueType, Alignment>::reference(StorageRef& ref) noexcept
    : state_type(make_state(0)),
      _ref(ref) {}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::reference(StorageRef& ref, size_type pos) noexcept
    : state_type(make_state(pos)),
      _ref(ref) {
  const size_type capacity = bitsof<word_type>();
  assert(pos <= capacity && ValueType::bits <= capacity - pos);
  if constexpr (Alignment != std::dynamic_extent) {
    assert(pos == Alignment);
  }
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(const reference& other) const noexcept {
  assign(other.value());
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
template <class OtherStorage, std::size_t OtherAlignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(
    const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept {
  assign(static_cast<bool>(other));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(const value_type& val) const noexcept {
  assign(static_cast<bool>(val));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(bool bit) const noexcept {
  assign(bit);
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::assign(word_type val) const noexcept {
  assign((val & word_type(1)) != 0);
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::assign(word_type val, size_type pos) const {
  assert(pos < word_bits);
  assign(((val >> pos) & word_type(1)) != 0);
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator&=(const value_type& other) const noexcept {
  assign(value() && static_cast<bool>(other));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator|=(const value_type& other) const noexcept {
  assign(value() || static_cast<bool>(other));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator^=(const value_type& other) const noexcept {
  assign(value() != static_cast<bool>(other));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::operator bool() const noexcept {
  return value();
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::operator value_type() const noexcept {
  return value_type(value());
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::set(bool bit) const noexcept {
  assign(bit);
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::set() const noexcept { return set(true); }

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::reset() const noexcept { return set(false); }

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::flip() const noexcept { return set(!value()); }

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr bit_pointer<typename reference<StorageRef, ValueType, Alignment>::const_word_type>
reference<StorageRef, ValueType, Alignment>::operator&() const noexcept {
  return bit_pointer<const_word_type>(std::addressof(_ref), this->position());
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr bit_pointer<typename reference<StorageRef, ValueType, Alignment>::WordType>
reference<StorageRef, ValueType, Alignment>::operator&() noexcept
  requires(writable_storage)
{
  return bit_pointer<WordType>(std::addressof(_ref), this->position());
}

/* *********************** ARRAY-BACKED REFERENCE ************************** */

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr auto reference<StorageRef, ValueType, Alignment>::_array_ref() const noexcept {
  return state_type::_array_ref();
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr typename reference<StorageRef, ValueType, Alignment>::const_value_array_type
reference<StorageRef, ValueType, Alignment>::value_array(
    const value_type& value) noexcept {
  static_assert(sizeof(value_type) % sizeof(value_word_type) == 0);
  static_assert(ValueType::bits <= sizeof(value_type) * 8);
  const auto* words = reinterpret_cast<const value_word_type*>(
      std::addressof(value));
  return const_value_array_type(words);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr typename reference<StorageRef, ValueType, Alignment>::value_array_type
reference<StorageRef, ValueType, Alignment>::value_array(value_type& value) noexcept {
  static_assert(sizeof(value_type) % sizeof(value_word_type) == 0);
  static_assert(ValueType::bits <= sizeof(value_type) * 8);
  auto* words =
      reinterpret_cast<value_word_type*>(std::addressof(value));
  return value_array_type(words);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr typename reference<StorageRef, ValueType, Alignment>::state_type
reference<StorageRef, ValueType, Alignment>::make_state(
    const StorageRef& ref, size_type pos) noexcept {
  if constexpr (proxy_storage) {
    return state_type(ref, pos);
  } else {
    return state_type(reinterpret_cast<field_word_type*>(std::addressof(ref)),
                      pos);
  }
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr typename reference<StorageRef, ValueType, Alignment>::mask_type
reference<StorageRef, ValueType, Alignment>::proxy_mask() const noexcept
  requires(proxy_storage)
{
  return static_cast<mask_type>(mask_type{1} << (this->position() % word_bits));
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr bool
reference<StorageRef, ValueType, Alignment>::proxy_value() const noexcept
  requires(proxy_storage)
{
  return (static_cast<word_type>(this->proxy_) & proxy_mask()) != 0;
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr void
reference<StorageRef, ValueType, Alignment>::proxy_assign(bool bit) const noexcept
  requires(proxy_storage)
{
  auto word = static_cast<word_type>(this->proxy_);
  if (bit) {
    word = static_cast<word_type>(word | proxy_mask());
  } else {
    word = static_cast<word_type>(word & ~proxy_mask());
  }
  this->proxy_ = word;
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>::reference(const reference& other) noexcept
    : state_type(other) {}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
/* explicit */ constexpr reference<StorageRef, ValueType, Alignment>::reference(
    const StorageRef& ref) noexcept
    : state_type(make_state(
          ref, Alignment == std::dynamic_extent ? 0 : Alignment)) {}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>::reference(
    const StorageRef& ref, size_type pos) noexcept
  requires(Alignment == std::dynamic_extent)
    : state_type(make_state(ref, pos)) {
  if constexpr (!proxy_storage) {
    assert(pos <= num_words * word_bits &&
           ValueType::bits <= num_words * word_bits - pos);
  }
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(const reference& other) const noexcept {
  *this = static_cast<value_type>(other);
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
template <class OtherStorage, std::size_t OtherAlignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(
    const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept {
  *this = static_cast<value_type>(other);
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(const value_type& value) const noexcept {
  if constexpr (proxy_storage) {
    proxy_assign(static_cast<bool>(value));
  } else {
    auto destination = _array_ref();
    if constexpr (ValueType::bits == 1) {
      *destination.begin() = static_cast<bool>(value);
    } else {
      const auto source = value_array(value);
      ::bit::copy(source.begin(), source.end(), destination.begin());
    }
  }
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(bool bit) const noexcept
  requires(ValueType::bits == 1)
{
  if constexpr (proxy_storage) {
    proxy_assign(bit);
  } else {
    _array_ref()[0] = bit;
  }
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator&=(const value_type& other) const noexcept {
  if constexpr (proxy_storage) {
    proxy_assign(proxy_value() && static_cast<bool>(other));
  } else {
    _array_ref() &= other;
  }
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator|=(const value_type& other) const noexcept {
  if constexpr (proxy_storage) {
    proxy_assign(proxy_value() || static_cast<bool>(other));
  } else {
    _array_ref() |= other;
  }
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator^=(const value_type& other) const noexcept {
  if constexpr (proxy_storage) {
    proxy_assign(proxy_value() != static_cast<bool>(other));
  } else {
    _array_ref() ^= other;
  }
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>::operator bool() const noexcept {
  if constexpr (proxy_storage) {
    return proxy_value();
  } else {
    return ::bit::find(_array_ref().begin(), _array_ref().end(), bit1) !=
           _array_ref().end();
  }
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>::operator value_type() const noexcept {
  if constexpr (proxy_storage) {
    return value_type(proxy_value());
  } else {
    value_type value;
    ::bit::copy(_array_ref().begin(), _array_ref().end(), &value);
    return value;
  }
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::set(bool bit) const noexcept
  requires(ValueType::bits == 1)
{
  return *this = bit;
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::set() const noexcept
  requires(ValueType::bits == 1)
{
  return set(true);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::reset() const noexcept
  requires(ValueType::bits == 1)
{
  return set(false);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::flip() const noexcept
  requires(ValueType::bits == 1)
{
  return set(!static_cast<bool>(*this));
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr bit_pointer<typename reference<StorageRef, ValueType, Alignment>::const_word_type>
reference<StorageRef, ValueType, Alignment>::operator&() const noexcept
  requires(!proxy_storage)
{
  const auto array = _array_ref();
  const auto first = array.begin();
  return bit_pointer<const_word_type>(first.base(), first.position());
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires array_ref_storage<StorageRef, ValueType>
constexpr bit_pointer<typename reference<StorageRef, ValueType, Alignment>::word_type>
reference<StorageRef, ValueType, Alignment>::operator&() noexcept
  requires(!std::is_const_v<storage_type> && !proxy_storage)
{
  auto array = _array_ref();
  const auto first = array.begin();
  return bit_pointer<word_type>(first.base(), first.position());
}

// Alias: bit_reference remains as an alias to the current form, i.e. a
// single-bit reference with the default storage reference type. It is defined
// here because forming reference<> requires primitive<1> to be complete.
using bit_reference = reference<>;

static_assert(bit_reference_c<reference<uint8_t&>>,
              "reference does not satisfy bit_reference_c concept!");

// Stream functions
template <class CharT, class Traits, class T>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    reference<T>& x) {
  using stream_type = std::basic_istream<CharT, Traits>;
  using traits_type = typename stream_type::traits_type;
  using ios_base = typename stream_type::ios_base;
  constexpr char zero = '0';
  constexpr char one = '1';
  constexpr typename stream_type::int_type eof = traits_type::eof();
  typename ios_base::iostate state = ios_base::goodbit;
  typename stream_type::char_type char_value = 0;
  typename stream_type::int_type int_value = 0;
  typename stream_type::sentry sentry(is);
  bool ok = false;
  bit_value tmp = x;
  if (sentry) {
    try {
      int_value = is.rdbuf()->sbumpc();
      if (traits_type::eq_int_type(int_value, eof)) {
        state |= ios_base::eofbit;
      } else {
        char_value = traits_type::to_char_type(int_value);
        if (traits_type::eq(char_value, is.widen(zero))) {
          tmp = bit_value(false);
          ok = true;
        } else if (traits_type::eq(char_value, is.widen(one))) {
          tmp = bit_value(true);
          ok = true;
        } else {
          int_value = is.rdbuf()->sputbackc(char_value);
          if (traits_type::eq_int_type(int_value, eof)) {
            state |= ios_base::failbit;
          }
        }
      }
    } catch (...) {
      is.setstate(ios_base::badbit);
    }
  }
  if (ok) {
    x = tmp;
  } else {
    state |= ios_base::failbit;
  }
  state ? is.setstate(state) : void();
  return is;
}

// Inserts a bit reference in an output stream
template <class CharT, class Traits, class T>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    reference<T> x) {
  constexpr char zero = '0';
  constexpr char one = '1';
  return os << os.widen(x ? one : zero);
}

}  // namespace bit

#endif  // _BIT_REFERENCE_DEFINITION_HPP_INCLUDED
