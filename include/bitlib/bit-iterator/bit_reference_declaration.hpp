#ifndef _BIT_REFERENCE_DECLARATION_HPP_INCLUDED
#define _BIT_REFERENCE_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <type_traits>

#include "bitlib/bit-algorithms/bit_algorithm_forward.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_span_declaration.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_integral.hpp"
#include "bitlib/bit-iterator/bit_iterator_forward.hpp"

namespace bit {

namespace detail {
template <class StorageRef, bool SingleWord>
struct reference_word_type;

template <class StorageRef>
struct reference_word_type<StorageRef, true> {
  using storage_type = std::remove_cvref_t<StorageRef>;
  static constexpr size_t storage_size =
      sizeof(std::remove_reference_t<StorageRef>);
  using type = std::conditional_t<
      std::is_integral_v<storage_type>, exact_floor_integral_t<storage_type>,
      ceil_integral<storage_size * 8>>;
};

template <class StorageRef>
struct reference_word_type<StorageRef, false> {
  using storage_type =
      std::remove_cv_t<std::remove_reference_t<StorageRef>>;
  using type = ceil_integral<alignof(storage_type) * 8>;
};

template <class StorageRef>
using reference_span_word_type = std::conditional_t<
    std::is_const_v<std::remove_reference_t<StorageRef>>,
    std::add_const_t<typename reference_word_type<StorageRef, false>::type>,
    typename reference_word_type<StorageRef, false>::type>;

template <class StorageRef, size_t Alignment>
struct reference_state {
  using word_type = typename reference_word_type<StorageRef, true>::type;
  using mask_type = std::make_unsigned_t<word_type>;

  constexpr size_t position() const noexcept { return Alignment; }

  constexpr mask_type mask() const noexcept {
    return static_cast<mask_type>(mask_type(1) << Alignment);
  }
};

template <class StorageRef>
struct reference_state<StorageRef, std::dynamic_extent> {
  using word_type = typename reference_word_type<StorageRef, true>::type;
  using mask_type = std::make_unsigned_t<word_type>;

  mask_type mask_;

  constexpr explicit reference_state(mask_type mask) noexcept : mask_(mask) {}

  constexpr size_t position() const noexcept {
    return static_cast<size_t>(std::countr_zero(mask_));
  }

  constexpr mask_type mask() const noexcept { return mask_; }
};
}  // namespace detail

/* ********************* SINGLE-WORD, SINGLE-BIT REFERENCE ****************** */
// The primary template is the mask-based one-bit reference. All multi-bit
// references use the span-backed specialization below.
template <typename StorageRef, primitive_c ValueType, size_t Alignment>
class reference : private detail::reference_state<StorageRef, Alignment> {
 private:
  using state_type = detail::reference_state<StorageRef, Alignment>;
  using storage_type = std::remove_cvref_t<StorageRef>;
  static constexpr size_t storage_size =
      sizeof(std::remove_reference_t<StorageRef>);
  static constexpr bool writable_storage =
      !std::is_const_v<std::remove_reference_t<StorageRef>>;

  using word_type_impl = exact_floor_integral_t<storage_type>;
  using storage_pointer = std::conditional_t<
      std::is_const_v<std::remove_reference_t<StorageRef>>,
      const word_type_impl*, word_type_impl*>;

 public:
  using WordType = word_type_impl;
  using word_type = WordType;
  using const_word_type = std::add_const_t<WordType>;
  using value_type = ValueType;
  using size_type = std::size_t;
  using mask_type = std::make_unsigned_t<WordType>;

  static constexpr size_type num_words = 1;
  static constexpr size_type word_bits = sizeof(word_type) * 8;

