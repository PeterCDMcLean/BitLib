#include <benchmark/benchmark.h>

#include "benchmark_utils.hpp"
#include "bitlib/bitlib.hpp"

auto BM_BitCopy = [](benchmark::State& state, auto input) {
  using container_type = typename std::tuple_element<0, decltype(input)>::type;
  using WordType = typename std::tuple_element<1, decltype(input)>::type;
  unsigned int total_bits = std::get<2>(input);
  auto digits = bit::binary_digits<WordType>::value;
  auto container_size = total_bits / digits + 1;
  auto bitvec1 = get_random_vec<WordType>(container_size);
  auto first1 = bit::bit_iterator<decltype(std::begin(bitvec1))>(std::begin(bitvec1));
  auto bitvec2 = get_random_vec<WordType>(container_size);
  auto first2 = bit::bit_iterator<decltype(std::begin(bitvec2))>(std::begin(bitvec2));
  // some magic numbers
  auto start1 = 3;
  auto start2 = 1;
  auto end1 = 4;

  for (auto _ : state) {
    bit::copy(
        first1 + start1,
        first1 + total_bits - end1,
        first2 + start2);
  }
};

auto BM_BoolCopy = [](benchmark::State& state, auto input) {
  using container_type = typename std::tuple_element<0, decltype(input)>::type;
  unsigned int total_bits = std::get<2>(input);
  auto container_size = total_bits;
  container_type boolvec1 = make_random_container<container_type>(container_size);
  container_type boolvec2 = make_random_container<container_type>(container_size);
  auto first1 = boolvec1.begin();
  auto first2 = boolvec2.begin();

  auto start1 = 3;
  auto start2 = 1;
  auto end1 = 4;

  for (auto _ : state) {
    std::copy(
        first1 + start1,
        first1 + total_bits - end1,
        first2 + start2);
  }
};

// Copy with compile-time Alignment = 0 (word-aligned bits, no runtime position tracking)
auto BM_BitCopyAligned0 = [](benchmark::State& state, auto input) {
  using container_type = typename std::tuple_element<0, decltype(input)>::type;
  using WordType = typename std::tuple_element<1, decltype(input)>::type;
  unsigned int total_bits = std::get<2>(input);
  auto digits = bit::binary_digits<WordType>::value;
  auto container_size = total_bits / digits + 1;
  auto bitvec1 = get_random_vec<WordType>(container_size);
  auto bitvec2 = get_random_vec<WordType>(container_size);

  // Use aligned iterators (Alignment = 0): bits start at bit 0 of each word
  using aligned_iterator = bit::iterator<decltype(std::begin(bitvec1)), bit::primitive<1>, 0>;

  // Start and end at word boundaries (Alignment = 0 requires word-aligned positions)
  auto start_word = 0;
  auto end_word = total_bits / digits;

  auto first1 = aligned_iterator(std::begin(bitvec1) + start_word);
  auto last1 = aligned_iterator(std::begin(bitvec1) + end_word);
  auto first2 = aligned_iterator(std::begin(bitvec2) + start_word);

  for (auto _ : state) {
    bit::copy(first1, last1, first2);
  }
};
