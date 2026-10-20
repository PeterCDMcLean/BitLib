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
#include <bit>
#include <cmath>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit_array_base.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit-iterator/bit.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

template <typename T = bit_value,
          std::size_t N = std::dynamic_extent,
          typename W = std::conditional_t<
              std::is_same_v<T, bit_value>,
              std::conditional_t<
                  (N == std::dynamic_extent),
                  std::uintptr_t,
                  ceil_integral<N>>,
              T>,
          typename Policy = policy::typical<std::remove_cvref_t<W>>>
class array;

template <std::size_t N = std::dynamic_extent,
          typename W = std::conditional_t<
              (N == std::dynamic_extent),
              std::uintptr_t,
              ceil_integral<N>>,
          typename Policy = policy::typical<std::remove_cvref_t<W>>>
using bit_array = array<bit_value, N, W, Policy>;

namespace detail {

template <typename value_type, typename word_type, std::size_t N>
constexpr size_t Words() { return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>(); }

template <typename value_type, typename word_type, std::size_t N>
struct array_iterator_types {
  using iterator = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                             bit_iterator<typename std::array<word_type, Words<value_type, word_type, N>()>::iterator>,
                                             typename std::array<word_type, Words<value_type, word_type, N>()>::iterator>::type;

  using const_iterator = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                                   bit_iterator<typename std::array<word_type, Words<value_type, word_type, N>()>::const_iterator>,
                                                   typename std::array<const word_type, Words<value_type, word_type, N>()>::const_iterator>::type;
};
}  // namespace detail

template <typename T, std::size_t N, typename W, typename Policy>
class array : public array_base<array<T, N, W, Policy>, T, N, W, false, Policy, detail::array_iterator_types<T, W, N>> {
 public:
  using base = array_base<array<T, N, W, Policy>, T, N, W, false, Policy, detail::array_iterator_types<T, W, N>>;
  using base::end;
  using base::size;
  using typename base::const_iterator;
  using typename base::const_pointer;
  using typename base::const_reference;
  using typename base::difference_type;
  using typename base::iterator;
  using typename base::pointer;
  using typename base::reference;
  using typename base::size_type;
  using typename base::value_type;
  using typename base::word_type;

  static constexpr std::size_t bits = N * bitsof<T>();

