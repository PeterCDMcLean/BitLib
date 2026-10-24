// ============================== ACCUMULATE TESTS ========================== //
// Project:         The Experimental Bit Algorithms Library
// Description:     Tests for accumulate and transform_accumulate
// License:         BSD 3-Clause License
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <algorithm>
#include <bit>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit-containers.hpp"
// Third-party libraries
#include "gtest/gtest.h"
// Miscellaneous
// ========================================================================== //

namespace {
using P1 = bit::primitive<1>;

bool word_bit(const auto* words, std::size_t position) {
  using word_type = std::remove_cv_t<std::remove_pointer_t<decltype(words)>>;
  return (words[position / bit::bitsof<word_type>()] >>
          (position % bit::bitsof<word_type>())) &
         1;
}

template <typename Word>
std::size_t popcount_range(const Word* words, std::size_t begin,
                           std::size_t length) {
  std::size_t result = 0;
  for (std::size_t i = 0; i < length; ++i) {
    result += word_bit(words, begin + i);
  }
  return result;
}

auto popcount_word_op = [](std::size_t acc, auto word) {
  return acc + std::popcount(
                   static_cast<std::make_unsigned_t<decltype(word)>>(word));
};

auto popcount_subword_op = [](std::size_t acc, auto word, std::size_t bits) {
  using word_type = std::remove_cvref_t<decltype(word)>;
  const word_type mask = bits == bit::bitsof<word_type>()
                             ? static_cast<word_type>(~word_type{0})
                             : static_cast<word_type>((word_type{1} << bits) - 1);
  return acc + std::popcount(
                   static_cast<std::make_unsigned_t<word_type>>(word & mask));
};

auto copy_word_op = [](auto acc, auto word) {
  return std::make_pair(word, acc);
};

auto copy_subword_op = [](auto acc, auto word, auto /*bits*/) {
  return std::make_pair(word, acc);
};
}  // namespace

TEST(AccumulateTest, ForwardRuntimeAndFixedAlignment) {
  constexpr std::size_t word_count = 6;
  std::uint64_t source[word_count] = {
      0x0123456789ABCDEFULL, 0x0F0F0F0F0F0F0F0FULL,
      0xF0F0F0F0F0F0F0F0ULL, 0xAAAAAAAAAAAAAAAAULL,
      0x5555555555555555ULL, 0x8000000000000001ULL};
  constexpr std::size_t start = 5;
  constexpr std::size_t length = 200;

  auto first = bit::bit_iterator<const std::uint64_t*>(source, start);
  auto last = first + length;
  const auto expected = popcount_range(source, start, length);
  EXPECT_EQ(bit::accumulate(first, last, std::size_t{0}, popcount_word_op,
                            popcount_subword_op),
            expected);

  // Fixed-Alignment iterators have no mutable position, so the front-end must
  // materialize mutable iterators before walking the range.
  bit::iterator<const std::uint64_t*, P1, start> fixed_first(source);
  bit::iterator<const std::uint64_t*, P1, start> fixed_last(source + 4);
  const auto fixed_expected = popcount_range(source, start, 4 * 64);
  EXPECT_EQ(bit::accumulate(fixed_first, fixed_last, std::size_t{0},
                            popcount_word_op, popcount_subword_op),
            fixed_expected);
}

TEST(AccumulateTest, BackwardWhile) {
  constexpr std::size_t word_count = 5;
  std::uint64_t source[word_count] = {
      0x0123456789ABCDEFULL, 0x0F0F0F0F0F0F0F0FULL,
      0xF0F0F0F0F0F0F0F0ULL, 0xAAAAAAAAAAAAAAAAULL,
      0x5555555555555555ULL};
  constexpr std::size_t start = 7;
  constexpr std::size_t length = 190;

  auto first = bit::bit_iterator<const std::uint64_t*>(source, start);
  auto last = first + length;
  const auto expected = popcount_range(source, start, length);
  EXPECT_EQ(bit::accumulate_backward_while(
                first, last, std::size_t{0},
                [](std::size_t acc, auto word) {
                  return std::make_pair(true, popcount_word_op(acc, word));
                },
                [](std::size_t acc, auto word, std::size_t bits) {
                  return std::make_pair(
                      true, popcount_subword_op(acc, word, bits));
                }),
            expected);
}

