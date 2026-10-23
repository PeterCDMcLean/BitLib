#ifndef _BIT_WORD_POINTER_ADAPTER_DECLARATION_HPP_INCLUDED
#define _BIT_WORD_POINTER_ADAPTER_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <type_traits>

#include "bitlib/bit-iterator/bit_details.hpp"

namespace bit {

template <typename target_word_ptr, typename source_word_ptr>
class bit_word_pointer_adapter {
 private:
  // Assertions
  using _traits_t = _cv_iterator_traits<target_word_ptr>;
  static_assert(binary_digits<typename _traits_t::value_type>::value, "");

 public:
  using target_word = std::remove_const_t<typename _traits_t::value_type>;
  using target_word_ref = _traits_t::reference;
  using source_word = typename _cv_iterator_traits<source_word_ptr>::value_type;
  using source_word_ref = std::add_lvalue_reference_t<source_word>;

  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = bit_word_reference_adapter<target_word_ref, source_word_ref>;
  using pointer = bit_word_pointer_adapter<target_word, source_word_ptr>;

  using iterator_type = target_word_ptr;
  using iterator_category = typename _cv_iterator_traits<source_word_ptr>::iterator_category;
  using value_type = target_word;

 private:
  static_assert(std::is_integral_v<source_word>, "source_word_ptr must be a pointer to a integral type");
  static_assert(std::is_integral_v<target_word>, "target_word must be an integral type");

  static constexpr bool is_small_to_big = sizeof(target_word) > sizeof(source_word);
  static constexpr bool is_big_to_small = sizeof(target_word) < sizeof(source_word);
  static constexpr size_t ratio = is_small_to_big ? sizeof(target_word) / sizeof(source_word)
                                                  : sizeof(source_word) / sizeof(target_word);

  source_word_ptr _source;
  size_type _index;

 public:
  explicit constexpr bit_word_pointer_adapter(source_word_ptr source)
    requires(is_small_to_big);
  explicit constexpr bit_word_pointer_adapter(source_word_ptr source, const size_type index = 0)
    requires(is_big_to_small);

  constexpr bit_word_pointer_adapter(const bit_word_pointer_adapter&) = default;
  constexpr bit_word_pointer_adapter& operator=(const bit_word_pointer_adapter&) = default;
  constexpr bit_word_pointer_adapter(bit_word_pointer_adapter&&) = default;
  constexpr bit_word_pointer_adapter& operator=(bit_word_pointer_adapter&&) = default;
  constexpr ~bit_word_pointer_adapter() = default;

  constexpr reference operator*() const noexcept;

  constexpr bit_word_pointer_adapter operator->() const noexcept;

  constexpr reference operator[](difference_type n) const noexcept;

  constexpr bit_word_pointer_adapter& operator++() noexcept;
  constexpr bit_word_pointer_adapter operator++(int) noexcept;
  constexpr bit_word_pointer_adapter& operator--() noexcept;
  constexpr bit_word_pointer_adapter operator--(int) noexcept;
  constexpr bit_word_pointer_adapter operator+(difference_type n) const noexcept;
  constexpr bit_word_pointer_adapter operator-(difference_type n) const noexcept;
  constexpr bit_word_pointer_adapter& operator+=(difference_type n) noexcept;
  constexpr bit_word_pointer_adapter& operator-=(difference_type n) noexcept;
  constexpr bool operator==(const bit_word_pointer_adapter&) const = default;
  constexpr auto operator<=>(const bit_word_pointer_adapter&) const = default;

  constexpr size_type index() const noexcept;
  constexpr source_word_ptr base() const noexcept;
  template <typename T, typename U, typename V, typename W>
  friend constexpr auto operator-(
      const bit_word_pointer_adapter<T, U>& lhs,
      const bit_word_pointer_adapter<V, W>& rhs);
};

}  // namespace bit

#endif  // _BIT_WORD_POINTER_ADAPTER_DECLARATION_HPP_INCLUDED
