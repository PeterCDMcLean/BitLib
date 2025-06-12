// ============================== BIT DETAILS =============================== //
// Project:         The C++ Bit Library
// Name:            bit_details.hpp
// Description:     Provides common implementation details and helper classes
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
//                  Bryce Kille [2019]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_DETAILS_HPP_INCLUDED
#define _BIT_DETAILS_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
#include <immintrin.h>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit_concepts.hpp"

// Project sources
// Third-party libraries
// Miscellaneous
namespace bit {
class bit_value;
template <class WordType>
class bit_reference;
template <class Iterator>
class bit_iterator;
template <class WordType>
using bit_pointer = bit_iterator<WordType*>;
template <typename target_word_ptr, typename source_word_ptr>
class bit_word_pointer_adapter;

// ========================================================================== //

/* ***************************** BINARY DIGITS ****************************** */
// Binary digits structure definition
// Implementation template: only instantiates static_asserts for non-byte types.
template <typename T, bool = std::is_same<T, std::byte>::value>
struct binary_digits_impl : std::integral_constant<std::size_t, std::numeric_limits<std::make_unsigned_t<T>>::digits> {
  static_assert(std::is_integral<T>::value, "Type must be integral");
  //static_assert(std::is_unsigned<T>::value, "Type must be unsigned");
  static_assert(!std::is_same<T, bool>::value, "Type must not be bool");
  static_assert(!std::is_same<T, char>::value, "Type must not be char");
};

// Specialization for std::byte.
template <>
struct binary_digits_impl<std::byte, true> : std::integral_constant<std::size_t, std::numeric_limits<unsigned char>::digits> {};

// Public interface that removes cv-qualifiers.
template <typename UIntType>
struct binary_digits : binary_digits_impl<std::remove_cv_t<UIntType>> {};

// Binary digits value
template <class T>
constexpr std::size_t binary_digits_v = binary_digits<T>::value;
/*************************************************************************** */

template <size_t N>
using ceil_integral = std::conditional_t<
    (N <= bitsof<std::uint8_t>()),
    std::uint8_t,
    std::conditional_t<
        (N <= bitsof<std::uint16_t>()),
        std::uint16_t,
        std::conditional_t<
            (N <= bitsof<std::uint32_t>()),
            std::uint32_t,
            std::conditional_t<
                (N <= bitsof<std::uint64_t>()),
                std::uint64_t,
                std::uint64_t>>>>;

/* *************** IMPLEMENTATION DETAILS: CV ITERATOR TRAITS *************** */
// Cv iterator traits structure definition
template <class Iterator>
struct _cv_iterator_traits
{
    // Assertions
    private:
    using _traits_t = std::iterator_traits<Iterator>;
    using _difference_t = typename _traits_t::difference_type;
    using _value_t = typename _traits_t::value_type;
    using _pointer_t = typename _traits_t::pointer;
    using _reference_t = typename _traits_t::reference;
    using _category_t =  typename _traits_t::iterator_category;
    using _no_pointer_t = typename std::remove_pointer<_pointer_t>::type;
    using _no_reference_t = typename std::remove_reference<_reference_t>::type;
    using _raw_value_t = typename std::remove_cv<_value_t>::type;
    using _raw_pointer_t = typename std::remove_cv<_no_pointer_t>::type;
    using _raw_reference_t = typename std::remove_cv<_no_reference_t>::type;
    using _cv_value_t = _no_reference_t;

    //    static_assert(std::is_same<_raw_pointer_t, _raw_value_t>::value, "");
    //    static_assert(std::is_same<_raw_reference_t, _raw_value_t>::value, "");

