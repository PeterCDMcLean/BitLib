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

template <std::signed_integral size_type = int>
class bounds {
  template <typename Derived, typename T, typename W, typename It, typename CIt>
  friend class bit_array_base;

 public:
  using None_t = std::tuple<>;
  static constexpr None_t None = None_t{};

 private:
  using var_t = std::variant<None_t, size_type>;
  var_t begin;
  var_t end;

 public:
  constexpr bounds() : begin(None), end(None) {
  }
  constexpr bounds(const bounds& other) = default;
  constexpr bounds(const bounds&& other)
      : begin(other.begin), end(other.end) {
  }
  constexpr bounds(const size_type pos)
      : begin(pos), end(None) {
  }
  constexpr bounds(const size_type begin, const size_type end)
      : begin(begin), end(end) {
  }
  constexpr bounds(std::initializer_list<var_t> components) {
    if (components.size() > 3) {
      throw std::invalid_argument("Initializer list must have at most 2 elements");
    }
    auto it = components.begin();
    begin = None;
    end = None;
    if (components.size() >= 1) {
      begin = *it;
    }
    if (components.size() >= 2) {
      it++;
      end = *it;
    }
    if (std::holds_alternative<size_type>(begin)) {
      std::cout << "pos " << std::get<size_type>(begin) << ", ";
    } else {
      std::cout << "None, ";
    }
    if (std::holds_alternative<size_type>(end)) {
      std::cout << "pos" << std::get<size_type>(end) << std::endl;
    } else {
      std::cout << "None" << std::endl;
    }
  }

    constexpr bool operator==(const bounds& other) const = default;
    constexpr auto operator<=>(const bounds& other) const = default;

    constexpr bounds& operator+=(const size_t& _size) {
      const size_type size = static_cast<size_type>(_size);
      if (std::holds_alternative<size_type>(end)) {
        size_type end_pos = std::get<size_type>(end);
        if (end_pos >= 0) {
          end = end_pos + size;
        } else {
          end = ((size_type(-1) - size) < end_pos) ? size_type(-1) : (end_pos + size);
        }
      } else if (std::holds_alternative<size_type>(begin)) {
        end = std::get<size_type>(begin) + size;
      }
      return *this;
    }

    constexpr bounds& operator-=(const size_t& _size) {
      const size_type size = static_cast<size_type>(_size);
      if (std::holds_alternative<size_type>(begin)) {
        size_type begin_pos = std::get<size_type>(begin);
        if (!std::holds_alternative<size_type>(end)) {
          end = begin_pos + 1;
          begin = begin_pos + 1 - size;
        } else {
          if (begin_pos < 0) {
            begin = begin_pos - size;
          } else {
            begin = ((size_type(0) + size) > begin_pos) ? size_type(0) : (begin_pos - size);
          }
        }
      }
      return *this;
    }

    constexpr std::pair<size_type, size_type> resolve(size_t _length) const {
      const size_type length = static_cast<size_type>(_length);
      // Helper: translate a possibly-negative int into a signed index
      auto translate_index = [&](size_type idx) -> size_type {
        size_type x = (idx);
        if (x < size_type(0)) {
          x += length;
        }
        return x;
      };

      // 1) Compute raw_start
      size_type raw_start;
      if (std::holds_alternative<None_t>(begin)) {
        raw_start = size_type(0);
      } else {
        raw_start = translate_index(std::get<size_type>(begin));
      }

      // 2) Compute raw_end
      size_type raw_end;
      if (std::holds_alternative<None_t>(end)) {
        raw_end = length;
      } else {
        raw_end = translate_index(std::get<size_type>(end));
      }

      // 3) Clamp into [0..L]
      return {std::clamp(raw_start,
                         size_type(0),
                         length),
              std::clamp(raw_end,
                         size_type(0),
                         length)};
    }
};

} // namespace bit

#endif
