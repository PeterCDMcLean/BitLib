// ============================= BIT REFERENCE ============================== //
// Project:         The C++ Bit Library
// Name:            bit_reference.hpp
// Description:     A class representing a reference to a bit sequence
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_REFERENCE_HPP_INCLUDED
#define _BIT_REFERENCE_HPP_INCLUDED
// ========================================================================== //

// ================================ PREAMBLE ================================ //
// C++ standard library
// Project sources
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>

#include "bit_details.hpp"
#include "bit_value.hpp"
#include "bitlib/bit-algorithms/bit_algorithm_details.hpp"
#include "bitlib/bit-algorithms/bit_algorithm_forward.hpp"
#include "bitlib/bit-containers/bit_span_declaration.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {
// ========================================================================== //

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

  static constexpr state_type make_state(size_type pos) noexcept {
    if constexpr (Alignment == std::dynamic_extent) {
      return state_type{pos < word_bits
                            ? static_cast<mask_type>(mask_type(1) << pos)
                            : mask_type(0)};
    } else {
      return state_type{};
    }
  }

  constexpr bool value() const noexcept {
    return (*_ref & this->mask()) != mask_type(0);
  }

  constexpr void assign(bool bit) const noexcept {
    if (bit) {
      *_ref = static_cast<word_type>(*_ref | this->mask());
    } else {
      *_ref = static_cast<word_type>(*_ref & ~this->mask());
    }
  }

 public:
  // Lifecycle
  constexpr reference(const reference& other) noexcept
      : state_type(other), _ref(other._ref) {}

  explicit constexpr reference(StorageRef& ref) noexcept
      : state_type(make_state(0)),
        _ref(reinterpret_cast<storage_pointer>(std::addressof(ref))) {}

  constexpr reference(StorageRef& ref, size_type pos) noexcept
      : state_type(make_state(pos)),
        _ref(reinterpret_cast<storage_pointer>(std::addressof(ref))) {
    const size_type capacity = bitsof<word_type>();
    assert(pos <= capacity && ValueType::bits <= capacity - pos);
    if constexpr (Alignment != std::dynamic_extent) {
      assert(pos == Alignment);
    }
  }

  // Assignment
 public:
  constexpr reference& operator=(const reference& other) const noexcept {
    assign(other.value());
    return const_cast<reference&>(*this);
  }

  template <class OtherStorage, size_type OtherAlignment>
  constexpr reference& operator=(
      const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept {
    assign(static_cast<bool>(other));
    return const_cast<reference&>(*this);
  }

  constexpr reference& operator=(const value_type& val) const noexcept {
    assign(static_cast<bool>(val));
    return const_cast<reference&>(*this);
  }

  constexpr reference& assign(word_type val) const noexcept {
    assign((val & word_type(1)) != 0);
    return const_cast<reference&>(*this);
  }

  constexpr reference& assign(word_type val, size_type pos) const {
    assert(pos < word_bits);
    assign(((val >> pos) & word_type(1)) != 0);
    return const_cast<reference&>(*this);
  }

  // Bitwise assignment operators
 public:
  constexpr reference& operator&=(const value_type& other) const noexcept {
    assign(value() && static_cast<bool>(other));
    return const_cast<reference&>(*this);
  }

  constexpr reference& operator|=(const value_type& other) const noexcept {
    assign(value() || static_cast<bool>(other));
    return const_cast<reference&>(*this);
  }

  constexpr reference& operator^=(const value_type& other) const noexcept {
    assign(value() != static_cast<bool>(other));
    return const_cast<reference&>(*this);
  }

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
  explicit constexpr operator bool() const noexcept {
    return value();
  }

  constexpr operator value_type() const noexcept {
    return value_type(value());
  }

  constexpr reference& set(bool bit) const noexcept {
    assign(bit);
    return const_cast<reference&>(*this);
  }

  constexpr reference& set() const noexcept { return set(true); }
  constexpr reference& reset() const noexcept { return set(false); }
  constexpr reference& flip() const noexcept { return set(!value()); }

  // Access
 public:
  constexpr bit_pointer<const_word_type> operator&() const noexcept {
    return bit_pointer<const_word_type>(_ref, this->position());
  }

  constexpr bit_pointer<WordType> operator&() noexcept
    requires(writable_storage)
  {
    return bit_pointer<WordType>(_ref, this->position());
  }

 public:
  // Implementation details: function members
 private:
  reference() noexcept = default;
  explicit constexpr reference(word_type* ptr) noexcept
      : state_type(make_state(0)), _ref(ptr) {}

  constexpr reference(word_type* ptr, size_type pos) noexcept
      : state_type(make_state(pos)), _ref(ptr) {
    const size_type capacity = bitsof<word_type>();
    assert(pos <= capacity && ValueType::bits <= capacity - pos);
  }
};

}  // namespace bit

namespace bit {

namespace detail {
template <typename WordType, size_t Extent, size_t Alignment>
class reference_field_span_state {
 private:
  using span_type = bit_span<WordType, Extent, Alignment, policy::typical<std::remove_cv_t<WordType>>>;
  span_type span_;

