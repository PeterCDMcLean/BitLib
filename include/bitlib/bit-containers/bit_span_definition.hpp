#ifndef _BIT_SPAN_DEFINITION_HPP_INCLUDED
#define _BIT_SPAN_DEFINITION_HPP_INCLUDED
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <type_traits>
// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_ref.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit-containers/bit_span_definition.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

static_assert(bit_range<bit_span<uint8_t, 11>>, "bit_span does not satisfy bit_range concept!");
static_assert(bit_sized_range<bit_span<uint8_t, 11>>, "bit_span does not satisfy bit_sized_range concept!");
static_assert(bit_range<bit_span<uint8_t>>, "bit_span does not satisfy bit_range concept!");
static_assert(bit_sized_range<bit_span<uint8_t>>, "bit_span does not satisfy bit_sized_range concept!");

// Class Template Argument Deduction

// CTAD guide for the constructor taking a WordType&,
// deducing Extent as bitsof<WordType>().
template <typename WordType>
bit_span(WordType&) -> bit_span<WordType, bitsof<WordType>()>;
template <typename WordType>
bit_span(WordType*) -> bit_span<WordType, bitsof<WordType>()>;

// CTAD guide for the constructor taking a WordType* and a size,
// which should always be used when Extent is std::dynamic_extent.
template <typename WordType>
bit_span(WordType&, std::size_t) -> bit_span<WordType, std::dynamic_extent>;
template <typename WordType>
bit_span(WordType*, std::size_t) -> bit_span<WordType, std::dynamic_extent>;

// --- Constructors ---

// Default constructor:
// For dynamic extent, we start with a null pointer and size zero.
// For fixed extent, the pointer is null but the size is always Extent.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span() noexcept
    : bit_span_storage<WordType, Extent, Alignment>(), data_() {}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span(const bit_span& other) noexcept
    : bit_span_storage<WordType, Extent, Alignment>(other), data_(other.data_) {
}

// Construct from a bit_pointer and a bit count.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span(pointer data, size_type bit_count) noexcept
  requires(Extent == std::dynamic_extent)
    : bit_span_storage<WordType, Extent, Alignment>(bit_count), data_(data) {}

// Construct from a WordType pointer and a word count.
// This converts the word count to a bit count using bitsof.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span(WordType* word_ptr, size_type bit_count) noexcept
  requires(Extent == std::dynamic_extent)
    : bit_span_storage<WordType, Extent, Alignment>(bit_count), data_(word_ptr) {}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span(WordType& word_ref, size_type bit_count) noexcept
  requires(Extent == std::dynamic_extent)
    : bit_span(&word_ref, bit_count) {}

// Construct from a bit_pointer and a bit count.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span(pointer data) noexcept
    : bit_span_storage<WordType, Extent, Alignment>(), data_(data) {}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span(WordType* word_ptr) noexcept
    : bit_span_storage<WordType, Extent, Alignment>(), data_(word_ptr) {}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span(WordType* word_ptr)
  requires(std::is_scalar_v<WordType>)
    : bit_span_storage<WordType, Extent, Alignment>(), data_(word_ptr) {
}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::bit_span(WordType& word_ref)
  requires(std::is_scalar_v<WordType>)
    : bit_span(&word_ref) {
}

// --- Observers ---

// Returns the number of bits in the span.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::size_type bit_span<WordType, Extent, Alignment, Policy>::size() const noexcept {
  if constexpr (Extent == std::dynamic_extent) {
    return this->size_;
  } else {
    return Extent;
  }
}

// Checks if the span is empty.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bool bit_span<WordType, Extent, Alignment, Policy>::empty() const noexcept { return size() == 0; }

// --- Element Access ---

// Non-const element access.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::reference bit_span<WordType, Extent, Alignment, Policy>::operator[](size_type pos) { return begin()[pos]; }

