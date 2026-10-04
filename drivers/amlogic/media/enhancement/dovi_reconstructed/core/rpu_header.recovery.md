# RPU header parser

`rpu_header.c` reconstructs `rpu_header_read` from the supplied
`s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The function is 1,764 bytes / 441 AArch64 instructions at `.text+0x1a2c4`.
The source uses the existing `read_u_v` and `read_ue_v` functions.
The decoder, data-root, 310-byte header, and 17-byte sequence-header
offsets come from the module's DWARF and are pinned in
`../rpu_header_dwarf_layout.json`. The source is a behavioral rewrite; it
does not claim recovery of the vendor's original source text.

The parser retains sparse writes, cached sequence values, mapping ID reuse,
32-bit bit-count wrap, byte narrowing, and the fallback mapping fields.
Unlike the later 5.15 module, the function has no seventeenth-pivot
trap. The source consequently performs the seventeenth write; both ordinary
and residual pivot cases are included in the differential corpus.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_rpu_header.ps1
..\..\.venv\Scripts\python.exe tests\test_rpu_header.py
```

The test pins the reference hash, the header symbol boundary, 45 function
call relocations, and four relocated string pointers. It executes the AM6B+
header and bitstream reader instructions under Unicorn, replacing only
`_mcount` profiling calls with NOPs. `sprintf`, logging, and event callbacks
are modeled at their call boundaries and their outputs are compared with
callbacks from the compiled C. Decoder, data-root, and full header bytes are
compared after normalizing valid pointers; input buffers are checked for
mutation.

`tests/rpu_header_results.json` records 590 passing fixtures and 440 of
441 header instructions visited. The single unvisited instruction is the
branch at function offset `0x5d8`: it follows a test for nonzero format
bits inside a path already selected by those bits being zero. The corpus
covers fresh and cached sequence headers, error returns, unavailable IDs,
mapping and filter variants, pivot values, and callback thresholds. These
are finite comparisons; full multi-frame parser integration, arbitrary
unchecked pivot counts, kernel loading, and hardware playback remain to be
validated.
