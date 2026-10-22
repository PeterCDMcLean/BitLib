#ifndef _BIT_REFERENCE_DEFINITION_HPP_INCLUDED
#define _BIT_REFERENCE_DEFINITION_HPP_INCLUDED

#include <bit>
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>

#include "bitlib/bit-algorithms/bit_algorithm_details.hpp"
#include "bitlib/bit-algorithms/bit_algorithm_forward.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_span_declaration.hpp"
#include "bitlib/bit-iterator/bit_reference_declaration.hpp"
#include "bitlib/bit-iterator/bit_value_definition.hpp"

namespace bit {

/* ********************* SINGLE-WORD, SINGLE-BIT REFERENCE ****************** */

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr typename reference<StorageRef, ValueType, Alignment>::state_type
reference<StorageRef, ValueType, Alignment>::make_state(size_type pos) noexcept {
  if constexpr (Alignment == std::dynamic_extent) {
    return state_type{pos < word_bits
                          ? static_cast<mask_type>(mask_type(1) << pos)
                          : mask_type(0)};
  } else {
    return state_type{};
  }
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr bool reference<StorageRef, ValueType, Alignment>::value() const noexcept {
  return (*_ref & this->mask()) != mask_type(0);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr void reference<StorageRef, ValueType, Alignment>::assign(bool bit) const noexcept {
  if (bit) {
    *_ref = static_cast<word_type>(*_ref | this->mask());
  } else {
    *_ref = static_cast<word_type>(*_ref & ~this->mask());
  }
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::reference(const reference& other) noexcept
    : state_type(other), _ref(other._ref) {}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
/* explicit */ constexpr reference<StorageRef, ValueType, Alignment>::reference(StorageRef& ref) noexcept
    : state_type(make_state(0)),
      _ref(reinterpret_cast<storage_pointer>(std::addressof(ref))) {}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::reference(StorageRef& ref, size_type pos) noexcept
    : state_type(make_state(pos)),
      _ref(reinterpret_cast<storage_pointer>(std::addressof(ref))) {
  const size_type capacity = bitsof<word_type>();
  assert(pos <= capacity && ValueType::bits <= capacity - pos);
  if constexpr (Alignment != std::dynamic_extent) {
    assert(pos == Alignment);
  }
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(const reference& other) const noexcept {
  assign(other.value());
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
template <class OtherStorage, std::size_t OtherAlignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(
    const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept {
  assign(static_cast<bool>(other));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(const value_type& val) const noexcept {
  assign(static_cast<bool>(val));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::assign(word_type val) const noexcept {
  assign((val & word_type(1)) != 0);
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::assign(word_type val, size_type pos) const {
  assert(pos < word_bits);
  assign(((val >> pos) & word_type(1)) != 0);
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator&=(const value_type& other) const noexcept {
  assign(value() && static_cast<bool>(other));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator|=(const value_type& other) const noexcept {
  assign(value() || static_cast<bool>(other));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator^=(const value_type& other) const noexcept {
  assign(value() != static_cast<bool>(other));
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::operator bool() const noexcept {
  return value();
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::operator value_type() const noexcept {
  return value_type(value());
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::set(bool bit) const noexcept {
  assign(bit);
  return const_cast<reference&>(*this);
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::set() const noexcept { return set(true); }

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::reset() const noexcept { return set(false); }

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::flip() const noexcept { return set(!value()); }

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr bit_pointer<typename reference<StorageRef, ValueType, Alignment>::const_word_type>
reference<StorageRef, ValueType, Alignment>::operator&() const noexcept {
  return bit_pointer<const_word_type>(_ref, this->position());
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr bit_pointer<typename reference<StorageRef, ValueType, Alignment>::WordType>
reference<StorageRef, ValueType, Alignment>::operator&() noexcept
  requires(writable_storage)
{
  return bit_pointer<WordType>(_ref, this->position());
}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
/* explicit */ constexpr reference<StorageRef, ValueType, Alignment>::reference(word_type* ptr) noexcept
    : state_type(make_state(0)), _ref(ptr) {}

template <typename StorageRef, primitive_c ValueType, size_t Alignment>
constexpr reference<StorageRef, ValueType, Alignment>::reference(word_type* ptr, size_type pos) noexcept
    : state_type(make_state(pos)), _ref(ptr) {
  const size_type capacity = bitsof<word_type>();
  assert(pos <= capacity && ValueType::bits <= capacity - pos);
}

/* *********************** SPAN-BACKED REFERENCE *************************** */

namespace detail {

template <typename WordType, size_t Extent, size_t Alignment>
constexpr reference_field_span_state<WordType, Extent, Alignment>::reference_field_span_state(
    WordType* words,
    size_t position) noexcept
    : span_(bit_pointer<WordType>(
          words + position / bitsof<WordType>(),
          position % bitsof<WordType>())) {}

}  // namespace detail

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr auto reference<StorageRef, ValueType, Alignment>::field_span() const noexcept {
  return state_type::field_span();
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr typename reference<StorageRef, ValueType, Alignment>::const_value_span_type
reference<StorageRef, ValueType, Alignment>::value_span(
    const value_type& value) noexcept {
  static_assert(sizeof(value_type) % sizeof(value_word_type) == 0);
  static_assert(ValueType::bits <= sizeof(value_type) * 8);
  const auto* words = reinterpret_cast<const value_word_type*>(
      std::addressof(value));
  return const_value_span_type(words);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr typename reference<StorageRef, ValueType, Alignment>::value_span_type
reference<StorageRef, ValueType, Alignment>::value_span(value_type& value) noexcept {
  static_assert(sizeof(value_type) % sizeof(value_word_type) == 0);
  static_assert(ValueType::bits <= sizeof(value_type) * 8);
  auto* words =
      reinterpret_cast<value_word_type*>(std::addressof(value));
  return value_span_type(words);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
template <class OtherStorage, std::size_t OtherAlignment>
constexpr void reference<StorageRef, ValueType, Alignment>::copy_from(
    const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept {
  const auto destination = field_span();
  using source_reference = reference<OtherStorage, ValueType, OtherAlignment>;
  if constexpr (source_reference::num_words == 1) {
    using source_word_type = typename source_reference::word_type;
    using source_span_type = bit_span<const source_word_type, ValueType::bits, OtherAlignment,
                                      policy::typical<source_word_type>>;
    copy_spans(source_span_type(&other), destination);
  } else {
    copy_spans(other.field_span(), destination);
  }
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
template <class SourceSpan, class DestinationSpan>
constexpr void reference<StorageRef, ValueType, Alignment>::copy_spans(
    const SourceSpan& source,
    const DestinationSpan& destination) const noexcept {
  const auto source_first = source.begin();
  const auto destination_first = destination.begin();
  const auto* source_base = source_first.base();
  const auto* destination_base = destination_first.base();

  if (static_cast<const void*>(source_base) ==
          static_cast<const void*>(destination_base) &&
      source_first.position() == destination_first.position()) {
    return;
  }

  const bool destination_after_source =
      std::less<const void*>{}(static_cast<const void*>(source_base),
                               static_cast<const void*>(destination_base)) ||
      (static_cast<const void*>(source_base) ==
           static_cast<const void*>(destination_base) &&
       destination_first.position() > source_first.position());
  if (destination_after_source) {
    ::bit::copy_backward(source_first, source.end(), destination.end());
  } else {
    ::bit::copy(source_first, source.end(), destination_first);
  }
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr bool reference<StorageRef, ValueType, Alignment>::field_any() const noexcept {
  const auto span = field_span();
  return ::bit::find(span.begin(), span.end(), bit1) != span.end();
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
template <char operation>
constexpr void reference<StorageRef, ValueType, Alignment>::combine_with(const value_type& other) const noexcept {
  if constexpr (ValueType::bits == 1) {
    const auto field = field_span();
    const bool lhs = static_cast<bool>(*field.begin());
    const bool rhs = static_cast<bool>(other);
    const bool result = operation == '&'   ? lhs && rhs
                        : operation == '|' ? lhs || rhs
                                           : lhs != rhs;
    *field.begin() = result ? bit1 : bit0;
  } else {
    const auto field = field_span();
    const auto value = value_span(other);
    const auto destination = field.begin();
    const auto source = value.begin();
    size_type offset = 0;
    while (offset < ValueType::bits) {
      const size_type available = word_bits -
                                  (destination.position() + offset) % word_bits;
      const size_type remaining = ValueType::bits - offset;
      const size_type count = remaining < available ? remaining : available;
      const mask_type lhs = get_masked_word<mask_type>(destination + offset,
                                                       count);
      const mask_type rhs = get_masked_word<mask_type>(source + offset, count);
      const mask_type result = operation == '&'   ? lhs & rhs
                               : operation == '|' ? lhs | rhs
                                                  : lhs ^ rhs;
      write_word(result, destination + offset, count);
      offset += count;
    }
  }
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr typename reference<StorageRef, ValueType, Alignment>::value_type
reference<StorageRef, ValueType, Alignment>::materialize(bool invert) const noexcept {
  if constexpr (ValueType::bits == 1) {
    return value_type(field_any() != invert);
  } else {
    value_type result{};
    const auto source = field_span();
    const auto destination = value_span(result);
    ::bit::copy(source.begin(), source.end(), destination.begin());
    if (invert) {
      for (size_type i = 0; i < ValueType::bits; ++i) {
        destination[i] = static_cast<bool>(destination[i]) ? bit0 : bit1;
      }
    }
    return result;
  }
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>::reference(const reference& other) noexcept
    : state_type(other) {}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
/* explicit */ constexpr reference<StorageRef, ValueType, Alignment>::reference(StorageRef& ref) noexcept
    : state_type(reinterpret_cast<storage_pointer>(std::addressof(ref)),
                 Alignment == std::dynamic_extent ? 0 : Alignment) {}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>::reference(StorageRef& ref, size_type pos) noexcept
  requires(Alignment == std::dynamic_extent)
    : state_type(reinterpret_cast<storage_pointer>(std::addressof(ref)),
                 pos) {
  assert(pos <= num_words * word_bits &&
         ValueType::bits <= num_words * word_bits - pos);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(const reference& other) const noexcept {
  copy_from(other);
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
template <class OtherStorage, std::size_t OtherAlignment>
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(
    const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept {
  copy_from(other);
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator=(const value_type& value) const noexcept {
  const auto destination = field_span();
  if constexpr (ValueType::bits == 1) {
    *destination.begin() = value;
  } else {
    const auto source = value_span(value);
    ::bit::copy(source.begin(), source.end(), destination.begin());
  }
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator&=(const value_type& other) const noexcept {
  combine_with<'&'>(other);
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator|=(const value_type& other) const noexcept {
  combine_with<'|'>(other);
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>&
reference<StorageRef, ValueType, Alignment>::operator^=(const value_type& other) const noexcept {
  combine_with<'^'>(other);
  return const_cast<reference&>(*this);
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>::operator bool() const noexcept {
  return field_any();
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr reference<StorageRef, ValueType, Alignment>::operator value_type() const noexcept {
  return materialize();
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr bit_pointer<typename reference<StorageRef, ValueType, Alignment>::const_word_type>
reference<StorageRef, ValueType, Alignment>::operator&() const noexcept {
  const auto first = field_span().begin();
  return bit_pointer<const_word_type>(first.base(), first.position());
}

template <class StorageRef, primitive_c ValueType, size_t Alignment>
  requires(
      !std::is_pointer_v<std::remove_reference_t<StorageRef>> &&
      (ValueType::bits > 1 ||
       !std::is_integral_v<
           std::remove_cv_t<std::remove_reference_t<StorageRef>>> ||
       sizeof(std::remove_cv_t<std::remove_reference_t<StorageRef>>) >
           sizeof(std::uintptr_t)))
constexpr bit_pointer<typename reference<StorageRef, ValueType, Alignment>::word_type>
reference<StorageRef, ValueType, Alignment>::operator&() noexcept
  requires(!std::is_const_v<storage_type>)
{
  const auto first = field_span().begin();
  return bit_pointer<word_type>(first.base(), first.position());
}

// Alias: bit_reference remains as an alias to the current form, i.e. a
// single-bit reference with the default storage reference type. It is defined
// here because forming reference<> requires primitive<1> to be complete.
using bit_reference = reference<>;

static_assert(bit_reference_c<reference<uint8_t&>>,
              "reference does not satisfy bit_reference_c concept!");

// Stream functions
template <class CharT, class Traits, class T>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    reference<T>& x) {
  using stream_type = std::basic_istream<CharT, Traits>;
  using traits_type = typename stream_type::traits_type;
  using ios_base = typename stream_type::ios_base;
  constexpr char zero = '0';
  constexpr char one = '1';
  constexpr typename stream_type::int_type eof = traits_type::eof();
  typename ios_base::iostate state = ios_base::goodbit;
  typename stream_type::char_type char_value = 0;
  typename stream_type::int_type int_value = 0;
  typename stream_type::sentry sentry(is);
  bool ok = false;
  bit_value tmp = x;
  if (sentry) {
    try {
      int_value = is.rdbuf()->sbumpc();
      if (traits_type::eq_int_type(int_value, eof)) {
        state |= ios_base::eofbit;
      } else {
        char_value = traits_type::to_char_type(int_value);
        if (traits_type::eq(char_value, is.widen(zero))) {
          tmp = bit_value(false);
          ok = true;
        } else if (traits_type::eq(char_value, is.widen(one))) {
          tmp = bit_value(true);
          ok = true;
        } else {
          int_value = is.rdbuf()->sputbackc(char_value);
          if (traits_type::eq_int_type(int_value, eof)) {
            state |= ios_base::failbit;
          }
        }
      }
    } catch (...) {
      is.setstate(ios_base::badbit);
    }
  }
  if (ok) {
    x = tmp;
  } else {
    state |= ios_base::failbit;
  }
  state ? is.setstate(state) : void();
  return is;
}

// Inserts a bit reference in an output stream
template <class CharT, class Traits, class T>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    reference<T> x) {
  constexpr char zero = '0';
  constexpr char one = '1';
  return os << os.widen(x ? one : zero);
}

}  // namespace bit

#endif  // _BIT_REFERENCE_DEFINITION_HPP_INCLUDED
