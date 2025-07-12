#ifndef _BIT_WORD_REFERENCE_ADAPTER_HPP_
#define _BIT_WORD_REFERENCE_ADAPTER_HPP_

namespace bit {

template <typename target_word_ptr, typename source_word_ptr>
class bit_word_pointer_adapter;

template <typename target_word_ref, typename source_word_ref>
class bit_word_reference_adapter {
 private:
  // Assertions
  static_assert(std::is_reference_v<source_word_ref>, "source_word_ref must be a reference type");

  using source_word = std::remove_reference_t<source_word_ref>;
  using source_word_ptr = std::add_pointer_t<source_word>;
  using target_word = std::remove_cvref_t<target_word_ref>;
  using target_word_ptr = std::add_pointer_t<target_word>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = bit_word_pointer_adapter<target_word_ptr, source_word_ptr>;

  static_assert(std::is_integral_v<source_word>, "source_word_ref must reference to a integral type");
  static_assert(std::is_integral_v<target_word>, "target_word must be an integral type");

  static constexpr bool is_small_to_big = sizeof(target_word) > sizeof(source_word);
  static constexpr bool is_big_to_small = sizeof(target_word) < sizeof(source_word);
  static constexpr size_t ratio = is_small_to_big ? sizeof(target_word) / sizeof(source_word)
                                                  : sizeof(source_word) / sizeof(target_word);

  source_word_ref _source;
  size_type _index;

 public:
  // Constructor from source
  explicit constexpr bit_word_reference_adapter(source_word_ref source)
    requires(is_small_to_big)
      : _source(source), _index(0) {
  }
  constexpr bit_word_reference_adapter(source_word_ref source, const size_type index)
    requires(is_big_to_small)
      : _source(source), _index(index) {
  }
  explicit constexpr bit_word_reference_adapter(const bit_word_reference_adapter& other)
      : _source(other._source), _index(other._index) {
  }
  constexpr bit_word_reference_adapter& operator=(const bit_word_reference_adapter& other) {
    if constexpr (is_small_to_big) {
      *reinterpret_cast<target_word*>(&_source) = *reinterpret_cast<const target_word*>(&other._source);
    } else {
      *(reinterpret_cast<target_word*>(&_source) + _index) = *(reinterpret_cast<const target_word*>(&other._source) + other._index);
    }
    return *this;
  }

  constexpr ~bit_word_reference_adapter() = default;

  constexpr pointer operator&() const noexcept {
    if constexpr (is_small_to_big) {
      return pointer(&_source);
    } else {
      return pointer(&_source, _index);
    }
  }

  // Assignment from value type.
  // NOT safe for big-endian systems
  constexpr bit_word_reference_adapter& operator=(const target_word& value) noexcept {
    if constexpr (is_small_to_big) {
      *reinterpret_cast<target_word*>(&_source) = value;
    } else {
      *(reinterpret_cast<target_word*>(&_source) + _index) = value;
    }
    return *this;
  }

  // Implicit conversion to value type
  // NOT safe for big-endian systems
  constexpr operator target_word() const noexcept {
    if constexpr (is_small_to_big) {
      return *reinterpret_cast<const target_word*>(&_source);
    } else {
      return *(reinterpret_cast<const target_word*>(&_source) + _index);
    }
  }

#if 0
  // Compound assignment (+=, -=, etc.)
  bit_word_reference_adapter& operator+=(const target_word&);
  bit_word_reference_adapter& operator-=(const target_word&);
  // ... others like *=, /=, etc.

  // Increment/decrement
  bit_word_reference_adapter& operator++();  // Prefix ++
  target_word operator++(int);               // Postfix ++
  bit_word_reference_adapter& operator--();  // Prefix --
  target_word operator--(int);               // Postfix --
#endif

  // Swap support

  void swap(bit_word_reference_adapter& other) noexcept {
    std::swap(_source, other._source);
    std::swap(_index, other._index);
  }

  friend void swap(bit_word_reference_adapter a, bit_word_reference_adapter b) {
    std::swap(a._source, b._source);
    std::swap(a._index, b._index);
  }
};

} // namespace bit

#endif  // _BIT_WORD_REFERENCE_ADAPTER_HPP_