TEST(TransformAccumulateTest, ForwardMixedWordSizes) {
  std::uint64_t source[2] = {0x0123456789ABCDEFULL,
                             0xFEDCBA9876543210ULL};
  std::uint16_t destination[8] = {};

  bit::transform_accumulate<true, true>(
      source + 0, source + 2, destination + 0, destination + 8, 0,
      copy_word_op, copy_subword_op);

  for (std::size_t i = 0; i < 128; ++i) {
    EXPECT_EQ(word_bit(destination, i), word_bit(source, i)) << i;
  }

  std::uint16_t narrow_source[8] = {
      0x0123, 0x4567, 0x89AB, 0xCDEF, 0xFEDC, 0xBA98, 0x7654, 0x3210};
  std::uint64_t wide_destination[2] = {};
  bit::transform_accumulate<true, true>(
      narrow_source + 0, narrow_source + 8, wide_destination + 0,
      wide_destination + 2, 0, copy_word_op, copy_subword_op);
  for (std::size_t i = 0; i < 128; ++i) {
    EXPECT_EQ(word_bit(wide_destination, i), word_bit(narrow_source, i)) << i;
  }
}

TEST(TransformAccumulateTest, FixedAlignmentForwardAndBackward) {
  std::uint64_t source[4] = {0x0123456789ABCDEFULL,
                             0x0F0F0F0F0F0F0F0FULL,
                             0xF0F0F0F0F0F0F0F0ULL,
                             0xAAAAAAAAAAAAAAAAULL};
  std::uint64_t destination[4] = {};

  bit::iterator<const std::uint64_t*, P1, 3> first(source);
  bit::iterator<const std::uint64_t*, P1, 3> last(source + 3);
  bit::iterator<std::uint64_t*, P1, 5> d_first(destination);
  bit::iterator<std::uint64_t*, P1, 5> d_last(destination + 3);

  bit::transform_accumulate<true, true>(
      first, last, d_first, d_last, 0, copy_word_op, copy_subword_op);
  for (std::size_t i = 0; i < 192; ++i) {
    EXPECT_EQ(word_bit(destination, 5 + i), word_bit(source, 3 + i)) << i;
  }

  std::uint64_t backward_destination[4] = {};
  bit::iterator<std::uint64_t*, P1, 5> backward_d_first(
      backward_destination);
  bit::iterator<std::uint64_t*, P1, 5> backward_d_last(
      backward_destination + 3);
  bit::transform_accumulate_backward(
      first, last, backward_d_first, backward_d_last, 0, copy_word_op,
      copy_subword_op);
  for (std::size_t i = 0; i < 192; ++i) {
    EXPECT_EQ(word_bit(backward_destination, 5 + i),
              word_bit(source, 3 + i))
        << i;
  }
}

TEST(TransformAccumulateTest, BackwardMixedWordSizes) {
  std::uint64_t source[2] = {0x0123456789ABCDEFULL,
                             0xFEDCBA9876543210ULL};
  std::uint16_t destination[8] = {};

  bit::transform_accumulate_backward(
      source + 0, source + 2, destination + 0, destination + 8, 0,
      copy_word_op, copy_subword_op);
  for (std::size_t i = 0; i < 128; ++i) {
    EXPECT_EQ(word_bit(destination, i), word_bit(source, i)) << i;
  }
}

