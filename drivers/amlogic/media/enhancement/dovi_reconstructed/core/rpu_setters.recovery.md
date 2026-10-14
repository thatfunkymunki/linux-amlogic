# decoder state setters

`rpu_decoder_flush`, `rpu_decoder_set_msg_log`, and
`register_dec_event_callback` reconstruct three small functions in the
pinned S922X-J module:

| Reference symbol | `.text` offset | Size | State change |
| --- | ---: | ---: | --- |
| `rpu_decoder_flush` | `0x15864` | 48 | write `last = 1` at decoder offset `0x28`; return 0 |
| `rpu_decoder_set_msg_log` | `0x15408` | 64 | write callback at `0x315c8`, severity at `0x315d0` |
| `register_dec_event_callback` | `0x15448` | 52 | write callback at `0x316d8` |

DWARF CU 16 identifies the 202,472-byte `rpu_decoder_ctx_s` and the
signatures. The message logger is `int(const char *, ...)`; the event
callback is `void(rpu_print_severity_t, uint32_t, char *)`. The severity
enum values range from 0 (`S_RPU_EMERGENCY`) to 8
(`S_RPU_NEVERPRINT`). `rpu_setter_view` contains only the accessed
fields plus opaque padding at their exact offsets.

Run from `reconstruction/same_kernel/am6b`:

```powershell
.\build_rpu_setters.ps1
..\..\.venv\Scripts\python.exe tests\test_rpu_setters.py
```

The test checks the ELF hash, all three function symbols, sizes, and sole
`_mcount` relocations. It substitutes each profiling call with an ARM64
NOP, emulates the reference code, and compares the entire 202,472-byte
context with the compiled C. It matched 64 flush, 60 message logger,
and 69 event callback calls. All 12, 16, and 13 reference instructions,
respectively, were visited. Results are in
`../tests/rpu_setters_results.json`.