    // Types
    public:
    using difference_type = _difference_t;
    using value_type = _cv_value_t;
    using pointer = _pointer_t;
    using reference = _reference_t;
    using iterator_category = _category_t;
};
/* ************************************************************************** */

#if 0
/* *********** IMPLEMENTATION DETAILS: NARROWEST AND WIDEST TYPES *********** */
// Narrowest type structure declaration
template <class... T>
struct _narrowest_type;

// Narrowest type structure specialization: selects the only passed type
template <class T>
struct _narrowest_type<T>
: std::common_type<T>
{
    static_assert(binary_digits<T>::value, "");
};

// Narrowest type structure specialization: selects the type with less bits
template <class T, class U>
struct _narrowest_type<T, U>
: _narrowest_type<
    typename std::conditional<
        (binary_digits<T>::value < binary_digits<U>::value),
        T,
        typename std::conditional<
            (binary_digits<T>::value > binary_digits<U>::value),
            U,
            typename std::common_type<T, U>::type
        >::type
    >::type
>
{
};

// Narrowest type structure specialization: recursively selects the right type
template <class T, class... U>
struct _narrowest_type<T, U...>
: _narrowest_type<T, typename _narrowest_type<U...>::type>
{
};

// Narrowest type alias
template <class... T>
using _narrowest_type_t = typename _narrowest_type<T...>::type;

// Widest type structure declaration
template <class... X>
struct _widest_type;

// Widest type structure specialization: selects the only passed type
template <class T>
struct _widest_type<T>
: std::common_type<T>
{
    static_assert(binary_digits<T>::value, "");
};

// Widest type structure specialization: selects the type with more bits
template <class T, class U>
struct _widest_type<T, U>
: _widest_type<
    typename std::conditional<
        (binary_digits<T>::value > binary_digits<U>::value),
        T,
        typename std::conditional<
            (binary_digits<T>::value < binary_digits<U>::value),
            U,
            typename std::common_type<T, U>::type
        >::type
    >::type
>
{
};

// Widest type structure specialization: recursively selects the right type
template <class T, class... X>
struct _widest_type<T, X...>
: _widest_type<T, typename _widest_type<X...>::type>
{
};

// Widest type alias
template <class... T>
using _widest_type_t = typename _widest_type<T...>::type;
/* ************************************************************************** */



/* ************ IMPLEMENTATION DETAILS: NARROWER AND WIDER TYPES ************ */
// Narrower type structure definition
template <class T, int I = 0>
struct _narrower_type
{
    using tuple = std::tuple<
        unsigned long long int,
        unsigned long int,
        unsigned int,
        unsigned short int,
        unsigned char
    >;
    using lhs_bits = binary_digits<T>;
    using rhs_bits = binary_digits<typename std::tuple_element<I, tuple>::type>;
    using type = typename std::conditional<
        (lhs_bits::value > rhs_bits::value),
        typename std::tuple_element<I, tuple>::type,
        typename std::conditional<
            (I + 1 < std::tuple_size<tuple>::value),
            typename _narrower_type<
                T,
                (I + 1 < std::tuple_size<tuple>::value ? I + 1 : -1)
            >::type,
            typename std::tuple_element<I, tuple>::type
        >::type
    >::type;
};

// Narrower type structure specialization: not found
template <class T>
struct _narrower_type<T, -1>
{
    using type = T;
};

// Narrower type alias
template <class T>
using _narrower_type_t = typename _narrower_type<T>::type;

// Wider type structure definition
template <class T, int I = 0>
struct _wider_type
{
    using tuple = std::tuple<
        unsigned char,
        unsigned short int,
        unsigned int,
        unsigned long int,
        unsigned long long int
    >;
    using lhs_bits = binary_digits<T>;
    using rhs_bits = binary_digits<typename std::tuple_element<I, tuple>::type>;
    using type = typename std::conditional<
        (lhs_bits::value < rhs_bits::value),
        typename std::tuple_element<I, tuple>::type,
        typename std::conditional<
            (I + 1 < std::tuple_size<tuple>::value),
            typename _narrower_type<
                T,
                (I + 1 < std::tuple_size<tuple>::value ? I + 1 : -1)
            >::type,
            typename std::tuple_element<I, tuple>::type
        >::type
    >::type;
};

// Wider type structure specialization: not found
template <class T>
struct _wider_type<T, -1>
{
    using type = T;
};

// Wider type alias
template <class T>
using _wider_type_t = typename _wider_type<T>::type;
/* ************************************************************************** */
#endif

/*
exact_ceil_integral is used to determine the exact integral type that a proxy reference
can be implicitly converted to.
If the proxy reference supports multiple types, it will pick the largest, preferring unsigned.
*/
template <typename T>
struct exact_ceil_integral {
 private:
  using U = std::remove_cvref_t<T>;

  template <typename From, typename To>
  static constexpr bool is_exactly_convertible() {
    if constexpr (!std::is_convertible_v<From, To>) {
      return false;
    } else {
      // Try brace-initialization to detect narrowing
      return requires(From f) {
        To{f};  // will fail if narrowing
      };
    }
  }

