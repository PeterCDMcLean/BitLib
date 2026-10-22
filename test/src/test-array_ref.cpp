#include <cstdint>
#include <limits>

#include "bitlib/bitlib.hpp"
#include "gtest/gtest.h"

struct WideBitValue {
  static constexpr std::size_t bits = 96;
  bit::bit_array<bits> data{};

  bool bit_at(std::size_t index) const {
    return static_cast<bool>(data[index]);
  }
  auto operator[](std::size_t index) { return data[index]; }
  auto operator[](std::size_t index) const { return data[index]; }
};

struct BytePackedValue64 {
  static constexpr std::size_t bits = 64;
  std::uint8_t bytes[8]{};
};

struct BytePackedValue13 {
  static constexpr std::size_t bits = 13;
  std::uint8_t bytes[2]{};
};

struct alignas(16) OverAlignedValue96 {
  static constexpr std::size_t bits = 96;
  std::uint8_t bytes[16]{};
};

namespace {
std::uint64_t read_word_field(const std::uint64_t* words,
                              std::size_t position,
                              std::size_t count = 64) {
  std::uint64_t value = 0;
  for (std::size_t i = 0; i < count; ++i) {
    const std::uint64_t bit =
        (words[(position + i) / 64] >> ((position + i) % 64)) & 1;
    value |= bit << i;
  }
  return value;
}
}  // namespace

TEST(BitArrayRef, ConstructorAndAssignment) {
  // Prepare original bit array
  bit::bit_array<> original(64, bit::bit0);
  original[0] = bit::bit1;
  original[10] = bit::bit1;
  original[63] = bit::bit1;

  // Create a bit_array_ref to original's storage
  bit::bit_array_ref<> ref(original.data(), original.size());

  // Verify the reference matches the original content
  EXPECT_EQ(ref.size(), original.size());
  EXPECT_EQ(ref[0], original[0]);
  EXPECT_EQ(ref[10], original[10]);
  EXPECT_EQ(ref[63], original[63]);

  // Modify through the reference
  ref[20] = bit::bit1;
  ref[30] = bit::bit1;

  // Verify the original was modified
  EXPECT_EQ(original[20], bit::bit1);
  EXPECT_EQ(original[30], bit::bit1);

  // Create another array
  bit::bit_array<> other(64, bit::bit1);

  // Create another reference
  bit::bit_array_ref<> other_ref(other.data(), other.size());

  // Copy content (not pointer)
  ref = other_ref;

  // Verify content was copied
  for (size_t i = 0; i < ref.size(); ++i) {
    EXPECT_EQ(ref[i], bit::bit1);
    EXPECT_EQ(original[i], bit::bit1);
  }
}

TEST(BitArrayRef, SliceOperations) {
  // Prepare original bit array
  bit::bit_array<> original(64, bit::bit0);
  original[10] = bit::bit1;
  original[20] = bit::bit1;
  original[30] = bit::bit1;

  // Create a slice (bit_array_ref) from offset 5 to 35
  auto slice = original(5, 35);

  // Verify the slice's content
  EXPECT_EQ(slice.size(), 30);
  EXPECT_EQ(slice[5], bit::bit1);  // original[10]
  EXPECT_EQ(slice[15], bit::bit1); // original[20]
  EXPECT_EQ(slice[25], bit::bit1); // original[30]

  // Modify through the slice
  slice[10] = bit::bit1;  // original[15]

  // Verify the original was modified
  EXPECT_EQ(original[15], bit::bit1);

  // Create another slice from our slice
  auto subslice = slice(10, 20);

  // Verify the subslice's content
  EXPECT_EQ(subslice.size(), 10);
  EXPECT_EQ(subslice[0], bit::bit1); // slice[10], original[15]
  EXPECT_EQ(subslice[5], bit::bit1); // slice[15], original[20]

  // Modify through the subslice
  subslice[2] = bit::bit1;  // slice[12], original[17]

  // Verify the original and slice were modified
  EXPECT_EQ(slice[12], bit::bit1);
  EXPECT_EQ(original[17], bit::bit1);
}

