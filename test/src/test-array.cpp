#include <stdexcept>
#include <utility>

#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_array_dynamic_extent.hpp"
#include "fixtures.hpp"

// Third-party libraries
#include "gtest/gtest.h"

/*
 * Constructor tests
 */
// Tests the default c'tor.
TEST(ArrayTest, DefaultConstructor) {
  bit::bit_array<bit::bit_value, 11> barr(bit::bit0);
  EXPECT_EQ(2, sizeof(barr));
  EXPECT_EQ(11u, barr.size());
  EXPECT_EQ(bit::bit0, barr[0]);
  EXPECT_EQ(bit::bit0, barr[1]);
  EXPECT_EQ(bit::bit0, barr[2]);
  EXPECT_EQ(bit::bit0, barr[3]);
  EXPECT_EQ(bit::bit0, barr[4]);
  EXPECT_EQ(bit::bit0, barr[5]);
  EXPECT_EQ(bit::bit0, barr[6]);
  EXPECT_EQ(bit::bit0, barr[7]);
  EXPECT_EQ(bit::bit0, barr[8]);
  EXPECT_EQ(bit::bit0, barr[9]);
  EXPECT_EQ(bit::bit0, barr[10]);
}

TEST(ArrayTest, BitsOf) {
  bit::bit_array<bit::bit_value, 11> barr(bit::bit0);
  EXPECT_EQ(11u, bitsof(barr));
  EXPECT_EQ(11u, bitsof(bit::bit_array<bit::bit_value, 11>()));
}

TEST(ArrayTest, BasicIteration) {
                        //  <-- LSB, apparently 🙄
                        bit::bit_array<bit::bit_value, 11> barr("0110_0101_110");
                        int i = 0;
                        for (const auto& bbit : barr) {
                          switch (10 - i++) {
                            case 0: EXPECT_EQ(bit::bit0, bbit); break;
                            case 1: EXPECT_EQ(bit::bit1, bbit); break;
                            case 2: EXPECT_EQ(bit::bit1, bbit); break;
                            case 3: EXPECT_EQ(bit::bit1, bbit); break;
                            case 4: EXPECT_EQ(bit::bit0, bbit); break;
                            case 5: EXPECT_EQ(bit::bit1, bbit); break;
                            case 6: EXPECT_EQ(bit::bit0, bbit); break;
                            case 7: EXPECT_EQ(bit::bit0, bbit); break;
                            case 8: EXPECT_EQ(bit::bit1, bbit); break;
                            case 9: EXPECT_EQ(bit::bit1, bbit); break;
                            case 10: EXPECT_EQ(bit::bit0, bbit); break;
                          }
                        }
}

TEST(ArrayTest, ZeroSize) {
  bit::bit_array<bit::bit_value, 0> barr{};
  std::array<uint8_t, 0> foo{};
  EXPECT_EQ(sizeof(foo), sizeof(barr));
  EXPECT_EQ(0, bitsof(barr));
}

// Test that the default constructor initializes all bits to false.
TEST(ArrayTest, DefaultInitialization) {
  bit::bit_array<bit::bit_value, 8> ba;
  for (size_t i = 0; i < ba.size(); ++i) {
    EXPECT_FALSE(ba[i]) << "Bit " << i << " should be false by default";
  }
}

// Test the fill() method.
TEST(ArrayTest, FillMethod) {
  bit::bit_array<bit::bit_value, 10> ba;
  ba.fill(bit::bit_value(true));
  for (size_t i = 0; i < ba.size(); ++i) {
    EXPECT_TRUE(ba[i]) << "Bit " << i << " should be true after fill(true)";
  }
    ba.fill(bit::bit_value(false));
    for (size_t i = 0; i < ba.size(); ++i) {
        EXPECT_FALSE(ba[i]) << "Bit " << i << " should be false after fill(false)";
    }
}

