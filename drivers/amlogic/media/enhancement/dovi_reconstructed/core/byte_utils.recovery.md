# byte utility recovery

Three helpers from the module's `CdmUtilFxp.c` compile unit are in
`byte_utils.c`: `MemCpyByte`, `MemEqByte`, and `abs_int32`, exposed here
with `` names. Ghidra MCP supplied the decompilation and disassembly;
the test independently pins the ELF symbols and `_mcount` relocations.

Copying proceeds forward one byte at a time, including for overlapping
ranges. Comparison returns 1 for zero length or all equal bytes. Absolute
value saturates `INT32_MIN` to `INT32_MAX`. Valid readable/writable pointers
and nonnegative copy/compare lengths are the tested contract.

From `reconstruction/same_kernel/am6b`, run:

```powershell
.\build_byte_utils.ps1
..\..\.venv\Scripts\python.exe tests\test_byte_utils.py
```

The differential test passed 12,680 calls and visited all 72 instructions
across the three functions. It substitutes only the three profiling
`_mcount` calls with NOPs. This is a standalone source unit; Kbuild and
hardware integration remain separate work.
