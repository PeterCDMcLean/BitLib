#ifndef _BIT_SPAN_HPP_INCLUDED
#define _BIT_SPAN_HPP_INCLUDED
#include <cstddef>
#include <cassert>

#include <span>
#include <cmath>
#include <string>
#include <iostream>
#include <algorithm>
#include <type_traits>
// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-iterator/bit.hpp"

namespace bit {

namespace policy {
struct typical;
}

template <typename T, typename W, typename Policy>
class bit_array_ref;

// Helper storage: for a fixed extent no runtime size is stored.
template<typename WordType, std::size_t Extent>
struct bit_span_storage {
    constexpr std::size_t size() const noexcept { return Extent; }
    bit_span_storage() = default;
};

// Specialization for dynamic_extent: store the size at runtime.
template<typename WordType>
struct bit_span_storage<WordType, std::dynamic_extent> {
    std::size_t size_;
    constexpr std::size_t size() const noexcept { return size_; }
    bit_span_storage() : size_(0) {}
    bit_span_storage(std::size_t size) : size_(size) {}
};

template <typename WordType = uint8_t, std::size_t Extent = std::dynamic_extent, typename Policy = policy::typical>
class bit_span : private bit_span_storage<WordType, Extent> {
 public:
  using word_type = WordType;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = bit_reference<WordType&>;
  using const_reference = const bit_reference<WordType&>;
  using pointer = bit_pointer<WordType>;
  using const_pointer = const pointer;
  using iterator = bit_iterator<WordType*>;
  using const_iterator = bit_iterator<const WordType*>;

 private:
  // The start of the span, represented as a bit_pointer.
  pointer data_;

  template <std::size_t NewOffset, std::size_t NewCount>
  static constexpr size_t subspan_extent() {
    if constexpr (NewCount != std::dynamic_extent)
      return NewCount;
    else if constexpr (Extent != std::dynamic_extent)
      return Extent - NewOffset;
    else
      return std::dynamic_extent;
  }

 public:
  // --- Constructors ---

  // Default constructor:
  constexpr bit_span() noexcept;

  constexpr bit_span(const bit_span& other) noexcept;

  // Construct from a bit_pointer and a bit count.
  constexpr bit_span(pointer data, size_type bit_count) noexcept
    requires(Extent == std::dynamic_extent);
  constexpr bit_span(pointer data) noexcept;

  // Construct from a WordType pointer and a bit count.
  constexpr bit_span(WordType* word_ptr, size_type bit_count) noexcept
    requires(Extent == std::dynamic_extent);
  constexpr bit_span(WordType* word_ptr) noexcept;
  constexpr bit_span(WordType& word_ref, size_type bit_count) noexcept
    requires(Extent == std::dynamic_extent);
  constexpr bit_span(WordType& word_ref) noexcept;

  constexpr bit_span(WordType& s)
    requires(std::is_scalar_v<WordType>);
  constexpr bit_span(WordType* s)
    requires(std::is_scalar_v<WordType>);

  // --- Observers ---

  // Returns the number of bits in the span.
  constexpr size_type size() const noexcept;
  // Checks if the span is empty.
  constexpr bool empty() const noexcept;

  // --- Element Access ---

  // Non-const element access.
  constexpr reference operator[](size_type pos);
  // Const element access returns a const_reference.
  constexpr const_reference operator[](size_type pos) const;
  // Bounds-checked access
  constexpr reference at(size_type idx);
  // Bounds-checked access
  constexpr const_reference at(size_type idx) const;

  // Non-const front/back access.
  constexpr reference front();
  constexpr reference back();

  // Const front/back access returns a const_reference.
  constexpr const_reference front() const;
  constexpr const_reference back() const;

  // --- Iterators ---

  // Returns an iterator to the first bit.
  constexpr iterator begin() const noexcept;

  // Returns an iterator just past the last bit.
  constexpr iterator end() const noexcept;

  // --- Subspan Functionality ---

  // Creates a subspan starting at offset with count bits.
  template <std::size_t NewOffset, std::size_t NewExtent = std::dynamic_extent>
  constexpr auto subspan() const noexcept -> bit_span<WordType, subspan_extent<NewOffset, NewExtent>()>;
  constexpr bit_span<WordType, std::dynamic_extent> subspan(size_type offset, size_type count = std::dynamic_extent) const noexcept
    requires(Extent == std::dynamic_extent);

  constexpr bit_array_ref<bit_value, WordType, Policy> operator()(size_type begin, size_type end) const noexcept;

  template <std::size_t NewExtent>
  constexpr bit_span<WordType, NewExtent> first() const noexcept
    requires(NewExtent != std::dynamic_extent);
  constexpr bit_span<WordType, std::dynamic_extent> first(size_type offset) const noexcept
    requires(Extent == std::dynamic_extent);

