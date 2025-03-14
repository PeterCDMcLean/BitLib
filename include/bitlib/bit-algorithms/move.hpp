// ================================== MOVE ================================== //
// Project: The Experimental Bit Algorithms Library
// Name: move.hpp
// Description: bit_iterator overloads for std::move
// Contributor(s):
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _MOVE_HPP_INCLUDED
#define _MOVE_HPP_INCLUDED
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <type_traits>
#include <math.h>
// Project sources
#include "bitlib/bit-iterator/bit.hpp"
// Third-party libraries
// Miscellaneous

namespace bit {
// ========================================================================== //

// Status: Does not work for Input/Output iterators due to distance call
template <class RandomAccessIt1, class RandomAccessIt2>
constexpr bit_iterator<RandomAccessIt2> move(bit_iterator<RandomAccessIt1> first,
                            bit_iterator<RandomAccessIt1> last,
                            bit_iterator<RandomAccessIt2> d_first
)
{
    // Types and constants
    using dst_word_type = typename bit_iterator<RandomAccessIt2>::word_type;
    using src_word_type = typename bit_iterator<RandomAccessIt1>::word_type;
    using word_type = dst_word_type;
    using size_type = typename bit_iterator<RandomAccessIt2>::size_type;
    constexpr size_type digits = binary_digits<word_type>::value;

    // Assertions
    _assert_range_viability(first, last);
    static_assert(std::is_same<dst_word_type, src_word_type>::value, "Underlying word types must be equal");
    if (first == last) return d_first;


    // Initialization
    const bool is_d_first_aligned = d_first.position() == 0;
    size_type total_bits_to_move = distance(first, last);
    size_type remaining_bits_to_move = total_bits_to_move;
    auto it = d_first.base();


    // d_first is not aligned. Copy partial word to align it
    if (!is_d_first_aligned) {
        size_type partial_bits_to_move = std::min(
                remaining_bits_to_move,
                digits - d_first.position()
                );
        *it = _bitblend(
                *it,
                static_cast<word_type>(
                  get_word<word_type>(first, partial_bits_to_move)
                    << static_cast<word_type>(d_first.position())
                ),
                static_cast<word_type>(d_first.position()),
                static_cast<word_type>(partial_bits_to_move)
                );
        remaining_bits_to_move -= partial_bits_to_move;
        advance(first, partial_bits_to_move);
        it++;
    }

    if (remaining_bits_to_move > 0) {
        const bool is_first_aligned = first.position() == 0;
        //size_type words_to_move = std::ceil(remaining_bits_to_move / static_cast<float>(digits));
        // d_first will be aligned at this point
        if (is_first_aligned && remaining_bits_to_move > digits) {
            auto N = std::distance(first.base(), last.base());
            it = std::move(first.base(), last.base(), it);
            first += digits * N;
            remaining_bits_to_move -= digits * N;
        } else {
            // TODO benchmark if its faster to std::move the entire range then shift
            while (remaining_bits_to_move >= digits) {
                *it = get_word<word_type>(first, digits);
                remaining_bits_to_move -= digits;
                it++;
                advance(first, digits);
            }
        }
        if (remaining_bits_to_move > 0) {
            *it = _bitblend(
                    *it,
                    get_word<word_type>(first, remaining_bits_to_move),
                    static_cast<word_type>(
                        (static_cast<word_type>(1) << remaining_bits_to_move) - 1)
            );
        }
    }
    return d_first + total_bits_to_move;
}
// -------------------------------------------------------------------------- //



// ========================================================================== //
} // namespace bit

#endif // _MOVE_HPP_INCLUDED
// ========================================================================== //
