# Reconstructed Dolby Vision module for P3i T4c

This is the editable scalar Dolby Vision core reconstructed for CoreELEC
P3i T4c on the Ugoos AM6B+. It contains 66 core C units, their local header
and table dependencies, and two kernel glue units. Kbuild produces
`dovi_reconstructed.ko` through the T4c host's six-slot, 48-byte callback table.

The kernel baseline is `pannal/linux-amlogic`, branch `amlogic-4.9-20`,
tag `T4c`, commit `3a4dcb1466bbf9a9d8b0d54ea8d2ab3ff7d656a5`
(Linux 4.9.269). The behavior reference is the supplied `p3i-t4c-dovi.ko`,
SHA-256 `5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Its historical 56-byte table differs from the T4c host, so `module.c` uses
the actual T4c declarations and checks the relevant layouts at compile time.
`host_control.c` translates host SDR BT.2020 value 6 to core value 4 and
preserves the host-owned format fields and trailing output words.

`source_manifest.json` records the original source paths, original SHA-256
hashes and published-file hashes. The C files and local dependencies retain
the validated reconstruction's C tokens; line endings and blank-line trailing
whitespace are normalized. `core_sources.json` is the explicit 66-unit list;
`core_objects.mk` is its checked-in Kbuild object list. The per-unit
`core/*.recovery.md` files describe recovered behavior and test exclusions.
The reference binary and generated build artifacts are not required source
inputs and are not included in this directory.

## Build

Use Linux with an AArch64 cross compiler. Prepare a complete matching kernel
build with generated headers and `Module.symvers`; `modules_prepare` alone
does not supply the symbol CRCs needed by `CONFIG_MODVERSIONS`.
This snapshot builds as an explicitly requested external module with `M=`.

```sh
source_path=/absolute/path/to/linux-amlogic
prepared_path=/absolute/path/to/completed-T4c-kernel-output
module_path="$source_path/drivers/amlogic/media/enhancement/dovi_reconstructed"

bash "$module_path/build.sh" "$module_path" "$source_path" "$prepared_path"
```

The build copies generated headers, host tools, configuration and symbol
versions into this module's ignored `_kernel/` output directory. It then
invokes Kbuild against that copy, retaining the completed kernel output.
The compiler prefix is `aarch64-linux-gnu-`; the checked build uses GCC
15.2.1. This module enables GNU11, `-Werror`, `-fno-builtin`, static frame
reporting and a 2048-byte frame warning. Two object-specific style warning
waivers retain previously validated expressions in `dm_commit.c` and
`kernel_maps.c`.

## Validation and runtime status

`validation.json` records a fresh build of this packaged copy against the
pinned T4c source and prepared output. Static validation checks AArch64 ELF,
vermagic, all imported symbol CRCs, the compiled 48-byte callback table and
its five function targets, and the absence of a removal entry point. Source
hash checks connect that result to this directory.

The reconstruction's host differential suites cover recovered parser,
control/register, display mapping, tone curve and saturation behavior using
the original AArch64 instructions as the oracle. Their full harness and
binary evidence remain in the reconstruction workspace; this commit is the
module source snapshot. Compilation and static ABI checks do not establish
successful device loading, FEL/MEL playback, seeking/reset, concurrency or
hardware timing. Those device milestones remain pending.

The T4c host retains the callback table without reader lifetime protection.
The module therefore has no `module_exit`: after successful registration,
reboot is required to replace it. The inherited runtime is a singleton and
requires callbacks not to overlap or reenter. The adapter does not establish
serialization across host control, parser, release and initialization calls.
Use the matching device kernel's generated inputs when evaluating a runtime
module; matching `uname -r` alone does not prove provider ABI compatibility.
