# Tone-control preparation and gamma arithmetic

The source is independently recovered from `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The Ghidra program is explicitly `p3i-t4c-dovi.ko`, with `.text` at
`0x100028`. No 5.15 source or behavior is used as proof.

`tools/extract_tone_controls.py` extracts function bytes, DWARF field
offsets and types, and the exact lookup tables. Its output is pinned in
`evidence/tone_controls_elf.json`; the pseudocode and instruction evidence
are `tone_controls_ghidra.json`, `tone_dependencies_ghidra.json`,
`tone_controls_assembly.json` and `tone_controls_additional_assembly.json`
in that directory. The generator also emits `core/tone_tables.inc`
directly from the original little-endian bytes.

| Original symbol | .text offset | Bytes | Functional API |
| --- | ---: | ---: | --- |
| UpdateTcCtrlLevel | 0x70bc | 168 | update_tc_ctrl_level |
| TmCtrl2TcCtrl.isra.3 | 0x789c | 304 | tm_ctrl_to_tc_ctrl |
| UpdateTcSigHlgA.part.13 | 0x7ff0 | 112 | update_tc_signal_hlg_a |
| CreateToneCurve.isra.14.part.15 | 0x8060 | 540 | create_tone_curve_changed |
| CreateToneCurve.isra.14 | 0x827c | 100 | create_tone_curve |
| DeGamma | 0x82e0 | 228 | de_gamma |
| TgtSigEnv2TcCtrl | 0x84f8 | 168 | target_signal_to_tc_ctrl |
| log10Taylor | 0xc4a4 | 72 | log10_taylor |
| sdiv_sls | 0xd25c | 80 | sdiv_sls |
| calcTaTb | 0xff64 | 340 | calc_ta_tb |

The concrete path uses existing independently recovered
`trim_luminance.c`, `control_luminance.c`, and `byte_utils.c`. `ClampLevel2`
has one implementation in `kernel_maps.c`; the standalone build links it.
`sdiv_sls` now lives once in `tone_math.c`, shared with the parameter and LUT
units; its original 80-byte body and arithmetic behavior are unchanged.
The new source preserves 32-bit wrap before signed interpretation, ARM64
variable-shift widths, signed divide-by-zero results and saturation, and
the DeGamma low32 reduction after the optional 64-bit gain product.

DWARF gives TmCtrl 168, DmCtrl 104, TgtSigEnv 204, TcSig 28, TcCtrl 168, and
DmKsTMap 4104 bytes. Byte access preserves untouched fields. HLG is EOTF 3.
An explicit nonzero PQ pair writes only the target minimum, maximum and
diagonal fields, retaining its existing HLG fields. The fallback computes
PQ, writes the HLG flag, and invokes the real Taylor chain for HLG.
Backward invertibility requires the configured flag, direction 1, a
nonzero graphics source and secondary-input 1. Backward noninvertible
mode chooses backward weights and wraps the negated 16-bit midpoint bias.

The optimized CreateToneCurve machine ABI has **three** object arguments:
TcCtrl, kernel tone map, and used TcCtrl. The fourth historical DWARF
argument is unused. The outer wrapper compares the first 160 bytes and
returns 0 if equal. Its part prepares exact source 56 and display 48
structures, passes four 512-entry short LUTs with ratio NULL, and calls
CreateToneCurve3pxx. It copies 160 bytes **after** that callback, retaining
the final 8 bytes of used TcCtrl. The return is 0x100000 when the first and
selected last tone-intensity values are equal, otherwise 1. Direction 2
disables output clamping; direction 1 uses the target crush/mid/clip,
and all other directions use source values with forward mode 0.

CreateToneCurve3pxx is now concrete in `tone_curve.c`. Its 832-byte body calls
the recovered CalcMappedParams3pxx 1296, CalcCurveParams 1976,
GenerateToneCurveLut 1952, ToneMapS 448, SaturationMapS 448, SaturationMapI 580,
and their real arithmetic/LUT helpers. `struct tone_curve_ops` remains the
typed interface: a null operations pointer or null `build` member selects
`tone_curve_concrete_ops()`. A supplied builder is an explicit override.
Equal input does not inspect the operations pointer. Source/display padding
and output parameter initial contents remain unspecified, matching the
original stack locals. `tone_curve.recovery.md` records the concrete builder,
the formal finite LUT/capacity domain, direct tests and component coverage.

`build_tone_controls.ps1` builds the native DLL with MSVC `/W4 /WX /GS- /Zl`
and `/NOENTRY /NODEFAULTLIB`. Run the differential suite from reconstruction:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_tone_controls.py
```

The historical `test_tone_controls.py` boundary audit remains a separate
fixture. Its strict executor runs the selected original controls as relocated
ARM64 instructions, modeling `_mcount` and the deliberately injected
CreateToneCurve3pxx boundary. Fixed-point, clamp, PQ, byte equality and byte
copy callees execute original code. Literal pointers just outside GCC
function sizes are patched only using their exact ELF ABS64 relocations.
This audit compares every initialized source/display field, all four LUT
addresses, null ratio, count 512 and clamp mode; its callback fills all LUT
bytes and demonstrates the cache copy occurs after callback mutation.
`test_tone_curve.py` separately executes the real original and recovered
builder with no numeric dependency model, including default null operations.

The historical boundary-audit report records 11568 comparisons and 82836 guard checks, hashes
of all source/build/test/evidence inputs, exact function/table pins, and a
deterministic transcript. Tests compare complete objects and arena bytes;
they exercise full-width gamma arithmetic, saturation and zero division,
sum 65535/65536, PQ fallback and retained HLG fields, level clamps and wrap,
all invertibility conditions, first 160 cache behavior and ignored final 8,
LUT index 0/1/255/511, output aliases Ta=Tb, same input/output control
buffers, target/input alias, and used/input aliases. Every instruction in
all 20 selected concrete functions is visited. This is finite differential
evidence for the stated domain, not a proof for arbitrary invalid pointers
or tone endpoint indices outside 0..511. It makes no device adapter, kernel
load or hardware playback claim. Historical reports are preserved by hash
before updating the active report.
