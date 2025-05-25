#include <stdexcept>
#include <string_view>
#include <utility>

#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_array_dynamic_extent.hpp"
#include "bitlib/bit-containers/bit_integer.hpp"
#include "fixtures.hpp"

// Third-party libraries
#include "gtest/gtest.h"

TEST(Integer, Constructors) {
  bit::bit_integer<13, false, uint8_t> bitint;
  using bitarray_t = typename bit::bit_integer<13, false, uint8_t>::base_class;
  EXPECT_EQ(static_cast<bitarray_t>(bitint).size(), 13);
  EXPECT_EQ(bit::bitsof(bitint), 13);
}

TEST(Integer, Increment) {
  auto bitliteral = 13'5894_b;
  bit::bit_integer<13, false, uint8_t> bitint(bitliteral);
  EXPECT_EQ(bitint, 13'5894_b);
  EXPECT_EQ(++bitint, 13'5895_b);
  EXPECT_EQ(bitint++, 13'5895_b);
  EXPECT_EQ(bitint++, 13'5896_b);
  EXPECT_EQ(bitint, 13'5897_b);
}
