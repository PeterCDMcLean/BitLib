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

// Generic bit iterator with configurable primitive bit width and alignment.
// bit_iterator remains as an alias to the default instantiation (a single-bit
// iterator, i.e. PrimitiveBits == 1, whose value_type is bit_value).
template <
    class Iterator,
    size_t PrimitiveBits,
    size_t Alignment
>
class iterator
{
  template <class, size_t, size_t>
  friend class iterator;

  // Assertions
 private:
  using _traits_t = _cv_iterator_traits<Iterator>;
  static_assert(PrimitiveBits >= 1, "iterator must iterate at least 1 bit");
  static_assert(
      PrimitiveBits <= binary_digits<std::iter_value_t<Iterator>>::value,
      "primitive bit width cannot exceed the underlying word width");
  static_assert(binary_digits<std::iter_value_t<Iterator>>::value, "");

  // Types
 public:
  using iterator_type = Iterator;
  using word_type = std::iter_value_t<Iterator>;
  using iterator_category = typename _traits_t::iterator_category;
  using value_type = primitive<PrimitiveBits>;
  using difference_type = std::ptrdiff_t;
  using pointer = iterator<word_type*, PrimitiveBits, Alignment>;
  using reference = reference<typename _traits_t::reference, PrimitiveBits>;
  using size_type = std::size_t;
  using mask_type = std::make_unsigned_t<std::remove_cv_t<word_type>>;

  static constexpr size_type primitive_bits = PrimitiveBits;
  static constexpr size_type alignment_value = Alignment;

  // Lifecycle
 public:
  constexpr iterator();
  constexpr iterator(const iterator& other);
  template <class T, size_t B, size_t A>
  constexpr iterator(const iterator<T, B, A>& other)
    requires std::constructible_from<iterator_type, T>;
  explicit constexpr iterator(iterator_type i);
  constexpr iterator(iterator_type i, size_type pos);
  explicit constexpr iterator(const pointer& ptr)
    requires std::constructible_from<iterator_type, word_type*>;

  template <typename T>
  constexpr iterator(const iterator<bit_word_pointer_adapter<T, Iterator>,
                                     PrimitiveBits, Alignment>& other);

  // Assignment
 public:
  constexpr iterator& operator=(const iterator& other);
  template <class T, size_t B, size_t A>
  constexpr iterator& operator=(const iterator<T, B, A>& other);

  // Access
 public:
  constexpr reference operator*() const noexcept;
  constexpr pointer operator->() const noexcept;
  constexpr reference operator[](difference_type n) const;

  // Increment and decrement operators
 public:
  constexpr iterator& operator++();
  constexpr iterator& operator--();
  constexpr iterator operator++(int);
  constexpr iterator operator--(int);
  constexpr iterator operator+(difference_type n) const;
  constexpr iterator operator-(difference_type n) const;
  constexpr iterator& operator+=(difference_type n);
  constexpr iterator& operator-=(difference_type n);
  constexpr difference_type operator-(const iterator&) const;

  // Underlying details
 public:
  constexpr iterator_type base() const;
  constexpr iterator_type address() const;
  constexpr size_type position() const noexcept;
  constexpr size_type alignment() const noexcept { return Alignment; }

  auto operator<=>(const iterator&) const = default;
  // Implementation details: data members
 private:
  iterator_type _current;
  size_type _position;

  // Non-member arithmetic operators
  template <class T, size_t B, size_t A>
  friend constexpr iterator<T, B, A> operator+(
      typename iterator<T, B, A>::difference_type n,
      const iterator<T, B, A>& i);
  template <class T, size_t B, size_t A, class U, size_t C, size_t D>
  friend constexpr typename std::common_type<
      typename iterator<T, B, A>::difference_type,
      typename iterator<U, C, D>::difference_type>::type
  operator-(
      const iterator<T, B, A>& lhs,
      const iterator<U, C, D>& rhs);
};
/* ************************************************************************** */



// ------------------------ BIT ITERATOR: LIFECYCLE ------------------------- //
// Implicitly default constructs a bit iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>::iterator()
    : _current(), _position() {
}

// Implicitly constructs a bit iterator from another bit iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>::iterator(
    const iterator& other)
    : _current(other.base()), _position(other.position()) {
  assert(_position < binary_digits<word_type>::value);
}

template <class Iterator, size_t PrimitiveBits, size_t Alignment>
template <class T, size_t B, size_t A>
constexpr iterator<Iterator, PrimitiveBits, Alignment>::iterator(
    const iterator<T, B, A>& other)
  requires std::constructible_from<iterator_type, T>
    : _current(other.base()), _position(other.position()) {
  assert(_position < binary_digits<word_type>::value);
}

// Explicitly constructs an aligned bit iterator from an iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>::iterator(
    const iterator_type i)
    : _current(i), _position(0) {
}