TEST(BitArrayRef, Operations) {
  // Prepare original bit array
  bit::bit_array<> original(64, bit::bit0);

  // Create a bit_array_ref to original's storage
  bit::bit_array_ref<> ref(original.data(), original.size());

  // Test fill
  ref.fill(bit::bit1);

  // Verify the original was filled with bit1
  for (size_t i = 0; i < original.size(); ++i) {
    EXPECT_EQ(original[i], bit::bit1);
  }

  // Create two arrays
  bit::bit_array<> array1(32, bit::bit1);
  bit::bit_array<> array2(32, bit::bit0);

  // Create refs
  bit::bit_array_ref<> ref1(array1.data(), array1.size());
  bit::bit_array_ref<> ref2(array2.data(), array2.size());

  // Test swap
  ref1.swap(ref2);

  // Verify contents were swapped
  for (size_t i = 0; i < array1.size(); ++i) {
    EXPECT_EQ(array1[i], bit::bit0);
    EXPECT_EQ(array2[i], bit::bit1);
  }

  // Test equality
  EXPECT_NE(ref1, ref2);

  ref1.fill(bit::bit1);
  ref2.fill(bit::bit1);

  EXPECT_EQ(ref1, ref2);
}

TEST(BitArrayRef, IntegralConversion) {
  // Test small values
  bit::bit_array<> value16(16);
  value16[0] = bit::bit1; // 1
  value16[1] = bit::bit1; // 2
  value16[2] = bit::bit1; // 4

  bit::bit_array_ref<> ref16(value16.data(), value16.size());

  // Conversion
  uint16_t int_val = static_cast<uint16_t>(ref16);
  EXPECT_EQ(int_val, 7u); // 2^0 + 2^1 + 2^2 = 1 + 2 + 4 = 7

  // Test larger values
  bit::bit_array<> value32(32);
  value32[0] = bit::bit1;  // 1
  value32[16] = bit::bit1; // 65536
  value32[31] = bit::bit1; // 2^31

  bit::bit_array_ref<> ref32(value32.data(), value32.size());

  // Conversion
  uint32_t int_val32 = static_cast<uint32_t>(ref32);
  EXPECT_EQ(int_val32, 1u + 65536u + (1u << 31));
}

TEST(BitArrayRef, BitPointerConstructor) {
  // Prepare original bit array
  bit::bit_array<> original(64, bit::bit0);
  original[0] = bit::bit1;
  original[10] = bit::bit1;
  original[63] = bit::bit1;

  // Create a bit_pointer to original's storage
  bit::bit_pointer<std::uintptr_t> bit_ptr(original.data());

  // Create a bit_array_ref using the bit_pointer
  bit::bit_array_ref<> ref(bit_ptr, original.size());

  // Verify the reference matches the original content
  EXPECT_EQ(ref.size(), original.size());
  EXPECT_EQ(ref[0], original[0]);
  EXPECT_EQ(ref[10], original[10]);
  EXPECT_EQ(ref[63], original[63]);

  // Modify through the reference
  ref[20] = bit::bit1;
  ref[30] = bit::bit1;

  // Verify the original was modified
  EXPECT_EQ(original[20], bit::bit1);
  EXPECT_EQ(original[30], bit::bit1);
}

TEST(BitArrayRef, MoveAssignment) {
  bit::bit_array<> original(64, bit::bit0);
  original[3] = bit::bit1;
  original[13] = bit::bit1;
  original[33] = bit::bit1;
  original[43] = bit::bit1;
  bit::bit_array_ref<> ref1(original, 32);
  bit::bit_array_ref<> ref2(&original[32], 32);
  ref1 = ref2;
  EXPECT_EQ(ref1, original(32, 64));
}

TEST(BitArrayRef, OffsetBitPointerConstructor) {
  // Prepare original bit array
  bit::bit_array<> original(64, bit::bit0);
  original[3] = bit::bit1;
  original[13] = bit::bit1;

  // Create an offset bit_pointer (starting from the 3rd bit)
  bit::bit_pointer<std::uintptr_t> bit_ptr(original.data(), 3);

  // Create a bit_array_ref using the offset bit_pointer
  bit::bit_array_ref<> ref(bit_ptr, 32);

  // Verify the reference matches the expected content
  EXPECT_EQ(ref.size(), 32);
  EXPECT_EQ(ref[0], bit::bit1);  // This should be original[3]
  EXPECT_EQ(ref[10], bit::bit1); // This should be original[13]

  // Modify through the reference
  ref[5] = bit::bit1;

  // Verify the original was modified at the correct offset
  EXPECT_EQ(original[8], bit::bit1);  // 3 + 5 = 8
}

