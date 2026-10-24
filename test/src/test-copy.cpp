// ============================= COPY  TESTS =============================== //
// Project:         The Experimental Bit Algorithms Library
// Description:     Tests for copy algorithms
// Contributor(s):  Bryce Kille
// License:         BSD 3-Clause License
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <math.h>
#include <algorithm>
// Project sources
#include "bitlib/bit-algorithms/bit_algorithm.hpp"
#include "bitlib/bit-containers/bit-containers.hpp"
#include "fixtures.hpp"
// Third-party libraries
#include "gtest/gtest.h"
// Miscellaneous
// ========================================================================== //

TYPED_TEST(DoubleRangeTest, Copy) {
    for (size_t idx = 0; idx < this->random_bitvecs1.size(); ++idx) {
        using WordType = typename TestFixture::base_type;
        bit::bit_vector<WordType>& bitvec1 = this->random_bitvecs1[idx];
        bit::bit_vector<WordType>& bitvec2 = this->random_bitvecs2[idx];
        constexpr auto digits = bit::binary_digits<WordType>::value;
        std::vector<bool>& boolvec1 = this->random_boolvecs1[idx];
        std::vector<bool>& boolvec2 = this->random_boolvecs2[idx];
        long long start1 = generate_random_number(
            0,
            std::min<long long>(bitvec1.size() - 1, digits + 1));
        long long start2 = generate_random_number(
            0,
            std::min<long long>(bitvec2.size() - 1, digits + 1));
        const auto min_range = (start2 > start1) ? start2 - start1 : 0;
        const auto max_range = std::max<long long>(
            min_range,
            std::min<long long>(digits, bitvec1.size() - start1));
        long long end1 = generate_random_number(min_range, max_range);

        auto bitret = bit::copy(
            bitvec1.begin() + start1,
            bitvec1.end() - end1,
            bitvec2.begin() + start2);
        auto boolret = std::copy(
            boolvec1.begin() + start1,
            boolvec1.end() - end1,
            boolvec2.begin() + start2);
        EXPECT_EQ(
            bit::distance(bitvec2.begin(), bitret),
            std::distance(boolvec2.begin(), boolret));
        EXPECT_TRUE(std::equal(
                    bitvec2.begin(), bitvec2.end(),
                    boolvec2.begin(), boolvec2.end(), comparator)
        );
        start2 = generate_random_number(0, start1);
        bitret = bit::copy(
            bitvec1.begin() + start1,
            bitvec1.end() - end1,
            bitvec1.begin() + start2);
        boolret = std::copy(
            boolvec1.begin() + start1,
            boolvec1.end() - end1,
            boolvec1.begin() + start2);
        EXPECT_EQ(
            bit::distance(bitvec1.begin(), bitret),
            std::distance(boolvec1.begin(), boolret));
        EXPECT_TRUE(std::equal(
                    bitvec1.begin(), bitvec1.end(),
                    boolvec1.begin(), boolvec1.end(), comparator)
        );
    }
}

TYPED_TEST(MixedDoubleRangeTest, Copy) {
  for (size_t idx = 0; idx < this->random_bitvecs1.size(); ++idx) {
    bit::bit_vector<typename TestFixture::FromWordType>& bitvec1 = this->random_bitvecs1[idx];
    bit::bit_vector<typename TestFixture::ToWordType>& bitvec2 = this->random_bitvecs2[idx];
    constexpr auto min_digits = std::min(bit::bitsof<typename TestFixture::FromWordType>(), bit::bitsof<typename TestFixture::ToWordType>());
    std::vector<bool>& boolvec1 = this->random_boolvecs1[idx];
    std::vector<bool>& boolvec2 = this->random_boolvecs2[idx];
    long long start1 = generate_random_number(
        0,
        std::min<long long>(bitvec1.size() - 1, min_digits + 1));
    long long start2 = generate_random_number(
        0,
        std::min<long long>(bitvec2.size() - 1, min_digits + 1));
    const auto min_range = (start2 > start1) ? start2 - start1 : 0;
    const auto max_range = std::max<long long>(
        min_range,
        std::min<long long>(min_digits, bitvec1.size() - start1));
    long long end1 = generate_random_number(min_range, max_range);

    auto bitret2 = bit::copy(
        bitvec1.begin() + start1,
        bitvec1.end() - end1,
        bitvec2.begin() + start2);
    auto boolret = std::copy(
        boolvec1.begin() + start1,
        boolvec1.end() - end1,
        boolvec2.begin() + start2);
    EXPECT_EQ(
        bit::distance(bitvec2.begin(), bitret2),
        std::distance(boolvec2.begin(), boolret));
    EXPECT_TRUE(std::equal(
        bitvec2.begin(), bitvec2.end(),
        boolvec2.begin(), boolvec2.end(), comparator));
    start2 = generate_random_number(0, start1);
    auto bitret1 = bit::copy(
        bitvec1.begin() + start1,
        bitvec1.end() - end1,
        bitvec1.begin() + start2);
    boolret = std::copy(
        boolvec1.begin() + start1,
        boolvec1.end() - end1,
        boolvec1.begin() + start2);
    EXPECT_EQ(
        bit::distance(bitvec1.begin(), bitret1),
        std::distance(boolvec1.begin(), boolret));
    EXPECT_TRUE(std::equal(
        bitvec1.begin(), bitvec1.end(),
        boolvec1.begin(), boolvec1.end(), comparator));
  }
}

