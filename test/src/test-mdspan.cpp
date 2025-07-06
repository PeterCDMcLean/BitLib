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
      bit::bit_default_accessor<uintptr_t> >
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

TEST(MdSpanTest, BitFixedWordLayout) {
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
  using accessor_t = bit::bit_word_accessor<5, uintptr_t>;
  using mdspan_t =
      std::mdspan<
          accessor_t::element_type,
          std::dextents<size_t, 2>,
          std::layout_right,
          accessor_t>;

  mdspan_t myspan(&dynarr[0], 6, 7);
  for (size_t i = 0; i < myspan.extent(0); i++) {
    for (size_t j = 0; j < myspan.extent(1); j++) {
      auto ref = myspan[i, j];
      for (size_t k = 0; k < 5; k++) {
        bit::bit_value expected = ((rot & 1) ? bit::bit1 : bit::bit0);
        EXPECT_EQ(ref[k], expected);
        rot = (rot >> 1) | ((rot & 1) << 31);
      }
    }
  }
}

TEST(MdSpanTest, BitDynamicWordLayout) {
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
  using accessor_t = bit::bit_word_accessor<std::dynamic_extent, uintptr_t>;
  using extents_t = std::dextents<size_t, 2>;
  using mdspan_t =
      std::mdspan<
          accessor_t::element_type,
          std::dextents<size_t, 2>,
          std::layout_right,
          accessor_t>;

  accessor_t myaccessor(5);
  extents_t dyn_ext{6, 7};
  mdspan_t myspan(&dynarr[0], dyn_ext, myaccessor);
  for (size_t i = 0; i < myspan.extent(0); i++) {
    for (size_t j = 0; j < myspan.extent(1); j++) {
      auto ref = myspan[i, j];
      for (size_t k = 0; k < 5; k++) {
        bit::bit_value expected = ((rot & 1) ? bit::bit1 : bit::bit0);
        EXPECT_EQ(ref[k], expected);
        rot = (rot >> 1) | ((rot & 1) << 31);
      }
    }
  }
}
