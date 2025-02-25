
#include "bitlib/bit-containers/bit_array.hpp"

#include "fixtures.hpp"

// Third-party libraries
#include "gtest/gtest.h"

/*
 * Constructor tests
 */
// Tests the default c'tor.
TEST(ArrayTest, DefaultConstructor) {
  bit::bit_array<11> barr(bit::bit0);
  EXPECT_EQ(2, sizeof(barr));
  EXPECT_EQ(11u, barr.size());
  EXPECT_EQ(bit::bit0, barr[0]);
  EXPECT_EQ(bit::bit0, barr[1]);
  EXPECT_EQ(bit::bit0, barr[2]);
  EXPECT_EQ(bit::bit0, barr[3]);
  EXPECT_EQ(bit::bit0, barr[4]);
  EXPECT_EQ(bit::bit0, barr[5]);
  EXPECT_EQ(bit::bit0, barr[6]);
  EXPECT_EQ(bit::bit0, barr[7]);
  EXPECT_EQ(bit::bit0, barr[8]);
  EXPECT_EQ(bit::bit0, barr[9]);
  EXPECT_EQ(bit::bit0, barr[10]);
}

TEST(ArrayTest, BitsOf) {
  bit::bit_array<11> barr(bit::bit0);
  EXPECT_EQ(11u, bitsof(barr));
  EXPECT_EQ(11u, bitsof(bit::bit_array<11>()));
}

TEST(ArrayTest, BasicIteration) {
                        //  <-- LSB, apparently 🙄
  bit::bit_array<11> barr("0110_0101_110");
  int i=0;
  for(auto bbit : barr) {
    switch(10 - i++) {
      case  0: EXPECT_EQ(bit::bit0, bbit); break;
      case  1: EXPECT_EQ(bit::bit1, bbit); break;
      case  2: EXPECT_EQ(bit::bit1, bbit); break;
      case  3: EXPECT_EQ(bit::bit1, bbit); break;
      case  4: EXPECT_EQ(bit::bit0, bbit); break;
      case  5: EXPECT_EQ(bit::bit1, bbit); break;
      case  6: EXPECT_EQ(bit::bit0, bbit); break;
      case  7: EXPECT_EQ(bit::bit0, bbit); break;
      case  8: EXPECT_EQ(bit::bit1, bbit); break;
      case  9: EXPECT_EQ(bit::bit1, bbit); break;
      case 10: EXPECT_EQ(bit::bit0, bbit); break;
    }
  }
}

TEST(ArrayTest, ZeroSize) {
  bit::bit_array<0> barr{};
  std::array<uint8_t,0> foo{};
  EXPECT_EQ(sizeof(foo), sizeof(barr));
  EXPECT_EQ(0, bitsof(barr));
}

