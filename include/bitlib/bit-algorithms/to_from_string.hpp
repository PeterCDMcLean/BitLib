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
#include <iomanip>
#include <sstream>
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
  for (unsigned char i = 0; i < static_cast<char>(Base); ++i) {
    map[i] = static_cast<char>((i < 10) ? ('0' + i) : ('A' + (i - 10)));
  }
  return map;
}

constexpr std::span<const char> make_digit_map(std::size_t Base) {
  switch (Base) {
    case 2: {
      static constexpr auto map = make_digit_map<2>();
      return std::span<const char>(static_cast<const char*>(map.data()), map.size());
    }
    case 4: {
      static constexpr auto map = make_digit_map<4>();
      return std::span<const char>(static_cast<const char*>(map.data()), map.size());
    }
    case 8: {
      static constexpr auto map = make_digit_map<8>();
      return std::span<const char>(static_cast<const char*>(map.data()), map.size());
    }
    case 16: {
      static constexpr auto map = make_digit_map<16>();
      return std::span<const char>(static_cast<const char*>(map.data()), map.size());
    }
    case 32: {
      static constexpr auto map = make_digit_map<32>();
      return std::span<const char>(static_cast<const char*>(map.data()), map.size());
    }
    case 64: {
      static constexpr auto map = make_digit_map<64>();
      return std::span<const char>(static_cast<const char*>(map.data()), map.size());
    }
    default:
      return {};  // or throw, or abort
  }
}

template <std::size_t Base>
constexpr auto make_from_digit_map() {
  static_assert((Base >= 2) && ((Base & (Base - 1)) == 0), "Base must be power of 2 >= 2");
  static_assert(Base <= 64, "Base too large for simple char mapping");

  ::std::array<char, 128> map{};
  map.fill(~0);
  for (unsigned char i = '0'; i <= 'z'; ++i) {
    if (i >= '0' && i <= '9') {
      map[i] = static_cast<char>(i - '0');
    }
    if (i >= 'a' && i <= 'z') {
      map[i] = static_cast<char>((i - 'a') + 10);
    }
    if (i >= 'A' && i <= 'Z') {
      map[i] = static_cast<char>((i - 'A') + 10);
    }
  }
  return map;
}

constexpr auto make_from_digit_map(std::size_t Base) {
  switch (Base) {
    case 2: {
      static constexpr auto map2 = make_from_digit_map<2>();
      return map2;
    }
    case 4: {
      static constexpr auto map4 = make_from_digit_map<4>();
      return map4;
    }
    case 8: {
      static constexpr auto map8 = make_from_digit_map<8>();
      return map8;
    }
    case 16: {
      static constexpr auto map16 = make_from_digit_map<16>();
      return map16;
    }
    case 32: {
      static constexpr auto map32 = make_from_digit_map<32>();
      return map32;
    }
    case 64: {
      static constexpr auto map64 = make_from_digit_map<64>();
      return map64;
    }
    default:
      throw std::runtime_error("Base not implemented");
  }
}

struct metadata_t {
  size_t base;
  bool is_signed;
  std::endian endian;
  bool str_sign_extend_zeros;
  char fill;
};

constexpr metadata_t typical(size_t base = 10, bool str_sign_extend_zeros = false) {
  return {
      .base = base,
      .is_signed = false,
      .endian = std::endian::big,
      .str_sign_extend_zeros = str_sign_extend_zeros,
      .fill = '\0'};
}

}  // namespace string

