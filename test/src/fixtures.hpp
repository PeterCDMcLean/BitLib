// =============================== FIXTURES ================================= //
// Project:         The Experimental Bit Algorithms Library
// Description:     Fixtures for testing
// Contributor(s):  Bryce Kille
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _FIXTURES_HPP_INCLUDED
#define _FIXTURES_HPP_INCLUDED
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <vector>
#include <list>
#include <forward_list>
#include <set>
#include <random>
#include <limits>
#include <cstring>
#include <cstdint>
// Project sources
#include "bitlib/bit-containers/bit-containers.hpp"
#include "bitlib/bit-containers/bit_bitsof.hpp"
#include "bitlib/bit-iterator/bit_iterator.hpp"
#include "test_utils.hpp"
// Third-party libraries
#include "gtest/gtest.h"
// Miscellaneous
// ========================================================================== //

//TODO tests need a lot of cleanup. We should only copy what we need from random_vec
//and also refactor the vec generation to reduce duplication

using BaseTypes = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t>;

template <typename WordType>
class VectorTest : public testing::Test {
 protected:
  using base_type = WordType;
  using vec_type = bit::bit_vector<WordType>;
  vec_type empty_vec;
  std::vector<bool> empty_vec_bool;
  vec_type v2_ = vec_type(18);
  vec_type v3_ = vec_type("010111111");

  std::vector<vec_type> random_bitvecs;
  std::vector<std::vector<bool>> random_boolvecs;
  std::vector<WordType> random_vec;
  void SetUp() override {
    const size_t word_size = 4;
    const size_t bit_size = word_size * bit::binary_digits<WordType>::value;
    const size_t digits = bit::binary_digits<WordType>::value;
    empty_vec = vec_type();
    random_vec = get_random_vec<WordType>(word_size);
    for (size_t cont_size = 1; cont_size < bit_size; ++cont_size) {
      auto bitvec = vec_type(bit_size);
      std::memcpy(&(*bitvec.begin().base()), &(random_vec[0]), word_size);
      bitvec.resize(cont_size);

      auto boolvec = boolvec_from_bitvec(bitvec);
      random_bitvecs.push_back(bitvec);
      random_boolvecs.push_back(boolvec);
    }
    size_t big_size = 64 * 64 * 10;
    for (int i = -4; i < 4; ++i) {
      size_t cont_size = big_size + i;
      auto bitvec = vec_type(bit_size);
      std::memcpy(&(*bitvec.begin().base()), &(random_vec[0]), word_size);
      bitvec.resize(cont_size);

      auto boolvec = boolvec_from_bitvec(bitvec);
      random_bitvecs.push_back(bitvec);
      random_boolvecs.push_back(boolvec);
    }
  }
};
TYPED_TEST_SUITE(VectorTest, BaseTypes);

template<typename WordType>
class SingleRangeTest : public testing::Test {
 protected:
  using base_type = WordType;

  std::vector<bit::bit_vector<WordType>> random_bitvecs;
  std::vector<std::vector<bool>> random_boolvecs;
  std::vector<WordType> random_vec;

 protected:
  void SetUp() override {
    const size_t digits = bit::binary_digits<WordType>::value;
    size_t word_size = 4;
    size_t bit_size = word_size * digits;
    random_vec = get_random_vec<WordType>(word_size);
    for (size_t cont_size = 1; cont_size < bit_size; ++cont_size) {
      auto bitvec = bit::bit_vector<WordType>(bit_size);
      std::copy(random_vec.begin(), random_vec.end(), bitvec.begin().base());
      bitvec.resize(cont_size);

      auto boolvec = boolvec_from_bitvec(bitvec);
      random_bitvecs.push_back(bitvec);
      random_boolvecs.push_back(boolvec);
    }
    word_size = 2 * 64 * 64;
    bit_size = word_size * digits;
    random_vec = get_random_vec<WordType>(word_size);
    for (size_t cont_size = bit_size - digits - 4; cont_size < bit_size - digits + 4; ++cont_size) {
      auto bitvec = bit::bit_vector<WordType>(bit_size);
      std::copy(random_vec.begin(), random_vec.end(), bitvec.begin().base());
      bitvec.resize(cont_size);

      auto boolvec = boolvec_from_bitvec(bitvec);
      random_bitvecs.push_back(bitvec);
      random_boolvecs.push_back(boolvec);
    }

    auto zeros = bit::bit_vector<WordType>(bit_size);
    std::fill(zeros.begin(), zeros.end(), bit::bit0);
    *(zeros.end() - 1024 - digits - 4) = bit::bit1;
    random_bitvecs.push_back(zeros);
    random_boolvecs.push_back(boolvec_from_bitvec(zeros));

    auto ones = bit::bit_vector<WordType>(bit_size);
    std::fill(ones.begin(), ones.end(), bit::bit1);
    *(ones.end() - 1024 - digits - 4) = bit::bit0;
    random_bitvecs.push_back(ones);
    random_boolvecs.push_back(boolvec_from_bitvec(ones));
  }
};
TYPED_TEST_SUITE(SingleRangeTest, BaseTypes);