// Test element access via operator[] and at(), including out-of-range checking.
TEST(ArrayTest, ElementAccess) {
  bit::bit_array<bit::bit_value, 5> ba;
  ba.fill(bit::bit_value(false));
  ba[2] = bit::bit_value(true);
  EXPECT_TRUE(ba.at(2));
  EXPECT_THROW(ba.at(5), std::out_of_range);
}

// Test front() and back() member functions.
TEST(ArrayTest, FrontBackAccess) {
  bit::bit_array<bit::bit_value, 4> ba;
  ba.fill(bit::bit_value(false));
  ba.front() = bit::bit_value(true);
  ba.back() = bit::bit_value(true);
  EXPECT_TRUE(ba.front());
  EXPECT_TRUE(ba.back());
}

// Test iterator functionality (both non-const and range-based).
TEST(ArrayTest, IteratorFunctionality) {
  bit::bit_array<bit::bit_value, 4> ba;
  ba.fill(bit::bit_value(false));
  int index = 0;
  for (auto it = ba.begin(); it != ba.end(); ++it) {
    // Change the second element using the iterator.
    if (index == 1) {
      *it = bit::bit_value(true);
    }
    ++index;
  }
    EXPECT_TRUE(ba[1]);
}

// Test const_iterator functionality.
TEST(ArrayTest, ConstIteratorFunctionality) {
  bit::bit_array<bit::bit_value, 4> ba;
  ba.fill(bit::bit_value(true));
  const bit::bit_array<bit::bit_value, 4>& const_ba = ba;
  for (auto it = const_ba.begin(); it != const_ba.end(); ++it) {
    EXPECT_TRUE(*it);
  }
}

// Test the swap() member function.
TEST(ArrayTest, SwapFunctionality) {
  bit::bit_array<bit::bit_value, 4> ba1, ba2;
  ba1.fill(bit::bit_value(false));
  ba2.fill(bit::bit_value(true));
  ba1.swap(ba2);
  for (size_t i = 0; i < ba1.size(); ++i) {
    EXPECT_TRUE(ba1[i]) << "After swap, ba1[" << i << "] should be true";
    EXPECT_FALSE(ba2[i]) << "After swap, ba2[" << i << "] should be false";
  }
}

// Test comparison operators (== and !=).
TEST(ArrayTest, ComparisonOperators) {
  bit::bit_array<bit::bit_value, 5> ba1 = {bit::bit1, bit::bit1, bit::bit0, bit::bit0, bit::bit1};
  bit::bit_array<bit::bit_value, 5> ba2 = {bit::bit1, bit::bit1, bit::bit0, bit::bit0, bit::bit1};
  EXPECT_EQ(ba1, ba2);
  ba2[2] = bit::bit_value(true);  // Change one element
  EXPECT_NE(ba1, ba2);
  bit::bit_array<bit::bit_value, std::dynamic_extent> ba3{bit::bit1, bit::bit1, bit::bit1, bit::bit0, bit::bit0, bit::bit1};
  EXPECT_NE(ba1, ba3);
}

// Test the data() method to access the underlying storage.
TEST(ArrayTest, DataAccess) {
  bit::bit_array<bit::bit_value, 8> ba;
  ba.fill(bit::bit_value(false));
  // Assume data() returns a pointer to a boolean array.
  uint8_t* data_ptr = ba.data();
  EXPECT_FALSE(data_ptr[0]);
  ba[0] = bit::bit_value(true);
  EXPECT_TRUE(data_ptr[0]);
}

// Test size() and empty() functions.
TEST(ArrayTest, SizeAndEmpty) {
  bit::bit_array<bit::bit_value, 0> ba_empty;
  EXPECT_EQ(ba_empty.size(), 0);
  EXPECT_TRUE(ba_empty.empty());

  bit::bit_array<bit::bit_value, 5> ba;
  EXPECT_EQ(ba.size(), 5);
  EXPECT_FALSE(ba.empty());
}

// Test initializer list construction.
TEST(ArrayTest, InitializerListConstruction) {
  bit::bit_array<bit::bit_value, 3> ba = {bit::bit1, bit::bit0, bit::bit1};
  EXPECT_TRUE(ba[0]);
  EXPECT_FALSE(ba[1]);
  EXPECT_TRUE(ba[2]);
}

