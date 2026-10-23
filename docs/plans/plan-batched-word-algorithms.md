# Plan: Batched (multi-word) transform, accumulate, and bitwise adapters

## Motivation

The current alignment/word-size-aware frontends invoke the user callable once
per machine word:

- `bit::transform` calls a unary/binary lambda per source word.
- `bit::accumulate` calls a binary/subword callback per word or partial word.
- `bit::transform_accumulate` does the same while writing an output word.

That is a good default for arbitrary callables, but it leaves performance on
the table for algorithms whose natural implementation processes large
contiguous chunks:

- `copy`/`move` want `memcpy`, `memmove`, or a vectorized copy loop.
- `count` wants a popcount reduction over many words.
- `find` wants a vectorized first-nonzero scan.
- Bitwise `and`/`or`/`xor` want wide word or SIMD loops.
- Future hash/checksum/compare algorithms want whole-cache-line batches.

On this branch, the current transform identity implementation is competitive
for misaligned copy but loses on aligned copy, while accumulate-based count and
find pay a per-word callback cost:

| workload | baseline | transform/accumulate version |
|---|---:|---:|
| copy, aligned (2 MiB) | 91.2 µs | 115.4 µs (1.27x slower) |
| copy, misaligned (2 MiB) | 564.9 µs | 116.5 µs (4.8x faster) |
| count (1 Mi bit) | 2.38 µs | 3.48 µs (1.46x slower) |
| find at 75% (1 Mi bit) | 2.83 µs | 5.69 µs (2.01x slower) |

A batched lambda API would let callers choose the right implementation per
algorithm without adding a public template parameter to `bit::reference` or
changing the existing scalar frontends.

## Goals

1. Keep the existing scalar `transform`/`accumulate`/`transform_accumulate`
   APIs and behavior unchanged.
2. Allow a callable to process a contiguous batch of whole words in one call.
3. Reuse the existing word-size adaptation and alignment handling from
   `word_adapter.hpp`.
4. Make `copy`, `count`, `find`, `equal`, and bitwise `and`/`or`/`xor` able to
   opt into batching without duplicating the range-walking logic.
5. Preserve early-exit semantics for `find`/`accumulate_while`.
6. Avoid unbounded template/code-size growth.

## Non-goals

- Do not assume the user's operation is associative or commutative.
- Do not silently change the order of accumulator calls.
- Do not require batch callables for algorithms that cannot provide
  contiguous, writable words.
- Do not add a public `bool Batched`/`ArrayBacked` template parameter to
  `bit::reference` or similar user-visible types.

## Core abstraction

Introduce a lightweight batch view in a new
`include/bitlib/bit-algorithms/batch_adapter.hpp`:

```cpp
namespace bit {

template <typename Word>
struct word_batch {
  Word* data;
  std::size_t size;      // number of whole words
  std::size_t bit_count; // size * bitsof<Word>()

  constexpr Word* begin() const noexcept { return data; }
  constexpr Word* end() const noexcept { return data + size; }
  constexpr Word& operator[](std::size_t i) const noexcept { return data[i]; }
};

template <typename Word>
using const_word_batch = word_batch<const Word>;

}  // namespace bit
```

`word_batch` is a non-owning view, like `std::span`, but it carries the exact
word count and bit count and avoids exposing a new public container type. It is
trivially copyable and usable in both scalar and vectorized user code.

## API design

### 1. Batched transform

Provide a separate entry point rather than overloading the scalar callable
signature implicitly. This avoids ambiguity with generic lambdas that happen to
accept both a word and a batch.

```cpp
template <std::size_t BatchWords = 0,
          typename First, typename Last, typename Dst, typename BatchOp>
constexpr auto transform_batched(
    const First& first,
    const Last& last,
    const Dst& d_first,
    BatchOp batch_op);

template <std::size_t BatchWords = 0,
          typename First1, typename Last1, typename First2,
          typename Dst, typename BatchOp>
constexpr auto transform_batched(
    const First1& first1,
    const Last1& last1,
    const First2& first2,
    const Dst& d_first,
    BatchOp batch_op);
```

Callable contracts:

```cpp
// Unary
void batch_op(const_word_batch<Word> src, word_batch<Word> dst);

// Binary
void batch_op(const_word_batch<Word> src1,
              const_word_batch<Word> src2,
              word_batch<Word> dst);
```

`BatchWords == 0` means "choose a runtime default" (for example 8 words or
one cache-line-sized chunk). A non-zero value caps each call; the algorithm
still handles the final short batch.

The return value is the same iterator type/position as the scalar overload.

### 2. Batched accumulate

```cpp
template <std::size_t BatchWords = 0,
          typename First, typename Last, typename T,
          typename BatchOp, typename WordOp, typename SubwordOp>
constexpr auto accumulate_batched(
    const First& first,
    const Last& last,
    T acc,
    BatchOp batch_op,
    WordOp word_op,
    SubwordOp subword_op);
```

Contract:

```cpp
// Per-batch, left fold over the batch.
T batch_op(T acc, const_word_batch<Word> words);
```

