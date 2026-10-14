# Kernel maps and trim commit

The unit recovers eight functions from `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Ghidra program selection was explicit; its `.text` starts at `0x100028`.
No later-kernel routine supplied implementation evidence.

| Original | Section-relative offset | Bytes |
| --- | --- | --- |
| ClampLevel2 | 0x6ff0 | 204 |
| SigEnv2KsFrmFmt | 0x7164 | 328 |
| GetRange | 0x72ac | 208 |
| CommitKsOMap | 0x737c | 1312 |
| CommitUs.isra.4 | 0x79cc | 200 |
| CommitTrim.isra.11 | 0x7a94 | 936 |
| GetIptNormParam | 0x7e60 | 176 |
| CommitIptKsIMap | 0x7f10 | 224 |

`evidence/kernel_maps_abi.json` pins each symbol's full original byte hash
and DWARF sizes/offsets. `tools/extract_kernel_maps_abi.py` reproduces it
directly from the pinned module. The decompilation and instruction evidence
are `kernel_maps_ghidra.json` and `kernel_maps_instructions.json`.

`CommitUs.isra.4` has a stale DWARF/Ghidra prototype. Its actual ABI is a
32-bit chroma selector in w0, a pointer to the 32-bit bit-depth scalar in x1,
and a 62-byte upsampling object in x2. It preserves unspecified filter slots
and padding. Frame-format conversion likewise preserves chroma/color fields
that the original helper never writes.

Range arithmetic uses the original 32-bit masked shifts and 16-bit narrowing.
Normalization uses signed 64-bit division of 0xffffffff by a sign-extended
32-bit denominator, including the original zero-denominator result. Input
mapping installs the original unscaled identity and keeps untouched LUT and
field bytes. Output mapping retains separately captured bit depth, the
original range selection, unsigned offset shifts, and the late DM-kernel
pointer reload when target min/max is updated.

`kernel_maps_bindings.c` binds the typed `KernelMapOps` table to the real
`color_matrix.c` recovery. Explicit operation tables remain available for
ordered boundary audits. Required operations are supplied by the caller;
there is no fallback numerical model in the reconstructed implementation.
Byte copies call the existing `byte_utils.c`; ClampLevel2 has one public
implementation here and is also used by the tone-control unit.

Trim commit mutates the first trim's target-max key to the source maximum,
sorts indices with the original equal-key swapping, then selects/interpolates
six trim components. It preserves 32-bit intermediate wrapping, the exact
bias/adjustment indexes, backward-invertibility handling, clamp domains, and
the final 4096 offset in component six. Valid metadata supplies seven columns
and zero through sixteen additional trims. This recovery keeps the original
assertion's source path, `InterpolateTrim` name, line 0x80a and condition,
followed by a dump callback through `KernelTrimOps`. The assertion arm is
unreachable for a valid sorted trim array; it is preserved explicitly and
is not claimed as dynamically covered. Out-of-contract dimensions/counts
that overrun the original fixed local arrays are excluded.

Run from this directory:

```powershell
.\build_kernel_maps.ps1
```

Then from `reconstruction/`:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_kernel_maps.py
```

`-Optimize` builds with /O2; the default is /Od. Both require /W4 /WX and
link /NOENTRY /NODEFAULTLIB. The default includes concrete color bindings;
`-WithColor:$false` can build only the boundary-facing exports.

The differential suite compares the complete guarded arena after every
original ARM64 and native call. It checks exact ordered callback arguments,
input/output state, all guards, frame-format aliasing, aliased range outputs,
upsampling scalar/output aliases, embedded DM configuration and tone-control
aliases, late kernel-pointer redirection, enum combinations, edge bit widths,
trim interpolation/equal keys, signed wrapping and clamp boundaries. Its
4,257 cases include 1,092 concrete color integration cases in addition to
the typed boundary audit. `tests/kernel_maps_results.json` records source,
header, dependency, generator and DLL hashes, original hashes, build flags,
coverage, counts and the full arena/callback transcript digest. The /O2
snapshot is `tests/kernel_maps_optimized_results.json`.

These are freestanding native builds and original-instruction differential
comparisons. They do not establish a T4c Kbuild module, device load, or playback.
