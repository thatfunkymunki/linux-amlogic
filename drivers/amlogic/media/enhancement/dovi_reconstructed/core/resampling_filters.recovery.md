# explicit resampling filter readers

Recovered independently from Ghidra program `p3i-t4c-dovi.ko`, with the
program argument supplied on every analysis call. The input module SHA-256 is
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The 5.15 filter/test was reviewed for harness structure only; every offset,
instruction, relocation and behavior below was checked against AM6B+.

| Helper | ELF .text offset | Bytes | Ghidra address |
| --- | --- | --- | --- |
| `rpu_data_chroma_resampling_filter_1D_exp_read` | `0x17428` | 672 | `0x117450` |
| `rpu_data_spatial_resampling_filter_exp_read` | `0x17258` | 252 | `0x117280` |

Ghidra maps `.text` at `0x100028`; its displayed addresses are not ELF offsets
plus `0x100000`. Saved Ghidra disassembly and decompilation are in
`../evidence/chroma_filter.disassembly.json`,
`../evidence/spatial_filter.disassembly.json` and
`../evidence/resampling_filters.decompiled.json`.

## Layout and recovered behavior

Both readers receive the raw decoder layout. Decoder `+0` points to the
24-byte RPU data root (`+0` header, `+8` payload), `+8` points to the RPU
context, and `+0x40` contains the 32-byte bitstream. Chroma reads the message
callback at decoder `+0x315c8` and unsigned severity at `+0x315d0`.
These decoder-local fields are separate from the 48-byte T4c host ABI.

The chroma helper uses a payload partition stride of `0x7038`, and chooses
component/direction with `combination = component*2 + direction`:

- Signed predictor count: context `+0x2d8 + combination*4`.
- Prediction flag: partition `+0x649c + component*0x50 + direction*0x28`.
- Prediction delta: partition `+0x64c0 + component*0x50 + direction*0x28`.
- Explicit coefficient: partition `+0x64a0 + component*0x50 + direction*0x28`.
- Coefficient offset: partition `+0x6f54 + combination*4`.

Nonpositive predictor counts force a zero prediction flag without consuming
that bit. Positive counts consume one bit. A set flag with one predictor uses
partition zero; greater counts consume an Exp-Golomb delta, add one and
truncate it to a byte before subtracting it from the current partition.
Invalid predicted partitions return `-2` after updating the flag/delta and
bitstream, leaving coefficients, offset and count unchanged. When the log
callback exists and unsigned severity is greater than two, the five integer
format arguments match the reference exactly, including the predicted index.

Valid prediction copies the earlier partition's coefficient offset and does
not increment the predictor count. Explicit parsing uses header `+0x20` for
horizontal coefficient count or `+0x1f` for every nonzero direction. It reads
`header[0x0c]+1` bits for each coefficient and sign-extends using the original
masked 32-bit shift. It writes the local coefficient offset and increments
the predictor count modulo 32 bits, including when coefficient count is zero.

Spatial parsing starts at payload `+0x6544 + (direction*6+pivot)*0x20`.
Each positive phase advances by `0x60`, and has eight taps for direction zero
or six taps for any nonzero direction. Each tap stores a signed Exp-Golomb
integer, then `header[0x0b]` fraction bits at integer address `+0x180`.
Nonpositive phase counts consume no input and write no object state. This
helper always returns zero and does not use callbacks.

The original helpers do not validate indices, coefficient counts or
available bitstream length. The C API retains that contract. Callers must
provide accessible backing for every selected object and padded input.

## Differential validation

`../tests/resampling_filters_results.json` records 2,297 matched calls:
1,381 chroma and 916 spatial, with 351 matched log callbacks. All 168 chroma
and 63 spatial instructions were visited. Every feasible conditional branch
successor was visited. Chroma `+0xfc` has one infeasible successor: a one-bit
read returns zero or one, and zero branches to explicit parsing before the
comparison with one. The remaining reader coverage is reported separately;
`read_u_v` is 101/106 here, and `read_ue_v`/`read_se_v` are complete.

The corpus covers all 256 denominator bytes, all 256 chroma length bytes,
signed predictor boundaries, all 256 prediction delta bytes, larger
Exp-Golomb truncation cases, callback absence and unsigned severity limits,
zero/negative/positive spatial phases, horizontal/vertical layouts,
refill/tail boundaries, counter wrap and repeated calls on evolving states.
Spatial phase/pivot tests include noncanonical but accessible direction and
pivot values; byte-sized malformed coefficient widths and lengths retain
reference writes within deliberately oversized payload backing.

The harness executes the original ARM64 readers, applies only ELF-specified
call/literal relocations, and substitutes one `_mcount` call per executed
function with NOP. The chroma message literal is relocated from `.rodata`
`+0x1788`, using the eight-byte literal at `.text+0x176c8`.
No parsing or coefficient operation is hooked. Complete decoder, RPU root,
header, context, payload and input snapshots match, including 16-byte
canaries; only known pointer fields are normalized. Read-only mappings
protect the reference root, header and input. The report records every object
write range and conditional successor, hashes before/after, seven rejected
raw-pointer normalization checks, one callback mismatch check and three
state mismatch checks.

Build and run from the project root:

```powershell
.\reconstruction\same_kernel\am6b\build_resampling_filters.ps1
uv run --cache-dir .\reconstruction\.cache --python .\reconstruction\.venv\Scripts\python.exe --no-project .\reconstruction\same_kernel\am6b\tests\test_resampling_filters.py
```

The native DLL passes `/O2 /W4 /WX` and links without CRT dependencies.
The aggregate `build_core_arm64.ps1` discovers this C source automatically.
Its freestanding object build remains a separate milestone from a T4c
kernel module. Out-of-backing accesses, live module loading and Dolby
playback are outside this validation.
