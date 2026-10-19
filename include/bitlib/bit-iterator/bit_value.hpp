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
// Bit value class definition - generic N-bit primitive
template <size_t Bits>
class primitive
{
    // Assertions
    static_assert(Bits >= 1, "primitive must have at least 1 bit");
    static_assert(
        Bits <= bitsof<std::size_t>(),
        "primitive bit width exceeds the native integer size of the architecture");
    static_assert(Bits <= bitsof<std::uint64_t>(),
                  "primitive supports at most 64 bits");

    // Friendship
    template <class> friend class bit_reference;

    // Types
  public:
    static constexpr std::size_t bits = Bits;
    using size_type = std::size_t;
    using storage_type = ceil_integral<Bits>;

  private:
    // Mask keeping only the low Bits bits significant
    static constexpr storage_type bit_mask()
    {
        if constexpr (Bits == bitsof<storage_type>()) {
            return ~storage_type(0);
        } else {
            return (storage_type(1) << Bits) - storage_type(1);
        }
    }

    // Implementation details: data members
  private:
    storage_type _value;

  public:
    // Lifecycle
    constexpr primitive() noexcept;
    template <class T>
    constexpr primitive(bit_reference<T> ref) noexcept;
    template <class WordType>
    explicit constexpr primitive(WordType val) noexcept;
    explicit constexpr primitive(bool val) noexcept;
    template <class WordType>
    constexpr primitive(WordType val, size_type pos);

    // Assignment
  public:
    template <class T>
    constexpr primitive& operator=(bit_reference<T> ref) noexcept;
    template <class WordType>
    constexpr primitive& assign(WordType val) noexcept;
    template <class WordType>
    constexpr primitive& assign(WordType val, size_type pos);

    // Bitwise assignment operators
  public:
    constexpr primitive& operator&=(primitive other) noexcept;
    constexpr primitive& operator|=(primitive other) noexcept;
    constexpr primitive& operator^=(primitive other) noexcept;

    // Conversion
  public:
    explicit constexpr operator bool() const noexcept;

    // Swap members
  public:
    void swap(primitive& other) noexcept;
    template <class T>
    void swap(bit_reference<T> other) noexcept;

    // Bit manipulation
  public:
    constexpr primitive& set(bool b) noexcept;
    constexpr primitive& set() noexcept;
    constexpr primitive& reset() noexcept;
    constexpr primitive& flip() noexcept;

    // Bitwise operators
  public:
    friend constexpr primitive operator~(
        primitive rhs
    ) noexcept
    {
        return primitive(static_cast<storage_type>(~rhs._value));
    }

    friend constexpr primitive operator&(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return primitive(static_cast<storage_type>(lhs._value & rhs._value));
    }

    friend constexpr primitive operator|(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return primitive(static_cast<storage_type>(lhs._value | rhs._value));
    }

    friend constexpr primitive operator^(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return primitive(static_cast<storage_type>(lhs._value ^ rhs._value));
    }

    // Comparison operators
  public:
    friend constexpr bool operator==(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return lhs._value == rhs._value;
    }

    friend constexpr bool operator!=(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return lhs._value != rhs._value;
    }

    friend constexpr bool operator<(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return lhs._value < rhs._value;
    }

    friend constexpr bool operator<=(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return lhs._value <= rhs._value;
    }

    friend constexpr bool operator>(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return lhs._value > rhs._value;
    }

    friend constexpr bool operator>=(
        primitive lhs,
        primitive rhs
    ) noexcept
    {
        return lhs._value >= rhs._value;
    }
};

// Alias: bit_value is a single bit, i.e. a partial-default instantiation
// of the generic primitive<Bits> class with Bits fixed to 1.
using bit_value = primitive<1>;

// Stream functions
template <class CharT, class Traits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    bit_value& x
);
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    bit_value x
);

// Constants
/* ************************************************************************** */


// -------------------------- BIT VALUE: LIFECYCLE -------------------------- //
// Implicitly default constructs a bit value initialized to zero
template <size_t Bits>
constexpr primitive<Bits>::primitive(
) noexcept
: _value(storage_type(0))
{
}

// Implicitly constructs a bit value from a bit reference
template <size_t Bits>
template <class T>
constexpr primitive<Bits>::primitive(
    bit_reference<T> ref
) noexcept
: _value(static_cast<storage_type>(static_cast<bool>(ref)))
{
}

// Explicitly constructs a bit from a bool
template <size_t Bits>
constexpr primitive<Bits>::primitive(
    bool b
) noexcept
: _value(static_cast<storage_type>(b))
{
}

// Explicitly constructs an aligned bit value
template <size_t Bits>
template <class WordType>
constexpr primitive<Bits>::primitive(
    WordType val
) noexcept
: _value(static_cast<storage_type>(val) & bit_mask())
{
    static_assert(binary_digits<WordType>::value, "");
}

