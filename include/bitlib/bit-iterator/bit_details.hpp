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
// clang-format off
#if defined(_MSC_VER)
  #if __has_include(<intrin.h>)
    #include <intrin.h>
  #else
    #define NO_X86_INTRINSICS
  #endif
#else
  #if __has_include(<x86intrin.h>)
      #include <x86intrin.h>
  #else
    #if __has_include(<immintrin.h>)
      #include <immintrin.h>
    #else
      #define NO_X86_INTRINSICS
    #endif
  #endif
#endif  // defined(_MSC_VER)
// clang-format on

#include <algorithm>
#include <bit>
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

/*
exact_floor_integral is used to determine the exact integral type that a proxy reference
can be implicitly converted to.
If the proxy reference supports multiple types, it will pick the smallest, preferring unsigned.
*/
template <typename T>
struct exact_floor_integral {
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
  using type =
      std::conditional_t<
          is_exactly_convertible<U, uint8_t>(), uint8_t,
          std::conditional_t<
              is_exactly_convertible<U, int8_t>(), int8_t,
              std::conditional_t<
                  is_exactly_convertible<U, uint16_t>(), uint16_t,
                  std::conditional_t<
                      is_exactly_convertible<U, int16_t>(), int16_t,
                      std::conditional_t<
                          is_exactly_convertible<U, uint32_t>(), uint32_t,
                          std::conditional_t<
                              is_exactly_convertible<U, int32_t>(), int32_t,
                              std::conditional_t<
                                  is_exactly_convertible<U, uint64_t>(), uint64_t,
                                  std::conditional_t<
                                      is_exactly_convertible<U, int64_t>(), int64_t,
                                      void>>>>>>>>;
};

// Helper alias
template <typename T>
using exact_floor_integral_t = typename exact_floor_integral<T>::type;

template <typename From, typename To, typename = void>
struct is_static_castable : std::false_type {};

template <typename From, typename To>
struct is_static_castable<From, To, std::void_t<decltype(static_cast<To>(std::declval<From>()))>>
    : std::true_type {};

template <typename From, typename To>
constexpr bool is_static_castable_v = is_static_castable<From, To>::value;

