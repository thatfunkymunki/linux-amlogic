# Concrete metadata pipeline

`metadata_bindings.c` connects the recovered metadata initializer and callback
bodies to the concrete parser bindings, composer conversion, composer/DM
validators, DM serializer and parser reset. The operation tables remain local
integration code; their dependencies execute recovered C without payload models.
The original injectable body helpers remain available for boundary tests.

The reference is the supplied `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Original callback recovery is recorded in `metadata_callbacks.recovery.md`
and `md_parser_init.recovery.md`. No new original function is claimed for these
four binding entries:

- `metadata_pipeline_init`
- `metadata_pipeline_process`
- `metadata_pipeline_reset`
- `metadata_pipeline_event`

## Storage and lifecycle

The caller supplies aligned, initially zeroed `struct metadata_pipeline` storage.
It contains the exact 164992-byte metadata storage first, counters, and a logger.
The event context remains the original app-context pointer. Both first-member
offsets are asserted so the bridge can recover the containing pipeline safely.
The parser/decoder retains the original 334048-byte singleton, so only one
pipeline may be active at a time. Initialization replaces its callback context;
separate app storage does not create independent parsers.

Initialization clears precisely the original app/payload regions and preserves
their padding and the counters. Frame count persists across init, process and
reset, and increments even for errors or unknown events. Logger installation
precedes init. The decoder initializer erases its own logger/severity/event
slots, as the reference does; the binding does not reinstall them. The app and
parser logger slots remain installed. `metadata_parser_release` stays the
already-recovered no-op.

The logger must be nonnull and stable and may record diagnostics without
mutating input/storage or recursively entering init/process/reset. Composer
output needs 8192 writable bytes and metadata output needs 1024, disjoint from
input/storage. Input requires actual readable backing, padding and a nonzero
payload byte for the original unchecked trailing-zero scans. No new input
guards or allocation behavior are introduced. Init leaves output pointers null;
bind them before a direct sample event or a T35 call using this event handler.

## Differential evidence

From `reconstruction/`:

```powershell
& ./same_kernel/am6b/build_metadata_bindings.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_metadata_pipeline.py
```

The host DLL builds all core units with MSVC C11 `/Od /W4 /WX`, without runtime
libraries. `tests/metadata_pipeline_results.json` records 57 matching top-level
calls: two initializations, 36 process calls, three explicit resets and 16
direct events. Five process calls emit nonzero composer and serialized metadata
outputs. Ordinary frame fixtures cover both output depths, DM modes 0/1/2,
extension records, end-of-stream values and trailing zero counts 0/1/31/62/63.
Actual header/coefficient/level errors leave caller length slots untouched;
successful calls after them retain the reference's error and reset semantics.

Direct events cover validation errors after composer output, accumulation,
composer capacity 6392/6393, metadata capacity 953/954 for the 71-byte scalar
sequence, full metadata buffers, unknown/error events and frame-counter wrap.
The exact fit produces 1024 metadata bytes; the first rejected value preserves
metadata count while composer output is already written. Reset with an
uninitialized parser takes the real error/logging path. Reinitialization
preserves counters and clears only the original storage regions.

After every call the test compares the complete metadata storage, complete
parser singleton, counters, guarded input/output/length buffers, return values
and ordered diagnostic logs. Only declared pointer slots are normalized,
including unaligned pointers in the decoder's root object. Input helpers,
compiled DLL, every core source/header and original function bytes are hashed.
Original callback, parser, decoder, composer, validator and serializer bodies
execute real instructions. Only `_mcount`, memory imports, bounded `sprintf`
and logger recording are modeled. No callback body is intercepted.

These finite comparisons cover the supplied synthetic stream/storage contract.
They do not establish arbitrary malformed backing, aliases, state-mutating
callbacks, multiple independent parser instances, a T4c host registration
adapter or hardware playback.
