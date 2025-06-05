#ifndef _BIT_WORD_POINTER_ADAPTER_HPP_
#define _BIT_WORD_POINTER_ADAPTER_HPP_

#include "bit_details.hpp"

namespace bit {

template < typename target_word_ref, typename source_word_ref>
class bit_word_reference_adapter;

template <typename target_word_ptr, typename source_word_ptr>
class bit_word_pointer_adapter {
 private:
  // Assertions
  using _traits_t = _cv_iterator_traits<target_word_ptr>;
  static_assert(binary_digits<typename _traits_t::value_type>::value, "");

 public:
  using target_word = _traits_t::value_type;
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
    requires(is_small_to_big)
      : _source(source) {
  }
  explicit constexpr bit_word_pointer_adapter(source_word_ptr source, const size_type index = 0)
    requires(is_big_to_small)
      : _source(source), _index(index) {
  }

  constexpr bit_word_pointer_adapter(const bit_word_pointer_adapter&) = default;
  constexpr bit_word_pointer_adapter& operator=(const bit_word_pointer_adapter&) = default;
  constexpr bit_word_pointer_adapter(bit_word_pointer_adapter&&) = default;
  constexpr bit_word_pointer_adapter& operator=(bit_word_pointer_adapter&&) = default;
  constexpr ~bit_word_pointer_adapter() = default;

  constexpr reference operator*() const noexcept {
    if constexpr (is_small_to_big) {
      return reference(*_source);
    } else {
      return reference(*_source, _index);
    }
  }

  constexpr bit_word_pointer_adapter operator->() const noexcept {
    return bit_word_pointer_adapter(&*_source, _index);
  }

  constexpr reference operator[](difference_type n) const noexcept {
    if constexpr (is_small_to_big) {
      return reference(*std::next(_source, n*ratio));
    } else {
      const difference_type sum = _index + n;
      difference_type src_diff = sum / ratio;
      const size_type new_index = sum % ratio;
      if (new_index > _index) {
        --src_diff;
      }
      return reference(*std::next(_source, src_diff), _index);
    }
  }

  constexpr bit_word_pointer_adapter& operator++() noexcept {
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
  constexpr bit_word_pointer_adapter operator++(int) noexcept {
    bit_word_pointer_adapter old = *this;
    ++(*this);
    return old;
  }
  constexpr bit_word_pointer_adapter& operator--() noexcept {
    if constexpr (is_small_to_big) {
      _source = std::next(_source, -ratio);
    } else {
      _index--;
      if (_index < 0) {
        _source = std::prev(_source);
        _index += ratio;
      }
    }
    return *this;
  }
  constexpr bit_word_pointer_adapter operator--(int) noexcept {
    bit_word_pointer_adapter old = *this;
    --(*this);
    return old;
  }
  constexpr bit_word_pointer_adapter operator+(difference_type n) const noexcept {
    if constexpr (is_small_to_big) {
      return bit_word_pointer_adapter(std::next(_source, n*ratio));
    } else {
      return bit_word_pointer_adapter(_source, _index + n);
    }
  }
  constexpr bit_word_pointer_adapter operator-(difference_type n) const noexcept {
    if constexpr (is_small_to_big) {
      return bit_word_pointer_adapter(std::next(_source, -n*ratio));
    } else {
      return bit_word_pointer_adapter(_source, _index - n);
    }
  }
  constexpr bit_word_pointer_adapter& operator+=(difference_type n) noexcept {
    if constexpr (is_small_to_big) {
      _source = std::next(_source, n * ratio);
    } else {
      const difference_type sum = _index + n;
      difference_type src_diff = sum / ratio;
      const size_type new_index = sum % ratio;
      if (new_index > _index) {
        --src_diff;
      }
      _source = std::next(_source, src_diff);
      _index = new_index;
    }
    return *this;
  }
  constexpr bit_word_pointer_adapter& operator-=(difference_type n) noexcept {
    if constexpr (is_small_to_big) {
      _source = std::next(_source, -n * ratio);
    } else {
      const difference_type sum = _index - n;
      difference_type src_diff = sum / ratio;
      const size_type new_index = sum % ratio;
      if (new_index > _index) {
        --src_diff;
      }
      _source = std::next(_source, src_diff);
      _index = new_index;
    }
    return *this;
  }
  constexpr bool operator==(const bit_word_pointer_adapter&) const = default;
  constexpr auto operator<=>(const bit_word_pointer_adapter&) const = default;

  constexpr size_type index() const noexcept {
    return _index;
  }
  constexpr source_word_ptr base() const noexcept {
    return _source;
  }
  template <typename T, typename U, typename V, typename W>
  friend constexpr auto operator-(
    const bit_word_pointer_adapter<T, U>& lhs,
    const bit_word_pointer_adapter<V, W>& rhs);
};

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
    // “small→large” mode: each base‐step is 1 small word, but difference is in big words:
    auto small_diff = (lhs._source - rhs._source);
    return small_diff / static_cast<std::common_type_t<lhs_type, rhs_type>>(bit_word_pointer_adapter<T, U>::ratio);
  }
}

} // namespace bit

#endif  // _BIT_WORD_POINTER_ADAPTER_HPP_
