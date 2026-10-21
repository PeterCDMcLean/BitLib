// ===================== BIT ALGORITHM FORWARD DECLARATIONS ================ //
// Project: The Experimental Bit Algorithms Library
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ALGORITHM_FORWARD_HPP_INCLUDED
#define _BIT_ALGORITHM_FORWARD_HPP_INCLUDED

#include "bitlib/bit-iterator/bit_details.hpp"
#include "bitlib/bit-iterator/bit_value.hpp"

namespace bit {

template <class It1, class It2>
constexpr bit_iterator<It2> copy(const bit_iterator<It1>&,
                                 const bit_iterator<It1>&,
                                 const bit_iterator<It2>&);

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