// Test copy constructor and copy assignment operator.
TEST(ArrayTest, CopyAndAssignment) {
  bit::bit_array<bit::bit_value, 5> ba1 = {bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1};
  bit::bit_array<bit::bit_value, 5> ba_copy(ba1);
  EXPECT_EQ(ba1, ba_copy);

  bit::bit_array<bit::bit_value, 5> ba_assigned;
  ba_assigned = ba1;
  EXPECT_EQ(ba1, ba_assigned);
}

// Test move semantics (move constructor and move assignment), if implemented.
TEST(ArrayTest, MoveSemantics) {
  bit::bit_array<bit::bit_value, 5> ba1 = {bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1};
  bit::bit_array<bit::bit_value, 5> ba_moved(std::move(ba1));
  // We test the moved-to container's values. The moved-from object is valid but unspecified.
  EXPECT_TRUE(ba_moved[0]);
  EXPECT_FALSE(ba_moved[1]);
  EXPECT_TRUE(ba_moved[2]);
  EXPECT_FALSE(ba_moved[3]);
  EXPECT_TRUE(ba_moved[4]);

  bit::bit_array<bit::bit_value, 5> ba2 = {bit::bit0, bit::bit0, bit::bit0, bit::bit0, bit::bit0};
  ba2 = std::move(ba_moved);
  EXPECT_TRUE(ba2[0]);
  EXPECT_FALSE(ba2[1]);
  EXPECT_TRUE(ba2[2]);
  EXPECT_FALSE(ba2[3]);
  EXPECT_TRUE(ba2[4]);
}

TEST(ArrayTest, Throws) {
  bit::bit_array<bit::bit_value, 5> ba1{bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1};
  EXPECT_THROW(ba1.at(5), std::out_of_range);
  bit::bit_array<bit::bit_value, std::dynamic_extent> ba2{bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1};
  bit::bit_array<bit::bit_value, 5> ba3(ba2);
  EXPECT_EQ(ba1, ba3);
  EXPECT_EQ(ba1, ba2);
  bit::bit_array<bit::bit_value, std::dynamic_extent> ba4{bit::bit1, bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1};

  using barr5 = bit::bit_array<bit::bit_value, 5>;  // command in template messes up gtest macro
  EXPECT_THROW(barr5{ba4}, std::invalid_argument);
}

TEST(BitArrayDynamicTest, Throws) {
  bit::bit_array<> ba1(5);
  ba1 = {bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1};
  EXPECT_THROW(ba1.at(5), std::out_of_range);
  bit::bit_array<> ba2(6);
  ba2 = {bit::bit0, bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1};
  EXPECT_THROW(ba1 = ba2, std::invalid_argument);
  EXPECT_THROW(ba1 = std::move(ba2), std::invalid_argument);
  EXPECT_NO_THROW(ba1 = ba2(0, 5));
}

//
// Test Suite for bit::bit_array<>
//

// As bit_array<> is dynamic, bitsof will not return the number stored bits
// the class contains a size_t and a pointer
TEST(BitArrayDynamicTest, Bitsof) {
  bit::bit_array<> arr(23, bit::bit1);
  EXPECT_EQ(bitsof(arr), 8 * sizeof(arr));
  EXPECT_EQ(arr.size(), 23);
}

TEST(BitArrayDynamicTest, SizeConstructorCreatesArrayOfGivenSize) {
  const std::size_t size = 10;
  bit::bit_array<> arr(size);
  EXPECT_EQ(arr.size(), size);
  EXPECT_FALSE(arr.empty());
}

TEST(BitArrayDynamicTest, SizeAndValueConstructorInitializesCorrectly) {
  const std::size_t size = 16;
  // Initialize all bits to true.
  bit::bit_array<> arr(size, bit::bit_value(true));
  for (std::size_t i = 0; i < size; ++i) {
    EXPECT_EQ(arr[i], bit::bit_value(true));
  }
}

