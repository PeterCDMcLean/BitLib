#include <stdexcept>
#include <utility>

#include "bitlib/bit-containers/bit_span.hpp"

#include "fixtures.hpp"

// Third-party libraries
#include "gtest/gtest.h"

namespace {
    // For these tests we use a simple word type.
    using WordType = uint8_t;
    // We assume that bit::bitsof<WordType>() returns the number of bits in a WordType.
}

// ---------- Dynamic Extent Tests ----------

TEST(BitSpanTest, DefaultConstructorDynamic) {
    bit::bit_span<WordType> span;
    EXPECT_EQ(span.size(), 0u);
    EXPECT_TRUE(span.empty());
}

TEST(BitSpanTest, ConstructorFromBitPointerDynamic) {
    WordType word = 0;
    bit::bit_pointer<WordType> ptr(&word, 0);
    bit::bit_span<WordType> span(ptr, bit::bitsof<WordType>());
    EXPECT_EQ(span.size(), bit::bitsof<WordType>());
    EXPECT_FALSE(span.empty());
}

TEST(BitSpanTest, ConstructorDeductionWordType) {
  uint32_t word = 0xDEADBEEF;
  bit::bit_span span(word);
  EXPECT_EQ(span.size(), 32);
  //0xF
  EXPECT_EQ(span[0], bit::bit1);
  EXPECT_EQ(span[1], bit::bit1);
  EXPECT_EQ(span[2], bit::bit1);
  EXPECT_EQ(span[3], bit::bit1);
  //0xE
  EXPECT_EQ(span[4], bit::bit0);
  EXPECT_EQ(span[5], bit::bit1);
  EXPECT_EQ(span[6], bit::bit1);
  EXPECT_EQ(span[7], bit::bit1);
  //0xE
  EXPECT_EQ(span[8], bit::bit0);
  EXPECT_EQ(span[9], bit::bit1);
  EXPECT_EQ(span[10], bit::bit1);
  EXPECT_EQ(span[11], bit::bit1);
  //0xB
  EXPECT_EQ(span[12], bit::bit1);
  EXPECT_EQ(span[13], bit::bit1);
  EXPECT_EQ(span[14], bit::bit0);
  EXPECT_EQ(span[15], bit::bit1);
}

TEST(BitSpanTest, ConstructorFromWordPointerDynamic) {
    WordType word = 0;
    // One word gives bit::bitsof<WordType>() bits.
    bit::bit_span<WordType> span(&word, 1);
    EXPECT_EQ(span.size(), 1);
}

TEST(BitSpanTest, ElementAccessDynamic) {
    WordType word = 0;
    bit::bit_span<WordType> span(&word, 1);
    // Initially, all bits are bit::bit0.
    EXPECT_EQ(span.front(), bit::bit0);
    EXPECT_EQ(span.back(), bit::bit0);

    // Modify first bit.
    span[0] = bit::bit1;
    EXPECT_EQ(span.front(), bit::bit1);
    EXPECT_EQ(span[0], bit::bit1);

    // Modify last bit.
    span[bit::bitsof<WordType>() - 1] = bit::bit1;
    EXPECT_EQ(span.back(), bit::bit1);
}

TEST(BitSpanTest, IteratorsDynamic) {
    WordType word = 0;
    bit::bit_span<WordType> span(&word, 1);

    // Set a pattern: even indices = bit::bit1, odd indices = bit::bit0.
    for (std::size_t i = 0; i < span.size(); ++i) {
        span[i] = (i % 2 == 0) ? bit::bit1 : bit::bit0;
    }

    std::size_t idx = 0;
    for (auto it = span.begin(); it != span.end(); ++it, ++idx) {
        if (idx % 2 == 0)
            EXPECT_EQ(*it, bit::bit1);
        else
            EXPECT_EQ(*it, bit::bit0);
    }
    EXPECT_EQ(idx, span.size());
}

TEST(BitSpanTest, SubspanDynamic) {
    WordType word = 0;
    bit::bit_span<WordType> span(&word, 1);

    // Set a known pattern: 1,0,1,0,...
    for (std::size_t i = 0; i < span.size(); ++i) {
        span[i] = (i % 2 == 0) ? bit::bit1 : bit::bit0;
    }
    // Create a subspan starting at index 2 with 4 bits.
    auto sub = span.subspan(2, 4);
    EXPECT_EQ(sub.size(), 4u);
    for (std::size_t i = 0; i < sub.size(); ++i) {
        EXPECT_EQ(sub[i], span[i + 2]);
    }
}

