#include "bitlib/bitlib.hpp"
// Third-party libraries
#include <memory>

#include "gtest/gtest.h"

TEST(UseCaseTest, VectorAppend) {
  bit::bit_vector<uint16_t> vec;
  uint16_t test = 0xBEEF;
  vec.append_range(bit::bit_span(test));
  EXPECT_EQ(vec.size(), 16);
  EXPECT_EQ(vec[0], bit::bit1);
  EXPECT_EQ(vec[4], bit::bit0);
  EXPECT_EQ(vec[8], bit::bit0);
  EXPECT_EQ(vec[12], bit::bit1);
  vec.push_back(bit::bit0);
  EXPECT_EQ(vec.size(), 17);
  EXPECT_EQ(vec[16], bit::bit0);

  uint32_t test32 = 0xABBADABA;
  vec.append_range(bit::bit_span(test32));
  EXPECT_EQ(vec[17 + 0], bit::bit0);
  EXPECT_EQ(vec[17 + 4], bit::bit1);
  EXPECT_EQ(vec[17 + 8], bit::bit0);
  EXPECT_EQ(vec[17 + 12], bit::bit1);
  EXPECT_EQ(vec[17 + 16], bit::bit0);
  EXPECT_EQ(vec[17 + 20], bit::bit1);
  EXPECT_EQ(vec[17 + 24], bit::bit1);
  EXPECT_EQ(vec[17 + 28], bit::bit0);
}

/*
c++23
TEST(UseCaseTest, StartLifeAs) {
  uint16_t test = 0xBEEF;
  auto ba = std::start_lifetime_as<bit::bit_array<16>>(&test);
  EXPECT_EQ(ba.size(), 16);
  EXPECT_EQ(ba[0], bit::bit1);
  EXPECT_EQ(ba[4], bit::bit0);
  EXPECT_EQ(ba[8], bit::bit0);
  EXPECT_EQ(ba[12], bit::bit1);
}
*/
