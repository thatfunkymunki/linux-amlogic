# last-RPU output predicate

`rpu_decoder_last_rpu_output` reconstructs
`rpu_decoder_last_rpu_output` from the pinned S922X-J module at
`.text+0x16410` (68 bytes). DWARF CU 16 gives the signature
`int rpu_decoder_last_rpu_output(rpu_decoder_ctx_t *rdc)` and the decoder
context fields used by the function. Ghidra shows its caller is
`dv_md_parser_process`.

The ARM64 instructions load the output buffer index from context offset
`0x24`, the `int *rpu_buffer_level` pointer from `0x18`, the selected
level with a 4-byte scaled index, and `last` from `0x28` only when the
selected level is zero. The result is 1 precisely when that level is zero
and `last` is nonzero; otherwise it is 0. The source keeps the original
unchecked pointer/index behavior. Its `rpu_decoder_prefix` defines
only the fields required through offset `0x28`, without redefining the
202,472-byte complete decoder context.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_rpu_last_output.ps1
..\..\.venv\Scripts\python.exe tests\test_rpu_last_output.py
```

The test checks the module SHA-256, ELF function symbol/size, and the sole
`_mcount` relocation. It substitutes that profiling call with an ARM64 NOP,
then emulates the original function with Unicorn and compares it with the
compiled C for 4,880 cases. The cases cover all 16 valid array indices,
zero and nonzero signed levels, zero and nonzero `last` values, and
deterministic random arrays. All 17 reference instructions are visited.
Results are in `../tests/rpu_last_output_results.json`. This verifies the
predicate, not the surrounding decoder state machine.
