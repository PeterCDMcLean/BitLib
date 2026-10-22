#ifndef _BIT_ARRAY_DECLARATION_HPP_INCLUDED
#define _BIT_ARRAY_DECLARATION_HPP_INCLUDED

#include <array>
#include <cstddef>
#include <initializer_list>
#include <span>
#include <string_view>
#include <type_traits>

#include "bitlib/bit-containers/bit_array_base_declaration.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_containers_forward.hpp"
#include "bitlib/bit-containers/bit_policy.hpp"
#include "bitlib/bit-iterator/bit_iterator_forward.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

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
class array : public array_base<array<T, N, W, Policy>, T, N, W, 0, false, Policy, detail::array_iterator_types<T, W, N>> {
 public:
  using base = array_base<array<T, N, W, Policy>, T, N, W, 0, false, Policy, detail::array_iterator_types<T, W, N>>;
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
  constexpr array() noexcept;
  constexpr array(value_type bit_val);
  constexpr array(detail::uninitialized_t, const size_t& size);
  template <std::integral U>
  constexpr array(const U& integral);
  constexpr array(const array<T, N, W, Policy>& other) noexcept;
  constexpr array(const array<T, N, W, Policy>&& other) noexcept;
  constexpr array(const bit_sized_range auto& other);
  constexpr array(const std::initializer_list<value_type> init)
    requires(!std::is_same_v<value_type, word_type>);
  constexpr array(const std::initializer_list<bool> init);
  constexpr array(const std::initializer_list<word_type> init);
  constexpr array(const std::string_view s)
    requires(std::is_same_v<value_type, bit_value>);

  ~array() = default;

  /*
   * Assignment
   */
  constexpr array& operator=(const array<T, N, W, Policy>& other) = default;
  constexpr array& operator=(const bit_sized_range auto& other);
  constexpr array& operator=(array<T, N, W, Policy>&& other) noexcept;

  constexpr word_type* data() noexcept;
  constexpr const word_type* data() const noexcept;

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept;
  constexpr const_iterator begin() const noexcept;

  /*
   * Capacity
   */
  constexpr size_type size() const noexcept;

  /*
   * Operations
   */
  constexpr void swap(array<T, N, W, Policy>& other) noexcept;

  constexpr operator bit_value() const noexcept
    requires(std::is_same_v<value_type, bit_value> && N == 1);
};

}  // namespace bit

#endif  // _BIT_ARRAY_DECLARATION_HPP_INCLUDED
