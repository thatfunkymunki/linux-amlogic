# parser creation, initialization, and release

Reference: `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Ghidra calls explicitly selected that program. All offsets below are ELF
`.text` offsets, rather than Ghidra's relocated listing addresses.

| Original function | ELF offset | Bytes | Recovered function |
| --- | ---: | ---: | --- |
| `dv_md_parser_create` | `0x104a4` | 56 | `dv_md_parser_create` |
| `dv_md_parser_init` | `0x10568` | 308 | `dv_md_parser_init` |
| `metadata_parser_release` | `0x1af38` | 24 | `metadata_parser_release` |

The original `md_parser` is a 334,048-byte `.bss` singleton at offset
`0x266c0`. Create always returns it and clears exactly its first 112 bytes.
It does not allocate, free, or clear decoder storage. The recovered union
retains the parser type and eight-byte alignment. The singleton size is enough
for the module's level-0, payload-only configuration; broader flags require
caller-provided backing storage sized for decoder initialization, as described
in `rpu_init.recovery.md`.

Init returns 2 for a null handle without reading config. Otherwise it clears
`last_rc`, `f_init`, and `ui64_sample`, points the decoder at `handle+112`,
copies the callback/context, all seven flag words, and the logger, then calls
the actual recovered decoder logger setter with severity 5. A missing event
callback returns 9, writes `last_rc=9`, and logs when a logger is present.
The setter still ran, so the decoder logger/severity are installed in this
error case. The wrapper leaves both timestamps and parser padding untouched.
Config and parser storage must be disjoint and config must be readable when
the handle is nonnull; the original does not validate those preconditions.

With a callback, init invokes the actual recovered `rpu_decoder_init`.
The decoder's initial whole-struct clear erases the logger and severity that
the wrapper just installed. The parser's own logger remains. The original
decoder returns zero unconditionally, so the parser marks itself initialized
and returns zero. Reinitialization rebuilds decoder state in the same arena:
the wrapper resets `f_init` before testing whether it was already initialized.

The already-initialized branch and negative decoder-return branch are
preserved in the C, but unreachable with these original linked callees.
Their instruction offsets are recorded individually in the JSON results.
No speculative error translation replaces the original negative return.

`metadata_parser_release` contains only a profiling call and stack
prologue/epilogue. After removing profiling, it has no state, logging,
cleanup, or free effects. It is separate from `dv_md_parser_destroy`, which
clears the parser's decoder pointer and initialization byte.

## Validation

From the repository root:

```powershell
reconstruction\same_kernel\am6b\build_md_parser_init.ps1
uv --cache-dir reconstruction\.cache\uv run --no-project `
  --python reconstruction\.venv\Scripts\python.exe `
  --with unicorn --with pyelftools `
  python reconstruction\same_kernel\am6b\tests\test_md_parser_init.py
```

The MSVC C11 DLL links the recovered decoder initializer, logger setter,
memory queries, parser/decoder reset and destroy, and diagnostics. No decoder
implementation stubs participate. The emulator loads the pinned original
functions and relocates calls to those original ARM64 callees, with only
profiling removed and kernel `memset`/logging imported through hooks.
The test checks full backing arenas, both 64-byte guards, unchanged config,
callback/log pointers, timestamps, output bindings, singleton tails, and
captured logs. Native callback invocations are recorded and asserted absent.
DWARF evidence independently pins the 112-byte parser and 1920-byte app
context layouts and every member offset.

The suite matched 167 top-level calls: 16 create, 61 init, 40 reset, 40
destroy, 5 callback-body init, and 5 release. Cases include null handle,
missing/present callbacks and loggers, initial `f_init` sentinels, levels
0/1/2/3/15, payload-only and full input-buffer modes, and
init/reset/destroy/reinit sequences. Init uses caller arenas for the larger
configurations, including zero level-table rows whose fixed clear extends
past the memory-query size.

Create reached 14/14 original instructions, init 61/77, and release 6/6.
The 16 unreached init instructions are precisely the infeasible
already-initialized and negative decoder-return paths. The separate
callback-body scope and coverage are described in
`metadata_init_body.recovery.md`. Source, DLL, original code hashes,
call-graph entry counts, and DWARF layouts are saved in
`../tests/md_parser_init_results.json`.

Both new C units also compile with Clang for `aarch64-linux-gnu` using
freestanding C11, general registers only, and `-Wall -Wextra -Werror`.
The lane's existing aggregate builder discovers them automatically.

This is source and differential behavior recovery. It does not implement
metadata parsing, event handling, registration of the T4c 48-byte host ABI,
or device playback.
