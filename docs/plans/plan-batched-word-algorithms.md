# Plan: Alignment-planned batched transform, accumulate, and bitwise operations

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

The plan below lets callers process whole words in one call without adding new
batch view types, and without making the existing scalar frontends depend on
alignment assumptions they cannot prove.

## Goals

1. Keep the existing scalar `transform`/`accumulate`/`transform_accumulate`
   APIs and behavior unchanged.
2. Allow a callable to process a contiguous run of whole words in one call.
3. Pass words as raw pointers plus a word count. Do **not** introduce
   `word_batch`/`const_word_batch` view types.
4. Extend the use of `Alignment` so it describes both the bit phase and the
   higher-level word/batch/SIMD alignment.
5. Plan prefix, full-batch, SIMD, and tail loops from the range's known
   size/extent instead of checking alignment on every iteration.
6. Reuse the existing word-size adaptation in `word_adapter.hpp`.
7. Preserve early-exit semantics for `find`/`accumulate_while`.

## Non-goals

- Do not assume the user's operation is associative or commutative.
- Do not silently change the order of accumulator calls.
- Do not add a public `bool Batched`/`ArrayBacked` template parameter to
  `bit::reference` or similar user-visible types.
- Do not require batch callables for algorithms that cannot provide
  contiguous, writable words.
- Do not make alignment hints a correctness requirement. They select a faster
  path; the scalar path remains the fallback.

## Alignment model

`Alignment` is currently the compile-time bit position inside one machine
word. Extend its role so the batched frontends can use it as a **bit-granular
alignment descriptor**, not just a within-word position.

Let `A` be the effective bit alignment of the first element in an aligned
coordinate system:

```text
A = bit address of the first element
  = address_bits(base) + iterator position
```

For a bit iterator, `address_bits(base)` is the bit address of the underlying
word storage and `iterator position` is the compile-time/runtime bit offset in
that word. For a raw integral pointer, the position is zero and the address
supplies the alignment.

The within-word position remains:

```text
position = A % bitsof<Word>()
```

The higher-level alignment checks are:

```text
word-aligned  <=> A % bitsof<Word>() == 0
batch-aligned <=> A % batch_bits       == 0
SIMD-aligned  <=> A % simd_bits        == 0
```

where:

```text
batch_bits = BatchWords * bitsof<Word>()
simd_bits  = SIMDLanes  * bitsof<Word>()
```

If a policy expresses a batch or SIMD width in words rather than bits, convert
to bits once before applying the modulus. The moduli are applied successively:

1. `A % bitsof<Word>()` selects the within-word prefix/scalar phase.
2. `A % batch_bits` decides whether an unrolled batch loop starts immediately.
3. `A % simd_bits` decides whether the SIMD loop starts immediately.

When a higher-level alignment does not hold, the planner can either:

- process a prefix with the word or scalar loop until the next boundary, then
  run the aligned batch/SIMD loop, or
- skip the wider loop and use the next-lower strategy (SIMD -> batch -> word ->
  scalar).

The choice is made once from the range extent and alignment, not inside the
hot loop.

### Compatibility with the current iterator

The existing fixed-Alignment iterator stores `Alignment` as the position inside
one word and asserts `Alignment < bitsof<uintptr_t>()`. The plan must preserve
that behavior while exposing the wider alignment information:

- Keep `position = Alignment % bitsof<Word>()` in the iterator.
- Add an alignment accessor/descriptor that reports the wider effective
  alignment when it is known.
- Do not change the meaning of existing `iterator<..., Alignment>` types that
  use a within-word offset.
- If a larger `Alignment` value is used for a wider hint, normalize it modulo
  the largest strategy width used by the planner so it remains a bounded
  compile-time value.

## Callable contracts

No batch view type is created. A batch callable receives raw word pointers and
a count.

### Unary transform

```cpp
void batch_op(const Word* src, Word* dst, std::size_t word_count);
```

### Binary transform

```cpp
void batch_op(const Word* src1,
              const Word* src2,
              Word* dst,
              std::size_t word_count);
```

