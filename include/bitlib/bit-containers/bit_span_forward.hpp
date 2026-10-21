#ifndef _BIT_SPAN_FORWARD_HPP_INCLUDED
#define _BIT_SPAN_FORWARD_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

namespace bit {
namespace policy {
template <typename W>
struct typical;
}  // namespace policy

template <typename WordType = std::uintptr_t,
          std::size_t Extent = std::dynamic_extent,
          std::size_t Alignment = std::dynamic_extent,
          typename Policy =
              policy::typical<std::remove_cvref_t<WordType>>>
class bit_span;
}  // namespace bit

#endif  // _BIT_SPAN_FORWARD_HPP_INCLUDED
