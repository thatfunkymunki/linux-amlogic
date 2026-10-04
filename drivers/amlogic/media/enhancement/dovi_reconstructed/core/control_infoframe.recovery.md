# Control infoframe and fixed metadata

Recovered independently from Ghidra program `p3i-t4c-dovi.ko`, module SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The reference remains byte-identical to the supplied `s922x-j_dovi.ko`.
The `.text` section offsets below refer to ELF bytes. Ghidra instruction
addresses in this import differ by an additional `0x28` after the section base.

| Function | ELF `.text` offset | Bytes | Original code SHA-256 |
| --- | ---: | ---: | --- |
| `get_info_frame` | `0x2680` | 1036 | `bc5f388a954346a41637a5567760c50bcfc47bf3c445b0f6b35f0686db30a75d` |
| `get_fixed_metadata` | `0x1f60` | 452 | `0e71ea341b27e43ff1c22f3e175c61a39d75d5bef7a7386fe90684794ded34c0` |
| `GetStbTmm` | `0xa148` | 380 | `35555ba48127363e24ff019e81e9383aff86c4411737813ec8f672c07ffa359e` |
| `PQ12ToL` dependency | `0x83e8` | 104 | `a94c3f3adc7cf3e4a77002a5cf3040b76544d2e090fe7d0478d0aa49ee4e7fce` |

## Behavior and ABI

`get_info_frame` writes a 29-byte output containing a five-byte header and
the HDR payload. For format 1 it copies HDR10 primaries, truncates minimum
luminance to 16 bits and converts the unsigned 32-bit maximum from units of
0.0001 nit to integer nits, retaining the low 16 bits. Formats 0 (Dolby Vision)
and 2 (SDR) use the binary's fixed primary coordinates and the target luminance
fields. Unrecognized formats return 0 after changing only the header; they
leave the other 24 bytes unchanged.

For Dolby Vision with `ctx->dovi2hdr10_nomapping != 0`, it converts the source
PQ words with the actual control-local `PQ12ToL` table, multiplies by 10000
and shifts right 18. It processes all packed extension records in input
order. Every level 6 overwrites MaxCLL and MaxFALL. The mastering-luminance
quartet overwrites the previous values only if at least one of those four
bytes is nonzero. A later level 6 with four zero mastering bytes consequently
retains the earlier mastering luminance while replacing the light levels.

This unit deliberately uses opaque buffers with byte-wise field reads.
The source metadata is the **packed control layout**: 71-byte base, extension
records beginning at byte 71, 261-byte extension stride, level at record byte
4 and payload at record byte 5. It is different from the parser's aligned
DM payload. `control_abi_recovered.h` supplies the shared DWARF sizes/offsets.
The context DM handle is at byte 0, the no-mapping flag at byte `0x102c`, and
target minimum/maximum are unsigned 32-bit words at bytes 40/44.

`get_fixed_metadata` overwrites only the 71-byte metadata base. It emits the
original constant matrices/format fields, fetches target minimum/maximum PQ
from the concrete getter, stores those words in big-endian byte order, sets
source diagonal 42 and extension count 0. Existing extension storage remains
untouched. The getter output parameters are passed in the original order,
with both luminance outputs null.

`control_get_stb_tmm` implements the full `GetStbTmm`, including optional
luminance outputs. Null or invalid DM handle returns -1 without changing
outputs. For a valid handle it writes outputs in the original order:
minimum PQ, minimum luminance, maximum PQ, maximum luminance. PQ words are
at handle bytes `0xa9c` and `0xa9e`, configuration-valid word at byte 4,
active-config pointer at `0x7b8`, kernel-state pointer at `0x1128`.
When config color at `0x310` equals 3 and mode at byte 8 equals 1, the
binary inlines the same clamped PQ interpolation recovered as
`control_pq12_to_l`. Other modes use config luminance words at `0x3b8` /
`0x3bc`; other colors use kernel-state words at `0x2a4c` / `0x2a50`.

The interpolation helper and its exact original lookup bytes live in
`control_luminance.c` and `control_target_tables.inc`, independently recovered
by the target-parameter unit. `depq1024Lut` is section 8 offset `0x103a0`,
4096 bytes, SHA-256
`7497ef7ec322277f9a391be6621acca94bad4aa4eab3d8eb75baea859cd20a7e`.

## Differential evidence

Build with `build_control_infoframe.ps1`, then from `reconstruction/` run:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_control_infoframe.py
```

The 8,507 calls compare the compiled host C against original AArch64 machine
instructions using the SHA-pinned shared `Reference` harness. All 259
infoframe instructions, 113 fixed-metadata instructions, 95 getter
instructions and 26 PQ conversion instructions are visited. Only `_mcount`
is modeled as a no-op; DM getter and PQ logic execute their actual original
instructions. The two original ABS64 table literals just outside function
symbols are relocated using their ELF relocation records at `.text+0x8450`
and `.text+0xa2c8`; no original code instruction is replaced.

Cases cover recognized/unrecognized formats, null unused inputs, unsigned
luminance boundaries, all 4096 PQ codes and values above the clamp, every
extension-level byte, zero/nonzero level-6 mastering quartets, repeated
levels through count 255, constant metadata with random previous contents,
all 16 optional-getter-output masks, config color/mode paths and
null/invalid getter handles. Complete outputs, unchanged sources/configs,
unwritten output bytes and 204,168 guard comparisons match. The original
dependency call order is also checked against the expected path; the native
functions call concrete recovered dependencies and do not expose a separate
call trace. `tests/control_infoframe_results.json` pins the own
sources, shared luminance dependency sources, DLL and transcript hashes.

## Limits

Objects are distinct, readable/writable for their full recovered layout,
and required pointers are valid. Aliasing is not covered. A valid DM handle
is required for `get_fixed_metadata`: the original ignores `GetStbTmm`
failure and then reads uninitialized stack words. No equivalence claim is
made for that undefined caller case. `get_fixed_metadata` has no specified
return value. These are host-versus-ARM64 comparisons, not kernel loading,
module adapter or device playback validation.
