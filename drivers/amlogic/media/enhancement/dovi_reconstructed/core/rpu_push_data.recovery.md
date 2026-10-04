# byte ingress recovery

This unit recovers `rpu_decoder_push_data` and its actual
`dump_rpu_buffer_info` helper from `p3i-t4c-dovi.ko`. The installed T4c reference
and `s922x-j_dovi.ko` are byte-identical, with SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.

| Original function | ELF `.text` offset | Ghidra address | Instruction bytes | SHA-256 |
| --- | --- | --- | --- | --- |
| `rpu_decoder_push_data` | `0x15a98` | `00115ac0` | 868 | `a1108f1e0f10db3436f14bab496564acc1e29a176f48d4bbd5ae89909792f602` |
| `dump_rpu_buffer_info` | `0x15278` | `001152a0` | 224 | `4f20e1c558dc435c65ed59c821852d93f6cfa05e31e789905ba25c10bdb36c92` |

The test checks symbol offsets, sizes and hashes against the pinned ELF, and
checks the 202,472-byte `rpu_decoder_ctx_s` DWARF layout, including the input
slot, level pointer, logger, unsigned severity comparison and RBSP state.
The C header exposes a prefix ending at `0x315d8`; callers supply the full
decoder arena when combining it with other recovered units.

The reference executes all original instructions in both functions. Only
`_mcount`, `memcpy` and the logger are modeled. `memcpy` validates the supplied
object ranges, the logger records ordered format/value events, and `_mcount`
does nothing. Calls to the diagnostic helper execute its original ARM64
instructions. The tests explicitly apply six `R_AARCH64_ABS64` relocations in
literal pools outside the functions' `st_size`: push at `0x15e00`, and dump at
`0x15358` through `0x15378`. The original seven-byte switch table at
`.rodata+0x1d2e0` is checked as `44 71 3b 33 2c 1c 4b` and remains executable
reference evidence, rather than a modeled state machine.

## Behavior preserved

Invalid decoder, negative size, or null positive-size input returns `-7`
before changing the decoder. A valid call with nonzero `last` sets `last=1`;
it neither flushes held zeros nor resets stream state.

| State | Incoming byte | Result |
| --- | --- | --- |
| Search / one start zero | zero | Advance to one / two start zeros |
| Two start zeros | `01` | Await `19` |
| Await `19` | `19` | Advance input by `(count-1) & (input+1)`; reject occupied slot with `-22`; otherwise enter payload |
| Payload | nonzero / zero | Append byte / hold one zero |
| One held zero | nonzero / zero | Append zero then byte / hold two zeros |
| Two held zeros | `03` | Append two zeros, discard emulation-prevention byte |
| Two held zeros | `01` | Await `19`, preserving the completed slot's level |
| Two held zeros | `00` | Enter the two-start-zero state, discarding held zeros |
| Two held zeros | `02` | Return `-1`, preserving state and level |
| Two held zeros | other | Append two zeros and the incoming byte |
| Undefined state | any | Preserve state and skip byte |

Start-search mismatches return to search without reconsidering the current
byte. The ring operation is a bit mask, including for non-power-of-two buffer
counts. A full-ring error retains the already advanced input index and the
state awaiting `19`.

Stream mode compares the current signed level with 4096 **after** each byte's
zero expansion and writes. Level 4096 is accepted; a following write can cross
into the next slot, and one byte can cause up to three writes before returning
`-22`. Padding and neighboring slots are compared in the tests. The source
preserves this ordering rather than changing it to a pre-write capacity check.

Payload-only mode trims trailing zero bytes, replaces the current slot, and
sets its level to the trimmed length. It performs no slot capacity check. Its
backwards scan has no lower bound: the tests include entirely zero nominal
buffers with an explicitly supplied nonzero prefix byte, which produces a
zero-length copy. If the scan continues farther backwards, the original can
underread and pass a negative length widened to `size_t` to `memcpy`. Such
invalid storage is outside the native comparison domain; the recovery does
not introduce a lower-bound guard. Input and destination must not overlap.

Severity is compared as an unsigned word: `7`, `8` and negative bit patterns
request the dump even after a zero-size success. Overflow and occupied-ring
errors request it independently of severity. The helper prints the exact
original formats and levels/indices; a null logger returns immediately.
The tested logger only records events and does not mutate decoder storage.

## Validation and integration

Run from `reconstruction/` using the locked environment:

```powershell
& .\same_kernel\am6b\build_rpu_push_data.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_rpu_push_data.py
```

`tests/rpu_push_data_results.json` records 4,871 matched calls over 194
stateful streams, with instruction coverage **217/217** for push and **56/56**
for dump. The test compares the return, entire decoder arena, all 64 payload
slots with leading/trailing guards, every level, unchanged input, and ordered
logger events after each call. It covers every byte in all seven defined
states, undefined states, null/negative arguments, unsigned severity edges,
non-power-of-two ring counts, occupied slots, post-write overflow, payload-only
trimming, every two-way split of crafted streams, one-byte fragments, random
fragmented streams, and caller consumption after full-ring errors.

An independent freestanding AArch64 compile with `-Wall -Wextra -Werror`
produces `build/arm64/rpu_push_data.o`, defining
`rpu_decoder_push_data` and `dump_rpu_buffer_info` with no undefined
symbols. The aggregate core builder automatically includes this new `.c` file.
Add `rpu_push_data` to the shared test runner's build list, and
`tests/test_rpu_push_data.py` to its test list. The source has no dependency
on a libc or kernel import; payload copies use an explicit loop.

## Remaining processing dependencies

`rpu_decoder_process_buffer` begins at `.text+0x15e08` (`00115e30` in Ghidra)
and has 1,064 instruction bytes. Inspection confirms that it orchestrates the
bit reader, header parser, level/spatial/profile validation, RPU payload parser,
MEL detection and DM payload parser, then swaps payload handles and publishes
output pointers. Its previous-payload fallback also initializes identity
mapping and NLQ fields. This unit implements none of those processing/output
paths. The evidence establishes byte ingress and diagnostic behavior for the
specified storage domain; it does not establish whole-parser equivalence,
kernel-host ABI compatibility, module loading, or Dolby playback.
