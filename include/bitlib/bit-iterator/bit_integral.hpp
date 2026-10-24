// ============================= BIT INTEGRAL =============================== //
// Project: The Experimental Bit Algorithms Library
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_INTEGRAL_HPP_INCLUDED
#define _BIT_INTEGRAL_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "bitlib/bit-containers/bit_bitsof.hpp"

namespace bit {

// Smallest unsigned integral type that can hold N bits.
template <std::size_t N>
using ceil_integral = std::conditional_t<
    (N <= bitsof<std::uint8_t>()),
    std::uint8_t,
    std::conditional_t<
        (N <= bitsof<std::uint16_t>()),
        std::uint16_t,
        std::conditional_t<
            (N <= bitsof<std::uint32_t>()),
            std::uint32_t,
            std::conditional_t<
                (N <= bitsof<std::uint64_t>()),
                std::uint64_t,
                std::uint64_t>>>>;

}  // namespace bit

#endif  // _BIT_INTEGRAL_HPP_INCLUDED