The scalar `word_op`/`subword_op` are still required for the aligned prefix,
the final partial word, and as a fallback when batching is not possible. This
keeps the accumulator's call order identical to the scalar algorithm:
prefix bits, complete batches in order, tail bits.

For early-exit variants:

```cpp
struct batch_result {
  bool keep_going;
  std::size_t consumed_words; // words consumed from this batch
};

batch_result batch_op(T acc, const_word_batch<Word> words);
```

`accumulate_while_batched` uses `consumed_words` to resume at the exact word
where the batch stopped. The index of the found item can be carried in the
accumulator (as the current `FindAcc` prototype does) or returned separately
in a small result struct.

### 3. Batched transform-accumulate

```cpp
template <std::size_t BatchWords = 0,
          typename First, typename Last, typename DFirst, typename DLast,
          typename T, typename BatchOp, typename WordOp, typename SubwordOp>
constexpr auto transform_accumulate_batched(
    const First& first,
    const Last& last,
    const DFirst& d_first,
    const DLast& d_last,
    T acc,
    BatchOp batch_op,
    WordOp word_op,
    SubwordOp subword_op);
```

Contract:

```cpp
// Read a batch, write a batch, return the updated accumulator.
T batch_op(T acc, const_word_batch<Word> src, word_batch<Word> dst);
```

A batched backward version follows the same shape and uses the existing
backward alignment/tail rules.

## Dispatch rules

Batching is only legal when the underlying storage is contiguous and the
source/destination word widths can be unified.

1. Normalize all inputs with `make_mutable_bit_iterator`, exactly as the
   current frontends do.
2. Select the common target word width with
   `detail::smallest_word_bits<...>()` and `detail::adapt_word_iterator`.
3. Process the unaligned prefix with the scalar `word_op`/`subword_op`.
4. Batch only if:
   - the adapted iterator's `word_type` matches,
   - the underlying iterator is contiguous
     (`bit_contiguous_iterator_c` / `std::contiguous_iterator`),
   - source and destination are word-aligned after the prefix,
   - the batch does not include a partial word.
5. Otherwise fall back to the current scalar word loop.
6. Process the final partial word with the scalar subword callback.

This means mixed-width ranges can still batch when the adapted target type is
the same. For example, a `uint64_t` source and `uint16_t` destination adapt to
`uint16_t`; if both adapters expose contiguous target-word storage, a
`uint16_t` batch copy is possible. If the adapter is not contiguous, the
algorithm uses the existing scalar path.

## Batch size policy

- Default `BatchWords`: choose at runtime from a small table, e.g. 8 words for
  64-bit words (one cache line), clamped to the remaining count.
- Optional compile-time `BatchWords`: allows unrolling and fixed-size SIMD in
  the user lambda, but can cause code bloat. Use it only in library internals or
  performance-critical algorithms.
- Never cross the end of the range; a short final batch is always legal.
- For backward algorithms, batch from the end and pass the words in forward
  order inside each batch so the user's fold order remains well-defined.

## Relationship to `and`/`or`/`xor`

The bitwise container operators already route through `transform`. Add
specialized batched callables, not new algorithms:

```cpp
template <typename Word>
struct and_batch {
  void operator()(const_word_batch<Word> a,
                  const_word_batch<Word> b,
                  word_batch<Word> out) const noexcept {
    for (std::size_t i = 0; i < a.size; ++i) {
      out[i] = a[i] & b[i];
    }
  }
};
```

Then:

```cpp
bit::transform_batched(first, last, second, out, and_batch<Word>{});
```

The same pattern covers `or`, `xor`, `andnot`, and future three-input
operations. A vectorized implementation can live entirely in the callable,
which keeps the core algorithm free of SIMD/type dependencies.

## Proposed files

- `include/bitlib/bit-algorithms/word_adapter.hpp` (existing)
  - continue to own word-size/alignment adaptation and aligned iterator
    construction.
- `include/bitlib/bit-algorithms/batch_adapter.hpp` (new)
  - `word_batch`, `const_word_batch`, `batch_result`, batch-size policy,
    contiguity/alignment checks, prefix/batch/tail driver.
- `include/bitlib/bit-algorithms/transform.hpp`
  - add `transform_batched` unary/binary overloads.
- `include/bitlib/bit-algorithms/accumulate.hpp`
  - add `accumulate_batched` and `accumulate_while_batched`.
- `include/bitlib/bit-algorithms/transform_accumulate.hpp`
  - add forward/backward batched overloads.
- `include/bitlib/bit-algorithms/copy.hpp`, `count.hpp`, `find.hpp`
  - add internal batched fast paths, or call the new frontends with library
    batch callables.
- `test/src/test-batch-algorithms.cpp` (new)
  - correctness for aligned, misaligned, mixed-width, short-batch, empty,
    early-exit, and overlapping/non-overlapping cases.
- `benchmark/src/` (optional)
  - copy/count/find/and/or/xor batch benchmarks behind the existing benchmark
    target.

## Implementation phases

### Phase 1: Batch view and driver

1. Add `word_batch`/`const_word_batch` and `batch_result`.
2. Add `can_batch(first, last, d_first)` helpers using
   `bit_contiguous_iterator_c` and `iterator_alignment_v`.
