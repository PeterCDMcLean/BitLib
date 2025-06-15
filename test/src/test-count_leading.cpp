#include <stdexcept>
#include <string_view>
#include <utility>

#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_array_dynamic_extent.hpp"
#include "bitlib/bit-algorithms/count.hpp"
#include "fixtures.hpp"

// Third-party libraries
#include "gtest/gtest.h"

TEST(CountLeading, CountLeadingZeroes) {
  auto num = bit::bit_array<bit::bit_value, 128>(bit::bit0);
  auto clz = bit::count_msb(num.begin(), num.end(), bit::bit0);
  std::cout << "Count leading zeroes: " << clz << std::endl;
  EXPECT_EQ(clz, 128);
}
