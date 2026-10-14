# Register commit orchestration

Source is the pinned P3i T4c module, SHA256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The original `commit_reg` occupies `.text+0x30e0`, 6200 bytes, SHA256
`1dac3ce2370dd97ac43ba1a8aaa649e0710330566e28451991b58546556194c3`.
The independently executed configuration commit `CommitDmCfg` occupies
`.text+0x8760`, 244 bytes, SHA256
`61e4db19c4ac2f27f8678da81973dab8d0ead3bedde6981a77ea5776fef4c06b`.

`commit_reg_with_ops` retains the fourteen original operands and adds explicit
bindings. It recovers source/graphics/target configuration, delayed priority,
VSVDB low-latency selection, metadata-to-extension conversion, compositor
identity setup, all three register maps, packed raw metadata and both 5120-byte
LUT outputs. It uses the concrete compositor packer, control PQ tables, fixed
metadata and HDMI preparation; a null trim callback selects actual trim
reduction. The original ignores the configuration commit return, propagates
the metadata commit return and preserves partial output even on that error.
Unsupported enhancement-layer input returns -1 after updating the target
pointer, before subsequent configuration/register writes.

`control_commit_dm_config` performs the actual forward 984-byte copy, source
limits override for video priority, active configuration comparison, validity
updates and frame pointer calculations. Machine instructions show frame
offsets of **0x10e0000 and 0x21c0000 bytes**: the decompiler displayed half those
values because the original pointers point to 16-bit samples. Its apparent
null-handle error is unreachable for ordinary memory because it loads the
pending pointer before checking the handle; the exported caller contract
requires a valid handle.

`CommitMds` remains a required typed binding for the unrecovered matrix/tone
mapping algorithms. Successful host/link tests do not supply those algorithms.
A configuration callback and trim callback exist for deliberate mutation
audits; ordinary configuration/trim calls use concrete recovered code.

`tools/extract_register_commit_abi.py` independently extracts exact nested
DWARF sizes/offsets, including 1800-byte composition, 71-byte packed metadata
base, 261-byte extensions, trim descriptors, kernel maps and register/LUT
records. Enumerator names can have different values in separate compilation
units; the evidence records every distinct value and machine execution fixes
the control domain (PQ EOTF is 1). Full original instruction evidence is in
`evidence/register_commit_disassembly.json`; configuration decompilation is in
`evidence/register_commit_config_ghidra.json`.

The differential harness executes original relocated ARM64 with the full
fourteen-argument stack ABI. It compares complete context, metadata,
extensions, both configurations, both kernel arenas, registers, LUTs and guard
bytes; only validated address slots are normalized. It covers every input and
output enum plus unknown values, target versions/lengths/interfaces, RGB
capabilities and dimensions, priority persistence, graphics formats, unsigned
arithmetic wrap, signed bit depth, 8-bit compositor defaults, metadata trims,
all return values, and callback pointer/configuration changes. Size-triggered
HDMI cases run both explicit mutation callbacks and actual trim code. The
signed trim dimension cases -32 and -40 write before Trima while remaining
inside its supplied extension arena. Addressing preserves signed displacement
and descriptor/pointer reloads after every field store. The callback supplies
`CommitMds` state/return only, recorded plainly in the
results. Exact totals, output transcript, native DLL/source hashes and measured
instruction coverage are in `tests/register_commit_results.json`.

The original has unchecked pointers/counts, signed trim descriptors and raw
metadata lengths. Tests provide storage for every accessed byte. Callers must
do the same; no new bounds validation or format clamping is added. This is
finite host/original-machine comparison, not a hardware playback result or a
runtime kernel ABI adapter.
