// ============================= BIT REFERENCE ============================== //
// Project:         The C++ Bit Library
// Name:            bit_reference.hpp
// Description:     A class representing a reference to a bit
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_REFERENCE_HPP_INCLUDED
#define _BIT_REFERENCE_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
// Project sources
#include <type_traits>

#include "bit_details.hpp"
#include "bit_value.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {
// ========================================================================== //



/* ***************************** BIT REFERENCE ****************************** */
// Bit reference class definition
template <class WordType = uint8_t>
class bit_reference {
  // Assertions
  static_assert(binary_digits<WordType>::value, "");

  // Friendship
  template <class>
  friend class bit_reference;

  // Types
 public:
  using word_type = WordType;
  using size_type = std::size_t;
  using mask_type = std::make_unsigned_t<std::remove_cv_t<word_type>>;

  // Lifecycle
 public:
  template <class T>
  constexpr bit_reference(const bit_reference<T>& other) noexcept;
  constexpr bit_reference(const bit_reference& other) noexcept;
  explicit constexpr bit_reference(word_type& ref) noexcept;
  constexpr bit_reference(word_type& ref, size_type pos);

  // Assignment
 public:
  constexpr bit_reference& operator=(const bit_reference& other) const noexcept;
  template <class T>
  constexpr bit_reference& operator=(const bit_reference<T>& other) const noexcept;
  constexpr bit_reference& operator=(const bit_value val) const noexcept;
  constexpr bit_reference& assign(word_type val) const noexcept;
  constexpr bit_reference& assign(word_type val, size_type pos) const;

  // Bitwise assignment operators
 public:
  constexpr bit_reference& operator&=(bit_value other) const noexcept;
  constexpr bit_reference& operator|=(bit_value other) const noexcept;
  constexpr bit_reference& operator^=(bit_value other) const noexcept;

  // Conversion
 public:
  explicit constexpr operator bool() const noexcept;

  // Access
 public:
  constexpr bit_pointer<WordType> operator&() const noexcept;

  // Swap members
 public:
  template <class T>
  void swap(bit_reference<T> other) const;
  void swap(bit_value& other) const;

  // Bit manipulation
 public:
  constexpr bit_reference& set(bool b) const noexcept;
  constexpr bit_reference& set() const noexcept;
  constexpr bit_reference& reset() const noexcept;
  constexpr bit_reference& flip() const noexcept;

  // Implementation details: function members
 private:
  bit_reference() noexcept = default;
  explicit constexpr bit_reference(word_type* ptr) noexcept;
  constexpr bit_reference(word_type* ptr, size_type pos);

  // Implementation details: data members
 private:
  word_type& _ref;
  const mask_type _mask;
};
static_assert(bit_reference_c<bit_reference<uint8_t>>, "bit_reference does not satisfy bit_reference_c concept!");

// Swap
template <class T, class U>
void swap(bit_reference<T> lhs, bit_reference<U> rhs) noexcept;
template <class T>
void swap(bit_reference<T> lhs, bit_value& rhs) noexcept;
template <class U>
void swap(bit_value& lhs, bit_reference<U> rhs) noexcept;

// Stream functions
template <class CharT, class Traits, class T>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, bit_reference<T>& x);
template <class CharT, class Traits, class T>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, bit_reference<T> x);
/* ************************************************************************** */



// ------------------------ BIT REFERENCE: LIFECYCLE ------------------------ //
// Implicitly constructs a bit reference from another bit reference
template <class WordType>
template <class T>
constexpr bit_reference<WordType>::bit_reference(const bit_reference<T>& other) noexcept
    : _ref(other._ref), _mask(other._mask) {
}

template <class WordType>
constexpr bit_reference<WordType>::bit_reference(const bit_reference<WordType>& other) noexcept
    : _ref(other._ref), _mask(other._mask) {
}

// Explicitly constructs an aligned bit reference
template <class WordType>
constexpr bit_reference<WordType>::bit_reference(word_type& ref) noexcept
    : _ref(ref), _mask(1) {
}

