// ============================== BIT ITERATOR ============================== //
// Project:         The C++ Bit Library
// Name:            bit_iterator_adapter.hpp
// Description:     A class representing an iterator on bit sequences
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ITERATOR_ADAPTER_HPP_INCLUDED
#define _BIT_ITERATOR_ADAPTER_HPP_INCLUDED
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
// Bit iterator class definition
template <typename Iterator, typename BaseIterator>
class bit_iterator_adapter {
  template <typename, typename>
  friend class bit_iterator_adapter;

  // Assertions
 private:
  using _traits_t = _cv_iterator_traits<Iterator>;
  static constexpr bool big_to_small = sizeof(typename _cv_iterator_traits<BaseIterator>::value_type) >
                                       sizeof(typename _cv_iterator_traits<Iterator>::value_type);
  static constexpr size_t ratio = big_to_small ? (sizeof(typename _cv_iterator_traits<BaseIterator>::value_type) /
                                                  sizeof(typename _cv_iterator_traits<Iterator>::value_type))
                                               : (sizeof(typename _cv_iterator_traits<Iterator>::value_type) /
                                                  sizeof(typename _cv_iterator_traits<BaseIterator>::value_type));

  // Types
 public:
  using iterator_type = Iterator;
  using base_iterator_type = BaseIterator;
  using word_type = typename _traits_t::value_type;
  using iterator_category = typename _traits_t::iterator_category;
  using value_type = typename _traits_t::value_type;
  using difference_type = std::ptrdiff_t;
  using pointer = typename _traits_t::pointer;
  using reference = typename _traits_t::reference;
  using size_type = std::size_t;

  // Lifecycle
 public:
  constexpr bit_iterator_adapter();
  constexpr bit_iterator_adapter(const bit_iterator_adapter<Iterator, BaseIterator>& other);
  template <typename T, typename U>
  constexpr bit_iterator_adapter(const bit_iterator_adapter<T, U>& other)
    requires std::constructible_from<iterator_type, T>;
  explicit constexpr bit_iterator_adapter(iterator_type i);
  constexpr bit_iterator_adapter(base_iterator_type i, size_type pos = 0);
  explicit constexpr bit_iterator_adapter(const pointer& ptr)
    requires std::constructible_from<iterator_type, word_type*>;

  // Assignment
 public:
  constexpr bit_iterator_adapter& operator=(const bit_iterator_adapter<Iterator, BaseIterator>& other);
  template <typename T, typename U>
  constexpr bit_iterator_adapter& operator=(const bit_iterator_adapter<T, U>& other);

  // Access
 public:
  constexpr reference operator*() const noexcept;
  constexpr pointer operator->() const noexcept;
  constexpr reference operator[](difference_type n) const;

  // Increment and decrement operators
 public:
  constexpr bit_iterator_adapter& operator++();
  constexpr bit_iterator_adapter& operator--();
  constexpr bit_iterator_adapter operator++(int);
  constexpr bit_iterator_adapter operator--(int);
  constexpr bit_iterator_adapter operator+(difference_type n) const;
  constexpr bit_iterator_adapter operator-(difference_type n) const;
  constexpr bit_iterator_adapter& operator+=(difference_type n);
  constexpr bit_iterator_adapter& operator-=(difference_type n);
  constexpr difference_type operator-(const bit_iterator_adapter&) const;

  // Underlying details
 public:
  constexpr base_iterator_type base() const;
  constexpr size_type position() const noexcept;

  bool operator==(const bit_iterator_adapter&) const = default;
  auto operator<=>(const bit_iterator_adapter&) const = default;
  // Implementation details: data members
 private:
  base_iterator_type _current;
  size_type _position;

  // Non-member arithmetic operators
  template <typename T, typename U>
  friend constexpr bit_iterator_adapter<T, U> operator+(
      typename bit_iterator_adapter<T, U>::difference_type n,
      const bit_iterator_adapter<T, U>& i);
  template <typename T, typename U, typename V, typename W>
  friend constexpr typename std::common_type<
      typename bit_iterator_adapter<T, U>::difference_type,
      typename bit_iterator_adapter<V, W>::difference_type>::type
  operator-(
      const bit_iterator_adapter<T, U>& lhs,
      const bit_iterator_adapter<V, W>& rhs);
};
/* ************************************************************************** */



// ------------------------ BIT ITERATOR: LIFECYCLE ------------------------- //
// Implicitly default constructs a bit iterator
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>::bit_iterator_adapter()
    : _current(), _position() {
}

// Implicitly constructs a bit iterator from another bit iterator
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>::bit_iterator_adapter(const bit_iterator_adapter<Iterator, BaseIterator>& other)
    : _current(other.base()), _position(other.position()) {
  assert(_position < ratio);
}

