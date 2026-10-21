#ifndef _BIT_SPAN_DECLARATION_HPP_INCLUDED
#define _BIT_SPAN_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <span>
#include <type_traits>

#include "bitlib/bit-containers/bit_span_forward.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"

namespace bit {

template <typename WordType, std::size_t Extent, std::size_t Alignment>
struct bit_span_storage {
  constexpr std::size_t size() const noexcept { return Extent; }
  bit_span_storage() = default;
};

template <typename WordType>
struct bit_span_storage<WordType, std::dynamic_extent> {
  std::size_t size_;
  constexpr std::size_t size() const noexcept { return size_; }
  bit_span_storage() : size_(0) {}
  bit_span_storage(std::size_t size) : size_(size) {}
};

template <typename WordType, std::size_t Extent, std::size_t Alignment, typename Policy>
class bit_span : private bit_span_storage<WordType, Extent, Alignment> {
 public:
  using word_type = WordType;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = ::bit::reference<WordType&>;
  using const_reference = const ::bit::reference<WordType&>;
  using pointer = bit_pointer<WordType, 1, Alignment>;
  using const_pointer = const pointer;
  using iterator = bit_iterator<WordType*>;
  using const_iterator = bit_iterator<const WordType*>;

 private:
  pointer data_{};

  template <std::size_t NewOffset, std::size_t NewCount>
  static constexpr size_type subspan_extent() {
    if constexpr (NewCount != std::dynamic_extent) {
      return NewCount;
    } else if constexpr (Extent != std::dynamic_extent) {
      return Extent - NewOffset;
    } else {
      return std::dynamic_extent;
    }
  }

 public:
  constexpr bit_span() noexcept;
  constexpr bit_span(const bit_span& other) noexcept;
  constexpr bit_span(pointer data, size_type bit_count) noexcept
    requires(Extent == std::dynamic_extent);
  constexpr bit_span(pointer data) noexcept;
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

  constexpr size_type size() const noexcept;
  constexpr bool empty() const noexcept;
  constexpr reference operator[](size_type pos);
  constexpr const_reference operator[](size_type pos) const;
  constexpr reference at(size_type idx);
  constexpr const_reference at(size_type idx) const;
  constexpr reference front();
  constexpr reference back();
  constexpr const_reference front() const;
  constexpr const_reference back() const;
  constexpr iterator begin() const noexcept;
  constexpr iterator end() const noexcept;

  template <std::size_t NewOffset,
            std::size_t NewExtent = std::dynamic_extent>
  constexpr auto subspan() const noexcept
      -> bit_span<WordType, subspan_extent<NewOffset, NewExtent>(), Policy>;
  constexpr bit_span<WordType, std::dynamic_extent, Policy> subspan(
      size_type offset, size_type count = std::dynamic_extent) const noexcept
    requires(Extent == std::dynamic_extent);
  constexpr auto operator()(size_type begin, size_type end) const noexcept;
  template <std::size_t NewExtent>
  constexpr bit_span<WordType, NewExtent, Policy> first() const noexcept
    requires(NewExtent != std::dynamic_extent);
  constexpr bit_span<WordType, std::dynamic_extent, Policy> first(
      size_type offset) const noexcept
    requires(Extent == std::dynamic_extent);
  template <std::size_t RevOffset>
  constexpr bit_span<WordType, Extent - RevOffset, Alignment, Policy> last() const noexcept
    requires(RevOffset != std::dynamic_extent);
  constexpr bit_span<WordType, std::dynamic_extent, Policy> last(
      size_type offset) const noexcept
    requires(Extent == std::dynamic_extent);
};

}  // namespace bit

#endif  // _BIT_SPAN_DECLARATION_HPP_INCLUDED
