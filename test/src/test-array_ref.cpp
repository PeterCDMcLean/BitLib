#include <limits>

#include "bitlib/bitlib.hpp"
#include "gtest/gtest.h"

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
  bit::bit_pointer<std::uint8_t> bit_ptr(original.data());

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

TEST(BitArrayRef, OffsetBitPointerConstructor) {
  // Prepare original bit array
  bit::bit_array<> original(64, bit::bit0);
  original[3] = bit::bit1;
  original[13] = bit::bit1;

  // Create an offset bit_pointer (starting from the 3rd bit)
  bit::bit_pointer<std::uint8_t> bit_ptr(original.data(), 3);

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
}