  // Assertions
  static_assert(std::is_integral_v<word_type>,
                "storage must have an integral word representation");
  static_assert(std::is_integral_v<storage_type>,
                "single-bit references require integral word storage");
  static_assert(storage_size <= sizeof(std::uintptr_t),
                "larger storage requires the multi-word specialization");
  static_assert(storage_size == sizeof(word_type),
                "single-word storage must match its inferred word size");
  static_assert(alignof(storage_type) >= alignof(word_type),
                "single-word storage is under-aligned for its inferred word type");
  static_assert(
      ValueType::bits <= storage_size * 8,
      "reference bit width exceeds the storage capacity");
  static_assert(ValueType::bits == 1,
                "multi-bit references use the span-backed implementation");
  static_assert(
      Alignment == std::dynamic_extent ||
          (Alignment <= num_words * word_bits &&
           ValueType::bits <= num_words * word_bits - Alignment),
      "reference bit range exceeds the accessible storage");

  // Friendship
  template <class, primitive_c, size_t>
  friend class reference;

 private:
  storage_pointer _ref;

  static constexpr state_type make_state(size_type pos) noexcept;

  constexpr bool value() const noexcept;

  constexpr void assign(bool bit) const noexcept;

 public:
  // Lifecycle
  constexpr reference(const reference& other) noexcept;

  explicit constexpr reference(StorageRef& ref) noexcept;

  constexpr reference(StorageRef& ref, size_type pos) noexcept;

  // Assignment
 public:
  constexpr reference& operator=(const reference& other) const noexcept;

  template <class OtherStorage, size_type OtherAlignment>
  constexpr reference& operator=(
      const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept;

  constexpr reference& operator=(const value_type& val) const noexcept;

  constexpr reference& assign(word_type val) const noexcept;

  constexpr reference& assign(word_type val, size_type pos) const;

  // Bitwise assignment operators
 public:
  constexpr reference& operator&=(const value_type& other) const noexcept;

  constexpr reference& operator|=(const value_type& other) const noexcept;

  constexpr reference& operator^=(const value_type& other) const noexcept;

  // Comparison operators
 public:
  friend constexpr bool operator==(
      const reference& lhs,
      const reference& rhs) noexcept {
    return lhs.value() == rhs.value();
  }
  friend constexpr bool operator!=(
      const reference& lhs,
      const reference& rhs) noexcept {
    return !(lhs == rhs);
  }
  // Bitwise not operator
  friend constexpr value_type operator~(
      const reference& ref) noexcept {
    return value_type(!ref.value());
  }

  // Conversion
 public:
  explicit constexpr operator bool() const noexcept;

  constexpr operator value_type() const noexcept;

  constexpr reference& set(bool bit) const noexcept;

  constexpr reference& set() const noexcept;
  constexpr reference& reset() const noexcept;
  constexpr reference& flip() const noexcept;

  // Access
 public:
  constexpr bit_pointer<const_word_type> operator&() const noexcept;

  constexpr bit_pointer<WordType> operator&() noexcept
    requires(writable_storage);

 public:
  // Implementation details: function members
 private:
  reference() noexcept = default;
  explicit constexpr reference(word_type* ptr) noexcept;

  constexpr reference(word_type* ptr, size_type pos) noexcept;
};

namespace detail {
template <typename WordType, size_t Extent, size_t Alignment>
class reference_field_span_state {
 private:
  using span_type = bit_span<WordType, Extent, Alignment, policy::typical<std::remove_cv_t<WordType>>>;
  span_type span_;

 public:
  constexpr reference_field_span_state(WordType* words,
                                       size_t position) noexcept;