/* ***************************** BINARY DIGITS ****************************** */
// Binary digits structure definition
// Implementation template: only instantiates static_asserts for non-byte types.
template <typename T, bool = std::is_same<T, std::byte>::value>
struct binary_digits_impl : std::integral_constant<std::size_t, std::numeric_limits<std::make_unsigned_t<exact_floor_integral_t<T>>>::digits> {
  static_assert(std::is_integral<exact_floor_integral_t<T>>::value, "Type must be integral");
  //static_assert(std::is_unsigned<T>::value, "Type must be unsigned");
  static_assert(!std::is_same<exact_floor_integral_t<T>, bool>::value, "Type must not be bool");
  static_assert(!std::is_same<exact_floor_integral_t<T>, char>::value, "Type must not be char");
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

/* ******************* IMPLEMENTATION DETAILS: UTILITIES ******************** */
// Assertions
template <class Iterator>
constexpr bool _assert_range_viability(Iterator first, Iterator last);
/* ************************************************************************** */

// Bit field extraction
template <class T, class = decltype(__builtin_ia32_bextr_u64(T(), T(), T()))>
constexpr T _bextr(T src, T start, T len) noexcept;
template <class T, class... X>
constexpr T _bextr(T src, T start, T len, X...) noexcept;

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

// Double precision shift left
template <class T>
constexpr T _shld(T dst, T src, T cnt) noexcept;

// Double precision shift right
template <class T>
constexpr T _shrd(T dst, T src, T cnt) noexcept;

// Multiword multiply
template <typename T, typename T128 = decltype(__uint128_t(T()))>
constexpr T _mulx(T src0, T src1, T* hi) noexcept;
template <typename T, typename... X>
constexpr T _mulx(T src0, T src1, T* hi, X...) noexcept;
template <typename T, typename T128 = decltype(__uint128_t(T()))>
constexpr T _divx(const T& numerator_hi, const T& numerator_lo, const T& denominator, T* remainder) noexcept;
template <typename T, typename... X>
constexpr T _divx(const T& numerator_hi, const T& numerator_lo, const T& denominator, T* remainder, X...) noexcept;
/* ************************************************************************** */

/*
Logical shift right
*/
template <std::integral T, typename size_type = size_t>
constexpr T lsr(const T val, const size_type shift) {
#ifdef BITLIB_DETECT_UNDEFINED_SHIFT
  assert(static_cast<size_t>(shift) < bitsof<T>());
#endif
  return static_cast<T>(static_cast<std::make_unsigned_t<T>>(val) >> shift);
}

/*
Logic shift right when `val` operand is a proxy reference
*/
template <typename T, typename size_type = size_t>
constexpr exact_floor_integral_t<T> lsr(const T val, const size_type shift) {
  static_assert(!std::is_same_v<exact_floor_integral_t<T>, void>,
                "Type T must be convertible to an integral type");
#ifdef BITLIB_DETECT_UNDEFINED_SHIFT
  assert(static_cast<size_t>(shift) < bitsof<exact_floor_integral_t<T>>());
#endif
  return static_cast<exact_floor_integral_t<T>>(static_cast<std::make_unsigned_t<exact_floor_integral_t<T>>>(val) >> shift);
}

/*
Logical shift left
*/
template <std::integral T, typename size_type = size_t>
constexpr T lsl(const T val, const size_type shift) {
#ifdef BITLIB_DETECT_UNDEFINED_SHIFT
  assert(static_cast<size_t>(shift) < bitsof<T>());
#endif
  return static_cast<T>(static_cast<std::make_unsigned_t<T>>(val) << shift);
}

/*
Logic shift left when `val` operand is a proxy reference
*/
template <typename T, typename size_type = size_t>
constexpr exact_floor_integral_t<T> lsl(const T val, const size_type shift) {
  static_assert(!std::is_same_v<exact_floor_integral_t<T>, void>,
                "Type T must be convertible to an integral type");
#ifdef BITLIB_DETECT_UNDEFINED_SHIFT
  assert(static_cast<size_t>(shift) < bitsof<exact_floor_integral_t<T>>());
#endif
  return static_cast<exact_floor_integral_t<T>>(static_cast<std::make_unsigned_t<exact_floor_integral_t<T>>>(val) << shift);
}

enum class _mask_len {
  unknown,
  in_range
};

template <std::integral T, _mask_len len_in_range = _mask_len::in_range, typename size_type = size_t>
constexpr T _mask(const size_type len) {
  using unsigned_t = std::make_unsigned_t<T>;
  constexpr unsigned_t one = unsigned_t(1);
  if constexpr (len_in_range != _mask_len::unknown) {
#ifdef BITLIB_DETECT_UNDEFINED_SHIFT
    assert(static_cast<size_t>(len) < bitsof<T>());
#endif
    return static_cast<T>((one << static_cast<unsigned_t>(len)) - one);
  } else {
    // The digits_mask is solely here to prevent Undefined Sanitizer
    // complaining about shift of len >= digits
    // Note: on -O1 the (len & digits_mask) is optimized to simply (len)
    constexpr unsigned_t digits_mask = static_cast<unsigned_t>(bitsof<T>()) - one;
    return static_cast<T>((one << (static_cast<unsigned_t>(len) & digits_mask)) * static_cast<unsigned_t>(len < bitsof<T>()) - one);
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

// ------------- IMPLEMENTATION DETAILS: INSTRUCTIONS: BIT SWAP ------------- //
// Reverses the order of the bits with or without of compiler intrinsics
template <class T>
constexpr T _bitswap(T src) noexcept {
  static_assert(binary_digits<T>::value, "");
  using byte_t = unsigned char;
  constexpr T digits = binary_digits<T>::value;
  constexpr unsigned long long int first = 0x80200802ULL;
  constexpr unsigned long long int second = 0x0884422110ULL;
  constexpr unsigned long long int third = 0x0101010101ULL;
  constexpr unsigned long long int fourth = 32;
  constexpr bool is_size1 = sizeof(T) == 1;
  constexpr bool is_byte = digits == std::numeric_limits<byte_t>::digits;
  constexpr bool is_octet = std::numeric_limits<byte_t>::digits == 8;
  constexpr bool is_pow2 = std::has_single_bit(static_cast<std::make_unsigned_t<T>>(digits));
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
constexpr exact_floor_integral_t<T> _bitblend(
    const T src0_,
    const U src1_,
    const exact_floor_integral_t<T> msk) noexcept
  requires(std::is_same_v<exact_floor_integral_t<T>, exact_floor_integral_t<U>>)
{
  static_assert(binary_digits<exact_floor_integral_t<T>>::value, "");
  const exact_floor_integral_t<T> src0 = static_cast<exact_floor_integral_t<T>>(src0_);
  const exact_floor_integral_t<U> src1 = static_cast<exact_floor_integral_t<U>>(src1_);
  return src0 ^ ((src0 ^ src1) & msk);
}

// Replaces len bits of src0 by the ones of src1 starting at start
template <typename T, typename U>
constexpr exact_floor_integral_t<T> _bitblend(
    const T src0_,
    const U src1_,
    const exact_floor_integral_t<T> start,
    const exact_floor_integral_t<T> len) noexcept
  requires(std::is_same_v<exact_floor_integral_t<T>, exact_floor_integral_t<U>>)
{
  using resolved_t = exact_floor_integral_t<T>;
  using promoted_t = std::conditional_t<bitsof<resolved_t>() < bitsof<int>(), int, resolved_t>;
  static_assert(binary_digits<exact_floor_integral_t<T>>::value, "");
  constexpr resolved_t digits = static_cast<resolved_t>(bitsof<resolved_t>());
  const promoted_t src0 = static_cast<promoted_t>(src0_);
  const promoted_t src1 = static_cast<promoted_t>(src1_);
  const promoted_t msk = _mask<promoted_t, _mask_len::unknown>(len) << static_cast<promoted_t>(start);
  return static_cast<resolved_t>(src0 ^ ((src0 ^ src1) & (msk * (start < digits))));
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

// --- IMPLEMENTATION DETAILS: INSTRUCTIONS: DOUBLE PRECISION SHIFT LEFT ---- //
// Left shifts dst by cnt bits, filling the lsbs of dst by the msbs of src
template <class T>
constexpr T _shld(T dst, T src, T cnt) noexcept {
  static_assert(binary_digits<T>::value, "");
  constexpr T digits = binary_digits<T>::value;
  if (cnt < digits) {
    dst = lsl(dst, cnt) | (lsr(src, (digits - cnt)));
  } else {
    dst = lsl(src, cnt - digits) * (cnt < (digits + digits));
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
    dst = (lsr(dst, cnt)) | lsl(src, (digits - cnt));
  } else {
    dst = (lsr(src, (cnt - digits))) * (cnt < (digits + digits));
  }
  return dst;
}
// -------------------------------------------------------------------------- //

#if defined(NO_X86_INTRINSICS)
template <bool Add, std::integral U>
static inline unsigned char add_carry_sub_borrow(unsigned char c_in, U a, U b, U* out) noexcept {
  static_assert(std::is_unsigned_v<U>, "Only unsigned types are supported");

  if constexpr (Add) {
    U sum1 = a + b;
    U sum2 = sum1 + static_cast<U>(c_in);
    *out = sum2;

    // Carry occurs if either sum1 overflows a+b or sum2 overflows sum1+carry
    return static_cast<unsigned char>((sum1 < a) || (sum2 < sum1));
  } else {
    U diff1 = a - b;
    U diff2 = diff1 - static_cast<U>(c_in);
    *out = diff2;

    // Borrow occurs if a < b or a - b < carry_in
    return static_cast<unsigned char>((a < b) || (diff1 < c_in));
  }
}
#else
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
    return ADDCARRYSUBBORROW32<Add>(c_in, static_cast<uint32_t>(a), static_cast<uint32_t>(b), reinterpret_cast<uint32_t*>(out));
  } else if constexpr (64 == bitsof<U>()) {
    return ADDCARRYSUBBORROW64<Add>(c_in, static_cast<uint64_t>(a), static_cast<uint64_t>(b), reinterpret_cast<uint64_t*>(out));
  } else if constexpr (0 == (bitsof<U>() % 64)) {
    using t64 = std::conditional<std::is_signed_v<U>, int64_t, uint64_t>;
    unsigned char carry;
    for (int i = 0; i < (bitsof<U>() / 64); i++) {
      carry = ADDCARRYSUBBORROW64<Add>(c_in, static_cast<t64>(a >> (i * 64)), static_cast<t64>(b >> (i * 64)), reinterpret_cast<t64*>(out) + i);
    }
    return carry;
  } else {
    assert(((void)"add carry intrinsics support only support powers of 2 bits", false));
  }
}
#endif  // NO_X86_INTRINSICS

template <std::integral U>
static inline unsigned char add_carry(unsigned char c_in, U a, U b, U* out) noexcept {
  return add_carry_sub_borrow<true, U>(c_in, a, b, out);
}

template <std::integral U>
static inline unsigned char sub_borrow(unsigned char c_in, U a, U b, U* out) noexcept {
  return add_carry_sub_borrow<false, U>(c_in, a, b, out);
}

// -------------------------------------------------------------------------- //

template <typename T, typename T128>
constexpr T _divx(const T& numerator_hi, const T& numerator_lo, const T& denominator, T* remainder) noexcept {
  constexpr auto digits = bitsof<T>();
  if constexpr (digits > bitsof<uint32_t>()) {
    T128 tmp128 = (static_cast<T128>(numerator_hi) << digits) | static_cast<T128>(numerator_lo);
    T quotient = static_cast<T>(tmp128 / static_cast<T128>(denominator));
    if (remainder) {
      *remainder = static_cast<T>(tmp128 % static_cast<T128>(denominator));
    }
    return quotient;
  } else {
    return _divx(numerator_hi, numerator_lo, denominator, remainder, std::ignore);
  }
}

// This code is from a 'public domain' post of ridiculousfish:
// https://ridiculousfish.com/blog/posts/labor-of-division-episode-v.html
template <typename T, typename... X>
constexpr T _divx(const T& numerator_hi, const T& numerator_lo, const T& denominator, T* remainder, X...) noexcept {
  constexpr auto digits = bitsof<T>();
  using wider_t = ceil_integral<digits + digits>;
  if constexpr ((digits + digits) <= binary_digits<wider_t>::value) {
    wider_t tmp = (static_cast<wider_t>(numerator_hi) << digits) | static_cast<wider_t>(numerator_lo);
    T quotient = static_cast<T>(tmp / static_cast<wider_t>(denominator));
    if (remainder) {
      *remainder = static_cast<T>(tmp % static_cast<wider_t>(denominator));
    }
    return quotient;
  } else if constexpr (digits > bitsof<uint32_t>()) {
    T numhi = numerator_hi;
    T numlo = numerator_lo;
    T den = denominator;
    // We work in base 2**32.
    // A uint32 holds a single digit. A uint64 holds two digits.
    // Our numerator is conceptually [num3, num2, num1, num0].
    // Our denominator is [den1, den0].
    const uint64_t b = (1ull << 32);

    // The high and low digits of our computed quotient.
    uint32_t q1;
    uint32_t q0;

    // The normalization shift factor.
    int shift;

    // The high and low digits of our denominator (after normalizing).
    // Also the low 2 digits of our numerator (after normalizing).
    uint32_t den1;
    uint32_t den0;
    uint32_t num1;
    uint32_t num0;

    // A partial remainder.
    uint64_t rem;

    // The estimated quotient, and its corresponding remainder (unrelated to true remainder).
    uint64_t qhat;
    uint64_t rhat;

    // Variables used to correct the estimated quotient.
    uint64_t c1;
    uint64_t c2;

    // Check for overflow and divide by 0.
    if (numhi >= den) {
      if (remainder != nullptr) {
        *remainder = ~0ull;
      }
      return ~0ull;
    }

    // Determine the normalization factor. We multiply den by this, so that its leading digit is at
    // least half b. In binary this means just shifting left by the number of leading zeros, so that
    // there's a 1 in the MSB.
    // We also shift numer by the same amount. This cannot overflow because numhi < den.
    // The expression (-shift & 63) is the same as (64 - shift), except it avoids the UB of shifting
    // by 64. The funny bitwise 'and' ensures that numlo does not get shifted into numhi if shift is 0.
    // clang 11 has an x86 codegen bug here: see LLVM bug 50118. The sequence below avoids it.
    shift = std::countl_zero(den);
    den <<= shift;
    numhi <<= shift;
    numhi |= (numlo >> (-shift & 63)) & (-static_cast<int64_t>(shift) >> 63);
    numlo <<= shift;

    // Extract the low digits of the numerator and both digits of the denominator.
    num1 = static_cast<uint32_t>(numlo >> 32);
    num0 = static_cast<uint32_t>(numlo & 0xFFFFFFFFu);
    den1 = static_cast<uint32_t>(den >> 32);
    den0 = static_cast<uint32_t>(den & 0xFFFFFFFFu);

    // We wish to compute q1 = [n3 n2 n1] / [d1 d0].
    // Estimate q1 as [n3 n2] / [d1], and then correct it.
    // Note while qhat may be 2 digits, q1 is always 1 digit.
    qhat = numhi / den1;
    rhat = numhi % den1;
    c1 = qhat * den0;
    c2 = rhat * b + num1;
    if (c1 > c2) {
      qhat -= (c1 - c2 > den) ? 2 : 1;
    }
    q1 = static_cast<uint32_t>(qhat);

    // Compute the true (partial) remainder.
    rem = numhi * b + num1 - q1 * den;

    // We wish to compute q0 = [rem1 rem0 n0] / [d1 d0].
    // Estimate q0 as [rem1 rem0] / [d1] and correct it.
    qhat = rem / den1;
    rhat = rem % den1;
    c1 = qhat * den0;
    c2 = rhat * b + num0;
    if (c1 > c2) {
      qhat -= (c1 - c2 > den) ? 2 : 1;
    }
    q0 = static_cast<uint32_t>(qhat);

    // Return remainder if requested.
    if (remainder != nullptr) {
      *remainder = (rem * b + num0 - q0 * den) >> shift;
    }
    return (static_cast<uint64_t>(q1) << 32) | q0;
  } else {
    assert(false);
  }
}

// -------- IMPLEMENTATION DETAILS: INSTRUCTIONS: MULTIWORD MULTIPLY -------- //
// Multiplies src0 and src1 and gets the full result with compiler intrinsics
template <typename T, typename T128>
constexpr T _mulx(T src0, T src1, T* hi) noexcept {
  constexpr auto digits = bitsof<T>();
  if constexpr (digits > bitsof<uint32_t>()) {
    T128 tmp128 = static_cast<T128>(src0) * static_cast<T128>(src1);
    *hi = static_cast<T>(tmp128 >> digits);
    return static_cast<T>(tmp128);
  } else {
    return _mulx(src0, src1, hi, std::ignore);
  }
}

template <typename T, typename... X>
constexpr T _mulx(T src0, T src1, T* hi, X...) noexcept {
  constexpr T digits = bitsof<T>();
  using wider_t = ceil_integral<bitsof<T>() + bitsof<T>()>;
  if constexpr ((digits + digits) <= bitsof<wider_t>()) {
    wider_t tmp = static_cast<wider_t>(src0) * static_cast<wider_t>(src1);
    *hi = tmp >> digits;
    return static_cast<T>(tmp);
  } else {
    // Multiplies src0 and src1 and gets the full result without compiler intrinsics
    constexpr T offset = digits / 2;
    const T lsbs0 = src0 & _mask<T>(digits - offset);
    const T msbs0 = lsr(src0, offset);
    const T lsbs1 = src1 & _mask<T>(digits - offset);
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
}
// -------------------------------------------------------------------------- //

template <typename AlgoFunc, std::random_access_iterator SrcIt, std::random_access_iterator DstIt>
constexpr auto with_bit_iterator_adapter(
    const bit_iterator<SrcIt>& first,
    const bit_iterator<SrcIt>& last,
    const bit_iterator<DstIt>& d_first) {
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

template <typename AlgoFunc, std::random_access_iterator SrcIt, std::random_access_iterator DstIt>
constexpr auto with_bit_iterator_adapter(
    const SrcIt& first,
    const SrcIt& last,
    const bit_iterator<DstIt>& d_first) {
  return with_bit_iterator_adapter<AlgoFunc>(bit_iterator(first), bit_iterator(last), d_first);
}

template <typename AlgoFunc, std::random_access_iterator SrcIt, std::random_access_iterator DstIt>
constexpr auto with_bit_iterator_adapter(
    const bit_iterator<SrcIt>& first,
    const bit_iterator<SrcIt>& last,
    const DstIt& d_first) {
  return with_bit_iterator_adapter<AlgoFunc>(first, last, bit_iterator(d_first));
}

template <typename AlgoFunc, std::random_access_iterator SrcIt, std::random_access_iterator DstIt>
constexpr auto with_bit_iterator_adapter(
    const bit_iterator<SrcIt>& first,
    const bit_iterator<DstIt>& last) {
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

template <typename AlgoFunc, std::random_access_iterator SrcIt, std::random_access_iterator DstIt>
constexpr auto with_bit_iterator_adapter(
    const SrcIt& first,
    const bit_iterator<DstIt>& last) {
  return with_bit_iterator_adapter<AlgoFunc>(bit_iterator(first), last);
}

template <typename AlgoFunc, std::random_access_iterator SrcIt, std::random_access_iterator DstIt>
constexpr auto with_bit_iterator_adapter(
    const bit_iterator<SrcIt>& first,
    const DstIt& last) {
  return with_bit_iterator_adapter<AlgoFunc>(first, bit_iterator(last));
}

namespace detail {

struct uninitialized_t {
  explicit uninitialized_t() = default;
};
inline constexpr uninitialized_t uninitialized{};

struct initialized_t {
  explicit initialized_t() = default;
};
inline constexpr initialized_t initialized{};

template <typename size_type, bool resizeable, std::size_t Extent>
struct container_size_storage {
  constexpr size_type size() const noexcept {
    return Extent;
  }

  constexpr container_size_storage() noexcept {}
};

template <typename size_type, bool resizeable>
struct container_size_storage<size_type, resizeable, std::dynamic_extent> {
  using maybe_const_size_type = std::conditional_t<resizeable, size_type, std::add_const_t<size_type>>;

  maybe_const_size_type size_;
  constexpr size_type size() const noexcept {
    return size_;
  }
  constexpr void resize(const size_type& new_size)
    requires(resizeable)
  {
    size_ = new_size;
  }

  constexpr container_size_storage() noexcept : size_() {}
  constexpr container_size_storage(const size_type& size) noexcept
      : size_(size) {}
};

}  // namespace detail

// ========================================================================== //
}  // namespace bit
#endif // _BIT_DETAILS_HPP_INCLUDED
       // ========================================================================== //