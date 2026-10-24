#include <cstddef>
#include <cstdint>
#include <vector>

#include "bitlib/bitlib.hpp"
#include "gtest/gtest.h"

namespace {

template <typename Word>
bool get_bit(const bit::bit_vector<Word>& bits, std::size_t position) {
  constexpr std::size_t digits = bit::bitsof<Word>();
  return ((bits.data()[position / digits] >> (position % digits)) &
          static_cast<Word>(1)) != 0;
}

template <typename Word>
void set_bit(bit::bit_vector<Word>& bits, std::size_t position) {
  constexpr std::size_t digits = bit::bitsof<Word>();
  bits.data()[position / digits] |=
      static_cast<Word>(static_cast<Word>(1) << (position % digits));
}

template <typename Word>
void fill_pattern(bit::bit_vector<Word>& bits, Word seed) {
  constexpr std::size_t digits = bit::bitsof<Word>();
  const std::size_t word_count = (bits.size() + digits - 1) / digits;
  for (std::size_t i = 0; i < word_count; ++i) {
    bits.data()[i] = static_cast<Word>(
        (static_cast<unsigned long long>(0x9E3779B97F4A7C15ULL) * (i + 1)) ^
        static_cast<unsigned long long>(seed));
  }
}

// Reference addition over a bit range, returning the low bits and setting
// carry for an overflow out of the top bit.
template <typename Word>
std::vector<bool> reference_add(const bit::bit_vector<Word>& source,
                                std::size_t start, std::size_t length,
                                Word operand, bool& carry) {
  std::vector<bool> result(length);
  Word remaining_operand = operand;
  unsigned carry_in = 0;
  for (std::size_t i = 0; i < length; ++i) {
    const unsigned sum =
        (get_bit(source, start + i) ? 1u : 0u) +
        static_cast<unsigned>(remaining_operand & static_cast<Word>(1)) +
        carry_in;
    result[i] = (sum & 1u) != 0;
    remaining_operand >>= 1;
    carry_in = sum >> 1;
  }
  // The loop only consumes the operand bits that fit in the range.  Any
  // remaining high operand bits also overflow the range.
  carry = length != 0 && (remaining_operand != 0 || carry_in != 0);
  return result;
}

template <typename Word>
bool same_words(const bit::bit_vector<Word>& lhs,
                const bit::bit_vector<Word>& rhs) {
  constexpr std::size_t digits = bit::bitsof<Word>();
  const std::size_t word_count = (lhs.size() + digits - 1) / digits;
  for (std::size_t i = 0; i < word_count; ++i) {
    if (lhs.data()[i] != rhs.data()[i]) {
      return false;
    }
  }
  return true;
}

template <typename Word>
bool range_matches(const bit::bit_vector<Word>& actual,
                   const std::vector<bool>& expected, std::size_t start,
                   std::size_t length, const bit::bit_vector<Word>& original) {
  for (std::size_t i = 0; i < actual.size(); ++i) {
    const bool want =
        (i >= start && i < start + length)
            ? expected[i - start]
            : get_bit(original, i);
    if (get_bit(actual, i) != want) {
      return false;
    }
  }
  return true;
}

template <typename Word>
void check_in_place_sweep(Word operand,
                          const std::vector<std::size_t>& lengths) {
  constexpr std::size_t total_bits = 256;
  bit::bit_vector<Word> source(total_bits, bit::bit0);
  fill_pattern(source, static_cast<Word>(0xA5A5));

  for (std::size_t start = 0; start < 9; ++start) {
    for (const std::size_t length : lengths) {
      if (start + length > total_bits) {
        continue;
      }
      bool expected_carry = false;
      const auto expected =
          reference_add(source, start, length, operand, expected_carry);
      auto actual = source;
      const unsigned char carry = bit::addition(
          actual.begin() + start, actual.begin() + start + length, operand);
      SCOPED_TRACE(testing::Message()
                   << "operand=" << static_cast<unsigned long long>(operand)
                   << " start=" << start << " length=" << length);
      EXPECT_EQ(carry != 0, expected_carry);
      EXPECT_TRUE(range_matches(actual, expected, start, length, source));
    }
  }
}

}  // namespace