 public:
  using type = std::conditional_t<
      is_exactly_convertible<U, uint64_t>(), uint64_t,
      std::conditional_t<
          is_exactly_convertible<U, int64_t>(), int64_t,
          std::conditional_t<
              is_exactly_convertible<U, uint32_t>(), uint32_t,
              std::conditional_t<
                  is_exactly_convertible<U, int32_t>(), int32_t,
                  std::conditional_t<
                      is_exactly_convertible<U, uint16_t>(), uint16_t,
                      std::conditional_t<
                          is_exactly_convertible<U, int16_t>(), int16_t,
                          std::conditional_t<
                              is_exactly_convertible<U, uint8_t>(), uint8_t,
                              std::conditional_t<
                                  is_exactly_convertible<U, int8_t>(), int8_t,
                                  void>>>>>>>>;
};

// Helper alias
template <typename T>
using exact_ceil_integral_t = typename exact_ceil_integral<T>::type;

/* ******************* IMPLEMENTATION DETAILS: UTILITIES ******************** */
// Assertions
template <class Iterator>
constexpr bool _assert_range_viability(Iterator first, Iterator last);
/* ************************************************************************** */



/* ****************** IMPLEMENTATION DETAILS: INSTRUCTIONS ****************** */
// Population count
template <class T, class = decltype(__builtin_popcountll(T()))>
constexpr T _popcnt(T src) noexcept;
template <class T, class... X>
constexpr T _popcnt(T src, X...) noexcept;

// Leading zeros count
template <class T, class = decltype(__builtin_clzll(T()))>
constexpr T _lzcnt(T src) noexcept;
template <class T, class... X>
constexpr T _lzcnt(T src, X...) noexcept;

// Trailing zeros count
template <class T, class = decltype(__builtin_ctzll(T()))>
constexpr T _tzcnt(T src) noexcept;
template <class T, class... X>
constexpr T _tzcnt(T src, X...) noexcept;

// Bit field extraction
template <class T, class = decltype(__builtin_ia32_bextr_u64(T(), T(), T()))>
constexpr T _bextr(T src, T start, T len) noexcept;
template <class T, class... X>
constexpr T _bextr(T src, T start, T len, X...) noexcept;

#if 0
// Parallel bits deposit
template <class T, class = decltype(_pdep_u64(T()))>
constexpr T _pdep(T src, T msk) noexcept;
template <class T, class... X>
constexpr T _pdep(T src, T msk, X...) noexcept;

// Parallel bits extract
template <class T, class = decltype(_pext_u64(T()))>
constexpr T _pext(T src, T msk) noexcept;
template <class T, class... X>
constexpr T _pext(T src, T msk, X...) noexcept;

// Byte swap
template <class T, class T128 = decltype(__uint128_t(__builtin_bswap64(T())))>
constexpr T _byteswap(T src) noexcept;
template <class T, class... X>
constexpr T _byteswap(T src, X...) noexcept;

#endif

// Bit swap
template <class T>
constexpr T _bitswap(T src) noexcept;
template <class T, std::size_t N>
constexpr T _bitswap(T src) noexcept;
template <class T, std::size_t N>
constexpr T _bitswap() noexcept;

// Bit exchange
template <class T>
constexpr void _bitexch(T& src0, T& src1, T msk) noexcept;
template <class T, class S>
constexpr void _bitexch(T& src0, T& src1, S start, S len) noexcept;
template <class T, class S>
constexpr void _bitexch(T& src0, T& src1, S start0, S start1, S len) noexcept;

// Bit compare
template <class T>
constexpr T _bitcmp(T src0, T src1, T start0, T start1, T len) noexcept;

// Double precision shift left
template <class T>
constexpr T _shld(T dst, T src, T cnt) noexcept;

// Double precision shift right
template <class T>
constexpr T _shrd(T dst, T src, T cnt) noexcept;

// Add carry
template <class... T>
using _supports_adc = decltype(__builtin_ia32_addcarryx_u64(T()...));
template <class C, class T, class = _supports_adc<C, T, T, std::nullptr_t>>
constexpr C _addcarry(C carry, T src0, T src1, T* dst) noexcept;
template <class C, class T, class... X>
constexpr C _addcarry(C carry, T src0, T src1, T* dst, X...) noexcept;

// Sub borrow
template <class... T>
using _supports_sbb = decltype(__builtin_ia32_sbb_u64(T()...));
template <class... T>
using _supports_sbb_alt = decltype(__builtin_ia32_subborrow_u64(T()...));
template <class B, class T, class = _supports_sbb<B, T, T, std::nullptr_t>>
constexpr B _subborrow(B borrow, T src0, T src1, T* dst) noexcept;
template <class B, class T, class = _supports_sbb_alt<B, T, T, std::nullptr_t>>
constexpr B _subborrow(const B& borrow, T src0, T src1, T* dst) noexcept;
template <class B, class T, class... X>
constexpr B _subborrow(B borrow, T src0, T src1, T* dst, X...) noexcept;

// Multiword multiply
template <class T, class T128 = decltype(__uint128_t(T()))>
constexpr T _mulx(T src0, T src1, T* hi) noexcept;
template <class T, class... X>
constexpr T _mulx(T src0, T src1, T* hi, X...) noexcept;
/* ************************************************************************** */

/*
Logical shift right
*/
template <std::integral T, typename size_type = size_t>
constexpr T lsr(const T val, const size_type shift) {
  return static_cast<T>(static_cast<std::make_unsigned_t<T>>(val) >> shift);
}

template <typename T, typename size_type = size_t>
constexpr exact_ceil_integral_t<T> lsr(const T val, const size_type shift) {
  return static_cast<exact_ceil_integral_t<T>>(static_cast<std::make_unsigned_t<exact_ceil_integral_t<T>>>(val) >> shift);
}

enum class _mask_len {
  unknown,
  in_range
};

template <std::integral T, _mask_len len_in_range = _mask_len::in_range, typename size_type = size_t>
constexpr T _mask(const size_type len) {
  constexpr std::make_unsigned_t<T> one = std::make_unsigned_t<T>(1);
  if constexpr (len_in_range != _mask_len::unknown) {
    return static_cast<T>((one << len) - one);
  } else {
    // The digits_mask is solely here to prevent Undefined Sanitizer
    // complaining about shift of len >= digits
    // Note: on -O1 the (len & digits_mask) is optimized to simply (len)
    constexpr std::make_unsigned_t<T> digits_mask = bitsof<T>() - one;
    return static_cast<T>((one << (len & digits_mask)) * (len < bitsof<T>()) - one);
  }
}

// ------------- IMPLEMENTATION DETAILS: UTILITIES: ASSERTIONS -------------- //
// If the range allows multipass iteration, checks if last - first >= 0
template <class Iterator>
constexpr bool _assert_range_viability(Iterator first, Iterator last) {
  using traits_t = std::iterator_traits<Iterator>;
  using category_t = typename traits_t::iterator_category;
  using multi_t = std::forward_iterator_tag;
  constexpr bool is_multipass = std::is_base_of<multi_t, category_t>::value;
  const bool is_viable = !is_multipass || std::distance(first, last) >= 0;
  assert(is_viable);
  return is_viable;
}
// -------------------------------------------------------------------------- //

// --------- IMPLEMENTATION DETAILS: INSTRUCTIONS: POPULATION COUNT --------- //
// Counts the number of bits set to 1 with compiler intrinsics
template <class T, class>
constexpr T _popcnt(T src) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  if (digits <= std::numeric_limits<unsigned int>::digits) {
    src = __builtin_popcount(static_cast<std::make_unsigned_t<T>>(src));
  } else if (digits <= std::numeric_limits<unsigned long int>::digits) {
    src = __builtin_popcountl(static_cast<std::make_unsigned_t<T>>(src));
  } else if (digits <= std::numeric_limits<unsigned long long int>::digits) {
    src = __builtin_popcountll(static_cast<std::make_unsigned_t<T>>(src));
  } else {
    src = _popcnt(src, std::ignore);
  }
  return src;
}

