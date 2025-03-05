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
  bit::bit_array<11> barr(bit::bit0);
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
  bit::bit_array<11> barr(bit::bit0);
  EXPECT_EQ(11u, bitsof(barr));
  EXPECT_EQ(11u, bitsof(bit::bit_array<11>()));
}

TEST(ArrayTest, BasicIteration) {
                        //  <-- LSB, apparently 🙄
  bit::bit_array<11> barr("0110_0101_110");
  int i=0;
  for(const auto& bbit : barr) {
    switch(10 - i++) {
      case  0: EXPECT_EQ(bit::bit0, bbit); break;
      case  1: EXPECT_EQ(bit::bit1, bbit); break;
      case  2: EXPECT_EQ(bit::bit1, bbit); break;
      case  3: EXPECT_EQ(bit::bit1, bbit); break;
      case  4: EXPECT_EQ(bit::bit0, bbit); break;
      case  5: EXPECT_EQ(bit::bit1, bbit); break;
      case  6: EXPECT_EQ(bit::bit0, bbit); break;
      case  7: EXPECT_EQ(bit::bit0, bbit); break;
      case  8: EXPECT_EQ(bit::bit1, bbit); break;
      case  9: EXPECT_EQ(bit::bit1, bbit); break;
      case 10: EXPECT_EQ(bit::bit0, bbit); break;
    }
  }
}

TEST(ArrayTest, ZeroSize) {
  bit::bit_array<0> barr{};
  std::array<uint8_t,0> foo{};
  EXPECT_EQ(sizeof(foo), sizeof(barr));
  EXPECT_EQ(0, bitsof(barr));
}

// Test that the default constructor initializes all bits to false.
TEST(ArrayTest, DefaultInitialization) {
    bit::bit_array<8> ba;
    for (size_t i = 0; i < ba.size(); ++i) {
        EXPECT_FALSE(ba[i]) << "Bit " << i << " should be false by default";
    }
}

// Test the fill() method.
TEST(ArrayTest, FillMethod) {
    bit::bit_array<10> ba;
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
    bit::bit_array<5> ba;
    ba.fill(bit::bit_value(false));
    ba[2] = bit::bit_value(true);
    EXPECT_TRUE(ba.at(2));
    EXPECT_THROW(ba.at(5), std::out_of_range);
}

// Test front() and back() member functions.
TEST(ArrayTest, FrontBackAccess) {
    bit::bit_array<4> ba;
    ba.fill(bit::bit_value(false));
    ba.front() = bit::bit_value(true);
    ba.back() = bit::bit_value(true);
    EXPECT_TRUE(ba.front());
    EXPECT_TRUE(ba.back());
}

// Test iterator functionality (both non-const and range-based).
TEST(ArrayTest, IteratorFunctionality) {
    bit::bit_array<4> ba;
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
    bit::bit_array<4> ba;
    ba.fill(bit::bit_value(true));
    const bit::bit_array<4>& const_ba = ba;
    for (auto it = const_ba.begin(); it != const_ba.end(); ++it) {
        EXPECT_TRUE(*it);
    }
}

// Test the swap() member function.
TEST(ArrayTest, SwapFunctionality) {
    bit::bit_array<4> ba1, ba2;
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
    bit::bit_array<5> ba1 = { bit::bit1, bit::bit1, bit::bit0, bit::bit0, bit::bit1 };
    bit::bit_array<5> ba2 = { bit::bit1, bit::bit1, bit::bit0, bit::bit0, bit::bit1 };
    EXPECT_EQ(ba1, ba2);
    ba2[2] = bit::bit_value(true);  // Change one element
    EXPECT_NE(ba1, ba2);
}

// Test the data() method to access the underlying storage.
TEST(ArrayTest, DataAccess) {
    bit::bit_array<8> ba;
    ba.fill(bit::bit_value(false));
    // Assume data() returns a pointer to a boolean array.
    uint8_t* data_ptr = ba.data();
    EXPECT_FALSE(data_ptr[0]);
    ba[0] = bit::bit_value(true);
    EXPECT_TRUE(data_ptr[0]);
}

// Test size() and empty() functions.
TEST(ArrayTest, SizeAndEmpty) {
    bit::bit_array<0> ba_empty;
    EXPECT_EQ(ba_empty.size(), 0);
    EXPECT_TRUE(ba_empty.empty());

    bit::bit_array<5> ba;
    EXPECT_EQ(ba.size(), 5);
    EXPECT_FALSE(ba.empty());
}

// Test initializer list construction.
TEST(ArrayTest, InitializerListConstruction) {
    bit::bit_array<3> ba = { bit::bit1, bit::bit0, bit::bit1 };
    EXPECT_TRUE(ba[0]);
    EXPECT_FALSE(ba[1]);
    EXPECT_TRUE(ba[2]);
}

// Test copy constructor and copy assignment operator.
TEST(ArrayTest, CopyAndAssignment) {
    bit::bit_array<5> ba1 = { bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1 };
    bit::bit_array<5> ba_copy(ba1);
    EXPECT_EQ(ba1, ba_copy);

    bit::bit_array<5> ba_assigned;
    ba_assigned = ba1;
    EXPECT_EQ(ba1, ba_assigned);
}

// Test move semantics (move constructor and move assignment), if implemented.
TEST(ArrayTest, MoveSemantics) {
    bit::bit_array<5> ba1 = { bit::bit1, bit::bit0, bit::bit1, bit::bit0, bit::bit1 };
    bit::bit_array<5> ba_moved(std::move(ba1));
    // We test the moved-to container's values. The moved-from object is valid but unspecified.
    EXPECT_TRUE(ba_moved[0]);
    EXPECT_FALSE(ba_moved[1]);
    EXPECT_TRUE(ba_moved[2]);
    EXPECT_FALSE(ba_moved[3]);
    EXPECT_TRUE(ba_moved[4]);

    bit::bit_array<5> ba2 = { bit::bit0, bit::bit0, bit::bit0, bit::bit0, bit::bit0 };
    ba2 = std::move(ba_moved);
    EXPECT_TRUE(ba2[0]);
    EXPECT_FALSE(ba2[1]);
    EXPECT_TRUE(ba2[2]);
    EXPECT_FALSE(ba2[3]);
    EXPECT_TRUE(ba2[4]);
}
