#include <stdexcept>
#include <string_view>
#include <utility>

#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_array_dynamic_extent.hpp"
#include "fixtures.hpp"

// Third-party libraries
#include "gtest/gtest.h"

TEST(Bitwise, UnaryNot) {
    auto num = 13'8191_b;
    EXPECT_EQ(~num, 13'0_b);
    EXPECT_EQ(~(num(0, 7)), 7'0_b);
}

TEST(Bitwise, Or) {
    auto num = 0x20'DEAD0000_b;
    auto num2 = 0x20'0000BEEF_b;
    auto ord = num | num2;
    EXPECT_EQ(ord, 0x20'DEADBEEF_b);
    EXPECT_EQ(ord.size(), 0x20);
    num |= num2;
    EXPECT_EQ(ord, num);
}

TEST(Bitwise, And) {
  auto num = 0x20'DEAD0000_b;
  auto num2 = 0x20'0000BEEF_b;
  auto anded = num & num2;
  EXPECT_EQ(anded, 0x20'00000000_b);
  EXPECT_EQ(anded.size(), 0x20);
  num &= num2;
  EXPECT_EQ(anded, num);
}

TEST(Bitwise, Xor) {
  auto num = 0x20'DEAD0000_b;
  auto num2 = 0x20'0000BEEF_b;
  auto xord = num ^ num2;
  EXPECT_EQ(xord, 0x20'DEADBEEF_b);
  EXPECT_EQ(xord.size(), 0x20);
  num ^= num2;
  EXPECT_EQ(xord, num);
}
