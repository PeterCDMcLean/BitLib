#ifndef _BIT_WORD_ACCESSOR_DEFINITION_HPP_INCLUDED
#define _BIT_WORD_ACCESSOR_DEFINITION_HPP_INCLUDED

#include "bitlib/bit-containers/bit_array_ref.hpp"
#include "bitlib/bit-containers/bit_word_accessor_declaration.hpp"

namespace bit {

template <size_t N, typename WordType>
constexpr typename bit_word_accessor<N, WordType>::reference
bit_word_accessor<N, WordType>::access(data_handle_type p, std::size_t i) const noexcept
  requires(N == std::dynamic_extent)
{
  return reference(p + i * this->size(), this->size());
}

template <size_t N, typename WordType>
constexpr typename bit_word_accessor<N, WordType>::reference
bit_word_accessor<N, WordType>::access(data_handle_type p, std::size_t i) const noexcept
  requires(N != std::dynamic_extent)
{
  return reference(p + i * this->size());
}

template <size_t N, typename WordType>
constexpr typename bit_word_accessor<N, WordType>::data_handle_type
bit_word_accessor<N, WordType>::offset(data_handle_type p, std::size_t i) const noexcept {
  return p + i * this->size();
}

template <size_t N, typename WordType>
constexpr bit_word_accessor<N, WordType>::bit_word_accessor()
  requires(N != std::dynamic_extent)
    : detail::container_size_storage<size_t, false, N>() {
}

template <size_t N, typename WordType>
constexpr bit_word_accessor<N, WordType>::bit_word_accessor(const size_t& size)
  requires(N == std::dynamic_extent)
    : detail::container_size_storage<size_t, false, N>(size) {
}

}  // namespace bit

#endif  // _BIT_WORD_ACCESSOR_DEFINITION_HPP_INCLUDED
