// ================================= BIT_ARRAY =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_literal.hpp
// Description: Implementation of bit_array user defined literal
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_LITERAL_HPP_INCLUDED
#define _BIT_LITERAL_HPP_INCLUDED

// ================================ PREAMBLE ================================ //
// C++ standard library
#include <bit>

// Project sources
#include "bitlib/bit-containers/bit_array.hpp"

namespace bit {

// ========================================================================== //
template <char Bit>
constexpr void _parameter_pack_base_bits_prefix_len(size_t& base, size_t& bits, size_t& prefix_len, uint64_t& num) {
  if ('\'' == Bit) {
    if (0 != bits) {
      return;  //skip. Should we, though? It seems like it will be confusing
    }
    bits = num;
    num = 0;
    ++prefix_len;
    if (0 == base) {
      base = 10;
    }
  } else if (0 == base) {
    if (Bit == 'b' || Bit == 'B') {
      base = 2;
      num = 0;
    } else if (Bit == 'x' || Bit == 'X') {
      base = 16;
      num = 0;
    } else if (Bit == 'd' || Bit == 'D') {
      base = 10;
      num = 0;
    } else {
      num = (num * 10) + (Bit - '0');
    }
    ++prefix_len;
  } else {
    uint8_t decimal;
    switch (base) {
      case 2:
        decimal = static_cast<uint8_t>(Bit - '0');
        break;
      case 10:
        decimal = static_cast<uint8_t>(Bit - '0');
        break;
      case 16:
        decimal = static_cast<uint8_t>(Bit - '0');
        if (Bit >= 'a' && Bit <= 'f') {
          decimal = static_cast<uint8_t>(Bit - 'a') + 10u;
        }
        if (Bit >= 'A' && Bit <= 'F') {
          decimal = static_cast<uint8_t>(Bit - 'A') + 10u;
        }
        break;
    }
    num = num * base + decimal;
  }
}

template <char Bit, char... Bits>
constexpr void _parameter_pack_base_bits_prefix_len(size_t& base, size_t& bits, size_t& prefix_len, uint64_t& num)
  requires(sizeof...(Bits) > 0)
{
  _parameter_pack_base_bits_prefix_len<Bit>(base, bits, prefix_len, num);
  _parameter_pack_base_bits_prefix_len<Bits...>(base, bits, prefix_len, num);
}

template <char... Bits>
constexpr std::pair<size_t, uint64_t> _parameter_pack_decode_prefixed_num() {
  size_t base = 0;
  size_t bits = 0;
  size_t prefix_len = 0;
  uint64_t num = 0;
  _parameter_pack_base_bits_prefix_len<Bits...>(base, bits, prefix_len, num);
  size_t digits = (sizeof...(Bits) - prefix_len);
  if (0 == base) {
    base = 10;
    digits = prefix_len;
    prefix_len = 0;
  }
  if (0 == bits) {
    if (base == 2) {
      bits = digits;
    } else if (base == 10) {
      bits = std::bit_width(num);
    } else {
      bits = 4 * digits;
    }
  }
  return {bits, num};
}

} // namespace bit

template <char... Str>
constexpr bit::bit_array<bit::bit_value, bit::_parameter_pack_decode_prefixed_num<Str...>().first> operator""_b() {
  bit::bit_array<bit::bit_value, bit::_parameter_pack_decode_prefixed_num<Str...>().first> arr{};
  auto [bits, num] = bit::_parameter_pack_decode_prefixed_num<Str...>();
  for (int i = 0; i < arr.size(); i++) {
    arr[i] = (num & 0x1) ? bit::bit1 : bit::bit0;
    num >>= 1;
  }
  return arr;
}

#endif
