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
// Bit reference class definition - generic reference to a PrimitiveBits-wide
// bit sequence. bit_reference remains as an alias (reference<>).
template <typename WordRef = uintptr_t&, size_t PrimitiveBits = 1>
class reference {
    // Assertions
    static_assert(PrimitiveBits >= 1,
                  "reference must reference at least 1 bit");
    static_assert(
        PrimitiveBits <= bitsof<std::size_t>(),
        "reference bit width exceeds the native integer size of the architecture");

    // Types
  public:
    using WordType = exact_floor_integral_t<std::remove_cv_t<WordRef>>;
    using word_type = WordType;
    using const_word_type = std::add_const_t<WordType>;
    using value_type = primitive<PrimitiveBits>;
    using size_type = std::size_t;
    using mask_type = std::make_unsigned_t<WordType>;

  private:
    // Mask covering the low PrimitiveBits bits.
    static constexpr mask_type bit_mask()
    {
        if constexpr (PrimitiveBits == bitsof<mask_type>()) {
            return ~mask_type(0);
        } else {
            return (mask_type(1) << PrimitiveBits) - mask_type(1);
        }
    }

    // Friendship
    template <class, size_t>
    friend class reference;

    // Lifecycle
  public:
    constexpr reference(const reference& other) noexcept;
    explicit constexpr reference(const WordRef& ref) noexcept;
    constexpr reference(const WordRef& ref, size_type pos) noexcept;

