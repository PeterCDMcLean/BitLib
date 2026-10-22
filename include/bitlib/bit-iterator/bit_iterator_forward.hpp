// =================== BIT ITERATOR FORWARD DECLARATIONS ==================== //
// Project: The Experimental Bit Algorithms Library
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ITERATOR_FORWARD_HPP_INCLUDED
#define _BIT_ITERATOR_FORWARD_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <span>

#include "bitlib/bit_concepts.hpp"

namespace bit {

// Generic N-bit primitive value type. primitive<1> is defined in
// bit_value.hpp; the primary template (Bits > 1) is defined in bit_array.hpp.
template <std::size_t Bits>
class primitive;

using bit_value = primitive<1>;

// Proxy reference to a bit field. The primary template is the single-word
// implementation; multi-bit fields and multi-word storage use the array-backed
// specialization.
template <typename WordRef = std::uintptr_t&,
          primitive_c ValueType = primitive<1>,
          std::size_t Alignment = std::dynamic_extent>
class reference;

// Random-access iterator over a bit sequence.
template <class Iterator,
          primitive_c ValueType = primitive<1>,
          std::size_t Alignment = std::dynamic_extent>
class iterator;

template <class Iterator, std::size_t Alignment = std::dynamic_extent>
using bit_iterator = iterator<Iterator, primitive<1>, Alignment>;

template <typename WordType,
          primitive_c ValueType,
          std::size_t Alignment = std::dynamic_extent>
using pointer = iterator<WordType*, ValueType, Alignment>;

template <typename WordType, std::size_t Alignment = std::dynamic_extent>
using bit_pointer = iterator<WordType*, primitive<1>, Alignment>;

// Adapters that re-express a bit range in differently sized words.
template <typename target_word_ptr, typename source_word_ptr>
class bit_word_pointer_adapter;

template <typename target_word_ref, typename source_word_ref>
class bit_word_reference_adapter;

}  // namespace bit

#endif  // _BIT_ITERATOR_FORWARD_HPP_INCLUDED
