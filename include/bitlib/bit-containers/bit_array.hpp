// ================================= BIT_ARRAY =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array.hpp
// Description: Implementation of bit_array
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ARRAY_HPP_INCLUDED
#define _BIT_ARRAY_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
#include <algorithm>
#include <cmath>
#include <span>
#include <string>
#include <type_traits>
#include <vector>
// Project sources
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"

namespace bit {
// ========================================================================== //

template <typename T = bit_value,
          std::size_t N = std::dynamic_extent,
          std::align_val_t V = std::align_val_t(alignof(T)),
          typename W = std::conditional<std::is_same_v<T, bit_value>, uint8_t, T>::type>
class bit_array {
 public:
  static constexpr std::size_t bits = N;
  using word_type = W;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = typename std::conditional<std::is_same_v<T, bit_value>, bit_reference<word_type>, T&>::type;
  using const_reference = typename std::conditional<std::is_same_v<T, bit_value>, const bit_reference<const word_type>, const T&>::type;
  using pointer = typename std::conditional<std::is_same_v<T, bit_value>, bit_pointer<word_type>, T&>::type;
  using const_pointer = const pointer;

 private:
  static constexpr std::size_t Words = (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>();

  alignas(V) std::array<word_type, Words> storage;

 public:
  using iterator = typename std::conditional<std::is_same_v<T, bit_value>,
                                             bit_iterator<typename std::array<word_type, Words>::iterator>,
                                             typename std::array<word_type, Words>::iterator>::type;
  using const_iterator = typename std::conditional<std::is_same_v<T, bit_value>,
                                                   bit_iterator<typename std::array<word_type, Words>::const_iterator>,
                                                   typename std::array<const word_type, Words>::const_iterator>::type;
  /*
  * Constructors, copies and moves...
  */
  constexpr bit_array() noexcept;
  constexpr bit_array(value_type bit_val);
  constexpr bit_array(const bit_array<T, N, V, W>& other) = default;
  constexpr bit_array(const bit_array<T, N, V, W>&& other) noexcept;
  constexpr bit_array(const std::initializer_list<value_type> init)
    requires(!std::is_same_v<value_type, word_type>);
  constexpr bit_array(const std::initializer_list<bool> init);
  constexpr bit_array(const std::initializer_list<word_type> init);
  constexpr bit_array(const std::string_view s)
    requires(std::is_same_v<value_type, bit_value>);

  ~bit_array() = default;
  /*
    * Assignment
    */
  constexpr bit_array& operator=(const bit_array<T, N, V, W>& other) = default;
  constexpr bit_array& operator=(bit_array<T, N, V, W>&& other) noexcept;

  constexpr bool operator==(const bit_array<T, N, V, W>& other) const noexcept;

  /*
    * Element Access
    */
  constexpr reference operator[](size_type pos);
  constexpr const_reference operator[](size_type pos) const;
  constexpr reference at(size_type pos);
  constexpr const_reference at(size_type pos) const;
  constexpr reference front();
  constexpr const_reference front() const;
  constexpr reference back();
  constexpr const_reference back() const;
  constexpr word_type* data() noexcept;
  constexpr const word_type* data() const noexcept;

  /*
    * Iterators
    */
  constexpr iterator begin() noexcept;
  constexpr iterator end() noexcept;
  constexpr const_iterator begin() const noexcept;
  constexpr const_iterator end() const noexcept;
  constexpr const_iterator cbegin() const noexcept;
  constexpr const_iterator cend() const noexcept;

  /*
    * Capacity
    */
  constexpr bool empty() const noexcept;
  constexpr size_type size() const noexcept;
  constexpr size_type max_size() const noexcept;

  /*
    * Operations
    */
  constexpr void fill(value_type bit_val) noexcept;
  constexpr void swap(bit_array<T, N, V, W>& other) noexcept;
  //constexpr std::synth-three-way-result<bit_array> operator<=>() const noexcept;
};

static_assert(bit_range<bit_array<bit_value, 11>>, "bit_array does not satisfy bit_range concept!");
static_assert(bit_sized_range<bit_array<bit_value, 11>>, "bit_array does not satisfy bit_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<bit_array<11>>, "bit_array does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<bit_array<11>>, "bit_array does not satisfy bit_contiguous_sized_range concept!");
#endif

#if 0
// Class Template Argument Deduction
// CTAD guide for the constructor taking a word_type&,
// deducing Extent as bitsof<word_type>().
template <typename word_type>
bit_array(word_type&) -> bit_array<word_type, bitsof<word_type>()>;
template <typename word_type>
bit_array(word_type*) -> bit_array<word_type, bitsof<word_type>()>;

// CTAD guide for the constructor taking a word_type* and a size,
// which should always be used when Extent is std::dynamic_extent.
template <typename word_type>
bit_array(word_type&, std::size_t) -> bit_array<word_type, std::dynamic_extent>;
template <typename word_type>
bit_array(word_type*, std::size_t) -> bit_array<word_type, std::dynamic_extent>;
#endif

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bit_array<T, N, V, W>::bit_array() noexcept : storage{} {}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bit_array<T, N, V, W>::bit_array(bit_array<T, N, V, W>::value_type bit_val) : storage{} {
  fill(bit_val);
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr void bit_array<T, N, V, W>::fill(bit_array<T, N, V, W>::value_type bit_val) noexcept {
  std::fill(this->begin(), this->end(), bit_val);
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bit_array<T, N, V, W>::bit_array(const std::initializer_list<value_type> init)
  requires(!std::is_same_v<value_type, word_type>)
{
  if(init.size() != bitsof(*this)) [[unlikely]] {
    throw std::invalid_argument("initialize_list contains an invalid number of bits for bit_array.");
  }
  std::copy(init.begin(), init.end(), this->begin());
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bit_array<T, N, V, W>::bit_array(const std::initializer_list<bool> init) {
  if(init.size() != bitsof(*this)) [[unlikely]] {
    throw std::invalid_argument("initialize_list contains an invalid number of bits for bit_array.");
  }
  std::copy(init.begin(), init.end(), this->begin());
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bit_array<T, N, V, W>::bit_array(const std::initializer_list<W> init) : storage(init) {
  static_assert(init.size() == storage.size());
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bit_array<T, N, V, W>::bit_array(const std::string_view s)
  requires(std::is_same_v<value_type, bit_value>)
{
  if(bitsof(*this) != (std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))) [[unlikely]] {
    throw std::invalid_argument("String contains an invalid number of bits for bit_array.");
  };
  size_type i = 0;
  for (char c : s) {
    if (c == '0') {
      begin()[i++] = bit0;
    } else if (c == '1') {
      begin()[i++] = bit1;
    }
  }
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bit_array<T, N, V, W>::bit_array(const bit_array<T, N, V, W>&& other) noexcept
    : storage(other.storage) {}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bit_array<T, N, V, W>& bit_array<T, N, V, W>::operator=(bit_array<T, N, V, W>&& other) noexcept {
  std::copy(other.storage.begin(), other.storage.end(), storage.begin());
  return *this;
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bool bit_array<T, N, V, W>::operator==(const bit_array<T, N, V, W>& other) const noexcept {
  return equal(begin(), end(), other.begin());
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr void bit_array<T, N, V, W>::swap(bit_array<T, N, V, W>& other) noexcept {
  std::swap(this->storage, other.storage);
}

// -------------------------------------------------------------------------- //
  /*
    * Element Access
    */
template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::reference bit_array<T, N, V, W>::operator[](size_type pos) {
  return begin()[pos];
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::const_reference bit_array<T, N, V, W>::operator[](size_type pos) const {
  return begin()[pos];
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::reference bit_array<T, N, V, W>::at(size_type pos) {
  if (pos < size()) {
    return begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::const_reference bit_array<T, N, V, W>::at(size_type pos) const {
  if (pos < size()) {
    return begin()[pos];
  } else {
    throw std::out_of_range("Position is out of range");
  }
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::reference bit_array<T, N, V, W>::front() {
  return begin()[0];
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::const_reference bit_array<T, N, V, W>::front() const {
  return begin()[0];
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::reference bit_array<T, N, V, W>::back() {
  return begin()[size()-1];
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::const_reference bit_array<T, N, V, W>::back() const {
  return begin()[size()-1];
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::word_type* bit_array<T, N, V, W>::data() noexcept {
  return size() ? storage.data() : nullptr;
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr const typename bit_array<T, N, V, W>::word_type* bit_array<T, N, V, W>::data() const noexcept {
  return size() ? storage.data() : nullptr;
}

// -------------------------------------------------------------------------- //

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::size_type bit_array<T, N, V, W>::size() const noexcept { return N; }

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::size_type bit_array<T, N, V, W>::max_size() const noexcept { return size(); }

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr bool bit_array<T, N, V, W>::empty() const noexcept { return 0 == size(); }

// Iterators
// -------------------------------------------------------------------------- //

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::iterator bit_array<T, N, V, W>::begin() noexcept {
  return iterator(storage.begin());
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::iterator bit_array<T, N, V, W>::end() noexcept {
  return begin() + size();
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::const_iterator bit_array<T, N, V, W>::begin() const noexcept {
  return const_iterator(storage.begin());
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::const_iterator bit_array<T, N, V, W>::end() const noexcept {
  return const_iterator(storage.begin()) + size();
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::const_iterator bit_array<T, N, V, W>::cbegin() const noexcept {
  return const_iterator(storage.begin());
}

template <typename T, std::size_t N, std::align_val_t V, typename W>
constexpr typename bit_array<T, N, V, W>::const_iterator bit_array<T, N, V, W>::cend() const noexcept {
  return const_iterator(storage.begin()) + size();
}

// -------------------------------------------------------------------------- //

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
        decimal = Bit - '0';
        break;
      case 10:
        decimal = Bit - '0';
        break;
      case 16:
        decimal = Bit - '0';
        if (Bit >= 'a' && Bit <= 'f') {
          decimal = Bit - 'a' + 10u;
        }
        if (Bit >= 'A' && Bit <= 'F') {
          decimal = Bit - 'A' + 10u;
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
      bits = static_cast<size_t>(std::ceil(digits * std::log2(10)));
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

#endif // _BIT_ARRAY_HPP_INCLUDED
// ========================================================================== //