TEST(TransformAccumulateTest, AlignmentAndLengthSweep) {
  std::uint64_t source[12] = {
      0x0123456789ABCDEFULL, 0x0F0F0F0F0F0F0F0FULL,
      0xF0F0F0F0F0F0F0F0ULL, 0xAAAAAAAAAAAAAAAAULL,
      0x5555555555555555ULL, 0x8000000000000001ULL,
      0xDEADBEEFCAFEBABEULL, 0x13579BDF2468ACE0ULL,
      0xFFFFFFFF00000000ULL, 0x00000000FFFFFFFFULL,
      0x7F7F7F7F7F7F7F7FULL, 0x0102030405060708ULL};
  constexpr std::size_t lengths[] = {0, 1, 2, 63, 64, 65, 127,
                                     128, 129, 191, 192, 193, 255};
  for (std::size_t src_align = 0; src_align < 10; ++src_align) {
    for (std::size_t dst_align = 0; dst_align < 10; ++dst_align) {
      for (const std::size_t length : lengths) {
        std::uint64_t destination[12] = {};
        auto first =
            bit::bit_iterator<const std::uint64_t*>(source, src_align);
        auto last = first + length;
        auto d_first = bit::bit_iterator<std::uint64_t*>(destination,
                                                         dst_align);
        auto d_last = d_first + length;

        bit::transform_accumulate<true, true>(
            first, last, d_first, d_last, 0, copy_word_op, copy_subword_op);
        for (std::size_t i = 0; i < length; ++i) {
          SCOPED_TRACE(testing::Message()
                       << "forward src_align=" << src_align
                       << " dst_align=" << dst_align << " len=" << length
                       << " bit=" << i);
          EXPECT_EQ(word_bit(destination, dst_align + i),
                    word_bit(source, src_align + i));
        }

        std::uint64_t backward_destination[12] = {};
        auto backward_d_first =
            bit::bit_iterator<std::uint64_t*>(backward_destination,
                                              dst_align);
        auto backward_d_last = backward_d_first + length;
        bit::transform_accumulate_backward(
            first, last, backward_d_first, backward_d_last, 0, copy_word_op,
            copy_subword_op);
        for (std::size_t i = 0; i < length; ++i) {
          SCOPED_TRACE(testing::Message()
                       << "backward src_align=" << src_align
                       << " dst_align=" << dst_align << " len=" << length
                       << " bit=" << i);
          EXPECT_EQ(word_bit(backward_destination, dst_align + i),
                    word_bit(source, src_align + i));
        }
      }
    }
  }
}

TEST(AccumulateTest, AlignmentAndLengthSweep) {
  std::uint64_t source[8] = {
      0x0123456789ABCDEFULL, 0x0F0F0F0F0F0F0F0FULL,
      0xF0F0F0F0F0F0F0F0ULL, 0xAAAAAAAAAAAAAAAAULL,
      0x5555555555555555ULL, 0x8000000000000001ULL,
      0xDEADBEEFCAFEBABEULL, 0x13579BDF2468ACE0ULL};
  constexpr std::size_t lengths[] = {0, 1, 2, 63, 64, 65, 127,
                                     128, 129, 191, 192, 193, 255};
  for (std::size_t src_align = 0; src_align < 10; ++src_align) {
    for (const std::size_t length : lengths) {
      auto first = bit::bit_iterator<const std::uint64_t*>(source, src_align);
      auto last = first + length;
      const auto expected = popcount_range(source, src_align, length);
      SCOPED_TRACE(testing::Message()
                   << "src_align=" << src_align << " len=" << length);
      EXPECT_EQ(bit::accumulate(first, last, std::size_t{0},
                                popcount_word_op, popcount_subword_op),
                expected);
      EXPECT_EQ(bit::accumulate_backward_while(
                    first, last, std::size_t{0},
                    [](std::size_t acc, auto word) {
                      return std::make_pair(true,
                                            popcount_word_op(acc, word));
                    },
                    [](std::size_t acc, auto word, std::size_t bits) {
                      return std::make_pair(
                          true, popcount_subword_op(acc, word, bits));
                    }),
                expected);
    }
  }
}

