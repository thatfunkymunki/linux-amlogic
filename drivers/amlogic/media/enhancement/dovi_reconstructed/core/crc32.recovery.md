# RPU CRC32 source recovery

The reference is `s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Its `rpu_crc32` is 92 bytes at `.text+0x103e8` (23 AArch64
instructions). Its 256-word `crc32_lut` is at `.rodata+0x1be10`, selected
by the `R_AARCH64_ABS64` relocation at `.text+0x10448`. The table SHA-256 is
`0053f121cdcaf9f510a940ba891fb2bccd41487953c5a4c0048c4a3ccdec782e`.
This table is byte-identical to the separately recovered 5.15 table; the
function and relocation positions were checked independently.

`rpu_crc32(seed, data, length)` complements the 32-bit seed, then for
each byte computes `table[byte ^ (crc >> 24)] ^ (crc << 8)` in 32-bit
arithmetic. It returns the accumulator without a final complement. A zero
length never reads `data` or the table and returns the complemented seed.
The high 32 bits of the incoming first AArch64 register are ignored. The
length is 64-bit.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_crc32.ps1
..\..\.venv\Scripts\python.exe tests\test_crc32.py
```

The standalone test pins the binary and table hashes, checks both AM6B+
relocations, compares the checked-in C table word-for-word with the binary,
and runs the instructions under Unicorn against the compiled C. It patches
only the `_mcount` profiling call to a NOP and applies the ABS64 table
relocation. The saved result contains 1,791 passing calls, all 23 function
instructions, and all 256 table indexes. Inputs and guards remain unchanged.
The test does not exercise kernel loading or the surrounding parser.

The `` name is a source/test interface. Linking it under the original
`rpu_crc32` name is an integration step for the module.