// Counts the number of bits set to 1 without compiler intrinsics
template <class T, class... X>
constexpr T _popcnt(T src, X...) noexcept {
  static_assert(binary_digits<T>::value, "");
  T dst = T();
  for (dst = T(); src; src = lsr(src, 1)) {
    dst += src & 1;
  }
  return dst;
}
// -------------------------------------------------------------------------- //

// ------- IMPLEMENTATION DETAILS: INSTRUCTIONS: LEADING ZEROS COUNT -------- //
// Counts the number of leading zeros with compiler intrinsics
template <class T, class>
constexpr T _lzcnt(T src) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = T();
  if (digits < std::numeric_limits<unsigned int>::digits) {
    dst = src ? __builtin_clz(src) - (std::numeric_limits<unsigned int>::digits - digits)
              : digits;
  } else if (digits == std::numeric_limits<unsigned int>::digits) {
    dst = src ? __builtin_clz(src) : digits;
  } else if (digits < std::numeric_limits<unsigned long int>::digits) {
    dst = src ? __builtin_clzl(src) - (std::numeric_limits<unsigned long int>::digits - digits)
              : digits;
  } else if (digits == std::numeric_limits<unsigned long int>::digits) {
    dst = src ? __builtin_clzl(src) : digits;
  } else if (digits < std::numeric_limits<unsigned long long int>::digits) {
    dst = src ? __builtin_clzll(src) - (std::numeric_limits<unsigned long long int>::digits - digits)
              : digits;
  } else if (digits == std::numeric_limits<unsigned long long int>::digits) {
    dst = src ? __builtin_clzll(src) : digits;
  } else {
    dst = _lzcnt(src, std::ignore);
  }
  return dst;
}

// Counts the number of leading zeros without compiler intrinsics
template <class T, class... X>
constexpr T _lzcnt(T src, X...) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = src != T();
  while ((src = lsr(src, 1))) {
    ++dst;
  }
  return digits - dst;
}
// -------------------------------------------------------------------------- //

// ------- IMPLEMENTATION DETAILS: INSTRUCTIONS: TRAILING ZEROS COUNT ------- //
// Counts the number of trailing zeros with compiler intrinsics
template <class T, class>
constexpr T _tzcnt(T src) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = T();
  if (digits <= std::numeric_limits<unsigned int>::digits) {
    dst = src ? __builtin_ctz(src) : digits;
  } else if (digits <= std::numeric_limits<unsigned long int>::digits) {
    dst = src ? __builtin_ctzl(src) : digits;
  } else if (digits <= std::numeric_limits<unsigned long long int>::digits) {
    dst = src ? __builtin_ctzll(src) : digits;
  } else {
    dst = _tzcnt(src, std::ignore);
  }
  return dst;
}

// Counts the number of trailing zeros without compiler intrinsics
template <class T, class... X>
constexpr T _tzcnt(T src, X...) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = digits;
  if (src) {
    src = lsr((src ^ (src - 1)), 1);
    for (dst = T(); src; dst++) {
      src = lsr(src, 1);
    }
  }
  return dst;
}
// -------------------------------------------------------------------------- //

// ------- IMPLEMENTATION DETAILS: INSTRUCTIONS: BIT FIELD EXTRACTION ------- //
// Extacts to lsbs a field of contiguous bits with compiler intrinsics
template <class T, class>
constexpr T _bextr(T src, T start, T len) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = T();
  if (digits <= std::numeric_limits<unsigned int>::digits) {
    dst = __builtin_ia32_bextr_u32(src, start, len);
  } else if (digits <= std::numeric_limits<unsigned long long int>::digits) {
    dst = __builtin_ia32_bextr_u64(src, start, len);
  } else {
    dst = _bextr(src, start, len, std::ignore);
  }
  return dst;
}

// Extacts to lsbs a field of contiguous bits without compiler intrinsics
template <class T, class... X>
constexpr T _bextr(T src, T start, T len, X...) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  constexpr T one = 1;
  const T msk = (one << len) * (len < digits) - one;
  return (lsr(src, start)) & msk * (start < digits);
}
// -------------------------------------------------------------------------- //

#if 0
// ------- IMPLEMENTATION DETAILS: INSTRUCTIONS: PARALLEL BIT DEPOSIT ------- //
// Deposits bits according to a mask with compiler instrinsics
template <class T, class>
constexpr T _pdep(T src, T msk) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = T();
  if (digits <= std::numeric_limits<unsigned int>::digits) {
    dst = _pdep_u32(src, msk);
  } else if (digits <= std::numeric_limits<unsigned long long int>::digits) {
    dst = _pdep_u64(src, msk);
  } else {
    dst = _pdep(src, msk, std::ignore);
  }
  return dst;
}