template <typename RandomAccessIt, typename CharIt>
constexpr CharIt to_string(
    const bit_iterator<RandomAccessIt>& bit_first,
    const bit_iterator<RandomAccessIt>& bit_last,
    const CharIt str_first,
    const CharIt str_last,
    string::metadata_t meta = string::typical()) {
  if (std::has_single_bit(meta.base)) {
    const auto base_bits = std::bit_width(meta.base - 1);
    const auto base_digits = string::make_digit_map(meta.base);

    CharIt start = accumulate_while(
        policy::AccumulateNoInitialSubword{},
        bit_first, bit_last, str_last,
        [meta, base_bits, base_digits, str_first](CharIt cursor, auto word, const size_t bits = bitsof<decltype(word)>()) {
          for (size_t i = 0; i < ((bits + base_bits - 1) / base_bits); i++) {
            if (cursor == str_first) {
              return std::make_pair(false, cursor);
            }
            *(--cursor) = base_digits[word & (meta.base - 1)];
            word >>= static_cast<decltype(word)>(base_bits);
          }
          return std::make_pair(cursor != str_first, cursor);
        });
    if (start != str_first) {
      return std::copy(start, str_last, str_first);
    } else {
      return str_last;
    }
  } else {
    using word_type = typename bit_iterator<RandomAccessIt>::word_type;
    size_t store_bits = distance(bit_first, bit_last);
    std::vector<word_type> vec((store_bits + bitsof<word_type>() - 1) / bitsof<word_type>());
    vec.back() = 0;  // Ensure last word is zeroed
    bit_iterator<word_type*> bit_it(vec.data());

    const unsigned char base = static_cast<unsigned char>(meta.base);
    auto remainder = ::bit::division(bit_first, bit_last, bit_it, base);
    CharIt cursor = str_last;
    *(--cursor) = static_cast<char>(remainder + '0');

    while ((cursor != str_first) && (store_bits -= ::bit::count_msb(bit_it, bit_it + store_bits, bit0))) {
      remainder = ::bit::division(bit_it, bit_it + store_bits, bit_it, base);
      *(--cursor) = static_cast<char>(remainder + '0');
    }
    if (cursor != str_first) {
      return std::copy(cursor, str_last, str_first);
    }
    return str_last;
  }
}

template <typename RandomAccessIt>
constexpr size_t estimate_length(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const size_t base,
    const bool str_sign_extend_zeros) {
  if (std::has_single_bit(base)) {
    const auto base_bits = std::bit_width(base - 1);

    size_t skip_leading_bits = str_sign_extend_zeros ? 0 : count_msb(first, last, bit0);

    size_t str_len = (distance(first, last) - skip_leading_bits);
    str_len = (str_len + base_bits - 1) / base_bits;  // Round up to nearest base digit
    return static_cast<size_t>(std::max(static_cast<size_t>(1), str_len));
  } else {
    const uint32_t LOG2BASE = static_cast<uint32_t>(std::ceil(static_cast<float>(1 << 16) / std::logbf(static_cast<float>(base))));
    size_t skip_leading_bits = str_sign_extend_zeros ? 0 : count_msb(first, last, bit0);
    const auto bits = distance(first, last) - skip_leading_bits;
    const auto fixed_point = (bits * LOG2BASE);
    const size_t max_len = (fixed_point >> 16) + ((fixed_point & ((1 << 16) - 1)) != 0);
    return static_cast<size_t>(std::max(static_cast<size_t>(1), max_len));
  }
}

template <typename RandomAccessIt>
constexpr std::string to_string(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    string::metadata_t meta = string::typical()) {
  std::string buffer(estimate_length(first, last, meta.base, meta.str_sign_extend_zeros), meta.fill);
  if (meta.fill) {
    std::fill(to_string(first, last, buffer.begin(), buffer.end(), meta), buffer.end(), meta.fill);
  } else {
    buffer.resize(to_string(first, last, buffer.begin(), buffer.end(), meta) - buffer.begin());
  }
  return buffer;
}

template <string::metadata_t meta = string::typical(), typename RandomAccessIt>
constexpr std::string to_string(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last) {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  return to_string(first, last, meta);
}

constexpr std::string to_string(const bit_sized_range auto& bits, string::metadata_t meta = string::typical()) {
  return to_string(bits.begin(), bits.end(), meta);
}

