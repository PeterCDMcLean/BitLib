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

namespace detail {
  template <typename size_type, size_t Alignment>
  struct iterator_position_storage;

  template <typename size_type>
  struct iterator_position_storage<size_type, std::dynamic_extent> {
    size_type _position;
    iterator_position_storage () : _position(0) {}
    iterator_position_storage (size_type position) : _position(position) {

    }
    constexpr size_type position() const noexcept{
      return _position;
    }
    constexpr size_type& position() noexcept {
      return _position;
    }
  };

  template <typename size_type, size_t Alignment>
  struct iterator_position_storage {
    iterator_position_storage() {

    }
    iterator_position_storage(size_type position) {
      assert(position == Alignment);
    }
    constexpr size_type position() const noexcept {
      return Alignment;
    }
  };
}

// Generic bit iterator with configurable primitive value type and origin
// alignment. bit_iterator remains as an alias to the default instantiation (a
// single-bit iterator, i.e. ValueType == primitive<1>, whose value_type is
// bit_value). Alignment describes the initial bit alignment of the
// iterator origin; a value of 1 means no known alignment (runtime position
// tracking). When Alignment > 1 and equals ValueType::bits, the iterator
// can use compile-time optimizations since every element starts at a known
// aligned position within its word.
template <
    class Iterator,
    primitive_c ValueType,
    size_t Alignment
>
class iterator : public detail::iterator_position_storage<std::size_t, Alignment>
{
  template <class, primitive_c, size_t>
  friend class iterator;

  // Assertions
 private:
  using _traits_t = _cv_iterator_traits<Iterator>;
  static_assert(
      ValueType::bits <= binary_digits<std::iter_value_t<Iterator>>::value,
      "primitive bit width cannot exceed the underlying word width");
  static_assert(binary_digits<std::iter_value_t<Iterator>>::value, "");
  static_assert(Alignment == std::dynamic_extent || Alignment <= bitsof<std::size_t>(),
      "alignment exceeds the native integer size of the architecture");

  // Types
 public:
  using iterator_type = Iterator;
  using word_type = std::iter_value_t<Iterator>;
  using iterator_category = typename _traits_t::iterator_category;
  using value_type = ValueType;
  using difference_type = std::ptrdiff_t;
  using pointer = iterator<word_type*, ValueType, Alignment>;
  using reference = reference<typename _traits_t::reference, ValueType>;
  using size_type = std::size_t;
  using mask_type = std::make_unsigned_t<std::remove_cv_t<word_type>>;
  using detail::iterator_position_storage<size_t, Alignment>::position;

  // Lifecycle
 public:
  constexpr iterator();
  constexpr iterator(const iterator& other);
  template <class T, primitive_c U, size_t O>
  constexpr iterator(const iterator<T, U, O>& other)
    requires std::constructible_from<iterator_type, T>;
  explicit constexpr iterator(iterator_type i);
  constexpr iterator(iterator_type i, size_type pos);
  explicit constexpr iterator(const pointer& ptr)
    requires std::constructible_from<iterator_type, word_type*>;

  template <typename T>
  constexpr iterator(const iterator<bit_word_pointer_adapter<T, Iterator>,
                                    ValueType, Alignment>& other);

  // Assignment
 public:
  constexpr iterator& operator=(const iterator& other);
  template <class T, primitive_c U, size_t O>
  constexpr iterator& operator=(const iterator<T, U, O>& other);

  // Access
 public:
  constexpr reference operator*() const noexcept;
  constexpr pointer operator->() const noexcept;
  constexpr reference operator[](difference_type n) const;

  // Increment and decrement operators - advance by ValueType::bits elements
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
  //constexpr size_type position() const noexcept;
  constexpr size_type origin_alignment() const noexcept { return Alignment; }

  auto operator<=>(const iterator&) const = default;
  // Implementation details: data members
 private:
  iterator_type _current;

  // Non-member arithmetic operators
  template <class T, primitive_c U, size_t O>
  friend constexpr iterator<T, U, O> operator+(
      typename iterator<T, U, O>::difference_type n,
      const iterator<T, U, O>& i);
  template <class T, primitive_c U, size_t O, class V, primitive_c W, size_t P>
  friend constexpr typename std::common_type<
      typename iterator<T, U, O>::difference_type,
      typename iterator<V, W, P>::difference_type>::type
  operator-(
      const iterator<T, U, O>& lhs,
      const iterator<V, W, P>& rhs);
};
/* ************************************************************************** */



