# host metadata callback bodies

Reference: `p3i-t4c-dovi.ko`, byte-identical to `s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Ghidra program: `p3i-t4c-dovi.ko`; `.text` base `00100028`.

| Original | ELF offset | Executable size | C body |
| --- | ---: | ---: | --- |
| `md_parser_evt_handler_cb` | `0x1aa40` | 416 | `md_parser_evt_handler_body` |
| `metadata_parser_reset` | `0x1ad90` | 80 | `metadata_parser_reset_body` |
| `metadata_parser_process` | `0x1adf8` | 296 | `metadata_parser_process_body` |

The C supplies original globals as caller-owned `metadata_init_storage`
and `metadata_counters`. This exposes bodies for composition and testing;
it does not provide the final host callback signatures or global bindings.
`metadata_parser_release` is already recovered as an empty function in
`md_parser_init`; no replacement destruction behavior is added.

The composer conversion, composer/DM validation, DM serialization, and parser
process/reset functions are explicit ops. Their tests model those call
boundaries. The already recovered diagnostic functions execute as original
ARM64 instructions in the reference and compiled C in the candidate. Missing
payload/composer bodies are not counted as reconstructed by this unit.

The event body increments its 32-bit frame counter on all events, including
unknown/error events, and preserves wraparound. Sample events clear exactly
1800 bytes before conversion. Composer capacity is an unsigned comparison
against 6392; valid blocks occupy up to 8192 bytes. The composer block and
counter update occur before DM validation, so later errors retain partial
output. The serializer is offered 66626 bytes, but metadata output capacity
uses a signed 32-bit `1024 - length` comparison. No buffer rollback occurs.
The explicit arithmetic preserves the instruction's widths without signed
C overflow. Only called ops are required to be nonnull.

The process wrapper sets its pointers and zeroes composer/metadata counters
before scanning at most 63 trailing zero bytes. It skips the first five input
bytes and passes the remaining length with unsigned 32-bit wrap. Successful
calls publish output lengths, invoke reset, ignore reset's return, and return
the decoder flags. Failures preserve the caller's output-length values and
return -1. Explicit reset stores the parser status, ignores its flag argument,
and translates any nonzero status to -1. Log formats match the ELF strings.

Supported domain: stable 64-bit pointer layout, valid context/storage/counter
objects and called function pointers, readable input and payloads, correctly
sized writable output/scratch areas, and disjoint copy ranges. Buffer bases
may have guards, but accepted negative metadata offsets require real backing
storage. A serializer yielding a negative copy length or a length accepted
by wrapped arithmetic must supply the actual requested ranges; the tests do
not attempt enormous or invalid memcpy operations. Invalid pointers, races,
aliasing side effects, and kernel fault ordering are outside this evidence.

`tests/test_metadata_callbacks.py` compares return values, complete 164992-
byte storage, the 12-byte counters, all supplied buffers and guards, parser
state, and ordered dependency/log traces. Fixtures cover errors at every
stage, exact capacities, frame wrap, callback mutations, repeated sample/error/
unknown events, 0..64 trailing zero bytes, EOS byte values, preserved output
lengths on errors, and ignored post-success reset errors. All 104 event,
74 process, and 20 reset instructions are reached. Finite scoped evidence is
in `tests/metadata_callbacks_results.json`.

Reproduce from `reconstruction/`:

```powershell
& ./same_kernel/am6b/build_metadata_callbacks.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_metadata_callbacks.py
```
