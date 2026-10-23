// =============================== TRANSFORM =============================== //
// Project: The Experimental Bit Algorithms Library
// Name: transform.hpp
// Description: bit_iterator overloads for std::transform
// License: BSD 3-Clause License
// ========================================================================== //
#ifndef _TRANSFORM_HPP_INCLUDED
#define _TRANSFORM_HPP_INCLUDED
// ========================================================================== //

// ============================== PREAMBLE ================================== //
// C++ standard library
#include <functional>
#include <type_traits>
// Project sources
#include "bitlib/bit-iterator/bit.hpp"
// Third-party libraries
// Miscellaneous

namespace bit {
// ========================================================================== //

//template <class RandomAccessIt, class WordType>
//constexpr bit_iterator<RandomAccessIt> transform(
//bit_iterator<RandomAccessIt> first,
//bit_iterator<RandomAccessIt> last,
//bit_iterator<RandomAccessIt> d_first,
//std::bit_or<WordType>) {

//return d_first;
//}
struct transform_impl {
  template <class FirstIt, class LastIt, class DstIt, class UnaryOperation>
  static constexpr auto run_impl(const FirstIt& first_in, const LastIt& last_in,
                                 const DstIt& d_first_in,
                                 UnaryOperation unary_op) {
    auto first = make_mutable_bit_iterator(first_in);
    auto last = make_mutable_bit_iterator(last_in);
    auto d_first = make_mutable_bit_iterator(d_first_in);
    using in_iterator = decltype(first);
    using word_type = typename in_iterator::word_type;
    using size_type = typename in_iterator::size_type;
    constexpr size_type digits = binary_digits<word_type>::value;
    constexpr std::size_t FirstAlign = iterator_alignment_v<FirstIt>;
    constexpr std::size_t DstAlign = iterator_alignment_v<DstIt>;

    // Assertions
    _assert_range_viability(first, last);
    if (first == last) {
      return d_first;
    }

    // Initialization
    size_type d_first_position;
    if constexpr (DstAlign == 0) {
      d_first_position = 0;
    } else if constexpr (DstAlign == std::dynamic_extent) {
      d_first_position = d_first.position();
    } else {
      d_first_position = DstAlign;
    }
    const bool is_d_first_aligned = d_first_position == 0;
    size_type total_bits_to_op = distance(first, last);
    size_type remaining_bits_to_op = total_bits_to_op;
    auto it = d_first.base();

    // d_first is not aligned. Copy partial word to align it
    if (!is_d_first_aligned) {
      size_type partial_bits_to_op = ::std::min(
          remaining_bits_to_op,
          digits - d_first_position);
      word_type result;
      if constexpr (std::is_invocable_v<UnaryOperation, word_type, size_type>) {
        result = unary_op(
            static_cast<word_type>(
                get_masked_word<word_type>(first, partial_bits_to_op)
                << static_cast<word_type>(d_first_position)),
            partial_bits_to_op);
      } else {
        result = unary_op(
            static_cast<word_type>(
                get_masked_word<word_type>(first, partial_bits_to_op)
                << static_cast<word_type>(d_first_position)));
      }
      *it = _bitblend<word_type>(
          *it,
          result,
          static_cast<word_type>(d_first_position),
          static_cast<word_type>(partial_bits_to_op));
      remaining_bits_to_op -= partial_bits_to_op;
      advance(first, partial_bits_to_op);
      it++;
    }
    auto firstIt = first.base();
    if (remaining_bits_to_op > 0) {
      // Use the compile-time alignment hints where they still apply. If the
      // prefix write advanced the source, the hint may no longer describe the
      // iterator and the run-time position is consulted.
      const bool is_first_aligned = [&]() {
        if constexpr (FirstAlign != std::dynamic_extent) {
          if constexpr (DstAlign == FirstAlign) {
            return true;
          } else {
            return FirstAlign == static_cast<std::size_t>(d_first_position);
          }
        } else {
          return first.position() == 0;
        }
      }();
      //size_type words_to_op = ::std::ceil(remaining_bits_to_op / static_cast<float>(digits));
      // d_first will be aligned at this point
      if (is_first_aligned && remaining_bits_to_op > digits) {
        auto N = ::std::distance(firstIt, last.base());
#ifdef BITLIB_HWY
        if constexpr (std::is_same_v<UnaryOperation, std::bit_not<word_type>>) {
          // Align to 64 bit boundary
          for (; firstIt != last.base() && !is_aligned(&*firstIt, 64); firstIt++, it++) {
            *it = unary_op(*firstIt);
          }

          bool out_is_aligned = is_aligned(&*it, 64);

          constexpr hn::ScalableTag<word_type> d;
          for (; std::distance(firstIt, last.base()) >= hn::Lanes(d); firstIt += hn::Lanes(d), it += hn::Lanes(d)) {
            const auto v = hn::Not(hn::Load(d, &*firstIt));
            if (out_is_aligned) {
              hn::Store(v, d, &*it);
            } else {
              hn::StoreU(v, d, &*it);
            }
          }
        }
#endif
        size_t std_dist = ::std::distance(firstIt, last.base());
        for (auto in_it = firstIt; in_it != last.base(); std::advance(in_it, 1), std::advance(it, 1)) {
          if constexpr (std::is_invocable_v<UnaryOperation, word_type, size_type>) {
            *it = unary_op(*in_it, digits);
          } else {
            *it = unary_op(*in_it);
          }
        }
        //it = std::transform(firstIt, last.base(), it, unary_op);
        firstIt += std_dist;
        first = in_iterator(firstIt);
        remaining_bits_to_op -= digits * N;
      } else {
        while (remaining_bits_to_op >= digits) {
          if constexpr (std::is_invocable_v<UnaryOperation, word_type, size_type>) {
            *it = unary_op(get_word<word_type>(first, digits), digits);
          } else {
            *it = unary_op(get_word<word_type>(first, digits));
          }

          remaining_bits_to_op -= digits;
          it++;
          advance(first, digits);
        }
      }
      if (remaining_bits_to_op > 0) {
        word_type result;
        if constexpr (std::is_invocable_v<UnaryOperation, word_type, size_type>) {
          result = unary_op(get_masked_word<word_type>(first, remaining_bits_to_op), remaining_bits_to_op);
        } else {
          result = unary_op(get_masked_word<word_type>(first, remaining_bits_to_op));
        }
        *it = _bitblend<word_type>(
            *it,
            result,
            _mask<word_type>(remaining_bits_to_op));
      }
    }
    return d_first + total_bits_to_op;
  }