// ------------------------ BIT ITERATOR: LIFECYCLE ------------------------- //
// Implicitly default constructs a bit iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>::iterator()
    : detail::iterator_position_storage<std::size_t, Alignment>(), _current() {
}

// Implicitly constructs a bit iterator from another bit iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>::iterator(
    const iterator& other)
    : detail::iterator_position_storage<std::size_t, Alignment>(other.position()), _current(other.base()) {
  assert(position() < binary_digits<word_type>::value);
}

template <class Iterator, primitive_c ValueType, size_t Alignment>
template <class T, primitive_c U, size_t O>
constexpr iterator<Iterator, ValueType, Alignment>::iterator(
    const iterator<T, U, O>& other)
  requires std::constructible_from<iterator_type, T>
    : detail::iterator_position_storage<std::size_t, Alignment>(other.position()), _current(other.base()) {
  assert(position() < binary_digits<word_type>::value);
}

// Explicitly constructs an aligned bit iterator from an iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>::iterator(
    const iterator_type i)
    : detail::iterator_position_storage<std::size_t, Alignment>(), _current(i) {
}

// Explicitly constructs an unaligned bit iterator from an iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>::iterator(
    const iterator_type i,
    size_type pos)
    : detail::iterator_position_storage<std::size_t, Alignment>((assert(pos < binary_digits<word_type>::value), pos)), _current(i) {
}

// Explicitly constructs an unaligned bit iterator from a pointer
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>::iterator(
    const pointer& ptr)
  requires std::constructible_from<iterator_type, word_type*>
    : detail::iterator_position_storage<std::size_t, Alignment>(ptr.position()), _current(ptr._current) {
  assert(position() < binary_digits<word_type>::value);
}

template <class Iterator, primitive_c ValueType, size_t Alignment>
template <typename T>
constexpr iterator<Iterator, ValueType, Alignment>::iterator(
    const iterator<bit_word_pointer_adapter<T, Iterator>, ValueType, Alignment>& other)
    : detail::iterator_position_storage<std::size_t, Alignment>(
        (other.base().index() *
          binary_digits<typename bit_word_pointer_adapter<T, Iterator>::value_type>::value +
            other.position())
      ),
      _current(other.base().base()) {
  assert(position() < binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //



// ------------------------ BIT ITERATOR: ASSIGNMENT ------------------------ //
// Assigns a bit iterator to the bit iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator=(
    const iterator& other) {
  _current = other._current;
  position() = other._position;
  assert(position() < binary_digits<word_type>::value);
  return *this;
}

// Assigns a bit iterator to the bit iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
template <class T, primitive_c U, size_t O>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator=(
    const iterator<T, U, O>& other) {
  _current = other._current;
  position() = other.position();
  assert(position() < binary_digits<word_type>::value);
  return *this;
}
// -------------------------------------------------------------------------- //



// -------------------------- BIT ITERATOR: ACCESS -------------------------- //
// Gets a bit reference from the bit iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr typename iterator<Iterator, ValueType, Alignment>::reference
iterator<Iterator, ValueType, Alignment>::operator*() const noexcept {
  return reference(*_current, position());
}

// Gets a pointer to a bit
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr typename iterator<Iterator, ValueType, Alignment>::pointer
iterator<Iterator, ValueType, Alignment>::operator->() const noexcept {
  return pointer(&*_current, position());
}

// Gets a bit reference, decrementing or incrementing the iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr typename iterator<Iterator, ValueType, Alignment>::reference
iterator<Iterator, ValueType, Alignment>::operator[](difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  constexpr difference_type step = ValueType::bits;
  const difference_type sum = static_cast<difference_type>(position()) +
                              n * static_cast<difference_type>(step);
  const difference_type diff = (sum - (n < 0) * (digits - 1)) / digits;
  return reference(*std::next(_current, diff),
                   static_cast<size_type>(sum) % static_cast<size_type>(digits));
}
// -------------------------------------------------------------------------- //



// ------------- BIT ITERATOR: INCREMENT AND DECREMENT OPERATORS ------------ //
// Increments the bit iterator by ValueType::bits and returns it
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator++() {
  constexpr auto digits = binary_digits<word_type>::value;
  constexpr auto step = ValueType::bits;
  position() += step;
  if (position() >= digits) {
    position() -= digits;
    ++_current;
  }
  return *this;
}

// Decrements the bit iterator by ValueType::bits and returns it
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator--() {
  constexpr auto digits = binary_digits<word_type>::value;
  constexpr auto step = ValueType::bits;
  if (position() < step) {
    --_current;
    position() += digits - step;
  } else {
    position() -= step;
  }
  return *this;
}

// Increments the bit iterator and returns the old one
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>
iterator<Iterator, ValueType, Alignment>::operator++(int) {
  iterator old = *this;
  ++(*this);
  return old;
}

// Decrements the bit iterator and returns the old one
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>
iterator<Iterator, ValueType, Alignment>::operator--(int) {
  iterator old = *this;
  --(*this);
  return old;
}

// Looks forward several elements and gets an iterator at this position
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>
iterator<Iterator, ValueType, Alignment>::operator+(difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  constexpr difference_type step = ValueType::bits;
  const difference_type sum = static_cast<difference_type>(position()) +
                              n * static_cast<difference_type>(step);
  const difference_type diff = (sum - (n < 0) * (digits - 1)) / digits;
  const size_type new_pos = static_cast<size_t>(sum) % static_cast<size_t>(digits);
  return iterator(std::next(_current, diff), new_pos);
}

// Looks backward several elements and gets an iterator at this position
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>
iterator<Iterator, ValueType, Alignment>::operator-(difference_type n) const {
  return *this + (-n);
}

// Increments the iterator by several elements and returns it
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator+=(difference_type n) {
  *this = *this + n;
  return *this;
}

// Decrements the iterator by several elements and returns it
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator-=(difference_type n) {
  *this = *this - n;
  return *this;
}
// -------------------------------------------------------------------------- //

template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr typename iterator<Iterator, ValueType, Alignment>::difference_type
iterator<Iterator, ValueType, Alignment>::operator-(
    const iterator& other) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  constexpr difference_type step = ValueType::bits;
  const difference_type bit_diff = (_current - other._current) * digits +
                                   (static_cast<difference_type>(position()) -
                                    static_cast<difference_type>(other.position()));
  return bit_diff / static_cast<difference_type>(step);
}

