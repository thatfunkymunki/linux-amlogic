# bitstream byte alignment

`align_to_byte` reconstructs the 168-byte `align_to_byte` function at
`.text+0x168e8` in `s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
It uses the 32-byte `bitstream` layout already defined in
`bitstream.h`; this is a separate source unit.

If the signed 32-bit deficit at state `+4` is positive, the function refills
from two bytes. It first rewinds the cursor at `+8` to the tail pointer at
`+16` when the cursor's numeric address exceeds `tail+4`. It merges the
big-endian pair into the 32-bit cache at `+0` using an AArch64-masked shift,
advances the cursor by two, and subtracts 16 from the deficit. It then shifts
the cache left until the deficit is divisible by eight. Other state fields
remain untouched. This preserves the original unchecked padded-input
contract and 32-bit wrap behavior.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_align_to_byte.ps1
..\..\.venv\Scripts\python.exe tests\test_align_to_byte.py
```

The test pins the symbol and sole `_mcount` relocation, substitutes a NOP
for that profiling call, and compares full 32-byte state against Unicorn
execution of the original instructions. It also checks input immutability
and the exact number of byte reads. The saved report records 2,560 matching
cases and visits all 42 instructions, including refill, cursor rewind, and
alignment paths. This is finite evidence for the helper, not a parser or
kernel-module integration test.