  template <class First1It, class Last1It, class First2It, class DstIt,
            class BinaryOperation>
  static constexpr auto run_impl(const First1It& first1_in,
                                 const Last1It& last1_in,
                                 const First2It& first2_in,
                                 const DstIt& d_first_in,
                                 BinaryOperation binary_op) {
    auto first1 = make_mutable_bit_iterator(first1_in);
    auto last1 = make_mutable_bit_iterator(last1_in);
    auto first2 = make_mutable_bit_iterator(first2_in);
    auto d_first = make_mutable_bit_iterator(d_first_in);
    using in_iterator = decltype(first1);
    using second_iterator = decltype(first2);
    using word_type = typename in_iterator::word_type;
    using second_word_type = typename second_iterator::word_type;
    static_assert(bitsof<word_type>() == bitsof<second_word_type>(),
                  "transform inputs must have the same word size");
    using size_type = typename in_iterator::size_type;
    constexpr size_type digits = binary_digits<word_type>::value;
    constexpr std::size_t First1Align = iterator_alignment_v<First1It>;
    constexpr std::size_t First2Align = iterator_alignment_v<First2It>;
    constexpr std::size_t DstAlign = iterator_alignment_v<DstIt>;

    // Assertions
    _assert_range_viability(first1, last1);
    if (first1 == last1) {
      return d_first;
    }

    // Initialization
    size_type d_first_position;
    if constexpr (DstAlign == 0) {
      d_first_position = 0;
    } else if constexpr (DstAlign == std::dynamic_extent) {
      d_first_position = d_first.position();
    } else {
      d_first_position = DstAlign;
    }
    const bool is_d_first_aligned = d_first_position == 0;
    size_type total_bits_to_op = distance(first1, last1);
    size_type remaining_bits_to_op = total_bits_to_op;
    auto it = d_first.base();

    // d_first is not aligned. Copy partial word to align it
    if (!is_d_first_aligned) {
      size_type partial_bits_to_op = ::std::min(
          remaining_bits_to_op,
          digits - d_first_position);
      *it = _bitblend<word_type>(
          *it,
          static_cast<word_type>(binary_op(
              static_cast<word_type>(
                  get_masked_word<word_type>(first1, partial_bits_to_op)
                  << static_cast<word_type>(d_first_position)),
              static_cast<word_type>(
                  get_masked_word<word_type>(first2, partial_bits_to_op)
                  << static_cast<word_type>(d_first_position)))),
          d_first_position,
          partial_bits_to_op);
      remaining_bits_to_op -= partial_bits_to_op;
      advance(first1, partial_bits_to_op);
      advance(first2, partial_bits_to_op);
      it++;
    }
    if (remaining_bits_to_op > 0) {
      const bool is_first1_aligned = [&]() {
        if constexpr (First1Align != std::dynamic_extent) {
          if constexpr (DstAlign == First1Align) {
            return true;
          } else {
            return First1Align == static_cast<std::size_t>(d_first_position);
          }
        } else {
          return first1.position() == 0;
        }
      }();
      const bool is_first2_aligned = [&]() {
        if constexpr (First2Align != std::dynamic_extent) {
          if constexpr (DstAlign == First2Align) {
            return true;
          } else {
            return First2Align == static_cast<std::size_t>(d_first_position);
          }
        } else {
          return first2.position() == 0;
        }
      }();
      //size_type words_to_op = ::std::ceil(remaining_bits_to_op / static_cast<float>(digits));
      // d_first will be aligned at this point
      if (is_first1_aligned && is_first2_aligned && remaining_bits_to_op > digits) {
        auto N = ::std::distance(first1.base(), last1.base());
        it = std::transform(first1.base(), last1.base(), first2.base(), it, binary_op);
        first1 += digits * N;
        first2 += digits * N;
        remaining_bits_to_op -= digits * N;
      } else {
        while (remaining_bits_to_op >= digits) {
          *it = binary_op(
              get_word<word_type>(first1, digits),
              get_word<word_type>(first2, digits));
          remaining_bits_to_op -= digits;
          it++;
          advance(first1, digits);
          advance(first2, digits);
        }
      }
      if (remaining_bits_to_op > 0) {
        *it = _bitblend<word_type>(
            *it,
            static_cast<word_type>(binary_op(
                get_masked_word<word_type>(first1, remaining_bits_to_op),
                get_masked_word<word_type>(first2, remaining_bits_to_op))),
            _mask<word_type>(remaining_bits_to_op));
      }
    }
    return d_first + total_bits_to_op;
  }
};

namespace detail {
// Construct an iterator at a possibly compile-time alignment.
template <std::size_t Alignment, typename Iterator>
constexpr auto make_transform_iterator(const Iterator& base,
                                       std::size_t position) {
  if constexpr (Alignment == std::dynamic_extent) {
    return iterator<Iterator, primitive<1>, std::dynamic_extent>(base,
                                                                 position);
  } else {
    assert(position == Alignment);
    return ::bit::next<Alignment>(iterator<Iterator, primitive<1>, 0>(base));
  }
}

// Re-express one input iterator in a smaller target word type. The alignment
// is unpacked from the iterator itself, and its position is translated from
// source-word bits into target-word bits.
template <std::size_t TargetBits, typename TargetWord, typename Iterator>
constexpr auto adapt_transform_iterator(const Iterator& input) {
  constexpr std::size_t Alignment = iterator_alignment_v<Iterator>;
  auto it = make_mutable_bit_iterator(input);
  using mutable_iterator = decltype(it);
  constexpr std::size_t source_bits =
      bitsof<std::remove_cv_t<typename mutable_iterator::word_type>>();
  constexpr std::size_t adapted_alignment =
      Alignment == std::dynamic_extent ? std::dynamic_extent
                                       : Alignment % TargetBits;

  if constexpr (source_bits == TargetBits) {
    return make_transform_iterator<Alignment>(it.base(), it.position());
  } else {
    using source_iterator = typename mutable_iterator::iterator_type;
    using source_reference = std::iter_reference_t<source_iterator>;
    constexpr bool source_is_const =
        std::is_const_v<std::remove_reference_t<source_reference>>;
    using target_pointer =
        std::conditional_t<source_is_const, const TargetWord*, TargetWord*>;
    using adapted_iterator =
        bit_word_pointer_adapter<target_pointer, source_iterator>;
    const auto adapted =
        adapted_iterator(it.base(), it.position() / TargetBits);
    return make_transform_iterator<adapted_alignment>(
        adapted, it.position() % TargetBits);
  }
}

template <typename... Iterators>
consteval std::size_t smallest_word_bits() {
  std::size_t result = std::dynamic_extent;
  ((result = std::min(result,
                      bitsof<std::remove_cv_t<typename Iterators::word_type>>())),
   ...);
  return result;
}

template <typename First, typename Last, typename Destination,
          typename UnaryOperation>
constexpr auto with_transform_adapter(const First& first_in,
                                      const Last& last_in,
                                      const Destination& d_first_in,
                                      UnaryOperation unary_op) {
  auto first_probe = make_mutable_bit_iterator(first_in);
  auto d_first_probe = make_mutable_bit_iterator(d_first_in);
  constexpr std::size_t target_bits =
      smallest_word_bits<decltype(first_probe), decltype(d_first_probe)>();
  using target_word = ceil_integral<target_bits>;

  auto adapted_first =
      adapt_transform_iterator<target_bits, target_word>(first_in);
  auto adapted_last =
      adapt_transform_iterator<target_bits, target_word>(last_in);
  auto adapted_d_first =
      adapt_transform_iterator<target_bits, target_word>(d_first_in);
  return transform_impl::run_impl(adapted_first, adapted_last, adapted_d_first,
                                  unary_op);
}

template <typename First, typename Last, typename Second,
          typename Destination, typename BinaryOperation>
constexpr auto with_transform_adapter(const First& first_in,
                                      const Last& last_in,
                                      const Second& first2_in,
                                      const Destination& d_first_in,
                                      BinaryOperation binary_op) {
  auto first_probe = make_mutable_bit_iterator(first_in);
  auto first2_probe = make_mutable_bit_iterator(first2_in);
  auto d_first_probe = make_mutable_bit_iterator(d_first_in);
  constexpr std::size_t target_bits =
      smallest_word_bits<decltype(first_probe), decltype(first2_probe),
                         decltype(d_first_probe)>();
  using target_word = ceil_integral<target_bits>;

  auto adapted_first =
      adapt_transform_iterator<target_bits, target_word>(first_in);
  auto adapted_last =
      adapt_transform_iterator<target_bits, target_word>(last_in);
  auto adapted_first2 =
      adapt_transform_iterator<target_bits, target_word>(first2_in);
  auto adapted_d_first =
      adapt_transform_iterator<target_bits, target_word>(d_first_in);
  return transform_impl::run_impl(adapted_first, adapted_last, adapted_first2,
                                  adapted_d_first, binary_op);
}
}  // namespace detail

// Alignment- and word-size-aware unary transform front-end.
template <typename First, typename Last, typename Destination,
          typename UnaryOperation>
constexpr auto transform(const First& first, const Last& last,
                         const Destination& d_first, UnaryOperation unary_op) {
  static_assert(bit_iterator_like_c<First> && bit_iterator_like_c<Last> &&
                    bit_iterator_like_c<Destination>,
                "transform() requires bit iterators or raw pointers");
  static_assert(
      std::is_same_v<underlying_iterator_t<First>, underlying_iterator_t<Last>>,
      "first and last must have the same underlying iterator type");
  return detail::with_transform_adapter(first, last, d_first, unary_op);
}

// Alignment- and word-size-aware binary transform front-end.
template <typename First, typename Last, typename Second, typename Destination,
          typename BinaryOperation>
constexpr auto transform(const First& first, const Last& last,
                         const Second& first2, const Destination& d_first,
                         BinaryOperation binary_op) {
  static_assert(bit_iterator_like_c<First> && bit_iterator_like_c<Last> &&
                    bit_iterator_like_c<Second> &&
                    bit_iterator_like_c<Destination>,
                "transform() requires bit iterators or raw pointers");
  static_assert(
      std::is_same_v<underlying_iterator_t<First>, underlying_iterator_t<Last>>,
      "first and last must have the same underlying iterator type");
  return detail::with_transform_adapter(first, last, first2, d_first,
                                        binary_op);
}

//constexpr bit_iterator<RandomAccessIt> transform_word(bit_iterator<RandomAccessIt1> first,
//bit_iterator<RandomAccessIt1> last, bit_iterator<RandomAccessIt2> first2,
//bit_iterator<RandomAccessIt> d_first, BinaryOperation binary_op) {
//// Assertions
//_assert_range_viability(first, last);

//// Types and constants
//using word_type1    = typename bit_iterator<RandomAccessIt1>::word_type;
//using word_type2    = typename bit_iterator<RandomAccessIt2>::word_type;
//using word_type_out = typename bit_iterator<RandomAccessIt>::word_type;
//using size_type1    = typename bit_iterator<RandomAccessIt1>::size_type;
//using size_type2    = typename bit_iterator<RandomAccessIt2>::size_type;
//using size_type_out = typename bit_iterator<RandomAccessIt>::size_type;
//constexpr size_type1 digits1     = binary_digits<word_type1>::value;
//constexpr size_type2 digits2     = binary_digits<word_type2>::value;
//constexpr size_type_out digits_out  = binary_digits<word_type_out>::value;

//// This is just for now. Perhaps later we can expand to different word sizes
//assert(digits1 == digits2);
//assert(digits2 == digits_out);
//using word_type = word_type1;
//using size_type = size_type1;
//constexpr size_type digits = digits1;

//if (is_within<digits>(first, last)) {
//word_type d = distance(first, last);
//write_word(
//binary_op(
//get_word(first, d),
//get_word(first2, d)
//),
//d_first,
//d
//);
//return next(d_first, d);
//}

//RandomAccessIt1 it1    = first.base();
//if (first.position() != 0) {
//word_type d = digits - first.position();
//write_word(
//binary_op(
//static_cast<word_type>(*first.base() >> first.position()),
//get_word(first2, d)
//),
//d_first,
//d
//);
//++it1;
//advance(first2, d);
//advance(d_first, d);
//}

//while (it1 != last1.base()) {
//write_word(
//binary_op(
//*it1++,
//get_word<word_type>(first2)
//),
//d_first,
//(word_type)digits
//);
//advance(d_first, digits);
//advance(first2, digits);
//}

//if (last1.position() != 0) {
//write_word(
//binary_op(
//*it1,
//get_word(first2, last1.position())
//),
//d_first,
//static_cast<word_type>(last1.position())
//);
//advance(d_first, last1.position());
//}
//return d_first;
//}

// ========================================================================== //
}  // namespace bit

#endif  // _TRANSFORM_HPP_INCLUDED
// ========================================================================== //
