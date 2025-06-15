// ================================= BIT_ARRAY_REF =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        to_string.hpp
// Description: Implementation of bit_array_ref
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //

#ifndef _BIT_TO_STRING_HPP_INCLUDED
#define _BIT_TO_STRING_HPP_INCLUDED

#include <bit>

#include "bitlib/bit-algorithms/accumulate.hpp"
#include "bitlib/bit-algorithms/count.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

namespace detail {

template <std::size_t Base>
constexpr auto make_digit_map() {
  static_assert((Base >= 2) && ((Base & (Base - 1)) == 0), "Base must be power of 2 >= 2");
  static_assert(Base <= 64, "Base too large for simple char mapping");

  std::array<char, Base> map = {};
  for (std::size_t i = 0; i < Base; ++i) {
    map[i] = (i < 10) ? ('0' + i) : ('A' + (i - 10));
  }
  return map;
}

}  // namespace detail

template <std::size_t base, bool prepend_zeros = false, std::endian endian = std::endian::little>
constexpr std::string to_string(const bit_sized_range auto& bits, std::string prefix = "") {
  if constexpr (std::has_single_bit(base)) {
    constexpr const auto base_bits = std::bit_width(base - 1);

    int skip_leading_bits = prepend_zeros ? 0 : count_msb(bits.begin(), bits.end(), bit0);

    int str_len = (bits.size() - skip_leading_bits);
    str_len += (0 != (str_len % base_bits));
    std::string& str = prefix;
    str.resize(str.length() + str_len);

    static constexpr auto base_digits = detail::make_digit_map<base>();

    return accumulate(
        policy::AccumulateNoInitialSubword{},
        bits.begin(), bits.end() - skip_leading_bits, (str.data() + str_len),
        [](char* acc, auto word, const size_t bits = bitsof<decltype(word)>()) {
          const int characters = ((bits + base_bits - 1) / base_bits);
          acc -= characters;
          for (int i = characters - 1; i >= 0; i--) {
            acc[i] = base_digits[word & (base - 1)];
            word >>= base_bits;
          }
          return acc;
        });
  }
}

constexpr std::string to_string(const bit_sized_range auto& bits, const uint8_t base, const std::endian endian = std::endian::native) {
  return "";
}
}

#endif // _BIT_TO_STRING_HPP_INCLUDED