TEST(BitArrayDynamicTest, CopyConstructorCopiesContent) {
  const std::size_t size = 8;
  bit::bit_array<> original(size, bit::bit_value(true));
  bit::bit_array<> copy(original);
  EXPECT_TRUE(copy == original);
}

TEST(BitArrayDynamicTest, MoveConstructorMovesContent) {
  const std::size_t size = 8;
  bit::bit_array<> original(size, bit::bit_value(true));
  bit::bit_array<> moved(std::move(original));
  // Check that moved now contains the expected content.
  EXPECT_EQ(moved.size(), size);
  for (std::size_t i = 0; i < size; ++i) {
    EXPECT_EQ(moved[i], bit::bit_value(true));
  }
  // The state of original is valid but unspecified.
}

TEST(BitArrayDynamicTest, InitializerListBitValueConstructorWorks) {
  std::initializer_list<bit::bit_value> init = {bit::bit0, bit::bit1, bit::bit0};
  bit::bit_array<> arr(init);
  EXPECT_EQ(arr.size(), init.size());
  auto it = arr.begin();
  for (bit::bit_value expected : init) {
    EXPECT_EQ(*it, expected);
    ++it;
  }
}

TEST(BitArrayDynamicTest, InitializerListWordTypeConstructorWorks) {
  // For this test, we assume that the initializer list for WordType initializes the underlying storage.
  // Here we use two bytes as an example.
  std::initializer_list<std::uint8_t> init = {0b10101010, 0b01010101};
  bit::bit_array<> arr(init);
  // Assuming each std::uint8_t provides 8 bits, we expect the size to be the number of initializer elements * 8.
  EXPECT_EQ(arr.size(), init.size() * 8u);
  // Check that the underlying storage matches the initializer values.
  const std::uint8_t* data = arr.data();
  auto wordIt = init.begin();
  for (std::size_t i = 0; i < init.size(); ++i) {
    EXPECT_EQ(data[i], *(wordIt++));
  }
}

TEST(BitArrayDynamicTest, StringViewConstructorWorks) {
  // Assuming the string_view constructor interprets a string of '0' and '1' characters.
  std::string_view s = "1011001";
  bit::bit_array<> arr(s);
  EXPECT_EQ(arr.size(), s.size());
  for (std::size_t i = 0; i < s.size(); ++i) {
    // Interpret '1' as true and '0' as false.
    bool expected = (s[i] == '1');
    EXPECT_EQ(arr[i], bit::bit_value(expected));
  }
}

TEST(BitArrayDynamicTest, ElementAccessAtAndBracketConsistency) {
  const std::size_t size = 5;
  bit::bit_array<> arr(size, bit::bit_value(false));
  // Set a value using operator[] and check with at().
  arr[2] = bit::bit_value(true);
  EXPECT_EQ(arr.at(2), bit::bit_value(true));
  EXPECT_EQ(arr[2], bit::bit_value(true));

  // Test front() and back().
  arr[0] = bit::bit_value(true);
  arr[size - 1] = bit::bit_value(true);
  EXPECT_EQ(arr.front(), bit::bit_value(true));
  EXPECT_EQ(arr.back(), bit::bit_value(true));
}

TEST(BitArrayDynamicTest, IteratorTraversal) {
  const std::size_t size = 10;
  bit::bit_array<> arr(size, bit::bit_value(false));
  // Set alternate bits to true.
  for (std::size_t i = 0; i < size; ++i) {
    arr[i] = bit::bit_value(i % 2 == 0);
  }
  std::size_t index = 0;
  for (auto it = arr.begin(); it != arr.end(); ++it, ++index) {
    bool expected = (index % 2 == 0);
    EXPECT_EQ(*it, bit::bit_value(expected));
  }
}

