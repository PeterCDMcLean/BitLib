// ============================== BIT ITERATOR ============================== //
// Project:         The C++ Bit Library
// Name:            bit_iterator.hpp
// Description:     A class representing an iterator on bit sequences
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ITERATOR_HPP_INCLUDED
#define _BIT_ITERATOR_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
#include <compare>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit_concepts.hpp"
// Project sources
#include "bit_details.hpp"
#include "bit_reference.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {

/* ****************************** BIT ITERATOR ****************************** */

template <typename target_word_ptr, typename source_word_ptr>
class bit_word_pointer_adapter;

// Bit iterator class definition
template <class Iterator>
class bit_iterator
{
  template <class>
  friend class bit_iterator;

  // Assertions
 private:
  using _traits_t = _cv_iterator_traits<Iterator>;
  static_assert(binary_digits<std::iter_value_t<Iterator>>::value, "");

  // Types
 public:
  using iterator_type = Iterator;
  using word_type = std::iter_value_t<Iterator>;
  using iterator_category = typename _traits_t::iterator_category;
  using value_type = bit_value;
  using difference_type = std::ptrdiff_t;
  using pointer = bit_pointer<word_type>;
  using reference = bit_reference<typename _traits_t::reference>;  // typename _traits_t::reference;
  using size_type = std::size_t;
  using mask_type = std::make_unsigned_t<std::remove_cv_t<word_type>>;

  // Lifecycle
 public:
  constexpr bit_iterator();
  constexpr bit_iterator(const bit_iterator<Iterator>& other);
  template <class T>
  constexpr bit_iterator(const bit_iterator<T>& other)
    requires std::constructible_from<iterator_type, T>;
  explicit constexpr bit_iterator(iterator_type i);
  constexpr bit_iterator(iterator_type i, size_type pos);
  explicit constexpr bit_iterator(const pointer& ptr)
    requires std::constructible_from<iterator_type, word_type*>;

  template <typename T>
  constexpr bit_iterator(const bit_iterator<bit_word_pointer_adapter<T, Iterator>>& other);

  // Assignment
 public:
  constexpr bit_iterator& operator=(const bit_iterator<Iterator>& other);
  template <class T>
  constexpr bit_iterator& operator=(const bit_iterator<T>& other);

  // Access
 public:
  constexpr reference operator*() const noexcept;
  constexpr pointer operator->() const noexcept;
  constexpr reference operator[](difference_type n) const;

  // Increment and decrement operators
 public:
  constexpr bit_iterator& operator++();
  constexpr bit_iterator& operator--();
  constexpr bit_iterator operator++(int);
  constexpr bit_iterator operator--(int);
  constexpr bit_iterator operator+(difference_type n) const;
  constexpr bit_iterator operator-(difference_type n) const;
  constexpr bit_iterator& operator+=(difference_type n);
  constexpr bit_iterator& operator-=(difference_type n);
  constexpr difference_type operator-(const bit_iterator&) const;

  // Underlying details
 public:
  constexpr iterator_type base() const;
  constexpr iterator_type address() const;
  constexpr size_type position() const noexcept;

  auto operator<=>(const bit_iterator&) const = default;
  // Implementation details: data members
 private:
  iterator_type _current;
  size_type _position;

  // Non-member arithmetic operators
  template <class T>
  friend constexpr bit_iterator<T> operator+(
      typename bit_iterator<T>::difference_type n,
      const bit_iterator<T>& i);
  template <class T, class U>
  friend constexpr typename std::common_type<
      typename bit_iterator<T>::difference_type,
      typename bit_iterator<U>::difference_type>::type
  operator-(
      const bit_iterator<T>& lhs,
      const bit_iterator<U>& rhs);
};
/* ************************************************************************** */



// ------------------------ BIT ITERATOR: LIFECYCLE ------------------------- //
// Implicitly default constructs a bit iterator
template <class Iterator>
constexpr bit_iterator<Iterator>::bit_iterator()
    : _current(), _position() {
}

// Implicitly constructs a bit iterator from another bit iterator
template <class Iterator>
constexpr bit_iterator<Iterator>::bit_iterator(const bit_iterator<Iterator>& other)
    : _current(other.base()), _position(other.position()) {
  assert(_position < bitsof<word_type>());
}

