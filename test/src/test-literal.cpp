// =============================== FIXTURES ================================= //
// Project:         The Experimental Bit Algorithms Library
// Description:     Fixtures for testing
// Contributor(s):  Bryce Kille
// License:         BSD 3-Clause License
// ========================================================================== //


// ============================== PREAMBLE ================================== //
// C++ standard library
#include <math.h>
#include <type_traits>
// Project sources
#include "bitlib/bit-containers/bit-containers.hpp"
#include "fixtures.hpp"
// Third-party libraries
#include "gtest/gtest.h"
// Miscellaneous
// ========================================================================== //

TEST(BitLiteral, dec_base) {
  auto one = 1'1_b;
  EXPECT_EQ(one.size(), 1);
  EXPECT_EQ(one[0], bit::bit1);
  auto three = 2'1'1_b;
  EXPECT_EQ(three.size(), 2);
  EXPECT_EQ(three[0], bit::bit1);
  EXPECT_EQ(three[1], bit::bit1);
  three = 2'10_b;
  EXPECT_EQ(three.size(), 2);
  EXPECT_EQ(three[0], bit::bit0);
  EXPECT_EQ(three[1], bit::bit1);
}

TEST(BitLiteral, hex_base) {
    auto fifteen = 0xF'1234_b;
    EXPECT_EQ(fifteen.size(), 15);
    EXPECT_EQ(fifteen[0], bit::bit0);
    EXPECT_EQ(fifteen[1], bit::bit0);
    EXPECT_EQ(fifteen[2], bit::bit1);
}

TEST(BitLiteral, bin_base) {
  auto fifteen = 0b1111'001000110100_b;
  EXPECT_EQ(fifteen.size(), 15);
  EXPECT_EQ(fifteen[0], bit::bit0);
  EXPECT_EQ(fifteen[1], bit::bit0);
  EXPECT_EQ(fifteen[2], bit::bit1);
}
