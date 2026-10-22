#ifndef _BIT_VALUE_DECLARATION_HPP_INCLUDED
#define _BIT_VALUE_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <type_traits>
#include <utility>

#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_iterator_forward.hpp"

namespace bit {

/* ******************************* BIT VALUE ******************************** */
// primitive<1> is the fundamental bit type, i.e. bit_value.
// It stores a single bool and does not depend on array.
template <>
class primitive<1> {
  // Friendship
  template <class, primitive_c, size_t>
  friend class reference;

  // Types
 public:
  static constexpr std::size_t bits = 1;
  using size_type = std::size_t;
  using storage_type = bool;
  using word_type = std::uint8_t;

 private:
  bool _value;

 public:
  // Lifecycle
  constexpr primitive() noexcept : _value(false) {}
  template <class T>
  constexpr primitive(reference<T> ref) noexcept;
  template <class WordType>
  explicit constexpr primitive(WordType val) noexcept;
  explicit constexpr primitive(bool val) noexcept : _value(val) {}

  // Assignment
 public:
  template <class T>
  constexpr primitive& operator=(reference<T> ref) noexcept;
  template <class WordType>
  constexpr primitive& assign(WordType val) noexcept;

  // Bitwise assignment operators
 public:
  constexpr primitive& operator&=(primitive other) noexcept;
  constexpr primitive& operator|=(primitive other) noexcept;
  constexpr primitive& operator^=(primitive other) noexcept;

  // Conversion
 public:
  explicit constexpr operator bool() const noexcept;
  constexpr word_type word_value() const noexcept;

  // Bit manipulation
  constexpr primitive<1>& flip() noexcept;

  // Swap members
 public:
  void swap(primitive& other) noexcept;
  template <class T>
  void swap(reference<T> other) noexcept;

  // Bitwise operators
 public:
  friend constexpr primitive operator~(
      primitive rhs) noexcept {
    return primitive(!rhs._value);
  }

  friend constexpr primitive operator&(
      primitive lhs,
      primitive rhs) noexcept {
    return primitive(lhs._value & rhs._value);
  }

  friend constexpr primitive operator|(
      primitive lhs,
      primitive rhs) noexcept {
    return primitive(lhs._value | rhs._value);
  }

  friend constexpr primitive operator^(
      primitive lhs,
      primitive rhs) noexcept {
    return primitive(lhs._value ^ rhs._value);
  }

  // Comparison operators
 public:
  friend constexpr bool operator==(
      primitive lhs,
      primitive rhs) noexcept {
    return lhs._value == rhs._value;
  }

  friend constexpr bool operator!=(
      primitive lhs,
      primitive rhs) noexcept {
    return lhs._value != rhs._value;
  }

  friend constexpr bool operator<(
      primitive lhs,
      primitive rhs) noexcept {
    return lhs._value < rhs._value;
  }

  friend constexpr bool operator<=(
      primitive lhs,
      primitive rhs) noexcept {
    return lhs._value <= rhs._value;
  }

  friend constexpr bool operator>(
      primitive lhs,
      primitive rhs) noexcept {
    return lhs._value > rhs._value;
  }

  friend constexpr bool operator>=(
      primitive lhs,
      primitive rhs) noexcept {
    return lhs._value >= rhs._value;
  }
};

// Stream functions
template <class CharT, class Traits, size_t Bits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    primitive<Bits>& x);
template <class CharT, class Traits, size_t Bits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    primitive<Bits> x);

// Constants are defined in bit_value_definition.hpp.
// -------------------------------------------------------------------------- //

}  // namespace bit

#endif  // _BIT_VALUE_DECLARATION_HPP_INCLUDED
