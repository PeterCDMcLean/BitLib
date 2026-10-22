#ifndef _BIT_VALUE_DEFINITION_HPP_INCLUDED
#define _BIT_VALUE_DEFINITION_HPP_INCLUDED

#include <istream>
#include <ostream>
#include <utility>

#include "bitlib/bit-iterator/bit_value_declaration.hpp"

namespace bit {

template <class T>
constexpr primitive<1>::primitive(reference<T> ref) noexcept
    : _value(static_cast<bool>(ref)) {}

/* explicit */ template <class WordType>
constexpr primitive<1>::primitive(WordType val) noexcept
    : _value(static_cast<bool>(val & 1)) {}

template <class T>
constexpr primitive<1>& primitive<1>::operator=(reference<T> ref) noexcept {
  _value = static_cast<bool>(ref);
  return *this;
}

template <class WordType>
constexpr primitive<1>& primitive<1>::assign(WordType val) noexcept {
  _value = static_cast<bool>(val & 1);
  return *this;
}

constexpr primitive<1>& primitive<1>::operator&=(primitive other) noexcept {
  _value &= other._value;
  return *this;
}

constexpr primitive<1>& primitive<1>::operator|=(primitive other) noexcept {
  _value |= other._value;
  return *this;
}

constexpr primitive<1>& primitive<1>::operator^=(primitive other) noexcept {
  _value ^= other._value;
  return *this;
}

constexpr primitive<1>::operator bool() const noexcept { return _value; }

constexpr primitive<1>::word_type primitive<1>::word_value() const noexcept {
  return static_cast<word_type>(_value);
}

constexpr primitive<1>& primitive<1>::flip() noexcept {
  _value = !_value;
  return *this;
}

inline void primitive<1>::swap(primitive& other) noexcept {
  std::swap(_value, other._value);
}

template <class T>
void primitive<1>::swap(reference<T> other) noexcept {
  if (other != *this) {
    flip();
    other.flip();
  }
}

// Constants
/* ************************************************************************** */

constexpr bit_value bit_off(0U);
constexpr bit_value bit_on(1U);
constexpr bit_value bit0(0U);
constexpr bit_value bit1(1U);
// -------------------------------------------------------------------------- //

// Stream functions
template <class CharT, class Traits, size_t Bits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    primitive<Bits>& x) {
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
  if (sentry) {
    try {
      int_value = is.rdbuf()->sbumpc();
      if (traits_type::eq_int_type(int_value, eof)) {
        state |= ios_base::eofbit;
      } else {
        char_value = traits_type::to_char_type(int_value);
        if (traits_type::eq(char_value, is.widen(zero))) {
          x = primitive<Bits>(false);
          ok = true;
        } else if (traits_type::eq(char_value, is.widen(one))) {
          x = primitive<Bits>(true);
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
    // x already set
  } else {
    state |= ios_base::failbit;
  }
  state ? is.setstate(state) : void();
  return is;
}

template <class CharT, class Traits, size_t Bits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    primitive<Bits> x) {
  constexpr char zero = '0';
  constexpr char one = '1';
  return os << os.widen(static_cast<bool>(x) ? one : zero);
}

}  // namespace bit

#endif  // _BIT_VALUE_DEFINITION_HPP_INCLUDED