// -------------------- BIT ITERATOR: UNDERLYING DETAILS -------------------- //
// Returns a copy of the underlying iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr typename iterator<Iterator, ValueType, Alignment>::iterator_type
iterator<Iterator, ValueType, Alignment>::base() const {
  return _current;
}

template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr typename iterator<Iterator, ValueType, Alignment>::iterator_type
iterator<Iterator, ValueType, Alignment>::address() const {
  return _current;
}

/*
// Returns the position of the bit within the underlying word
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr typename iterator<Iterator, ValueType, Alignment>::size_type
iterator<Iterator, ValueType, Alignment>::position() const noexcept {
  return _position;
}
  */

// -------------- BIT ITERATOR: NON-MEMBER ARITHMETIC OPERATORS ------------- //
// Advances the bit iterator several elements
template <class T, primitive_c U, size_t O>
constexpr iterator<T, U, O> operator+(
    typename iterator<T, U, O>::difference_type n,
    const iterator<T, U, O>& i) {
  return i + n;
}

// Computes the distance in elements separating two bit iterators
template <class T, primitive_c U, size_t O, class V, primitive_c W, size_t P>
constexpr typename std::common_type<
    typename iterator<T, U, O>::difference_type,
    typename iterator<V, W, P>::difference_type>::type
operator-(const iterator<T, U, O>& lhs, const iterator<V, W, P>& rhs) {
  using lhs_utype = typename iterator<T, U, O>::word_type;
  using rhs_utype = typename iterator<V, W, P>::word_type;
  using lhs_type = typename iterator<T, U, O>::difference_type;
  using rhs_type = typename iterator<V, W, P>::difference_type;
  using difference_type = typename std::common_type<lhs_type, rhs_type>::type;
  constexpr difference_type lhs_digits =
      binary_digits<lhs_utype>::value;
  constexpr difference_type rhs_digits =
      binary_digits<rhs_utype>::value;
  constexpr difference_type digits = rhs_digits;
  static_assert(lhs_digits == rhs_digits, "");
  constexpr difference_type step = U::bits;
  const difference_type main = lhs._current - rhs._current;
  const difference_type bit_diff = main * digits +
                                   (static_cast<difference_type>(lhs.position()) -
                                    static_cast<difference_type>(rhs.position()));
  return bit_diff / step;
}

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
}  // namespace bit
#endif  // _BIT_ITERATOR_HPP_INCLUDED
// ========================================================================== //
