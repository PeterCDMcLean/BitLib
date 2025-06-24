#ifndef _BIT_CONCEPTS_HPP_INCLUDED
#define _BIT_CONCEPTS_HPP_INCLUDED

#include <concepts>
#include <cstdint>
#include <iterator>
#include <ranges>

#if (defined(__GLIBCXX__) && (_GLIBCXX_RELEASE < 14))
namespace std {
struct from_range_t {
  explicit from_range_t() = default;
};
inline constexpr from_range_t from_range{};
}  // namespace std
#endif

namespace bit {

class bit_value;

template <typename reference_type, typename value_type>
concept reference_like =
    std::convertible_to<reference_type, value_type> &&  // Can be converted to the value type
    std::assignable_from<reference_type&, value_type>;  // Supports assignment from value type

static_assert(reference_like<uint8_t&, uint8_t>, "pointer_like does not match native types");

template <typename pointer_type, typename reference_type, typename value_type>
concept pointer_like = reference_like<reference_type, value_type> && requires(pointer_type proxy, reference_type ref) {
  // Must be dereferenceable
  { *proxy } -> std::same_as<reference_type>;

  // Must be assignable from reference_type*
  { proxy = &ref } -> std::same_as<pointer_type&>;

  // Should support comparison with other proxies
  { proxy == proxy } -> std::convertible_to<bool>;
  { proxy != proxy } -> std::convertible_to<bool>;

  // Should support swapping
  { std::swap(proxy, proxy) };
};

static_assert(pointer_like<uint8_t*, uint8_t&, uint8_t>, "pointer_like does not match native types");

template <typename T>
concept bit_reference_c = reference_like<T, bit_value>;

template <typename Ptr, typename Ref>
concept bit_pointer_c = pointer_like<Ptr, Ref, bit_value>;

template <typename It>
concept bit_iterator_c =
    std::random_access_iterator<It> &&
    std::same_as<typename std::iterator_traits<It>::value_type, bit_value> &&
    bit_reference_c<typename std::iterator_traits<It>::reference> &&
    bit_pointer_c<typename std::iterator_traits<It>::pointer, typename std::iterator_traits<It>::reference> &&
    requires(It it) {
      // Must have a nested size_type, iterator_type.
      typename It::size_type;
      typename It::iterator_type;
      typename It::word_type;
      typename It::difference_type;
      requires std::integral<typename It::size_type>;
      requires std::random_access_iterator<typename It::iterator_type>;

      // It must provide base(), position(), and mask() with proper return types.
      { it.base() } -> std::same_as<typename It::iterator_type>;
      { it.position() } -> std::same_as<typename It::size_type>;
    };

template <typename It>
concept bit_contiguous_iterator_c = bit_iterator_c<It> &&
                                    std::contiguous_iterator<typename It::iterator_type> &&
                                    std::has_unique_object_representations_v<typename std::iterator_traits<typename It::iterator_type>::value_type>;


template <typename R>
concept bit_range =
    std::ranges::range<R> &&
    std::convertible_to<std::ranges::range_value_t<R>, bit_value>;

template <typename R>
concept bit_sized_range =
    bit_range<R> &&
    std::ranges::sized_range<R>;

#ifdef CONTIGUOUS_RANGE
template <typename R>
concept bit_contiguous_range = bit_range<R> &&
                               std::contiguous_iterator<typename std::ranges::iterator_t<R>::iterator_type> &&
                               std::has_unique_object_representations_v<typename std::iterator_traits<typename std::ranges::iterator_t<R>::iterator_type>::value_type>;

template <typename R>
concept bit_contiguous_sized_range =
    bit_contiguous_range<R> &&
    std::ranges::sized_range<R>;
#endif

} // namespace bit

#endif
