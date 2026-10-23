#ifndef _BIT_PRIMITIVE_DECLARATION_HPP_INCLUDED
#define _BIT_PRIMITIVE_DECLARATION_HPP_INCLUDED

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "bitlib/bit-containers/bit_containers_forward.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_integral.hpp"
#include "bitlib/bit-iterator/bit_iterator_forward.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

/* ****************************** BIT PRIMITIVE ****************************** */
// primitive<1> (= bit_value) is defined in bit_value.hpp as a full specialization.
// primitive<Bits> (Bits > 1) is defined here, using array<bit_value, Bits, ...>
// as storage. This breaks the recursive dependency:
//   primitive<1> (bit_value) -> no array dependency
//   array<bit_value, ...> -> depends on bit_value (= primitive<1>)
//   primitive<Bits> (Bits > 1) -> depends on array<bit_value, ...>
template <size_t Bits>
class primitive {
  static_assert(Bits >= 1, "primitive must have at least 1 bit");
  static_assert(
      Bits <= bitsof<std::size_t>(),
      "primitive bit width exceeds the native integer size of the architecture");
  static_assert(Bits <= bitsof<std::uint64_t>(),
                "primitive supports at most 64 bits");

  using word_type = ceil_integral<Bits>;
  using policy_type = policy::typical<word_type>;
  using array_type = array<bit_value, Bits, word_type, policy_type>;

  // Friendship
  template <class, primitive_c, size_t>
  friend class reference;

 public:
  static constexpr std::size_t bits = Bits;
  using size_type = std::size_t;
  using storage_type = word_type;
  using value_type = bit_value;
  using reference_type = reference<word_type&>;
  using const_reference_type = const reference<const word_type&>;

 private:
  array_type _data;

 public:
  // Lifecycle
  constexpr primitive() noexcept = default;
  constexpr primitive(const primitive&) noexcept = default;

  constexpr explicit primitive(bool b) noexcept;

  template <class WordType>
  constexpr explicit primitive(WordType val) noexcept
    requires std::integral<WordType>;

  template <class WordType>
  constexpr primitive(WordType val, size_type pos)
    requires std::integral<WordType>;

  // Access
  constexpr reference<word_type&> operator[](size_type pos);
  constexpr const reference<const word_type&> operator[](size_type pos) const;

  // Assignment
  constexpr primitive& operator=(const primitive& other) noexcept;

  template <class T>
  constexpr primitive& operator=(reference<T> ref) noexcept;

  template <class WordType>
  constexpr primitive& assign(WordType val) noexcept
    requires std::integral<WordType>;

  template <class WordType>
  constexpr primitive& assign(WordType val, size_type pos)
    requires std::integral<WordType>;

  // Bitwise assignment operators
  constexpr primitive& operator&=(const primitive& other) noexcept;
  constexpr primitive& operator|=(const primitive& other) noexcept;
  constexpr primitive& operator^=(const primitive& other) noexcept;

  // Conversion
  explicit constexpr operator bool() const noexcept;

  constexpr word_type word_value() const noexcept;

  // Swap members
  void swap(primitive& other) noexcept;

  template <class T>
  void swap(reference<T> other) noexcept;

  // Bitwise operators
  friend constexpr primitive operator~(
      primitive rhs) noexcept {
    for (size_type i = 0; i < Bits; ++i) {
      rhs._data[i] = bit_value(!static_cast<bool>(rhs._data[i]));
    }
    return rhs;
  }

  friend constexpr primitive operator&(
      primitive lhs,
      const primitive& rhs) noexcept {
    for (size_type i = 0; i < Bits; ++i) {
      lhs._data[i] &= rhs._data[i];
    }
    return lhs;
  }

  friend constexpr primitive operator|(
      primitive lhs,
      const primitive& rhs) noexcept {
    for (size_type i = 0; i < Bits; ++i) {
      lhs._data[i] |= rhs._data[i];
    }
    return lhs;
  }

  friend constexpr primitive operator^(
      primitive lhs,
      const primitive& rhs) noexcept {
    for (size_type i = 0; i < Bits; ++i) {
      lhs._data[i] ^= rhs._data[i];
    }
    return lhs;
  }

  // Comparison operators
  friend constexpr bool operator==(
      const primitive& lhs,
      const primitive& rhs) noexcept {
    for (size_type i = 0; i < Bits; ++i) {
      if (static_cast<bool>(lhs._data[i]) != static_cast<bool>(rhs._data[i])) {
        return false;
      }
    }
    return true;
  }

  friend constexpr bool operator!=(
      const primitive& lhs,
      const primitive& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend constexpr bool operator<(
      const primitive& lhs,
      const primitive& rhs) noexcept {
    for (size_type i = 0; i < Bits; ++i) {
      bool l = static_cast<bool>(lhs._data[i]);
      bool r = static_cast<bool>(rhs._data[i]);
      if (l != r) {
        return l < r;
      }
    }
    return false;
  }

  friend constexpr bool operator<=(
      const primitive& lhs,
      const primitive& rhs) noexcept {
    return !(rhs < lhs);
  }

  friend constexpr bool operator>(
      const primitive& lhs,
      const primitive& rhs) noexcept {
    return rhs < lhs;
  }

  friend constexpr bool operator>=(
      const primitive& lhs,
      const primitive& rhs) noexcept {
    return !(lhs < rhs);
  }
};

}  // namespace bit

#endif  // _BIT_PRIMITIVE_DECLARATION_HPP_INCLUDED
