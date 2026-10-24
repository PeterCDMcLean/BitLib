#ifndef _BIT_WORD_ACCESSOR_DECLARATION_HPP_INCLUDED
#define _BIT_WORD_ACCESSOR_DECLARATION_HPP_INCLUDED

#include <cstddef>

#include "bitlib/bit-containers/bit_containers_forward.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"

namespace bit {

template <size_t N, typename WordType>
struct bit_word_accessor : private detail::container_size_storage<size_t, false, N> {
  using element_type = bit_array<N, WordType>;
  using data_handle_type = bit_pointer<WordType>;
  using reference = bit_array_ref<N, WordType>;
  using offset_policy = bit_word_accessor<N, WordType>;

  constexpr reference access(data_handle_type p, std::size_t i) const noexcept
    requires(N == std::dynamic_extent);
  constexpr reference access(data_handle_type p, std::size_t i) const noexcept
    requires(N != std::dynamic_extent);
  constexpr data_handle_type offset(data_handle_type p, std::size_t i) const noexcept;

  constexpr bit_word_accessor()
    requires(N != std::dynamic_extent);
  constexpr bit_word_accessor(const size_t& size)
    requires(N == std::dynamic_extent);
};

}  // namespace bit

#endif  // _BIT_WORD_ACCESSOR_DECLARATION_HPP_INCLUDED