  constexpr span_type field_span() const noexcept { return span_; }
};
}  // namespace detail

/* *********************** SPAN-BACKED REFERENCE *************************** */
// Every multi-bit field and every multi-word storage object uses this
// implementation. The word count is inferred from the storage object's size
// and alignment; the fixed-extent bit_span stores the field start.
template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
class reference<StorageRef, ValueType, Alignment>
    : private detail::reference_field_span_state<
          detail::reference_span_word_type<StorageRef>,
          ValueType::bits, Alignment> {
 private:
  using storage_type = std::remove_reference_t<StorageRef>;
  using unqualified_storage_type = std::remove_cv_t<storage_type>;
  using storage_word_type =
      ceil_integral<alignof(unqualified_storage_type) * 8>;
  using storage_pointer = std::conditional_t<
      std::is_const_v<storage_type>, const storage_word_type*,
      storage_word_type*>;
  using span_word_type = std::remove_pointer_t<storage_pointer>;
  using state_type = detail::reference_field_span_state<
      span_word_type, ValueType::bits, Alignment>;

 public:
  using WordType = storage_word_type;
  using word_type = storage_word_type;
  using const_word_type = std::add_const_t<word_type>;
  using value_type = ValueType;
  using size_type = std::size_t;
  using mask_type = std::make_unsigned_t<word_type>;
  static constexpr size_type num_words = sizeof(unqualified_storage_type) / sizeof(word_type);
  static constexpr size_type word_bits = sizeof(word_type) * 8;

 private:
  static_assert(std::is_integral_v<word_type>,
                "span-backed storage must use integral words");
  static_assert(sizeof(unqualified_storage_type) % sizeof(word_type) == 0,
                "storage size must be a multiple of its inferred word size");
  static_assert(
      Alignment == std::dynamic_extent ||
          (Alignment <= num_words * word_bits &&
           ValueType::bits <= num_words * word_bits - Alignment),
      "reference bit range exceeds the accessible storage");

  template <class, primitive_c, size_t>
  friend class reference;

  constexpr auto field_span() const noexcept;

  using value_word_type = ceil_integral<alignof(value_type) * 8>;
  using const_value_span_type = bit_span<const value_word_type, ValueType::bits, Alignment, policy::typical<value_word_type>>;
  using value_span_type = bit_span<value_word_type, ValueType::bits, Alignment, policy::typical<value_word_type>>;

  static constexpr const_value_span_type value_span(
      const value_type& value) noexcept;

  static constexpr value_span_type value_span(value_type& value) noexcept;

  template <class OtherStorage, size_type OtherAlignment>
  constexpr void copy_from(
      const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept;

  template <class SourceSpan, class DestinationSpan>
  constexpr void copy_spans(const SourceSpan& source,
                            const DestinationSpan& destination) const noexcept;

  constexpr bool field_any() const noexcept;

  template <char operation>
  constexpr void combine_with(const value_type& other) const noexcept;

  constexpr value_type materialize(bool invert = false) const noexcept;

 public:
  constexpr reference(const reference& other) noexcept;

  explicit constexpr reference(StorageRef& ref) noexcept;

  constexpr reference(StorageRef& ref, size_type pos) noexcept
    requires(Alignment == std::dynamic_extent);

  constexpr reference& operator=(const reference& other) const noexcept;

  template <class OtherStorage, size_t OtherAlignment>
  constexpr reference& operator=(
      const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept;

  constexpr reference& operator=(const value_type& value) const noexcept;

  constexpr reference& operator&=(const value_type& other) const noexcept;

  constexpr reference& operator|=(const value_type& other) const noexcept;

  constexpr reference& operator^=(const value_type& other) const noexcept;

  friend constexpr bool operator==(const reference& lhs,
                                   const reference& rhs) noexcept {
    const auto lhs_span = lhs.field_span();
    const auto rhs_span = rhs.field_span();
    return ::bit::equal(lhs_span.begin(), lhs_span.end(), rhs_span.begin());
  }
  friend constexpr bool operator!=(const reference& lhs,
                                   const reference& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend constexpr value_type operator~(const reference& ref) noexcept {
    return ref.materialize(true);
  }

  explicit constexpr operator bool() const noexcept;

  constexpr operator value_type() const noexcept;

  constexpr bit_pointer<const_word_type> operator&() const noexcept;

  constexpr bit_pointer<word_type> operator&() noexcept
    requires(!std::is_const_v<storage_type>);
};

// Stream functions
template <class CharT, class Traits, class T>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is, reference<T>& x);
template <class CharT, class Traits, class T>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os, reference<T> x);

}  // namespace bit

#endif  // _BIT_REFERENCE_DECLARATION_HPP_INCLUDED