// Explicitly constructs an unaligned bit iterator from an iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>::iterator(
    const iterator_type i,
    size_type pos)
    : _current(i), _position((assert(pos < binary_digits<word_type>::value), pos)) {
}

// Explicitly constructs an unaligned bit iterator from a pointer
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>::iterator(
    const pointer& ptr)
  requires std::constructible_from<iterator_type, word_type*>
    : _current(ptr._current), _position(ptr.position()) {
  assert(_position < binary_digits<word_type>::value);
}

template <class Iterator, size_t PrimitiveBits, size_t Alignment>
template <typename T>
constexpr iterator<Iterator, PrimitiveBits, Alignment>::iterator(
    const iterator<bit_word_pointer_adapter<T, Iterator>,
                   PrimitiveBits, Alignment>& other)
    : _current(other.base().base()),
      _position(other.base().index() *
                    binary_digits<typename bit_word_pointer_adapter<T,
                                    Iterator>::value_type>::value +
                other.position()) {
  assert(_position < binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //



// ------------------------ BIT ITERATOR: ASSIGNMENT ------------------------ //
// Assigns a bit iterator to the bit iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>&
iterator<Iterator, PrimitiveBits, Alignment>::operator=(
    const iterator& other) {
  _current = other._current;
  _position = other._position;
  assert(_position < binary_digits<word_type>::value);
  return *this;
}

// Assigns a bit iterator to the bit iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
template <class T, size_t B, size_t A>
constexpr iterator<Iterator, PrimitiveBits, Alignment>&
iterator<Iterator, PrimitiveBits, Alignment>::operator=(
    const iterator<T, B, A>& other) {
  _current = other._current;
  _position = other._position;
  assert(_position < binary_digits<word_type>::value);
  return *this;
}
// -------------------------------------------------------------------------- //



// -------------------------- BIT ITERATOR: ACCESS -------------------------- //
// Gets a bit reference from the bit iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr typename iterator<Iterator, PrimitiveBits, Alignment>::reference
iterator<Iterator, PrimitiveBits, Alignment>::operator*() const noexcept {
  return reference(*_current, _position);
}

// Gets a pointer to a bit
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr typename iterator<Iterator, PrimitiveBits, Alignment>::pointer
iterator<Iterator, PrimitiveBits, Alignment>::operator->() const noexcept {
  return pointer(&*_current, _position);
}

// Gets a bit reference, decrementing or incrementing the iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr typename iterator<Iterator, PrimitiveBits, Alignment>::reference
iterator<Iterator, PrimitiveBits, Alignment>::operator[](difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = static_cast<difference_type>(_position) + n;
  const difference_type diff = (sum - (n < 0) * (digits - 1)) / digits;
  return reference(*std::next(_current, diff),
                   static_cast<size_type>(sum) % static_cast<size_type>(digits));
}
// -------------------------------------------------------------------------- //

// ------------- BIT ITERATOR: INCREMENT AND DECREMENT OPERATORS ------------ //
// Increments the bit iterator and returns it
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>&
iterator<Iterator, PrimitiveBits, Alignment>::operator++() {
  constexpr auto digits = binary_digits<word_type>::value;
  _position = (_position + 1) % digits;
  if (0 == _position) {
    ++_current;
  }
  return *this;
}

// Decrements the bit iterator and returns it
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>&
iterator<Iterator, PrimitiveBits, Alignment>::operator--() {
  constexpr size_type digits = binary_digits<word_type>::value;
  if (0 == _position) {
    --_current;
  }
  _position = (_position - 1) % digits;
  return *this;
}

// Increments the bit iterator and returns the old one
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>
iterator<Iterator, PrimitiveBits, Alignment>::operator++(int) {
  iterator old = *this;
  ++(*this);
  return old;
}

// Decrements the bit iterator and returns the old one
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>
iterator<Iterator, PrimitiveBits, Alignment>::operator--(int) {
  iterator old = *this;
  --(*this);
  return old;
}

// Looks forward several bits and gets an iterator at this position
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>
iterator<Iterator, PrimitiveBits, Alignment>::operator+(difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = static_cast<difference_type>(_position) + n;
  const difference_type diff = (sum - (n < 0) * (digits - 1)) / digits;
  const size_type new_pos = static_cast<size_t>(sum) % static_cast<size_t>(digits);
  return iterator(std::next(_current, diff), new_pos);
}

// Looks backward several bits and gets an iterator at this position
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>
iterator<Iterator, PrimitiveBits, Alignment>::operator-(difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = static_cast<difference_type>(_position) - n;
  const difference_type diff = (sum - (n > 0) * (digits - 1)) / digits;
  const size_type new_pos = static_cast<size_t>(sum) % static_cast<size_t>(digits);
  return iterator(std::next(_current, diff), new_pos);
}

// Increments the iterator by several bits and returns it
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>&
iterator<Iterator, PrimitiveBits, Alignment>::operator+=(difference_type n) {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = static_cast<difference_type>(_position) + n;
  const difference_type diff = (sum - (n < 0) * (digits - 1)) / digits;
  const size_type new_pos = static_cast<size_t>(sum) % static_cast<size_t>(digits);
  _current = std::next(_current, diff);
  _position = new_pos;
  return *this;
}

// Decrements the iterator by several bits and returns it
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr iterator<Iterator, PrimitiveBits, Alignment>&
iterator<Iterator, PrimitiveBits, Alignment>::operator-=(difference_type n) {
  constexpr difference_type digits = binary_digits<word_type>::value;
  const difference_type sum = static_cast<difference_type>(_position) - n;
  const difference_type diff = (sum - (n > 0) * (digits - 1)) / digits;
  const size_type new_pos = static_cast<size_t>(sum) % static_cast<size_t>(digits);
  _current = std::next(_current, diff);
  _position = new_pos;
  return *this;
}
// -------------------------------------------------------------------------- //

template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr typename iterator<Iterator, PrimitiveBits, Alignment>::difference_type
iterator<Iterator, PrimitiveBits, Alignment>::operator-(
    const iterator& other) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  return (_current - other._current) * digits +
         (static_cast<difference_type>(_position) -
          static_cast<difference_type>(other._position));
}

// -------------------- BIT ITERATOR: UNDERLYING DETAILS -------------------- //
// Returns a copy of the underlying iterator
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr typename iterator<Iterator, PrimitiveBits, Alignment>::iterator_type
iterator<Iterator, PrimitiveBits, Alignment>::base() const {
  return _current;
}

template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr typename iterator<Iterator, PrimitiveBits, Alignment>::iterator_type
iterator<Iterator, PrimitiveBits, Alignment>::address() const {
  return _current;
}

// Returns the position of the bit within the underlying word
template <class Iterator, size_t PrimitiveBits, size_t Alignment>
constexpr typename iterator<Iterator, PrimitiveBits, Alignment>::size_type
iterator<Iterator, PrimitiveBits, Alignment>::position() const noexcept {
  return _position;
}

// -------------- BIT ITERATOR: NON-MEMBER ARITHMETIC OPERATORS ------------- //
// Advances the bit iterator several times
template <class T, size_t B, size_t A>
constexpr iterator<T, B, A> operator+(
    typename iterator<T, B, A>::difference_type n,
    const iterator<T, B, A>& i) {
  return i + n;
}

// Computes the distance in bits separating two bit iterators
template <class T, size_t B, size_t A, class U, size_t C, size_t D>
constexpr typename std::common_type<
    typename iterator<T, B, A>::difference_type,
    typename iterator<U, C, D>::difference_type>::type
operator-(const iterator<T, B, A>& lhs, const iterator<U, C, D>& rhs) {
  using lhs_utype = typename iterator<T, B, A>::word_type;
  using rhs_utype = typename iterator<U, C, D>::word_type;
  using lhs_type = typename iterator<T, B, A>::difference_type;
  using rhs_type = typename iterator<U, C, D>::difference_type;
  using difference_type = typename std::common_type<lhs_type, rhs_type>::type;
  constexpr difference_type lhs_digits =
      binary_digits<lhs_utype>::value;
  constexpr difference_type rhs_digits =
      binary_digits<rhs_utype>::value;
  constexpr difference_type digits = rhs_digits;
  static_assert(lhs_digits == rhs_digits, "");
  const difference_type main = lhs._current - rhs._current;
  return main * digits +
         (static_cast<difference_type>(lhs._position) -
          static_cast<difference_type>(rhs._position));
}

// Alias: bit_iterator remains as an alias to the default instantiation of the
// generic iterator<Iterator, PrimitiveBits, Alignment> class (single-bit,
// value_type == bit_value).
template <class Iterator>
using bit_iterator = iterator<Iterator, 1, 1>;

// Alias: bit_pointer is a bit iterator over a word pointer (defined in
// bit_details.hpp as iterator<WordType*>).

static_assert(
    bit_iterator_c<bit_iterator<uint8_t*>>,
    "bit_iterator does not satisfy bit_iterator_c concept!");
static_assert(
    bit_pointer_c<bit_pointer<uint8_t>, reference<uint8_t&>>,
    "bit_pointer does not satisfy bit_pointer_c concept!");
static_assert(
    bit_iterator_c<bit_pointer<uint8_t>>,
    "bit_pointer does not satisfy bit_iterator_c concept!");

// ========================================================================== //
} // namespace bit
#endif // _BIT_ITERATOR_HPP_INCLUDED
// ========================================================================== //
