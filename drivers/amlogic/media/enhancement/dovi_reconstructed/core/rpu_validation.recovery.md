# RPU level and profile validation

Reference: `s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
This unit recovers `validate_level` and `validate_profile` from the AM6B+
`rpu_common.c` path. It is separate from the 5.15 STB 2.6 implementation.

| Item | ELF location | Size | SHA-256 |
| --- | --- | ---: | --- |
| `validate_level` | `.text+0x11060` | 416 | `944ee663ac3c63b4f29574b0717dd72febe9fe158dee46909af4feb38b2cd2dc` |
| `validate_profile` | `.text+0x11238` | 500 | `a7da3f6450e3cf1aa9363399bdc5a67f5fc455b699800c43fbbfbf28638a1c58` |
| `level_limits` | `.rodata+0x1c820` | 320 | `130a76d080558b7acc8bb62f9a7b77f4e804988d43bdc9a25337855ad3169b68` |
| `profile_limits_fixed` | `.rodata+0x1c960` | 2432 | `300646e0d6a5d1edb302bf0543cb547847d1c9d88b6df5c1bcf232d02d1d43f4` |

The tables each contain 16 rows, at 20 and 152 bytes per row. Their bytes
also match the corresponding tables in the 5.15 binary, but the ELF is
the pinned reference for this unit. The source defines its own table
objects, and the test checks the compiled objects byte for byte against AM6B+
`.rodata`.

`validate_level` reads the header pointer at root offset `0`, then checks
header level at `+5`. For a table row it checks Y partitions (`+0x134`),
X partitions (`+0x133`), three pivot counts (`+0x25` to `+0x27`), NLQ pivots
(`+0xf1`), current RPU ID (`+0x1b`), and previous RPU ID (`+0x1a`), in that
order. It returns `0` on success and `-4` on failure. A nonnull logger
receives the original format string and numeric arguments when the unsigned
severity exceeds `2`.

`validate_profile` copies the selected 152-byte profile row in the original.
The source reads the equivalent immutable table row. For each partition and
component it requires at least two pivots and checks each mapping method
against the row's sentinel-terminated allowed list. It then checks mapping
color space (`+0x1c`), chroma format (`+0x1d`), and explicit resampling flags
(`+0x13` and `+8`). A partition's payload has stride `0x7038`; its first
45 bytes hold the three 15-byte mapping method arrays. The function returns
`-2` for too few pivots, `-5` for a disallowed profile setting, and `0` on
success. Logging uses the same unsigned severity threshold as level
validation.

The reference functions store pointers and diagnostic strings in literal
pools immediately after their ELF function symbol ranges. The differential
test applies their recorded `R_AARCH64_ABS64` relocations to private code
memory before execution. It models `_mcount` as instrumentation and models
the original `memcpy` of the profile row with a checked memory copy. Input
objects and compiled tables are checked for mutation.

Run from the project root with the reconstruction virtual environment:

```text
reconstruction/.venv/Scripts/python.exe reconstruction/same_kernel/am6b/core/test_rpu_validation.py
```

The Windows Visual C run passed 622 level and 462 profile differential calls.
Reference instruction coverage was 104/104 for level and 125/125 for profile.
Cases include table rows 0–15, field thresholds, partition grids, mapping
values, pivot boundaries, logger nullability, severity boundaries (including
the unsigned high half), selected noninitial partitions, and deterministic
random combinations. These are finite comparisons, not universal proof.

The comparison domain uses table indices 0–15 and stable, readable ordinary
memory: a root with header and payload pointers, a 310-byte header, and
enough 0x7038-byte payload records for the selected grid. The public enums
name levels 0–2 and profiles 0–5. Out-of-range indices 16–255 can read
beyond the original tables or trap; the source returns `-4` or `-5` instead
and does not claim parity for them. Pathological pivot counts that read
beyond the 15-byte mapping arrays, invalid pointers, concurrent mutation,
and device-memory access are also outside the tested domain. The unit has
not been linked into an kernel module or exercised on AM6B hardware.
