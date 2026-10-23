#ifndef _BIT_ARRAY_REF_DECLARATION_HPP_INCLUDED
#define _BIT_ARRAY_REF_DECLARATION_HPP_INCLUDED

#include <cstddef>
#include <memory>
#include <type_traits>

#include "bitlib/bit-containers/bit_array_base_declaration.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-containers/bit_containers_forward.hpp"
#include "bitlib/bit-iterator/bit_iterator_forward.hpp"
#include "bitlib/bit_concepts.hpp"

namespace bit {

namespace detail {
template <typename word_type>
struct array_ref_iterator_types {
  using iterator = bit_iterator<word_type*>;
  using const_iterator = bit_iterator<const word_type*>;
};

// Storage compatibility for viewing a primitive value through an array_ref.
// The view's word must not be more strictly aligned than the value, and the
// value's storage must be large enough to hold the requested bit range.
template <typename T, typename W, std::size_t N, std::size_t A, typename P>
concept primitive_array_ref_compatible =
    primitive_c<P> && !bit_sized_range<P> && std::is_same_v<T, bit_value> &&
    std::is_integral_v<std::remove_cv_t<W>> &&
    (N == std::dynamic_extent ||
     N == static_cast<std::size_t>(std::remove_cv_t<P>::bits)) &&
    alignof(std::remove_cv_t<W>) <= alignof(std::remove_cv_t<P>) &&
    ((((A == std::dynamic_extent ? std::size_t{0} : A) +
       static_cast<std::size_t>(std::remove_cv_t<P>::bits) +
       bitsof<std::remove_cv_t<W>>() - 1) /
      bitsof<std::remove_cv_t<W>>()) *
         sizeof(std::remove_cv_t<W>) <=
     sizeof(std::remove_cv_t<P>));
}  // namespace detail

/**
 * @brief A non-owning reference to a bit array
 *
 * Similar to array_dynamic_extent but does not allocate or deallocate memory.
 * The pointer and size are const class members and cannot be re-bound.
 * Assignment operations always copy content and can't rebind the pointer/size.
 *
 * @tparam T The value type (typically bit_value)
 * @tparam N The number of value types
 * @tparam W The word type used for storage
 * @tparam A The bit alignment of values in storage, std::dynamic_extent for runtime tracked
 */
template <typename T, size_t N, typename W, size_t A, typename Policy>
class array_ref
    : public array_base<array_ref<T, N, W, A, Policy>, T, N, W, A, false, Policy, detail::array_ref_iterator_types<W>> {
 public:
  using base = array_base<array_ref<T, N, W, A, Policy>, T, N, W, A, false, Policy, detail::array_ref_iterator_types<W>>;
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

 private:
  const pointer<word_type, value_type, A> m_storage;

 public:
  // Constructors
  array_ref() = delete;

  /**
   * @brief Constructs a non-owning reference to a bit array
   *
   * @param storage Pointer to the storage
   * @param extent Number of bits
   */
  constexpr array_ref(word_type* storage, size_type extent)
    requires(N == std::dynamic_extent);
  constexpr array_ref(word_type* storage)
    requires(N != std::dynamic_extent);

  /**
   * @brief Constructs a non-owning reference to a bit array using a bit_pointer
   *
   * @param storage bit_pointer to the storage
   * @param extent Number of bits
   */
  constexpr array_ref(bit_pointer<word_type> storage, size_type extent)
    requires(N == std::dynamic_extent);
  constexpr array_ref(bit_pointer<word_type> storage)
    requires(N != std::dynamic_extent);

  /**
   * @brief Constructs a non-owning reference to a bit array from a bit_sized_range
   *
   * @param other bit_sized_range
   */
  constexpr array_ref(bit_range auto& other, size_type extent)
    requires(N == std::dynamic_extent);
  constexpr array_ref(bit_range auto& other)
    requires(N != std::dynamic_extent);

  /**
   * @brief Constructs a non-owning reference to a bit array from a bit_sized_range
   *
   * @param other bit_sized_range
   */
  constexpr array_ref(const bit_range auto& other, size_type extent)
    requires((N == std::dynamic_extent) && std::is_const_v<W>);
  constexpr array_ref(const bit_range auto& other)
    requires((N != std::dynamic_extent) && std::is_const_v<W>);

  /**
   * @brief Constructs a non-owning view over a primitive value's storage
   *
   * The word type and extent must already match the primitive's storage. The
   * view aliases the primitive, so the primitive must outlive the array_ref.
   */
  template <primitive_c P>
    requires detail::primitive_array_ref_compatible<T, W, N, A, P> &&
             (!std::is_const_v<W>) && (!std::is_const_v<P>)
                                    constexpr array_ref(P& value)
               requires(N == std::dynamic_extent);
  template <primitive_c P>
    requires detail::primitive_array_ref_compatible<T, W, N, A, P> &&
             (!std::is_const_v<W>) && (!std::is_const_v<P>)
                                    constexpr array_ref(P& value)
               requires(N != std::dynamic_extent);
  template <primitive_c P>
    requires detail::primitive_array_ref_compatible<T, W, N, A, P> &&
             std::is_const_v<W>
             constexpr array_ref(const P& value)
               requires(N == std::dynamic_extent);
  template <primitive_c P>
    requires detail::primitive_array_ref_compatible<T, W, N, A, P> &&
             std::is_const_v<W>
             constexpr array_ref(const P& value)
               requires(N != std::dynamic_extent);

  /**
   * @brief Copy constructor
   */
  constexpr array_ref(const array_ref& other) = default;

  /**
   * @brief Move constructor
   */
  constexpr array_ref(array_ref&& other) = default;

  /**
   * @brief Range Assignment operator - copies content but doesn't rebind
   */
  constexpr array_ref& operator=(const bit_sized_range auto& other);

  /**
   * @brief Copy Assignment operator - copies content but doesn't rebind
   */
  constexpr array_ref& operator=(const array_ref& other);

  /**
   * @brief Move assignment operator - copies content but doesn't rebind
   */
  constexpr array_ref& operator=(array_ref&& other);

  /**
   * @brief No destructor needed as we don't own the memory
   */
  ~array_ref() = default;

  /*
   * Iterators
   */
  constexpr iterator begin() noexcept;
  constexpr const_iterator begin() const noexcept;

  /*
   * Operations
   */
  constexpr void swap(array_ref& other);

  /*
   * Element Access
   */
  constexpr word_type* data() noexcept;
  constexpr const word_type* data() const noexcept;
};

// Deduce a view over a primitive value's storage as bit_value bits.
template <primitive_c P>
  requires(!bit_sized_range<P>)
array_ref(P&)
    -> array_ref<bit_value,
                 static_cast<std::size_t>(std::remove_cv_t<P>::bits),
                 detail::inferred_word_type<std::remove_cv_t<P>>, 0,
                 policy::typical<detail::inferred_word_type<std::remove_cv_t<P>>>>;

template <primitive_c P>
  requires(!bit_sized_range<P>)
array_ref(const P&)
    -> array_ref<bit_value,
                 static_cast<std::size_t>(std::remove_cv_t<P>::bits),
                 const detail::inferred_word_type<std::remove_cv_t<P>>, 0,
                 policy::typical<detail::inferred_word_type<std::remove_cv_t<P>>>>;

}  // namespace bit

#endif  // _BIT_ARRAY_REF_DECLARATION_HPP_INCLUDED
