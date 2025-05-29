// ============================= TRANSFORM  TESTS =============================== //
// Project:         The Experimental Bit Algorithms Library
// Description:     Tests for transform algorithms
// Contributor(s):  Bryce Kille
// License:         BSD 3-Clause License
// ========================================================================== //


// ============================== PREAMBLE ================================== //
// C++ standard library
#include <math.h>
#include <algorithm>
#include <functional>
// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit-containers.hpp"
#include "fixtures.hpp"
// Third-party libraries
#include "gtest/gtest.h"
// Miscellaneous
// ========================================================================== //
#include "bitlib/bit-iterator/bit_iterator_adapter.hpp"

TEST(IteratorAdapter, Basic) {
    std::vector<uint16_t> base{0x1234, 0x2345, 0x3456, 0x4567};
    bit::bit_iterator_adapter<uint8_t*, std::vector<uint16_t>::iterator> adapter(base.begin());
    EXPECT_EQ(*adapter, 0x34);
    EXPECT_EQ(*(++adapter), 0x12);
    EXPECT_EQ(*(adapter++), 0x12);
    EXPECT_EQ(*adapter, 0x45);
    adapter += 2;
    EXPECT_EQ(*adapter, 0x56);
}
