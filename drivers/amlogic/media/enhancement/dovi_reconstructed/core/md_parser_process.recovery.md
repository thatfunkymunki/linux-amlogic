# metadata parser dispatch

`md_parser_process.c` recovers two parser orchestration functions from
the P3i T4c playback reference, with an explicit decoder operations table.

| Original symbol | ELF `.text` offset | Ghidra address | Size |
| --- | ---: | ---: | ---: |
| `dv_md_parser_process` | `0x10958` | `00110980` | 660 bytes |
| `dv_md_parser_process_user_data_registered_itu_t_t35` | `0x107b8` | `001107e0` | 408 bytes |

Reference SHA-256:
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The supplied `p3i-t4c-dovi.ko` and historical `s922x-j_dovi.ko` are byte-identical.
The tests retain the existing SHA-pinned loader; the static Ghidra evidence
targets `p3i-t4c-dovi.ko` explicitly. Addresses use `.text` base `00100028`.
Instruction listings are saved in
`../evidence/md_parser_process_ghidra.json`.

## Recovered behavior

Both wrappers return 2 for a null parser and 1 for an uninitialized parser.
Active paths preserve the original unchecked decoder and event callback pointers.
The process wrapper also requires a flags pointer, and both body helpers require
the relevant operations-table functions. The operations table is an explicit
reconstruction interface, not part of the original parser ABI.

`dv_md_parser_process` updates the timestamp values when AU flag bit 3 is set,
the incoming timestamp is present, and its value differs from `curr_pts`.
Only the 64-bit values move; the two timescales and timestamp padding are retained.
The branch at parser offset `0x2c` selects `flags.input_is_RBSP`, **not**
`flags.payload_only` at `0x28`.

For RBSP input the wrapper processes one buffer. A positive decoder result
generates a sample callback and returns 0. Every nonpositive result is stored
in `last_rc` and generates an error callback, whose status argument is zero.
The positive result's output pointers must have been written by the decoder;
the original does not initialize these local pointers in this branch.

For stream input the wrapper pushes data and drains output until a nonpositive
result. A push error updates `last_rc` without an error callback. Each positive
output runs `last_rpu_output` before generating a sample callback; a nonzero
result moves the current timestamp value into the callback timestamp.
Drain results -21 and -20 normalize to 0 and preserve `last_rc`. Other terminal
results, including 0, update `last_rc` and generate an error callback.
Pointer output slots are cleared before each drain call, but the two integer
size slots retain their values across calls. Finally the current decoder's MEL
byte at `0x316e0` sets or clears only flags bit 0, including after push errors.

The T35 wrapper increments the unsigned 64-bit sample count before pushing the
2094 payload. With valid PTS it updates the timestamp value on the first sample
or when the value changes. Consecutive equal values log the original fatal-error
message if a logger is bound; the wrapper reloads the sample count after that
callback and increments the reloaded value. Overflow wraps at 64 bits.
It drains outputs with the same -21/-20 terminal normalization and error event
rules, without consulting `last_rpu_output` or writing process flags.

Successful wrapper calls preserve `last_rc` unless a bound callback changes it.
Callbacks may mutate parser timestamps, diagnostic state and flags; their
modifications are retained according to the original instruction order.

## Differential validation

From `reconstruction/`:

```powershell
& ./same_kernel/am6b/build_md_parser_process.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_md_parser_process.py
```

The MSVC build uses C11, `/O2 /W4 /WX`, and no default runtime library.
The aggregate freestanding AArch64 build passed with this unit included,
using `-Wall -Wextra -Werror`. The linked partial object has no undefined
symbols; source/object hashes and its public definitions are recorded in
`../tests/core_arm64_build_results.json`. This is an object-build milestone,
not a kernel-module load or playback result.

The test records **1,302 matching wrapper calls**: 654 process and 648 T35.
It reaches **165/165** and **102/102** original instructions respectively.
Fixtures cover null/uninitialized parsers, absent/changing/duplicate timestamps,
signed decoder statuses and sizes, null payload outputs, positive output loops,
retained integer sizes, input mode selection, sample-count wrap, MEL flags,
and callback mutation of timestamps, flags, `last_rc` and sample count.
The 32 mixed sequences of 32 calls retain all state between calls.

Comparisons include the return value, all 112 parser bytes with address
normalization, all 202472 decoder bytes plus 32 guards, input/timestamp/flags
and payload-object bytes, and the ordered decoder/event/logger calls with
parser snapshots and exact callback arguments. Result evidence, function and
source hashes, relocations, coverage and call counts are in
`../tests/md_parser_process_results.json`.

**Dependency limit:** the original wrapper instructions execute in Unicorn;
push_data, push_2094_payload, get_output, process_buffer and last_rpu_output
are intercepted at their original entry addresses and injected with controlled
state changes and return values. The same dependency contract is bound to the
C operations table. `_mcount`, parser events and the message logger are also
modeled explicitly. No original decoder/payload body is executed by this test.
The evidence establishes parser dispatch semantics and callback ordering. It
does not establish payload decoding, playable Dolby output, a bound host
callback or a loadable kernel module.
