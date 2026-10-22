// ===================== BIT ALGORITHM FORWARD DECLARATIONS ================ //
// Project: The Experimental Bit Algorithms Library
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ALGORITHM_FORWARD_HPP_INCLUDED
#define _BIT_ALGORITHM_FORWARD_HPP_INCLUDED

#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_value.hpp"

namespace bit {

// Helpers shared by copy.hpp and its forward declaration. They live here so
// that headers (such as bit_reference.hpp) which call copy() before copy.hpp is
// included can still name the exact same function template.
//
// Extract the underlying pointer type from either a bit::iterator or a raw
// pointer.
template <typename T>
struct copy_iterator_type {
  using type = typename std::remove_cvref_t<T>::iterator_type;
};
template <typename T>
struct copy_iterator_type<T*> {
  using type = T*;
};
template <typename T>
using copy_iterator_type_t =
    typename copy_iterator_type<std::remove_cvref_t<T>>::type;

// Anything copy() accepts as a source or destination: a bit::iterator of any
// Alignment, or a raw pointer (which denotes a word-aligned bit range).
template <typename T>
concept copy_iterator_like =
    std::is_pointer_v<std::remove_cvref_t<T>> ||
    requires { typename std::remove_cvref_t<T>::iterator_type; };

// The result is always a mutable, run-time-positioned iterator over the
// destination's underlying word iterator.
template <typename DstIt>
using copy_return_t =
    iterator<copy_iterator_type_t<DstIt>, primitive<1>, std::dynamic_extent>;

template <typename SrcIt1, typename SrcIt2, typename DstIt>
constexpr copy_return_t<DstIt> copy(const SrcIt1&, const SrcIt2&,
                                    const DstIt&);

template <class It1, class It2>
constexpr bit_iterator<It2> copy_backward(const bit_iterator<It1>&,
                                          const bit_iterator<It1>&,
                                          const bit_iterator<It2>&);

template <class It1, class It2>
constexpr bool equal(const bit_iterator<It1>&, const bit_iterator<It1>&,
                     const bit_iterator<It2>&);

template <class It>
constexpr bit_iterator<It> find(bit_iterator<It>, bit_iterator<It>, bit_value);

}  // namespace bit

#endif  // _BIT_ALGORITHM_FORWARD_HPP_INCLUDED
