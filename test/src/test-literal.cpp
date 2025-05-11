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
  auto ten = 4'10_b;
  EXPECT_EQ(ten.size(), 4);
  EXPECT_EQ(ten[0], bit::bit0);
  EXPECT_EQ(ten[1], bit::bit1);
  EXPECT_EQ(ten[2], bit::bit0);
  EXPECT_EQ(ten[3], bit::bit1);
  auto ten_2 = 10_b;
  EXPECT_EQ(ten_2.size(), 4);
  EXPECT_EQ(ten_2[0], bit::bit0);
  EXPECT_EQ(ten_2[1], bit::bit1);
  EXPECT_EQ(ten_2[2], bit::bit0);
  EXPECT_EQ(ten_2[3], bit::bit1);
}

TEST(BitLiteral, hex_base) {
  auto fifteen = 0xF'1234_b;
  EXPECT_EQ(fifteen.size(), 15);
  EXPECT_EQ(fifteen[0], bit::bit0);
  EXPECT_EQ(fifteen[1], bit::bit0);
  EXPECT_EQ(fifteen[2], bit::bit1);
  auto unsized = 0x1234_b;
  EXPECT_EQ(unsized.size(), 16);
  EXPECT_EQ(unsized[0], bit::bit0);
  EXPECT_EQ(unsized[1], bit::bit0);
  EXPECT_EQ(unsized[2], bit::bit1);
}

TEST(BitLiteral, bin_base) {
  auto fifteen = 0b1111'001000110100_b;
  EXPECT_EQ(fifteen.size(), 15);
  EXPECT_EQ(fifteen[0], bit::bit0);
  EXPECT_EQ(fifteen[1], bit::bit0);
  EXPECT_EQ(fifteen[2], bit::bit1);
  auto unsized = 0b01101_b;
  EXPECT_EQ(unsized.size(), 5);
  EXPECT_EQ(unsized[0], bit::bit1);
  EXPECT_EQ(unsized[1], bit::bit0);
  EXPECT_EQ(unsized[2], bit::bit1);
}

TEST(BitLiteral, UserDefinedLiteral) {
  auto arr = 0b01001101_b;
  EXPECT_EQ(arr.size(), 8);
  EXPECT_EQ(arr[7], bit::bit0);
  EXPECT_EQ(arr[0], bit::bit1);
  auto arr2 = 0b1'01001101_b;
  EXPECT_EQ(arr2.size(), 1);
}

TEST(BitLiteral, UserDefinedHexLiteral) {
  auto arr = 0x010A110A_b;
  EXPECT_EQ(arr.size(), 32);
  EXPECT_EQ(arr[0], bit::bit0);
  EXPECT_EQ(arr[1], bit::bit1);
  EXPECT_EQ(arr[2], bit::bit0);
  EXPECT_EQ(arr[3], bit::bit1);
  EXPECT_EQ(arr[7], bit::bit0);
  auto arr2 = 0x19'010A110A_b;
  EXPECT_EQ(arr2.size(), 25);
}

TEST(BitLiteral, UserDefinedDecLiteral) {
  auto arr = 16'12345_b;
  EXPECT_EQ(arr.size(), 16);
  auto arr2 = 0x3039_b;
  EXPECT_EQ(arr, arr2);
  auto arr3 = 16'123'45_b;
  EXPECT_EQ(arr, arr3);
}
