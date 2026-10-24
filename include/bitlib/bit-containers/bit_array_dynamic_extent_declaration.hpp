#ifndef _BIT_ARRAY_DYNAMIC_EXTENT_DECLARATION_HPP_INCLUDED
#define _BIT_ARRAY_DYNAMIC_EXTENT_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <span>
#include <string_view>
#include <type_traits>

#include "bitlib/bit-containers/bit_array_base_declaration.hpp"
#include "bitlib/bit-containers/bit_array_declaration.hpp"
#include "bitlib/bit-iterator/bit_iterator_forward.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {
namespace detail {

template <typename value_type, typename word_type>
struct array_dextent_iterator_types {
  using iterator = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                             bit_iterator<word_type*>,
                                             word_type*>::type;
  using const_iterator = typename std::conditional<std::is_same_v<value_type, bit_value>,
                                                   bit_iterator<const word_type*>,
                                                   const word_type*>::type;
};
}  // namespace detail

template <typename T, typename W, typename Policy>
class array<T, std::dynamic_extent, W, Policy>
    : public array_base<array<T, std::dynamic_extent, W, Policy>, T, std::dynamic_extent, W, 0, false, Policy, detail::array_dextent_iterator_types<T, W>> {
 public:
  using base = array_base<array<T, std::dynamic_extent, W, Policy>, T, std::dynamic_extent, W, 0, false, Policy, detail::array_dextent_iterator_types<T, W>>;
  using base::end;
  using base::size;
  using typename base::const_iterator;
  using typename base::const_reference;
  using typename base::const_value_pointer;
  using typename base::difference_type;
  using typename base::iterator;
  using typename base::reference;
  using typename base::size_type;
  using typename base::value_pointer;
  using typename base::value_type;
  using typename base::word_type;
  using Allocator = typename Policy::allocator;

 private:
  using word_type_ptr = word_type*;
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-pointer-div"
#endif
  static const size_type FixedWords = sizeof(word_type_ptr) / sizeof(word_type);
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
  static const size_type FixedBits = FixedWords * bitsof<word_type>();

  struct Storage {
    union {
      word_type_ptr pointer;
      word_type fixed[FixedWords];
    };
    Allocator m_allocator;

    Storage(size_type words, const Allocator& allocator, detail::uninitialized_t);
    Storage(size_type words, const Allocator& allocator = Allocator());
    Storage(size_type words, Storage&& other);
    Storage(size_type words, const Storage& other, const Allocator& allocator = Allocator());
    template <typename U>
    Storage(size_type words, const U& val, const Allocator& allocator = Allocator());
    Storage() = delete;
    ~Storage() {};
  } storage;

  static constexpr size_type Words(size_type N) {
    return (N * bitsof<value_type>() + bitsof<word_type>() - 1) / bitsof<word_type>();
  };

 public:
  ~array();

  /*
  * Constructors, copies and moves...
  */
  array() = delete;

  constexpr array(const size_type extent, const Allocator& allocator = Allocator());
  constexpr array(detail::uninitialized_t, const size_type extent, const Allocator& allocator = Allocator());
  template <std::integral U>
  constexpr array(const size_type extent, const U& integral, const Allocator& allocator = Allocator());
  constexpr array(const size_type extent, const word_type val, const Allocator& allocator = Allocator());
  constexpr array(const size_type extent, const value_type bit_val, const Allocator& allocator = Allocator())
    requires(!std::is_same<value_type, word_type>::value);
  constexpr array(const array<T, std::dynamic_extent, W, Policy>& other);
  constexpr array(const array<T, std::dynamic_extent, W, Policy>& other, const Allocator& allocator);
  constexpr array(const bit_sized_range auto& other, const Allocator& allocator = Allocator());
  constexpr array(array<T, std::dynamic_extent, W, Policy>&& other);
  constexpr array(array<T, std::dynamic_extent, W, Policy>&& other, const Allocator& allocator);
  constexpr array(const std::initializer_list<value_type> init, const Allocator& allocator = Allocator())
    requires(!std::is_same_v<value_type, word_type>);
  template <typename U>
  constexpr array(const std::initializer_list<U> init, const Allocator& allocator = Allocator());
  constexpr array(const std::string_view s, const Allocator& allocator = Allocator())
    requires(std::is_same_v<value_type, bit_value>);

  /*
   * Assignment
   */
  constexpr array<T, std::dynamic_extent, W, Policy>& operator=(const array<T, std::dynamic_extent, W, Policy>& other);
  constexpr array<T, std::dynamic_extent, W, Policy>& operator=(const bit_sized_range auto& other);
  constexpr array<T, std::dynamic_extent, W, Policy>& operator=(array<T, std::dynamic_extent, W, Policy>&& other);

  /*
   * Element Access
   */
  constexpr word_type* data() noexcept;
  constexpr const word_type* data() const noexcept;

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept;
  constexpr const_iterator begin() const noexcept;

  /*
   * Operations
   */
  constexpr void swap(array<T, std::dynamic_extent, W, Policy>& other) noexcept;
};

}  // namespace bit

#endif  // _BIT_ARRAY_DYNAMIC_EXTENT_DECLARATION_HPP_INCLUDED
