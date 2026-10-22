#ifndef _BIT_ITERATOR_DECLARATION_HPP_INCLUDED
#define _BIT_ITERATOR_DECLARATION_HPP_INCLUDED

#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_iterator_forward.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

/* ****************************** BIT ITERATOR ****************************** */

namespace detail {
template <typename size_type, size_t Alignment>
struct iterator_position_storage;

template <typename size_type>
struct iterator_position_storage<size_type, std::dynamic_extent> {
  size_type _position;
  iterator_position_storage() : _position(0) {}
  iterator_position_storage(size_type position) : _position(position) {
  }
  constexpr size_type position() const noexcept {
    return _position;
  }
  constexpr size_type& position() noexcept {
    return _position;
  }
  constexpr bool operator==(const iterator_position_storage&) const noexcept = default;
  constexpr auto operator<=>(const iterator_position_storage&) const noexcept = default;
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
  constexpr bool operator==(const iterator_position_storage&) const noexcept = default;
  constexpr auto operator<=>(const iterator_position_storage&) const noexcept = default;
};
}  // namespace detail

// Generic bit iterator with configurable primitive value type and alignment.
// bit_iterator remains as an alias to the default instantiation (a
// single-bit iterator, i.e. ValueType == primitive<1>, whose value_type is
// bit_value).
// Alignment of std::dynamic_extent means no compile-time alignment
// and the mutable position is tracked at run-time
// Alignment >= 0 and < bitsof(word_type) means the iterator's position is
// not mutable
template <
    class Iterator,
    primitive_c ValueType,
    size_t Alignment>
class iterator : public detail::iterator_position_storage<std::size_t, Alignment> {
  template <class, primitive_c, size_t>
  friend class iterator;

  // Assertions
 private:
  using _traits_t = _cv_iterator_traits<Iterator>;
  static_assert(Alignment == std::dynamic_extent || Alignment < bitsof<uintptr_t>(),
                "alignment exceeds the native integer size of the architecture");

  // Types
 private:
  using underlying_value_type =
      std::remove_cv_t<std::iter_value_t<Iterator>>;

  // The storage word is the underlying value type when it is already an
  // integral type (bool is excluded because it has no usable unsigned
  // counterpart). Otherwise, infer the largest integral type that respects
  // the value type's alignment, so multi-word objects can still be traversed
  // with native word operations.
  using iterator_word_type = std::conditional_t<
      std::is_integral_v<underlying_value_type> &&
          !std::is_same_v<underlying_value_type, bool>,
      underlying_value_type,
      detail::inferred_word_type<underlying_value_type>>;

 public:
  using iterator_type = Iterator;
  using word_type = iterator_word_type;
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
    requires std::constructible_from<iterator_type, T> && (Alignment == std::dynamic_extent || Alignment == O);
  explicit constexpr iterator(iterator_type i);
  constexpr iterator(iterator_type i, size_type pos)
    requires(Alignment == std::dynamic_extent);
  explicit constexpr iterator(const pointer& ptr)
    requires std::constructible_from<iterator_type, word_type*>;

  template <typename T>
  constexpr iterator(const iterator<bit_word_pointer_adapter<T, Iterator>,
                                    ValueType, Alignment>& other);

  // Assignment
 public:
  constexpr iterator& operator=(const iterator& other)
    requires(Alignment == std::dynamic_extent);
  template <class T, primitive_c U, size_t O>
  constexpr iterator& operator=(const iterator<T, U, O>& other)
    requires(Alignment == std::dynamic_extent);
  constexpr iterator& operator=(const iterator& other)
    requires(Alignment != std::dynamic_extent);
  template <class T, primitive_c U, size_t O>
  constexpr iterator& operator=(const iterator<T, U, O>& other)
    requires(Alignment == O);

  // Access
 public:
  constexpr reference operator*() const noexcept;
  constexpr pointer operator->() const noexcept;
  constexpr reference operator[](difference_type n) const;

  // Increment and decrement operators - advance by ValueType::bits elements
 public:
  constexpr iterator& operator++()
    requires(Alignment == std::dynamic_extent);
  constexpr iterator& operator--()
    requires(Alignment == std::dynamic_extent);
  constexpr iterator operator++(int)
    requires(Alignment == std::dynamic_extent);
  constexpr iterator operator--(int)
    requires(Alignment == std::dynamic_extent);
  constexpr iterator<Iterator, ValueType, std::dynamic_extent> operator+(difference_type n) const;
  constexpr iterator<Iterator, ValueType, std::dynamic_extent> operator-(difference_type n) const;
  template <difference_type N>
  constexpr iterator<Iterator, ValueType, std::dynamic_extent> next() const
    requires(Alignment == std::dynamic_extent);
  template <difference_type N>
  constexpr auto next() const
    requires(Alignment != std::dynamic_extent);
  constexpr iterator& operator+=(difference_type n)
    requires(Alignment == std::dynamic_extent);
  constexpr iterator& operator-=(difference_type n)
    requires(Alignment == std::dynamic_extent);
  constexpr difference_type operator-(const iterator&) const;

  // Underlying details
 public:
  constexpr iterator_type base() const;
  constexpr iterator_type address() const;

  // Comparison: two iterators are equal when they refer to the same underlying
  // word and the same bit position within it. Defaulted: compares the base
  // position storage first, then the underlying iterator.
  constexpr auto operator<=>(const iterator&) const noexcept = default;
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

// Non-member arithmetic operators
template <class T, primitive_c U, size_t O>
constexpr iterator<T, U, O> operator+(
    typename iterator<T, U, O>::difference_type n,
    const iterator<T, U, O>& i);

template <class T, primitive_c U, size_t O, class V, primitive_c W, size_t P>
constexpr typename std::common_type<
    typename iterator<T, U, O>::difference_type,
    typename iterator<V, W, P>::difference_type>::type
operator-(const iterator<T, U, O>& lhs, const iterator<V, W, P>& rhs);

// Free-function counterparts of iterator::next<N>().
template <
    std::size_t N = std::dynamic_extent,
    class Iterator,
    primitive_c ValueType,
    size_t Alignment>
constexpr auto next(const iterator<Iterator, ValueType, Alignment>& it);

template <class Iterator, primitive_c ValueType, size_t Alignment>
constexpr auto next(const iterator<Iterator, ValueType, Alignment>& it, std::size_t n);

}  // namespace bit

#endif  // _BIT_ITERATOR_DECLARATION_HPP_INCLUDED
