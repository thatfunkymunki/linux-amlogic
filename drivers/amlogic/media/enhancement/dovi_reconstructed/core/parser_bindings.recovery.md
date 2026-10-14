# Concrete parser dispatch bindings

`parser_bindings.c` supplies two concrete entry points:

- `dv_md_parser_process_bound(parser, au_flags, pts, buffer, size, last, flags)`
- `dv_md_parser_process_t35_bound(parser, au_flags, pts, buffer, size)`

They bind the existing injectable parser wrappers to the real recovered
`rpu_decoder_push_data`, `rpu_decoder_push_2094_payload`, concrete decoder
process/output, and `rpu_decoder_last_rpu_output`. The injectable APIs remain
available. Typed adapters avoid incompatible function-pointer casts and pointer
aliasing through `void **`; unsigned size bits are converted to the original
signed ingress/process parameters without an implementation-defined cast.
Adapters and constant operation tables are integration code. The ARM64 oracle
executes the original wrappers and every selected decoder dependency.

The reference is the supplied P3i T4c `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The test explicitly reads this file and asserts byte identity with the existing
reference reader's alias before using its parsed ELF data. Original parser
process is `.text+0x10958`, 660 bytes, instruction SHA-256
`1defdfebb06d8321dbc3c29ef6bfa6f27fa9c6c365b813b923d2ddff6f88eb25`.
Original T35 dispatch is `.text+0x107b8`, 408 bytes, instruction SHA-256
`c2550aa18c147d64b8aab462634155cdaece3e6498be1ac800cc3fcdad56f788`.

## Validation

Run from `reconstruction/`:

```powershell
& .\same_kernel\am6b\build_parser_bindings.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_parser_pipeline.py
```

The concrete DLL builds with MSVC C11 `/Od /W4 /WX`, without the C runtime or
default libraries. It links every core source, including the binding units.
The differential test passes **1,183 matched top-level calls**:

| Original entry point | Calls |
| --- | ---: |
| `dv_md_parser_process` | 1,120 |
| `dv_md_parser_process_user_data_registered_itu_t_t35` | 35 |
| `dv_md_parser_reset` | 23 |
| `dv_md_parser_process_vui` | 2 |
| `dv_md_parser_process_mastering_display_colour_volume` | 2 |
| `rpu_decoder_push_data` | 1 |

Each call compares the complete 2 MiB caller arena and outer guards, normalized
pointer fields, flags, return, and ordered diagnostic/parser callbacks. Parser
callbacks include all nine arguments, the complete timestamp, all 112 parser
bytes at callback time with pointer normalization, and SHA-256 of the complete
header/mapping/DM payload bytes delivered to the callback. This catches transient
differences before another frame overwrites the objects. The corpus has 50 parser
events, 13 decoder events and eight logger calls.

Both the RBSP and payload-only queued paths run persistent eight-frame streams:
fresh mapping and DM, mapping plus DM-cache reuse, another mapping id, a real
coefficient-type failure, successful decoding after the failure, reuse of the
previous id, MEL, and a four-partition map. Their return sequence is
`[0, 0, 0, -3, 0, 0, 0, 0]`. Timestamp updates include changed/equal timestamps,
missing PTS and absent AU timestamp flags. The two paths retain their different
final-sample timestamp behavior: the RBSP callback sees 1200 while the final
queued callback sees 1280. Reset runs with real decoder storage and permits a
fresh frame afterward; null parser, uninitialized parser and null decoder reset
branches are also compared.

Seven Annex-B streams carry the same three valid frames with actual byte ingress
and emulation-prevention bytes. Chunk sizes 1/2/3/7/17/64/4096 exercise prefix
splits, reader refill boundaries, ring advancement, multiple samples in one
dispatch and final input. They take 529/265/177/76/32/9/1 parser calls respectively,
and each yields three samples. A real `00 00 02` ingress failure returns `-1`;
reset and another Annex-B frame recover. Signed size-bit boundaries
`0x80000000`/`0xffffffff`, null input and zero size exercise real push-data
argument handling.

T35 sequences compare both provider codes 49/59, both accepted user identifiers,
levels 1/2/3/8/254, a no-refresh message, an ignored country and repeated PTS with
the parser logger enabled/disabled. Actual VUI and ST2086 wrappers establish the
two prerequisites for T35 publication; with them the five-message sequence yields
four samples, and without them it yields none. A null T35 input returns `-7`,
zero size succeeds, reset clears the sample counter, and T35 dispatch also drains
a malformed RPU queued through actual push-data, returning `-3` and the original
error callback. This specifically validates the shared RPU/T35 output path.

Integrated instruction coverage is complete for parser process (165/165), T35
dispatch (102/102), parser reset (22/22), last-output detection (17/17) and MEL
detection (36/36). Real ingress, output and payload readers have partial coverage
in this corpus; their wider standalone reports remain separate evidence.

`tests/parser_pipeline_results.json` pins original instruction hashes, compiled
DLL, build script, test/input helpers and every core source/header. It records
per-function instruction coverage and every modeled callback/import. Only
`_mcount`, `memcpy`, `memset`, bounded decimal `sprintf`, and recording callbacks
are modeled. No decoder or parser body is intercepted. Literal-pool pointers
are fixed through their recorded ELF relocations, including literals beyond
function symbol extents.

## Integration contract and limits

Callers may initialize through the existing `dv_md_parser_init` and use the two
bound entry points with their existing event callback. Reset/destroy and
VUI/ST2086 retain their existing concrete interfaces. Active dispatch requires
valid parser event, decoder and flag pointers. Input/refill padding, disjoint
payload storage, ring slots and levels retain the original recovered backing
contracts. RBSP mode is selected by parser flags; payload-only and Annex-B
ingress remain decoder flags. No allocation or implicit configuration is added.

The fixture uses disjoint objects, four mapping-history ids, up to four payload
partitions, one or four ring slots, padded input and recording callbacks that do
not mutate decoder state. This does not establish arbitrary malformed backing,
overlapping buffers or mutating callback behavior. Original missing diagnostic
varargs are excluded as in the decoder pipeline report.

Add `parser_bindings` to the shared DLL build list and `test_parser_pipeline.py`
after decoder pipeline tests. The freestanding aggregate build discovers the new
C unit automatically. The host metadata event body and serializer are separate
integration units. This work does not establish the 48-byte T4c host adapter,
matching-kernel module linkage, live-device compatibility or Dolby playback.