template<typename WordType>
class DoubleRangeTest : public testing::Test {
    protected:

    using base_type = WordType;

    std::vector<bit::bit_vector<WordType>> random_bitvecs1;
    std::vector<bit::bit_vector<WordType>> random_bitvecs2;
    std::vector<std::vector<bool>> random_boolvecs1;
    std::vector<std::vector<bool>> random_boolvecs2;
    std::vector<WordType> random_vec;
    std::vector<WordType> random_vec_big;

    void SetUp() override {
      const size_t word_size = 4;
      const size_t big_size = 64 * 64 * 2;
      const size_t digits = bit::binary_digits<WordType>::value;
      const size_t bit_size = word_size * digits;

      // TODO this is ugly, need to refactor
      random_vec = get_random_vec<WordType>(word_size);
      random_vec_big = get_random_vec<WordType>(big_size);
      for (size_t cont_size = 1; cont_size < bit_size; ++cont_size) {
        auto bitvec = bit::bit_vector<WordType>(bit_size);
        std::copy(random_vec.begin(), random_vec.end(), bitvec.begin().base());
        bitvec.resize(cont_size);

        auto boolvec = boolvec_from_bitvec(bitvec);
        random_bitvecs1.push_back(bitvec);
        random_boolvecs1.push_back(boolvec);
      }
        for (int i = -4; i < 4; ++i) {
            size_t cont_size = (big_size-1)*digits + i;
            auto bitvec = bit::bit_vector<WordType>(big_size*digits);
            std::copy(random_vec_big.begin(), random_vec_big.end(), bitvec.begin().base());
            bitvec.resize(cont_size);

            auto boolvec = boolvec_from_bitvec(bitvec);
            random_bitvecs1.push_back(bitvec);
            random_boolvecs1.push_back(boolvec);
        }
        random_vec = get_random_vec<WordType>(word_size);
        random_vec_big = get_random_vec<WordType>(big_size);
        for (size_t cont_size = 1; cont_size < bit_size; ++cont_size) {
            auto bitvec = bit::bit_vector<WordType>(bit_size);
            std::copy(random_vec.begin(), random_vec.end(), bitvec.begin().base());
            bitvec.resize(cont_size);

            auto boolvec = boolvec_from_bitvec(bitvec);
            random_bitvecs2.push_back(bitvec);
            random_boolvecs2.push_back(boolvec);
        }
        for (int i = -4; i < 4; ++i) {
            size_t cont_size = (big_size-1)*digits + i;
            auto bitvec = bit::bit_vector<WordType>(big_size*digits);
            std::copy(random_vec_big.begin(), random_vec_big.end(), bitvec.begin().base());
            bitvec.resize(cont_size);

            auto boolvec = boolvec_from_bitvec(bitvec);
            random_bitvecs2.push_back(bitvec);
            random_boolvecs2.push_back(boolvec);
        }
    }
};
TYPED_TEST_SUITE(DoubleRangeTest, BaseTypes);
template <typename FromToPair>
class MixedDoubleRangeTest : public testing::Test {
 protected:
  using FromWordType = typename FromToPair::first_type;
  using ToWordType = typename FromToPair::second_type;

  static constexpr size_t word_size = 4;
  static constexpr size_t big_size = 64 * 64 * 2;

  // test data
  std::vector<bit::bit_vector<FromWordType>> random_bitvecs1;
  std::vector<bit::bit_vector<ToWordType>> random_bitvecs2;
  std::vector<std::vector<bool>> random_boolvecs1, random_boolvecs2;

