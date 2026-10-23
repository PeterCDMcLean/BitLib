// ============================ WORD ADAPTER ================================ //
// Project: The Experimental Bit Algorithms Library
// Description: Shared word-size and Alignment adaptation helpers
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_WORD_ADAPTER_HPP_INCLUDED
#define _BIT_WORD_ADAPTER_HPP_INCLUDED
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <type_traits>
// Project sources
#include "bitlib/bit-iterator/bit.hpp"
// Third-party libraries
// Miscellaneous

namespace bit {
namespace detail {
// ========================================================================== //

// Construct an iterator at a possibly compile-time alignment.
template <std::size_t Alignment, typename Iterator>
constexpr auto make_aligned_iterator(const Iterator& base,
                                     std::size_t position) {
  if constexpr (Alignment == std::dynamic_extent) {
    return iterator<Iterator, primitive<1>, std::dynamic_extent>(base,
                                                                 position);
  } else {
    assert(position == Alignment);
    return ::bit::next<Alignment>(iterator<Iterator, primitive<1>, 0>(base));
  }
}

// Re-express one input iterator in a smaller target word type. The alignment
// is unpacked from the iterator itself, and its position is translated from
// source-word bits into target-word bits.
template <std::size_t TargetBits, typename TargetWord, typename Iterator>
constexpr auto adapt_word_iterator(const Iterator& input) {
  constexpr std::size_t Alignment = iterator_alignment_v<Iterator>;
  auto it = make_mutable_bit_iterator(input);
  using mutable_iterator = decltype(it);
  constexpr std::size_t source_bits =
      bitsof<std::remove_cv_t<typename mutable_iterator::word_type>>();
  constexpr std::size_t adapted_alignment =
      Alignment == std::dynamic_extent ? std::dynamic_extent
                                       : Alignment % TargetBits;

  if constexpr (source_bits == TargetBits) {
    return make_aligned_iterator<Alignment>(it.base(), it.position());
  } else {
    using source_iterator = typename mutable_iterator::iterator_type;
    using source_reference = std::iter_reference_t<source_iterator>;
    constexpr bool source_is_const =
        std::is_const_v<std::remove_reference_t<source_reference>>;
    using target_pointer =
        std::conditional_t<source_is_const, const TargetWord*, TargetWord*>;
    using adapted_iterator =
        bit_word_pointer_adapter<target_pointer, source_iterator>;
    const auto adapted =
        adapted_iterator(it.base(), it.position() / TargetBits);
    return make_aligned_iterator<adapted_alignment>(
        adapted, it.position() % TargetBits);
  }
}

// The smallest word width used by any of the supplied mutable iterators.
template <typename... Iterators>
consteval std::size_t smallest_word_bits() {
  std::size_t result = std::dynamic_extent;
  ((result = std::min(result,
                      bitsof<std::remove_cv_t<typename Iterators::word_type>>())),
   ...);
  return result;
}

// ========================================================================== //
}  // namespace detail
}  // namespace bit

#endif  // _BIT_WORD_ADAPTER_HPP_INCLUDED
// ========================================================================== //