// Explicitly constructs an unaligned bit reference
template <class WordType>
constexpr bit_reference<WordType>::bit_reference(word_type& ref, size_type pos)
    : _ref(ref), _mask(static_cast<word_type>(1) << pos) {
  assert(pos < binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //



// ----------------------- BIT REFERENCE: ASSIGNMENT ------------------------ //
// Copies a bit reference to the bit reference
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::operator=(const bit_reference& other) const noexcept {
  other ? set() : reset();
  return const_cast<bit_reference<WordType>&>(*this);
}

// Assigns a bit reference to the bit reference
template <class WordType>
template <class T>
constexpr bit_reference<WordType>& bit_reference<WordType>::operator=(const bit_reference<T>& other) const noexcept {
  other ? set() : reset();
  return const_cast<bit_reference<WordType>&>(*this);
}

// Assigns a bit value to the bit reference
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::operator=(const bit_value val) const noexcept {
  val ? set() : reset();
  return const_cast<bit_reference<WordType>&>(*this);
}

// Assigns the aligned bit of a value to the bit reference
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::assign(word_type val) const noexcept {
  val & 1 ? set() : reset();
  return const_cast<bit_reference<WordType>&>(*this);
}

// Assigns an unaligned bit of a value to the bit reference
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::assign(word_type val, size_type pos) const {
  assert(pos < binary_digits<word_type>::value);
  val >> pos & 1 ? set() : reset();
  return const_cast<bit_reference<WordType>&>(*this);
}
// -------------------------------------------------------------------------- //



// -------------- BIT REFERENCE: BITWISE ASSIGNMENT OPERATORS --------------- //
// Assigns the value of the referenced bit through a bitwise and operation
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::operator&=(bit_value other) const noexcept {
  _ref &= ~(_mask * static_cast<word_type>(!other._value));
  return const_cast<bit_reference<WordType>&>(*this);
}

// Assigns the value of the referenced bit through a bitwise or operation
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::operator|=(bit_value other) const noexcept {
  _ref |= _mask * static_cast<word_type>(other._value);
  return const_cast<bit_reference<WordType>&>(*this);
}

// Assigns the value of the referenced bit through a bitwise xor operation
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::operator^=(bit_value other) const noexcept {
  _ref ^= _mask * static_cast<word_type>(other._value);
  return const_cast<bit_reference<WordType>&>(*this);
}
// -------------------------------------------------------------------------- //



// ----------------------- BIT REFERENCE: CONVERSION ------------------------ //
// Explicitly converts the bit reference to a boolean value
template <class WordType>
constexpr bit_reference<WordType>::operator bool() const noexcept {
  return _ref & _mask;
}
// -------------------------------------------------------------------------- //



// ------------------------- BIT REFERENCE: ACCESS -------------------------- //
// Gets a bit pointer from the bit reference
template <class WordType>
constexpr bit_pointer<WordType> bit_reference<WordType>::operator&() const noexcept {
  return bit_pointer<WordType>(&_ref, _tzcnt(_mask));
}
// -------------------------------------------------------------------------- //



// ---------------------- BIT REFERENCE: SWAP MEMBERS ----------------------- //
// Swaps the value of the referenced bit with another bit reference
template <class WordType>
template <class T>
void bit_reference<WordType>::swap(bit_reference<T> other) const {
  if (other != *this) {
    flip();
    other.flip();
  }
}

// Swaps the value of the referenced bit with a bit value
template <class WordType>
void bit_reference<WordType>::swap(bit_value& other) const {
  if (other != *this) {
    flip();
    other.flip();
  }
}
// -------------------------------------------------------------------------- //



// -------------------- BIT REFERENCE: BIT MANIPULATION --------------------- //
// Sets the value of the referenced bit to the provided boolean value
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::set(
    bool b) const noexcept {
  b ? set() : reset();
  return const_cast<bit_reference<WordType>&>(*this);
}

// Sets the value of the referenced bit to 1
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::set() const noexcept {
  _ref |= _mask;
  return const_cast<bit_reference<WordType>&>(*this);
}

// Resets the value of the referenced bit to 0
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::reset() const noexcept {
  _ref &= ~_mask;
  return const_cast<bit_reference<WordType>&>(*this);
}

// Flips the value of the referenced bit
template <class WordType>
constexpr bit_reference<WordType>& bit_reference<WordType>::flip() const noexcept {
  _ref ^= _mask;
  return const_cast<bit_reference<WordType>&>(*this);
}
// -------------------------------------------------------------------------- //

// -------------------------- BIT REFERENCE: SWAP --------------------------- //
// Swaps two bit references
template <class T, class U>
void swap(bit_reference<T> lhs, bit_reference<U> rhs) noexcept {
  if (lhs != rhs) {
    lhs.flip();
    rhs.flip();
  }
}

// Swaps a bit reference and a bit value
template <class T>
void swap(bit_reference<T> lhs, bit_value& rhs) noexcept {
  if (lhs != rhs) {
    lhs.flip();
    rhs.flip();
  }
}

// Swaps a bit value and a bit reference
template <class U>
void swap(bit_value& lhs, bit_reference<U> rhs) noexcept {
  if (lhs != rhs) {
    lhs.flip();
    rhs.flip();
  }
}
// -------------------------------------------------------------------------- //



// -------------------- BIT REFERENCE: STREAM FUNCTIONS --------------------- //
// Extracts a bit reference from an input stream
template <class CharT, class Traits, class T>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, bit_reference<T>& x) {
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
          tmp.reset();
          ok = true;
        } else if (traits_type::eq(char_value, is.widen(one))) {
          tmp.set();
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
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, bit_reference<T> x) {
  constexpr char zero = '0';
  constexpr char one = '1';
  return os << os.widen(x ? one : zero);
}
// -------------------------------------------------------------------------- //

// -------- BIT REFERENCE: IMPLEMENTATION DETAILS: FUNCTION MEMBERS --------- //
// Privately explicitly constructs an aligned bit reference from a pointer
template <class WordType>
constexpr bit_reference<WordType>::bit_reference(word_type* ptr) noexcept
    : _ref(*ptr), _mask(1) {
}

// Privately explicitly constructs an unaligned bit reference from a pointer
template <class WordType>
constexpr bit_reference<WordType>::bit_reference(word_type* ptr, size_type pos)
    : _ref(*ptr), _mask(static_cast<word_type>(1) << pos) {
  assert(pos < binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //



// ========================================================================== //
} // namespace bit
#endif // _BIT_REFERENCE_HPP_INCLUDED
// ========================================================================== //
