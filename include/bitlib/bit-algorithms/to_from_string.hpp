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

#include <array>
#include <bit>
#include <string>

#include "bitlib/bit-algorithms/accumulate.hpp"
#include "bitlib/bit-algorithms/count.hpp"
#include "bitlib/bit-algorithms/division.hpp"
#include "bitlib/bit-algorithms/multiplication.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

namespace string {

template <std::size_t Base>
constexpr auto make_digit_map() {
  static_assert((Base >= 2) && ((Base & (Base - 1)) == 0), "Base must be power of 2 >= 2");
  static_assert(Base <= 64, "Base too large for simple char mapping");

  ::std::array<char, Base> map{};
  for (std::size_t i = 0; i < Base; ++i) {
    map[i] = (i < 10) ? ('0' + i) : ('A' + (i - 10));
  }
  return map;
}

template <std::size_t Base>
constexpr auto make_from_digit_map() {
  static_assert((Base >= 2) && ((Base & (Base - 1)) == 0), "Base must be power of 2 >= 2");
  static_assert(Base <= 64, "Base too large for simple char mapping");

  ::std::array<char, 128> map{};
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
    str_len = (str_len + base_bits - 1) / base_bits;  // Round up to nearest base digit
    if (0 == str_len) {
      return prefix + "0";
    }
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
#if 0
template <string::metadata_t meta = string::typical(), typename Policy = policy::typical<uintptr_t>, typename RandomAccessIt>
constexpr size_t pessimistic_bits_for_string(
    const char* str_first, const char* str_last) {
  const char* it = str_first;
  for (; (it != str_last) && (*it == '0'); ++it) {
  }
  const size_t non_zero_str_len = str_last - it;
  if (non_zero_str_len == 0) {
    return meta.is_signed ? 1 : 0;  // All zeros
  }
  if constexpr (std::has_single_bit(meta.base)) {
    constexpr const auto base_bits = std::bit_width(meta.base - 1);
    static constexpr auto base_from_digits = string::make_from_digit_map<meta.base>();
    return non_zero_str_len * base_bits + meta.is_signed;
  } else {
    constexpr double base_in_base2 = std::log2(meta.base);

    return static_cast<size_t>(std::floor(base_in_base2 * non_zero_str_len)) + 1 + meta.is_signed;
  }
}

template <string::metadata_t meta = string::typical(), typename Policy = policy::typical<uintptr_t>, typename RandomAccessIt>
constexpr void from_string(
    Policy,
    const char* str_first, const char* str_last,
    bit_iterator<RandomAccessIt> bit_first, bit_iterator<RandomAccessIt> bit_last) {
  const auto str_len = str_last - str_first;
  const auto store_bits = distance(bit_first, bit_last);
  if constexpr (std::has_single_bit(meta.base)) {
    constexpr const auto base_bits = std::bit_width(meta.base - 1);
    static constexpr auto base_from_digits = string::make_from_digit_map<meta.base>();
    constexpr const auto str_bits = str_len * base_bits;
    if (store_bits < str_bits) {
      Policy::truncation::template from_string<std::dynamic_extent>(
          str_first, str_last, str_cur, bit_first, bit_last);
    } else if (store_bits > str_bits) {
      Policy::extension::template from_string<std::dynamic_extent>(
          str_first, str_last, str_cur, bit_first, bit_last);
    } else {
      char str_cur = str_last - 1;
      bit_iterator<RandomAccessIt> bit_cur = bit_first;
      while (str_cur >= str_first) {
        str_cur--;
      }
    }
  }
}
#endif

template <string::metadata_t meta = string::typical(), typename RandomAccessIt, typename Policy = policy::typical<typename RandomAccessIt::value_type>>
constexpr void from_string(
    const char* str_first, const char* str_last,
    const bit_iterator<RandomAccessIt>& bit_first, const bit_iterator<RandomAccessIt>& bit_last) {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  if constexpr (std::has_single_bit(meta.base)) {
    constexpr const auto base_bits = std::bit_width(meta.base - 1);
    static constexpr auto base_from_digits = string::make_from_digit_map<meta.base>();
    using word_type = uint64_t;
    std::vector<word_type> vec;
    size_t store_bits = distance(bit_first, bit_last);

    bit_iterator<RandomAccessIt> bit_it = bit_first;
    str_last--;
    while (str_last >= str_first && store_bits) {
      word_type work = 0;
      size_t bits = 0;
      for (; (bits < bitsof<word_type>()) && (str_last >= str_first); str_last--) {
        char c = *str_last;
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
      if (store_bits < bits) {
        Policy::truncation::template from_integral<word_type, std::dynamic_extent, RandomAccessIt>(
            bit_it, bit_last, work);
        return;
      } else if ((store_bits > bits) && (str_last < str_first)) {
        const bit_iterator<word_type*> p_integral(&work);
        bit_it = ::bit::copy(p_integral, p_integral + bits, bit_it);
        Policy::extension::template from_integral<word_type, std::dynamic_extent, RandomAccessIt>(
            bit_it, bit_last, work);
      } else if (store_bits >= bits) {
        const bit_iterator<word_type*> p_integral(&work);
        bit_it = ::bit::copy(p_integral, p_integral + bits, bit_it);
      }
    }
  } else {
    if (meta.base != 10) {
      throw std::runtime_error("Base not implemented");
    }
    using word_type = typename bit_iterator<RandomAccessIt>::word_type;
    std::vector<word_type> vec;
    size_t store_bits = distance(bit_first, bit_last);

    // TODO: template with uninitialized_t
    ::bit::fill(bit_first, bit_last, bit0);  // Clear the bits first

    while (str_first != str_last) {
      unsigned char c = (*str_first - '0');
      if (c <= 9) {
        auto overflow_mult = ::bit::multiplication(bit_first, bit_last, word_type{10});
        auto overflow_add = ::bit::addition(bit_first, bit_last, c);
        if (overflow_mult || overflow_add) {
          //Policy::truncation::template overflow(bit_first, bit_last);
          return;
        }
      }
      str_first++;
    }
    //Policy::extension::template extend(bit_first, bit_last);
  }
}

template <string::metadata_t meta = string::typical()>
constexpr std::vector<uintptr_t> from_string(
    const char* first, const char* last) {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  if constexpr (std::has_single_bit(meta.base)) {
    constexpr const auto base_bits = std::bit_width(meta.base - 1);
    static constexpr auto base_from_digits = string::make_from_digit_map<meta.base>();

    std::vector<uintptr_t> vec;

    last--;
    while (last >= first) {
      uintptr_t work = 0;
      size_t bits = 0;
      for (; (bits < bitsof<uintptr_t>()) && (last >= first); last--) {
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
        vec.push_back(work);
      }
    }
    return vec;
  } else {
    //from_string base 10 not implemented yet;
    return {};
  }
}

template <string::metadata_t meta = string::typical(), typename RandomAccessIt, typename Policy = policy::typical<typename RandomAccessIt::value_type>>
constexpr void from_string(
    const std::string& str,
    const bit_iterator<RandomAccessIt>& bit_first, const bit_iterator<RandomAccessIt>& bit_last) {
  from_string<meta, RandomAccessIt, Policy>(str.c_str(), str.c_str() + str.length(), bit_first, bit_last);
}

template <string::metadata_t meta = string::typical(), bit_range RangeT, typename Policy = policy::typical<typename std::ranges::iterator_t<RangeT>::value_type>>
constexpr void from_string(
    const std::string& str,
    RangeT& bits) {
  using range_iterator_t = std::ranges::iterator_t<RangeT>;
  using RandomAccessIt = typename range_iterator_t::iterator_type;
  from_string<meta, RandomAccessIt, Policy>(str.c_str(), str.c_str() + str.length(), bits.begin(), bits.end());
}

}  // namespace bit

#endif // _BIT_TO_STRING_HPP_INCLUDED