 public:
  constexpr reference_field_span_state(WordType* words,
                                       size_t position) noexcept
      : span_(bit_pointer<WordType>(
            words + position / bitsof<WordType>(),
            position % bitsof<WordType>())) {}

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

  constexpr auto field_span() const noexcept {
    return state_type::field_span();
  }

  using value_word_type = ceil_integral<alignof(value_type) * 8>;
  using const_value_span_type = bit_span<const value_word_type, ValueType::bits, Alignment, policy::typical<value_word_type>>;
  using value_span_type = bit_span<value_word_type, ValueType::bits, Alignment, policy::typical<value_word_type>>;

  static constexpr const_value_span_type value_span(
      const value_type& value) noexcept {
    static_assert(sizeof(value_type) % sizeof(value_word_type) == 0);
    static_assert(ValueType::bits <= sizeof(value_type) * 8);
    const auto* words = reinterpret_cast<const value_word_type*>(
        std::addressof(value));
    return const_value_span_type(words);
  }

  static constexpr value_span_type value_span(value_type& value) noexcept {
    static_assert(sizeof(value_type) % sizeof(value_word_type) == 0);
    static_assert(ValueType::bits <= sizeof(value_type) * 8);
    auto* words =
        reinterpret_cast<value_word_type*>(std::addressof(value));
    return value_span_type(words);
  }

  template <class OtherStorage, size_type OtherAlignment>
  constexpr void copy_from(
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

  template <class SourceSpan, class DestinationSpan>
  constexpr void copy_spans(const SourceSpan& source,
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

  constexpr bool field_any() const noexcept {
    const auto span = field_span();
    return ::bit::find(span.begin(), span.end(), bit1) != span.end();
  }

  template <char operation>
  constexpr void combine_with(const value_type& other) const noexcept {
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

  constexpr value_type materialize(bool invert = false) const noexcept {
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

 public:
  constexpr reference(const reference& other) noexcept
      : state_type(other) {}

  explicit constexpr reference(StorageRef& ref) noexcept
      : state_type(reinterpret_cast<storage_pointer>(std::addressof(ref)),
                   Alignment == std::dynamic_extent ? 0 : Alignment) {}

  constexpr reference(StorageRef& ref, size_type pos) noexcept
    requires(Alignment == std::dynamic_extent)
      : state_type(reinterpret_cast<storage_pointer>(std::addressof(ref)),
                   pos) {
    assert(pos <= num_words * word_bits &&
           ValueType::bits <= num_words * word_bits - pos);
  }

  constexpr reference& operator=(const reference& other) const noexcept {
    copy_from(other);
    return const_cast<reference&>(*this);
  }

  template <class OtherStorage, size_t OtherAlignment>
  constexpr reference& operator=(
      const reference<OtherStorage, ValueType, OtherAlignment>& other) const noexcept {
    copy_from(other);
    return const_cast<reference&>(*this);
  }

  constexpr reference& operator=(const value_type& value) const noexcept {
    const auto destination = field_span();
    if constexpr (ValueType::bits == 1) {
      *destination.begin() = value;
    } else {
      const auto source = value_span(value);
      ::bit::copy(source.begin(), source.end(), destination.begin());
    }
    return const_cast<reference&>(*this);
  }

  constexpr reference& operator&=(const value_type& other) const noexcept {
    combine_with<'&'>(other);
    return const_cast<reference&>(*this);
  }

  constexpr reference& operator|=(const value_type& other) const noexcept {
    combine_with<'|'>(other);
    return const_cast<reference&>(*this);
  }

  constexpr reference& operator^=(const value_type& other) const noexcept {
    combine_with<'^'>(other);
    return const_cast<reference&>(*this);
  }

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

  explicit constexpr operator bool() const noexcept {
    return field_any();
  }

  constexpr operator value_type() const noexcept {
    return materialize();
  }

  constexpr bit_pointer<const_word_type> operator&() const noexcept {
    const auto first = field_span().begin();
    return bit_pointer<const_word_type>(first.base(), first.position());
  }

  constexpr bit_pointer<word_type> operator&() noexcept
    requires(!std::is_const_v<storage_type>)
  {
    const auto first = field_span().begin();
    return bit_pointer<word_type>(first.base(), first.position());
  }
};

// Alias: bit_reference remains as an alias to the current form, i.e. a
// single-bit reference with the default storage reference type.
using bit_reference = reference<>;

static_assert(bit_reference_c<reference<uint8_t&>>,
              "reference does not satisfy bit_reference_c concept!");

// Stream functions
template <class CharT, class Traits, class T>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is, reference<T>& x);
template <class CharT, class Traits, class T>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os, reference<T> x);
/* ************************************************************************** */

// -------------------- BIT REFERENCE: STREAM FUNCTIONS --------------------- //
// Extracts a bit reference from an input stream
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
// -------------------------------------------------------------------------- //

// ========================================================================== //
}  // namespace bit
#endif  // _BIT_REFERENCE_HPP_INCLUDED
// ========================================================================== //
