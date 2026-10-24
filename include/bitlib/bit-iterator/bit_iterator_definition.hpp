#ifndef _BIT_ITERATOR_DEFINITION_HPP_INCLUDED
#define _BIT_ITERATOR_DEFINITION_HPP_INCLUDED

#include <cassert>
#include <iterator>
#include <limits>
#include <type_traits>

#include "bitlib/bit-iterator/bit_iterator_declaration.hpp"
#include "bitlib/bit-iterator/bit_reference_definition.hpp"
#include "bitlib/bit-iterator/bit_word_pointer_adapter_definition.hpp"

namespace bit {

/* ****************************** BIT ITERATOR ****************************** */

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
  requires std::constructible_from<iterator_type, T> && (Alignment == std::dynamic_extent || Alignment == O)
    : detail::iterator_position_storage<std::size_t, Alignment>(other.position()), _current(other.base()) {
  assert(position() < binary_digits<word_type>::value);
}

// Explicitly constructs an aligned bit iterator from an iterator
/* explicit */ template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>::iterator(
    const iterator_type i)
    : detail::iterator_position_storage<std::size_t, Alignment>(), _current(i) {
}

// Explicitly constructs an unaligned bit iterator from an iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>::iterator(
    const iterator_type i,
    size_type pos)
  requires(Alignment == std::dynamic_extent)
    : detail::iterator_position_storage<std::size_t, Alignment>((assert(pos < binary_digits<word_type>::value), pos)), _current(i) {
}

// Explicitly constructs an unaligned bit iterator from a pointer
/* explicit */ template <class Iterator, primitive_c ValueType, size_t Alignment>
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
           other.position())),
      _current(other.base().base()) {
  assert(position() < binary_digits<word_type>::value);
}
// -------------------------------------------------------------------------- //

// ------------------------ BIT ITERATOR: ASSIGNMENT ------------------------ //
// Assigns a bit iterator to the bit iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator=(
    const iterator& other)
  requires(Alignment == std::dynamic_extent)
{
  _current = other._current;
  position() = other._position;
  assert(position() < binary_digits<word_type>::value);
  return *this;
}

template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator=(
    const iterator& other)
  requires(Alignment != std::dynamic_extent)
{
  _current = other._current;
  return *this;
}

// Assigns a bit iterator to the bit iterator
template <class Iterator, primitive_c ValueType, size_t Alignment>
template <class T, primitive_c U, size_t O>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator=(
    const iterator<T, U, O>& other)
  requires(Alignment == std::dynamic_extent)
{
  _current = other._current;
  position() = other.position();
  assert(position() < binary_digits<word_type>::value);
  return *this;
}

