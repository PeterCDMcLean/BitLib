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
#include "bitlib/bit-iterator/bit_word_pointer_adapter.hpp"

//
// GoogleTest suite for bit::bit_word_pointer_adapter.
// Covers both "big‐to‐small" and "small‐to‐large" modes.
//

// Assumes a little‐endian platform (so that reinterpret_cast<…> of bytes
// within larger words behaves predictably). If you compile on a big‐endian
// machine, these tests will fail.
//
// Verify that we are on a little‐endian machine:
static_assert(std::endian::native == std::endian::little,
              "These tests assume little‐endian byte order");

TEST(IteratorAdapter, Basic) {
    std::vector<uint16_t> base{0x1234, 0x2345, 0x3456, 0x4567};
    bit::bit_word_pointer_adapter<uint8_t*, std::vector<uint16_t>::iterator> adapter(base.begin());
    EXPECT_EQ(*adapter, 0x34);
    EXPECT_EQ(*(++adapter), 0x12);
    EXPECT_EQ(*(adapter++), 0x12);
    EXPECT_EQ(*adapter, 0x45);
    adapter += 2;
    EXPECT_EQ(*adapter, 0x56);
}

// -----------------------------------------------------------------------------
// Helper: Convert a 32‐bit word into its 4 individual bytes (little‐endian)
// -----------------------------------------------------------------------------
static void split_u32_le(uint32_t value, uint8_t out_bytes[4]) {
  out_bytes[0] = static_cast<uint8_t>(value & 0xFFu);
  out_bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
  out_bytes[2] = static_cast<uint8_t>((value >> 16) & 0xFFu);
  out_bytes[3] = static_cast<uint8_t>((value >> 24) & 0xFFu);
}

// -----------------------------------------------------------------------------
// TEST SUITE: Big-to-Small Mode (BaseIterator.value_type > Iterator.value_type)
// -----------------------------------------------------------------------------
TEST(BitIteratorAdapter_BigToSmall, DereferenceAndIncrement) {
  // Base‐words are 32-bit => 4 × 8-bit subwords
  constexpr size_t N = 2;
  uint32_t base_arr[N] = {0x04030201u, 0x08070605u};
  // Bytes in memory (little‐endian):
  // base_arr[0]: {0x01,0x02,0x03,0x04}
  // base_arr[1]: {0x05,0x06,0x07,0x08}

  // Iterator type: iterate in 8-bit steps over a 32-bit array
  using Iter8 = uint8_t*;
  using Base32 = uint32_t*;
  using Adapter8_32 = bit::bit_word_pointer_adapter<Iter8, Base32>;

  Adapter8_32 it(base_arr, /*pos=*/0);

  // Check that *it sees the first byte = 0x01
  EXPECT_EQ(*it, 0x01u);

  // Advance 1: pos=1, still in base_arr[0]
  ++it;
  EXPECT_EQ(it.index(), 1u);
  EXPECT_EQ(*it, 0x02u);

  // Advance to pos=3
  it += 2;
  EXPECT_EQ(it.index(), 3u);
  EXPECT_EQ(*it, 0x04u);

  // Next increment should roll over to base_arr[1], pos=0
  ++it;
  EXPECT_EQ(it.base(), base_arr + 1);
  EXPECT_EQ(it.index(), 0u);
  EXPECT_EQ(*it, 0x05u);

  // Further increments
  ++it;  // pos=1
  EXPECT_EQ(it.index(), 1u);
  EXPECT_EQ(*it, 0x06u);

  ++it;  // pos=2
  EXPECT_EQ(it.index(), 2u);
  EXPECT_EQ(*it, 0x07u);

  ++it;  // pos=3
  EXPECT_EQ(it.index(), 3u);
  EXPECT_EQ(*it, 0x08u);
}

TEST(BitIteratorAdapter_BigToSmall, OperatorStarAndArrow) {
  // Use a single base word = 0xAABBCCDD
  uint32_t base_val = 0xAABBCCDDu;
  // Memory bytes: { DD, CC, BB, AA } little-endian
  using Iter8 = uint8_t*;
  using Base32 = uint32_t*;
  using Adapter8_32 = bit::bit_word_pointer_adapter<Iter8, Base32>;

  Adapter8_32 it(&base_val, /*pos=*/0);

  EXPECT_EQ(*it, 0xDDu);
  // operator->() should return a pointer to the byte
  auto p = *it;
  EXPECT_EQ(p, 0xDDu);

  // Move to pos=2
  it += 2;
  EXPECT_EQ(*it, 0xBBu);
  EXPECT_EQ(it.index(), 2u);

  // operator->() at pos=2
  p = *it;
  EXPECT_EQ(p, 0xBBu);
}