TEST(BitArrayRef, Throws) {
  bit::bit_array<> original(64, bit::bit0);
  bit::bit_array_ref<> ref(original, 32);
  bit::bit_array<> right_side(33, bit::bit0);
  EXPECT_THROW(ref = right_side, std::invalid_argument);
  bit::bit_array_ref<> ref2(&original[31], 33);
  EXPECT_THROW(ref = ref2, std::invalid_argument);
  EXPECT_THROW(ref.swap(ref2), std::invalid_argument);
}

TEST(BitReference, MultiWordChunkCopy) {
  struct storage {
    std::uint64_t words[2];
  } bits{{0x0123456789abcdefULL, 0xfedcba9876543210ULL}};
  using value_type = bit::primitive<64>;
  using reference_type = bit::reference<storage&, value_type>;

  const auto read_field = [](const storage& source, std::size_t pos) {
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < value_type::bits; ++i) {
      const std::uint64_t bit =
          (source.words[(pos + i) / 64] >> ((pos + i) % 64)) & 1;
      value |= bit << i;
    }
    return value;
  };

  auto before = bits;
  reference_type source(bits, 3);
  reference_type destination(bits, 19);
  const auto expected_forward = read_field(before, 3);
  destination = source;
  EXPECT_EQ(read_field(bits, 19), expected_forward);

  before = bits;
  reference_type source_backward(bits, 21);
  reference_type destination_backward(bits, 4);
  const auto expected_backward = read_field(before, 21);
  destination_backward = source_backward;
  EXPECT_EQ(read_field(bits, 4), expected_backward);

  std::uint64_t single_word = 0xd00df00d12345678ULL;
  bit::reference<std::uint64_t&, value_type> single_source(single_word);
  reference_type cross_word_destination(bits, 57);
  cross_word_destination = single_source;
  EXPECT_EQ(read_field(bits, 57), single_word);

  std::uint64_t single_destination = 0;
  bit::reference<std::uint64_t&, value_type> single_word_destination(
      single_destination);
  reference_type wide_source(bits, 37);
  const auto expected_single = read_field(bits, 37);
  single_word_destination = wide_source;
  EXPECT_EQ(single_destination, expected_single);
}

TEST(BitReference, ValueTypeWiderThanSizeType) {
  struct storage {
    std::uint64_t words[2]{};
  } bits;
  using reference_type = bit::reference<storage&, WideBitValue>;

  WideBitValue value;
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    value[i] = bit::bit_value((i % 3) == 0 || i > 70);
  }

  reference_type first(bits, 5);
  reference_type second(bits, 21);
  first = value;
  const WideBitValue initial_round_trip = static_cast<WideBitValue>(first);
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    EXPECT_EQ(initial_round_trip.bit_at(i), value.bit_at(i)) << i;
  }
  second = first;
  const WideBitValue copied = static_cast<WideBitValue>(second);
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    EXPECT_EQ(copied.bit_at(i), value.bit_at(i)) << i;
  }

  const WideBitValue round_trip = copied;
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    EXPECT_EQ(round_trip.bit_at(i), value.bit_at(i));
  }

  const WideBitValue inverted = ~second;
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    EXPECT_EQ(inverted.bit_at(i), !value.bit_at(i));
  }
}

TEST(BitReference, WideValueBitwise) {
  struct storage {
    std::uint64_t words[2]{};
  } bits;
  using reference_type = bit::reference<storage&, WideBitValue>;

  WideBitValue lhs;
  WideBitValue rhs;
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    lhs[i] = bit::bit_value((i % 3) == 0);
    rhs[i] = bit::bit_value((i % 5) < 2);
  }

  reference_type ref(bits, 7);

  ref = lhs;
  ref &= rhs;
  const WideBitValue and_result = static_cast<WideBitValue>(ref);
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    EXPECT_EQ(and_result.bit_at(i), lhs.bit_at(i) && rhs.bit_at(i)) << i;
  }

  ref = lhs;
  ref |= rhs;
  const WideBitValue or_result = static_cast<WideBitValue>(ref);
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    EXPECT_EQ(or_result.bit_at(i), lhs.bit_at(i) || rhs.bit_at(i)) << i;
  }

  ref = lhs;
  ref ^= rhs;
  const WideBitValue xor_result = static_cast<WideBitValue>(ref);
  for (std::size_t i = 0; i < WideBitValue::bits; ++i) {
    EXPECT_EQ(xor_result.bit_at(i), lhs.bit_at(i) != rhs.bit_at(i)) << i;
  }
}

