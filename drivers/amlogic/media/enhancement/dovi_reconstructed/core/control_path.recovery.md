# Outer control callback recovery

`control_path.c` recovers `control_path` from the exact P3i T4c module.
The module SHA-256 is
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The original function is `.text+0x4c78`, 2964 bytes, with SHA-256
`5b6d96199ef1ec4174a244e424219517fae2344dcf4e00598279613460015929`.
Its 17 original arguments are retained after explicit storage and operations.
The historical callback output is 13468 bytes; the current T4c callback table
requires an independent runtime adapter.

The recovered flow uses concrete arena initialization, graphics parameter
population, VSVDB parsing, infoframe construction, packed metadata ingress and
filtering, HDMI metadata preparation and trim reduction. Persistent globals
are supplied by their owner in `control_path_storage`. `control_path_ops.commit`
accepts `commit_reg_with_ops` directly with its explicit `CommitMds` binding.
CPU identification, logging and optional memory diagnostics are explicit host
services. A null trim callback selects the concrete recovered trim chain.

`tools/extract_control_output.py` independently extracts the historical output
and extension layouts from pinned DWARF into `core/control_output_abi.h` and
`evidence/control_output_abi_dwarf.json`. Pointer fields remain opaque bytes.
The source preserves the original 1800-byte composer record reads, equality
termination, initialization persistence across destroy calls, partial output
on metadata errors, CPU-dependent clearing, 32-bit metadata length arithmetic,
raw metadata packing and retained extension bytes on short payload copies.

`tests/test_control_path.py` executes relocated original AArch64 instructions
in Unicorn against the native strict MSVC `/W4 /WX /NODEFAULTLIB` build. Its
local AAPCS64 caller supplies x0..x7 and nine 8-byte stack slots without changing
the shared reference harness. It compares all original persistent objects,
the complete output and guarded inputs, return values and callback transcripts.
Only actual pointer values are normalized, after checking their expected
referents. `tests/control_path_results.json` records the finite comparisons,
individual code pins, coverage, source/native hashes and transcript digest.

The outer suite injects a typed commit producer to isolate its orchestration.
It exercises every input/output mode, CPU clearing and repeated CPU reads,
graphics and output dimensions, persistent reset transitions, multi-frame
processing, callback-enabled metadata filtering, disabled composition and
truncated/invalid metadata. Large records execute concrete trim reduction and
the metadata overflow branch, checking retained bytes after size capping.

`tests/test_control_register_pipeline.py` separately composes the actual outer
callback and recovered commit body, configuration commit, composer/register
packing and LUT packing. Only `CommitMds` remains a deterministic modeled DM
producer. That fixture promotes pending configuration, commits metadata and
provides nonzero kernel arrays to test register and LUT handoffs; those values
are not a recovered tone-mapping algorithm. The full output and original
persistent objects are compared through fresh, multi-frame and persistent
sequences. Its report is `tests/control_register_pipeline_results.json`.

These are finite host differential checks. Invalid pointers, undersized
backing, overflowing fixed trim arrays and impossible valid-arena initialization
failures are outside their caller contracts. The binary's unchecked reads are
retained. No test establishes device module loading or Dolby playback.
