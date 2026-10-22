// ================== BIT ALGORITHM FORWARD DECLARATIONS ==================== //
// Project: The Experimental Bit Algorithms Library
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ALGORITHMS_FORWARD_HPP_INCLUDED
#define _BIT_ALGORITHMS_FORWARD_HPP_INCLUDED

#include <cstddef>

#include "bitlib/bit-algorithms/bit_algorithm_forward.hpp"

namespace bit {

// Tag type used by the type-trait utilities.
struct bit;

namespace policy {
struct AccumulateInitialSubword;
struct AccumulateNoInitialSubword;
}  // namespace policy

namespace string {
struct metadata_t;
}  // namespace string

}  // namespace bit

#endif  // _BIT_ALGORITHMS_FORWARD_HPP_INCLUDED
