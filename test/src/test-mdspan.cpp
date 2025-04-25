#include "bitlib/bitlib.hpp"
// Third-party libraries
#include <memory>
#include <ranges>

#include "gtest/gtest.h"
#if defined(MDSPAN_IMPL_STANDARD_NAMESPACE)
#include "mdspan/mdspan.hpp"
#else
#include <mdspan>
#endif

TEST(MdSpanTest, BitDefaultLayout) {
  uint32_t rot = 0xDEADBEEF;
  bit::bit_array<> dynarr(5 * 6 * 7);
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 6; j++) {
      for (int k = 0; k < 7; k++) {
        dynarr[i * 6 * 7 + j * 7 + k] = (rot & 1) ? bit::bit1 : bit::bit0;
        rot = (rot >> 1) | ((rot & 1) << 31);
      }
    }
  }
  rot = 0xDEADBEEF;
  std::mdspan<
      bit::bit_value,
      std::dextents<size_t, 3>,
      std::layout_right,
      bit::bit_default_accessor<uint8_t> >
      myspan(&dynarr[0], 5, 6, 7);

  for (size_t i = 0; i < myspan.extent(0); i++) {
    for (size_t j = 0; j < myspan.extent(1); j++) {
      for (size_t k = 0; k < myspan.extent(2); k++) {
        bit::bit_value expected = ((rot & 1) ? bit::bit1 : bit::bit0);
        EXPECT_EQ(dynarr[i * 6 * 7 + j * 7 + k], expected);
        bit::bit_value actual = myspan[i, j, k];
        EXPECT_EQ(actual, expected);
        rot = (rot >> 1) | ((rot & 1) << 31);
      }
    }
  }
}