// Const element access returns a const_reference.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::const_reference bit_span<WordType, Extent, Alignment, Policy>::operator[](size_type pos) const { return begin()[pos]; }

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::reference bit_span<WordType, Extent, Alignment, Policy>::at(size_type pos) {
  if (pos >= size()) {
    throw std::out_of_range("span::at - index out of range");
  }
  return begin()[pos];
}

// Const element access returns a const_reference.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::const_reference bit_span<WordType, Extent, Alignment, Policy>::at(size_type pos) const {
  if (pos >= size()) {
    throw std::out_of_range("span::at - index out of range");
  }
  return begin()[pos];
}

// Non-const front/back access.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::reference bit_span<WordType, Extent, Alignment, Policy>::front() { return begin()[0]; }
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::reference bit_span<WordType, Extent, Alignment, Policy>::back() { return begin()[size() - 1]; }

// Const front/back access returns a const_reference.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::const_reference bit_span<WordType, Extent, Alignment, Policy>::front() const { return begin()[0]; }
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::const_reference bit_span<WordType, Extent, Alignment, Policy>::back() const { return begin()[size() - 1]; }

// --- Iterators ---

// Returns an iterator to the first bit.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::iterator bit_span<WordType, Extent, Alignment, Policy>::begin() const noexcept { return data_; }

// Returns an iterator just past the last bit.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, Extent, Alignment, Policy>::iterator bit_span<WordType, Extent, Alignment, Policy>::end() const noexcept { return begin() + size(); }

// --- Subspan Functionality ---

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
template <std::size_t NewOffset, std::size_t NewExtent>
constexpr auto bit_span<WordType, Extent, Alignment, Policy>::subspan() const noexcept
    -> bit_span<WordType, subspan_extent<NewOffset, NewExtent>(), Policy> {
  using NewSpanType = bit_span<WordType, subspan_extent<NewOffset, NewExtent>(), Policy>;

  if constexpr (NewExtent == std::dynamic_extent) {
    return NewSpanType(begin() + NewOffset, size() - NewOffset);
  } else {
    return NewSpanType(begin() + NewOffset);
  }
}

// Creates a subspan starting at offset with count bits.
template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, std::dynamic_extent, Policy> bit_span<WordType, Extent, Alignment, Policy>::subspan(size_type offset, size_type count) const noexcept
  requires(Extent == std::dynamic_extent)
{
  size_type new_count = (count == std::dynamic_extent) ? size() - offset : count;
  return bit_span<WordType, Extent, Policy>(begin() + offset, new_count);
}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr auto bit_span<WordType, Extent, Alignment, Policy>::operator()(size_type begin, size_type end) const noexcept {
  return bit_array_ref<std::dynamic_extent, WordType, Policy>(&(this->begin()[begin]), end - begin);
}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
template <std::size_t NewExtent>
constexpr bit_span<WordType, NewExtent, Policy> bit_span<WordType, Extent, Alignment, Policy>::first() const noexcept
  requires(NewExtent != std::dynamic_extent)
{
  return bit_span<WordType, NewExtent, Policy>(begin());
}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, std::dynamic_extent, Policy> bit_span<WordType, Extent, Alignment, Policy>::first(size_type offset) const noexcept
  requires(Extent == std::dynamic_extent)
{
  return bit_span<WordType, Extent, Policy>(begin() + offset);
}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
template <std::size_t RevOffset>
constexpr bit_span<WordType, Extent - RevOffset, Policy> bit_span<WordType, Extent, Alignment, Policy>::last() const noexcept
  requires(RevOffset != std::dynamic_extent)
{
  return bit_span<WordType, Extent - RevOffset, Policy>(begin() + size() - RevOffset);
}

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
constexpr bit_span<WordType, std::dynamic_extent, Policy> bit_span<WordType, Extent, Alignment, Policy>::last(size_type offset) const noexcept
  requires(Extent == std::dynamic_extent)
{
  return bit_span<WordType, Extent, Policy>(begin() + size() - offset);
}

}  // namespace bit
#endif  // _BIT_SPAN_DEFINITION_HPP_INCLUDED
