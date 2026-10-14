# decoder and parser memory queries

The reference is `s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
`rpu_decoder_mem_query` is 128 bytes at `.text+0x15380` and
`dv_md_parser_mem_query` is 48 bytes at `.text+0x104e8`. The 320-byte
`level_limits` table is at `.rodata+0x1c820` with SHA-256
`130a76d080558b7acc8bb62f9a7b77f4e804988d43bdc9a25337855ad3169b68`.
The checked-in C table matches all 80 original words.

Decoder config words at `+4` and `+12` are the level and `payload_only` flag.
For levels 0 through 15, the exact return is:

```text
units = uint32(level_limits[level][4] + 2)
units = uint32(units * level_limits[level][0])
units = uint32(units * level_limits[level][1])
base = payload_only == 0 ? 0x828f6 : 0x437fa
return uint64(units) * 0x7038 + base
```

The parser query passes `config+0x10` to the decoder query and adds `0x70`.
The original has no explicit out-of-range trap for level 16 or greater.
The source and differential test therefore claim only the table's 0..15
domain; invalid levels may read beyond the original table.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_memory_query.ps1
..\..\.venv\Scripts\python.exe tests\test_memory_query.py
```

`tests/memory_query_results.json` records 2,128 matching calls, including
every level, zero and nonzero flags, misaligned ordinary-memory configs, and
randomized unused fields. The Unicorn harness executes both original
functions, applying the `R_AARCH64_CALL26` wrapper call and ABS64 table
pointer. It replaces only `_mcount` profiling calls with NOPs. All 32 decoder
and 12 parser-wrapper instructions were visited. This establishes finite
differential evidence for size calculations, not allocation or parser behavior.