TEST(BitReference, FixedAlignmentChunkCopy) {
  struct storage {
    std::uint64_t words[2];
  };
  using value_type = bit::primitive<64>;

  {
    storage bits{{0x0123456789abcdefULL, 0xfedcba9876543210ULL}};
    const auto before = bits;
    bit::reference<storage&, value_type, 3> source(bits);
    bit::reference<storage&, value_type, 19> destination(bits);
    destination = source;
    EXPECT_EQ(read_word_field(bits.words, 19),
              read_word_field(before.words, 3));
  }

  {
    // The field crosses the word boundary at bit 64.
    storage bits{{0x0f1e2d3c4b5a6978ULL, 0x8877665544332211ULL}};
    const auto before = bits;
    bit::reference<storage&, value_type, 57> source(bits);
    bit::reference<storage&, value_type, 4> destination(bits);
    destination = source;
    EXPECT_EQ(read_word_field(bits.words, 4),
              read_word_field(before.words, 57));
  }

  {
    // Fixed alignments are normalized to a word offset and word index.
    storage bits{{0x1111222233334444ULL, 0x5555666677778888ULL}};
    const auto before = bits;
    using narrow_value_type = bit::primitive<32>;
    bit::reference<storage&, narrow_value_type, 67> source(bits);
    bit::reference<storage&, narrow_value_type, 64> destination(bits);
    destination = source;
    EXPECT_EQ(read_word_field(bits.words, 64, 32),
              read_word_field(before.words, 67, 32));
  }
}

TEST(BitReference, FixedAlignmentValueAndBitwise) {
  using value_type = bit::primitive<8>;
  std::uint64_t word = ~std::uint64_t{0};
  bit::reference<std::uint64_t&, value_type, 5> ref(word);

  ref = value_type(0xA5);
  EXPECT_EQ((word >> 5) & 0xff, 0xA5u);
  EXPECT_EQ(word & ~(std::uint64_t{0xff} << 5),
            ~std::uint64_t{0} & ~(std::uint64_t{0xff} << 5));
  EXPECT_EQ(static_cast<value_type>(ref).word_value(), 0xA5u);

  ref &= value_type(0x0F);
  EXPECT_EQ((word >> 5) & 0xff, 0x05u);
  ref |= value_type(0x30);
  EXPECT_EQ((word >> 5) & 0xff, 0x35u);
  ref ^= value_type(0xFF);
  EXPECT_EQ((word >> 5) & 0xff, 0xCAu);

  const value_type inverted = ~ref;
  EXPECT_EQ(inverted.word_value(), 0x35u);
}

TEST(BitReference, FixedAlignmentCrossWordValue) {
  struct storage {
    std::uint64_t words[2];
  } bits{{0x0123456789abcdefULL, 0xfedcba9876543210ULL}};
  using value_type = bit::primitive<64>;

  bit::reference<storage&, value_type, 57> ref(bits);
  const value_type value(0xd00df00d12345678ULL);
  ref = value;
  EXPECT_EQ(read_word_field(bits.words, 57), value.word_value());
  EXPECT_EQ(static_cast<value_type>(ref).word_value(), value.word_value());

  const value_type inverted = ~ref;
  EXPECT_EQ(inverted.word_value(), ~value.word_value());
}