template <string::metadata_t meta = string::typical()>
constexpr std::string to_string(const bit_sized_range auto& bits) {
  return to_string(bits, meta);
}

template <string::metadata_t meta = string::typical(), typename RandomAccessIt, typename CharIt>
constexpr CharIt to_string(
    const bit_iterator<RandomAccessIt>& first,
    const bit_iterator<RandomAccessIt>& last,
    const CharIt str_first,
    const CharIt str_last) {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  return to_string(first, last, str_first, str_last, meta);
}

template <typename CharIt>
constexpr CharIt to_string(
    const bit_sized_range auto& bits,
    const CharIt str_first,
    const CharIt str_last,
    string::metadata_t meta = string::typical()) {
  return to_string(str_first, str_last, bits.begin(), bits.end(), meta);
}

template <string::metadata_t meta = string::typical(), typename CharIt>
constexpr CharIt to_string(
    const bit_sized_range auto& bits,
    const CharIt str_first,
    const CharIt str_last) {
  return to_string(bits, str_first, str_last, meta);
}

template <typename CharIt, typename RandomAccessIt, typename Policy = policy::typical<typename RandomAccessIt::value_type>>
constexpr void from_string(
    const CharIt str_first, const CharIt str_last,
    const bit_iterator<RandomAccessIt>& bit_first, const bit_iterator<RandomAccessIt>& bit_last,
    string::metadata_t meta = string::typical()) {
  // TODO: This should be a policy
  if (str_first == str_last) {
    return;  // Nothing to do
  }
  if (std::has_single_bit(meta.base)) {
    const auto base_bits = std::bit_width(meta.base - 1);
    const auto base_from_digits = string::make_from_digit_map(meta.base);
    using word_type = uint64_t;
    std::vector<word_type> vec;
    size_t store_bits = distance(bit_first, bit_last);

    bit_iterator<RandomAccessIt> bit_it = bit_first;
    auto cursor = std::distance(str_first, str_last) - 1;
    while ((cursor >= 0) && store_bits) {
      word_type work = 0;
      size_t bits = 0;
      for (; (bits < bitsof<word_type>()) && (cursor >= 0); cursor--) {
        char c = str_first[cursor];
        // TODO: This should be a policy
        if (static_cast<size_t>(c) >= base_from_digits.size()) {
          continue;
        }
        auto digit = base_from_digits[c];
        // TODO: This should be a policy
        if (~0 == digit) {
          continue;
        }
        work |= (static_cast<word_type>(digit) << bits);
        bits += base_bits;
      }
      if (store_bits < bits) {
        Policy::truncation::template from_integral<word_type, std::dynamic_extent, RandomAccessIt>(
            work, bit_it, bit_last);
        return;
      } else if ((store_bits > bits) && (cursor < 0)) {
        const bit_iterator<word_type*> p_integral(&work);
        bit_it = ::bit::copy(p_integral, p_integral + bits, bit_it);
        // TODO: policy
        ::bit::fill(bit_it, bit_last, meta.is_signed ? bit_it[-1] : bit0);  // Clear the rest
        return;
      } else if (store_bits >= bits) {
        const bit_iterator<word_type*> p_integral(&work);
        bit_it = ::bit::copy(p_integral, p_integral + bits, bit_it);
        store_bits -= bits;
      }
    }
  } else {
    if (meta.base != 10) {
      throw std::runtime_error("Base not implemented");
    }
    using word_type = typename bit_iterator<RandomAccessIt>::word_type;
    std::vector<word_type> vec;

    // TODO: template with uninitialized_t
    ::bit::fill(bit_first, bit_last, bit0);  // Clear the bits first

    CharIt cursor = str_first;
    while (cursor != str_last) {
      unsigned char c = static_cast<unsigned char>(*cursor - '0');
      if (c <= 9) {
        auto overflow_mult = ::bit::multiplication(bit_first, bit_last, word_type{10});
        auto overflow_add = ::bit::addition(bit_first, bit_last, c);
        if (overflow_mult || overflow_add) {
          //Policy::truncation::template overflow(bit_first, bit_last);
          return;
        }
      }
      cursor++;
    }
    //Policy::extension::template extend(bit_first, bit_last);
  }
}

