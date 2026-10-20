// ============================= BIT REFERENCE ============================== //
// Project:         The C++ Bit Library
// Name:            bit_reference.hpp
// Description:     A class representing a reference to a bit sequence
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
#include <bit>
#include <type_traits>

#include "bit_details.hpp"
#include "bit_value.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {
// ========================================================================== //



/* ***************************** BIT REFERENCE ****************************** */
// Bit reference class definition - generic reference to a primitive_c-compliant
// bit sequence. bit_reference remains as an alias (reference<>).
template <typename WordRef, primitive_c ValueType>
class reference {
    // Assertions
    static_assert(
        ValueType::bits <= bitsof<std::size_t>(),
        "reference bit width exceeds the native integer size of the architecture");

    // Types
  public:
    using WordType = exact_floor_integral_t<std::remove_cv_t<WordRef>>;
    using word_type = WordType;
    using const_word_type = std::add_const_t<WordType>;
    using value_type = ValueType;
    using size_type = std::size_t;
    using mask_type = std::make_unsigned_t<WordType>;

  private:
    // Mask covering the low ValueType::bits bits.
    static constexpr mask_type bit_mask()
    {
        if constexpr (ValueType::bits == bitsof<mask_type>()) {
            return ~mask_type(0);
        } else {
            return (mask_type(1) << ValueType::bits) - mask_type(1);
        }
    }

    // Friendship
    template <class, primitive_c>
    friend class reference;

    // Lifecycle
  public:
    constexpr reference(const reference& other) noexcept;
    explicit constexpr reference(const WordRef& ref) noexcept;
    constexpr reference(const WordRef& ref, size_type pos) noexcept;

    // Assignment
  public:
    constexpr reference& operator=(const reference& other) const noexcept;
    template <class T>
    constexpr reference& operator=(const reference<T, ValueType>& other) const noexcept;
    constexpr reference& operator=(const value_type& val) const noexcept;
    constexpr reference& assign(word_type val) const noexcept;
    constexpr reference& assign(word_type val, size_type pos) const;

    // Bitwise assignment operators
  public:
    constexpr reference& operator&=(const value_type& other) const noexcept;
    constexpr reference& operator|=(const value_type& other) const noexcept;
    constexpr reference& operator^=(const value_type& other) const noexcept;

    // Comparison operators
  public:
    friend constexpr bool operator==(
        const reference& lhs,
        const reference& rhs
    ) noexcept {
        return (lhs._ref & lhs._mask) == (rhs._ref & rhs._mask);
    }
    friend constexpr bool operator!=(
        const reference& lhs,
        const reference& rhs
    ) noexcept {
        return (lhs._ref & lhs._mask) != (rhs._ref & rhs._mask);
    }
    friend constexpr bool operator<(
        const reference& lhs,
        const reference& rhs
    ) noexcept {
        return (lhs._ref & lhs._mask) < (rhs._ref & rhs._mask);
    }
    friend constexpr bool operator<=(
        const reference& lhs,
        const reference& rhs
    ) noexcept {
        return (lhs._ref & lhs._mask) <= (rhs._ref & rhs._mask);
    }
    friend constexpr bool operator>(
        const reference& lhs,
        const reference& rhs
    ) noexcept {
        return (lhs._ref & lhs._mask) > (rhs._ref & rhs._mask);
    }
    friend constexpr bool operator>=(
        const reference& lhs,
        const reference& rhs
    ) noexcept {
        return (lhs._ref & lhs._mask) >= (rhs._ref & rhs._mask);
    }

    // Bitwise not operator
    friend constexpr value_type operator~(
        const reference& ref
    ) noexcept {
        // Extract the referenced bits to the LSBs, flip them, and let the
        // primitive constructor mask the result to ValueType::bits.
        word_type extracted =
            (static_cast<word_type>(ref._ref) & ref._mask)
            >> std::countr_zero(ref._mask);
        return value_type(static_cast<typename value_type::storage_type>(
            (~extracted) & bit_mask()));
    }

    // Conversion
  public:
    explicit constexpr operator bool() const noexcept;
    constexpr operator value_type() const noexcept;

    // Access
  public:
    constexpr bit_pointer<const_word_type> operator&() const noexcept;
    constexpr bit_pointer<WordType> operator&() noexcept;

    // Swap members
  public:
    template <class T>
    void swap(reference<T, ValueType> other) const;
    void swap(ValueType& other) const;

    // Implementation details: function members
  private:
    reference() noexcept = default;
    explicit constexpr reference(word_type* ptr) noexcept;
    constexpr reference(word_type* ptr, size_type pos) noexcept;

    // Implementation details: data members
  private:
    WordRef _ref;
    mask_type _mask;
};

// Alias: bit_reference remains as an alias to the current form, i.e. a
// single-bit reference with the default word reference type.
using bit_reference = reference<>;

static_assert(bit_reference_c<reference<uint8_t&>>,
              "reference does not satisfy bit_reference_c concept!");