template <typename Iterator, typename BaseIterator>
template <typename T, typename U>
constexpr bit_iterator_adapter<Iterator, BaseIterator>::bit_iterator_adapter(const bit_iterator_adapter<T, U>& other)
  requires std::constructible_from<iterator_type, T>
    : _current(other.base()), _position(other.position()) {
  assert(_position < ratio);
}

// Explicitly constructs an aligned bit iterator from an iterator
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>::bit_iterator_adapter(const iterator_type i)
    : _current(reinterpret_cast<BaseIterator>(i)), _position(0) {
}

// Explicitly constructs an unaligned bit iterator from an iterator
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>::bit_iterator_adapter(const base_iterator_type i, size_type pos)
    : _current(i), _position((assert(pos < binary_digits<word_type>::value), pos)) {
}

// Explicitly constructs an unaligned bit iterator from a pointer
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>::bit_iterator_adapter(const pointer& ptr)
  requires std::constructible_from<iterator_type, word_type*>
    : _current(ptr._current), _position(ptr.position()) {
  assert(_position < ratio);
}
// -------------------------------------------------------------------------- //



// ------------------------ BIT ITERATOR: ASSIGNMENT ------------------------ //
// Assigns a bit iterator to the bit iterator
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>& bit_iterator_adapter<Iterator, BaseIterator>::operator=(
    const bit_iterator_adapter<Iterator, BaseIterator>& other) {
  _current = other._current;
  _position = other._position;
  assert(_position < ratio);
  return *this;
}

// Assigns a bit iterator to the bit iterator
template <typename Iterator, typename BaseIterator>
template <typename T, typename U>
constexpr bit_iterator_adapter<Iterator, BaseIterator>& bit_iterator_adapter<Iterator, BaseIterator>::operator=(
    const bit_iterator_adapter<T, U>& other) {
  _current = other._current;
  _position = other._position;
  assert(_position < ratio);
  return *this;
}
// -------------------------------------------------------------------------- //



// -------------------------- BIT ITERATOR: ACCESS -------------------------- //
// Gets a bit reference from the bit iterator
template <typename Iterator, typename BaseIterator>
constexpr typename bit_iterator_adapter<Iterator, BaseIterator>::reference
bit_iterator_adapter<Iterator, BaseIterator>::operator*() const noexcept {
  return reference(reinterpret_cast<word_type*>(&(*_current))[_position]);
}

// Gets a pointer to a bit
template <typename Iterator, typename BaseIterator>
constexpr typename bit_iterator_adapter<Iterator, BaseIterator>::pointer bit_iterator_adapter<Iterator, BaseIterator>::operator->() const noexcept {
  return pointer(&reinterpret_cast<word_type*>(&(*_current))[_position]);
}

// Gets a bit reference, decrementing or incrementing the iterator
template <typename Iterator, typename BaseIterator>
constexpr typename bit_iterator_adapter<Iterator, BaseIterator>::reference bit_iterator_adapter<Iterator, BaseIterator>::operator[](difference_type n) const {
  const difference_type sum = _position + n;
  difference_type diff = sum / ratio;
  if (sum < 0 && diff * ratio != sum) {
    --diff;
  }
  return reference(reinterpret_cast<Iterator>(&(*std::next(_current, diff)))[sum - diff * ratio]);
}
// -------------------------------------------------------------------------- //



// ------------- BIT ITERATOR: INCREMENT AND DECREMENT OPERATORS ------------ //
// Increments the bit iterator and returns it
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>& bit_iterator_adapter<Iterator, BaseIterator>::operator++() {
  if (_position + 1 < ratio) {
    ++_position;
  } else {
    ++_current;
    _position = 0;
  }
  return *this;
}

// Decrements the bit iterator and returns it
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>& bit_iterator_adapter<Iterator, BaseIterator>::operator--() {
  if (_position) {
    --_position;
  } else {
    --_current;
    _position = ratio - 1;
  }
  return *this;
}

// Increments the bit iterator and returns the old one
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator> bit_iterator_adapter<Iterator, BaseIterator>::operator++(int) {
  bit_iterator_adapter old = *this;
  ++(*this);
  return old;
}

// Decrements the bit iterator and returns the old one
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator> bit_iterator_adapter<Iterator, BaseIterator>::operator--(int) {
  bit_iterator_adapter old = *this;
  --(*this);
  return old;
}

// Looks forward several bits and gets an iterator at this position
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator> bit_iterator_adapter<Iterator, BaseIterator>::operator+(difference_type n) const {
  const difference_type sum = _position + n;
  difference_type diff = sum / ratio;
  if (sum < 0 && diff * ratio != sum) {
    --diff;
  }
  return bit_iterator_adapter(&reinterpret_cast<Iterator>(&(*std::next(_current, diff)))[sum - diff * ratio], sum - diff * ratio);
}

