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
#include "bit_details.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {
// ========================================================================== //



/* ***************************** BIT REFERENCE ****************************** */
// Bit reference class definition
template <typename WordType, typename MaskType=std::remove_cv<WordType>::type>
class bit_reference
{
    // Assertions
    static_assert(binary_digits<WordType>::value, "");

    // Friendship
    template <typename, typename> friend class bit_reference;
    friend class bit_pointer<WordType>;

    // Types
    public:
    using word_type = WordType;
    using size_type = std::size_t;

    // Lifecycle
    public:
    template <typename T, typename M>
    constexpr bit_reference(const bit_reference<T, M>& other) noexcept;
    constexpr bit_reference(const bit_reference& other) noexcept;
    explicit constexpr bit_reference(word_type& ref) noexcept;
    constexpr bit_reference(word_type& ref, size_type pos);

    // Assignment
    public:
    constexpr bit_reference& operator=(const bit_reference& other) noexcept;
    template <typename T, typename M>
    constexpr bit_reference& operator=(const bit_reference<T, M>& other) noexcept;
    constexpr bit_reference& operator=(const bit_value val) noexcept;
    constexpr bit_reference& assign(word_type val) noexcept;
    constexpr bit_reference& assign(word_type val, size_type pos);

    // Bitwise assignment operators
    public:
    constexpr bit_reference& operator&=(bit_value other) noexcept;
    constexpr bit_reference& operator|=(bit_value other) noexcept;
    constexpr bit_reference& operator^=(bit_value other) noexcept;

    // Conversion
    public:
    explicit constexpr operator bool() const noexcept;

    // Access
    public:
    constexpr bit_pointer<WordType> operator&() const noexcept;

    // Swap members
    public:
    template <typename T>
    void swap(bit_reference<T> other);
    void swap(bit_value& other);

    // Bit manipulation
    public:
    constexpr bit_reference& set(bool b) noexcept;
    constexpr bit_reference& set() noexcept;
    constexpr bit_reference& reset() noexcept;
    constexpr bit_reference& flip() noexcept;

    // Underlying details
    public:
    constexpr word_type* address() const noexcept;
    constexpr size_type position() const noexcept;
    constexpr typename std::remove_cv<word_type>::type mask() const noexcept;

    // Implementation details: function members
    private:
    bit_reference() noexcept = default;
    explicit constexpr bit_reference(std::nullptr_t) noexcept;
    explicit constexpr bit_reference(word_type* ptr) noexcept;
    constexpr bit_reference(word_type* ptr, size_type pos);

    // Implementation details: data members
    private:
    word_type* _ptr;
    MaskType _mask;
};

// Swap
template <typename T, typename U>
void swap(
    bit_reference<T> lhs,
    bit_reference<U> rhs
) noexcept;
template <typename T>
void swap(
    bit_reference<T> lhs,
    bit_value& rhs
) noexcept;
template <typename U>
void swap(
    bit_value& lhs,
    bit_reference<U> rhs
) noexcept;

// Stream functions
template <typename CharT, typename Traits, typename T>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    bit_reference<T>& x
);
template <typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    bit_reference<T> x
);
/* ************************************************************************** */



// ------------------------ BIT REFERENCE: LIFECYCLE ------------------------ //
// Implicitly constructs a bit reference from another bit reference
template <typename WordType, typename MaskType>
template <typename T,typename M>
constexpr bit_reference<WordType,MaskType>::bit_reference(
    const bit_reference<T,M>& other
) noexcept
: _ptr(other._ptr)
, _mask(other._mask)
{
}

template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>::bit_reference(
    const bit_reference<WordType,MaskType>& other
) noexcept
: _ptr(other._ptr)
, _mask(other._mask)
{
}

// Explicitly constructs an aligned bit reference
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>::bit_reference(
    word_type& ref
) noexcept
: _ptr(&ref)
, _mask(1)
{
}

// Explicitly constructs an unaligned bit reference
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>::bit_reference(
    word_type& ref,
    size_type pos
)
: _ptr((assert(pos < binary_digits<word_type>::value), &ref))
, _mask(static_cast<word_type>(1) << pos)
{
}
// -------------------------------------------------------------------------- //



// ----------------------- BIT REFERENCE: ASSIGNMENT ------------------------ //
// Copies a bit reference to the bit reference
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::operator=(
    const bit_reference& other
) noexcept
{
    other ? set() : reset();
    return *this;
}

// Assigns a bit reference to the bit reference
template <typename WordType, typename MaskType>
template <typename T, typename M>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::operator=(
    const bit_reference<T,M>& other
) noexcept
{
    other ? set() : reset();
    return *this;
}

// Assigns a bit value to the bit reference
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::operator=(
    const bit_value val
) noexcept
{
    val ? set() : reset();
    return *this;
}

// Assigns the aligned bit of a value to the bit reference
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::assign(
    word_type val
) noexcept
{
    val & 1 ? set() : reset();
    return *this;
}

// Assigns an unaligned bit of a value to the bit reference
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::assign(
    word_type val,
    size_type pos
)
{
    assert(pos < binary_digits<word_type>::value);
    val >> pos & 1 ? set() : reset();
    return *this;
}
// -------------------------------------------------------------------------- //



// -------------- BIT REFERENCE: BITWISE ASSIGNMENT OPERATORS --------------- //
// Assigns the value of the referenced bit through a bitwise and operation
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::operator&=(
    bit_value other
) noexcept
{
    *_ptr &= ~(_mask * static_cast<word_type>(!other._value));
    return *this;
}

