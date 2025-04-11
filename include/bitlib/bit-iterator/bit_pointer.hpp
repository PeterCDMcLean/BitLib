// ============================== BIT POINTER =============================== //
// Project:         The C++ Bit Library
// Name:            bit_pointer.hpp
// Description:     A class representing a pointer to a bit
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_POINTER_HPP_INCLUDED
#define _BIT_POINTER_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
// Project sources
#include <type_traits>

#include "../bit-containers/bit_bitsof.hpp"
#include "bit_details.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {
// ========================================================================== //



/* ****************************** BIT POINTER ******************************* */
// Bit pointer class definition
template <class WordType>
class bit_pointer
{
    // Assertions
    static_assert(binary_digits<WordType>::value, "");

    // Friendship
    template <class> friend class bit_pointer;

    // Types
    public:
    using word_type = WordType;
    using mask_type = std::make_unsigned_t<std::remove_cv_t<word_type>>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Lifecycle
    public:
    constexpr bit_pointer() noexcept;
    constexpr bit_pointer(const bit_pointer& other) noexcept;
    template <class T>
    constexpr bit_pointer(const bit_pointer<T>& other) noexcept;
    constexpr bit_pointer(std::nullptr_t) noexcept;
    explicit constexpr bit_pointer(word_type* ptr) noexcept;
    constexpr bit_pointer(word_type* ptr, size_type pos);

    // Assignment
    public:
    constexpr bit_pointer& operator=(std::nullptr_t) noexcept;
    constexpr bit_pointer& operator=(const bit_pointer& other) noexcept;
    template <class T>
    constexpr bit_pointer& operator=(const bit_pointer<T>& other) noexcept;

    // Conversion
    public:
    explicit constexpr operator bool() const noexcept;

    // Access
    public:
     constexpr bit_reference<WordType> operator*() const noexcept;
     constexpr bit_reference<WordType> operator[](difference_type n) const;

     // Increment and decrement operators
    public:
    constexpr bit_pointer& operator++();
    constexpr bit_pointer& operator--();
    constexpr bit_pointer operator++(int);
    constexpr bit_pointer operator--(int);
    constexpr bit_pointer operator+(difference_type n) const;
    constexpr bit_pointer operator-(difference_type n) const;
    constexpr bit_pointer& operator+=(difference_type n);
    constexpr bit_pointer& operator-=(difference_type n);
    constexpr size_t position() const { return _tzcnt(_mask); }
    constexpr word_type* address() const { return _ptr; }

    // Implementation details: data members
    private:
     word_type* _ptr;
     mask_type _mask;

     // Non-member arithmetic operators
     template <class T>
     friend constexpr bit_pointer<T> operator+(typename bit_pointer<T>::difference_type n, bit_pointer<T> x);
     template <class T, class U>
     friend constexpr typename std::common_type<
         typename bit_pointer<T>::difference_type,
         typename bit_pointer<U>::difference_type>::type
     operator-(bit_pointer<T> lhs, bit_pointer<U> rhs);

