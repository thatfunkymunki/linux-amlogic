# Tone curve LUT generation

This unit is independently recovered from `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Ghidra analysis explicitly selects that program; its `.text` base is
`0x100028`. The code and table are pinned in
`evidence/tone_curve_lut_elf.json`. `tools/extract_tone_curve_lut.py` checks
the supplied module hash and extracts the exact little-endian table into
`core/tone_curve_lut_table.inc`.

| Original | .text offset | Ghidra address | Bytes | API |
| --- | --- | --- | ---: | --- |
| GenerateToneCurveLut | 0xf748 | 0x10f770 | 1952 | generate_tone_curve_lut |
| fxPOW2TaylorInt | 0xf6c8 | 0x10f6f0 | 128 | fx_pow2_taylor_int |

The 4096-entry `toneCurveXlogTable` is a literal table with SHA-256
`5b83f408526d1467eb7bc519449c7b82aefb5fc6fdce0962214d822437efb1ea`.
The original function/dependency hashes and table bytes are retained in
the ELF evidence. `evidence/tone_curve_lut_ghidra.json` records the helper
decompilation and both assembly listings; the generator pseudocode is in
`evidence/tone_curve_generation_ghidra.json`. The display and parameter
pair layouts use the independently DWARF-pinned structs from
`tone_controls.h` and `evidence/tone_controls_elf.json`.

The generator has fourteen actual arguments. The optimized body discards
the historical `xMin` and `xMax` arguments and samples the literal table.
Display mode 0 evaluates the forward rational curve, applies rolloff and
trim slope/offset/power, rounds and enforces an unsigned 16-bit monotonic
output. Mode 1 reverses the trims and rational curve using the real
reciprocal and Taylor helpers, then rounds and enforces the same monotonic
output. Other modes store stepped input values directly. Clamping uses
the signed display bounds in their original order, including reversed
bounds. Negative clamp flags also enable clamping.

`fxPOW2TaylorInt` returns a signed 64-bit value: assembly uses `smull` on
the Taylor result and a signed 32-bit power-of-two factor. Inputs above
`0x4000000` split the exponent using the original fixed 26-bit boundary,
even when a different input scale is passed. Factor shifts mask to five
bits and can create a negative factor at bit 31. There is no floating
point substitution. W/X register wrapping, masked shifts, signed divide
by zero, saturation and short narrowing are explicit in the recovered
implementation. Existing real `trim_luminance.c` and `tone_math.c` supply
the numeric dependencies; no numeric helper is stubbed in the tests.

For a positive signed-short LUT size `L` from 1 through 4096, the sample
step is `floor(4096/L)` and the number of samples written is
`N=ceil(4096/step)`. In particular, size 3 writes 4 samples, size 31 writes
32, size 511 writes 512, and size 513 writes 586. With a null 4K output,
the work buffer needs `N` shorts. With a nonnull 4K output, samples start
at work index 1; the work buffer needs `max(515,N+1)` shorts, and the 4K
output needs 4096 shorts. The generator replicates the first sample into
work index 0 and index 512 into indices 513 and 514. The subsequent fixed
cubic interpolation reads indices 0 through 514 and writes 512 groups
of eight shorts. The intended 512 samples occur for step 8, corresponding
to sizes 456 through 512. Other steps retain the fixed interpolation,
including untouched work cells when fewer than 512 samples were written.
The interpolation uses seven exact coefficient sets and 32-bit wrapped
products/sums, then clamps to the signed current sample and the final
unsigned monotonic value. Passthrough modes keep that interpolation
ceiling at zero, as the original does.

Zero and sizes with magnitude above 4096 give step zero, so the original
loop has no index-based exit and keeps advancing the output pointer.
Negative sizes from -4096 through -1 give a negative step: curve modes
attempt table access before its first element on the second iteration;
passthrough modes can reach an exit only after 32-bit index wrap, requiring
`floor(2147483648/abs(step))+1` output samples. These are unsupported LUT
inputs and are not passed to native code in the differential suite. No
new early return or error value is invented for them.

Build and validate from `reconstruction/`:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tools/extract_tone_curve_lut.py
powershell -ExecutionPolicy Bypass -File same_kernel/am6b/build_tone_curve_lut.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_tone_curve_lut.py
```

The native DLLs use strict MSVC `/W4 /WX`, `/Od` and `/O2`, `/GS- /Zl`,
and `/NOENTRY /NODEFAULTLIB`. The reference executes the exact selected,
ELF-relocated AArch64 bodies and literal relocation records after
`st_size`. Only `_mcount` is modeled as a no-op; all numeric dependencies
execute their original instructions. Fourteen-argument calls retain
the eight-register/six-stack-slot ABI.

The finite differential suite covers full-width signed power inputs,
shift-scale wrapping, exponent factor boundaries, forward/backward/
passthrough modes, representative sample counts, zero reciprocals,
trim wrapping, full-width parameter norms, reversed/signed clamp bounds,
515-to-4096 coefficient evaluation and interpolation overflow. Twenty-four
parameter tuples are first generated by the real original and recovered
`CalcCurveParams`, compared as whole guarded objects, and then passed
through both generators. Whole arena comparisons retain guards and
unrelated input bytes. Reports are saved as
`tests/tone_curve_lut_results.json` and preserved provenance-named copies.
The report pins every recursively included source/header/table from the four
native compilation units, the test/builder/extractor and analysis evidence,
and both tested DLL hashes. It records 1903 cases per variant and 3806 native
comparisons, with all 488 generator and 32 integer-helper instructions visited.
These are finite component comparisons; device playback and matching
kernel module validation remain separate milestones.