    // Assignment
  public:
    constexpr reference& operator=(const reference& other) const noexcept;
    template <class T, size_t B>
    constexpr reference& operator=(const reference<T, B>& other) const noexcept;
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
        // primitive constructor mask the result to PrimitiveBits.
        word_type extracted =
            (static_cast<word_type>(ref._ref) & ref._mask)
            >> std::countr_zero(ref._mask);
        return value_type(static_cast<typename value_type::storage_type>(
            ~extracted));
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
    template <class T, size_t B>
    void swap(reference<T, B> other) const;
    void swap(bit_value& other) const;

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

template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>::reference(
    const reference& other
) noexcept
    : _ref(other._ref), _mask(other._mask) {
}

// Explicitly constructs an aligned bit reference
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>::reference(const WordRef& ref) noexcept
    : _ref(ref), _mask(bit_mask()) {
}

// Explicitly constructs an unaligned bit reference
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>::reference(
    const WordRef& ref,
    size_type pos
) noexcept
    : _ref(ref), _mask(bit_mask() << pos) {
  assert(pos + PrimitiveBits <= binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //



// ----------------------- BIT REFERENCE: ASSIGNMENT ------------------------ //
// Copies a bit reference to the bit reference
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>&
reference<WordRef, PrimitiveBits>::operator=(const reference& other)
    const noexcept {
  // Copy all PrimitiveBits bits from other, not just its boolean truth
  word_type other_val = static_cast<word_type>(
      (static_cast<word_type>(other._ref) & other._mask)
      >> std::countr_zero(other._mask));
  _ref = static_cast<word_type>(
      (static_cast<word_type>(_ref) & ~_mask) |
      (static_cast<word_type>(other_val) << std::countr_zero(_mask)));
  return const_cast<reference<WordRef, PrimitiveBits>&>(*this);
}

// Assigns a bit reference to the bit reference
template <class WordRef, size_t PrimitiveBits>
template <class T, size_t B>
constexpr reference<WordRef, PrimitiveBits>&
reference<WordRef, PrimitiveBits>::operator=(const reference<T, B>& other)
    const noexcept {
  // Copy all PrimitiveBits bits from other, not just its boolean truth
  word_type other_val = static_cast<word_type>(
      (static_cast<word_type>(other._ref) & other._mask)
      >> std::countr_zero(other._mask));
  _ref = static_cast<word_type>(
      (static_cast<word_type>(_ref) & ~_mask) |
      (static_cast<word_type>(other_val) << std::countr_zero(_mask)));
  return const_cast<reference<WordRef, PrimitiveBits>&>(*this);
}

// Assigns a primitive value to the bit reference
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>&
reference<WordRef, PrimitiveBits>::operator=(const value_type& val)
    const noexcept {
  // reference is a friend of primitive, so we can access its storage directly
  word_type v = static_cast<word_type>(val._value);
  word_type masked = (v & bit_mask()) << std::countr_zero(_mask);
  _ref = static_cast<word_type>((_ref & ~_mask) | masked);
  return const_cast<reference<WordRef, PrimitiveBits>&>(*this);
}

// Assigns the aligned bits of a value to the bit reference
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>&
reference<WordRef, PrimitiveBits>::assign(word_type val) const noexcept {
  word_type masked = (val & bit_mask()) << std::countr_zero(_mask);
  _ref = static_cast<word_type>((_ref & ~_mask) | masked);
  return const_cast<reference<WordRef, PrimitiveBits>&>(*this);
}

// Assigns unaligned bits of a value to the bit reference
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>&
reference<WordRef, PrimitiveBits>::assign(word_type val, size_type pos) const {
  assert(pos + PrimitiveBits <= binary_digits<word_type>::value);
  mask_type msk = bit_mask() << pos;
  word_type masked = (val >> pos) & bit_mask();
  _ref = static_cast<word_type>((_ref & ~msk) | (masked << pos));
  return const_cast<reference<WordRef, PrimitiveBits>&>(*this);
}
// -------------------------------------------------------------------------- //



// -------------- BIT REFERENCE: BITWISE ASSIGNMENT OPERATORS --------------- //
// Assigns the value of the referenced bits through a bitwise and operation
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>&
reference<WordRef, PrimitiveBits>::operator&=(const value_type& other)
    const noexcept {
  word_type o = static_cast<word_type>(other._value) & bit_mask();
  word_type shifted = o << std::countr_zero(_mask);
  _ref = static_cast<word_type>(
      (_ref & ~_mask) | ((_ref & _mask) & shifted));
  return const_cast<reference<WordRef, PrimitiveBits>&>(*this);
}

// Assigns the value of the referenced bits through a bitwise or operation
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>&
reference<WordRef, PrimitiveBits>::operator|=(const value_type& other)
    const noexcept {
  word_type o = static_cast<word_type>(other._value) & bit_mask();
  word_type shifted = o << std::countr_zero(_mask);
  _ref = static_cast<word_type>(
      (_ref & ~_mask) | ((_ref & _mask) | shifted));
  return const_cast<reference<WordRef, PrimitiveBits>&>(*this);
}

// Assigns the value of the referenced bits through a bitwise xor operation
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>&
reference<WordRef, PrimitiveBits>::operator^=(const value_type& other)
    const noexcept {
  word_type o = static_cast<word_type>(other._value) & bit_mask();
  word_type shifted = o << std::countr_zero(_mask);
  _ref = static_cast<word_type>(
      (_ref & ~_mask) | ((_ref & _mask) ^ shifted));
  return const_cast<reference<WordRef, PrimitiveBits>&>(*this);
}
// -------------------------------------------------------------------------- //



// ----------------------- BIT REFERENCE: CONVERSION ------------------------ //
// Explicitly converts the bit reference to a boolean value
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>::operator bool() const noexcept {
  return (_ref & _mask) != mask_type(0);
}

// Converts the bit reference to its primitive value type
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>::operator value_type() const noexcept {
  return value_type(static_cast<typename value_type::storage_type>(
      (static_cast<word_type>(_ref) & _mask) >> std::countr_zero(_mask)));
}
// -------------------------------------------------------------------------- //



// ------------------------- BIT REFERENCE: ACCESS -------------------------- //
// Gets a bit pointer from the bit reference
template <class WordRef, size_t PrimitiveBits>
constexpr bit_pointer<typename reference<WordRef, PrimitiveBits>::
    const_word_type>
reference<WordRef, PrimitiveBits>::operator&() const noexcept {
  return bit_pointer<const_word_type>(&_ref, std::countr_zero(_mask));
}

template <class WordRef, size_t PrimitiveBits>
constexpr bit_pointer<typename reference<WordRef, PrimitiveBits>::WordType>
reference<WordRef, PrimitiveBits>::operator&() noexcept {
  return bit_pointer<WordType>(&_ref, std::countr_zero(_mask));
}
// -------------------------------------------------------------------------- //



// ---------------------- BIT REFERENCE: SWAP MEMBERS ----------------------- //
// Swaps the value of the referenced bits with another bit reference
template <class WordRef, size_t PrimitiveBits>
template <class T, size_t B>
void reference<WordRef, PrimitiveBits>::swap(reference<T, B> other) const {
  if (other != *this) {
    size_t my_shift = std::countr_zero(_mask);
    typename reference<T, B>::mask_type other_mask = other._mask;
    typename reference<T, B>::word_type other_shift = std::countr_zero(other_mask);

    word_type my_val =
        (static_cast<word_type>(_ref) & _mask) >> my_shift;
    typename reference<T, B>::word_type other_val =
        (static_cast<typename reference<T, B>::word_type>(other._ref) & other_mask)
        >> other_shift;

    _ref = static_cast<WordRef>(
        (static_cast<word_type>(_ref) & ~_mask) |
        (static_cast<word_type>(other_val) << my_shift));
    other._ref = static_cast<T>(
        (static_cast<typename reference<T, B>::word_type>(other._ref) & ~other_mask) |
        (static_cast<typename reference<T, B>::word_type>(my_val) << other_shift));
  }
}

// Swaps the value of the referenced bits with a bit value
template <class WordRef, size_t PrimitiveBits>
void reference<WordRef, PrimitiveBits>::swap(bit_value& other) const {
  if (other != *this) {
    word_type my_mask = _mask;
    size_t my_shift = countr_zero(my_mask);
    word_type my_val =
        (static_cast<word_type>(_ref) & my_mask) >> my_shift;
    bool other_val = static_cast<bool>(other);

    _ref = static_cast<WordRef>(
        (static_cast<word_type>(_ref) & ~my_mask) |
        (static_cast<word_type>(other_val) << my_shift));
    other = primitive<1>(static_cast<typename primitive<1>::storage_type>(my_val));
  }
}
// -------------------------------------------------------------------------- //



// Swaps two bit references
template <class T, class U>
void swap(reference<T> lhs, reference<U> rhs) noexcept {
  if (lhs != rhs) {
    bool l = static_cast<bool>(lhs);
    bool r = static_cast<bool>(rhs);
    lhs = bit_value(r);
    rhs = bit_value(l);
  }
}

// Swaps a bit reference and a bit value
template <class T>
void swap(reference<T> lhs, bit_value& rhs) noexcept {
  if (lhs != rhs) {
    bool l = static_cast<bool>(lhs);
    bool r = static_cast<bool>(rhs);
    lhs = bit_value(r);
    rhs = bit_value(l);
  }
}

// Swaps a bit value and a bit reference
template <class U>
void swap(bit_value& lhs, reference<U> rhs) noexcept {
  if (lhs != rhs) {
    bool l = static_cast<bool>(lhs);
    bool r = static_cast<bool>(rhs);
    lhs = bit_value(r);
    rhs = bit_value(l);
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
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>::reference(word_type* ptr) noexcept
    : _ref(*ptr), _mask(bit_mask()) {
}

// Privately explicitly constructs an unaligned bit reference from a pointer
template <class WordRef, size_t PrimitiveBits>
constexpr reference<WordRef, PrimitiveBits>::reference(
    word_type* ptr,
    size_type pos
) noexcept
    : _ref(*ptr), _mask(bit_mask() << pos) {
  assert(pos + PrimitiveBits <= binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //



// ========================================================================== //
} // namespace bit
#endif // _BIT_REFERENCE_HPP_INCLUDED
// ========================================================================== //
