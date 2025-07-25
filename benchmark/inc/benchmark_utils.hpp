#ifndef _BENCHMARK_UTILS_HPP_
#define _BENCHMARK_UTILS_HPP_

#include <forward_list>
#include <iostream>
#include <list>
#include <random>
#include <string>
#include <vector>

// Produces container of random numbers from min to max
template <class Container, typename T = typename Container::value_type>
Container make_random_container(
    std::size_t size,
    T min = std::numeric_limits<T>::min(),
    T max = std::numeric_limits<T>::max(),
    const T& seed = T()
)
{
    Container c(size);
    std::random_device device;
    std::mt19937 engine(seed == T() ? device() : seed);
    std::uniform_int_distribution<std::uintmax_t> distribution(min, max);
    auto it = std::begin(c);
    for (std::size_t i = 0; i < size; ++i) {
        *it = distribution(engine);
        ++it;
    }
    return c;
}

// clang-format off
template <typename T>
  requires std::integral<T>
struct uniform_dist_type {
  static constexpr bool is_signed = std::is_signed_v<T>;
  static constexpr std::size_t size = sizeof(T);

  using type = std::conditional_t<size <= 2,
                 std::conditional_t<is_signed, short, unsigned short>,
                 std::conditional_t<size == 4,
                   std::conditional_t<is_signed, int, unsigned int>,
                   std::conditional_t<size == 8,
                     std::conditional_t<is_signed, long long, unsigned long long>,
                     void  // fallback (shouldn't happen for standard integral types)
                     >>>;
};
// clang-format on

template <typename T>
using uniform_dist_type_t = typename uniform_dist_type<T>::type;

template <typename WordType>
std::vector<WordType> get_random_vec(
    unsigned long long int size,
    WordType min = std::numeric_limits<WordType>::min(),
    WordType max = std::numeric_limits<WordType>::max()) {
  std::random_device device;
  std::mt19937 mersenne_engine(device());
  std::uniform_int_distribution<uniform_dist_type_t<WordType>> dist{min, max};

  auto gen = [&dist, &mersenne_engine]() {
    return static_cast<WordType>(dist(mersenne_engine));
  };
  std::vector<WordType> vec(size);
  generate(begin(vec), end(vec), gen);
  return vec;
}

#endif