 protected:
  static constexpr std::size_t Words(std::size_t size_) {
    return (size_ * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>();
  }

 private:
  std::array<word_type, Words(N)> storage;

 public:
  /*
  * Constructors, copies and moves...
  */
  constexpr array() noexcept : storage{} {}

  constexpr array(value_type bit_val) {
    this->fill(bit_val);
  }

  constexpr array(detail::uninitialized_t, const size_t& size) {
    assert(size == N);
  }

  template <std::integral U>
  constexpr array(const U& integral) {
    this->from_integral(integral);
  }

  constexpr array(const array<T, N, W, Policy>& other) noexcept
      : base(), storage(other.storage) {}

  constexpr array(const array<T, N, W, Policy>&& other) noexcept
      : base(), storage(other.storage) {}

  constexpr array(const bit_sized_range auto& other) : base() {
    if (other.size() != this->size()) [[unlikely]] {
      throw std::invalid_argument("other bit_range contains an invalid number of bits for array.");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
  };

  constexpr array(const std::initializer_list<value_type> init)
    requires(!std::is_same_v<value_type, word_type>)
      : base() {
    if (init.size() != bitsof(*this)) [[unlikely]] {
      throw std::invalid_argument("initialize_list contains an invalid number of bits for array.");
    }
    std::copy(init.begin(), init.end(), this->begin());
  }

  constexpr array(const std::initializer_list<bool> init) : base() {
    if (init.size() != bitsof(*this)) [[unlikely]] {
      throw std::invalid_argument("initialize_list contains an invalid number of bits for array.");
    }
    std::copy(init.begin(), init.end(), this->begin());
  }

  constexpr array(const std::initializer_list<word_type> init) : base(), storage{} {
    // Make sure we handle the case where init.size() != Words
    auto it = init.begin();
    for (size_type i = 0; i < std::min(Words(N), init.size()); ++i, ++it) {
      storage[i] = *it;
    }
  }

  constexpr array(const std::string_view s)
    requires(std::is_same_v<value_type, bit_value>)
      : base() {
    if (bitsof(*this) != static_cast<size_t>(std::count(s.begin(), s.end(), '0') + std::count(s.begin(), s.end(), '1'))) [[unlikely]] {
      throw std::invalid_argument("String contains an invalid number of bits for array.");
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

  ~array() = default;
  /*
    * Assignment
    */
  constexpr array& operator=(const array<T, N, W, Policy>& other) = default;

  constexpr array& operator=(const bit_sized_range auto& other) {
    if (other.size() != this->size()) [[unlikely]] {
      throw std::invalid_argument("other bit_sized_range contains an invalid number of bits for array.");
    }
    ::bit::copy(other.begin(), other.end(), this->begin());
    return *this;
  };

  constexpr array& operator=(array<T, N, W, Policy>&& other) noexcept {
    std::copy(other.storage.begin(), other.storage.end(), storage.begin());
    return *this;
  }

  constexpr word_type* data() noexcept {
    return size() ? storage.data() : nullptr;
  }

  constexpr const word_type* data() const noexcept {
    return size() ? storage.data() : nullptr;
  }

  /*
    * Iterators
    */
  constexpr iterator begin() noexcept {
    return iterator(storage.begin());
  }

  constexpr const_iterator begin() const noexcept {
    return const_iterator(storage.begin());
  }

  /*
    * Capacity
    */
  constexpr size_type size() const noexcept {
    return N;
  }

  /*
    * Operations
    */
  constexpr void swap(array<T, N, W, Policy>& other) noexcept {
    std::swap(this->storage, other.storage);
  }

  constexpr operator bit_value() const noexcept
    requires(std::is_same_v<value_type, bit_value> && N == 1)
  {
    return this->begin()[0];
  }
};

static_assert(bit_range<bit_array<11>>, "array does not satisfy bit_range concept!");
static_assert(bit_sized_range<bit_array<11>>, "array does not satisfy bit_sized_range concept!");
#ifdef CONTIGUOUS_RANGE
static_assert(bit_contiguous_range<array<11>>, "array does not satisfy bit_contiguous_range concept!");
static_assert(bit_contiguous_sized_range<array<11>>, "array does not satisfy bit_contiguous_sized_range concept!");
#endif

#if 0
// Class Template Argument Deduction
// CTAD guide for the constructor taking a word_type&,
// deducing Extent as bitsof<word_type>().
template <typename word_type>
array(word_type&) -> array<word_type, bitsof<word_type>()>;
template <typename word_type>
array(word_type*) -> array<word_type, bitsof<word_type>()>;

// CTAD guide for the constructor taking a word_type* and a size,
// which should always be used when Extent is std::dynamic_extent.
template <typename word_type>
array(word_type&, std::size_t) -> array<word_type, std::dynamic_extent>;
template <typename word_type>
array(word_type*, std::size_t) -> array<word_type, std::dynamic_extent>;
#endif

/* ****************************** BIT PRIMITIVE ****************************** */
// primitive<1> (= bit_value) is defined in bit_value.hpp as a full specialization.
// primitive<Bits> (Bits > 1) is defined here, using array<bit_value, Bits, ...>
// as storage. This breaks the recursive dependency:
//   primitive<1> (bit_value) -> no array dependency
//   array<bit_value, ...> -> depends on bit_value (= primitive<1>)
//   primitive<Bits> (Bits > 1) -> depends on array<bit_value, ...>
template <size_t Bits>
class primitive {
    static_assert(Bits >= 1, "primitive must have at least 1 bit");
    static_assert(
        Bits <= bitsof<std::size_t>(),
        "primitive bit width exceeds the native integer size of the architecture");
    static_assert(Bits <= bitsof<std::uint64_t>(),
                  "primitive supports at most 64 bits");

    using word_type = ceil_integral<Bits>;
    using policy_type = policy::typical<word_type>;
    using array_type = array<bit_value, Bits, word_type, policy_type>;

    // Friendship
    template <class, primitive_c, size_t>
    friend class reference;

  public:
    static constexpr std::size_t bits = Bits;
    using size_type = std::size_t;
    using value_type = bit_value;
    using reference_type = reference<word_type&>;
    using const_reference_type = const reference<const word_type&>;

  private:
    array_type _data;

  public:
    // Lifecycle
    constexpr primitive() noexcept = default;

    constexpr explicit primitive(bool b) noexcept {
        _data.fill(b ? bit_value(true) : bit_value(false));
    }

    template <class WordType>
    constexpr explicit primitive(WordType val) noexcept
        requires std::integral<WordType> {
        static_assert(binary_digits<WordType>::value, "");
        for (size_type i = 0; i < Bits; ++i) {
            _data[i] = bit_value(static_cast<bool>((val >> i) & 1));
        }
    }

    template <class WordType>
    constexpr primitive(WordType val, size_type pos)
        requires std::integral<WordType> {
        static_assert(binary_digits<WordType>::value, "");
        assert(pos + Bits <= binary_digits<WordType>::value);
        for (size_type i = 0; i < Bits; ++i) {
            _data[i] = bit_value(static_cast<bool>((val >> (pos + i)) & 1));
        }
    }

    // Access
    constexpr reference<word_type&> operator[](size_type pos) {
        assert(pos < Bits);
        return _data[pos];
    }

    constexpr const reference<const word_type&> operator[](size_type pos) const {
        assert(pos < Bits);
        return _data[pos];
    }

    // Assignment
    constexpr primitive& operator=(const primitive& other) noexcept {
        _data = other._data;
        return *this;
    }

    template <class T>
    constexpr primitive& operator=(reference<T> ref) noexcept {
        // Set bit 0 to ref's value, clear all others
        _data[0] = bit_value(static_cast<bool>(ref));
        for (size_type i = 1; i < Bits; ++i) {
            _data[i] = bit_value(false);
        }
        return *this;
    }

    template <class WordType>
    constexpr primitive& assign(WordType val) noexcept
        requires std::integral<WordType> {
        static_assert(binary_digits<WordType>::value, "");
        for (size_type i = 0; i < Bits; ++i) {
            _data[i] = bit_value(static_cast<bool>((val >> i) & 1));
        }
        return *this;
    }

    template <class WordType>
    constexpr primitive& assign(WordType val, size_type pos)
        requires std::integral<WordType> {
        static_assert(binary_digits<WordType>::value, "");
        assert(pos + Bits <= binary_digits<WordType>::value);
        for (size_type i = 0; i < Bits; ++i) {
            _data[i] = bit_value(static_cast<bool>((val >> (pos + i)) & 1));
        }
        return *this;
    }

    // Bitwise assignment operators
    constexpr primitive& operator&=(const primitive& other) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            _data[i] &= other._data[i];
        }
        return *this;
    }

    constexpr primitive& operator|=(const primitive& other) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            _data[i] |= other._data[i];
        }
        return *this;
    }