// Looks backward several bits and gets an iterator at this position
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator> bit_iterator_adapter<Iterator, BaseIterator>::operator-(difference_type n) const {
  const difference_type sum = _position - n;
  difference_type diff = sum / ratio;
  if (sum < 0 && diff * ratio != sum) {
    --diff;
  }
  return bit_iterator_adapter(&reinterpret_cast<Iterator>(&(*std::next(_current, diff)))[sum - diff * ratio], sum - diff * ratio);
}

// Increments the iterator by several bits and returns it
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>& bit_iterator_adapter<Iterator, BaseIterator>::operator+=(difference_type n) {
  const difference_type sum = _position + n;
  difference_type diff = sum / ratio;
  if (sum < 0 && diff * ratio != sum) {
    --diff;
  }
  _current = std::next(_current, diff);
  _position = (sum - diff * ratio);
  return *this;
}

// Decrements the iterator by several bits and returns it
template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>& bit_iterator_adapter<Iterator, BaseIterator>::operator-=(difference_type n) {
  const difference_type sum = _position - n;
  difference_type diff = sum / ratio;
  if (sum < 0 && diff * ratio != sum) {
    --diff;
  }
  _current = std::next(_current, diff);
  _position = (sum - diff * ratio);
  return *this;
}
// -------------------------------------------------------------------------- //

template <typename Iterator, typename BaseIterator>
constexpr bit_iterator_adapter<Iterator, BaseIterator>::difference_type
bit_iterator_adapter<Iterator, BaseIterator>::operator-(const bit_iterator_adapter<Iterator, BaseIterator>& other) const {
  return (_current - other._current) * ratio + (_position - other._position);
}

// -------------------- BIT ITERATOR: UNDERLYING DETAILS -------------------- //
// Returns a copy of the underlying iterator
template <typename Iterator, typename BaseIterator>
constexpr typename bit_iterator_adapter<Iterator, BaseIterator>::base_iterator_type bit_iterator_adapter<Iterator, BaseIterator>::base() const {
  return _current;
}

// Returns the position of the bit within the underlying word
template <typename Iterator, typename BaseIterator>
constexpr typename bit_iterator_adapter<Iterator, BaseIterator>::size_type bit_iterator_adapter<Iterator, BaseIterator>::position() const noexcept {
  return _position;
}

// -------------- BIT ITERATOR: NON-MEMBER ARITHMETIC OPERATORS ------------- //
// Advances the bit iterator several times
template <typename T, typename U>
constexpr bit_iterator_adapter<T, U> operator+(typename bit_iterator_adapter<T, U>::difference_type n, const bit_iterator_adapter<T, U>& i) {
  return i + n;
}

// Computes the distance in bits separating two bit iterators
template <typename T, typename U, typename V, typename W>
constexpr typename std::common_type<
    typename bit_iterator_adapter<T, U>::difference_type,
    typename bit_iterator_adapter<V, W>::difference_type>::type
operator-(const bit_iterator_adapter<T, U>& lhs, const bit_iterator_adapter<V, W>& rhs) {
  using lhs_utype = typename bit_iterator_adapter<T, U>::word_type;
  using rhs_utype = typename bit_iterator_adapter<V, W>::word_type;
  using lhs_type = typename bit_iterator_adapter<T, U>::difference_type;
  using rhs_type = typename bit_iterator_adapter<V, W>::difference_type;
  using size_type = typename std::common_type<typename bit_iterator_adapter<T, U>::size_type,
                                              typename bit_iterator_adapter<V, W>::size_type>::type;
  using difference_type = typename std::common_type<typename bit_iterator_adapter<T, U>::difference_type,
                                                    typename bit_iterator_adapter<V, W>::difference_type>::type;
  constexpr size_type lhs_ratio = bit_iterator_adapter<T, U>::ratio;
  constexpr size_type rhs_ratio = bit_iterator_adapter<V, W>::ratio;
  static_assert(lhs_ratio == rhs_ratio, "");
  const difference_type main = lhs._current - rhs._current;
  return main * lhs_ratio + (lhs._position - rhs._position);
}

//static_assert(bit_iterator_adapter_c<bit_iterator_adapter<uint8_t*>>, "bit_iterator_adapter does not satisfy bit_iterator_adapter_c concept!");
//static_assert(bit_pointer_c<bit_pointer<uint8_t>, bit_reference<uint8_t>>, "bit_pointer does not satisfy bit_pointer_c concept!");
//static_assert(bit_iterator_adapter_c<bit_pointer<uint8_t>>, "bit_pointer does not satisfy bit_iterator_adapter_c concept!");

// ========================================================================== //
} // namespace bit
#endif // _BIT_ITERATOR_ADAPTER_HPP_INCLUDED
// ========================================================================== //
