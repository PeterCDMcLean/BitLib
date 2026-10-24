#ifndef _BIT_VECTOR_DECLARATION_HPP_INCLUDED
#define _BIT_VECTOR_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "bitlib/bit-containers/bit_containers_forward.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

//! A bit-vector with a similar interface to std::vector<bool>
template <class WordType, class Allocator>
class bit_vector {
 private:
  static constexpr size_t digits = binary_digits<WordType>::value;
  std::vector<WordType, Allocator> word_vector;
  size_t length_ = 0;

  // @brief Get the number of words needed to represnt num_bits bits
  constexpr uint64_t word_count(size_t num_bits);

  // Iterator pair constructor specializations
  // Passing in iterator over bool
  template <class RandomAccessIt>
  typename std::enable_if<
      std::is_same<
          typename std::iterator_traits<RandomAccessIt>::value_type,
          bool>::value>::type constexpr range_constructor(RandomAccessIt first,
                                                          RandomAccessIt last,
                                                          const Allocator& alloc);

  // Passing in iterator over WordType constructs via whole words
  template <class RandomAccessIt>
  typename std::enable_if<
      std::is_same<
          typename std::iterator_traits<RandomAccessIt>::value_type,
          WordType>::value>::type constexpr range_constructor(RandomAccessIt first,
                                                              RandomAccessIt last,
                                                              const Allocator& alloc);

 public:
  /*
   * Types and typedefs
   */
  using value_type = bit_value;
  using base_type = WordType;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = ::bit::reference<WordType&>;
  using const_reference = const reference;
  using pointer = bit_pointer<WordType>;
  using iterator = bit_iterator<typename std::vector<WordType>::iterator>;
  using const_iterator = bit_iterator<typename std::vector<WordType>::const_iterator>;

  /*
   * Constructors, copies and moves...
   */
  constexpr bit_vector() noexcept(noexcept(Allocator()));
  constexpr explicit bit_vector(const Allocator& alloc) noexcept;
  constexpr bit_vector(
      size_type count,
      value_type bit_val,
      const Allocator& alloc = Allocator());
  constexpr explicit bit_vector(
      size_type count,
      const Allocator& alloc = Allocator());
  template <bit_iterator_c Iterator>
  constexpr bit_vector(Iterator first, Iterator last, const Allocator& alloc = Allocator());

  template <bit_range _Range>
  constexpr bit_vector(std::from_range_t, _Range&& rg, const Allocator& alloc = Allocator());

  template <class RandomAccessIt>
  constexpr bit_vector(
      RandomAccessIt first,
      RandomAccessIt last,
      const Allocator& alloc = Allocator());
  constexpr bit_vector(const bit_vector<WordType, Allocator>& other) = default;
  constexpr bit_vector(const bit_vector<WordType, Allocator>& other, const Allocator& alloc);
  constexpr bit_vector(const bit_vector<WordType, Allocator>&& other) noexcept;
  constexpr bit_vector(const bit_vector<WordType, Allocator>&& other, const Allocator& alloc);
  constexpr bit_vector(std::initializer_list<bit_value> init, const Allocator& alloc = Allocator());
  constexpr bit_vector(std::initializer_list<bool> init, const Allocator& alloc = Allocator());
  constexpr bit_vector(std::initializer_list<WordType> init, const Allocator& alloc = Allocator());
  constexpr bit_vector(std::string_view s);

#if __cplusplus == 201703L
  ~bit_vector();
#else
  constexpr ~bit_vector();
#endif

  /*
   * Assignment
   */
  constexpr bit_vector& operator=(const bit_vector<WordType, Allocator>& other) = default;
  constexpr bit_vector& operator=(bit_vector<WordType, Allocator>&& other) noexcept;

  /*
   * Element Access
   */
  constexpr reference operator[](size_type pos);
  constexpr reference operator[](size_type pos) const;
  constexpr reference at(size_type pos);
  constexpr const_reference at(size_type pos) const;
  constexpr reference front();
  constexpr const_reference front() const;
  constexpr reference back();
  constexpr const_reference back() const;
  constexpr WordType* data() noexcept;
  constexpr const WordType* data() const noexcept;

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept;
  constexpr iterator end() noexcept;
  constexpr const_iterator begin() const noexcept;
  constexpr const_iterator end() const noexcept;
  constexpr const_iterator cbegin() const noexcept;
  constexpr const_iterator cend() const noexcept;

  /*
   * Capacity
   */
  constexpr bool empty() const noexcept;
  constexpr size_type size() const noexcept;
  constexpr size_type max_size() const noexcept;
  constexpr void reserve(size_type new_cap);
  constexpr size_type capacity() const noexcept;
  constexpr void shrink_to_fit();

  /*
   * Modifiers
   */
  constexpr void clear() noexcept;
  constexpr iterator insert(const_iterator pos, const value_type& value);
  constexpr iterator insert(const_iterator pos, size_type count, const value_type& value);
  template <bit_iterator_c OtherIt>
  constexpr iterator insert(const_iterator pos, OtherIt first, OtherIt last);
  constexpr iterator erase(iterator pos);
  constexpr iterator erase(iterator first, iterator last);
  constexpr void push_back(const value_type& value);
  constexpr void pop_back();
  constexpr void resize(size_type count);
  constexpr void resize(size_type count, const value_type& value);
  template <bit_range R>
  constexpr iterator insert_range(const_iterator pos, R&& range);
  template <bit_range R>
  constexpr void append_range(R&& range);

  /*
   * Slice
   */
  constexpr bit_array_ref<std::dynamic_extent, WordType> operator()(size_type begin, size_type end) const noexcept;

  /*
   * Helper functions
   */
  constexpr std::string debug_string(const_iterator first, const_iterator end);
  constexpr std::string debug_string();
};

}  // namespace bit

#endif  // _BIT_VECTOR_DECLARATION_HPP_INCLUDED
