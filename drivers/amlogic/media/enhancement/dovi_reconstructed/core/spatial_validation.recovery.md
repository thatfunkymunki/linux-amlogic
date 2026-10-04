# spatial-filter validation

Reference: `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Original `validate_spatial_filter` is ELF `.text+0x10ec4`, 380 bytes,
Ghidra `00110eec`. Disassembly is retained in
`../evidence/spatial_st2094_ghidra.json`.

The validator checks three vertical and three horizontal filter selectors
against the original two 768-byte tables. A failed selector returns -6;
diagnostics require a nonnull logger and unsigned severity greater than 2.
Vertical mode is captured once, while horizontal mode is reloaded for each
component. Diagnostic arguments reload fields after the first logger call,
preserving the original observable behavior when the callback changes them.

`../tests/test_spatial_validation.py` compares 12,390 calls against the
actual instructions, visiting all 95 instructions. It checks returns,
ordered logger arguments, the complete header/root and guards, and all 1,536
table bytes. Cases exhaust selector bytes across all three components and
the eight backed table modes, severity gates, null logging, and selected
callback mutations. No validator body is injected into the reference.
`../tests/spatial_validation_results.json` pins function, table, source,
header and DLL hashes.

The independent `../tests/review_spatial_callbacks.py` audit adds six
calls that replace the root header pointer during logging and checks that
the original header snapshot and fresh diagnostic detail reads are preserved.

Supply a readable root with its header pointer at +0, a complete backed
header, and mode values 0..7. Larger table indices have the original unchecked
memory behavior and are outside this evidence. Invalid/null pointer fault
ordering, races, kernel registration and playback are not established.

Build with `../build_spatial_validation.ps1`, then run from reconstruction:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_spatial_validation.py
```
