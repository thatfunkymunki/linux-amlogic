# Tone parameter recovery

`calc_mapped_params` and `calc_curve_params` recover the complete numeric bodies
of `CalcMappedParams3pxx` and `CalcCurveParams` from the supplied P3i T4c module.
The target is the user's CoreELEC AM6B+ image; this unit makes no live-device or
playback claim.

## Pinned evidence

- Program: `p3i-t4c-dovi.ko`, module SHA-256
  `5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
- Ghidra `.text` base: `0x100028`.
- `CalcMappedParams3pxx`: `.text+0xe430`, Ghidra `0x10e458`, 1296 bytes,
  SHA-256 `e80cc09821e7213d21d0c43af46a5c2f328dea25a61deb8e19a5092840f8b144`.
- `CalcCurveParams`: `.text+0xef00`, Ghidra `0x10ef28`, 1976 bytes,
  SHA-256 `812f62e37fc1b7585ab071f787131caa00ec1c864cd056a165b2f74357cdfd4c`.

`evidence/tone_parameters_elf.json` pins original function bodies and the real
15-function arithmetic closure. `evidence/tone_parameters_disassembly.json`
records exact Ghidra listings; `evidence/tone_curve_generation_ghidra.json`
records pseudocode. The source/display/pair layouts come from the pinned DWARF
in `evidence/tone_controls_elf.json` and use `tone_controls.h`'s existing types.

## Semantics

The mapped routine retains mode-dependent source/display selection, optional
brightness preservation and its fifth-power ease calculation, screen-size
log/square-root ratio, contrast/brightness mapping, bounds, and midpoint limits.
The curve routine performs all six log/power conversions, normalization,
three-point coefficient arithmetic, inverse-denominator division, and pair
normalization limits. Numeric helper bodies are in `trim_luminance.c` and
`tone_math.c`; neither routine substitutes callbacks or dependency models.

The C implementation explicitly uses 64-bit `long long` for the original LP64
`long` operations, retaining AArch64 low-word wrap, low-5/low-6-bit masked shifts,
signed right shifts, saturating helper calls, 16-bit pair norms, and original
output store/reload order. Pair padding remains unchanged. All pointers must
refer to complete writable output objects and readable input objects.

## Differential validation

Run from `reconstruction/`:

```powershell
& ./same_kernel/am6b/build_tone_parameters.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_tone_parameters.py
```

The strict MSVC `/W4 /WX /NODEFAULTLIB` builds exercise both `/Od` and `/O2`.
Each variant compares 1,228 mapped and 675 curve calls byte for byte against
relocated original AArch64 code, with 34,254 guard checks and identical result
digests. Tests include 250 supported mapped-to-curve parameter pipelines,
250 full-width cases per routine, every scalar input field at 21 signed/full-width
boundaries, mode/preserve combinations, all 21 mapped output pair aliases and
all-output alias, and all six curve output pair aliases and all-output alias.

Instruction coverage is complete: mapped 324/324 and curve 494/494. All selected
dependency bodies have complete instruction coverage except `unorm_u` at
18/19: relative `0x24` is not reached through this parameter
corpus. Scalar-helper validation is recorded separately in the tone-math
recovery. The original `_mcount` is the sole modeled call; all numeric
arithmetic executes as original instructions. Literal relocations beyond symbol
bodies are applied from their original ELF records.

Reports: `tests/tone_parameters_Od_report.json` and
`tests/tone_parameters_O2_report.json`, with SHA-256 snapshots, pinned body
hashes, exact coverage and missing offsets, group counts, and transcript digest.
Historical reports are retained by content hash. Input/output overlap and
partial-byte output overlap are outside this unit's tested contract. Host
differential evidence and freestanding ARM compilation are distinct from
kernel integration and Dolby playback.
