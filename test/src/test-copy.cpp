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
