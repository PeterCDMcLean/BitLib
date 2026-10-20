// =============================== BIT VALUE ================================ //
// Project:         The C++ Bit Library
// Name:            bit_value.hpp
// Description:     A class representing an independent, non-referenced bit
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_VALUE_HPP_INCLUDED
#define _BIT_VALUE_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
// Project sources
#include "bit_details.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {
// ========================================================================== //



/* ******************************* BIT VALUE ******************************** */
// Forward declaration of the generic N-bit primitive class.
// The primary template (Bits > 1) is defined in bit_array.hpp after the
// array class is available, breaking the recursive dependency:
//   primitive<1> (bit_value) does not depend on array
//   array<bit_value, ...> depends on bit_value (= primitive<1>)
//   primitive<Bits> (Bits > 1) depends on array<bit_value, ...>
template <size_t Bits>
class primitive;

// primitive<1> is the fundamental bit type, i.e. bit_value.
// It stores a single bool and does not depend on array.
template <>
class primitive<1> {
    // Friendship
    template <class, primitive_c> friend class reference;

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
    constexpr primitive(reference<T> ref) noexcept
        : _value(static_cast<bool>(ref)) {}
    template <class WordType>
    explicit constexpr primitive(WordType val) noexcept
        : _value(static_cast<bool>(val & 1)) {}
    explicit constexpr primitive(bool val) noexcept : _value(val) {}

    // Assignment
  public:
    template <class T>
    constexpr primitive& operator=(reference<T> ref) noexcept {
        _value = static_cast<bool>(ref);
        return *this;
    }
    template <class WordType>
    constexpr primitive& assign(WordType val) noexcept {
        _value = static_cast<bool>(val & 1);
        return *this;
    }

    // Bitwise assignment operators
  public:
    constexpr primitive& operator&=(primitive other) noexcept {
        _value &= other._value;
        return *this;
    }
    constexpr primitive& operator|=(primitive other) noexcept {
        _value |= other._value;
        return *this;
    }
    constexpr primitive& operator^=(primitive other) noexcept {
        _value ^= other._value;
        return *this;
    }

    // Conversion
  public:
    explicit constexpr operator bool() const noexcept { return _value; }
    constexpr word_type word_value() const noexcept { return static_cast<word_type>(_value); }

    // Bit manipulation
    constexpr primitive<1>& flip() noexcept { _value = !_value; return *this; }

    // Swap members
  public:
    void swap(primitive& other) noexcept {
        std::swap(_value, other._value);
    }
    template <class T>
    void swap(reference<T> other) noexcept {
        if (other != *this) {
            flip();
            other.flip();
        }
    }

    // Bitwise operators
  public:
    friend constexpr primitive operator~(
        primitive rhs
    ) noexcept {
        return primitive(!rhs._value);
    }

    friend constexpr primitive operator&(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return primitive(lhs._value & rhs._value);
    }

    friend constexpr primitive operator|(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return primitive(lhs._value | rhs._value);
    }

    friend constexpr primitive operator^(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return primitive(lhs._value ^ rhs._value);
    }

    // Comparison operators
  public:
    friend constexpr bool operator==(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return lhs._value == rhs._value;
    }

    friend constexpr bool operator!=(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return lhs._value != rhs._value;
    }

    friend constexpr bool operator<(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return lhs._value < rhs._value;
    }

    friend constexpr bool operator<=(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return lhs._value <= rhs._value;
    }

    friend constexpr bool operator>(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return lhs._value > rhs._value;
    }

    friend constexpr bool operator>=(
        primitive lhs,
        primitive rhs
    ) noexcept {
        return lhs._value >= rhs._value;
    }
};

// Alias: bit_value is a single bit, i.e. primitive<1>.
using bit_value = primitive<1>;

// Forward declaration of the array class template.
// Defined in bit_array.hpp.
template <typename T, std::size_t N, typename W, typename Policy>
class array;

// Stream functions
template <class CharT, class Traits, size_t Bits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    primitive<Bits>& x
);
template <class CharT, class Traits, size_t Bits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    primitive<Bits> x
);

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
    primitive<Bits>& x
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
    primitive<Bits> x
) {
    constexpr char zero = '0';
    constexpr char one = '1';
    return os << os.widen(static_cast<bool>(x) ? one : zero);
}


// ========================================================================== //
} // namespace bit
#endif // _BIT_VALUE_HPP_INCLUDED
// ========================================================================== //
