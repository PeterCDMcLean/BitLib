#ifndef _BIT_WORD_REFERENCE_ADAPTER_DECLARATION_HPP_INCLUDED
#define _BIT_WORD_REFERENCE_ADAPTER_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <type_traits>

#include "bitlib/bit-iterator/bit_iterator_forward.hpp"

namespace bit {

template <typename target_word_ref, typename source_word_ref>
class bit_word_reference_adapter {
 private:
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
  explicit constexpr bit_word_reference_adapter(source_word_ref source)
    requires(is_small_to_big);
  constexpr bit_word_reference_adapter(source_word_ref source, const size_type index)
    requires(is_big_to_small);
  explicit constexpr bit_word_reference_adapter(const bit_word_reference_adapter& other);
  constexpr bit_word_reference_adapter& operator=(const bit_word_reference_adapter& other);

  constexpr ~bit_word_reference_adapter() = default;

  constexpr pointer operator&() const noexcept;

  constexpr bit_word_reference_adapter& operator=(const target_word& value) noexcept;

  constexpr operator target_word() const noexcept;

  void swap(bit_word_reference_adapter& other) noexcept;

  friend void swap(bit_word_reference_adapter a, bit_word_reference_adapter b);
};

}  // namespace bit

#endif  // _BIT_WORD_REFERENCE_ADAPTER_DECLARATION_HPP_INCLUDED
