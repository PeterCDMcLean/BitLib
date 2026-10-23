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