TEST(Copy, ImplicitConvert) {
  std::array<uint8_t, 10> words = {0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x1A, 0x1B, 0x1C, 0x1D};
  bit::bit_array<>
      bits(5 * 8 + 4, bit::bit0);
  bit::copy(words.begin(), words.begin() + 5, bits.begin() + 4);
  EXPECT_EQ(bits, 0x2C'0E0D0C0B0A0_b);
}

// Fixed (non-dynamic_extent) Alignment iterators: same aligned source and
// destination.
TEST(Copy, AlignedSourceToAlignedDestination) {
  std::array<uint8_t, 4> src = {0xA5, 0x3C, 0x77, 0xE1};
  std::array<uint8_t, 4> got{};
  using aligned_iterator = bit::iterator<uint8_t*, bit::primitive<1>, 0>;

  bit::copy(aligned_iterator(src.data()),
            aligned_iterator(src.data() + src.size()),
            aligned_iterator(got.data()));

  EXPECT_EQ(got, src);
}

// first and last may have different fixed Alignments, and a raw pointer is a
// valid (word-aligned) destination.
TEST(Copy, AlignedRangeWithUnalignedEndToRawPointer) {
  std::array<uint8_t, 4> src = {0xA5, 0x3C, 0x77, 0xE1};
  std::array<uint8_t, 4> got{};
  std::array<uint8_t, 4> want{};
  using begin_iterator = bit::iterator<const uint8_t*, bit::primitive<1>, 0>;
  using end_iterator = bit::iterator<const uint8_t*, bit::primitive<1>, 4>;

  bit::copy(begin_iterator(src.data()), end_iterator(src.data() + 2),
            got.data());

  // Dynamic-extent oracle: same range and destination, without hints.
  bit::iterator<const uint8_t*> first(src.data());
  bit::iterator<const uint8_t*> last(src.data() + 2, 4);
  bit::iterator<uint8_t*> d_first(want.data());
  bit::copy(first, last, d_first);

  EXPECT_EQ(got, want);
}

// An aligned source that is advanced by the partial first destination word
// must not keep using the compile-time Alignment hint for the remainder.
TEST(Copy, AlignedSourceToUnalignedDestination) {
  std::array<uint8_t, 4> src = {0xB5, 0xCA, 0x73, 0x9D};
  std::array<uint8_t, 4> got{0x07, 0, 0, 0};
  std::array<uint8_t, 4> want{0x07, 0, 0, 0};
  using begin_iterator = bit::iterator<const uint8_t*, bit::primitive<1>, 0>;
  using end_iterator = bit::iterator<const uint8_t*, bit::primitive<1>, 4>;
  using dest_iterator = bit::iterator<uint8_t*, bit::primitive<1>, 3>;

  bit::copy(begin_iterator(src.data()), end_iterator(src.data() + 2),
            dest_iterator(got.data()));

  bit::iterator<const uint8_t*> first(src.data());
  bit::iterator<const uint8_t*> last(src.data() + 2, 4);
  bit::iterator<uint8_t*> d_first(want.data(), 3);
  bit::copy(first, last, d_first);

  EXPECT_EQ(got, want);
}

// Fixed-Alignment iterators whose word types differ must still work through
// the cross-width word adapter.
TEST(Copy, AlignedIteratorsWithWordAdapter) {
  std::array<uint8_t, 4> narrow = {0xA5, 0x3C, 0x77, 0xE1};
  std::array<uint16_t, 2> wide_got{};
  std::array<uint16_t, 2> wide_want{};

  using narrow_iterator = bit::iterator<const uint8_t*, bit::primitive<1>, 0>;
  using wide_iterator = bit::iterator<uint16_t*, bit::primitive<1>, 0>;

  bit::copy(narrow_iterator(narrow.data()),
            narrow_iterator(narrow.data() + 3), wide_iterator(wide_got.data()));

  bit::iterator<const uint8_t*> first(narrow.data());
  bit::iterator<const uint8_t*> last(narrow.data() + 3);
  bit::iterator<uint16_t*> d_first(wide_want.data());
  bit::copy(first, last, d_first);

  EXPECT_EQ(wide_got, wide_want);

  std::array<uint16_t, 3> wide = {0xA5C3, 0x1E77, 0x9D4B};
  std::array<uint8_t, 4> narrow_got{};
  std::array<uint8_t, 4> narrow_want{};

  using wide_src_iterator = bit::iterator<const uint16_t*, bit::primitive<1>, 0>;
  using narrow_dst_iterator = bit::iterator<uint8_t*, bit::primitive<1>, 0>;

  bit::copy(wide_src_iterator(wide.data()),
            wide_src_iterator(wide.data() + 2),
            narrow_dst_iterator(narrow_got.data()));

  bit::iterator<const uint16_t*> wide_first(wide.data());
  bit::iterator<const uint16_t*> wide_last(wide.data() + 2);
  bit::iterator<uint8_t*> narrow_d_first(narrow_want.data());
  bit::copy(wide_first, wide_last, narrow_d_first);

  EXPECT_EQ(narrow_got, narrow_want);
}

// Non-zero fixed Alignments must survive translation through the word adapter.
TEST(Copy, AlignedIteratorsWithWordAdapterAndOffsets) {
  std::array<uint8_t, 4> narrow = {0xA5, 0x3C, 0x77, 0xE1};
  std::array<uint16_t, 3> wide_got{};
  std::array<uint16_t, 3> wide_want{};

  using narrow_begin = bit::iterator<const uint8_t*, bit::primitive<1>, 3>;
  using narrow_end = bit::iterator<const uint8_t*, bit::primitive<1>, 6>;
  using wide_dest = bit::iterator<uint16_t*, bit::primitive<1>, 2>;

  // 19 bits: starts at bit 3 of a uint8_t word, ends at bit 6 of word 2.
  bit::copy(narrow_begin(narrow.data()), narrow_end(narrow.data() + 2),
            wide_dest(wide_got.data()));

  bit::iterator<const uint8_t*> first(narrow.data(), 3);
  bit::iterator<const uint8_t*> last(narrow.data() + 2, 6);
  bit::iterator<uint16_t*> d_first(wide_want.data(), 2);
  bit::copy(first, last, d_first);

  EXPECT_EQ(wide_got, wide_want);

  std::array<uint16_t, 4> wide = {0xA5C3, 0x1E77, 0x9D4B, 0x62F0};
  std::array<uint8_t, 6> narrow_got{};
  std::array<uint8_t, 6> narrow_want{};

  using wide_begin = bit::iterator<const uint16_t*, bit::primitive<1>, 5>;
  using wide_end = bit::iterator<const uint16_t*, bit::primitive<1>, 10>;
  using narrow_dest = bit::iterator<uint8_t*, bit::primitive<1>, 1>;

  // 37 bits: starts at bit 5 of a uint16_t word, ends at bit 10 of word 2.
  bit::copy(wide_begin(wide.data()), wide_end(wide.data() + 2),
            narrow_dest(narrow_got.data()));

  bit::iterator<const uint16_t*> wide_first(wide.data(), 5);
  bit::iterator<const uint16_t*> wide_last(wide.data() + 2, 10);
  bit::iterator<uint8_t*> narrow_d_first(narrow_want.data(), 1);
  bit::copy(wide_first, wide_last, narrow_d_first);

  EXPECT_EQ(narrow_got, narrow_want);
}