TEST(BitArrayDynamicTest, CapacityFunctions) {
  bit::bit_array<> arr{bit::bit0};
  EXPECT_FALSE(arr.empty());
  EXPECT_EQ(arr.size(), 1u);
  // max_size() is expected to be >= size.
  EXPECT_GE(arr.max_size(), arr.size());
}

TEST(BitArrayDynamicTest, FillOperationSetsAllBits) {
  const std::size_t size = 20;
  bit::bit_array<> arr(size, bit::bit_value(false));
  arr.fill(bit::bit_value(true));
  for (std::size_t i = 0; i < size; ++i) {
    EXPECT_EQ(arr[i], bit::bit_value(true));
  }
}

TEST(BitArrayDynamicTest, SwapOperationSwapsContents) {
  const std::size_t size1 = 8;
  const std::size_t size2 = 8;
  bit::bit_array<> arr1(size1, bit::bit_value(true));
  bit::bit_array<> arr2(size2, bit::bit_value(false));

  arr1.swap(arr2);

  EXPECT_EQ(arr1.size(), size2);
  EXPECT_EQ(arr2.size(), size1);

  for (std::size_t i = 0; i < arr1.size(); ++i) {
    EXPECT_EQ(arr1[i], bit::bit_value(false));
  }
  for (std::size_t i = 0; i < arr2.size(); ++i) {
    EXPECT_EQ(arr2[i], bit::bit_value(true));
  }
}

TEST(BitArrayDynamicTest, AssignmentOperatorCopiesContent) {
  const std::size_t size = 10;
  bit::bit_array<> arr1(size, bit::bit_value(true));
  bit::bit_array<> arr2(size);
  arr2 = arr1;
  EXPECT_TRUE(arr2 == arr1);
}

TEST(BitArrayDynamicTest, MoveAssignmentOperatorMovesContent) {
  const std::size_t size = 10;
  bit::bit_array<> arr1(size, bit::bit_value(true));
  bit::bit_array<> arr2(size);
  arr2 = std::move(arr1);
  EXPECT_EQ(arr2.size(), size);
  for (std::size_t i = 0; i < size; ++i) {
    EXPECT_EQ(arr2[i], bit::bit_value(true));
  }
  // The state of arr1 is valid but unspecified.
}

struct StructOfBitArrays {
  bit::bit_array<> arr1;
  bit::bit_array<> arr2;
  bit::bit_array<> arr3;
  StructOfBitArrays() = delete;
  StructOfBitArrays(size_t s1, size_t s2, size_t s3)
      : arr1(s1), arr2(s2), arr3(s3) {
  }
};

TEST(BitArrayDynamicTest, StructOfBitArray) {
  size_t s1 = rand();
  size_t s2 = rand();
  size_t s3 = rand();
  StructOfBitArrays s(s1, s2, s3);
  EXPECT_EQ(s.arr1.size(), s1);
  EXPECT_EQ(s.arr2.size(), s2);
  EXPECT_EQ(s.arr3.size(), s3);
}

TEST(BitArrayDynamicTest, StringConstructor) {
  bit::bit_array<> arr{"01001101"};

  EXPECT_EQ(arr.size(), 8);

  EXPECT_EQ(arr[7], bit::bit1);
  EXPECT_EQ(arr[6], bit::bit0);
  EXPECT_EQ(arr[5], bit::bit1);
  EXPECT_EQ(arr[4], bit::bit1);
  EXPECT_EQ(arr[3], bit::bit0);
  EXPECT_EQ(arr[2], bit::bit0);
  EXPECT_EQ(arr[1], bit::bit1);
  EXPECT_EQ(arr[0], bit::bit0);
}

TEST(BitArrayDynamicTest, UserDefinedLiteral) {
  auto arr = 0b01001101_b;
  EXPECT_EQ(arr.size(), 8);
  EXPECT_EQ(arr[7], bit::bit0);
  EXPECT_EQ(arr[0], bit::bit1);
  auto arr2 = 0b1'01001101_b;
  EXPECT_EQ(arr2.size(), 1);
}

