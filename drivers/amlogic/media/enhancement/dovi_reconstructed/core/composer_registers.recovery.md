# Composer register packing

The reconstruction implements `comp_cfg_2_comp_reg.constprop.6` from
`p3i-t4c-dovi.ko` (module SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`).
Its `.text` offset is `0x19e0`, length 1080 bytes, SHA-256
`159c5506988904f1bea4af3179c2aab50aee49ef79bcf77a7a65b70afbc97379`.

The optimized machine ABI has four arguments: composer pointer, register
pointer, width, height. The stale decompiler prototype lists an extra
`disable_el` argument; the instructions use `w2` and `w3` as width and height.
The C entry `comp_cfg_2_comp_reg` follows that actual four-argument ABI.
`tools/extract_composer_registers_abi.py` recovers the field offsets from the
same module's DWARF. The generated header and JSON evidence pin both structures.

The input composer occupies 1800 bytes and the output register object 900
bytes. Only output bytes 176 through 867 are written. The preceding DM fields
and trailing reserved bytes retain their original contents. Polynomial and
NLQ packing uses 32-bit variable shifts and the original masks; MMR uses
signed coefficient extension and 64-bit shifts before storing its 40-bit
result in two words. Resolution is the unmasked unsigned expression
`width | (height << 13)`. Byte access preserves unaligned operands. The two
objects must have distinct backing, as required by the original caller.

`tests/test_composer_registers.py` passes 4608 whole-object comparisons against
the original ARM64 instructions: 64 patterned objects, 2112 coefficient-shift
cases, 256 residual flag values, 64 resolution cases, 2048 random objects, and
64 concrete `comp_rpu_2_main_cfg` to register-packer pipelines. Both native and
reference operands are tested at byte alignments 0 through 7. All 270 packer
instructions are visited. The suite checks 36,864 guards, untouched input
objects, and untouched output prefixes and tails. Results and hashes are in
`tests/composer_registers_results.json`.

Only `_mcount` is modeled for the packer. The concrete composer conversion
retains its broader standalone differential suite. These comparisons establish
the tested host C behavior; they do not establish kernel-module or playback
equivalence.