// Deposits bits according to a mask without compiler instrinsics
template <class T, class... X>
constexpr T _pdep(T src, T msk, X...) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = T();
  T cnt = T();
  while (msk) {
    dst = lsr(dst, 1);
    if (msk & 1) {
      dst |= src << (digits - 1);
      src = lsr(src, 1);
    }
    msk = lsr(msk, 1);
    ++cnt;
  }
  dst = lsr(dst, (digits - cnt) * (cnt > 0));
  return dst;
}
// -------------------------------------------------------------------------- //

// ------- IMPLEMENTATION DETAILS: INSTRUCTIONS: PARALLEL BIT EXTRACT ------- //
// Extracts bits according to a mask with compiler instrinsics
template <class T, class>
constexpr T _pext(T src, T msk) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = T();
  if (digits <= std::numeric_limits<unsigned int>::digits) {
    dst = _pext_u32(src, msk);
  } else if (digits <= std::numeric_limits<unsigned long long int>::digits) {
    dst = _pext_u64(src, msk);
  } else {
    dst = _pext(src, msk, std::ignore);
  }
  return dst;
}

// Extracts bits according to a mask without compiler instrinsics
template <class T, class... X>
constexpr T _pext(T src, T msk, X...) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T dst = T();
  T cnt = T();
  while (msk) {
    if (msk & 1) {
      dst = lsr(dst, 1);
      dst |= src << (digits - 1);
      ++cnt;
    }
    src = lsr(src, 1);
    msk = lsr(msk, 1);
  }
  dst = lsr(dst, (digits - cnt) * (cnt > 0));
  return dst;
}
// -------------------------------------------------------------------------- //

// ------------ IMPLEMENTATION DETAILS: INSTRUCTIONS: BYTE SWAP ------------- //
// Reverses the order of the underlying bytes with compiler intrinsics
template <class T, class T128>
constexpr T _byteswap(T src) noexcept {
  static_assert(binary_digits<T>::value, "");
  using byte_t = unsigned char;
  constexpr T digits = sizeof(T) * std::numeric_limits<byte_t>::digits;
  std::uint64_t tmp64 = 0;
  std::uint64_t* ptr64 = nullptr;
  if (std::is_same<T, T128>::value) {
    ptr64 = reinterpret_cast<std::uint64_t*>(&src);
    tmp64 = __builtin_bswap64(*ptr64);
    *ptr64 = __builtin_bswap64(*(ptr64 + 1));
    *(ptr64 + 1) = tmp64;
  } else if (digits == std::numeric_limits<std::uint16_t>::digits) {
    src = __builtin_bswap16(src);
  } else if (digits == std::numeric_limits<std::uint32_t>::digits) {
    src = __builtin_bswap32(src);
  } else if (digits == std::numeric_limits<std::uint64_t>::digits) {
    src = __builtin_bswap64(src);
  } else if (digits > std::numeric_limits<byte_t>::digits) {
    src = _byteswap(src, std::ignore);
  }
  return src;
}

// Reverses the order of the underlying bytes without compiler intrinsics
template <class T, class... X>
constexpr T _byteswap(T src, X...) noexcept {
  static_assert(binary_digits<T>::value, "");
  using byte_t = unsigned char;
  constexpr T half = sizeof(T) / 2;
  constexpr T end = sizeof(T) - 1;
  unsigned char* bytes = reinterpret_cast<byte_t*>(&src);
  unsigned char byte = 0;
  for (T i = T(); i < half; ++i) {
    byte = bytes[i];
    bytes[i] = bytes[end - i];
    bytes[end - i] = byte;
  }
  return src;
}
// -------------------------------------------------------------------------- //
#endif

// ------------- IMPLEMENTATION DETAILS: INSTRUCTIONS: BIT SWAP ------------- //
// Reverses the order of the bits with or without of compiler intrinsics
template <class T>
constexpr T _bitswap(T src) noexcept {
  static_assert(binary_digits<T>::value, "");
  using byte_t = unsigned char;
  constexpr auto ignore = nullptr;
  constexpr T digits = binary_digits<T>::value;
  constexpr unsigned long long int first = 0x80200802ULL;
  constexpr unsigned long long int second = 0x0884422110ULL;
  constexpr unsigned long long int third = 0x0101010101ULL;
  constexpr unsigned long long int fourth = 32;
  constexpr bool is_size1 = sizeof(T) == 1;
  constexpr bool is_byte = digits == std::numeric_limits<byte_t>::digits;
  constexpr bool is_octet = std::numeric_limits<byte_t>::digits == 8;
  constexpr bool is_pow2 = _popcnt(digits, ignore) == 1;
  T dst = src;
  T i = digits - 1;
  if (is_size1 && is_byte && is_octet) {
    dst = static_cast<T>(lsr(((static_cast<std::make_unsigned_t<T>>(src) * first) & second) * third, fourth));
  } else if (is_pow2) {
    dst = _bitswap<T, digits>(src);
  } else {
    for (src = lsr(src, 1); src; src = lsr(src, 1)) {
      dst <<= 1;
      dst |= src & 1;
      i--;
    }
    dst <<= i;
  }
  return dst;
}

// Reverses the order of the bits: recursive metafunction
template <class T, std::size_t N>
constexpr T _bitswap(T src) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T cnt = N >> 1;
  constexpr T msk = _bitswap<T, cnt>();
  src = ((lsr(src, cnt)) & msk) | ((src << cnt) & ~msk);
  return cnt > 1 ? _bitswap<T, cnt>(src) : src;
}