template <string::metadata_t meta = string::typical(),
          typename CharIt,
          typename RandomAccessIt,
          typename Policy = policy::typical<typename RandomAccessIt::value_type>>
constexpr void from_string(
    const CharIt str_first, const CharIt str_last,
    const bit_iterator<RandomAccessIt>& bit_first, const bit_iterator<RandomAccessIt>& bit_last) {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  from_string<CharIt, RandomAccessIt, Policy>(str_first, str_last, bit_first, bit_last, meta);
}

template <typename CharIt>
constexpr std::vector<uintptr_t> from_string(
    const CharIt first, const CharIt last, string::metadata_t meta = string::typical()) {
  if (std::has_single_bit(meta.base)) {
    const auto base_bits = std::bit_width(meta.base - 1);
    const auto base_from_digits = string::make_from_digit_map(meta.base);

    std::vector<uintptr_t> vec;

    last--;
    while (last >= first) {
      uintptr_t work = 0;
      size_t bits = 0;
      for (; (bits < bitsof<uintptr_t>()) && (last >= first); last--) {
        char c = *last;
        // TODO: This should be a policy
        if (static_cast<size_t>(c) >= base_from_digits.size()) {
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

template <string::metadata_t meta = string::typical(),
          typename CharIt>
constexpr std::vector<uintptr_t> from_string(
    const CharIt first, const CharIt last) {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  return from_string(first, last, meta);
}

template <string::metadata_t meta = string::typical(), typename RandomAccessIt, typename Policy = policy::typical<typename RandomAccessIt::value_type>>
constexpr void from_string(
    const std::string& str,
    const bit_iterator<RandomAccessIt>& bit_first, const bit_iterator<RandomAccessIt>& bit_last) {
  static_assert(meta.endian == std::endian::big, "Only bit big endian support (MSB on the left)");
  from_string<meta, RandomAccessIt, Policy>(str.c_str(), str.c_str() + str.length(), bit_first, bit_last);
}

template <string::metadata_t meta = string::typical(), bit_range RangeT, typename Policy = policy::typical<typename std::ranges::iterator_t<RangeT>::value_type>>
constexpr void from_string(
    const std::string& str,
    RangeT&& bits) {
  using range_iterator_t = std::ranges::iterator_t<RangeT>;
  using RandomAccessIt = typename range_iterator_t::iterator_type;
  from_string<meta, std::string::const_iterator, RandomAccessIt, Policy>(str.begin(), str.end(), bits.begin(), bits.end());
}

template <typename RandomAccessIt, typename Policy = policy::typical<typename RandomAccessIt::value_type>>
constexpr void from_string(
    const std::string& str,
    const bit_iterator<RandomAccessIt>& bit_first, const bit_iterator<RandomAccessIt>& bit_last,
    string::metadata_t meta = string::typical()) {
  from_string<std::string::const_iterator, RandomAccessIt, Policy>(
      str.begin(), str.end(),
      bit_first, bit_last,
      meta);
}

template <bit_range RangeT, typename Policy = policy::typical<typename std::ranges::iterator_t<RangeT>::value_type>>
constexpr void from_string(
    const std::string& str,
    RangeT&& bits,
    string::metadata_t meta = string::typical()) {
  using range_iterator_t = std::ranges::iterator_t<RangeT>;
  using RandomAccessIt = typename range_iterator_t::iterator_type;
  from_string<std::string::const_iterator, RandomAccessIt, Policy>(
      str.begin(), str.end(),
      bits.begin(), bits.end(),
      meta);
}

}  // namespace bit

#endif // _BIT_TO_STRING_HPP_INCLUDED