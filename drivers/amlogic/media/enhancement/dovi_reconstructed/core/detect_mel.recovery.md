# MEL predicate recovery

`detect_mel` reconstructs `detect_MEL` at `.text+0x16a1c` (144 bytes) in
the SHA-pinned S922X-J module. Ghidra disassembly shows a decoder pointer to
an RPU root pointer at offset 0, then a payload pointer at root offset 8.
The function reads three 28-byte nonlinear quantization parameter records at
payload offsets `0x6874`, `0x6890`, and `0x68ac`. It returns 1 only when each
record has zero in its checked fields except a 32-bit 1 at offset 4.
Bytes 2 and 3 are not checked by the reference; the C preserves that behavior.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_detect_mel.ps1
..\..\.venv\Scripts\python.exe tests\test_detect_mel.py
```

The focused test checks the original ELF hash, symbol location, and sole
`_mcount` relocation. It substitutes that profiling call with NOP and
compares 2,133 calls with the compiled C. All 36 reference instructions were
visited. The test covers every byte of every parameter record, plus
deterministic randomized changes. It does not establish whole-decoder or
hardware behavior.
