# metadata parser reset and destroy wrappers

`dv_md_parser_reset` reconstructs `dv_md_parser_reset` at
`.text+0x106b8` (88 bytes), and `dv_md_parser_destroy` reconstructs
`dv_md_parser_destroy` at `.text+0x10518` (80 bytes). Both use the
112-byte `dv_md_parser_ctx_t` recovered from DWARF CU 14 in
`../dv_md_parser_abi_recovered.h`.

Reset returns `DV_MD_PARSER_INVALID_CONTEXT_HANDLE` (2) for a null
handle, or `DV_MD_PARSER_UNINITIALIZE` (1) when `f_init` at offset 0 is
zero. With a nonzero `f_init`, it calls `rpu_decoder_reset` if the
decoder pointer at offset `0x38` is nonnull, clears `ui64_sample` at
`0x68`, and returns `DV_MD_PARSER_OK` (0). It leaves `last_rc` and the
other parser fields unchanged. Destroy returns 2 for a null handle.
Otherwise it calls `rpu_decoder_cleanup` when a decoder is present,
clears that pointer, clears `f_init`, and returns 0. It does not erase
the rest of the parser context.

The source links the recovered `rpu_decoder_reset` and
`rpu_decoder_cleanup` implementations. Run from
`reconstruction/same_kernel/am6b`:

```powershell
.\build_md_parser_reset.ps1
..\..\.venv\Scripts\python.exe tests\test_md_parser_reset.py
```

The differential test checks the pinned ELF hash and all four function
symbols/sizes/relocations. It patches `_mcount` profiling calls to ARM64
NOPs and relocates wrapper calls to emulated copies of the original
decoder reset/cleanup functions. It then compares the return value,
entire 112-byte parser context, entire 202,472-byte decoder context,
pointed RPU context, and 64-entry level array with the compiled C.
It matched 55 reset cases, including null, uninitialized, missing
decoder, and initialized decoder states across payload modes and buffer
counts, plus 7 destroy cases. All 22, 20, 76, and 7 instructions in
the respective functions were visited. Results are in
`../tests/md_parser_reset_results.json`.
