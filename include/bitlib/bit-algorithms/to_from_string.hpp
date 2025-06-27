// ================================= array_REF =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        to_string.hpp
// Description: Implementation of array_ref
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

namespace string {

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

template <std::size_t Base>
constexpr auto make_from_digit_map() {
  static_assert((Base >= 2) && ((Base & (Base - 1)) == 0), "Base must be power of 2 >= 2");
  static_assert(Base <= 64, "Base too large for simple char mapping");

  std::array<char, 128> map = {};
  for (std::size_t i = 0; i < 128; ++i) {
    map[i] = ~0;
    if (i >= '0' && i <= '9') {
      map[i] = i - '0';
    }
    if (i >= 'a' && i <= 'z') {
      map[i] = (i - 'a') + 10;
    }
    if (i >= 'A' && i <= 'Z') {
      map[i] = (i - 'A') + 10;
    }
  }
  return map;
}

struct metadata_t {
  size_t base;
  bool is_signed;
  std::endian endian;
  bool str_sign_extend_zeros;
};

constexpr metadata_t typical(size_t base = 10, bool str_sign_extend_zeros = false) {
  return {
      .base = base,
      .is_signed = false,
      .endian = std::endian::big,
      .str_sign_extend_zeros = str_sign_extend_zeros};
}

}  // namespace string

template <string::metadata_t meta = string::typical(), typename RandomAccessIt>
constexpr std::string to_string(const bit_iterator<RandomAccessIt>& first, const bit_iterator<RandomAccessIt>& last, std::string prefix = "") {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  if constexpr (std::has_single_bit(meta.base)) {
    constexpr const auto base_bits = std::bit_width(meta.base - 1);

    int skip_leading_bits = meta.str_sign_extend_zeros ? 0 : count_msb(first, last, bit0);

    int str_len = (distance(first, last) - skip_leading_bits);
    str_len += (0 != (str_len % base_bits));
    std::string& str = prefix;
    str.resize(str.length() + str_len);

    static constexpr auto base_digits = string::make_digit_map<meta.base>();

    return accumulate(
        policy::AccumulateNoInitialSubword{},
        first, last - skip_leading_bits, (str.data() + str_len),
        [](char* acc, auto word, const size_t bits = bitsof<decltype(word)>()) {
          const int characters = ((bits + base_bits - 1) / base_bits);
          acc -= characters;
          for (int i = characters - 1; i >= 0; i--) {
            acc[i] = base_digits[word & (meta.base - 1)];
            word >>= base_bits;
          }
          return acc;
        });
  } else {
    return "not_implented_yet";
  }
}

template <string::metadata_t meta = string::typical()>
constexpr std::string to_string(const bit_sized_range auto& bits, std::string prefix = "") {
  return to_string<meta>(bits.begin(), bits.end(), prefix);
}

/*
Commenting this out temporarily as the reference to bit_vector/bit_array messes up include dependency DAG

template <string::metadata_t meta = string::typical(), typename Policy = policy::typical<uintptr_t>, typename RandomAccessIt>
constexpr void from_string(
    Policy,
    const char* str_first, const char* str_last,
    bit_iterator<RandomAccessIt> bit_first, bit_iterator<RandomAccessIt> bit_last) {
  const auto str_len = str_last - str_first;
}

template <string::metadata_t meta = string::typical()>
constexpr bit_vector<> from_string(const char* first, const char* last) {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  if constexpr (std::has_single_bit(meta.base)) {
    constexpr const auto base_bits = std::bit_width(meta.base - 1);
    static constexpr auto base_from_digits = string::make_from_digit_map<meta.base>();

    bit_vector<> vec;

    last--;
    while (last >= first) {
      uint64_t work = 0;
      size_t bits = 0;
      for (; (bits < bitsof<uint64_t>()) && (last >= first); last--) {
        char c = *last;
        // TODO: This should be a policy
        if (c >= base_from_digits.size()) {
          continue;
        }
        auto digit = base_from_digits[c];
        // TODO: This should be a policy
        if (~0 == digit) {
          continue;
        }
        work |= (digit << bits);
        bits += base_bits;
      }
      if (bits) {
        vec.append_range(bit_array<>(bits, work));
      }
    }
    return vec;
  } else {
    //from_string base 10 not implemented yet;
  }
}

template <string::metadata_t meta = string::typical()>
constexpr bit_vector<> from_string(const std::string& str) {
  return from_string<meta>(str.c_str(), str.c_str() + str.length());
}
*/

}  // namespace bit

#endif // _BIT_TO_STRING_HPP_INCLUDED