TEST(BitSpanTest, SubspanDefaultCountDynamic) {
    WordType word = 0;
    bit::bit_span<WordType> span(&word, bit::bitsof(word));

    // Fill span with a simple pattern.
    for (std::size_t i = 0; i < span.size(); ++i) {
        span[i] = (i % 2 == 0) ? bit::bit1 : bit::bit0;
    }
    auto sub = span.subspan(3);
    EXPECT_EQ(sub.size(), span.size() - 3);
    for (std::size_t i = 0; i < sub.size(); ++i) {
        EXPECT_EQ(sub[i], span[i + 3]);
    }
}

TEST(BitSpanTest, ConstSpanDynamic) {
    WordType word = 0;
    bit::bit_span<WordType> span(&word, 1);
    span[0] = bit::bit1;
    const bit::bit_span<WordType>& cspan = span;
    EXPECT_EQ(cspan[0], bit::bit1);
    EXPECT_EQ(cspan.front(), bit::bit1);
    EXPECT_EQ(cspan.back(), bit::bit1);
}

TEST(BitSpanTest, CopyConstructionDynamic) {
    WordType word = 0;
    bit::bit_span<WordType> span1(&word, 1);
    span1[0] = bit::bit1;
    bit::bit_span<WordType> span2 = span1;
    EXPECT_EQ(span2.size(), span1.size());
    EXPECT_EQ(span2[0], bit::bit1);
}

// ---------- Fixed Extent Tests ----------

TEST(BitSpanTest, DefaultConstructorFixed) {
    // For a fixed extent, the size is known at compile time.
    bit::bit_span<WordType, bit::bitsof<WordType>()> span;
    EXPECT_EQ(span.size(), bit::bitsof<WordType>());
    // Do not dereference the span if data is null.
}

TEST(BitSpanTest, ConstructorFromWordPointerFixed) {
    WordType word = 0;
    // For a fixed extent, one word is required.
    bit::bit_span<WordType, bit::bitsof<WordType>()> span(&word);
    EXPECT_EQ(span.size(), bit::bitsof<WordType>());
}

TEST(BitSpanTest, ElementAccessFixed) {
    WordType word = 0;
    bit::bit_span<WordType, bit::bitsof<WordType>()> span(&word);
    // Initially, all bits should be bit::bit0.
    EXPECT_EQ(span.front(), bit::bit0);
    EXPECT_EQ(span.back(), bit::bit0);

    // Modify a bit and check.
    span[3] = bit::bit1;
    EXPECT_EQ(span[3], bit::bit1);
}

TEST(BitSpanTest, IteratorsFixed) {
    WordType word = 0;
    bit::bit_span<WordType, bit::bitsof<WordType>()> span(&word);

    // Set a pattern: set bit at index 1 and 5.
    span[1] = bit::bit1;
    span[5] = bit::bit1;

    std::size_t idx = 0;
    for (auto it = span.begin(); it != span.end(); ++it, ++idx) {
        if (idx == 1 || idx == 5)
            EXPECT_EQ(*it, bit::bit1);
        else
            EXPECT_EQ(*it, bit::bit0);
    }
    EXPECT_EQ(idx, bit::bitsof<WordType>());
}

TEST(BitSpanTest, CopyConstructionFixed) {
    WordType word = 0;
    bit::bit_span<WordType, bit::bitsof<WordType>()> span1(&word);
    span1[2] = bit::bit1;
    bit::bit_span<WordType, bit::bitsof<WordType>()> span2 = span1;
    EXPECT_EQ(span2.size(), bit::bitsof<WordType>());
    EXPECT_EQ(span2[2], bit::bit1);
}

// ---------- Const Reference Alias Tests ----------

TEST(BitSpanTest, ConstReferenceAlias) {
    WordType word = 0;
    bit::bit_span<WordType> span(&word, 1);
    span[0] = bit::bit1;

    // Create a const view.
    const bit::bit_span<WordType>& cspan = span;

    // Use a static_assert to verify that the type of cspan[0]
    // is the same as the defined const_reference.
    static_assert(std::is_same_v<decltype(cspan[0]),
                  typename bit::bit_span<WordType>::const_reference>,
                  "cspan[0] is not of type const_reference");

    // At runtime, verify the value is as expected.
    EXPECT_EQ(cspan[0], bit::bit1);

    // The following assignment should fail to compile if uncommented,
    // which confirms that the const_reference is read-only.
    // cspan[0] = bit::bit0;
}