TEST(BitIteratorAdapter_BigToSmall, OperatorPlusMinusAndDistance) {
  // Fill two 32-bit words
  uint32_t base_arr[2] = {0x11223344u, 0x55667788u};
  // Bytes: {44,33,22,11,  88,77,66,55}

  using Iter8 = uint8_t*;
  using Base32 = uint32_t*;
  using Adapter8_32 = bit::bit_word_pointer_adapter<Iter8, Base32>;

  Adapter8_32 it(base_arr, /*pos=*/0);  // points at first byte (0x44)

  // it + 3 => should point to 4th byte of base_arr[0] = 0x11
  auto it3 = it + 3;
  EXPECT_EQ(it3.base(), base_arr);
  EXPECT_EQ(it3.index(), 3u);
  EXPECT_EQ(*it3, 0x11u);

  // it + 4 => moves into base_arr[1], pos=0 => should be 0x88
  auto it4 = it + 4;
  EXPECT_EQ(it4.base(), base_arr + 1);
  EXPECT_EQ(it4.index(), 0u);
  EXPECT_EQ(*it4, 0x88u);

  // Now distance between it and it+7 = 7
  auto it7 = it + 7;
  EXPECT_EQ(it7.base(), base_arr + 1);
  EXPECT_EQ(it7.index(), 3u);
  EXPECT_EQ(*it7, 0x55u);

  EXPECT_EQ(it7 - it, 7);
  EXPECT_EQ(it - it, 0);

  // Test operator- (iterator difference) with reversed operands
  EXPECT_EQ((it - it7), -7);

  // Advance it4 by -2 => (it4 - 2) = it + 2 => base_arr[0], pos=2 => 0x22
  auto it2 = it4 - 2;
  EXPECT_EQ(it2.base(), base_arr);
  EXPECT_EQ(it2.index(), 2u);
  EXPECT_EQ(*it2, 0x22u);
}

TEST(BitIteratorAdapter_BigToSmall, OperatorSubscript) {
  // 1 uint32_t: 0xDEADBEEFu => bytes { EF, BE, AD, DE }
  uint32_t base_val = 0xDEADBEEFu;
  using Iter8 = uint8_t*;
  using Base32 = uint32_t*;
  using Adapter8_32 = bit::bit_word_pointer_adapter<Iter8, Base32>;

  Adapter8_32 it(&base_val, /*pos=*/0);
  // it[0] == 0xEF, it[1] == 0xBE, it[2] == 0xAD, it[3] == 0xDE
  EXPECT_EQ(it[0], static_cast<uint8_t>(0xEFu));
  EXPECT_EQ(it[1], static_cast<uint8_t>(0xBEu));
  EXPECT_EQ(it[2], static_cast<uint8_t>(0xADu));
  EXPECT_EQ(it[3], static_cast<uint8_t>(0xDEu));

  // Negative index: it[ -1 ] => moves to previous base word;
  // but since there's no previous word, behavior is UB. We skip testing negative here.

  // If we do it + 4, then subscript( it + 4 )[0] should be same as it[4]
  auto it_shift = it + 4;  // rolls into next base (out-of-bounds, but for test allocate two words)
  // For safety, create two‐element array:
  uint32_t arr2[2] = {0x01020304u, 0x05060708u};
  Adapter8_32 it2(arr2, 0);
  EXPECT_EQ(it2[0], 0x04u);
  EXPECT_EQ(it2[1], 0x03u);
  EXPECT_EQ(it2[2], 0x02u);
  EXPECT_EQ(it2[3], 0x01u);
  EXPECT_EQ(it2[4], 0x08u);
  EXPECT_EQ(it2[5], 0x07u);
  EXPECT_EQ(it2[6], 0x06u);
  EXPECT_EQ(it2[7], 0x05u);
}

// -----------------------------------------------------------------------------
// TEST SUITE: Small-to-Large Mode (BaseIterator.value_type < Iterator.value_type)
// -----------------------------------------------------------------------------
TEST(BitIteratorAdapter_SmallToLarge, DereferenceAndIncrement) {
  // Base: array of uint8_t; Iterator’s value_type = uint16_t => ratio = 2
  uint8_t base_arr[] = {
      0x11, 0x22,  // first 16-bit chunk => 0x2211
      0x33, 0x44,  // second 16-bit chunk => 0x4433
      0x55, 0x66   // third 16-bit chunk => 0x6655
  };
  using Iter16 = uint16_t*;
  using Base8 = uint8_t*;
  using Adapter16_8 = bit::bit_word_pointer_adapter<Iter16, Base8>;

  Adapter16_8 it(base_arr);

  // *it: combine base[0] + (base[1] << 8) => 0x2211
  uint16_t first = *it;
  EXPECT_EQ(first, static_cast<uint16_t>(0x2211u));

  // ++it => skip 2 bytes: now base points to base_arr+2
  ++it;
  uint16_t second = *it;
  EXPECT_EQ(second, static_cast<uint16_t>(0x4433u));

  // ++it again
  ++it;
  uint16_t third = *it;
  EXPECT_EQ(third, static_cast<uint16_t>(0x6655u));
}