TEST(BitAddition, Basic) {
  auto bits = 0xF'0F_b;
  EXPECT_EQ(0, bit::addition(bits.begin(), bits.end(),
                             static_cast<uint8_t>(0x1u)));
  EXPECT_EQ(0xF'10_b, bits);
}

TEST(BitAddition, LowBits) {
  auto bits = 0x7'F_b;
  EXPECT_EQ(0, bit::addition(bits.begin(), bits.end(), 1'1_b));
  EXPECT_EQ(0x7'10_b, bits);
}

TEST(BitAddition, MultiwordAddsOperandOnce) {
  bit::bit_vector<std::uint64_t> bits(128, bit::bit0);
  bits.data()[0] = 0;
  bits.data()[1] = 0;

  EXPECT_EQ(0, bit::addition(bits.begin(), bits.end(),
                             std::uint64_t{1}));
  EXPECT_EQ(1u, bits.data()[0]);
  EXPECT_EQ(0u, bits.data()[1]);

  bits.data()[0] = ~std::uint64_t{0};
  bits.data()[1] = 0;
  EXPECT_EQ(0, bit::addition(bits.begin(), bits.end(),
                             std::uint64_t{1}));
  EXPECT_EQ(0u, bits.data()[0]);
  EXPECT_EQ(1u, bits.data()[1]);

  bits.data()[0] = ~std::uint64_t{0};
  bits.data()[1] = ~std::uint64_t{0};
  EXPECT_EQ(1, bit::addition(bits.begin(), bits.end(),
                             std::uint64_t{1}));
  EXPECT_EQ(0u, bits.data()[0]);
  EXPECT_EQ(0u, bits.data()[1]);
}

TEST(BitAddition, MisalignedRange) {
  bit::bit_vector<std::uint64_t> bits(16, bit::bit0);
  set_bit(bits, 4);

  EXPECT_EQ(0, bit::addition(bits.begin() + 4, bits.begin() + 8,
                             std::uint8_t{1}));
  EXPECT_FALSE(get_bit(bits, 4));
  EXPECT_TRUE(get_bit(bits, 5));
  EXPECT_FALSE(get_bit(bits, 6));
  EXPECT_FALSE(get_bit(bits, 7));
}

TEST(BitAddition, FullWordOperandOnPartialRange) {
  bit::bit_vector<std::uint64_t> bits(16, bit::bit0);
  set_bit(bits, 4);

  EXPECT_EQ(1, bit::addition(bits.begin() + 4, bits.begin() + 8,
                             std::uint64_t{0xFF}));
  EXPECT_FALSE(get_bit(bits, 4));
  EXPECT_FALSE(get_bit(bits, 5));
  EXPECT_FALSE(get_bit(bits, 6));
  EXPECT_FALSE(get_bit(bits, 7));
}

TEST(BitAddition, EmptyRange) {
  bit::bit_vector<std::uint64_t> bits(32, bit::bit0);
  fill_pattern(bits, std::uint64_t{0x1234});
  const auto original = bits;

  EXPECT_EQ(0, bit::addition(bits.begin() + 7, bits.begin() + 7,
                             std::uint64_t{1}));
  EXPECT_EQ(bits.data()[0], original.data()[0]);
}

TEST(BitAddition, AlignmentAndLengthSweep) {
  constexpr std::uint64_t operands[] = {
      0, 1, 3, 0xFF, 0xFFFF, ~std::uint64_t{0}};
  const std::vector<std::size_t> lengths = {
      0,  1,  2,  3,  4,  7,  8,  15, 16, 17, 31, 32, 33, 63,
      64, 65, 66, 95, 96, 127, 128, 129, 190, 191, 192, 193};

  for (const std::uint64_t operand : operands) {
    check_in_place_sweep(operand, lengths);
  }
}

TEST(BitAddition, SeparateDestinationAlignmentSweep) {
  constexpr std::size_t total_bits = 256;
  constexpr std::uint64_t operands[] = {1, 0xFFFF, ~std::uint64_t{0}};
  const std::vector<std::size_t> lengths = {
      0, 1, 4, 5, 31, 32, 63, 64, 65, 127, 128, 129, 191, 192, 193};

  bit::bit_vector<std::uint64_t> source(total_bits, bit::bit0);
  bit::bit_vector<std::uint64_t> destination(total_bits, bit::bit0);
  fill_pattern(source, std::uint64_t{0xA5A5});
  fill_pattern(destination, std::uint64_t{0x5A5A});

  for (const std::uint64_t operand : operands) {
    for (std::size_t source_start = 0; source_start < 9; ++source_start) {
      for (std::size_t destination_start = 0; destination_start < 9;
           ++destination_start) {
        for (const std::size_t length : lengths) {
          if (source_start + length > total_bits ||
              destination_start + length > total_bits) {
            continue;
          }
          bool expected_carry = false;
          const auto expected = reference_add(
              source, source_start, length, operand, expected_carry);
          auto actual_destination = destination;
          const auto original_source = source;

          const unsigned char carry = bit::addition(
              source.begin() + source_start,
              source.begin() + source_start + length,
              actual_destination.begin() + destination_start, operand);

          SCOPED_TRACE(testing::Message()
                       << "operand=" << operand << " source_start="
                       << source_start << " destination_start="
                       << destination_start << " length=" << length);
          EXPECT_EQ(carry != 0, expected_carry);
          EXPECT_TRUE(range_matches(actual_destination, expected,
                                    destination_start, length, destination));
          EXPECT_TRUE(same_words(source, original_source));
        }
      }
    }
  }
}

TEST(BitAddition, SignedAddCarry) {
  std::int8_t out8 = 0;
  EXPECT_EQ(1, bit::add_carry(std::int8_t{0}, std::int8_t{-1},
                              std::int8_t{1}, &out8));
  EXPECT_EQ(0, out8);

  std::int64_t out64 = 0;
  EXPECT_EQ(1, bit::add_carry(std::int64_t{3}, std::int64_t{-1},
                              std::int64_t{0}, &out64));
  EXPECT_EQ(2, out64);
}

TEST(BitAddition, SignedWordType) {
  bit::bit_vector<std::int64_t> bits(128, bit::bit0);
  bits.data()[0] = -1;
  bits.data()[1] = 0;

  EXPECT_EQ(0, bit::addition(bits.begin(), bits.end(), std::int64_t{1}));
  EXPECT_EQ(0, bits.data()[0]);
  EXPECT_EQ(1, bits.data()[1]);
}

TEST(BitAddition, SmallWordSweep) {
  constexpr std::uint16_t operands[] = {0, 1, 0xFF, 0xFFFF};
  const std::vector<std::size_t> lengths = {
      0, 1, 2, 3, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65};
  for (const std::uint16_t operand : operands) {
    check_in_place_sweep(operand, lengths);
  }
}