// Reverses the order of the bits: mask for the recursive metafunction
template <class T, std::size_t N>
constexpr T _bitswap() noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  T cnt = digits;
  T msk = ~T();
  while (cnt != N) {
    cnt = lsr(cnt, 1);
    msk ^= (msk << cnt);
  }
  return msk;
}
// -------------------------------------------------------------------------- //

// ------------ IMPLEMENTATION DETAILS: INSTRUCTIONS: BIT BLEND ------------- //
// Replaces bits of src0 by the ones of src1 where the mask is true

template <typename T, typename U>
constexpr exact_ceil_integral_t<T> _bitblend(
    const T src0_,
    const U src1_,
    const exact_ceil_integral_t<T> msk) noexcept
  requires(std::is_same_v<exact_ceil_integral_t<T>, exact_ceil_integral_t<U>>)
{
  static_assert(binary_digits<exact_ceil_integral_t<T>>::value, "");
  const exact_ceil_integral_t<T> src0 = static_cast<exact_ceil_integral_t<T>>(src0_);
  const exact_ceil_integral_t<U> src1 = static_cast<exact_ceil_integral_t<U>>(src1_);
  return src0 ^ ((src0 ^ src1) & msk);
}

// Replaces len bits of src0 by the ones of src1 starting at start
template <typename T, typename U>
constexpr exact_ceil_integral_t<T> _bitblend(
    const T src0_,
    const U src1_,
    const exact_ceil_integral_t<T> start,
    const exact_ceil_integral_t<T> len) noexcept
  requires(std::is_same_v<exact_ceil_integral_t<T>, exact_ceil_integral_t<U>>)
{
  static_assert(binary_digits<exact_ceil_integral_t<T>>::value, "");
  constexpr exact_ceil_integral_t<T> digits = bitsof<exact_ceil_integral_t<T>>();
  const exact_ceil_integral_t<T> src0 = static_cast<exact_ceil_integral_t<T>>(src0_);
  const exact_ceil_integral_t<U> src1 = static_cast<exact_ceil_integral_t<U>>(src1_);
  const exact_ceil_integral_t<T> msk = _mask<exact_ceil_integral_t<T>, _mask_len::unknown>(len) << start;
  return src0 ^ ((src0 ^ src1) & msk * (start < digits));
}
// -------------------------------------------------------------------------- //

// ---------- IMPLEMENTATION DETAILS: INSTRUCTIONS: BIT EXCHANGE ------------ //
// Exchanges/swaps bits of src0 by the ones of src1 where the mask is true
template <class T>
constexpr void _bitexch(T& src0, T& src1, T msk) noexcept {
  src0 = src0 ^ static_cast<T>(src1 & msk);
  src1 = src1 ^ static_cast<T>(src0 & msk);
  src0 = src0 ^ static_cast<T>(src1 & msk);
  return;
}

// Replaces len bits of src0 by the ones of src1 starting at start
template <class T, class S>
constexpr void _bitexch(T& src0, T& src1, S start, S len) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr auto digits = binary_digits<T>::value;
  const T msk = (len < digits)
                    ? _mask<T, _mask_len::unknown>(len) << start
                    : -1;  // TODO: What if start > 0 here?
  src0 = src0 ^ static_cast<T>(src1 & msk);
  src1 = src1 ^ static_cast<T>(src0 & msk);
  src0 = src0 ^ static_cast<T>(src1 & msk);
  return;
}

// Replaces len bits of src0 by the ones of src1 starting at start0
// in src0 and start1 in src1.
// len <= digits-max(start0, start1)
// clang-format off
template <class T, class S>
constexpr void _bitexch(T& src0, T& src1, S start0, S start1, S len) noexcept
{
    static_assert(binary_digits<T>::value, "");
    constexpr auto digits = binary_digits<T>::value;
    const T msk = _mask<T, _mask_len::unknown>(len);
    if (start0 >= start1) {
        src0 = src0 ^ (
                static_cast<T>(src1 << (start0 - start1))
                &
                static_cast<T>(msk << start0)
        );
        src1 = src1 ^ (
                static_cast<T>(lsr(src0, (start0 - start1)))
                &
                static_cast<T>(msk << start1)
        );
        src0 = src0 ^ (
                static_cast<T>(src1 << (start0 - start1))
                &
                static_cast<T>(msk << start0)
        );
    } else {
        src0 = src0 ^ (
                static_cast<T>(lsr(src1, (start1 - start0)))
                &
                static_cast<T>(msk << start0)
        );
        src1 = src1 ^ (
                static_cast<T>(src0 << (start1 - start0))
                &
                static_cast<T>(msk << start1)
        );
        src0 = src0 ^ (
                static_cast<T>(lsr(src1, (start1 - start0)))
                &
                static_cast<T>(msk << start0)
        );
    }
    return;
}
// clang-format on
// -------------------------------------------------------------------------- //

// ----------- IMPLEMENTATION DETAILS: INSTRUCTIONS: BIT COMPARE ------------ //
// Compares a subsequence of bits within src0 and src1 and returns 0 if equal
template <class T>
constexpr T _bitcmp(T src0, T src1, T start0, T start1, T len) noexcept {
  static_assert(binary_digits<T>::value, "");
  return _bextr(src0, start0, len) == _bextr(src1, start1, len);
}
// -------------------------------------------------------------------------- //

