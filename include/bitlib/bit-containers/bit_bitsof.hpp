#ifndef _BIT_BITSOF_HPP_INCLUDED
#define _BIT_BITSOF_HPP_INCLUDED

#include <span>
#include <concepts>

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

// Concept to check for a public static constexpr size_t bits member.
template<typename T>
concept HasBits = requires {
    { T::bits } -> std::convertible_to<std::size_t>;
};

// General case: for types that do not have a bits member.
template <typename T> requires (!HasBits<T>)
constexpr std::size_t bitsof() noexcept {
  return 8u * sizeof(T);
}

// Overload for types with a bits member.
template <typename T> requires HasBits<T>
constexpr std::size_t bitsof() noexcept {
  return T::bits;
}

// Overload for objects of any non-bits type.
template <typename T>
constexpr std::size_t bitsof(const T&) noexcept{
  return bitsof<T>();
}

} // namespace bit

#endif
