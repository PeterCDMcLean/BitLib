// ================================= BIT_SLICE =================================== //
// Project:     The Experimental Bit Algorithms Library
// \file        bit_array.hpp
// Description: Implementation of bit_array
// Creator:     Vincent Reverdy
// Contributor: Peter McLean [2025]
// License:     BSD 3-Clause License
// ========================================================================== //
#include <concepts>

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
  public:
    constexpr bounds() = delete;
    constexpr bounds(const bounds& other) = default;
    constexpr bounds(const bounds&& other) : begin(other.begin), end(other.end) {

    }
    constexpr bounds(const size_type& pos) : begin(pos), end(pos + 1) {
    }
    constexpr bounds(const size_type& begin, const size_type& end) :
      begin(begin), end(end) {
    }
    constexpr bounds(std::initializer_list<size_type> dims) {
      if (dims.size() > 2) {
        throw std::invalid_argument("Initializer list must have at most 2 elements");
      }
      auto it = dims.begin();
      if (dims.size() >= 1) {
        begin = *it;
        end = begin + 1;
      }
      if (dims.size() >= 2) {
        it++;
        end = *it;
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