     // Comparison operators
     template <class T, class U>
     friend constexpr bool operator==(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept;
     template <class T, class U>
     friend constexpr bool operator!=(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept;
     template <class T, class U>
     friend constexpr bool operator<(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept;
     template <class T, class U>
     friend constexpr bool operator<=(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept;
     template <class T, class U>
     friend constexpr bool operator>(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept;
     template <class T, class U>
     friend constexpr bool operator>=(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept;
};

static_assert(bit_pointer_c<bit_pointer<uint8_t>, bit_reference<uint8_t>>, "bit_pointer does not satisfy bit_pointer_c concept!");
/* ************************************************************************** */



// ------------------------- BIT POINTER: LIFECYCLE ------------------------- //
// Implicitly default constructs a null bit pointer
template <class WordType>
constexpr bit_pointer<WordType>::bit_pointer() noexcept
    : _ptr(nullptr), _mask(0) {
}

template <class WordType>
constexpr bit_pointer<WordType>::bit_pointer(const bit_pointer<WordType>& other) noexcept
    : _ptr(other._ptr), _mask(other._mask) {
}

// Implicitly constructs a bit pointer from another bit pointer
template <class WordType>
template <class T>
constexpr bit_pointer<WordType>::bit_pointer(const bit_pointer<T>& other) noexcept
    : _ptr(other._ptr), _mask(other._mask) {
}

// Explicitly constructs a bit pointer from a null pointer
template <class WordType>
constexpr bit_pointer<WordType>::bit_pointer(std::nullptr_t) noexcept
    : _ptr(nullptr), _mask(0) {
}

// Explicitly constructs an aligned bit pointer from a pointer
template <class WordType>
constexpr bit_pointer<WordType>::bit_pointer(word_type* ptr) noexcept
    : _ptr(ptr), _mask(1) {
}

// Explicitly constructs an unaligned bit pointer from a pointer
template <class WordType>
constexpr bit_pointer<WordType>::bit_pointer(
    word_type* ptr,
    size_type pos)
    : _ptr(ptr), _mask(1 << pos) {
}
// -------------------------------------------------------------------------- //

// ------------------------ BIT POINTER: ASSIGNMENT ------------------------- //
// Assigns a null pointer to the bit pointer
template <class WordType>
constexpr bit_pointer<WordType>& bit_pointer<WordType>::operator=(
    std::nullptr_t) noexcept {
  _ptr = nullptr;
  _mask = 0;
  return *this;
}

// Copies a bit pointer to the bit pointer
template <class WordType>
constexpr bit_pointer<WordType>& bit_pointer<WordType>::operator=(const bit_pointer& other) noexcept {
  _ptr = other._ptr;
  _mask = other._mask;
  return *this;
}

// Assigns a bit pointer to the bit pointer
template <class WordType>
template <class T>
constexpr bit_pointer<WordType>& bit_pointer<WordType>::operator=(const bit_pointer<T>& other) noexcept {
  _ptr = other._ptr;
  _mask = other._mask;
  return *this;
}
// -------------------------------------------------------------------------- //



// ------------------------ BIT POINTER: CONVERSION ------------------------- //
// Returns true if the bit pointer is null, false otherwise
template <class WordType>
constexpr bit_pointer<WordType>::operator bool() const noexcept {
  return (_ptr == nullptr);
}
// -------------------------------------------------------------------------- //



// -------------------------- BIT POINTER: ACCESS --------------------------- //
// Gets a bit reference from the bit pointer
template <class WordType>
constexpr bit_reference<WordType> bit_pointer<WordType>::operator*() const noexcept {
  return bit_reference<WordType>(*_ptr, _mask);
}

// Gets a bit reference, decrementing or incrementing the pointer
template <class WordType>
constexpr bit_reference<WordType> bit_pointer<WordType>::operator[](difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = position() + n;
  difference_type diff = sum / digits;
  if (sum < 0 && diff * digits != sum) {
    --diff;
  }
  return bit_reference<WordType>(_ptr + diff, sum - diff * digits);
}
// -------------------------------------------------------------------------- //



// ------------- BIT POINTER: INCREMENT AND DECREMENT OPERATORS ------------- //
// Increments the bit pointer and returns it
template <class WordType>
constexpr bit_pointer<WordType>& bit_pointer<WordType>::operator++() {
  _mask <<= 1;
  if (_mask == 0) {
    _mask = 1;
    _ptr++;
  }
  return *this;
}

// Decrements the bit pointer and returns it
template <class WordType>
constexpr bit_pointer<WordType>& bit_pointer<WordType>::operator--() {
  _mask >>= 1;
  if (_mask == 0) {
    _mask = mask_type(1) << (bitsof<mask_type>() - 1);
    _ptr--;
  }
  return *this;
}

// Increments the bit pointer and returns the old one
template <class WordType>
constexpr bit_pointer<WordType> bit_pointer<WordType>::operator++(int) {
  bit_pointer old = *this;
  ++(*this);
  return old;
}

// Decrements the bit pointer and returns the old one
template <class WordType>
constexpr bit_pointer<WordType> bit_pointer<WordType>::operator--(int) {
  bit_pointer old = *this;
  --(*this);
  return old;
}

// Looks forward several bits and gets a pointer at this position
template <class WordType>
constexpr bit_pointer<WordType> bit_pointer<WordType>::operator+(difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = position() + n;
  difference_type diff = sum / digits;
  if (sum < 0 && diff * digits != sum) {
    --diff;
  }
  return bit_pointer(_ptr + diff, sum - diff * digits);
}

// Looks backward several bits and gets a pointer at this position
template <class WordType>
constexpr bit_pointer<WordType> bit_pointer<WordType>::operator-(difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = position() - n;
  difference_type diff = sum / digits;
  if (sum < 0 && diff * digits != sum) {
    --diff;
  }
  return bit_pointer(_ptr + diff, sum - diff * digits);
}

// Increments the pointer by several bits and returns it
template <class WordType>
constexpr bit_pointer<WordType>& bit_pointer<WordType>::operator+=(difference_type n) {
  *this = *this + n;
  return *this;
}

// Decrements the pointer by several bits and returns it
template <class WordType>
constexpr bit_pointer<WordType>& bit_pointer<WordType>::operator-=(difference_type n) {
  *this = *this - n;
  return *this;
}
// -------------------------------------------------------------------------- //



// -------------- BIT POINTER: NON-MEMBER ARITHMETIC OPERATORS -------------- //
// Advances the bit pointer several times
template <class T>
constexpr bit_pointer<T> operator+(typename bit_pointer<T>::difference_type n, bit_pointer<T> x) {
  return x + n;
}

// Computes the distance in bits separating two bit pointers
template <class T, class U>
constexpr typename std::common_type<
    typename bit_pointer<T>::difference_type,
    typename bit_pointer<U>::difference_type>::type
operator-(bit_pointer<T> lhs, bit_pointer<U> rhs) {
  using lhs_type = typename bit_pointer<T>::difference_type;
  using rhs_type = typename bit_pointer<U>::difference_type;
  using difference_type = typename std::common_type<lhs_type, rhs_type>::type;
  constexpr difference_type lhs_digits = binary_digits<T>::value;
  constexpr difference_type rhs_digits = binary_digits<U>::value;
  constexpr difference_type digits = rhs_digits;
  static_assert(lhs_digits == rhs_digits, "");
  const difference_type main = lhs._ptr - rhs._ptr;
  return main * digits + (lhs.position() - rhs.position());
}
// -------------------------------------------------------------------------- //



// ------------------- BIT POINTER: COMPARISON OPERATORS -------------------- //
// Checks if the left hand side is equal to the right hand side
template <class T, class U>
constexpr bool operator==(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept {
  return lhs._ptr == rhs._ptr && lhs.position() == rhs.position();
}

// Checks if the left hand side is non equal to the right hand side
template <class T, class U>
constexpr bool operator!=(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept {
  return lhs._ptr != rhs._ptr || lhs.position() != rhs.position();
}

// Checks if the left hand side is less than the right hand side
template <class T, class U>
constexpr bool operator<(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept {
  return lhs._ptr < rhs._ptr || (lhs._ptr == rhs._ptr && lhs.position() < rhs.position());
}

// Checks if the left hand side is less than or equal to the right hand side
template <class T, class U>
constexpr bool operator<=(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept {
  return lhs._ptr < rhs._ptr || (lhs._ptr == rhs._ptr && lhs.position() <= rhs.position());
}

// Checks if the left hand side is greater than the right hand side
template <class T, class U>
constexpr bool operator>(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept {
  return lhs._ptr > rhs._ptr || (lhs._ptr == rhs._ptr && lhs.position() > rhs.position());
}

// Checks if the left hand side is greater than or equal to the right hand side
template <class T, class U>
constexpr bool operator>=(bit_pointer<T> lhs, bit_pointer<U> rhs) noexcept {
  return lhs._ptr > rhs._ptr || (lhs._ptr == rhs._ptr && lhs.position() >= rhs.position());
}
// -------------------------------------------------------------------------- //



// ========================================================================== //
}  // namespace bit
#endif // _BIT_POINTER_HPP_INCLUDED
// ========================================================================== //