  template <std::size_t RevOffset>
  constexpr bit_span<WordType, Extent - RevOffset> last() const noexcept
    requires(RevOffset != std::dynamic_extent);
  constexpr bit_span<WordType, std::dynamic_extent> last(size_type offset) const noexcept
    requires(Extent == std::dynamic_extent);
};

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
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span() noexcept : bit_span_storage<WordType, Extent>(), data_(nullptr) {}

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span(const bit_span& other) noexcept : bit_span_storage<WordType, Extent>(other), data_(other.data_) {
}

// Construct from a bit_pointer and a bit count.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span(pointer data, size_type bit_count) noexcept
  requires(Extent == std::dynamic_extent)
    : bit_span_storage<WordType, Extent>(bit_count), data_(data) {}

// Construct from a WordType pointer and a word count.
// This converts the word count to a bit count using bitsof.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span(WordType* word_ptr, size_type bit_count) noexcept
  requires(Extent == std::dynamic_extent)
    : bit_span_storage<WordType, Extent>(bit_count), data_(pointer(word_ptr, 0)) {}

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span(WordType& word_ref, size_type bit_count) noexcept
  requires(Extent == std::dynamic_extent)
    : bit_span(&word_ref, bit_count) {}

// Construct from a bit_pointer and a bit count.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span(pointer data) noexcept : bit_span_storage<WordType, Extent>(), data_(data) {}

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span(WordType* word_ptr) noexcept : bit_span_storage<WordType, Extent>(), data_(pointer(word_ptr, 0)) {}

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span(WordType* word_ptr)
  requires(std::is_scalar_v<WordType>)
    : bit_span_storage<WordType, Extent>(), data_(word_ptr) {
}

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::bit_span(WordType& word_ref)
  requires(std::is_scalar_v<WordType>)
    : bit_span(&word_ref) {
}

// --- Observers ---

// Returns the number of bits in the span.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::size_type bit_span<WordType, Extent, Policy>::size() const noexcept {
  if constexpr (Extent == std::dynamic_extent) {
    return this->size_;
  } else {
    return Extent;
  }
}

// Checks if the span is empty.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bool bit_span<WordType, Extent, Policy>::empty() const noexcept { return size() == 0; }

// --- Element Access ---

// Non-const element access.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::reference bit_span<WordType, Extent, Policy>::operator[](size_type pos) { return begin()[pos]; }

// Const element access returns a const_reference.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::const_reference bit_span<WordType, Extent, Policy>::operator[](size_type pos) const { return begin()[pos]; }

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::reference bit_span<WordType, Extent, Policy>::at(size_type pos) {
  if (pos >= size()) {
    throw std::out_of_range("span::at - index out of range");
  }
  return data_[pos];
}

// Const element access returns a const_reference.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::const_reference bit_span<WordType, Extent, Policy>::at(size_type pos) const {
  if (pos >= size()) {
    throw std::out_of_range("span::at - index out of range");
  }
  return data_[pos];
}

// Non-const front/back access.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::reference bit_span<WordType, Extent, Policy>::front() { return begin()[0]; }
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::reference bit_span<WordType, Extent, Policy>::back() { return begin()[size() - 1]; }

// Const front/back access returns a const_reference.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::const_reference bit_span<WordType, Extent, Policy>::front() const { return begin()[0]; }
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::const_reference bit_span<WordType, Extent, Policy>::back() const { return begin()[size() - 1]; }

// --- Iterators ---

// Returns an iterator to the first bit.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::iterator bit_span<WordType, Extent, Policy>::begin() const noexcept { return iterator(data_); }

// Returns an iterator just past the last bit.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, Extent, Policy>::iterator bit_span<WordType, Extent, Policy>::end() const noexcept { return begin() + size(); }

// --- Subspan Functionality ---

template <typename WordType, std::size_t Extent, typename Policy>
template <std::size_t NewOffset, std::size_t NewExtent>
constexpr auto bit_span<WordType, Extent, Policy>::subspan() const noexcept
    -> bit_span<WordType, subspan_extent<NewOffset, NewExtent>()> {
  using NewSpanType = bit_span<WordType, subspan_extent<NewOffset, NewExtent>()>;

  if constexpr (NewExtent == std::dynamic_extent) {
    return NewSpanType(begin() + NewOffset, size() - NewOffset);
  } else {
    return NewSpanType(begin() + NewOffset);
  }
}

// Creates a subspan starting at offset with count bits.
template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, std::dynamic_extent> bit_span<WordType, Extent, Policy>::subspan(size_type offset, size_type count) const noexcept
  requires(Extent == std::dynamic_extent)
{
  size_type new_count = (count == std::dynamic_extent) ? size() - offset : count;
  return bit_span<WordType, Extent>(begin() + offset, new_count);
}

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_array_ref<bit_value, WordType, Policy> bit_span<WordType, Extent, Policy>::operator()(size_type begin, size_type end) const noexcept {
  return bit_array_ref<bit_value, WordType, Policy>(&(this->begin()[begin]), end - begin);
}

template <typename WordType, std::size_t Extent, typename Policy>
template <std::size_t NewExtent>
constexpr bit_span<WordType, NewExtent> bit_span<WordType, Extent, Policy>::first() const noexcept
  requires(NewExtent != std::dynamic_extent)
{
  return bit_span<WordType, NewExtent>(data_);
}

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, std::dynamic_extent> bit_span<WordType, Extent, Policy>::first(size_type offset) const noexcept
  requires(Extent == std::dynamic_extent)
{
  return bit_span<WordType, Extent>(begin() + offset);
}

template <typename WordType, std::size_t Extent, typename Policy>
template <std::size_t RevOffset>
constexpr bit_span<WordType, Extent - RevOffset> bit_span<WordType, Extent, Policy>::last() const noexcept
  requires(RevOffset != std::dynamic_extent)
{
  return bit_span<WordType, Extent-RevOffset>(begin() + size() - RevOffset);
}

template <typename WordType, std::size_t Extent, typename Policy>
constexpr bit_span<WordType, std::dynamic_extent> bit_span<WordType, Extent, Policy>::last(size_type offset) const noexcept
  requires(Extent == std::dynamic_extent)
{
}

} // namespace bit
#endif
