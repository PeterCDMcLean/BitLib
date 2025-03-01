#ifndef _BIT_BITSOF_HPP_INCLUDED
#define _BIT_BITSOF_HPP_INCLUDED

/*
A bitsof() template function that resembles sizeof() as much as possible.
Possible calling styles:
  - `bitsof(_var_)`
  - `bitsof<_type>()`
  - `bitsof(_type())` <- Default constructor required
  - `bitsof(_type{})` <- Default constructor required
When calling within template code with a type, the callee must use the `bitsof<type>()` or `bitsof(type())` form
*/

namespace bit {

// Forward declare
template <std::size_t N>
class bit_array;

// Helper trait to detect bit_array types
template <typename T>
struct is_bit_array : std::false_type {};

template <std::size_t N>
struct is_bit_array<bit_array<N>> : std::true_type {};

template <typename T>
struct is_bit_vector : std::false_type {};

template <class WordType, class Allocator>
struct is_bit_vector<bit_vector<WordType, Allocator>> : std::true_type {};

// General case: for types that are not bit_array, bit_vector, or bit_value.
template <typename T,
          typename std::enable_if<!is_bit_array<T>::value &&
                                  !std::is_same<T, bit_value>::value &&
                                  !is_bit_vector<T>::value, int>::type = 0>
constexpr std::size_t bitsof() noexcept {
  return 8u * sizeof(T);
}

// Overload for bit_value.
template <typename T,
          typename std::enable_if<std::is_same<T, bit_value>::value, int>::type = 0>
constexpr std::size_t bitsof() noexcept {
  return 1;
}

// Overload for bit_array types.
template <typename T,
          typename std::enable_if<is_bit_array<T>::value, int>::type = 0>
constexpr std::size_t bitsof() noexcept {
  return T::bits;
}

// Overload for bit_vector
template <typename T,
          typename std::enable_if<is_bit_vector<T>::value, int>::type = 0>
std::size_t bitsof(const T& v) noexcept {
  return v.size();
}

template <typename T, typename std::enable_if<!is_bit_vector<T>::value, int>::type = 0>
constexpr std::size_t bitsof(const T&) noexcept {return bitsof<T>();}

}

#endif
