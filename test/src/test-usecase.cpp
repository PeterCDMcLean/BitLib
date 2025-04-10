#include "bitlib/bitlib.hpp"
// Third-party libraries
#include <memory>
#include <ranges>

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

TEST(UseCaseTest, RangeCopy) {
  uint32_t test32 = 0xABBADABA;
  auto bspan = bit::bit_span(test32);
  static_assert(std::indirectly_writable<int*, std::iter_reference_t<int*>>);
  static_assert(std::ranges::input_range<bit::bit_span<uint32_t, 32>>);
  static_assert(std::weakly_incrementable<bit::bit_vector<uint32_t>::iterator>);
  static_assert(std::indirectly_readable<std::ranges::iterator_t<bit::bit_span<uint32_t, 32>>>);

  using BitVecIterator = bit::bit_vector<uint32_t>::iterator;
  using BitSpanIterator = std::ranges::iterator_t<bit::bit_span<uint32_t, 32>>;
  using OtherRef = std::iter_reference_t<BitSpanIterator>;  // the type used on the RHS

  // 1. Check that *it = t is valid.
  static_assert(
      requires(BitVecIterator out, OtherRef in) {
        *out = in;
      },
      "Requirement failed: *it = t must be a valid assignment.");

  // 2. Check that *it++ = t is valid.
  static_assert(
      requires(BitVecIterator out, OtherRef in) {
        *out++ = in;
      },
      "Requirement failed: *it++ = t must be a valid assignment.");

  // 3. Check that const_cast<const OtherRef&&>(*it) = t is valid.

  static_assert(
      requires(BitVecIterator&& out, OtherRef&& in) {
        const_cast<const std::iter_reference_t<BitVecIterator>&&>(*out) = std::forward<OtherRef>(in);
      },
      "Requirement failed: const_cast<const OtherRef&&>(*it) = t must be a valid assignment.");

  static_assert(std::indirectly_writable<bit::bit_vector<uint32_t>::iterator, std::iter_reference_t<std::ranges::iterator_t<bit::bit_span<uint32_t, 32>>>>);
  /*
  static_assert(std::indirectly_copyable<std::ranges::iterator_t<bit::bit_span<uint32_t, 32>>, bit::bit_vector<uint32_t>::iterator>);
  */

  bit::bit_vector<uint32_t> v(32);
  std::ranges::copy(bspan, v.begin());
  EXPECT_EQ(v.size(), 32);
  EXPECT_EQ(v[0], (0xABBADABA & (1 << 0)) ? bit::bit1 : bit::bit0);
  EXPECT_EQ(v[1], (0xABBADABA & (1 << 1)) ? bit::bit1 : bit::bit0);
  EXPECT_EQ(v[2], (0xABBADABA & (2 << 1)) ? bit::bit1 : bit::bit0);
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
