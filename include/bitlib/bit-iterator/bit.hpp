/**
 * @file
 */

// ================================== BIT =================================== //
// Project:         The C++ Bit Library
// Name:            bit.hpp
// Description:     Includes the whole C++ bit library
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_HPP_INCLUDED
#define _BIT_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
// Project sources
#include "bit_details.hpp"
#include "bit_iterator.hpp"
#include "bit_reference.hpp"
#include "bit_value.hpp"
#include "bit_word_pointer_adapter.hpp"
#include "bit_word_reference_adapter.hpp"

// Third-party libraries
// Miscellaneous
// ========================================================================== //

static_assert(std::is_same_v<typename bit::bit_iterator<bit::bit_word_pointer_adapter<uint8_t*, uint16_t*>>::word_type, uint8_t>);

// ========================================================================== //
#endif // _BIT_HPP_INCLUDED
// ========================================================================== //
