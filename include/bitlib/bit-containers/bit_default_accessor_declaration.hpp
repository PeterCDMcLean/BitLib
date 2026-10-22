#ifndef _BIT_DEFAULT_ACCESSOR_DECLARATION_HPP_INCLUDED
#define _BIT_DEFAULT_ACCESSOR_DECLARATION_HPP_INCLUDED

#include <cstddef>

#include "bitlib/bit-iterator/bit_iterator_forward.hpp"

namespace bit {

template <typename WordType>
struct bit_default_accessor {
  using element_type = bit_value;
  using data_handle_type = bit_pointer<WordType>;
  using reference = reference<WordType&>;
  using offset_policy = bit_default_accessor<WordType>;

  constexpr reference access(data_handle_type p, std::size_t i) const noexcept;
  constexpr data_handle_type offset(data_handle_type p, std::size_t i) const noexcept;
};

}  // namespace bit

#endif  // _BIT_DEFAULT_ACCESSOR_DECLARATION_HPP_INCLUDED
