// =============================== BIT VALUE ================================ //
// Project:         The C++ Bit Library
// Name:            bit_value.hpp
// Description:     A class representing an independent, non-referenced bit
// Creator:         Vincent Reverdy
// Contributor(s):  Peter McLean [2025]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_MDSPAN_ACCESSOR_HPP_INCLUDED
#define _BIT_MDSPAN_ACCESSOR_HPP_INCLUDED

#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_reference.hpp"
#include "bitlib/bit-iterator/bit_value.hpp"

namespace bit {
template <typename WordType = uintptr_t>
struct bit_default_accessor {
  using element_type = bit_value;
  using data_handle_type = bit_pointer<WordType>;
  using reference = bit_reference<WordType&>;
  using offset_policy = bit_default_accessor<WordType>;
  constexpr reference access(data_handle_type p, std::size_t i) const noexcept {
    return p[i];
  }
  constexpr data_handle_type offset(data_handle_type p, std::size_t i) const noexcept {
    return p + i;
  }
};

template <size_t N = std::dynamic_extent, typename WordType = uintptr_t>
struct bit_word_accessor : private detail::container_size_storage<size_t, false, N> {
  using element_type = bit_array<N, WordType>;
  using data_handle_type = bit_pointer<WordType>;
  using reference = bit_array_ref<N, WordType>;
  using offset_policy = bit_word_accessor<N, WordType>;
  constexpr reference access(data_handle_type p, std::size_t i) const noexcept
    requires(N == std::dynamic_extent)
  {
    return reference(p + i * this->size(), this->size());
  }
  constexpr reference access(data_handle_type p, std::size_t i) const noexcept
    requires(N != std::dynamic_extent)
  {
    return reference(p + i * this->size());
  }
  constexpr data_handle_type offset(data_handle_type p, std::size_t i) const noexcept {
    return p + i * this->size();
  }
  constexpr bit_word_accessor()
    requires(N != std::dynamic_extent)
      : detail::container_size_storage<size_t, false, N>() {}
  constexpr bit_word_accessor(const size_t& size)
    requires(N == std::dynamic_extent)
      : detail::container_size_storage<size_t, false, N>(size) {}
};

}  // namespace bit

#endif