  void SetUp() override {
    // templated lambda to fill any bit-vector / bool-vector pair
    auto fill = [&]<typename W>(
                    std::vector<bit::bit_vector<W>>& bitvecs,
                    std::vector<std::vector<bool>>& boolvecs,
                    std::vector<W> const& small_words,
                    std::vector<W> const& big_words) {
      const size_t digits = bit::bitsof<W>();
      const size_t num_small_words = ((word_size * std::max(sizeof(FromWordType), sizeof(ToWordType))) / sizeof(W));
      const size_t num_big_words = ((big_size * std::max(sizeof(FromWordType), sizeof(ToWordType))) / sizeof(W));
      const size_t small_bit_size = num_small_words * digits;
      const size_t big_bit_size = num_big_words * digits;
      assert(small_words.size() == num_small_words);
      assert(big_words.size() == num_big_words);
      // small-size runs
      for (size_t cont_size = 1; cont_size < small_bit_size; ++cont_size) {
        bit::bit_vector<W> bv(small_bit_size);
        std::copy(small_words.begin(), small_words.end(), bv.begin().base());
        bv.resize(cont_size);
        bitvecs.push_back(bv);
        boolvecs.push_back(boolvec_from_bitvec(bv));
      }
      // "big" runs around (big_size-1)*digits +/-4
      const auto max_digits = std::max(bit::bitsof<FromWordType>(), bit::bitsof<ToWordType>());
      for (int i = -4; i < 4; ++i) {
        size_t cont_size = big_bit_size - max_digits + i;
        bit::bit_vector<W> bv(big_bit_size);
        std::copy(big_words.begin(), big_words.end(),
                  bv.begin().base());
        bv.resize(cont_size);
        bitvecs.push_back(bv);
        boolvecs.push_back(boolvec_from_bitvec(bv));
      }
    };

    // — fill for FromWordType —
    const auto small_words1 = (word_size * std::max(sizeof(FromWordType), sizeof(ToWordType))) / sizeof(FromWordType);
    const auto big_words1 = (big_size * std::max(sizeof(FromWordType), sizeof(ToWordType))) / sizeof(FromWordType);
    auto small1 = get_random_vec<FromWordType>(small_words1);
    auto big1 = get_random_vec<FromWordType>(big_words1);
    fill.template operator()<FromWordType>(
        random_bitvecs1,
        random_boolvecs1,
        small1, big1);

    // — fill for ToWordType —
    const auto small_words2 = (word_size * std::max(sizeof(FromWordType), sizeof(ToWordType))) / sizeof(ToWordType);
    const auto big_words2 = (big_size * std::max(sizeof(FromWordType), sizeof(ToWordType))) / sizeof(ToWordType);
    auto small2 = get_random_vec<ToWordType>(small_words2);
    auto big2 = get_random_vec<ToWordType>(big_words2);
    fill.template operator()<ToWordType>(
        random_bitvecs2,
        random_boolvecs2,
        small2, big2);

    // now random_bitvecs1.size() == random_bitvecs2.size()
  }
};

// -----------------------------------------------------------------------------
// Instantiate for the pairs you care about
// -----------------------------------------------------------------------------
using TypePairs = ::testing::Types<
    std::pair<uint8_t, uint8_t>,
    std::pair<uint8_t, uint16_t>,
    std::pair<uint8_t, uint32_t>,
    std::pair<uint8_t, uint64_t>,
    std::pair<uint16_t, uint8_t>,
    std::pair<uint16_t, uint16_t>,
    std::pair<uint16_t, uint32_t>,
    std::pair<uint16_t, uint64_t>,
    std::pair<uint32_t, uint8_t>,
    std::pair<uint32_t, uint16_t>,
    std::pair<uint32_t, uint32_t>,
    std::pair<uint32_t, uint64_t>,
    std::pair<uint64_t, uint8_t>,
    std::pair<uint64_t, uint16_t>,
    std::pair<uint64_t, uint32_t>,
    std::pair<uint64_t, uint64_t>>;

TYPED_TEST_SUITE(MixedDoubleRangeTest, TypePairs);

// ========================================================================== //
#endif // _FIXTURES_HPP_INCLUDED
// ========================================================================== //
