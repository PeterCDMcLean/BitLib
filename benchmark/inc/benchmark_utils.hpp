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

#endif