TEST(BitReference, BitwiseWithNarrowValueWords) {
  struct storage {
    std::uint64_t words[4];
  } bits{{0, 0, 0, 0}};

  const auto read_byte_value = [](const BytePackedValue64& value) {
    std::uint64_t result = 0;
    for (std::size_t i = 0; i < 8; ++i) {
      result |= static_cast<std::uint64_t>(value.bytes[i]) << (8 * i);
    }
    return result;
  };

  BytePackedValue64 lhs;
  BytePackedValue64 rhs;
  for (std::size_t i = 0; i < 8; ++i) {
    lhs.bytes[i] = static_cast<std::uint8_t>(i + 1);
    rhs.bytes[i] = static_cast<std::uint8_t>(0xF0 + i);
  }

  // The reference word is 64 bits while the value word is one byte.
  bit::reference<storage&, BytePackedValue64, 3> ref(bits);
  ref = lhs;
  EXPECT_EQ(read_word_field(bits.words, 3), read_byte_value(lhs));

  ref &= rhs;
  BytePackedValue64 expected_and;
  for (std::size_t i = 0; i < 8; ++i) {
    expected_and.bytes[i] = lhs.bytes[i] & rhs.bytes[i];
  }
  EXPECT_EQ(read_word_field(bits.words, 3), read_byte_value(expected_and));

  ref = lhs;
  ref |= rhs;
  BytePackedValue64 expected_or;
  for (std::size_t i = 0; i < 8; ++i) {
    expected_or.bytes[i] = lhs.bytes[i] | rhs.bytes[i];
  }
  EXPECT_EQ(read_word_field(bits.words, 3), read_byte_value(expected_or));

  ref = lhs;
  ref ^= rhs;
  BytePackedValue64 expected_xor;
  for (std::size_t i = 0; i < 8; ++i) {
    expected_xor.bytes[i] = lhs.bytes[i] ^ rhs.bytes[i];
  }
  EXPECT_EQ(read_word_field(bits.words, 3), read_byte_value(expected_xor));
}

TEST(BitReference, InvertTrimsValuePadding) {
  std::uint64_t word = ~std::uint64_t{0};
  BytePackedValue13 value;
  value.bytes[0] = 0x5a;
  value.bytes[1] = 0x1f;

  bit::reference<std::uint64_t&, BytePackedValue13, 4> ref(word);
  ref = value;
  EXPECT_EQ((word >> 4) & 0x1fff,
            static_cast<std::uint64_t>(0x1f5a) & 0x1fff);
  EXPECT_EQ(word & ~(std::uint64_t{0x1fff} << 4),
            ~std::uint64_t{0} & ~(std::uint64_t{0x1fff} << 4));

  const BytePackedValue13 inverted = ~ref;
  EXPECT_EQ(inverted.bytes[0], static_cast<std::uint8_t>(~value.bytes[0]));
  EXPECT_EQ(inverted.bytes[1],
            static_cast<std::uint8_t>(~value.bytes[1] & 0x1f));
}

TEST(BitReference, OverAlignedValueType) {
  struct storage {
    std::uint64_t words[2];
  } bits{{0, 0}};

  OverAlignedValue96 value;
  for (std::size_t i = 0; i < 8; ++i) {
    value.bytes[i] = static_cast<std::uint8_t>(0x10 + i);
  }
  std::uint64_t expected = 0;
  for (std::size_t i = 0; i < 8; ++i) {
    expected |= static_cast<std::uint64_t>(value.bytes[i]) << (8 * i);
  }

  bit::reference<storage&, OverAlignedValue96, 3> ref(bits);
  ref = value;
  EXPECT_EQ(read_word_field(bits.words, 3), expected);
  EXPECT_EQ(static_cast<OverAlignedValue96>(ref).bytes[0], value.bytes[0]);
  EXPECT_EQ(static_cast<OverAlignedValue96>(ref).bytes[7], value.bytes[7]);

  const OverAlignedValue96 inverted = ~ref;
  EXPECT_EQ(inverted.bytes[0], static_cast<std::uint8_t>(~value.bytes[0]));
  EXPECT_EQ(inverted.bytes[7], static_cast<std::uint8_t>(~value.bytes[7]));
}

TEST(BitReference, CopyBetweenDifferentWordSizes) {
  struct byte_storage {
    std::uint8_t bytes[2];
  } bytes{{0xab, 0xcd}};
  struct word_storage {
    std::uint64_t words[2]{};
  } words;
  using value_type = bit::primitive<16>;

  const std::uint64_t expected = 0xcdab;
  bit::reference<byte_storage&, value_type> byte_ref(bytes, 0);
  bit::reference<word_storage&, value_type> word_ref(words, 5);
  word_ref = byte_ref;
  EXPECT_EQ((words.words[0] >> 5) & 0xffff, expected);

  bytes.bytes[0] = 0;
  bytes.bytes[1] = 0;
  byte_ref = word_ref;
  EXPECT_EQ(bytes.bytes[0], 0xab);
  EXPECT_EQ(bytes.bytes[1], 0xcd);
}