    constexpr primitive& operator^=(const primitive& other) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            _data[i] ^= other._data[i];
        }
        return *this;
    }

    // Conversion
    explicit constexpr operator bool() const noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            if (static_cast<bool>(_data[i])) return true;
        }
        return false;
    }

    constexpr word_type word_value() const noexcept {
        word_type result = 0;
        for (size_type i = 0; i < Bits; ++i) {
            if (static_cast<bool>(_data[i])) {
                result |= (word_type(1) << i);
            }
        }
        return result;
    }

    // Swap members
    void swap(primitive& other) noexcept {
        _data.swap(other._data);
    }

    template <class T>
    void swap(reference<T> other) noexcept {
        if (static_cast<bool>(_data[0]) != static_cast<bool>(other)) {
            bit_value temp = static_cast<bit_value>(_data[0]);
            _data[0] = static_cast<bit_value>(other);
            other = temp;
        }
    }

    // Bitwise operators
    friend constexpr primitive operator~(
        primitive rhs
    ) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            rhs._data[i] = bit_value(!static_cast<bool>(rhs._data[i]));
        }
        return rhs;
    }

    friend constexpr primitive operator&(
        primitive lhs,
        const primitive& rhs
    ) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            lhs._data[i] &= rhs._data[i];
        }
        return lhs;
    }

    friend constexpr primitive operator|(
        primitive lhs,
        const primitive& rhs
    ) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            lhs._data[i] |= rhs._data[i];
        }
        return lhs;
    }

    friend constexpr primitive operator^(
        primitive lhs,
        const primitive& rhs
    ) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            lhs._data[i] ^= rhs._data[i];
        }
        return lhs;
    }

    // Comparison operators
    friend constexpr bool operator==(
        const primitive& lhs,
        const primitive& rhs
    ) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            if (static_cast<bool>(lhs._data[i]) != static_cast<bool>(rhs._data[i]))
                return false;
        }
        return true;
    }

    friend constexpr bool operator!=(
        const primitive& lhs,
        const primitive& rhs
    ) noexcept {
        return !(lhs == rhs);
    }

    friend constexpr bool operator<(
        const primitive& lhs,
        const primitive& rhs
    ) noexcept {
        for (size_type i = 0; i < Bits; ++i) {
            bool l = static_cast<bool>(lhs._data[i]);
            bool r = static_cast<bool>(rhs._data[i]);
            if (l != r) return l < r;
        }
        return false;
    }

    friend constexpr bool operator<=(
        const primitive& lhs,
        const primitive& rhs
    ) noexcept {
        return !(rhs < lhs);
    }

    friend constexpr bool operator>(
        const primitive& lhs,
        const primitive& rhs
    ) noexcept {
        return rhs < lhs;
    }

    friend constexpr bool operator>=(
        const primitive& lhs,
        const primitive& rhs
    ) noexcept {
        return !(lhs < rhs);
    }
};

}  // namespace bit

#endif  // _array_HPP_INCLUDED