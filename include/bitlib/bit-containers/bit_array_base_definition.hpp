#ifndef _BIT_ARRAY_BASE_DEFINITION_HPP_INCLUDED
#define _BIT_ARRAY_BASE_DEFINITION_HPP_INCLUDED

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <memory>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_base_declaration.hpp"
#include "bitlib/bit-containers/bit_array_definition.hpp"
#include "bitlib/bit-containers/bit_array_ref_definition.hpp"
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr Derived& array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::derived() noexcept {
  return static_cast<Derived&>(*this);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr const Derived& array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::derived() const noexcept {
  return static_cast<const Derived&>(*this);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::aligned_begin() noexcept
  requires(A == std::dynamic_extent)
{
  return derived().begin();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::aligned_end() noexcept
  requires(A == std::dynamic_extent || N == std::dynamic_extent)
{
  return derived().begin() + size();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::aligned_begin() noexcept
  requires(A != std::dynamic_extent)
{
  return ::bit::iterator<word_type*, value_type, A>(derived().data());
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::aligned_end() noexcept
  requires(!(A == std::dynamic_extent || N == std::dynamic_extent))
{
  constexpr size_type end_index = (A + N * bitsof<value_type>()) / bitsof<word_type>();
  constexpr size_type end_pos = (A + N * bitsof<value_type>()) % bitsof<word_type>();
  return ::bit::iterator<word_type*, value_type, end_pos>(derived().data()[end_index]);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::aligned_begin() const noexcept
  requires(A == std::dynamic_extent)
{
  return const_iterator(derived().begin());
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::aligned_end() const noexcept
  requires(A == std::dynamic_extent || N == std::dynamic_extent)
{
  return const_iterator(derived().begin() + size());
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::aligned_begin() const noexcept
  requires(A != std::dynamic_extent)
{
  return ::bit::iterator<const word_type*, value_type, A>(derived().data());
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::aligned_end() const noexcept
  requires(!(A == std::dynamic_extent || N == std::dynamic_extent))
{
  constexpr size_type end_index = (A + N * bitsof<value_type>()) / bitsof<word_type>();
  constexpr size_type end_pos = (A + N * bitsof<value_type>()) % bitsof<word_type>();
  return ::bit::iterator<const word_type*, value_type, end_pos>(&derived().data()[end_index]);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::array_base() noexcept
    : detail::container_size_storage<std::size_t, resizable, N>() {}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::array_base(const size_type& extent) noexcept
  requires(N == std::dynamic_extent)
    : detail::container_size_storage<std::size_t, resizable, N>(extent) {}

// Element access
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::reference
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator[](size_type pos) {
  return derived().begin()[pos];
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::const_reference
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator[](size_type pos) const {
  return derived().begin()[pos];
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::reference
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::at(size_type pos) {
  if (pos < size()) {
    return derived().begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::const_reference
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::at(size_type pos) const {
  if (pos < size()) {
    return derived().begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::reference
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::front()
  requires(A == std::dynamic_extent)
{
  return derived().begin()[0];
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::front()
  requires(A != std::dynamic_extent)
{
  return ::bit::reference<word_type&, value_type, A>(*derived().data());
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::const_reference
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::front() const
  requires(A == std::dynamic_extent)
{
  return derived().begin()[0];
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::front() const
  requires(A != std::dynamic_extent)
{
  return ::bit::reference<const word_type&, value_type, A>(*derived().data());
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::reference
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::back()
  requires(A == std::dynamic_extent || N == std::dynamic_extent)
{
  return derived().begin()[size() - 1];
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::back()
  requires(!(A == std::dynamic_extent || N == std::dynamic_extent))
{
  constexpr size_type back_index = (A + N * bitsof<value_type>() - 1) / bitsof<word_type>();
  constexpr size_type back_pos = (A + N * bitsof<value_type>() - 1) % bitsof<word_type>();
  return ::bit::reference<word_type&, value_type, back_pos>(derived().data()[back_index]);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::const_reference
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::back() const
  requires(A == std::dynamic_extent || N == std::dynamic_extent)
{
  return derived().begin()[size() - 1];
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::back() const
  requires(!(A == std::dynamic_extent || N == std::dynamic_extent))
{
  constexpr size_type back_index = (A + N * bitsof<value_type>() - 1) / bitsof<word_type>();
  constexpr size_type back_pos = (A + N * bitsof<value_type>() - 1) % bitsof<word_type>();
  return ::bit::reference<const word_type&, value_type, back_pos>(derived().data()[back_index]);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::iterator
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::end() noexcept {
  return derived().begin() + size();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::const_iterator
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::end() const noexcept {
  return const_iterator(derived().begin()) + size();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::const_iterator
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::cbegin() const noexcept {
  return const_iterator(derived().begin());
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::const_iterator
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::cend() const noexcept {
  return const_iterator(derived().end());
}

// Capacity
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr bool array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::empty() const noexcept {
  return 0 == size();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::size_type
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::max_size() const noexcept {
  return size();
}

// String representation
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr std::string
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::debug_string() const {
  return debug_string(derived().begin(), derived().end());
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr std::string
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::debug_string(const_iterator first, const_iterator last) const {
  std::string ret = "";
  auto position = 0;
  for (auto it = first; it != last; ++it) {
    if (position % bitsof<word_type>() == 0 && position != 0) {
      ret += " ";
    } else if (position % 8 == 0 && position != 0) {
      ret += '.';
    }
    ret += *it == bit1 ? '1' : '0';
    ++position;
  }
  return ret;
}

/**
 * @brief Slice operations - returns a array_ref
 */
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator()(size_type offset, size_type right) const noexcept {
  return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(&this->at(offset), right - offset);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator()(size_type offset, size_type right) noexcept {
  return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(&this->at(offset), right - offset);
}

/**
 * @brief Compile-time slice [Start, End) - returns a array_ref.
 *
 * Supports all combinations of Start/End being compile-time constants or
 * runtime values:
 *   slice<Start, End>()          - both compile-time constants
 *   slice<Start>(end)            - Start compile-time, End runtime
 *   slice<End>(start)            - Start runtime, End compile-time
 *
 * @tparam Start Start offset in bits
 * @tparam End End offset in bits
 */
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start, std::size_t End>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice() const noexcept
  requires(N != std::dynamic_extent && Start <= End && End <= N)
{
  return array_ref<value_type, End - Start, const word_type, std::dynamic_extent, Policy>(&this->at(Start));
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start, std::size_t End>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice() noexcept
  requires(N != std::dynamic_extent && Start <= End && End <= N)
{
  return array_ref<value_type, End - Start, word_type, std::dynamic_extent, Policy>(&this->at(Start));
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice(size_t end) const noexcept
  requires(Start != std::dynamic_extent && N != std::dynamic_extent && Start <= N)
{
  return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
      &this->at(Start), end - Start);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice(size_t end) noexcept
  requires(Start != std::dynamic_extent && N != std::dynamic_extent && Start <= N)
{
  return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
      &this->at(Start), end - Start);
}

/**
 * @brief Runtime Start, compile-time End slice [start, End) - returns a array_ref.
 *
 * Start=std::dynamic_extent means the start offset is a runtime argument.
 *
 * @tparam Start std::dynamic_extent (runtime start sentinel)
 * @tparam End End offset in bits (compile-time constant)
 * @param start Start offset in bits (runtime)
 */
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start, std::size_t End>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice(size_t start) const noexcept
  requires(Start == std::dynamic_extent && N != std::dynamic_extent)
{
  return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
      &this->at(start), End - start);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start, std::size_t End>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice(size_t start) noexcept
  requires(Start == std::dynamic_extent && N != std::dynamic_extent)
{
  return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
      &this->at(start), End - start);
}

/**
 * @brief Compile-time slice [Start, End) - returns a array_ref.
 *
 * Supports all combinations of Start/End being compile-time constants or
 * runtime values:
 *   slice<Start, End>()          - both compile-time constants
 *   slice<Start>(end)            - Start compile-time, End runtime
 *   slice<End>(start)            - Start runtime, End compile-time
 *
 * @tparam Start Start offset in bits
 * @tparam End End offset in bits
 */
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start, std::size_t End>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice() const noexcept
  requires(N == std::dynamic_extent && Start <= End)
{
  return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
      &this->at(Start), End - Start);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start, std::size_t End>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice() noexcept
  requires(N == std::dynamic_extent && Start <= End)
{
  return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
      &this->at(Start), End - Start);
}

/**
 * @brief Compile-time Start, runtime End slice [Start, end) - returns a array_ref.
 *
 * @tparam Start Start offset in bits (compile-time constant)
 * @param end End offset in bits (runtime)
 */
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice(size_t end) const noexcept
  requires(N == std::dynamic_extent)
{
  return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
      &this->at(Start), end - Start);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice(size_t end) noexcept
  requires(N == std::dynamic_extent)
{
  return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
      &this->at(Start), end - Start);
}

/**
 * @brief Runtime Start, compile-time End slice [start, End) - returns a array_ref.
 *
 * Start=std::dynamic_extent means the start offset is a runtime argument.
 * End is a compile-time constant upper bound.
 *
 * @tparam Start std::dynamic_extent (runtime start)
 * @tparam End End offset in bits (compile-time constant)
 * @param start Start offset in bits (runtime)
 */
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start, std::size_t End>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice(size_t start) const noexcept
  requires(Start == std::dynamic_extent && N == std::dynamic_extent)
{
  return array_ref<value_type, std::dynamic_extent, const word_type, std::dynamic_extent, Policy>(
      &this->at(start), End - start);
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::size_t Start, std::size_t End>
constexpr auto
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::slice(size_t start) noexcept
  requires(Start == std::dynamic_extent && N == std::dynamic_extent)
{
  return array_ref<value_type, std::dynamic_extent, word_type, std::dynamic_extent, Policy>(
      &this->at(start), End - start);
}

// Common operations
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr void array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::fill(value_type bit_val) noexcept {
  std::fill(derived().begin(), derived().end(), bit_val);
}

/**
 * @brief Explicit conversion to integral types
 */
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <std::integral U>
/* explicit */ constexpr array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator U() const noexcept {
  assert(size() <= bitsof<U>());
  U integral;

  if constexpr (N == std::dynamic_extent) {
    if (size() > bitsof<U>()) {
      Policy::truncation::template to_integral<U, N>(derived(), integral);
    } else {
      ::bit::copy(aligned_begin(), aligned_end(), &integral);  // TODO use iterator with alignment when possible
    }
    if (size() < bitsof<U>()) {
      Policy::extension::template to_integral<U, N>(detail::uninitialized, derived(), integral);
    }
  } else {
    if constexpr (N > bitsof<U>()) {
      Policy::truncation::template to_integral<U, N>(derived(), integral);
    } else {
      ::bit::copy(aligned_begin(), aligned_end(), &integral);  // TODO use iterator with alignment when possible
    }
    if constexpr (N < bitsof<U>()) {
      Policy::extension::template to_integral<U, N>(detail::uninitialized, derived(), integral);
    }
  }

  return integral;
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::compatible_bitarray
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator~() {
  compatible_bitarray result(detail::uninitialized, size());
  ::bit::transform(derived().begin(), derived().end(), result.begin(), [](const word_type& bits) -> word_type { return ~bits; });
  return result;
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::compatible_bitarray
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator|(const bit_sized_range auto& other) const {
  assert(other.size() == size());
  compatible_bitarray result(detail::uninitialized, size());
  ::bit::transform(derived().begin(), derived().end(), other.begin(), result.begin(),
                   [](const word_type& a, const word_type& b) -> word_type { return a | b; });
  return result;
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr Derived&
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator|=(const bit_sized_range auto& other) {
  assert(other.size() == size());
  ::bit::transform(derived().cbegin(), derived().cend(), other.begin(), derived().begin(),
                   [](const word_type& a, const word_type& b) -> word_type { return a | b; });
  return derived();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::compatible_bitarray
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator&(const bit_sized_range auto& other) const {
  assert(other.size() == size());
  compatible_bitarray result(detail::uninitialized, size());
  ::bit::transform(derived().begin(), derived().end(), other.begin(), result.begin(),
                   [](const word_type& a, const word_type& b) -> word_type { return a & b; });
  return result;
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr Derived&
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator&=(const bit_sized_range auto& other) {
  assert(other.size() == size());
  ::bit::transform(derived().cbegin(), derived().cend(), other.begin(), derived().begin(),
                   [](const word_type& a, const word_type& b) -> word_type { return a & b; });
  return derived();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::compatible_bitarray
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator^(const bit_sized_range auto& other) const {
  assert(other.size() == size());
  compatible_bitarray result(detail::uninitialized, size());
  ::bit::transform(derived().begin(), derived().end(), other.begin(), result.begin(),
                   [](const word_type& a, const word_type& b) -> word_type { return a ^ b; });
  return result;
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
constexpr Derived&
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator^=(const bit_sized_range auto& other) {
  assert(other.size() == size());
  ::bit::transform(derived().cbegin(), derived().cend(), other.begin(), derived().begin(),
                   [](const word_type& a, const word_type& b) -> word_type { return a ^ b; });
  return derived();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <primitive_c P>
  requires(std::is_same_v<T, bit_value> && !bit_sized_range<P>)
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::compatible_bitarray
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator|(const P& other) const {
  const auto view = array_ref(other);
  return derived() | view;
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <primitive_c P>
  requires(std::is_same_v<T, bit_value> && !bit_sized_range<P>)
constexpr Derived&
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator|=(const P& other) {
  const auto view = array_ref(other);
  assert(view.size() == size());
  ::bit::transform(derived().cbegin(), derived().cend(), view.begin(),
                   derived().begin(),
                   [](auto&& lhs, auto&& rhs) { return lhs | rhs; });
  return derived();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <primitive_c P>
  requires(std::is_same_v<T, bit_value> && !bit_sized_range<P>)
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::compatible_bitarray
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator&(const P& other) const {
  const auto view = array_ref(other);
  return derived() & view;
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <primitive_c P>
  requires(std::is_same_v<T, bit_value> && !bit_sized_range<P>)
constexpr Derived&
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator&=(const P& other) {
  const auto view = array_ref(other);
  assert(view.size() == size());
  ::bit::transform(derived().cbegin(), derived().cend(), view.begin(),
                   derived().begin(),
                   [](auto&& lhs, auto&& rhs) { return lhs & rhs; });
  return derived();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <primitive_c P>
  requires(std::is_same_v<T, bit_value> && !bit_sized_range<P>)
constexpr typename array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::compatible_bitarray
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator^(const P& other) const {
  const auto view = array_ref(other);
  return derived() ^ view;
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <primitive_c P>
  requires(std::is_same_v<T, bit_value> && !bit_sized_range<P>)
constexpr Derived&
array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::operator^=(const P& other) {
  const auto view = array_ref(other);
  assert(view.size() == size());
  ::bit::transform(derived().cbegin(), derived().cend(), view.begin(),
                   derived().begin(),
                   [](auto&& lhs, auto&& rhs) { return lhs ^ rhs; });
  return derived();
}

template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
template <typename U>
constexpr void array_base<Derived, T, N, W, A, resizable, Policy, Iterators>::from_integral(const U& integral) {
  if constexpr (N == std::dynamic_extent) {
    if ((size() * bitsof<value_type>()) < bitsof<U>()) {
      Policy::truncation::template from_integral<U, N>(integral, derived());
    } else {
      ::bit::copy(&integral, &integral + 1, aligned_begin());
    }
    if (bitsof<U>() < (size() * bitsof<value_type>())) {
      Policy::extension::template from_integral<U, N>(detail::uninitialized, integral, derived());
    }
  } else {
    if constexpr ((N * bitsof<value_type>()) < bitsof<U>()) {
      Policy::truncation::template from_integral<U, N>(integral, derived());
    } else {
      ::bit::copy(&integral, &integral + 1, aligned_begin());
    }
    if constexpr (bitsof<U>() < (N * bitsof<value_type>())) {
      Policy::extension::template from_integral<U, N>(detail::uninitialized, integral, derived());
    }
  }
}

constexpr bool operator==(const bit_sized_range auto& lhs, const bit_sized_range auto& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  return ::bit::equal(lhs.begin(), lhs.end(), rhs.begin());
}

}  // namespace bit

#endif  // _BIT_ARRAY_BASE_DEFINITION_HPP_INCLUDED
