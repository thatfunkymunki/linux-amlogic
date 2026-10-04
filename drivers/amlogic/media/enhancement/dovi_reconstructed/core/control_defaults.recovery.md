# DM signal, tone, configuration and extension defaults

`control_defaults.c` recovers six original fixed-value initialization helpers
from `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.

| Original helper | ELF .text offset | Instruction bytes | Reconstructed entry |
| --- | --- | ---: | --- |
| InitGrcSigEnv | 0x5954 | 224 | control_init_grc_signal |
| InitSrcSigEnv | 0x5a90 | 216 | control_init_src_signal |
| InitTgtSigEnv | 0x5c58 | 196 | control_init_tgt_signal |
| InitTmCtrl | 0x5d94 | 232 | control_init_tone |
| InitDmCfg | 0x5fb8 | 188 | control_init_config |
| InitMdsExt | 0x60e8 | 324 | control_init_extensions |

`evidence/control_defaults_elf.json` records each instruction SHA-256,
relocation map, exact nested DWARF offsets/widths, and scalar enums.
Regenerate it with `tools/extract_control_defaults.py`. The original
decompiles are retained in `evidence/control_init_dependencies.json`.
Source accesses are little-endian byte reads/writes, preserving ordinary
unaligned memory and the original mixed 16-, 32- and 64-bit field widths.

These helpers write selected defaults and retain all other object bytes.
They do not clear row-pitch words, coefficient arrays, unused trim storage,
or padding. The source signal helper also retains its chroma-format word.
The tone helper retains `Default2[0]`, `CodeBias2[0]`, `l2off`, and its
padding. The configuration helper stores its memory-manager handle verbatim,
selects the original CPU platform regardless of the platform argument, and
calls all four actual signal/tone helpers. It returns the original config
pointer. The memory-manager handle may be null; it is not dereferenced here.

The extension helper initializes min/mid/max PQ to 0/2047/4095, configures
two trim descriptors, and writes the first trim entry in each descriptor.
Its two 64-bit pointers are stored at extension offsets +96 and +112,
pointing to extension +120 and +358 respectively. The first entry contains
seven halfwords; the second contains four. Remaining entry values are the
corresponding default-minus-code-bias differences truncated to 16 bits.
Full 32-bit input arithmetic and halfword truncation preserve negative and
overflow differences. This object contains self pointers and must remain
at the initialized address while those pointers are in use.

All buffers must meet their DWARF sizes: graphics/source signal 228 bytes,
target signal 204, tone 168, config 984, and extension 520. The original
helpers do not check null pointers or physical bounds. Configuration and
extension buffers must not overlap. This recovery is limited to ordinary
memory and does not add validation to the original interface.

From `reconstruction/`, run:

```powershell
& same_kernel/am6b/build_control_defaults.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_control_defaults.py
```

`tests/control_defaults_results.json` records 16,177 matching top-level
calls against actual ARM64 instructions, covering all 345 original
instructions. The original InitDmCfg runs the original nested helpers;
only `_mcount` profiling calls are replaced by NOPs. No semantic dependency
is injected. Fixed patterns and random buffers exercise alignments 0..7,
retained fields and padding, object guards, repeated initialization,
ignored platform values, null/arbitrary 64-bit handles, returned config
identity, and initialized/raw tone configurations. Extension comparisons
verify both self pointers before normalizing their concrete addresses,
preserve all input config bytes, and exercise subtraction around 16- and
32-bit boundaries. These tests cover initialization behavior, not DM frame
processing or playback.
