# VSVDB target display capability conversion

`control_target.c` recovers `vsvdb_to_target_param` at original ELF
`.text+0x2c0c` (1,236 bytes), shown as `00102c34` by the Ghidra import.
The reference is `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The raw instruction SHA-256 is
`608789e03dbc16fa73d3ccf581937ba8a1e6c6ca4e422fd7eb8a89f526b9e325`.
`evidence/control_target_ghidra.json` retains the decompile;
`tests/control_target_tables.json` records ELF identities, relocations and DWARF.

The target object consists of 22 little-endian 32-bit words, occupying 88
bytes. Exact offsets appear in `control_abi_recovered.h` and are verified
against the binary's `target_param_t` DWARF. The source uses byte writes so
the reconstructed entry supports ordinary misaligned buffers.

The parser accepts the extended tag and Dolby identifier bytes
`[header, 01, 46, d0, 00]`. Version 0 expands twelve-bit chromaticity and PQ
fields; version 1 supports the advertised 12-byte compact and 15-byte forms;
version 2 expands compact chromaticity, interface, 444 capability and PQ
fields. Versions 1 and 2 assign the original fixed D65 white coordinates.
The existing width, height and low-latency request words remain untouched.

The exact partial-write/error behavior is preserved. A null input or invalid
identifier returns -1 without target writes. An unsupported version writes
length/version/sink-DM-version before returning -1. A version-1 block with
an unsupported advertised length additionally writes all four luminance/PQ
words before returning -1; it leaves chromaticity and interface words intact.
Fields belonging to other versions retain their incoming bytes.

The original API does not receive physical buffer length. It reads 21 bytes
for v0, 12 for compact v1/v2, and 15 for long v1 irrespective of the v0/v2
advertised length. Unsupported versions need six readable bytes; unsupported
v1 lengths still need eight. Tests put those exact readable extents against
an unmapped emulator page boundary. Callers should pass their complete
32-byte VSVDB storage and 88 writable target bytes. Input/output must not
overlap, and valid input requires a nonnull target. This recovery does not
add bounds validation to the original contract.

`control_luminance.c` supplies the actual control-local `PQ12ToL`
(`.text+0x83e8`, 104 instruction bytes plus an eight-byte literal) and
`LToPQ12` (`.text+0x8458`, 152 instruction bytes plus an eight-byte literal).
All four exact `.rodata` arrays are in `control_target_tables.inc`; regenerate
them from the hash-pinned module using `tools/extract_control_target.py`.
The control-local L-to-PQ conversion returns 4096 at the 10,000-nit endpoint;
the decoder-local helper instead saturates that result to 4095. Keeping the
helpers separate preserves this difference. PQ-to-luminance clamps unsigned
16-bit input to 4095, interpolates the original table, and returns Q18 units.

From `reconstruction/`, run:

```powershell
& same_kernel/am6b/build_control_target.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_control_target.py
```

`tests/control_target_results.json` records 111,987 successful differential
calls against the original relocated ARM64 instructions. All 309 VSVDB
instructions, 26 PQ-to-luminance instructions and 38 L-to-PQ instructions
were visited. Only `_mcount` profiling calls are replaced by NOPs; both
numeric dependencies execute as original instructions with their real tables.
There are no injected semantic dependencies.

The corpus covers every VSVDB version and advertised length, all sink DM
versions, header/payload byte mutations, all 4,096 v0 minimum/maximum PQ
values, all 16,384 v1 luminance pairs, all 65,536 unsigned-short PQ inputs,
all L-to-PQ table boundaries with adjacent values, and extreme/random
32-bit luminance. Each VSVDB call compares return value, all 88 target bytes,
surrounding guards, and retained input bytes. Finite instruction comparisons
establish this parsing/numeric recovery; they do not establish device playback
or a complete control path.