template <class Iterator>
template <class T>
constexpr bit_iterator<Iterator>::bit_iterator(const bit_iterator<T>& other)
  requires std::constructible_from<iterator_type, T>
    : _current(other.base()), _position(other.position()) {
  assert(_position < bitsof<word_type>());
}

// Explicitly constructs an aligned bit iterator from an iterator
template <class Iterator>
constexpr bit_iterator<Iterator>::bit_iterator(const iterator_type i)
    : _current(i), _position(0) {
}

// Explicitly constructs an unaligned bit iterator from an iterator
template <class Iterator>
constexpr bit_iterator<Iterator>::bit_iterator(const iterator_type i, size_type pos)
    : _current(i), _position((assert(pos < binary_digits<word_type>::value), pos)) {
}

// Explicitly constructs an unaligned bit iterator from a pointer
template <class Iterator>
constexpr bit_iterator<Iterator>::bit_iterator(const pointer& ptr)
  requires std::constructible_from<iterator_type, word_type*>
    : _current(ptr._current), _position(ptr.position()) {
  assert(_position < bitsof<word_type>());
}

template <class Iterator>
template <typename T>
constexpr bit_iterator<Iterator>::bit_iterator(const bit_iterator<bit_word_pointer_adapter<T, Iterator>>& other)
    : _current(other.base().base()), _position(other.base().index() * bitsof<typename bit_word_pointer_adapter<T, Iterator>::value_type>() + other.position()) {
  assert(_position < bitsof<word_type>());
}
// -------------------------------------------------------------------------- //



// ------------------------ BIT ITERATOR: ASSIGNMENT ------------------------ //
// Assigns a bit iterator to the bit iterator
template <class Iterator>
constexpr bit_iterator<Iterator>& bit_iterator<Iterator>::operator=(
    const bit_iterator<Iterator>& other) {
  _current = other._current;
  _position = other._position;
  assert(_position < bitsof<word_type>());
  return *this;
}

// Assigns a bit iterator to the bit iterator
template <class Iterator>
template <class T>
constexpr bit_iterator<Iterator>& bit_iterator<Iterator>::operator=(
    const bit_iterator<T>& other) {
  _current = other._current;
  _position = other._position;
  assert(_position < bitsof<word_type>());
  return *this;
}
// -------------------------------------------------------------------------- //



// -------------------------- BIT ITERATOR: ACCESS -------------------------- //
// Gets a bit reference from the bit iterator
template <class Iterator>
constexpr typename bit_iterator<Iterator>::reference
bit_iterator<Iterator>::operator*() const noexcept {
  return reference(*_current, _position);
}

// Gets a pointer to a bit
template <class Iterator>
constexpr typename bit_iterator<Iterator>::pointer bit_iterator<Iterator>::operator->() const noexcept {
  return pointer(&*_current, _position);
}

// Gets a bit reference, decrementing or incrementing the iterator
template <class Iterator>
constexpr typename bit_iterator<Iterator>::reference bit_iterator<Iterator>::operator[](difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = _position + n;
  difference_type diff = sum / digits;
  if (sum < 0 && diff * digits != sum) {
    --diff;
  }
  return reference(*std::next(_current, diff), sum - diff * digits);
}
// -------------------------------------------------------------------------- //



// ------------- BIT ITERATOR: INCREMENT AND DECREMENT OPERATORS ------------ //
// Increments the bit iterator and returns it
template <class Iterator>
constexpr bit_iterator<Iterator>& bit_iterator<Iterator>::operator++() {
  constexpr size_type digits = binary_digits<word_type>::value;
  if (_position + 1 < digits) {
    ++_position;
  } else {
    ++_current;
    _position = 0;
  }
  return *this;
}

// Decrements the bit iterator and returns it
template <class Iterator>
constexpr bit_iterator<Iterator>& bit_iterator<Iterator>::operator--() {
  constexpr size_type digits = binary_digits<word_type>::value;
  if (_position) {
    --_position;
  } else {
    --_current;
    _position = digits - 1;
  }
  return *this;
}

// Increments the bit iterator and returns the old one
template <class Iterator>
constexpr bit_iterator<Iterator> bit_iterator<Iterator>::operator++(int) {
  bit_iterator old = *this;
  ++(*this);
  return old;
}

// Decrements the bit iterator and returns the old one
template <class Iterator>
constexpr bit_iterator<Iterator> bit_iterator<Iterator>::operator--(int) {
  bit_iterator old = *this;
  --(*this);
  return old;
}

