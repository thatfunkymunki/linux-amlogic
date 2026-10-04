# Concrete three-point tone curve generation

`tone_curve.c` recovers `CreateToneCurve3pxx` from the explicit Ghidra program
`p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The 832-byte original is ELF `.text+0xe0f0`, Ghidra `0x10e118` with `.text`
based at `0x100028`, and body SHA-256
`2be7719e595708ad9e88f592b61b77f1d956db9f3abb8b66a4181f8b156eca63`.
`evidence/tone_curve_generation_elf.json`, `tone_curve_generation_ghidra.json`,
`tone_curve_builder_instructions.json`, and `tone_curve_builder_callees.json`
retain its original body, ABI/layout, pseudocode, instruction and call evidence.

The thirteen actual arguments are source, display, three parameter pairs,
slope-over-rolloff, four short LUTs, an optional ratio output, signed-short
LUT size and signed-short clamp flag. The source/display/pair objects use
the DWARF-pinned 56/48/8-byte layouts in `tone_controls.h`. Source crush,
midpoint and clip are clamped and written back in the original order. Trim
slope, offset and power are clamped into locals. Mode 1 uses the clamped source
clip directly; other modes apply the absolute display midpoint-bias weighting
before parameter fitting. The builder then calls the real
`calc_mapped_params`, `calc_curve_params`, and `generate_tone_curve_lut`.
The latter receives a null 4K output. A null ratio output runs `tone_map_s`,
`saturation_map_s` and `saturation_map_i` in that order; a nonnull ratio stores
the source-to-target ratio and skips those three auxiliary outputs.

`tone_curve_concrete_ops()` exposes the same builder through the existing
typed operations interface. `create_tone_curve` and
`create_tone_curve_changed` select it when the operations pointer or its
`build` member is null. A supplied callback remains an explicit override for
the historical boundary audit. Equal first 160 cache bytes still skip the
builder before the operations pointer is inspected. Changed input is copied
to the used-control object after construction, retaining the final 8 bytes.
The wrapper's three-object original machine ABI and return bits are unchanged.

The new concrete closure spans five C units: `tone_curve`, `tone_parameters`,
`tone_curve_lut`, `tone_maps`, and `tone_math`. They add 16 public functions,
including the operations factory; `sdiv_sls` moves from `tone_controls` to
`tone_math` and is counted once. Exact literal `toneCurveXlogTable` and
`depq1024Lut` bytes are retained in their generated includes. Existing Taylor,
PQ, matrix/map, byte and diagnostic bodies remain concrete dependencies.
Component recovery notes document their instruction-level arithmetic and
separate finite differential evidence.

The formal finite LUT domain is positive signed-short size `L` from 1 through
4096. The original generator uses `step=floor(4096/L)` and writes
`N=ceil(4096/step)` tone-intensity entries, which can exceed `L`: sizes 3, 31,
511 and 513 write 4, 32, 512 and 586 entries respectively. Direct builder callers
must provide `N` shorts for tone intensity and, when ratio is null, `L` shorts
for each auxiliary LUT. A nonnull ratio needs one writable integer; skipped
auxiliary LUTs need no backing. The cached wrapper always uses size 512 and
four 512-entry LUTs inside the complete 4104-byte tone-map object. Sizes <=0 or
>4096 do not describe a finite safe original generator call; no invented
error return or early exit broadens that contract. The separate generator's
nonnull 4K interpolation capacity contract is documented in
`tone_curve_lut.recovery.md` and `tone_curve_lut.h`.

Direct tests require complete distinct source/display/output backing except
the explicitly exercised exact parameter-pair aliases. They also exercise
used-control/input aliases in the cached wrapper. They do not establish
arbitrary source/display overlap, partial-byte pair overlap, invalid pointer
behavior or out-of-range cached endpoint indices.

`build_tone_curve.ps1` builds strict MSVC `/Od` or `-Optimize` `/O2` DLLs with
`/W4 /WX /GS- /Zl /NOENTRY /NODEFAULTLIB`. Run from `reconstruction/`:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_tone_curve.py
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_tone_curve.py --optimized
```

Each direct report records 465 comparisons and 13,020 guard checks. Both
`tests/tone_curve_results.json` and `tone_curve_optimized_results.json`
report 208/208 original builder instructions, 135/135 changed-wrapper and 25/25
cache-wrapper instructions visited. Their transcript digest agrees. The
corpus covers modes 0/1/2, representative finite sizes 1/3/16/127/256/511/512/
1024, clamp and ratio choices, every source/display scalar at signed edges,
deterministic supported random inputs, parameter output aliases including
mode 65535, and real/default operations with cache aliases and suffix rules.
The selected original arithmetic executes as relocated ARM64 instructions;
only instrumentation and host byte-memory services are modeled. No numeric
helper or deep builder is modeled in this direct suite.

This direct suite visits 357/488 LUT-generator instructions because the
builder always requests null 4K output. The separate LUT suite exercises the
fixed interpolation path. The direct suite visits 18/19 `unorm_u` instructions;
the separate scalar suite records its complete visitation. These report
boundaries are retained rather than replaced with a blanket closure-coverage
claim. Instruction visitation and finite comparisons are not exhaustive
input or branch proof.

The current freestanding AArch64 aggregate reports 62 C source units,
201 public functions and 14 public data symbols with no unresolved symbols.
It is a relocatable source-validation object, not a kernel module. Concrete
DM-to-tone integration is exercised separately by `test_dm_tone_pipeline.py`;
its report records the whole guarded DM state and the original deep closure.
Both native optimization variants pass 610 comparisons and 8,540 guard
checks, exercising modes 0/1/2. `test_register_tone_pipeline.py` adds 24
stateful packed-register/LUT comparisons and 816 guards per variant. Its
native metadata callback only adapts the public API arguments; the numerical
DM and tone algorithms are concrete on both sides.
The older injected-builder boundary fixtures remain separate evidence.
The T4c 48-byte adapter, matching Kbuild module, live module identity and
AM6B+ Dolby playback remain distinct pending milestones.

`tests/test_tone_curve_arm64.py` additionally executes the actual optimized
ARM64 object against the original module. Its separate report records 36
builder/cache comparisons and 1,008 guard checks, with no compiled-side imports
or modeled calls. The original main builder reaches 208/208 instructions on
this finite corpus; narrower callee coverage is recorded in that report.