TEST(BitReference, SingleBitMultiWordStorage) {
  struct storage {
    std::uint64_t words[2];
  } bits{{0, 0}};
  using value_type = bit::primitive<1>;

  bit::reference<storage&, value_type> dynamic_ref(bits, 70);
  bit::reference<storage&, value_type, 70> fixed_ref(bits);
  EXPECT_FALSE(dynamic_ref);
  EXPECT_FALSE(fixed_ref);

  dynamic_ref = true;
  EXPECT_TRUE(dynamic_ref);
  EXPECT_TRUE(fixed_ref);

  fixed_ref.reset();
  EXPECT_FALSE(dynamic_ref);
  EXPECT_FALSE(fixed_ref);

  dynamic_ref.flip();
  EXPECT_TRUE(dynamic_ref);
  EXPECT_TRUE(fixed_ref);
  EXPECT_TRUE(static_cast<bool>(~dynamic_ref) == false);

  std::uint64_t single_word = 0;
  bit::reference<std::uint64_t&, value_type> single_source(single_word, 3);
  single_source = true;
  dynamic_ref = single_source;
  EXPECT_TRUE(dynamic_ref);

  std::uint8_t byte_word = 0;
  bit::reference<std::uint8_t&, value_type> byte_source(byte_word, 3);
  byte_source = true;
  dynamic_ref = byte_source;
  EXPECT_TRUE(dynamic_ref);

  std::uint64_t single_destination = 0;
  bit::reference<std::uint64_t&, value_type> single_destination_ref(
      single_destination, 3);
  single_destination_ref = fixed_ref;
  EXPECT_TRUE(single_destination_ref);
  EXPECT_EQ(single_destination, std::uint64_t{1} << 3);

  std::uint8_t byte_destination = 0;
  bit::reference<std::uint8_t&, value_type> byte_destination_ref(
      byte_destination, 3);
  byte_destination_ref = fixed_ref;
  EXPECT_TRUE(byte_destination_ref);
  EXPECT_EQ(byte_destination, std::uint8_t{1} << 3);
}

TEST(BitReference, OverAlignedStorage) {
  struct alignas(16) storage {
    std::uint64_t words[2];
  } bits{{0x0123456789abcdefULL, 0xfedcba9876543210ULL}};
  using value_type = bit::primitive<32>;

  const auto before = bits;
  bit::reference<storage&, value_type, 67> source(bits);
  bit::reference<storage&, value_type, 64> destination(bits);
  destination = source;
  EXPECT_EQ(read_word_field(bits.words, 64, 32),
            read_word_field(before.words, 67, 32));

  const value_type value(0xdeadbeef);
  source = value;
  EXPECT_EQ(read_word_field(bits.words, 67, 32), value.word_value());
  EXPECT_EQ(static_cast<value_type>(source).word_value(), value.word_value());
}

TEST(BitReference, ConstStorageRead) {
  struct storage {
    std::uint64_t words[2];
  } bits{{0x0123456789abcdefULL, 0xfedcba9876543210ULL}};
  using value_type = bit::primitive<64>;

  const storage& const_bits = bits;
  bit::reference<const storage&, value_type> ref(const_bits, 9);
  EXPECT_EQ(static_cast<value_type>(ref).word_value(),
            read_word_field(bits.words, 9));
  EXPECT_EQ(static_cast<bool>(ref), read_word_field(bits.words, 9) != 0);
}

TEST(BitArrayRef, CompileTimeSlice) {
  bit::bit_array<> original(64, bit::bit0);
  original[10] = bit::bit1;
  original[20] = bit::bit1;
  original[30] = bit::bit1;

  // Create array_ref with compile-time alignment 0
  bit::array_ref<bit::bit_value, 64, std::uintptr_t, 0> ref(original.data());

  // Compile-time slice [10, 40) - both Start and End are compile-time constants
  auto sliced = ref.slice<10, 40>();
  EXPECT_EQ(sliced.size(), 30);
  EXPECT_EQ(sliced[0], original[10]);
  EXPECT_EQ(sliced[10], original[20]);
  EXPECT_EQ(sliced[20], original[30]);

  // Compile-time Start, runtime End: [0, 32)
  auto sliced2 = ref.slice<0>(32);
  EXPECT_EQ(sliced2.size(), 32);
  EXPECT_EQ(sliced2[0], original[0]);
  EXPECT_EQ(sliced2[10], original[10]);

  // Compile-time End, runtime Start: [10, 32)
  auto sliced3 = ref.slice<std::dynamic_extent, 32>(10);
  EXPECT_EQ(sliced3.size(), 22);
  EXPECT_EQ(sliced3[0], original[10]);
}