template <class Iterator, primitive_c ValueType, size_t Alignment>
template <class T, primitive_c U, size_t O>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator=(
    const iterator<T, U, O>& other)
  requires(Alignment == O)
{
  _current = other._current;
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
iterator<Iterator, ValueType, Alignment>::operator++()
  requires(Alignment == std::dynamic_extent)
{
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
iterator<Iterator, ValueType, Alignment>::operator--()
  requires(Alignment == std::dynamic_extent)
{
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
iterator<Iterator, ValueType, Alignment>::operator++(int)
  requires(Alignment == std::dynamic_extent)
{
  iterator old = *this;
  ++(*this);
  return old;
}

// Decrements the bit iterator and returns the old one
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>
iterator<Iterator, ValueType, Alignment>::operator--(int)
  requires(Alignment == std::dynamic_extent)
{
  iterator old = *this;
  --(*this);
  return old;
}

// Looks forward several elements and gets an iterator at this position
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, std::dynamic_extent>
iterator<Iterator, ValueType, Alignment>::operator+(difference_type n) const {
  constexpr difference_type digits = binary_digits<word_type>::value;
  constexpr difference_type step = ValueType::bits;
  const difference_type sum = static_cast<difference_type>(position()) +
                              n * static_cast<difference_type>(step);
  const difference_type diff = (sum - (n < 0) * (digits - 1)) / digits;
  const size_type new_pos = static_cast<size_t>(sum) % static_cast<size_t>(digits);
  return iterator<Iterator, ValueType, std::dynamic_extent>(std::next(_current, diff), new_pos);
}

// Looks forward several elements and gets an iterator at this position
template <class Iterator, primitive_c ValueType, size_t Alignment>
template <iterator<Iterator, ValueType, Alignment>::difference_type N>
constexpr iterator<Iterator, ValueType, std::dynamic_extent>
iterator<Iterator, ValueType, Alignment>::next() const
  requires(Alignment == std::dynamic_extent)
{
  constexpr difference_type digits = binary_digits<word_type>::value;
  constexpr difference_type step = ValueType::bits;
  const difference_type sum = static_cast<difference_type>(position()) +
                              N * static_cast<difference_type>(step);
  const difference_type diff = (sum - (N < 0) * (digits - 1)) / digits;
  const size_type new_pos = static_cast<size_t>(sum) % static_cast<size_t>(digits);
  return iterator(std::next(_current, diff), new_pos);
}

template <class Iterator, primitive_c ValueType, size_t Alignment>
template <iterator<Iterator, ValueType, Alignment>::difference_type N>
constexpr auto iterator<Iterator, ValueType, Alignment>::next() const
  requires(Alignment != std::dynamic_extent)
{
  constexpr difference_type digits = binary_digits<word_type>::value;
  constexpr difference_type step = ValueType::bits;
  constexpr difference_type sum = static_cast<difference_type>(Alignment) +
                                  N * static_cast<difference_type>(step);
  constexpr difference_type diff = (sum - (N < 0) * (digits - 1)) / digits;
  constexpr size_type new_pos = static_cast<size_t>(sum) % static_cast<size_t>(digits);

  // Fixed alignment: position stays at Alignment, just advance base pointer
  return iterator<Iterator, ValueType, new_pos>(std::next(_current, diff));
}

// Looks backward several elements and gets an iterator at this position
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, std::dynamic_extent>
iterator<Iterator, ValueType, Alignment>::operator-(difference_type n) const {
  return *this + (-n);
}

// Increments the iterator by several elements and returns it
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator+=(difference_type n)
  requires(Alignment == std::dynamic_extent)
{
  *this = *this + n;
  return *this;
}

// Decrements the iterator by several elements and returns it
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr iterator<Iterator, ValueType, Alignment>&
iterator<Iterator, ValueType, Alignment>::operator-=(difference_type n)
  requires(Alignment == std::dynamic_extent)
{
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

// ------------------------- BIT ITERATOR: NEXT ----------------------------- //
// Compile-time offset. N == std::dynamic_extent (the default) means no
// offset was supplied, so the one-argument form is rejected at compile time
// instead of silently doing nothing.
template <
    std::size_t N,
    class Iterator,
    primitive_c ValueType,
    size_t Alignment>
constexpr auto next(const iterator<Iterator, ValueType, Alignment>& it) {
  using difference_type = typename iterator<Iterator, ValueType, Alignment>::difference_type;
  static_assert(N != std::dynamic_extent,
                "bit::next(it) needs a compile-time offset; use bit::next(it, n) for a run-time offset");
  if constexpr (N == std::dynamic_extent) {
    return it;  // unreachable: rejected by the assertion above
  } else {
    static_assert(N <= static_cast<std::size_t>((std::numeric_limits<difference_type>::max)()),
                  "compile-time offset does not fit in the iterator difference_type");
    return it.template next<static_cast<difference_type>(N)>();
  }
}

// Run-time offset.
template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr auto next(const iterator<Iterator, ValueType, Alignment>& it, std::size_t n) {
  return it + static_cast<typename iterator<Iterator, ValueType, Alignment>::difference_type>(n);
}

static_assert(
    bit_iterator_c<bit_iterator<uint8_t*>>,
    "bit_iterator does not satisfy bit_iterator_c concept!");
static_assert(
    bit_pointer_c<bit_pointer<uint8_t>, reference<uint8_t&>>,
    "bit_pointer does not satisfy bit_pointer_c concept!");
static_assert(
    bit_iterator_c<bit_pointer<uint8_t>>,
    "bit_pointer does not satisfy bit_iterator_c concept!");

}  // namespace bit

#endif  // _BIT_ITERATOR_DEFINITION_HPP_INCLUDED
