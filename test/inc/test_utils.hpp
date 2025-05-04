// ============================= TEST_UTILS ================================ //
// Project:         The Experimental Bit Algorithms Library
// Name:            test_utils.hpp
// Description:     General utilities for testing
// Creator:         Bryce Kille
// Contributor(s):  Bryce Kille [2019],
//                  Collin Gress [2019]
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _TEST_UTILS_HPP_INCLUDED
#define _TEST_UTILS_HPP_INCLUDED
// ========================================================================== //


// ================================ PREAMBLE ================================ //
// C++ standard library
#include <gtest/gtest.h>

#include <forward_list>
#include <iostream>
#include <list>
#include <random>
#include <string>
#include <vector>
// Project sources
#include "bitlib/bitlib.hpp"
// Third-party libraries
// Miscellaneous


// =============================== DEBUGGING ================================ //

// ================================ TESTING ================================- //

// Comparator for bit_iterator to other iterators
constexpr auto comparator = [](auto b1, auto b2){
    return static_cast<bool>(b1) == static_cast<bool>(b2);
};

// Helper struct for rebind_container
template <class Container, class NewType> struct rebind_container;

// Helper struct for rebind_container
template <class T,
          class Alloc,
          template <class, class, class...> class Container,
          class NewType,
          class... Parameters
         >
struct rebind_container<Container<T, Alloc, Parameters...>, NewType>
{
     typedef Container<NewType, typename Alloc::template rebind<NewType>::other > type;
};


// Takes in Container<T> and returns Container<bool> of the same underlying bits
template <class Container, typename T = typename Container::value_type>
auto bitcont_to_boolcont(const Container bitcont){
    auto bfirst = bit::bit_iterator<decltype(std::begin(bitcont))>(std::begin(bitcont));
    auto blast = bit::bit_iterator<decltype(std::end(bitcont))>(std::end(bitcont));
    typename rebind_container<Container, bool>::type c(std::distance(bfirst,
                                                                     blast)
    );
    auto benchmark_it = std::begin(c);
    auto it = bfirst;
    for (; it != blast; ++it, ++benchmark_it) {
        *benchmark_it = static_cast<bool>(*it);
    }
    return c;
}

inline std::mt19937 GetSeededRNGFromTestName() {
  auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  if (test_info == nullptr) {
    std::string full_test_name = std::string(test_info->test_suite_name()) + "." + test_info->name();

    // Hash the name to get a seed
    std::size_t hash_value = std::hash<std::string>{}(full_test_name);

    // Use the hash as a seed
    return std::mt19937(static_cast<unsigned int>(hash_value));
  } else {
    std::random_device rnd_device;
    // Specify the engine and distribution.
    return std::mt19937(rnd_device());
  }
}

inline unsigned long long generate_random_number(size_t min, size_t max) {
  std::mt19937 mersenne_engine = GetSeededRNGFromTestName();
  std::uniform_int_distribution<unsigned long long> dist{min, max};

  return dist(mersenne_engine);
}

template <typename WordType, std::size_t N>
std::array<WordType, N> get_random_arr(
    WordType min = std::numeric_limits<WordType>::min(),
    WordType max = std::numeric_limits<WordType>::max()) {
  // Specify the engine and distribution.
  std::mt19937 mersenne_engine = GetSeededRNGFromTestName();
  std::uniform_int_distribution<WordType> dist{min, max};

  auto gen = [&dist, &mersenne_engine]() {
    return dist(mersenne_engine);
  };
  std::array<WordType, N> arr{};
  generate(begin(arr), end(arr), gen);
  return arr;
}

template <typename WordType>
std::vector<WordType> get_random_vec(
        unsigned long long int size,
        WordType min = std::numeric_limits<WordType>::min(),
        WordType max = std::numeric_limits<WordType>::max()
) {
  std::mt19937 mersenne_engine = GetSeededRNGFromTestName();
  std::uniform_int_distribution<WordType> dist{min, max};

  auto gen = [&dist, &mersenne_engine]() {
    return dist(mersenne_engine);
  };
  std::vector<WordType> vec(size);
  generate(begin(vec), end(vec), gen);
  return vec;
}

template <typename WordType>
bit::bit_vector<WordType> get_random_bvec(
        unsigned long long int size
) {
    constexpr auto digits = bit::binary_digits<WordType>::value;
    auto words = (size + digits - 1) / digits;
    auto word_vec = get_random_vec(words);
    bit::bit_vector<WordType> ret(word_vec.begin(), word_vec.end());
    ret.resize(size);
    return ret;
}

template <typename WordType>
std::vector<bool> boolvec_from_bitvec(bit::bit_vector<WordType> bv) {
    std::vector<bool> ret_vec{};
    for (bit::bit_value value : bv) {
       ret_vec.push_back(value == bit::bit1 ? true : false);
    }
    return ret_vec;
}

// ========================================================================== //
#endif // _TEST_UTILS_HPP_INCLUDED
// ========================================================================== //