// Looks forward several bits and gets an iterator at this position
template <class Iterator>
constexpr bit_iterator<Iterator> bit_iterator<Iterator>::operator+(difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = _position + n;
  difference_type diff = sum / digits;
  if (sum < 0 && diff * digits != sum) {
    --diff;
  }
  return bit_iterator(std::next(_current, diff), sum - diff * digits);
}

// Looks backward several bits and gets an iterator at this position
template <class Iterator>
constexpr bit_iterator<Iterator> bit_iterator<Iterator>::operator-(difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = _position - n;
  difference_type diff = sum / digits;
  if (sum < 0 && diff * digits != sum) {
    --diff;
  }
  return bit_iterator(std::next(_current, diff), sum - diff * digits);
}

// Increments the iterator by several bits and returns it
template <class Iterator>
constexpr bit_iterator<Iterator>& bit_iterator<Iterator>::operator+=(difference_type n) {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = _position + n;
  difference_type diff = sum / digits;
  if (sum < 0 && diff * digits != sum) {
    --diff;
  }
  _current = std::next(_current, diff);
  _position = (sum - diff * digits);
  return *this;
}

// Decrements the iterator by several bits and returns it
template <class Iterator>
constexpr bit_iterator<Iterator>& bit_iterator<Iterator>::operator-=(difference_type n) {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = _position - n;
  difference_type diff = sum / digits;
  if (sum < 0 && diff * digits != sum) {
    --diff;
  }
  _current = std::next(_current, diff);
  _position = (sum - diff * digits);
  return *this;
}
// -------------------------------------------------------------------------- //

template <class Iterator>
constexpr bit_iterator<Iterator>::difference_type
bit_iterator<Iterator>::operator-(const bit_iterator<Iterator>& other) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  return (_current - other._current) * digits + (_position - other._position);
}

// -------------------- BIT ITERATOR: UNDERLYING DETAILS -------------------- //
// Returns a copy of the underlying iterator
template <class Iterator>
constexpr typename bit_iterator<Iterator>::iterator_type bit_iterator<Iterator>::base() const {
  return _current;
}

template <class Iterator>
constexpr typename bit_iterator<Iterator>::iterator_type bit_iterator<Iterator>::address() const {
  return _current;
}

// Returns the position of the bit within the underlying word
template <class Iterator>
constexpr typename bit_iterator<Iterator>::size_type bit_iterator<Iterator>::position() const noexcept {
  return _position;
}

// -------------- BIT ITERATOR: NON-MEMBER ARITHMETIC OPERATORS ------------- //
// Advances the bit iterator several times
template <class T>
constexpr bit_iterator<T> operator+(typename bit_iterator<T>::difference_type n, const bit_iterator<T>& i) {
  return i + n;
}

// Computes the distance in bits separating two bit iterators
template <class T, class U>
constexpr typename std::common_type<
    typename bit_iterator<T>::difference_type,
    typename bit_iterator<U>::difference_type>::type
operator-(const bit_iterator<T>& lhs, const bit_iterator<U>& rhs) {
  using lhs_utype = typename bit_iterator<T>::word_type;
  using rhs_utype = typename bit_iterator<U>::word_type;
  using lhs_type = typename bit_iterator<T>::difference_type;
  using rhs_type = typename bit_iterator<U>::difference_type;
  using difference_type = typename std::common_type<lhs_type, rhs_type>::type;
  constexpr difference_type lhs_digits = binary_digits<lhs_utype>::value;
  constexpr difference_type rhs_digits = binary_digits<rhs_utype>::value;
  constexpr difference_type digits = rhs_digits;
  static_assert(lhs_digits == rhs_digits, "");
  const difference_type main = lhs._current - rhs._current;
  return main * digits + (lhs._position - rhs._position);
}

static_assert(bit_iterator_c<bit_iterator<uint8_t*>>, "bit_iterator does not satisfy bit_iterator_c concept!");
static_assert(bit_pointer_c<bit_pointer<uint8_t>, bit_reference<uint8_t&>>, "bit_pointer does not satisfy bit_pointer_c concept!");
static_assert(bit_iterator_c<bit_pointer<uint8_t>>, "bit_pointer does not satisfy bit_iterator_c concept!");

// ========================================================================== //
} // namespace bit
#endif // _BIT_ITERATOR_HPP_INCLUDED
// ========================================================================== //