// Assigns the value of the referenced bit through a bitwise or operation
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::operator|=(
    bit_value other
) noexcept
{
    *_ptr |= _mask * static_cast<word_type>(other._value);
    return *this;
}

// Assigns the value of the referenced bit through a bitwise xor operation
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::operator^=(
    bit_value other
) noexcept
{
    *_ptr ^= _mask * static_cast<word_type>(other._value);
    return *this;
}
// -------------------------------------------------------------------------- //



// ----------------------- BIT REFERENCE: CONVERSION ------------------------ //
// Explicitly converts the bit reference to a boolean value
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>::operator bool(
) const noexcept
{
    return *_ptr & _mask;
}
// -------------------------------------------------------------------------- //



// ------------------------- BIT REFERENCE: ACCESS -------------------------- //
// Gets a bit pointer from the bit reference
template <typename WordType, typename MaskType>
constexpr bit_pointer<WordType> bit_reference<WordType,MaskType>::operator&(
) const noexcept
{
    return bit_pointer<WordType>(_ptr, position());
}
// -------------------------------------------------------------------------- //



// ---------------------- BIT REFERENCE: SWAP MEMBERS ----------------------- //
// Swaps the value of the referenced bit with another bit reference
template <typename WordType, typename MaskType>
template <typename T>
void bit_reference<WordType,MaskType>::swap(
    bit_reference<T> other
)
{
    if (other != *this) {
        flip();
        other.flip();
    }
}

// Swaps the value of the referenced bit with a bit value
template <typename WordType, typename MaskType>
void bit_reference<WordType,MaskType>::swap(
    bit_value& other
)
{
    if (other != *this) {
        flip();
        other.flip();
    }
}
// -------------------------------------------------------------------------- //



// -------------------- BIT REFERENCE: BIT MANIPULATION --------------------- //
// Sets the value of the referenced bit to the provided boolean value
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::set(
    bool b
) noexcept
{
    b ? set() : reset();
    return *this;
}

// Sets the value of the referenced bit to 1
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::set(
) noexcept
{
    *_ptr |= _mask;
    return *this;
}

// Resets the value of the referenced bit to 0
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::reset(
) noexcept
{
    *_ptr &= ~_mask;
    return *this;
}

// Flips the value of the referenced bit
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>& bit_reference<WordType,MaskType>::flip(
) noexcept
{
    *_ptr ^= _mask;
    return *this;
}
// -------------------------------------------------------------------------- //



// ------------------- BIT REFERENCE: UNDERLYING DETAILS -------------------- //
// Returns a pointer to the underlying word
template <typename WordType, typename MaskType>
constexpr typename bit_reference<WordType,MaskType>::word_type*
bit_reference<WordType,MaskType>::address(
) const noexcept
{
    return _ptr;
}

// Returns the position of the referenced bit within the underlying word
template <typename WordType, typename MaskType>
constexpr typename bit_reference<WordType,MaskType>::size_type
bit_reference<WordType,MaskType>::position(
) const noexcept
{
    return _tzcnt(_mask);
}

// Returns a mask corresponding to the referenced bit
template <typename WordType, typename MaskType>
constexpr typename std::remove_cv<
    typename bit_reference<WordType,MaskType>::word_type
>::type bit_reference<WordType,MaskType>::mask(
) const noexcept
{
    return _mask;
}
// -------------------------------------------------------------------------- //



// -------------------------- BIT REFERENCE: SWAP --------------------------- //
// Swaps two bit references
template <typename T, typename U>
void swap(
    bit_reference<T> lhs,
    bit_reference<U> rhs
) noexcept
{
    if (lhs != rhs) {
        lhs.flip();
        rhs.flip();
    }
}

// Swaps a bit reference and a bit value
template <typename T>
void swap(
    bit_reference<T> lhs,
    bit_value& rhs
) noexcept
{
    if (lhs != rhs) {
        lhs.flip();
        rhs.flip();
    }
}

// Swaps a bit value and a bit reference
template <typename U>
void swap(
    bit_value& lhs,
    bit_reference<U> rhs
) noexcept
{
    if (lhs != rhs) {
        lhs.flip();
        rhs.flip();
    }
}
// -------------------------------------------------------------------------- //



// -------------------- BIT REFERENCE: STREAM FUNCTIONS --------------------- //
// Extracts a bit reference from an input stream
template <typename CharT, typename Traits, typename T>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    bit_reference<T>& x
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

// Inserts a bit reference in an output stream
template <typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    bit_reference<T> x
)
{
    constexpr char zero = '0';
    constexpr char one = '1';
    return os << os.widen(x ? one : zero);
}
// -------------------------------------------------------------------------- //



// -------- BIT REFERENCE: IMPLEMENTATION DETAILS: FUNCTION MEMBERS --------- //
// Privately explicitly constructs a bit reference from a nullptr
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>::bit_reference(
    std::nullptr_t
) noexcept
: _ptr(nullptr)
, _mask()
{
}

// Privately explicitly constructs an aligned bit reference from a pointer
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>::bit_reference(
    word_type* ptr
) noexcept
: _ptr(ptr)
, _mask(1)
{
}

// Privately explicitly constructs an unaligned bit reference from a pointer
template <typename WordType, typename MaskType>
constexpr bit_reference<WordType,MaskType>::bit_reference(
    word_type* ptr,
    size_type pos
)
: _ptr((assert(pos < binary_digits<word_type>::value), ptr))
, _mask(static_cast<word_type>(1) << pos)
{
}
// -------------------------------------------------------------------------- //



// ========================================================================== //
} // namespace bit
#endif // _BIT_REFERENCE_HPP_INCLUDED
// ========================================================================== //