// --- IMPLEMENTATION DETAILS: INSTRUCTIONS: DOUBLE PRECISION SHIFT LEFT ---- //
// Left shifts dst by cnt bits, filling the lsbs of dst by the msbs of src
template <class T>
constexpr T _shld(T dst, T src, T cnt) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  if (cnt < digits) {
    dst = (dst << cnt) | (lsr(src, (digits - cnt)));
  } else {
    dst = (src << (cnt - digits)) * (cnt < digits + digits);
  }
  return dst;
}
// -------------------------------------------------------------------------- //

// --- IMPLEMENTATION DETAILS: INSTRUCTIONS: DOUBLE PRECISION SHIFT RIGHT --- //
// Right shifts dst by cnt bits, filling the msbs of dst by the lsbs of src
template <class T>
constexpr T _shrd(T dst, T src, T cnt) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  if (cnt < digits) {
    dst = (lsr(dst, cnt)) | (src << (digits - cnt));
  } else {
    dst = (lsr(src, (cnt - digits))) * (cnt < digits + digits);
  }
  return dst;
}
// -------------------------------------------------------------------------- //

#if defined(__ADX__)
template <bool Add>
unsigned char ADDCARRYSUBBORROW32(unsigned char c, uint32_t a, uint32_t b, uint32_t* out) {
  return (Add ? _addcarryx_u32(c, a, b, out) : _subborrow_u32(c, a, b, out));
}
template <bool Add>
unsigned char ADDCARRYSUBBORROW64(unsigned char c, uint64_t a, uint64_t b, uint64_t* out) {
  static_assert(sizeof(uint64_t) == sizeof(unsigned long long int));
  return (Add ? _addcarryx_u64(c, a, b, reinterpret_cast<unsigned long long int*>(out)) : _subborrow_u64(c, a, b, reinterpret_cast<unsigned long long int*>(out)));
}
#else
template <bool Add>
unsigned char ADDCARRYSUBBORROW32(unsigned char c, uint32_t a, uint32_t b, uint32_t* out) {
  return (Add ? _addcarry_u32(c, a, b, out) : _subborrow_u32(c, a, b, out));
}
template <bool Add>
unsigned char ADDCARRYSUBBORROW64(unsigned char c, uint64_t a, uint64_t b, uint64_t* out) {
  static_assert(sizeof(uint64_t) == sizeof(unsigned long long int));
  return (Add ? _addcarry_u64(c, a, b, reinterpret_cast<unsigned long long int*>(out)) : _subborrow_u64(c, a, b, reinterpret_cast<unsigned long long int*>(out)));
}
#endif

template <bool Add, std::integral U>
static inline unsigned char add_carry_sub_borrow(unsigned char c_in, U a, U b, U* out) noexcept {
  if constexpr (32 > bitsof<U>()) {
    // a       [aaaaaaaa111111111111111111111111111]
    // b     + [bbbbbbbb000000000000000000000000000]
    // carry +                            [0000000c]
    const uint8_t shift = (32 - bitsof<U>());
    uint32_t carry_propagation = Add ? ((1 << shift) - 1) : 0;
    uint32_t tmp_out;
    unsigned char carry = ADDCARRYSUBBORROW32<Add>(
        c_in,
        (static_cast<uint32_t>(a) << shift) | carry_propagation,
        (static_cast<uint32_t>(b) << shift),
        &tmp_out);
    *out = static_cast<U>(tmp_out >> shift);
    return carry;
  } else if constexpr (32 == bitsof<U>()) {
    return ADDCARRYSUBBORROW32<Add>(c_in, static_cast<uint32_t>(a), static_cast<uint32_t>(b), reinterpret_cast<uint32_t>(out));
  } else if constexpr (64 == bitsof<U>()) {
    return ADDCARRYSUBBORROW64<Add>(c_in, static_cast<uint64_t>(a), static_cast<uint64_t>(b), reinterpret_cast<uint64_t>(out));
  } else if constexpr (0 == (bitsof<U>() % 64)) {
    using t64 = std::conditional<std::is_signed_v<U>, int64_t, uint64_t>;
    unsigned char carry;
    for (int i = 0; i < (bitsof<U>() / 64); i++) {
      carry = ADDCARRYSUBBORROW64<Add>(c_in, static_cast<t64>(a >> (i * 64)), static_cast<t64>(b >> (i * 64)), reinterpret_cast<t64>(out) + i);
    }
    return carry;
  } else {
    assert(((void)"add carry intrinsics support only support powers of 2 bits", false));
  }
}

template <std::integral U>
static inline unsigned char add_carry(unsigned char c_in, U a, U b, U* out) noexcept {
  return add_carry_sub_borrow<true, U>(c_in, a, b, out);
}

template <std::integral U>
static inline unsigned char sub_borrow(unsigned char c_in, U a, U b, U* out) noexcept {
  return add_carry_sub_borrow<false, U>(c_in, a, b, out);
}

// -------------------------------------------------------------------------- //

