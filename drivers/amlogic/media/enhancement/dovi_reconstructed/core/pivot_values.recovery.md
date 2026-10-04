# pivot value recovery

`assign_pivot_values` and `assign_nlq_pivot_values` reconstruct the
binary's `.text+0x10d30` and `.text+0x10e0c` functions. Ghidra's DWARF
signatures identify the root RPU object; disassembly pins the header field
offsets used here. The three color components accumulate 16-bit deltas from
header offsets `0x28`, `0x48`, and `0x68` into `0x88`, `0xa8`, and `0xc8`.
NLQ accumulates from `0xf2` into `0x112`. Both compare every written value
with `1 << (bit_depth & 31)` using the reference's signed 32-bit comparison
and return 0 or -13, preserving partial writes on error.

Unlike the separate 5.15 reconstruction, these functions have no explicit
trap for a pivot count above 16. The C follows the loop for counts 0 to
20 in the tested, padded header domain. The caller must supply writable
storage for every referenced offset; the original has no length check.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_pivot_values.ps1
..\..\.venv\Scripts\python.exe tests\test_pivot_values.py
```

The test pins the ELF hash, symbol offsets/sizes, and sole `_mcount`
relocation per function. It replaces profiling calls with NOPs and compares
return values plus the entire partially modified header for 10,320 cases,
including explicit 16-bit sum wraparound fixtures.
All 101 reference instructions were visited. No parser or module-wide
equivalence claim follows from this unit.
