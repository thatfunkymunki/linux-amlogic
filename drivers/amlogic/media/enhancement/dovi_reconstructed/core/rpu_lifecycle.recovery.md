# decoder reset and cleanup

`rpu_decoder_reset` reconstructs `rpu_decoder_reset` at
`.text+0x15734` (304 bytes) in the pinned S922X-J module.
`rpu_decoder_cleanup` reconstructs `rpu_decoder_cleanup` at
`.text+0x15718` (28 bytes). DWARF CU 16 identifies their shared parameter
as `rpu_decoder_ctx_t *`. The reset caller is `dv_md_parser_reset`.

The reset source uses a prefix/view of the 202,472-byte decoder context,
with padding for fields it does not access. Relevant DWARF offsets are:

| Context offset | Field |
| ---: | --- |
| `0x08` | `rpu_context` pointer |
| `0x18` | `rpu_buffer_level` pointer |
| `0x20`, `0x24` | input and output buffer indices |
| `0x28`, `0x30`, `0x38` | `last`, `payload_only`, buffer count |
| `0x78` | previous sequence header flag |
| `0x315d4` | RBSP parse state |
| `0x315d8`, `0x31658` | 16 payload buffer pointers, 15 handle pointers |
| `0x316d0`, `0x316e0` | empty handle pointer, MEL flag |

The pointed-to `rpu_context_s` has `max_prev_rpu_ids` at `0x678`.
In payload-only mode reset selects RBSP state 4, forces one buffer, clears
its level and MEL flag, and zeroes both buffer indices. It preserves
`last`, the previous sequence flag, and payload handles. In ordinary mode
it selects RBSP state 0, clears every buffer level, copies
`payload_buffer[0..max_prev_rpu_ids]` into corresponding handles, sets
the empty handle to the following payload pointer, clears `last` and the
previous sequence flag, sets the input index to buffer count minus one,
and clears MEL. The `0xffffffff` sentinel for `max_prev_rpu_ids` skips the
copy and selects `payload_buffer[0]`. The source retains the reference's
unchecked pointers and buffer count. Cleanup ignores its context and
returns zero.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_rpu_lifecycle.ps1
..\..\.venv\Scripts\python.exe tests\test_rpu_lifecycle.py
```

The differential test checks the ELF hash, function symbols and sizes,
and sole `_mcount` relocations. It substitutes those profiling calls with
ARM64 NOPs, emulates the original functions in Unicorn, and compares both
the return values and the full decoder/context/level memory with the
compiled C. It matched 60 reset states across payload modes, buffer
counts 0/1/2/64, and previous-RPU counts 0/1/2/14/sentinel; cleanup was
also compared with a null argument. All 76 reset and 7 cleanup
instructions were visited. Results are in
`../tests/rpu_lifecycle_results.json`.

`rpu_decoder_init` was inspected separately. It initializes the large
decoder memory arena and level-dependent tables, so its source recovery is
left for a separate component. The tested reset behavior assumes its
pointer fields and arrays have been initialized by a caller.