TEST(BitArrayRef, CompileTimeSliceAlignment) {
  bit::bit_array<> original(128, bit::bit0);
  // Set bits at positions 8, 24, 40, 56, 72, 88, 104, 120
  for (std::size_t i = 0; i < 8; ++i) {
    original[i * 16 + 8] = bit::bit1;
  }

  // Create array_ref with compile-time alignment 8
  bit::array_ref<bit::bit_value, 128, std::uintptr_t, 8> ref(original.data());

  // Slice [0, 64) - position preserved: starts at position 8 within word 0
  auto sliced = ref.slice<0, 64>();
  EXPECT_EQ(sliced.size(), 64);
  // sliced[0] reads position 8 of word 0 = original[8] = 1
  EXPECT_EQ(sliced[0], original[8]);

  // Slice [8, 72) - position preserved: starts at position 16 within word 0
  auto sliced2 = ref.slice<8, 72>();
  EXPECT_EQ(sliced2.size(), 64);
  // sliced2[0] reads position 16 of word 0 = original[16] = 0
  EXPECT_EQ(sliced2[0], original[16]);
}

TEST(BitArrayRef, CompileTimeFrontBackAt) {
  bit::bit_array<> original(64, bit::bit0);
  original[0] = bit::bit1;
  original[10] = bit::bit1;
  original[63] = bit::bit1;

  // Create array_ref with compile-time alignment 0
  bit::array_ref<bit::bit_value, 64, std::uintptr_t, 0> ref(original.data());

  // front() at alignment 0
  EXPECT_EQ(ref.front(), original[0]);

  // back() at last bit
  EXPECT_EQ(ref.back(), original[63]);

  // operator[] at known positions
  EXPECT_EQ(ref[0], original[0]);
  EXPECT_EQ(ref[10], original[10]);
  EXPECT_EQ(ref[63], original[63]);

  // at() at known positions
  EXPECT_EQ(ref.at(0), original[0]);
  EXPECT_EQ(ref.at(10), original[10]);
  EXPECT_EQ(ref.at(63), original[63]);
}

TEST(BitArrayRef, CompileTimeFrontBackAlignment) {
  // 128 bits of storage
  bit::bit_array<> original(128, bit::bit0);
  // Set bits at known positions
  original[0] = bit::bit1;    // bit 0 of word 0
  original[8] = bit::bit1;    // bit 8 of word 0
  original[63] = bit::bit1;   // bit 63 of word 0
  original[64] = bit::bit1;   // bit 0 of word 1
  original[71] = bit::bit1;   // bit 7 of word 1
  original[127] = bit::bit1;  // bit 63 of word 1

  // array_ref with alignment 0, 64 bits
  bit::array_ref<bit::bit_value, 64, std::uintptr_t, 0> ref0(original.data());

  // front() at alignment 0: reads bit 0 of word 0 = original[0]
  EXPECT_EQ(ref0.front(), original[0]);

  // back() at alignment 0, N=64: last bit position = (0 + 64 - 1) % 64 = 63
  // reads bit 63 of word 0 = original[63]
  EXPECT_EQ(ref0.back(), original[63]);

  // array_ref with alignment 8, 64 bits
  bit::array_ref<bit::bit_value, 64, std::uintptr_t, 8> ref8(original.data());

  // front() at alignment 8: reads bit 8 of word 0 = original[8]
  EXPECT_EQ(ref8.front(), original[8]);

  // back() at alignment 8, N=64: last bit position = (8 + 64 - 1) % 64 = 7
  // word index = (8 + 64 - 1) / 64 = 1
  // reads bit 7 of word 1 = original[64 + 7] = original[71]
  EXPECT_EQ(ref8.back(), original[71]);
}
