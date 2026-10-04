# bitstream source recovery

The six bitstream functions in the pinned `s922x-j_dovi.ko` are reconstructed
in `bitstream.c`. Ghidra MCP provided their DWARF signatures and
decompilation; the 32-byte `bit_stream_t` field offsets were checked against
the AArch64 disassembly. The exact function locations and sizes are pinned in
`tests/test_bitstream.py`.

The implementation keeps the reference's unchecked buffer contract and its
32-bit wrapping and masked-shift behavior. It shares the parsing algorithm
with the separately reconstructed 5.15 module, but its own source, ELF
locations, relocations, and differential evidence belong to the binary.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_bitstream.ps1
..\..\.venv\Scripts\python.exe tests\test_bitstream.py
```

The test pins the module SHA-256, applies the two local `read_u_v` call
relocations, and replaces six `_mcount` profiling calls with NOPs. It compares
return values and all 32 state bytes after every call, normalizing input
pointers, and checks input and state guards. The deterministic corpus includes
all 256 last-byte values, fixed widths and refill boundaries, Exp-Golomb
prefixes through the 31-zero cap, and mixed reader sequences. The current
result is 278,512 matching calls and full instruction coverage in all six
functions. This does not establish whole-parser or kernel-module equivalence.

The `` names are standalone source/test interfaces. The original symbol
names can be supplied by aliases when the Kbuild module is integrated.
