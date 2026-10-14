# Color and matrix recovery

The source in `color_matrix.c` implements the concrete color/matrix closure used
by CommitMds and the kernel maps. It is independently recovered from
`p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Ghidra analysis used the explicit program `p3i-t4c-dovi.ko` with `.text` base
`0x100028`. Historical 5.15 implementations are not evidence for this unit.

`color_matrix.h` exports 26 plain functional APIs corresponding to 27 pinned
original function bodies. `GetRgb2LmsM33` and `GetRgb2LmsByDefM33` share the same
source implementation. Matrices use nine signed 16-bit elements in row-major
order; vectors use three signed 32-bit elements. The optimized original
`M33TimesM33AS.constprop.2` has two machine arguments, RGB-to-XYZ input and LMS
output. The XYZ-to-LMS operand, input scale, and intermediate shift are fixed
by its instructions. Its older five-argument DWARF/decompiler signature does
not describe the executable ABI.

The extractor pins every body, its literal-pool extent, and its recorded
relocations in `evidence/color_matrix_elf.json`. The exact 36-byte XYZ-to-LMS,
18-byte LMS-to-IPT, and 18-byte IPT-to-LMS tables are extracted from `.rodata`;
the selected XYZ-to-LMS table is the one at `0x167f0`, not the different table
with the same ELF symbol name at `0x50`. Seven RGB-to-XYZ matrices are immediate
operands in `GetRgb2XyzM33`; the extractor executes that pinned original body
to emit their exact integer contents into `color_matrix_tables.inc`.
Decompiler and selected instruction listings are saved separately in
`evidence/color_matrix_ghidra.json` and
`evidence/color_matrix_dependencies_ghidra.json`.

The reconstruction retains signed 32/64-bit wrap, masked variable shifts,
signed division with ARM64 zero-divisor and minimum-integer semantics, the
original inverse rounding, and the precision routine's signed comparison
at bit 63. Consequently, magnitudes above `2^62` yield precision 64.
Inverse singularity returns -1 without writing the output. A cross-talk value
of `0x55558000` wraps the inverse transform denominator to zero, and that
branch skips the white-point reads. Byte accesses preserve legal unaligned
objects, overlapping buffers, and reloads across different element widths.

The standalone harness compares the entire 4096-byte arena and scalar returns
against the actual relocated ARM64 instructions of all 27 bodies. It executes
all numeric dependencies directly. It covers all original instructions and
11,897 cases per build, including all supported and invalid color definitions,
masked bit-depth shifts, power boundaries through signed 64-bit extrema,
singular/in-place/overlapping inverses, all byte offsets for narrowing aliases,
unsigned YUV offsets, cross-talk and zero white points, overlapping white/matrix
storage, random and degenerate primaries, and random fixed-matrix products.
Both `/Od` and `/O2` native DLLs pass `/W4 /WX /NODEFAULTLIB` builds and yield
the same comparison digest:
`f575a16bbf021e8d11cb83698652504c755c93391107c2fa734df94812e3949a`.

Reproduce from `reconstruction/`:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tools/extract_color_matrix.py
./same_kernel/am6b/build_color_matrix.ps1 -Optimize
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_color_matrix.py
./same_kernel/am6b/build_color_matrix.ps1 -OutputName color_matrix_debug
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_color_matrix.py --dll-name color_matrix_debug.dll --report-name color_matrix_debug_results.json
./same_kernel/am6b/build_color_matrix_arm64.ps1
```

The reports retain function/table/source/native hashes. `printk` and
`dump_stack` for an invalid RGB definition are explicit diagnostic models;
the native API returns the original zero matrix without a kernel diagnostic
sink. This unit does not supply the tone-curve algorithm, Kbuild integration,
live host ABI validation, or playback evidence. It reuses no historical source
and needs no external semantic math helpers.