### Accumulate

```cpp
T batch_op(T acc, const Word* words, std::size_t word_count);
```

### Accumulate with early exit

Return the updated accumulator, whether to continue, and how many words were
actually consumed:

```cpp
std::tuple<bool, T, std::size_t>
batch_op(T acc, const Word* words, std::size_t word_count);
// {keep_going, acc, consumed_words}
```

`consumed_words` is essential: the algorithm resumes at the exact word where
the batch stopped rather than restarting the batch.

### Transform-accumulate

```cpp
T batch_op(T acc,
           const Word* src,
           Word* dst,
           std::size_t word_count);
```

`const Word*` vs `Word*` provides const-correctness without a new view type.

## Range planning

Batched algorithms operate on ranges with a known size/extent. The planner
computes the loop shape before entering the hot path.

For a forward range:

```text
total_bits = distance(first, last)
prefix_bits = bits needed to reach the next chosen boundary
body_bits   = total_bits - prefix_bits
full_units  = body_bits / unit_bits
tail_bits   = body_bits % unit_bits
```

The unit can be a word, a batch of `BatchWords` words, or a SIMD block of
`SIMDLanes` words. The corresponding loop plans are:

```text
word loop:    n_words   = body_bits / word_bits
batch loop:   n_batches = body_bits / batch_bits
SIMD loop:    n_simd    = body_bits / simd_bits
```

For fixed-extent ranges, the extent and therefore the full loop counts can be
`constexpr`. The planner can then use `if constexpr` to select an unrolled loop
with a fixed trip count and leave only a compile-time-known tail. For dynamic
extents, the counts are computed once and the loop still has no per-iteration
alignment branch.

Backward algorithms use the alignment of the end of the range:

```text
A_end = A + total_bits
```

and plan the trailing batch/SIMD loop from `A_end`. The batch callable still
receives words in ascending order so the user's fold order is unchanged.

## API

Add separate batched entry points so scalar lambdas remain unambiguous:

```cpp
template <typename First, typename Last, typename Dst, typename BatchOp>
constexpr auto transform_batched(
    const First& first,
    const Last& last,
    const Dst& d_first,
    BatchOp batch_op);

template <typename First1, typename Last1, typename First2,
          typename Dst, typename BatchOp>
constexpr auto transform_batched(
    const First1& first1,
    const Last1& last1,
    const First2& first2,
    const Dst& d_first,
    BatchOp batch_op);

template <typename First, typename Last, typename T,
          typename BatchOp, typename WordOp, typename SubwordOp>
constexpr auto accumulate_batched(
    const First& first,
    const Last& last,
    T acc,
    BatchOp batch_op,
    WordOp word_op,
    SubwordOp subword_op);

template <typename First, typename Last,
          typename DFirst, typename DLast, typename T,
          typename BatchOp, typename WordOp, typename SubwordOp>
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

Batch/SIMD widths are selected internally from a policy. If callers need to
choose a width explicitly, add an optional `batch_size<N>`/`simd_width<N>` tag
rather than a batch view type. Library-internal algorithms can use fixed
compile-time widths for unrolling.

## Dispatch rules

Batching is only legal when the underlying storage is contiguous and the
source/destination word widths can be unified.

1. Normalize inputs with `make_mutable_bit_iterator`.
2. Select the common target word width with
   `detail::smallest_word_bits<...>()` and `detail::adapt_word_iterator`.
3. Compute `A` for the first element from the iterator's compile-time
   `Alignment`, its base address, and its runtime position.
4. Process any prefix needed to reach `A % bitsof<Word>() == 0` with the scalar
   word/subword path.
5. If the range is large enough and `A % simd_bits == 0`, run the SIMD loop.
6. Otherwise, if `A % batch_bits == 0`, run the unrolled batch loop.
7. Otherwise use the word loop, with any remaining prefix handled scalar.
8. Process the final partial word with the scalar subword callback.
9. If the adapted views are not contiguous (for example a
   `bit_word_pointer_adapter`), skip batching and use the scalar path.

The planner never checks alignment per word. It chooses the prefix length,
loop count, and tail once.

## Relationship to `and`/`or`/`xor`

The bitwise container operators already route through `transform`. Add batched
callables using raw pointers:

```cpp
template <typename Word>
struct and_batch {
  void operator()(const Word* a,
                  const Word* b,
                  Word* out,
                  std::size_t word_count) const noexcept {
    for (std::size_t i = 0; i < word_count; ++i) {
      out[i] = a[i] & b[i];
    }
  }
};
```

Then:

```cpp
bit::transform_batched(first, last, second, out, and_batch<Word>{});
```

The `and_batch` implementation can be scalar, unrolled, or SIMD. The alignment
planner decides whether the callable is invoked on a SIMD-sized count, a
batch-sized count, or a word-sized count. The same pattern covers `or`, `xor`,
`andnot`, and future multi-input operations.

## Proposed files

- `include/bitlib/bit-algorithms/word_adapter.hpp` (existing)
  - continue to own word-size adaptation and aligned iterator construction.
- `include/bitlib/bit-algorithms/batch_adapter.hpp` (new)
  - alignment decomposition (`A % word_bits`, `A % batch_bits`,
    `A % simd_bits`), prefix/loop/tail planning, and raw-pointer batch
    dispatch.
- `include/bitlib/bit-iterator/bit_iterator_declaration.hpp` and
  `bit_iterator_definition.hpp`
  - expose the wider alignment descriptor without changing the existing
    within-word `position()` contract.
- `include/bitlib/bit-algorithms/transform.hpp`
  - add unary/binary `transform_batched`.
- `include/bitlib/bit-algorithms/accumulate.hpp`
  - add `accumulate_batched` and `accumulate_while_batched`.
- `include/bitlib/bit-algorithms/transform_accumulate.hpp`
  - add forward/backward batched overloads.
- `include/bitlib/bit-algorithms/copy.hpp`, `count.hpp`, `find.hpp`
  - add batched fast paths or call the batched frontends with library
    callables.
- `test/src/test-batch-algorithms.cpp` (new)
  - correctness for aligned, misaligned, mixed-width, short-range, empty,
    early-exit, fixed-extent, dynamic-extent, and backward cases.
- `benchmark/src/` (optional)
  - copy/count/find/and/or/xor batch benchmarks behind the existing benchmark
    target.

## Implementation phases

### Phase 1: Alignment planning and loop planner

1. Add the extended alignment decomposition helper:
   - `within_word(A)`, `is_word_aligned(A)`,
   - `is_batch_aligned(A, batch_bits)`,
   - `is_simd_aligned(A, simd_bits)`.
2. Add a prefix/body/tail planner for forward and backward ranges.
3. Add a raw-pointer batch driver that calls a callable with
   `(src, dst, count)` or `(src1, src2, dst, count)`.
4. Unit-test the planner with a callable that records pointer/count pairs and
   verifies contiguous coverage exactly once.

### Phase 2: Transform

1. Implement unary and binary `transform_batched` using the planner.
2. Reuse `detail::adapt_word_iterator` and `smallest_word_bits`.
3. Keep the scalar overloads untouched.
4. Add tests for:
   - word-aligned, batch-aligned, and SIMD-aligned starts,
   - starts offset by 1..word_bits-1,
   - ranges shorter than one batch/SIMD block,
   - fixed and dynamic extents,
   - mixed word widths,
   - batch size larger than the range.
5. Benchmark copy, `and`, `or`, and `xor` against the scalar transform and the
   specialized baseline.

### Phase 3: Accumulate

1. Implement forward and backward `accumulate_batched`.
2. Implement `accumulate_while_batched` with
   `(keep_going, acc, consumed_words)`.
3. Preserve the initial-subword policy and call order.
4. Add tests with popcount batches and with early exit at every word position.
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
   when the ranges do not overlap, or a word loop otherwise.
2. `count`: use a batch popcount reduction; keep `libpopcnt` as the baseline.
3. `find`: use a batch first-nonzero scan with `std::countr_zero` and preserve
   the early-exit index.
4. `equal`: batch compare and early exit on the first differing word.
5. Bitwise `and`/`or`/`xor`: batch word loops or SIMD.
6. Keep the scalar algorithms as the fallback for non-contiguous storage.

### Phase 6: Validation and tuning

1. Run the full test suite on Clang/libstdc++, Clang/libc++, GCC, and MSVC.
2. Run the warning presets with
   `-Wall -Wextra -Wpedantic -Wshadow -Werror`.
3. Benchmark:
   - small (256 bits), medium (64 KiB), large (2 MiB and larger),
   - aligned and misaligned starts,
   - mixed word widths,
   - batch/SIMD widths 4/8/16/32 words,
   - fixed versus dynamic extents.
4. Acceptance targets:
   - batched copy aligned: within 5% of `std::copy`/current `bit::copy`;
   - batched copy misaligned: at least as fast as the current transform path;
   - batched count: within 10% of the current `bit::count`;
   - batched find: within 10% of the current `bit::find` at early and late
     match positions;
   - batched and/or/xor: no regression versus the current transform path.
5. Document the recommended batch callable patterns.

## Edge cases and correctness checklist

- Empty range: return immediately; never call a callable with count 0.
- Prefix/suffix partial words: scalar, with the correct bit count.
- `d_first` offset: shift partial output words before blending; never assume
  offset 0.
- Backward ranges: plan from `A_end = A + total_bits`, but pass each batch in
  ascending word order.
- Mixed word widths: adapt to the smallest word; batch only if the adapted
  views are contiguous and aligned.
- Early exit: `consumed_words` must be exact; resume at the next word, not the
  start of the batch.
- Unknown/dynamic alignment: fall back to the word/scalar path.
- Address alignment: a `Word*` may be aligned for the word type but not for a
  batch or SIMD block; verify `A % batch_bits`/`A % simd_bits` before using the
  wider loop.
- Overlap: `copy` must not use `memcpy` for overlapping ranges; document that
  `transform` output must not overlap inputs unless the callable handles it.
- Const storage: use `const Word*` and preserve const-correctness.
- MSVC: keep proxy references materialized as `Word` before generic callbacks
  and `_bitblend`, as required by the current transform_accumulate fix.
- Compile time/code size: cap compile-time unrolling and use a bounded
  normalized alignment value.

## Risks

| Risk | Mitigation |
|---|---|
| Extending `Alignment` semantics breaks existing fixed-Alignment iterators | Keep `position = Alignment % word_bits` and add a separate wider alignment descriptor |
| Over-aligned SIMD/batch assumptions | Check the effective alignment and fall back |
| Code bloat from fixed extents and unrolling | Cap unrolling and use runtime counts for large/dynamic extents |
| Wrong fold order | Batch callables always receive words in ascending order |
| Adapter is not contiguous | `can_batch` falls back to the scalar path |
| Early-exit bugs | Return `consumed_words`; test every stop position |
| Mixed-width aliasing | Batch only after adaptation to a common contiguous word type |
| Small-range regression | Keep scalar/word paths for ranges below the batch/SIMD threshold |
| Address alignment differs from bit alignment | Compute `A` from both the address and the iterator position |

## Open questions

1. Should `batch_bits` and `simd_bits` be expressed in bits everywhere, with
   words converted at the policy boundary? This plan assumes bits.
2. Should the wider alignment be exposed as a new iterator accessor, a
   descriptor, or an extension of `iterator_alignment_v`?
3. How should a larger `Alignment` template argument be normalized so it stays
   bounded while retaining `A % batch_bits`/`A % simd_bits` information?
4. Should callers be able to select a fixed batch/SIMD width with a tag, or
   should the planner always choose from the range extent?
5. For backward operations, should the planner align from the end or from the
   start and then use a mirrored loop? Aligning from the end matches the
   current tail/head implementation.