TEST(BitArrayDynamicTest, UserDefinedHexLiteral) {
  auto arr = 0x010A110A_b;
  EXPECT_EQ(arr.size(), 32);
  EXPECT_EQ(arr[0], bit::bit0);
  EXPECT_EQ(arr[1], bit::bit1);
  EXPECT_EQ(arr[2], bit::bit0);
  EXPECT_EQ(arr[3], bit::bit1);
  EXPECT_EQ(arr[7], bit::bit0);
  auto arr2 = 0x19'010A110A_b;
  EXPECT_EQ(arr2.size(), 25);
}

TEST(BitArrayDynamicTest, UserDefinedDecLiteral) {
  auto arr = 16'12345_b;
  EXPECT_EQ(arr.size(), 16);
  auto arr2 = 0x3039_b;
  EXPECT_EQ(arr, arr2);
  auto arr3 = 16'123'45_b;
  EXPECT_EQ(arr, arr3);
}

TEST(BitArrayDynamicTest, TwoDBitArraySizeValueConstructor) {
  bit::bit_array<bit::bit_array<>> arr(
      16,
      bit::bit_array<>(4, bit::bit1));
  EXPECT_EQ(arr.size(), 16);
  EXPECT_EQ(arr[0].size(), 4);
}

TEST(BitArrayDynamicTest, TwoDBitArraySizeConstructor) {
  bit::bit_array<bit::bit_array<>> arr(
      16,
      bit::bit_array<>(4));
  EXPECT_EQ(arr.size(), 16);
  EXPECT_EQ(arr[0].size(), 4);
}

// Test comparison operators (== and !=).
TEST(BitArrayDynamicTest, ComparisonOperators) {
  bit::bit_array<bit::bit_value, std::dynamic_extent> ba1{bit::bit1, bit::bit1, bit::bit0, bit::bit0, bit::bit1};
  bit::bit_array<bit::bit_value, std::dynamic_extent> ba2{bit::bit1, bit::bit1, bit::bit0, bit::bit0, bit::bit1};
  EXPECT_EQ(ba1, ba2);
  ba2[2] = bit::bit_value(true);  // Change one element
  EXPECT_NE(ba1, ba2);
  bit::bit_array<bit::bit_value, std::dynamic_extent> ba3{bit::bit0, bit::bit1, bit::bit1, bit::bit0, bit::bit0, bit::bit1};
  EXPECT_NE(ba1, ba3);
}

TEST(BitArrayTest, Slice) {
  auto arr = 0x20'DEADBEEF_b;
  auto span2 = arr(4, 8);
  EXPECT_EQ(span2.size(), 4);
  EXPECT_EQ(span2[0], (0xE & (1 << 0)) ? bit::bit1 : bit::bit0);
  EXPECT_EQ(span2[1], (0xE & (1 << 1)) ? bit::bit1 : bit::bit0);
  EXPECT_EQ(span2[2], (0xE & (1 << 2)) ? bit::bit1 : bit::bit0);
  EXPECT_EQ(span2[3], (0xE & (1 << 3)) ? bit::bit1 : bit::bit0);
  span2 = 0x4'A_b;
  EXPECT_EQ(arr, 0x20'DEADBEAF_b);
}

TEST(BitArrayTest, SliceModify) {
  auto arr = 0x24'DEADBEEF_b;
  auto span2 = arr(4, 8);
  EXPECT_EQ(span2.size(), 4);
  EXPECT_EQ(span2[0], (0xE & (1 << 0)) ? bit::bit1 : bit::bit0);
  EXPECT_EQ(span2[1], (0xE & (1 << 1)) ? bit::bit1 : bit::bit0);
  EXPECT_EQ(span2[2], (0xE & (1 << 2)) ? bit::bit1 : bit::bit0);
  EXPECT_EQ(span2[3], (0xE & (1 << 3)) ? bit::bit1 : bit::bit0);
  span2[3] = bit::bit0;
  EXPECT_EQ(span2[3], bit::bit0);
  EXPECT_EQ(arr, 0x24'DEADBE6F_b);
}