// Swap
template <class T, class U>
void swap(reference<T> lhs, reference<U> rhs) noexcept;
template <class T>
void swap(reference<T> lhs, bit_value& rhs) noexcept;
template <class U>
void swap(bit_value& lhs, reference<U> rhs) noexcept;

// Stream functions
template <class CharT, class Traits, class T>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is, reference<T>& x);
template <class CharT, class Traits, class T>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os, reference<T> x);
/* ************************************************************************** */

// ------------------------ BIT REFERENCE: LIFECYCLE ------------------------ //

template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>::reference(
    const reference<WordRef, ValueType>& other
) noexcept
    : _ref(other._ref), _mask(other._mask) {
}

// Explicitly constructs an aligned bit reference
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>::reference(const WordRef& ref) noexcept
    : _ref(ref), _mask(bit_mask()) {
}

// Explicitly constructs an unaligned bit reference
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>::reference(
    const WordRef& ref,
    size_type pos
) noexcept
    : _ref(ref), _mask(bit_mask() << pos) {
  assert(pos + ValueType::bits <= binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //



// ----------------------- BIT REFERENCE: ASSIGNMENT ------------------------ //
// Copies a bit reference to the bit reference
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>&
reference<WordRef, ValueType>::operator=(const reference& other)
    const noexcept {
  // Copy all ValueType::bits bits from other, not just its boolean truth
  word_type other_val = static_cast<word_type>(
      (static_cast<word_type>(other._ref) & other._mask)
      >> std::countr_zero(other._mask));
  _ref = static_cast<word_type>(
      (static_cast<word_type>(_ref) & ~_mask) |
      (static_cast<word_type>(other_val) << std::countr_zero(_mask)));
  return const_cast<reference<WordRef, ValueType>&>(*this);
}

// Assigns a bit reference to the bit reference
template <class WordRef, primitive_c ValueType>
template <class T>
constexpr reference<WordRef, ValueType>&
reference<WordRef, ValueType>::operator=(const reference<T, ValueType>& other)
    const noexcept {
  // Copy all ValueType::bits bits from other, not just its boolean truth
  word_type other_val = static_cast<word_type>(
      (static_cast<word_type>(other._ref) & other._mask)
      >> std::countr_zero(other._mask));
  _ref = static_cast<word_type>(
      (static_cast<word_type>(_ref) & ~_mask) |
      (static_cast<word_type>(other_val) << std::countr_zero(_mask)));
  return const_cast<reference<WordRef, ValueType>&>(*this);
}

// Assigns a primitive value to the bit reference
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>&
reference<WordRef, ValueType>::operator=(const ValueType& val)
    const noexcept {
  // reference is a friend of primitive, so we can access word_value() directly
  word_type v = static_cast<word_type>(val.word_value());
  word_type masked = (v & bit_mask()) << std::countr_zero(_mask);
  _ref = static_cast<word_type>((_ref & ~_mask) | masked);
  return const_cast<reference<WordRef, ValueType>&>(*this);
}

// Assigns the aligned bits of a value to the bit reference
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>&
reference<WordRef, ValueType>::assign(word_type val) const noexcept {
  word_type masked = (val & bit_mask()) << std::countr_zero(_mask);
  _ref = static_cast<word_type>((_ref & ~_mask) | masked);
  return const_cast<reference<WordRef, ValueType>&>(*this);
}

// Assigns unaligned bits of a value to the bit reference
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>&
reference<WordRef, ValueType>::assign(word_type val, size_type pos) const {
  assert(pos + ValueType::bits <= binary_digits<word_type>::value);
  mask_type msk = bit_mask() << pos;
  word_type masked = (val >> pos) & bit_mask();
  _ref = static_cast<word_type>((_ref & ~msk) | (masked << pos));
  return const_cast<reference<WordRef, ValueType>&>(*this);
}
// -------------------------------------------------------------------------- //



// -------------- BIT REFERENCE: BITWISE ASSIGNMENT OPERATORS --------------- //
// Assigns the value of the referenced bits through a bitwise and operation
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>&
reference<WordRef, ValueType>::operator&=(const value_type& other)
    const noexcept {
  word_type o = static_cast<word_type>(other.word_value()) & bit_mask();
  word_type shifted = o << std::countr_zero(_mask);
  _ref = static_cast<word_type>(
      (_ref & ~_mask) | ((_ref & _mask) & shifted));
  return const_cast<reference<WordRef, ValueType>&>(*this);
}

// Assigns the value of the referenced bits through a bitwise or operation
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>&
reference<WordRef, ValueType>::operator|=(const value_type& other)
    const noexcept {
  word_type o = static_cast<word_type>(other.word_value()) & bit_mask();
  word_type shifted = o << std::countr_zero(_mask);
  _ref = static_cast<word_type>(
      (_ref & ~_mask) | ((_ref & _mask) | shifted));
  return const_cast<reference<WordRef, ValueType>&>(*this);
}

// Assigns the value of the referenced bits through a bitwise xor operation
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>&
reference<WordRef, ValueType>::operator^=(const value_type& other)
    const noexcept {
  word_type o = static_cast<word_type>(other.word_value()) & bit_mask();
  word_type shifted = o << std::countr_zero(_mask);
  _ref = static_cast<word_type>(
      (_ref & ~_mask) | ((_ref & _mask) ^ shifted));
  return const_cast<reference<WordRef, ValueType>&>(*this);
}
// -------------------------------------------------------------------------- //



// ----------------------- BIT REFERENCE: CONVERSION ------------------------ //
// Explicitly converts the bit reference to a boolean value
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>::operator bool() const noexcept {
  return (_ref & _mask) != mask_type(0);
}

// Converts the bit reference to its primitive value type
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>::operator value_type() const noexcept {
  return value_type(static_cast<typename value_type::storage_type>(
      (static_cast<word_type>(_ref) & _mask) >> std::countr_zero(_mask)));
}
// -------------------------------------------------------------------------- //



// ------------------------- BIT REFERENCE: ACCESS -------------------------- //
// Gets a bit pointer from the bit reference
template <class WordRef, primitive_c ValueType>
constexpr bit_pointer<typename reference<WordRef, ValueType>::
    const_word_type>
reference<WordRef, ValueType>::operator&() const noexcept {
  return bit_pointer<const_word_type>(&_ref, std::countr_zero(_mask));
}

template <class WordRef, primitive_c ValueType>
constexpr bit_pointer<typename reference<WordRef, ValueType>::WordType>
reference<WordRef, ValueType>::operator&() noexcept {
  return bit_pointer<WordType>(&_ref, std::countr_zero(_mask));
}
// -------------------------------------------------------------------------- //



// ---------------------- BIT REFERENCE: SWAP MEMBERS ----------------------- //
// Swaps the value of the referenced bits with another bit reference
template <class WordRef, primitive_c ValueType>
template <class T>
void reference<WordRef, ValueType>::swap(reference<T, ValueType> other) const {
  if (other != *this) {
    value_type me = static_cast<value_type>(*this);
    value_type them = static_cast<value_type>(other);
    *this = them;
    other = me;
  }
}

// Swaps the value of the referenced bits with a primitive value
template <class WordRef, primitive_c ValueType>
void reference<WordRef, ValueType>::swap(ValueType& other) const {
  if (other != *this) {
    value_type me = static_cast<value_type>(*this);
    value_type them = static_cast<value_type>(other);
    *this = them;
    other = me;
  }
}
// -------------------------------------------------------------------------- //



// ------------------------ BIT REFERENCE: SWAP --------------------------- //
// Swaps two bit references
template <class T, class U>
void swap(reference<T> lhs, reference<U> rhs) noexcept {
  if (lhs != rhs) {
    bit_value l = static_cast<bit_value>(lhs);
    bit_value r = static_cast<bit_value>(rhs);
    lhs = r;
    rhs = l;
  }
}

// Swaps a bit reference and a bit value
template <class T>
void swap(reference<T> lhs, bit_value& rhs) noexcept {
  if (lhs != rhs) {
    bit_value l = static_cast<bit_value>(lhs);
    bit_value r = static_cast<bit_value>(rhs);
    lhs = r;
    rhs = l;
  }
}

// Swaps a bit value and a bit reference
template <class U>
void swap(bit_value& lhs, reference<U> rhs) noexcept {
  if (lhs != rhs) {
    bit_value l = static_cast<bit_value>(lhs);
    bit_value r = static_cast<bit_value>(rhs);
    lhs = r;
    rhs = l;
  }
}
// -------------------------------------------------------------------------- //



// -------------------- BIT REFERENCE: STREAM FUNCTIONS --------------------- //
// Extracts a bit reference from an input stream
template <class CharT, class Traits, class T>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    reference<T>& x
) {
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
    reference<T> x
) {
  constexpr char zero = '0';
  constexpr char one = '1';
  return os << os.widen(x ? one : zero);
}
// -------------------------------------------------------------------------- //



// -------- BIT REFERENCE: IMPLEMENTATION DETAILS: FUNCTION MEMBERS --------- //
// Privately explicitly constructs an aligned bit reference from a pointer
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>::reference(word_type* ptr) noexcept
    : _ref(*ptr), _mask(bit_mask()) {
}

// Privately explicitly constructs an unaligned bit reference from a pointer
template <class WordRef, primitive_c ValueType>
constexpr reference<WordRef, ValueType>::reference(
    word_type* ptr,
    size_type pos
) noexcept
    : _ref(*ptr), _mask(bit_mask() << pos) {
  assert(pos + ValueType::bits <= binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //



// ========================================================================== //
} // namespace bit
#endif // _BIT_REFERENCE_HPP_INCLUDED
// ========================================================================== //
