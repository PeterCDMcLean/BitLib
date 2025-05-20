// ================================= BIT_SLICE =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array.hpp
// Description: Implementation of bit_array
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#include <concepts>
#include <tuple>
#include <variant>

#ifndef _BIT_SLICE_HPP_INCLUDED
#define _BIT_SLICE_HPP_INCLUDED

namespace bit {

template <typename Derived, typename T, typename W, typename It, typename CIt>
class bit_array_base;

template <std::integral size_type = uint32_t>
class bounds {
  template <typename Derived, typename T, typename W, typename It, typename CIt>
  friend class bit_array_base;
  private:
    size_type begin;
    size_type end;
    size_type step;

   public:
    using None = std::tuple<>;
    constexpr bounds() = delete;
    constexpr bounds(const bounds& other) = default;
    constexpr bounds(const bounds&& other)
        : begin(other.begin), end(other.end), step(other.step) {
    }
    constexpr bounds(const size_type& pos)
        : begin(pos), end(pos + 1), step(1) {
    }
    constexpr bounds(const size_type& begin, const size_type& end, const size_type& step = 1)
        : begin(begin), end(end), step(step) {
    }
    constexpr bounds(const None begin, const size_type& end, const size_type& step = 1)
        : begin(0), end(end), step(step) {
    }
    constexpr bounds(std::initializer_list<std::variant<None, size_type>> components) {
      if (components.size() > 3) {
        throw std::invalid_argument("Initializer list must have at most 2 elements");
      }
      auto it = components.begin();
      begin = 0;
      end = 1;
      step = 1;
      if (components.size() >= 1) {
        if (std::holds_alternative<size_type>(*it)) {
          begin = std::get<size_type>(*it);
          end = begin + 1;
        } else {
          std::cout << "None for [0]" << std::endl;
          begin = 0;
          end = 1;
        }
      }
      if (components.size() >= 2) {
        it++;
        if (std::holds_alternative<size_type>(*it)) {
          end = std::get<size_type>(*it);
        }
      }
      if (components.size() >= 3) {
        it++;
        if (std::holds_alternative<size_type>(*it)) {
          step = std::get<size_type>(*it);
        } else {
          step = 1;
        }
      }
    }

    constexpr bool operator==(const bounds& other) const = default;
    constexpr auto operator<=>(const bounds& other) const = default;

    constexpr bounds& operator+=(const size_type& size) {
      end = begin + size;
      return *this;
    }
    constexpr bounds& operator-=(const size_type& size) {
      begin = end - size;
      return *this;
    }
    constexpr size_type size() const {
      return end - begin;
    }
};

} // namespace bit

#endif
