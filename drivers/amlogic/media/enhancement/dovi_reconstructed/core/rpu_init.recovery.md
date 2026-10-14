# decoder initialization recovery

`rpu_decoder_init` recovers `rpu_decoder_init` from `p3i-t4c-dovi.ko`,
SHA-256 `5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The reference is byte-identical to `s922x-j_dovi.ko`. Ghidra program
`p3i-t4c-dovi.ko` was explicitly selected for the analysis requests.

The ELF symbol is `.text+0x1547c`, 660 bytes (165 ARM64 instructions), shown
in Ghidra at `001154a4`. Its eight-byte literal pool at `.text+0x15710` has
an `R_AARCH64_ABS64` relocation to the 320-byte `level_limits` object at
`.rodata+0x1c820`. There are six `memset` call relocations and one `_mcount`
call relocation. `rpu_init.ghidra.json` preserves the decompilation and
instruction listing; `tests/rpu_init_results.json` preserves the pinned
symbol/code/table hashes, relocation contract, DWARF offsets, and results.

## Contract and arena layout

The function takes a writable caller-owned arena and a disjoint readable
28-byte `rpu_init_flags_t`. It reads `level` at +4, `disable_crc_check` at +8,
`payload_only` at +12, and `disable_MEL_detection` at +20. The other fields,
including `allow_changing_sequence_header` at +24, are ignored. `payload_only`
uses a zero/nonzero decision and preserves the supplied 32-bit flag words.
It performs no pointer, allocation-size, or level validation and returns zero.
The recovered contract covers level indices 0..15 and sufficient writable arena
space. Indices outside that range and overlapping flags are outside this unit's
validated contract.

DWARF sizes are decoder context `0x316e8` (202472), RPU context `0x87c`
(2172), and three-pointer RPU data object `0x18` (24). The arena order is:

| Region | Size / offset |
| --- | --- |
| Decoder context | `0x316e8` bytes |
| RPU buffers | `N * 0x1000` bytes, N=64 for zero payload_only, otherwise N=1 |
| Buffer levels | `N * 4` bytes |
| RPU context | `0x87c` bytes |
| RPU data object | `0x18` bytes |
| RPU header | `0x136` bytes |
| DM payload | `0x10744` bytes |
| Partition payload arena | `(max_prev+2) * max_partitions * 0x7038` bytes |

The payload base is `0x828f6` for the full-buffer mode and `0x437fa` for
payload-only mode. Some internal pointers are unaligned: the recovered source
uses explicit byte access so that host compilation preserves these accesses
without imposing stronger alignment assumptions.

The first three level rows are `(1,1,9,2,0)`, `(4,4,11,2,3)`, and
`(8,8,17,2,7)`, in the order x partitions, y partitions, pivots, NLQ pivots,
previous RPU IDs. The remaining thirteen rows are zero. Pointer slots are
filled for `max_prev+2` payload buffers and `max_prev+1` payload handles, with
the following buffer assigned to `payload_handle_empty`. Other decoder bytes,
including the event callback and message-log fields, become zero. The initial
input index is 63 in full-buffer mode and zero in payload-only mode. Output
index and buffer levels become zero, and RBSP state becomes 0 or 4 respectively.

Two fixed-size clears must be preserved exactly. The reference clears only
`0x1004` bytes at the start of the buffer region even when 64 buffers exist.
It clears exactly `0xe070` bytes at the payload base even when the level has
zero partitions, and leaves the larger partition arena's remaining bytes alone.
Consequently the required writable extent is:

`payload_base + max((max_prev+2)*max_partitions*0x7038, 0xe070)`.

For active levels 0..2 this matches `rpu_decoder_mem_query`; for zero rows
3..15 it exceeds that query by `0xe070` bytes. This unit preserves the binary's
behavior. Higher-level validation must select a valid operational level before
allocating solely from the original memory query.

## Differential validation

Build the host DLL with `build_rpu_init.ps1`. From `reconstruction/`, run:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_rpu_init.py
```

The test pins the ARM64 ELF symbols, all eight init relocations, and all 80
level-table words. `_mcount` is replaced by NOP; each imported `memset` applies
its observed zero writes and returns the original destination. The original
decoder reset and cleanup instructions run on initialized arenas as well.

There are 256 matching initialization calls across all sixteen rows, four
payload-only words (`0`, `1`, `UINT32_MAX`, `0x80000000`), and four initial
arena patterns. Arbitrary profile, RBSP, CRC, MEL, and sequence-header flag
words exercise preservation and ignored inputs. Another 64 matching reset
and 64 matching cleanup calls follow initialization after changes to buffer
levels and decoder state. Every allocated arena byte, including preserved
space, is compared after normalizing only known pointers. Return values,
unchanged input flags, and 64-byte guards before and after the arena are checked.
Each initialization's six exact memset destination/size pairs are asserted.

Initialization reaches 163/165 instructions. The remaining two are the
`max_prev_rpu_ids == UINT32_MAX` sentinel path at function offsets `0x28c`
and `0x290`; no in-range pinned level row contains that value. The sequences
reach 72/76 reset instructions and all seven cleanup instructions. The existing
standalone reset test covers all 76 reset instructions, including synthetic
states that initialization cannot create.

`tests/rpu_init_arm64_results.json` records the independent freestanding
AArch64 compile, source/header/object hashes, one public function, and zero
undefined symbols. The shared `build_core_arm64.ps1` already discovers every
`core/*.c` file, so this source needs no build-script edit. This remains a
partial pure-C core unit; it does not provide payload parsing, a loadable
module, or hardware/playback validation.
