# DM payload and extension readers

This unit targets `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Ghidra program `p3i-t4c-dovi.ko` loads ELF `.text` at `00100028`.
The 5.15 implementation and wire encoder corpus were study material; executable
evidence here comes exclusively from the pinned instructions. The AM6B+
calling convention and several differences were recovered independently.

| Original ELF function | `.text` offset | Bytes | Instructions visited |
| --- | --- | --- | --- |
| `dm_data_payload_read` | `0x19680` | 1916 | 479/479 |
| `ext_blocks_read` | `0x19280` | 980 | 245/245 |
| `ext_block_payload_read.isra.0` | `0x18998` | 2252 | 563/563 |
| `update_dm_extensions` | `0x15004` | 628 | 157/157 |
| `remove_dm_extensions` | `0x14e68` | 412 | 103/103 |
| `store_md` | `0x16990` | 140 | 35/35 |

`dm_payload.ghidra.json` preserves disassembly and decompilation; the
compiler-optimized extension reader's DWARF prototype is misleading. Its
actual register ABI is `x0=context`, `x1=event_callback_slot`, `x2=record`,
`x3=bitstream`, `x4=current_record_index_pointer`. The second argument points
at the callback slot; it is not a direct function pointer. `dm_payload.h`
exposes that ABI explicitly, with opaque buffers and recovered bitstream.

## DM behavior

DM reading clears the 256 availability counters at context `+0x67c`, reloading
the context pointer for each write. It reads three Exp-Golomb IDs into byte
fields, preserving truncation. The sequence header byte at header `+0x14`
controls reuse. Value 0 parses all scalar fields and sets previous-DM available
before parsing; value 1 copies the cached 67396-byte payload only when that
flag equals exactly 1, otherwise emits the original optional warning and
writes defaults. Other byte values preserve existing scalar payload fields.
All nonzero modes merge extension updates into the selected existing payload.

Fresh parsing reads nine 16-bit matrix coefficients, three 32-bit offsets,
nine further 16-bit coefficients, EOTF fields and signal/PQ fields. Signed
coefficients preserve their stored low 16 bits. Only the original default
stores are reproduced; unrelated padding and extension bytes survive.

Both extension passes use real recovered list/payload readers. The second
pass is gated by signed 64-bit `tail - next > 4`. Fresh parsing appends it to
the first list before saturating the reported count at 255, retaining the
original writes beyond the nominal 255-entry payload. Merge modes reuse
decoder extension/scratch areas at `+0x107c8` and `+0x20ec0`.

Successful fresh parsing stores the current metadata and removes extension
levels 1 through 5, 8 and 9 from the cached copy. Merge and removal order
records by extension level, preserve duplicate entries where the algorithm
does, and copy the 4-byte length, 1-byte level and 256-byte payload while
preserving record padding. They perform the original sentinel type reads at
the next position, including when that position equals the count. Error
paths preserve already-written state. The return value is the wrapping
32-bit bit count difference; list readers return zero on success.

## Extension behavior and differences

Records have a 264-byte stride, with length at `+0`, level at `+4`, and payload
at `+6`. The decoder context retains per-level availability and index bytes at
`+0x67c` and `+0x77c`. Levels 1, 3, 4, 5, 6, 9, 11, 254 and 255 reuse an
existing record on duplicate input, retaining its old header. Multiple trims
are limited to 8 entries at L2, 5 at L8, and 4 at L10; discarded trims still
consume their encoded bits. Rejected lengths above 256 or beyond the nominal
input tail return `-2`. Payload minimum lengths are not enforced: known levels
can read more than their declared length when padded backing exists.

the reference module's optimized payload helper takes five parameters, whereas the 5.15 study
helper takes a decoder and four parameters. only emits the payload-2 level
warning when `payload_num == 2`; other values besides 1 and 2 do not emit that
warning. consumes skipped payload bits with `read_u_v(1)`. Its unknown-level
direct reader does **not** cap the byte loop at 256. The list reader separately
enforces the 256-byte length limit; direct callers must supply backing for
their full declared length. Direct tests include 257- and 300-byte unknown
payloads with sufficient storage. diagnostic source paths and line values
are pinned to its own strings and instructions.

## Evidence and caller contract

`tests/test_dm_payload.py` runs 1593 complete DM fixtures plus 786 direct
extension-payload calls; each starts or continues an actual recovered
bitstream. Including initialization, 4758 top-level calls match. All six
recovered functions reach every instruction. Tests execute original AM6B+
bitreaders, alignment, error lookup, extension parsing, merging, removal and
store functions. There are no parsing/validation stubs in this evidence.

Only `_mcount`, `memcpy`, `memset`, and the six bounded decimal `sprintf`
formats are modeled imports. The reference's callback events are observed and
compared with the candidate. Literal-pool pointers are relocated using their
original ELF relocation records; no instruction bytes are replaced.
Each fixture compares return values, ordered diagnostics/events, every byte
of decoder/context/root/header/DM/input objects and surrounding guards, with
known pointers normalized. Fixtures include every extension level, scalar
boundaries, unknown levels, optional matrices, duplicate headers, trim limits,
alignment/refill quirks, truncated counts, combined-count overflow, logging
gates, cached metadata and repeated frames. Existing shared corpus files are
wire encoders and initial-state generators; they do not supply expected
decoded output. Negative checks verify callback and pointer comparison.

This preserves the unchecked original contract on externally serialized,
disjoint, backed objects. Supply the full decoder/root/header/context,
padded readable input and writable DM/extension/scratch areas. Count overflow
fixtures provide expanded DM storage for the actual pre-saturation writes;
they do not prove safety of the nominal allocation under those malformed
inputs. Stale record lengths are bounded in fixtures to avoid unbounded bit
consumption. Sentinel reads require real backing. Null/invalid pointer fault
ordering, races, callback-induced aliasing changes and hardware playback are
outside this finite evidence.

## Build and reproduction

Run from `reconstruction/`:

```powershell
./same_kernel/am6b/build_dm_payload.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_dm_payload.py
./same_kernel/am6b/build_dm_payload_arm64.ps1
```

The host DLL builds with MSVC `/W4 /WX` and `/NODEFAULTLIB`.
`tests/dm_payload_results.json` records original function hashes, source/
library hashes, corpus digest, calls, coverage and modeled imports.
`tests/dm_payload_arm64_results.json` records the freestanding AArch64
compiler output and successful relocatable link of these units with recovered
bitstream/alignment/error dependencies, with zero unresolved symbols. This
is an object-build milestone; no kernel module or AM6B+ playback is claimed.
