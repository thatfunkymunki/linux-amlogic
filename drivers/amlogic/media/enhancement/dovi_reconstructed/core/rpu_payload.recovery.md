# RPU payload parser

`rpu_payload_read` reconstructs the complete 4,768-byte AM6B+
`rpu_payload_read` body at ELF `.text+0x176d0`, Ghidra `001176f8`.
The target is `p3i-t4c-dovi.ko`, byte-identical to `s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The original instruction bytes hash to
`dcb002684163e67d377fa855e9abbcf7e02652d4c3a4253e1a4c25b599791d3a`.

The older 5.15 C and input encoder supplied study structure. Every implemented
branch, field offset, width and helper call is independently checked against
the Ghidra listing and real ARM64 differential execution. The files
`evidence/rpu_payload.ghidra.txt` and
`evidence/rpu_payload.disassembly.json` retain the explicit-program analysis.
AM6B+ has no legacy coefficient/pivot limit traps; this source introduces none.
also delegates spatial filters to its recovered helper and retains a
different source-path diagnostic string and an omitted diagnostic argument.

## Actual dependencies and layout

The source links the independently recovered bit reader, pivot accumulator,
chroma and spatial filter readers, and error-string lookup. The oracle executes
their original instructions: no parsing operation is replaced by a success
callback. Only `_mcount` and the recording logger are modeled.

| Region | offset / stride |
| --- | --- |
| Decoder root / context / bit reader | `+0`, `+8`, `+0x40` |
| Root header / payload | `+0`, `+8` |
| Decoder logger / severity / MEL flag | `+0x315c8`, `+0x315d0`, `+0x316e0` |
| Payload partition | `0x7038` bytes |
| Mapping id / prediction flag | `+0`, `+0x2d`, component stride 15 |
| Mapping records | `+0x5c`; POLY `0x30c`/`0x34`, MMR `0xa8c`/`0xb4`, POW `0x258`/`0x28`, LUT `0x2000` per component |
| Mapping coefficient offset table | `+0x6ea0`, component stride `0x3c` |
| NLQ flags / records / offset table | `+0x6844`, `+0x6874`, `+0x6f74`; pivot has three components |
| Context predictor counts | `0..0x39b`; all cleared on every call |
| Context first-partition histories | Begin at `+0x39c`; retained between calls |

The test records original offsets, sizes and instruction hashes for every
selected dependency. It applies eight original `R_AARCH64_ABS64` literal-pool
relocations outside symbol `st_size`: five payload strings at `0x18970..0x18990`,
one chroma string at `0x176c8`, and error lookup literals at `0x16830/0x16838`.
Allocated ELF data relocations are handled by the shared strict reference runner.

## Behavior and preserved quirks

Entry clears predictor counts before checking RPU type or use-previous flag.
Pivot accumulation can return `-13` before payload bits are read. The parser
then walks the header's partition grid and three mapping components, correcting
mapping pivot counts below two only after pivot accumulation.

Mapping kinds 0 through 3 parse polynomial/interpolation, multi-regression,
five-pair power mapping and LUT records. Unknown kinds preserve existing
prediction flags and can therefore trigger the ordinary range error. Fresh
records increment counters and publish offsets; predicted records reuse an
earlier partition's offset. Mapping prediction dereferences the supplied
previous-partition storage **before** checking its range. LUT endpoints above
1024 return `-2` with the original ordered diagnostics.

Polynomial order is stored as a byte after adding one; coefficient parsing
uses that byte without a record-array capacity guard. MMR order four writes
beyond its three-order array within the supplied union backing. Tests compare
these writes instead of inheriting the 5.15 traps.

Base-format chroma filters are parsed through the real helper. Explicit
spatial filters and pivots are read only for the first partition, including
one or two vertical phases. NLQ parses linear-dead-zone, mu-law, and unknown
method cases with 16-bit offset fields and wrapping 32-bit values. It sets MEL
for each NLQ partition, then clears it for the same conditions as the binary;
predicted NLQ records do not reevaluate the previous record's MEL fields.
The first partition can also read four 17-tap EL chroma coefficient groups
using the original masked sign extension.
The mask is `1u << ((width + 1u) & 31u)`, preserving the AArch64
shift-count wrap at widths 31, 63 and other values congruent to 31 modulo 32.
Independent review reproduced a width-31 mismatch in the initial C mask;
the corrected implementation and exhaustive byte-width corpus now agree.

The first-partition history tables intentionally survive successive payload
calls. A zero-valued first-partition entry can be replaced by a later fresh
partition. Four five-frame differential streams show that retained histories
can produce a reuse error on the next frame, followed by successful fresh
frames without clearing the parsed payload arena.

the reference module's NLQ prediction-range format contains five `%d` conversions, but its call
site supplies only four integer arguments (`001186f0..00118710`). The C keeps
that omission. Evidence compares the format and four supplied arguments;
the unspecified final vararg is excluded rather than given an invented value.
Loggers must record events without mutating caller-owned parsing objects.

## Validation

Run from `reconstruction/`:

```powershell
& .\same_kernel\am6b\build_rpu_payload.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_rpu_payload.py
```

The final evidence records **1,270 payload calls + 1,270 bitstream initialization
calls**, including 1,246 encoded fixtures, 20 persistent frames across four
streams, and four nominally truncated packets with explicit readable backing.
The payload body has **1,192/1,192** instruction coverage. Both pivot helpers
and both filter helpers also have full instruction coverage in this suite.
Bit-reader and diagnostic helper coverage is recorded separately; their
remaining paths retain their standalone recovery evidence.

Each call compares the return, entire decoder/header/context/payload/root/input
objects, leading/trailing guards, normalized pointers, and ordered logger
events. The encoded corpus includes skip/empty-grid paths, all mapping kinds,
byte-wrapped orders, interpolation, LUT boundaries and errors, predictor
reuse and errors, NLQ/MEL variants, explicit filters, bit-width edges through
255, counter wrap, count clearing, retained histories, and 128 varied grids.
The NLQ, chroma and spatial coefficient-filter paths each test all 256
header-byte widths, including 256/257-bit read requests where specified by
the original. Instruction coverage and operand-domain coverage are recorded
as distinct checks.
The report pins source, header, dependencies, corpus, tests, DLL and raw analysis
hashes in `tests/rpu_payload_results.json`.

Freestanding AArch64 compilation succeeds with `-Wall -Wextra -Werror`.
`tests/rpu_payload_arm64_results.json` records the unit object and a scoped
relocatable link with existing recovered dependency objects; the linked object
has no undefined symbols. The shared aggregate build will rebuild every C unit.

## Scope and integration

The runner builds `rpu_payload` and executes
`tests/test_rpu_payload.py`. The aggregate core build discovers
`rpu_payload.c`. `decoder_bindings.c` supplies `rpu_payload_read` to processing
orchestration's payload operation when integrating the parser.

The tested domain requires valid, disjoint, externally serialized objects;
pivot counts 0 through 16; grids up to four partitions; and explicit readable
bitstream/prediction padding. Bounded polynomial order 255 and MMR order four
are covered in sufficient writable union backing. Unmapped prediction targets,
arbitrary malformed counts, aliasing or state-mutating callbacks are excluded.
The parser intentionally retains the original unchecked storage assumptions.
This evidence establishes the scoped payload unit and real dependencies;
it does not establish an entire metadata stream, host ABI, kernel module load,
live-device compatibility or Dolby playback.

The separate `tests/test_decoder_pipeline.py` now supplies real header,
level/spatial/profile validation, payload, MEL and DM functions through
`decoder_bindings.c`. See `core/decoder_bindings.recovery.md` for its
direct processing, persistent frames and push/get-output queue evidence.
