# decoder output and processing orchestration

The source binary is `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
This unit is independently recovered from that binary's instructions and DWARF;
the historical filename `s922x-j_dovi.ko` has the same bytes. Ghidra program
`p3i-t4c-dovi.ko` loads `.text` at `00100028`.

| Original function | ELF `.text` offset | Ghidra address | Size | Instruction coverage |
| --- | --- | --- | --- | --- |
| `rpu_decoder_get_output` | `0x16250` | `00116278` | 448 | 112/112 |
| `rpu_decoder_process_buffer` | `0x15e08` | `00115e30` | 1064 | 266/266 |

`rpu_output.ghidra.json` preserves the Ghidra decompilation/disassembly.
`tests/rpu_output_results.json` records binary/function/source/DLL hashes,
comparison counts, coverage and explicit evidence scope.

## Recovered behavior

Output first publishes synthetic supplemental metadata when VUI, ST2086 and
ST2094 are all available. It consumes only `st2094_available`, returns 1, and
does not drain the queue. Otherwise it distinguishes need-more-data (`-20`)
from end-of-stream (`-21`) using the last flag, queue indices, payload-only
mode and the current buffer level. Each ring slot occupies exactly 4096 bytes.
Severity is unsigned: values greater than 6 call the dump operation before
processing. The output index is reloaded after that operation and after
processing; a processing failure still consumes a slot. The advance remains
`(num_rpu_buffers - 1) & (output_index + 1)`, including its non-power-of-two
quirk. It is not converted to modulo.

Processing assigns the empty payload handle before initializing the bitstream.
The header, level, spatial-filter, payload, profile, MEL and DM stages retain
the original call ordering and error exits. The partition capacity comparison
is signed 32-bit. Header/payload/DM consumed lengths accumulate modulo 2^32;
the final result is interpreted as signed 32-bit. Error logs use the logger
and unsigned severity captured at entry, while validation stages receive the
current logger/severity from decoder state.

Fresh payloads swap the current RPU ID's handle with the empty handle. Previous
payloads test availability using the **current** RPU ID, but select the stored
handle using `prev_vdr_rpu_id`. An unavailable previous payload gets identity
mapping/NLQ defaults in the first partition only. Mapping defaults touch each
of the three components and `num_pivots[component]-1` intervals. NLQ defaults
touch the first entry for each component when residuals are enabled. The
operation does not itself update the availability mask; parser dependencies
own any such mutation.

Published sizes are `num_x_partitions * num_y_partitions * 0x7038` for mapping
payloads and `0x10744` for DM payloads. Publication checks both the empty and
selected handles; allocation failure returns `-2` without updating output
slots. No implicit success/error stub is built into the C implementation.

## Call boundaries and integration limits

The public names include `_with_ops` deliberately. `rpu_process_ops` requires
seven explicit operations: header read, level validation, spatial validation,
payload read, profile validation, MEL detection and DM payload read.
`rpu_output_ops` requires processing and debug buffer-dump operations.
Independently recovered implementations may be supplied through small typed
adapters. Header read, level/profile validation and MEL detection already have
separate recovery units; this report does not claim recovery of the missing
payload readers or spatial validator. No call links into the original binary.

The bitstream initializer and error-string lookup are linked from recovered C
and are executed as original instructions in these tests. Tests replace the
seven operations with observable, configurable models on both sides. Thus
full instruction coverage establishes the outer control flow for supplied
dependency behavior, not encoded payload-parser correctness or Dolby playback.

The unchecked ABI requires readable padded buffers, valid queue/handle
indices, a complete writable decoder/root/header/payload arena and five
distinct writable output slots. Fresh RPU handle indices must remain inside
the caller's backing. Callback models test a previous-only ID of 31 to verify
the masked availability shift without indexing that ID's handle. An original
too-small partition diagnostic has a `%d` format with no actual vararg. C
preserves that call; tests capture the format without interpreting the
missing argument.

## Validation

Run from `reconstruction/`:

```powershell
./same_kernel/am6b/build_rpu_output.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_rpu_output.py
```

The native DLL builds with MSVC `/W4 /WX` and `/NODEFAULTLIB`. Tests compare
every byte of a persistent 917504-byte arena after each call, normalizing known
pointers, plus return values and ordered operation/log events. There are
732 output calls, 1,183 direct processing calls and 29 actual ingress calls,
covering queue occupancy,
supplemental availability combinations, failure exits, severity boundaries,
callback index/severity changes, mask-based advances, payload swaps and previous
identity defaults. Queue streams drain four frames, repeat after processing
errors, flush to end-of-stream and publish supplemental metadata before
returning to queued output. Sixteen successful/failed nested processing calls
are additional to the direct processing count.

Three actual push/drain streams split delimiters and prevention escapes into
single-byte, mixed-size and whole-stream chunks. A fourth exercises processing
failure followed by successful consumption of the next slot. These streams
execute the independently recovered `rpu_decoder_push_data` and real
`dump_rpu_buffer_info` against original push/dump instructions, with 29
push and 37 output calls. Their supplied padded RPU bodies are four or more
bytes; the payload-parser boundaries remain explicit models. The source and
original function hashes are included in the result JSON. This integration
evidence visits 160/217 ingress instructions and all 56 dump instructions;
the separate ingress unit's broader differential tests cover its remaining
branches.

All 112 output and 266 processing instructions are visited. These fixtures
also visit all 54 bitstream-initialization and 18 error-string instructions.
Reference `_mcount` is the only import modeled as a no-op. The actual-ingress
stream harness also models `memcpy` with exact bytes and verified mapped
ranges. Literal-pool pointers are relocated using their original ELF relocation
records; no instruction bytes are patched. The aggregate freestanding ARM64
result in `tests/core_arm64_build_results.json` includes this unit, and is
a separate validation milestone. No T4c module, hardware load or playback
result is claimed by this unit.