// -------- IMPLEMENTATION DETAILS: INSTRUCTIONS: MULTIWORD MULTIPLY -------- //
// Multiplies src0 and src1 and gets the full result with compiler intrinsics
template <class T, class T128>
constexpr T _mulx(T src0, T src1, T* hi) noexcept
{
    static_assert(binary_digits<T>::value, "");
    using wider_t = ceil_integral<bitsof<T>() + bitsof<T>()>;
    constexpr T digits = binary_digits<T>::value;
    wider_t tmp = 0;
    T128 tmp128 = 0;
    T lo = 0;
    if (digits == std::numeric_limits<std::uint64_t>::digits) {
        tmp128 = static_cast<T128>(src0) * static_cast<T128>(src1);
        *hi = tmp128 >> digits;
        lo = tmp128;
    } else if (digits + digits == binary_digits<wider_t>::value) {
        tmp = static_cast<wider_t>(src0) * static_cast<wider_t>(src1);
        *hi = tmp >> digits;
        lo = tmp;
    } else {
        lo = _mulx(src0, src1, hi, std::ignore);
    }
    return lo;
}

// Multiplies src0 and src1 and gets the full result without compiler intrinsics
template <class T, class... X>
constexpr T _mulx(T src0, T src1, T* hi, X...) noexcept
{
    static_assert(binary_digits<T>::value, "");
    constexpr T digits = binary_digits<T>::value;
    constexpr T offset = digits / 2;
    constexpr T ones = ~static_cast<T>(0);
    const T lsbs0 = src0 & static_cast<T>(lsr(ones, (digits - offset)));
    const T msbs0 = lsr(src0, offset);
    const T lsbs1 = src1 & static_cast<T>(lsr(ones, (digits - offset)));
    const T msbs1 = lsr(src1, offset);
    const T llsbs = lsbs0 * lsbs1;
    const T mlsbs = msbs0 * lsbs1;
    const T lmsbs = lsbs0 * msbs1;
    const T mi = mlsbs + lmsbs;
    const T lo = llsbs + static_cast<T>(mi << offset);
    const T lcarry = lo < llsbs || lo < static_cast<T>(mi << offset);
    const T mcarry = static_cast<T>(mi < mlsbs || mi < lmsbs) << offset;
    *hi = static_cast<T>(lsr(mi, offset)) + msbs0 * msbs1 + mcarry + lcarry;
    return lo;
}
// -------------------------------------------------------------------------- //

template <typename return_type, typename AlgoFunc, typename SrcIt, typename DstIt>
constexpr return_type with_bit_iterator_adapter(
    bit_iterator<SrcIt> first,
    bit_iterator<SrcIt> last,
    bit_iterator<DstIt> d_first) {
  using dst_word_type = typename bit_iterator<DstIt>::word_type;
  using src_word_type = typename bit_iterator<SrcIt>::word_type;
  if constexpr (!std::is_same_v<src_word_type, dst_word_type> && bitsof<src_word_type>() != bitsof<dst_word_type>()) {
    if constexpr (bitsof<src_word_type>() > bitsof<dst_word_type>()) {
      bit_iterator<bit_word_pointer_adapter<DstIt, SrcIt>> adapted_first(
          bit_word_pointer_adapter<DstIt, SrcIt>(first.base(), first.position() / bitsof<dst_word_type>()), first.position() % bitsof<dst_word_type>());
      bit_iterator<bit_word_pointer_adapter<DstIt, SrcIt>> adapted_last(
          bit_word_pointer_adapter<DstIt, SrcIt>(last.base(), last.position() / bitsof<dst_word_type>()), last.position() % bitsof<dst_word_type>());
      return AlgoFunc{}(adapted_first, adapted_last, d_first);
    } else {
      bit_iterator<bit_word_pointer_adapter<SrcIt, DstIt>> adapted_d_first(
          bit_word_pointer_adapter<SrcIt, DstIt>(d_first.base(), d_first.position() / bitsof<src_word_type>()), d_first.position() % bitsof<src_word_type>());
      return AlgoFunc{}(first, last, adapted_d_first);
    }
  } else {
    return AlgoFunc{}(first, last, d_first);
  }
}

template <typename return_type, typename AlgoFunc, typename SrcIt, typename DstIt>
constexpr return_type with_bit_iterator_adapter(
    bit_iterator<SrcIt> first,
    bit_iterator<DstIt> last) {
  using dst_word_type = typename bit_iterator<DstIt>::word_type;
  using src_word_type = typename bit_iterator<SrcIt>::word_type;
  if constexpr (!std::is_same_v<src_word_type, dst_word_type> && bitsof<src_word_type>() != bitsof<dst_word_type>()) {
    if constexpr (bitsof<src_word_type>() > bitsof<dst_word_type>()) {
      bit_iterator<bit_word_pointer_adapter<DstIt, SrcIt>> adapted_first(
          bit_word_pointer_adapter<DstIt, SrcIt>(first.base(), first.position() / bitsof<dst_word_type>()), first.position() % bitsof<dst_word_type>());
      return AlgoFunc{}(adapted_first, last);
    } else {
      bit_iterator<bit_word_pointer_adapter<SrcIt, DstIt>> adapted_last(
          bit_word_pointer_adapter<SrcIt, DstIt>(last.base(), last.position() / bitsof<src_word_type>()), last.position() % bitsof<src_word_type>());
      return AlgoFunc{}(first, adapted_last);
    }
  } else {
    return AlgoFunc{}(first, last);
  }
}

// ========================================================================== //
}  // namespace bit
#endif // _BIT_DETAILS_HPP_INCLUDED
// ========================================================================== //
