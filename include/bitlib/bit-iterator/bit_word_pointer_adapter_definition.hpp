#ifndef _BIT_WORD_POINTER_ADAPTER_DEFINITION_HPP_INCLUDED
#define _BIT_WORD_POINTER_ADAPTER_DEFINITION_HPP_INCLUDED

#include <iterator>
#include <type_traits>

#include "bitlib/bit-iterator/bit_word_pointer_adapter_declaration.hpp"

namespace bit {

template <typename target_word_ptr, typename source_word_ptr>
/* explicit */ constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::bit_word_pointer_adapter(
    source_word_ptr source)
  requires(is_small_to_big)
    : _source(source) {
}

template <typename target_word_ptr, typename source_word_ptr>
/* explicit */ constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::bit_word_pointer_adapter(
    source_word_ptr source,
    const size_type index)
  requires(is_big_to_small)
    : _source(source), _index(index) {
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr typename bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::reference
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator*() const noexcept {
  if constexpr (is_small_to_big) {
    return reference(*_source);
  } else {
    return reference(*_source, _index);
  }
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator->() const noexcept {
  return bit_word_pointer_adapter(&*_source, _index);
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr typename bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::reference
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator[](difference_type n) const noexcept {
  if constexpr (is_small_to_big) {
    return reference(*std::next(_source, n * ratio));
  } else {
    const difference_type sum = _index + n;
    const difference_type src_diff = (sum - (n < 0) * static_cast<difference_type>(ratio - 1)) / static_cast<difference_type>(ratio);
    const size_type new_index = sum % ratio;
    return reference(*std::next(_source, src_diff), new_index);
  }
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>&
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator++() noexcept {
  if constexpr (is_small_to_big) {
    _source = std::next(_source, ratio);
  } else {
    ++_index;
    if (_index >= ratio) {
      _source = std::next(_source);
      _index -= ratio;
    }
  }
  return *this;
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator++(int) noexcept {
  bit_word_pointer_adapter old = *this;
  ++(*this);
  return old;
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>&
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator--() noexcept {
  if constexpr (is_small_to_big) {
    _source = std::next(_source, -ratio);
  } else {
    if ((_index--) == 0) {
      _source = std::prev(_source);
      _index += ratio;
    }
  }
  return *this;
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator--(int) noexcept {
  bit_word_pointer_adapter old = *this;
  --(*this);
  return old;
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator+(difference_type n) const noexcept {
  if constexpr (is_small_to_big) {
    return bit_word_pointer_adapter(std::next(_source, n * ratio));
  } else {
    const difference_type sum = _index + n;
    const difference_type src_diff = (sum - (n < 0) * static_cast<difference_type>(ratio - 1)) / static_cast<difference_type>(ratio);
    const size_type new_index = static_cast<size_type>(sum) % ratio;
    return bit_word_pointer_adapter(std::next(_source, src_diff), new_index);
  }
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator-(difference_type n) const noexcept {
  if constexpr (is_small_to_big) {
    return bit_word_pointer_adapter(std::next(_source, -n * ratio));
  } else {
    const difference_type sum = _index - n;
    const difference_type src_diff = (sum - (n > 0) * static_cast<difference_type>(ratio - 1)) / static_cast<difference_type>(ratio);
    const size_type new_index = static_cast<size_type>(sum) % ratio;
    return bit_word_pointer_adapter(std::next(_source, src_diff), new_index);
  }
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>&
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator+=(difference_type n) noexcept {
  if constexpr (is_small_to_big) {
    _source = std::next(_source, n * ratio);
  } else {
    const difference_type sum = _index + n;
    const difference_type src_diff = (sum - (n < 0) * static_cast<difference_type>(ratio - 1)) / static_cast<difference_type>(ratio);
    const size_type new_index = static_cast<size_type>(sum) % ratio;
    _source = std::next(_source, src_diff);
    _index = new_index;
  }
  return *this;
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr bit_word_pointer_adapter<target_word_ptr, source_word_ptr>&
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::operator-=(difference_type n) noexcept {
  if constexpr (is_small_to_big) {
    _source = std::next(_source, -n * ratio);
  } else {
    const difference_type sum = _index - n;
    const difference_type src_diff = (sum - (n > 0) * static_cast<difference_type>(ratio - 1)) / static_cast<difference_type>(ratio);
    const size_type new_index = static_cast<size_type>(sum) % ratio;
    _source = std::next(_source, src_diff);
    _index = new_index;
  }
  return *this;
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr typename bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::size_type
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::index() const noexcept {
  return _index;
}

template <typename target_word_ptr, typename source_word_ptr>
constexpr source_word_ptr
bit_word_pointer_adapter<target_word_ptr, source_word_ptr>::base() const noexcept {
  return _source;
}

template <typename T, typename U, typename V, typename W>
constexpr auto operator-(
    const bit_word_pointer_adapter<T, U>& lhs,
    const bit_word_pointer_adapter<V, W>& rhs) {
  using lhs_type = typename bit_word_pointer_adapter<T, U>::difference_type;
  using rhs_type = typename bit_word_pointer_adapter<V, W>::difference_type;
  static_assert(
      bit_word_pointer_adapter<T, U>::ratio ==
          bit_word_pointer_adapter<V, W>::ratio,
      "Cannot subtract iterators with different ratios");
  if constexpr (bit_word_pointer_adapter<T, U>::is_big_to_small) {
    auto main = (lhs._source - rhs._source);
    return main * static_cast<std::common_type_t<lhs_type, rhs_type>>(bit_word_pointer_adapter<T, U>::ratio) + (static_cast<lhs_type>(lhs._index) - static_cast<lhs_type>(rhs._index));
  } else {
    // "small→large" mode: each base‐step is 1 small word, but difference is in big words:
    auto small_diff = (lhs._source - rhs._source);
    return small_diff / static_cast<std::common_type_t<lhs_type, rhs_type>>(bit_word_pointer_adapter<T, U>::ratio);
  }
}

}  // namespace bit

#endif  // _BIT_WORD_POINTER_ADAPTER_DEFINITION_HPP_INCLUDED
