# Concrete decoder pipeline bindings

`decoder_bindings.c` binds the recovered process/output orchestration to
the actual header, level validation, spatial validation, payload, profile
validation, MEL detector and DM parser. The two exported entry points are
`rpu_decoder_process_buffer` and `rpu_decoder_get_output` with the
original eight-argument and six-argument shapes. Typed MEL and diagnostic
wrappers adapt the recovered C interfaces without incompatible function-pointer
casts. These wrappers and the operation tables are native integration plumbing;
the original oracle executes `rpu_decoder_process_buffer` and
`rpu_decoder_get_output` plus their real selected callees.

The target is P3i T4c `p3i-t4c-dovi.ko`, byte-identical to `s922x-j_dovi.ko`,
SHA-256 `5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Original process `.text+0x15e08` is 1,064 bytes, instruction hash
`f9df2e48f7bd7c0aafff14556b41fef750e7a22bbca8b97f3fbe3c87910296de`.
Original output `.text+0x16250` is 448 bytes, instruction hash
`8196b2aefcc5c442cefbceadbf56e4a1a1101589664ee47a9208a42dda920914`.
Their standalone recovery reports describe all control-flow branches.

## Differential validation

Run from `reconstruction/`:

```powershell
& .\same_kernel\am6b\build_decoder_bindings.ps1
& .\same_kernel\am6b\build_rpu_push_data.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_decoder_pipeline.py
```

The test passes **109 top-level calls**: 91 direct process calls, six actual
push-data calls and 12 actual get-output calls. There are 85 independent encoded
cases and two six-frame persistent streams. No parser or validator callback is
injected. Header and payload encoders describe input syntax; an independent
header-only execution obtains the decoded header used to encode payload bits.
Those preparation calls are reported separately and are excluded from matched
pipeline calls and pipeline coverage. Header/payload/DM bits are concatenated
without intermediate stop bits; DM alignment is relative to the whole packet.

Each matched call compares the complete 2 MiB caller arena, five distinct output
slots, normalized pointer fields, return value, outer guards, unchanged input
backing, and ordered diagnostic/event callbacks. Payload objects, mapping slots,
context counters/history, sequence cache, DM cache/live records and bit-reader
state persist between stream frames. The queued stream compares each real push
and each real output call separately, including an empty read after every frame.

The independent cases cover mapping methods 0..3 over one, two and four
partitions; NLQ methods 0/1/2/7; MEL detection enabled/disabled; chroma, horizontal,
vertical and NLQ explicit filters; DM modes 0/1/2/7; and both DM extension lists.
DM-present fixtures include levels 1/2/4/6/7/8/10/13, warnings about payload kind,
defaults without a cache, and a real fresh-cache/reuse transition. Optional-filter
fixtures request NLQ coefficient widths 10/31/32/63/255; the wide requests preserve
the original bit-reader behavior and can produce a later payload/profile error.
Exhaustive byte-width validation is in the separate payload-unit report.

Explicit stage failures compare original return/state/logging:

| Trigger | Result |
| --- | --- |
| Unsupported RPU type or coefficient type | `-3` |
| Level-zero partition grid exceeds its limit | `-4` |
| Context payload capacity is zero | `-2` |
| Invalid vertical or horizontal filter | `-6` |
| Mapping method prohibited by profile zero | `-5` |
| Accumulated pivot exceeds base-layer depth | `-13` |
| DM extension length 257 exceeds the block limit | `-2` |

The failures are repeated with severity 0, a disabled logger/event callback,
and unsigned severity `0xffffffff`. The two persistent streams use identical
six-frame inputs: fresh mapping plus DM, reuse mapping plus cached DM, fresh
mapping id 1, a coefficient-type error, successful fresh id 2, and reuse of id 1.
Both streams return `[905, 66, 449, -3, 259, 63]`. The queued stream confirms that
processing errors consume the queued slot, following frames remain usable, and
empty reads return `-20` before final input and `-21` after it.

## Executed original functions and modeled imports

The original ARM64 code actually executes these 25 functions:

- `rpu_decoder_push_data`, `rpu_decoder_get_output`, `rpu_decoder_process_buffer`
- `bitstream_init`, `read_u_v`, `read_ue_v`, `read_se_v`, `align_to_byte`
- `rpu_header_read`, `rpu_payload_read`, `assign_pivot_values`, `assign_nlq_pivot_values`
- `rpu_data_chroma_resampling_filter_1D_exp_read`, `rpu_data_spatial_resampling_filter_exp_read`
- `validate_level`, `validate_spatial_filter`, `validate_profile`, `detect_MEL`
- `dm_data_payload_read`, `ext_blocks_read`, `ext_block_payload_read.isra.0`
- `update_dm_extensions`, `remove_dm_extensions`, `store_md`, `rpu_error_code_2_str`

Only `_mcount`, `memcpy`, `memset`, `sprintf` and recording logger/event callbacks
are modeled. Memory models validate ranges; `sprintf` handles the original
bounded decimal diagnostics with one supplied byte-sized integer. No unmodeled
import or execution outside selected functions is permitted. Original ABS64
text-literal relocations, including literal pools outside function symbol sizes,
are applied using their ELF records; no instructions are replaced by parser stubs.
Selected `read_u_1`, `read_i_v` and `dump_rpu_buffer_info` do not execute in this
corpus, and are explicitly recorded with zero coverage.

The report `tests/decoder_pipeline_results.json` pins every selected symbol's
offset, size and instruction hash, the compiled concrete DLL, the push DLL,
input/test files, all **34** C source units in the concrete DLL build, and their
headers. It records visited/total instruction counts for each function. This
integrated corpus covers spatial validation 95/95, MEL detection 36/36, spatial
filter reading 63/63, bitstream initialization 54/54 and metadata storage 35/35.
Other functions have partial integrated coverage; their wider standalone
evidence is separate. This integration evidence does not promote partial
coverage to whole-function completeness.

## Storage and integration scope

The fixture supplies disjoint decoder/root/header/context/DM objects within the
arena, five payload buffers with four partitions each, a preceding readable
partition, four mapping slots, a padded input region and a ring buffer. The six
queued packets are each at most 4,096 bytes and use payload-only ingress with
one ring slot. Direct processing also exercises packets larger than a ring slot.
There is no Annex-B queue claim from this integrated corpus; the standalone
push-data suite covers that state machine.

Callbacks record events without changing parsing state. The original missing
varargs in the payload-capacity and NLQ prediction-range diagnostics remain
unspecified and are excluded from comparisons. Unbacked prediction targets,
arbitrary malformed counts, aliasing and mutating callbacks are outside scope.
Add `decoder_bindings` to the shared build list and this test after the payload
test. The aggregate freestanding build includes the binding source automatically.
This is recovered decoder-core integration, separate from the 48-byte T4c
host callback adapter, a complete loadable module, live-device validation and
Dolby playback.
