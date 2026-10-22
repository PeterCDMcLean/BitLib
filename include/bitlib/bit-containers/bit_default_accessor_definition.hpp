#ifndef _BIT_DEFAULT_ACCESSOR_DEFINITION_HPP_INCLUDED
#define _BIT_DEFAULT_ACCESSOR_DEFINITION_HPP_INCLUDED

#include "bitlib/bit-containers/bit_default_accessor_declaration.hpp"

namespace bit {

template <typename WordType>
constexpr typename bit_default_accessor<WordType>::reference
bit_default_accessor<WordType>::access(data_handle_type p, std::size_t i) const noexcept {
  return p[i];
}

template <typename WordType>
constexpr typename bit_default_accessor<WordType>::data_handle_type
bit_default_accessor<WordType>::offset(data_handle_type p, std::size_t i) const noexcept {
  return p + i;
}

}  // namespace bit

#endif  // _BIT_DEFAULT_ACCESSOR_DEFINITION_HPP_INCLUDED
