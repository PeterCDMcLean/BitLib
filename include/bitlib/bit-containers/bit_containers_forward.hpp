// =================== BIT CONTAINER FORWARD DECLARATIONS =================== //
// Project: The Experimental Bit Algorithms Library
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_CONTAINERS_FORWARD_HPP_INCLUDED
#define _BIT_CONTAINERS_FORWARD_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <type_traits>

#include "bitlib/bit-iterator/bit_integral.hpp"
#include "bitlib/bit-iterator/bit_iterator_forward.hpp"

namespace bit {

namespace policy {
struct truncate;
struct sign_extend;

template <typename W>
struct typical;
}  // namespace policy

// Non-owning view over a contiguous bit range.
template <typename WordType = std::uintptr_t,
          std::size_t Extent = std::dynamic_extent,
          std::size_t Alignment = std::dynamic_extent,
          typename Policy = policy::typical<std::remove_cvref_t<WordType>>>
class bit_span;

template <typename WordType, std::size_t Extent>
struct bit_span_storage;

// Owning bit array. The primary template owns its storage; the
// std::dynamic_extent specialization is defined in bit_array_dynamic_extent.hpp.
template <typename T = bit_value,
          std::size_t N = std::dynamic_extent,
          typename W = std::conditional_t<
              std::is_same_v<T, bit_value>,
              std::conditional_t<(N == std::dynamic_extent),
                                 std::uintptr_t,
                                 ceil_integral<N>>,
              T>,
          typename Policy = policy::typical<std::remove_cvref_t<W>>>
class array;

template <std::size_t N = std::dynamic_extent,
          typename W = std::conditional_t<(N == std::dynamic_extent),
                                          std::uintptr_t,
                                          ceil_integral<N>>,
          typename Policy = policy::typical<std::remove_cvref_t<W>>>
using bit_array = array<bit_value, N, W, Policy>;

// Non-owning reference to an existing bit array.
template <typename T = bit_value,
          std::size_t N = std::dynamic_extent,
          typename W = std::uintptr_t,
          std::size_t A = std::dynamic_extent,
          typename Policy = policy::typical<std::remove_cvref_t<W>>>
class array_ref;

template <std::size_t N = std::dynamic_extent,
          typename W = std::uintptr_t,
          std::size_t A = std::dynamic_extent,
          typename Policy = policy::typical<std::remove_cvref_t<W>>>
using bit_array_ref = array_ref<bit_value, N, W, A, Policy>;

// CRTP base class shared by the array containers.
template <typename Derived,
          typename T,
          std::size_t N,
          typename W,
          std::size_t A,
          bool resizable,
          typename Policy,
          typename Iterators>
class array_base;

// Dynamically sized owning bit container.
template <class WordType = std::uint8_t,
          class Allocator = std::allocator<WordType>>
class bit_vector;

// mdspan accessors for bit ranges.
template <typename WordType = std::uintptr_t>
struct bit_default_accessor;

template <std::size_t N = std::dynamic_extent,
          typename WordType = std::uintptr_t>
struct bit_word_accessor;

}  // namespace bit

#endif  // _BIT_CONTAINERS_FORWARD_HPP_INCLUDED