TEST(TransformAccumulateTest, MixedWordAlignmentSweep) {
  std::uint64_t source[8] = {
      0x0123456789ABCDEFULL, 0x0F0F0F0F0F0F0F0FULL,
      0xF0F0F0F0F0F0F0F0ULL, 0xAAAAAAAAAAAAAAAAULL,
      0x5555555555555555ULL, 0x8000000000000001ULL,
      0xDEADBEEFCAFEBABEULL, 0x13579BDF2468ACE0ULL};
  constexpr std::size_t lengths[] = {0, 1, 2, 15, 16, 17, 31, 32, 33,
                                     63, 64, 65, 127, 128, 129, 191,
                                     192, 255};
  for (std::size_t src_align = 0; src_align < 10; ++src_align) {
    for (std::size_t dst_align = 0; dst_align < 10; ++dst_align) {
      for (const std::size_t length : lengths) {
        std::uint16_t destination[20] = {};
        auto first =
            bit::bit_iterator<const std::uint64_t*>(source, src_align);
        auto last = first + length;
        auto d_first = bit::bit_iterator<std::uint16_t*>(destination,
                                                         dst_align);
        auto d_last = d_first + length;

        bit::transform_accumulate<true, true>(
            first, last, d_first, d_last, 0, copy_word_op, copy_subword_op);
        for (std::size_t i = 0; i < length; ++i) {
          SCOPED_TRACE(testing::Message()
                       << "forward src_align=" << src_align
                       << " dst_align=" << dst_align << " len=" << length
                       << " bit=" << i);
          EXPECT_EQ(word_bit(destination, dst_align + i),
                    word_bit(source, src_align + i));
        }

        std::uint16_t backward_destination[20] = {};
        auto backward_d_first =
            bit::bit_iterator<std::uint16_t*>(backward_destination,
                                              dst_align);
        auto backward_d_last = backward_d_first + length;
        bit::transform_accumulate_backward(
            first, last, backward_d_first, backward_d_last, 0, copy_word_op,
            copy_subword_op);
        for (std::size_t i = 0; i < length; ++i) {
          SCOPED_TRACE(testing::Message()
                       << "backward src_align=" << src_align
                       << " dst_align=" << dst_align << " len=" << length
                       << " bit=" << i);
          EXPECT_EQ(word_bit(backward_destination, dst_align + i),
                    word_bit(source, src_align + i));
        }
      }
    }
  }
}

TEST(TransformAccumulateTest, MixedWordAlignmentSweepWideDestination) {
  std::uint16_t source[40];
  for (std::size_t i = 0; i < 40; ++i) {
    source[i] = static_cast<std::uint16_t>(0x9E37u * (i + 1));
  }
  constexpr std::size_t lengths[] = {0, 1, 2, 15, 16, 17, 31, 32, 33,
                                     63, 64, 65, 127, 128, 129, 191,
                                     192, 255};
  for (std::size_t src_align = 0; src_align < 10; ++src_align) {
    for (std::size_t dst_align = 0; dst_align < 10; ++dst_align) {
      for (const std::size_t length : lengths) {
        std::uint64_t destination[8] = {};
        auto first =
            bit::bit_iterator<const std::uint16_t*>(source, src_align);
        auto last = first + length;
        auto d_first = bit::bit_iterator<std::uint64_t*>(destination,
                                                         dst_align);
        auto d_last = d_first + length;

        bit::transform_accumulate<true, true>(
            first, last, d_first, d_last, 0, copy_word_op, copy_subword_op);
        for (std::size_t i = 0; i < length; ++i) {
          SCOPED_TRACE(testing::Message()
                       << "forward src_align=" << src_align
                       << " dst_align=" << dst_align << " len=" << length
                       << " bit=" << i);
          EXPECT_EQ(word_bit(destination, dst_align + i),
                    word_bit(source, src_align + i));
        }

        std::uint64_t backward_destination[8] = {};
        auto backward_d_first =
            bit::bit_iterator<std::uint64_t*>(backward_destination,
                                              dst_align);
        auto backward_d_last = backward_d_first + length;
        bit::transform_accumulate_backward(
            first, last, backward_d_first, backward_d_last, 0, copy_word_op,
            copy_subword_op);
        for (std::size_t i = 0; i < length; ++i) {
          SCOPED_TRACE(testing::Message()
                       << "backward src_align=" << src_align
                       << " dst_align=" << dst_align << " len=" << length
                       << " bit=" << i);
          EXPECT_EQ(word_bit(backward_destination, dst_align + i),
                    word_bit(source, src_align + i));
        }
      }
    }
  }
}
