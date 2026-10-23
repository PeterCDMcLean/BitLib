#ifndef _BIT_ARRAY_BASE_DECLARATION_HPP_INCLUDED
#define _BIT_ARRAY_BASE_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <ostream>
#include <span>
#include <string>
#include <type_traits>

#include "bitlib/bit-containers/bit_containers_forward.hpp"
#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

/**
 * @brief Base class template for array implementations
 *
 * This is a CRTP (Curiously Recurring Template Pattern) base class that provides
 * common functionality for array variants.
 *
 * @tparam Derived The derived class (CRTP pattern)
 * @tparam T The value type (typically bit_value)
 * @tparam W The word type used for storage
 * @tparam N The size of the bit array, or std::dynamic_extent for dynamic size
 * @tparam Policy The policy for integral conversion (default is typical)
 * @tparam Iterators A struct that provides iterator and const_iterator types
 */
template <typename Derived, typename T, size_t N, typename W, size_t A, bool resizable, typename Policy, typename Iterators>
class array_base : public detail::container_size_storage<std::size_t, resizable, N> {
 protected:
  constexpr Derived& derived() noexcept;
  constexpr const Derived& derived() const noexcept;

  constexpr auto aligned_begin() noexcept
    requires(A == std::dynamic_extent);
  constexpr auto aligned_end() noexcept
    requires(A == std::dynamic_extent || N == std::dynamic_extent);
  constexpr auto aligned_begin() noexcept
    requires(A != std::dynamic_extent);
  constexpr auto aligned_end() noexcept
    requires(!(A == std::dynamic_extent || N == std::dynamic_extent));

  constexpr const auto aligned_begin() const noexcept
    requires(A == std::dynamic_extent);
  constexpr const auto aligned_end() const noexcept
    requires(A == std::dynamic_extent || N == std::dynamic_extent);
  constexpr const auto aligned_begin() const noexcept
    requires(A != std::dynamic_extent);
  constexpr const auto aligned_end() const noexcept
    requires(!(A == std::dynamic_extent || N == std::dynamic_extent));

 public:
  using detail::container_size_storage<std::size_t, resizable, N>::size;
  using word_type = W;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = typename std::conditional<std::is_same_v<T, bit_value>, ::bit::reference<word_type&>, T&>::type;
  using const_reference = typename std::conditional<std::is_same_v<T, bit_value>, const ::bit::reference<const word_type&>, const T&>::type;
  using value_pointer = pointer<word_type, value_type>;
  using const_value_pointer = const value_pointer;
  using iterator = Iterators::iterator;
  using const_iterator = Iterators::const_iterator;

  constexpr array_base() noexcept;
  constexpr array_base(const size_type& extent) noexcept
    requires(N == std::dynamic_extent);

  // Element access
  constexpr reference operator[](size_type pos);
  constexpr const_reference operator[](size_type pos) const;
  constexpr reference at(size_type pos);
  constexpr const_reference at(size_type pos) const;
  constexpr reference front()
    requires(A == std::dynamic_extent);
  constexpr auto front()
    requires(A != std::dynamic_extent);
  constexpr const_reference front() const
    requires(A == std::dynamic_extent);
  constexpr const auto front() const
    requires(A != std::dynamic_extent);
  constexpr reference back()
    requires(A == std::dynamic_extent || N == std::dynamic_extent);
  constexpr auto back()
    requires(!(A == std::dynamic_extent || N == std::dynamic_extent));
  constexpr const_reference back() const
    requires(A == std::dynamic_extent || N == std::dynamic_extent);
  constexpr const auto back() const
    requires(!(A == std::dynamic_extent || N == std::dynamic_extent));

  constexpr iterator end() noexcept;
  constexpr const_iterator end() const noexcept;
  constexpr const_iterator cbegin() const noexcept;
  constexpr const_iterator cend() const noexcept;

  // Capacity
  constexpr bool empty() const noexcept;
  constexpr size_type max_size() const noexcept;

  // String representation
  constexpr std::string debug_string() const;
  constexpr std::string debug_string(const_iterator first, const_iterator last) const;

  /**
   * @brief Slice operations - returns a array_ref
   */
  constexpr auto operator()(size_type offset, size_type right) const noexcept;
  constexpr auto operator()(size_type offset, size_type right) noexcept;

