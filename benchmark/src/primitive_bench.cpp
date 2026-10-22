#include <benchmark/benchmark.h>

#include <iterator>
#include <string>
#include <tuple>
#include <utility>

#include "benchmark_utils.hpp"
#include "bitlib/bitlib.hpp"

// ============================================================================
// A/B Profiling: bit::primitive<N> abstraction overhead
// Side A: PrimitiveBits == 1 (single bit, bit_value equivalent — baseline)
// Side B: PrimitiveBits == N (multi bit — our new generic primitive<N>)
// ============================================================================

template <size_t PrimitiveBits, typename WordType>
auto BM_PrimitiveReadWrite = [](benchmark::State& state, auto input) {
  using container_type = typename std::tuple_element<0, decltype(input)>::type;
  unsigned int total_bits = std::get<2>(input);
  auto container_size = total_bits / bit::binary_digits<WordType>() + 1;
  auto bitvec = get_random_vec<WordType>(container_size);

  using It = bit::iterator<typename container_type::iterator, PrimitiveBits>;
  auto first = It(bitvec.begin(), 0);

  for (auto _ : state) {
    for (unsigned int i = 0; i + PrimitiveBits <= total_bits; i += PrimitiveBits) {
      auto val = *first;
      benchmark::DoNotOptimize(val);
      *first = val;
      std::advance(first, PrimitiveBits);
    }
    benchmark::ClobberMemory();
  }
};

template <size_t PrimitiveBits, typename WordType>
auto BM_PrimitiveCopy = [](benchmark::State& state, auto input) {
  using container_type = typename std::tuple_element<0, decltype(input)>::type;
  unsigned int total_bits = std::get<2>(input);
  auto container_size = total_bits / bit::binary_digits<WordType>() + 1;
  auto src = get_random_vec<WordType>(container_size);
  auto dst = get_random_vec<WordType>(container_size);

  using It = bit::iterator<typename container_type::iterator, PrimitiveBits>;
  auto src_first = It(src.begin(), 0);
  auto dst_first = It(dst.begin(), 0);

  for (auto _ : state) {
    if constexpr (PrimitiveBits == 1) {
      bit::copy(src_first, src_first + total_bits / PrimitiveBits, dst_first);
    } else {
      auto s = src_first;
      auto d = dst_first;
      for (unsigned int i = 0; i < total_bits / PrimitiveBits; ++i) {
        *d = *s;
        std::advance(s, PrimitiveBits);
        std::advance(d, PrimitiveBits);
      }
    }
    benchmark::ClobberMemory();
  }
};

template <size_t PrimitiveBits, typename WordType>
auto BM_PrimitiveConstruct = [](benchmark::State& state, auto input) {
  unsigned int count = std::get<2>(input);
  for (auto _ : state) {
    bit::primitive<PrimitiveBits> p(0);
    for (unsigned int i = 0; i < count; ++i) {
      p = bit::primitive<PrimitiveBits>(static_cast<WordType>(i));
      benchmark::DoNotOptimize(p);
    }
    benchmark::ClobberMemory();
  }
};

// Register for PrimitiveBits = 1 (baseline) and N = 8, 16, 32 (multi-bit)
#define REGISTER_PRIM(PBITS, WT, SIZE, SIZELABEL)                        \
  do {                                                                   \
    std::string rw_name =                                                \
        std::string("prim_readwrite<" #PBITS "/uint64_t>/") + SIZELABEL; \
    std::string cp_name =                                                \
        std::string("prim_copy<" #PBITS "/uint64_t>/") + SIZELABEL;      \
    std::string cn_name =                                                \
        std::string("prim_construct<" #PBITS "/uint64_t>/") + SIZELABEL; \
    benchmark::RegisterBenchmark(                                        \
        rw_name.c_str(),                                                 \
        BM_PrimitiveReadWrite<PBITS, WT>,                                \
        std::make_tuple(typename std::vector<WT>(), WT(), SIZE));        \
    benchmark::RegisterBenchmark(                                        \
        cp_name.c_str(),                                                 \
        BM_PrimitiveCopy<PBITS, WT>,                                     \
        std::make_tuple(typename std::vector<WT>(), WT(), SIZE));        \
    benchmark::RegisterBenchmark(                                        \
        cn_name.c_str(),                                                 \
        BM_PrimitiveConstruct<PBITS, WT>,                                \
        std::make_tuple(typename std::vector<WT>(), WT(), SIZE));        \
  } while (0)

int main(int argc, char** argv) {
  unsigned int size_small = 1 << 12;
  unsigned int size_large = 1 << 16;
  unsigned int construct_count = 1 << 16;

  for (auto [size, label] : {
           std::make_pair(size_small, std::string("small")),
           std::make_pair(size_large, std::string("large")),
       }) {
    REGISTER_PRIM(1, uint64_t, size, label);
    REGISTER_PRIM(8, uint64_t, size, label);
    REGISTER_PRIM(16, uint64_t, size, label);
    REGISTER_PRIM(32, uint64_t, size, label);
  }

  REGISTER_PRIM(1, uint64_t, construct_count, "construct");
  REGISTER_PRIM(8, uint64_t, construct_count, "construct");
  REGISTER_PRIM(16, uint64_t, construct_count, "construct");
  REGISTER_PRIM(32, uint64_t, construct_count, "construct");

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