// Explicitly constructs an unaligned bit value
template <size_t Bits>
template <class WordType>
constexpr primitive<Bits>::primitive(
    WordType val,
    size_type pos
)
: _value((assert(pos + Bits <= binary_digits<WordType>::value),
          static_cast<storage_type>(val >> pos) & bit_mask()))
{
    static_assert(binary_digits<WordType>::value, "");
}
// -------------------------------------------------------------------------- //


// ------------------------- BIT VALUE: ASSIGNMENT -------------------------- //
// Assigns a bit reference to the bit value
template <size_t Bits>
template <class T>
constexpr primitive<Bits>& primitive<Bits>::operator=(
    bit_reference<T> ref
) noexcept
{
    _value = static_cast<storage_type>(static_cast<bool>(ref));
    return *this;
}

// Assigns the aligned bit of a value to the bit value
template <size_t Bits>
template <class WordType>
constexpr primitive<Bits>& primitive<Bits>::assign(
    WordType val
) noexcept
{
    static_assert(binary_digits<WordType>::value, "");
    _value = static_cast<storage_type>(val) & bit_mask();
    return *this;
}

// Assigns an unaligned bit of a value to the bit value
template <size_t Bits>
template <class WordType>
constexpr primitive<Bits>& primitive<Bits>::assign(
    WordType val,
    size_type pos
)
{
    assert(pos + Bits <= binary_digits<WordType>::value);
    _value = static_cast<storage_type>(val >> pos) & bit_mask();
    return *this;
}
// -------------------------------------------------------------------------- //


// ---------------- BIT VALUE: BITWISE ASSIGNMENT OPERATORS ----------------- //
// Assigns the value of the bit through a bitwise and operation
template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::operator&=(
    primitive other
) noexcept
{
    _value &= other._value;
    return *this;
}

// Assigns the value of the bit through a bitwise or operation
template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::operator|=(
    primitive other
) noexcept
{
    _value |= other._value;
    return *this;
}

// Assigns the value of the bit through a bitwise xor operation
template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::operator^=(
    primitive other
) noexcept
{
    _value ^= other._value;
    return *this;
}
// -------------------------------------------------------------------------- //


// ------------------------- BIT VALUE: CONVERSION -------------------------- //
// Explicitly converts the bit value to a boolean value
template <size_t Bits>
constexpr primitive<Bits>::operator bool(
) const noexcept
{
    return _value != storage_type(0);
}
// -------------------------------------------------------------------------- //


// ------------------------ BIT VALUE: SWAP MEMBERS ------------------------- //
// Swaps the bit value with another bit value
template <size_t Bits>
inline void primitive<Bits>::swap(
    primitive& other
) noexcept
{
    std::swap(*this, other);
}

// Swaps the bit value with the value of a bit reference
template <size_t Bits>
template <class T>
void primitive<Bits>::swap(
    bit_reference<T> other
) noexcept
{
    if (static_cast<bool>(other) != static_cast<bool>(_value)) {
        flip();
        other.flip();
    }
}
// -------------------------------------------------------------------------- //


// ---------------------- BIT VALUE: BIT MANIPULATION ----------------------- //
// Sets the value of the bit to the provided boolean value
template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::set(
    bool b
) noexcept
{
    _value = static_cast<storage_type>(b);
    return *this;
}

// Sets the value of the bit to 1
template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::set(
) noexcept
{
    _value = bit_mask();
    return *this;
}

// Resets the value of the bit to 0
template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::reset(
) noexcept
{
    _value = storage_type(0);
    return *this;
}

// Flips the value of the bit
template <size_t Bits>
constexpr primitive<Bits>& primitive<Bits>::flip(
) noexcept
{
    _value = ~_value & bit_mask();
    return *this;
}
// -------------------------------------------------------------------------- //


// ---------------------- BIT VALUE: STREAM FUNCTIONS ----------------------- //
// Extracts a bit value from an input stream
template <class CharT, class Traits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    bit_value& x
)
{
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
        } catch(...) {
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

// Inserts a bit value in an output stream
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    bit_value x
)
{
    constexpr char zero = '0';
    constexpr char one = '1';
    return os << os.widen(x ? one : zero);
}
// -------------------------------------------------------------------------- //


// -------------------------- BIT VALUE: CONSTANTS -------------------------- //
// Constant bit values
constexpr bit_value bit_off(0U);
constexpr bit_value bit_on(1U);
constexpr bit_value bit0(0U);
constexpr bit_value bit1(1U);
// -------------------------------------------------------------------------- //


// ========================================================================== //
} // namespace bit
#endif // _BIT_VALUE_HPP_INCLUDED
// ========================================================================== //