  /**
   * @brief Compile-time slice [Start, End) - returns a array_ref.
   */
  template <std::size_t Start, std::size_t End>
  constexpr auto slice() const noexcept
    requires(N != std::dynamic_extent && Start <= End && End <= N);
  template <std::size_t Start, std::size_t End>
  constexpr auto slice() noexcept
    requires(N != std::dynamic_extent && Start <= End && End <= N);
  template <std::size_t Start>
  constexpr auto slice(size_t end) const noexcept
    requires(Start != std::dynamic_extent && N != std::dynamic_extent && Start <= N);
  template <std::size_t Start>
  constexpr auto slice(size_t end) noexcept
    requires(Start != std::dynamic_extent && N != std::dynamic_extent && Start <= N);
  template <std::size_t Start = std::dynamic_extent, std::size_t End>
  constexpr auto slice(size_t start) const noexcept
    requires(Start == std::dynamic_extent && N != std::dynamic_extent);
  template <std::size_t Start = std::dynamic_extent, std::size_t End>
  constexpr auto slice(size_t start) noexcept
    requires(Start == std::dynamic_extent && N != std::dynamic_extent);
  template <std::size_t Start, std::size_t End>
  constexpr auto slice() const noexcept
    requires(N == std::dynamic_extent && Start <= End);
  template <std::size_t Start, std::size_t End>
  constexpr auto slice() noexcept
    requires(N == std::dynamic_extent && Start <= End);
  template <std::size_t Start>
  constexpr auto slice(size_t end) const noexcept
    requires(N == std::dynamic_extent);
  template <std::size_t Start>
  constexpr auto slice(size_t end) noexcept
    requires(N == std::dynamic_extent);
  template <std::size_t Start = std::dynamic_extent, std::size_t End>
  constexpr auto slice(size_t start) const noexcept
    requires(Start == std::dynamic_extent && N == std::dynamic_extent);
  template <std::size_t Start = std::dynamic_extent, std::size_t End>
  constexpr auto slice(size_t start) noexcept
    requires(Start == std::dynamic_extent && N == std::dynamic_extent);

  // Common operations
  constexpr void fill(value_type bit_val) noexcept;

  /**
   * @brief Explicit conversion to integral types
   */
  template <std::integral U>
  explicit constexpr operator U() const noexcept;

  using compatible_bitarray = array<value_type, N, word_type, Policy>;

  constexpr compatible_bitarray operator~();
  constexpr compatible_bitarray operator|(const bit_sized_range auto& other) const;
  constexpr Derived& operator|=(const bit_sized_range auto& other);
  constexpr compatible_bitarray operator&(const bit_sized_range auto& other) const;
  constexpr Derived& operator&=(const bit_sized_range auto& other);
  constexpr compatible_bitarray operator^(const bit_sized_range auto& other) const;
  constexpr Derived& operator^=(const bit_sized_range auto& other);

  // Primitive values are not ranges, so accept them separately by viewing
  // their object representation as a bit array. Types that are already bit
  // sized ranges use the range overloads above.
  template <primitive_c P>
    requires(std::is_same_v<value_type, bit_value> && !bit_sized_range<P>)
  constexpr compatible_bitarray operator|(const P& other) const;
  template <primitive_c P>
    requires(std::is_same_v<value_type, bit_value> && !bit_sized_range<P>)
  constexpr Derived& operator|=(const P& other);
  template <primitive_c P>
    requires(std::is_same_v<value_type, bit_value> && !bit_sized_range<P>)
  constexpr compatible_bitarray operator&(const P& other) const;
  template <primitive_c P>
    requires(std::is_same_v<value_type, bit_value> && !bit_sized_range<P>)
  constexpr Derived& operator&=(const P& other);
  template <primitive_c P>
    requires(std::is_same_v<value_type, bit_value> && !bit_sized_range<P>)
  constexpr compatible_bitarray operator^(const P& other) const;
  template <primitive_c P>
    requires(std::is_same_v<value_type, bit_value> && !bit_sized_range<P>)
  constexpr Derived& operator^=(const P& other);

  friend std::ostream& operator<<(std::ostream& os, const array_base& cv) {
    // Save stream formatting settings
    std::ios_base::fmtflags flags = os.flags();
    char fill = os.fill();
    std::streamsize width = os.width();

    std::string content;
    switch (flags & std::ios_base::basefield) {
      case std::ios_base::hex:
        content = ::bit::to_string(cv.derived(), ::bit::string::typical(16));
        break;
      case std::ios_base::oct:
        content = ::bit::to_string(cv.derived(), ::bit::string::typical(8));
        break;
      case std::ios_base::dec:
        content = ::bit::to_string(cv.derived(), ::bit::string::typical(10));
        break;
      default:
        content = ::bit::to_string(cv.derived(), ::bit::string::typical(2));
        break;
    }

    // Clear width to avoid affecting next output
    os.width(0);

    // Apply padding manually if needed
    if (width > static_cast<std::streamsize>(content.size())) {
      std::streamsize pad = width - content.size();
      bool left = (flags & std::ios_base::adjustfield) == std::ios_base::left;

      if (!left) {
        os << std::string(pad, fill);
      }

      os << content;

      if (left) {
        os << std::string(pad, fill);
      }
    } else {
      os << content;
    }

    return os;
  }

 protected:
  template <typename U>
  constexpr void from_integral(const U& integral);
};

}  // namespace bit

#endif  // _BIT_ARRAY_BASE_DECLARATION_HPP_INCLUDED
