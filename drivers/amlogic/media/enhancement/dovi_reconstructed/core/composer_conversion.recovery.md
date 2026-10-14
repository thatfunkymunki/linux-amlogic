# composer conversion

`comp_rpu_2_main_cfg` recovers the real `comp_rpu_2_main_cfg` body in
`p3i-t4c-dovi.ko`: ELF `.text+0x16454`, Ghidra `0011647c`, 912 bytes,
228 instructions. Reference SHA-256:
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The function bytes hash to
`20c0a6e56a88993f904d4495b02fcfcf831d42f49423c2dd0e868eac0c4eefc6`.
Its only relocation/call dependency is `_mcount` at `.text+0x16484`.
Ghidra's explicitly selected instruction and decompiler evidence is in
`../evidence/composer_ghidra.json`.

The function converts the first RPU payload partition to the sparse
0x708-byte main composer configuration. It copies bit depths and denominator,
mapping selectors, pivots, polynomial or chroma MMR coefficients, NLQ values,
and selected resampling parameters. Counts 0 and 1 become 2 for polynomial
mapping, with one polynomial segment copied. MMR mapping is handled only for
the two chroma channels. Other mapping selectors leave coefficient arrays
unchanged. Disabled resampling flags preserve all unselected fields rather
than clearing them. Unused coefficient slots and padding are also retained.

The existing 5.15 converter was study material for field offsets and shape.
The instruction body was checked separately. contains **no** pivot,
polynomial-order or MMR-order breakpoint guards. It loops over byte-sized
counts and orders without clamping, including polynomial `order+1` terms and
MMR `order*7` terms. Invalid values can overwrite nominal configuration arrays
and access beyond the nominal input records. The C uses byte storage and
preserves those unchecked operations. Its API therefore requires stable,
disjoint backing objects covering every accessed byte; it does not silently
turn invalid counts into a bounded conversion.

## Validation

From `reconstruction/`:

```powershell
& ./same_kernel/am6b/build_composer.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_composer_conversion.py
```

The MSVC C11 host build passes `/O2 /W4 /WX` without a default runtime library.
The source also participates in the lane's aggregate freestanding AArch64
build; consult `../tests/core_arm64_build_results.json` for its current
source/object hash and the linked object milestone.

The suite records **4,619 matching original-versus-C calls**, exercising
**228/228 original instructions**. It compares every byte of the header,
payload, output and both external guards; inputs remain unchanged. Cases
cover all 256 mapping, pivot-count, polynomial-order, MMR-order and filter-flag
values in the relevant channels, the complete nominal pivot-count matrix,
numerical patterns and independently randomized records.

The controlled caller backing is 0x600 header bytes, 0x10000 payload bytes and
0x4000 output bytes. **1,127 cases write beyond the nominal 0x708 configuration**,
and their complete resulting bytes match the original while the external
guards remain intact. This padding characterizes the original behavior;
it does not enlarge the actual host's composer ABI.

`../tests/composer_conversion_results.json` pins the module/function,
source/header/test/DLL hashes and records coverage, fixture sizes and counts.
All conversion instructions execute in the original oracle; only `_mcount`
is modeled. There are no injected payload/composer bodies in this comparison.
Aliased input/output storage is excluded. This is conversion evidence, not
host registration, kernel ABI, module loading or Dolby playback evidence.