TEST(BitIteratorAdapter_SmallToLarge, OperatorPlusMinusDistance) {
  // Base: eight uint8_t => four uint16_t logical elements
  uint8_t base_arr[8] = {
      0x10, 0x01,  // 0x0110
      0x20, 0x02,  // 0x0220
      0x30, 0x03,  // 0x0330
      0x40, 0x04   // 0x0440
  };
  using Iter16 = uint16_t*;
  using Base8 = uint8_t*;
  using Adapter16_8 = bit::bit_word_pointer_adapter<Iter16, Base8>;

  Adapter16_8 it(base_arr);

  // it + 0 => *it == 0x0110
  EXPECT_EQ(*it, static_cast<uint16_t>(0x0110u));

  // it + 1 => combine bytes at indices [2,3] => 0x0220
  auto it1 = it + 1;
  EXPECT_EQ(*it1, static_cast<uint16_t>(0x0220u));

  // it + 2 => 0x0330
  auto it2 = it + 2;
  EXPECT_EQ(*it2, static_cast<uint16_t>(0x0330u));

  // it + 3 => 0x0440
  auto it3 = it + 3;
  EXPECT_EQ(*it3, static_cast<uint16_t>(0x0440u));

  // Distance: it3 - it == 3
  EXPECT_EQ(it3 - it, 3);
  EXPECT_EQ(it - it, 0);

  // it2 - it1 == 1
  EXPECT_EQ(it2 - it1, 1);

  // Subtraction reversed:
  EXPECT_EQ(it - it3, -3);
}

TEST(BitIteratorAdapter_SmallToLarge, OperatorStarAndArrow) {
  uint8_t base_arr[4] = {0xAA, 0xBB, 0xCC, 0xDD};
  using Iter16 = uint16_t*;
  using Base8 = uint8_t*;
  using Adapter16_8 = bit::bit_word_pointer_adapter<Iter16, Base8>;

  Adapter16_8 it(base_arr);
  // *it => (0xBB << 8) | 0xAA = 0xBBAA
  EXPECT_EQ(*it, static_cast<uint16_t>(0xBBAAu));
}

TEST(BitIteratorAdapter_SmallToLarge, OperatorPlusEqualsAndMinusEquals) {
  // Base: six uint8_t => three 16-bit words
  uint8_t base_arr[6] = {
      0x01, 0xA0,  // 0xA001
      0x02, 0xB0,  // 0xB002
      0x03, 0xC0   // 0xC003
  };
  using Iter16 = uint16_t*;
  using Base8 = uint8_t*;
  using Adapter16_8 = bit::bit_word_pointer_adapter<Iter16, Base8>;

  Adapter16_8 it(base_arr);

  // it += 1 => now at second 16-bit chunk: 0xB002
  it += 1;
  EXPECT_EQ(*it, static_cast<uint16_t>(0xB002u));

  // it += 1 => now at third: 0xC003
  it += 1;
  EXPECT_EQ(*it, static_cast<uint16_t>(0xC003u));

  // it -= 2 => go back to first chunk: 0xA001
  it -= 2;
  EXPECT_EQ(*it, static_cast<uint16_t>(0xA001u));
}

TEST(BitIteratorAdapter_SmallToLarge, OperatorMinusBetweenDifferentIterators) {
  // Base: eight uint8_t => four 16-bit words
  uint8_t base_arr[8] = {
      0x01, 0x10,  // 0x1001
      0x02, 0x20,  // 0x2002
      0x03, 0x30,  // 0x3003
      0x04, 0x40   // 0x4004
  };
  using Iter16 = uint16_t*;
  using Base8 = uint8_t*;
  using Adapter16_8 = bit::bit_word_pointer_adapter<Iter16, Base8>;

  Adapter16_8 it0(base_arr);
  Adapter16_8 it2(base_arr + 4);  // same as (it0 + 2)

  EXPECT_EQ(it2 - it0, 2);
  EXPECT_EQ(it0 - it2, -2);
}