3. Add a private `for_each_word_batch` driver that:
   - handles prefix,
   - computes a capped batch count,
   - invokes the batch callable,
   - advances source and destination by whole words,
   - handles the final partial word.
4. Unit-test the driver with a batch callable that just records pointers and
   counts, without any bitlib algorithm.

### Phase 2: Transform

1. Implement unary and binary `transform_batched` using the driver.
2. Reuse `detail::adapt_word_iterator` and `smallest_word_bits`.
3. Keep the scalar overloads untouched.
4. Add tests for:
   - both sources/destination aligned,
   - only destination aligned,
   - only sources aligned,
   - mixed word widths,
   - batch size larger than range,
   - batch size 1 (degenerate).
5. Benchmark identity copy and `and`/`or`/`xor` against the scalar transform
   and the specialized baseline.

### Phase 3: Accumulate

1. Implement `accumulate_batched` for forward and backward.
2. Implement `accumulate_while_batched` with `batch_result`.
3. Preserve the existing initial-subword policy and call order.
4. Add tests with a batch op that computes popcount and with one that stops at
   the first nonzero word.
5. Benchmark count and find against the current specialized implementations.

### Phase 4: Transform-accumulate

1. Implement forward and backward `transform_accumulate_batched`.
2. Reuse the corrected backward tail/head rules:
   - tail offset 0 when the range spans words,
   - head offset `d_first.position()`,
   - shift the partial word left by its destination offset before blending.
3. Add a mixed-width copy test and a division-style long-division test.

### Phase 5: Library algorithms

1. `copy`/`move`: use a batch callable that invokes `std::memcpy`/`memmove`
   when source and destination are not overlapping, or a word loop otherwise.
2. `count`: use a batch popcount reduction; keep `libpopcnt` as the current
   baseline.
3. `find`: use a batch first-nonzero scan with `std::countr_zero`, and preserve
   the existing early-exit index.
4. `equal`: batch compare and early exit on the first differing word.
5. Bitwise `and`/`or`/`xor`: batch word loops or SIMD.
6. Keep the existing scalar algorithms as the fallback for non-contiguous
   storage.

### Phase 6: Validation and tuning

1. Run the full test suite on Clang/libstdc++, Clang/libc++, and MSVC.
2. Run the warning presets with `-Wall -Wextra -Wpedantic -Wshadow -Werror`.
3. Benchmark:
   - small (256 bits), medium (64 KiB), large (2 MiB and larger),
   - aligned and misaligned,
   - mixed word widths,
   - batch sizes 4/8/16/32.
4. Acceptance targets:
   - batched copy aligned: within 5% of `std::copy`/current `bit::copy`;
   - batched copy misaligned: at least as fast as the current transform path;
   - batched count: within 10% of the current `bit::count`;
   - batched find: within 10% of the current `bit::find` at both early and
     late match positions;
   - batched and/or/xor: no regression versus the current transform path.
5. Document the recommended batch callable patterns in the plan/API docs.

## Edge cases and correctness checklist

- Empty range: return immediately, never call the batch op with size 0.
- Prefix/suffix partial words: always scalar, with the correct bit count.
- `d_first` offset: shift partial output words before blending; never assume
  offset 0.
- Backward ranges: process from the end, but pass each batch in ascending word
  order to preserve the user's fold order.
- Mixed word widths: adapt to the smallest word; batch only if both adapted
  views are contiguous and aligned.
- Early exit: `batch_result.consumed_words` must be exact; resume scalar at the
  next word, not at the start of the batch.
- Aliasing/overlap: `copy` must not use `memcpy` for overlapping ranges;
  document that `transform` output must not overlap inputs unless the callable
  handles it.
- `const` storage: use `const_word_batch` and preserve const-correctness.
- MSVC: avoid constrained partial specializations and CTAD in the new public
  signatures; prefer explicit `iterator<...>` spellings in internal code.
- Compile time/code size: default to a runtime batch size; only use
  `BatchWords` compile-time values in library-internal specializations.

## Risks

| Risk | Mitigation |
|---|---|
| Ambiguous lambda overloads | Separate `*_batched` entry points and explicit batch views |
| Code bloat from batch sizes | Runtime default, limited compile-time instantiations |
| Wrong fold order | Batch callables always see words in ascending order |
| Adapter is not contiguous | `can_batch` falls back to the scalar path |
| Early-exit bugs | Return consumed word count; unit-test every stop position |
| Mixed-width aliasing | Batch only after adaptation to a common contiguous word type |
| Performance regression on small ranges | Keep the scalar path for ranges below a threshold |

## Open questions

1. Should `word_batch` be a distinct type or an alias for `std::span<Word>`?
   A distinct type is easier to extend with `bit_count` and future metadata.
2. Should the default batch size be a cache line (8 × 64-bit words) or tunable
   per algorithm?
3. Should batched `accumulate` require a commutative operation for any
   vectorized fast path, or always document a strict left fold?
4. Do we want a policy tag (`batch_size<N>`) instead of a template parameter
   on the public function, to keep signatures stable?
