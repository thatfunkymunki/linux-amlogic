# Control initialization and graphics defaults

Recovered from supplied `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
`tests/test_control_init.py` independently pins section offsets, sizes and
instruction hashes for eleven original bodies and six real DM defaults.
`evidence/control_init_instructions.json` records the main assembly listings;
`evidence/control_init_dependencies.json` records Ghidra decompilation.

The 20040-byte arena comprises the 4192-byte control context, 10928-byte kernel
state, 520-byte extension state and 4400-byte DM context, in that order. Pointer
fields and partial initialization preserve the original ABI. The generated
`control_abi_recovered.h` and `evidence/control_abi_dwarf.json` record exact
DWARF sizes and offsets. Regenerate with
`uv --cache-dir .cache/uv run --locked python tools/extract_control_abi.py`.

`init_cp_mmg`, `init_cp`, `destroy_cp`, mapping/enhancement setters, InitMmg,
InitDmKs, InitDm, RegMemReq and MemAssignCbFunFc execute recovered code. Internal
helpers use descriptive `control_*` names. The original graphics helper uses
global cli_param; `populate_graphics_param_body` takes that storage explicitly.
The six DM defaults execute actual recovered dependencies. Kernel diagnostic
imports are explicit logger/dump bindings, required on the assertion path.

Preserved details include ushort rowPitchNum, ushort LUT sizes, untouched
padding and fields, default alignment 4, signed memory request indices, and
reloading reqNum after diagnostic callbacks. Registration rejection at count 32
does not prevent kernel/control initialization. Caller backing must accommodate
the original unchecked memory index; there is no new range guard. The context
DM and extension pointers are reloaded after callbacks, including valid pointer
redirection by dump_stack. Graphics formats 0..3 set their original SDR/HDR and
RGB/YUV constants; other values retain the corresponding old fields. Width and
height remain untouched for every graphics format.

725 differential entry calls compare full arenas/operands, all returned values,
diagnostic bytes and order, and 11,600 guards. Tests include all graphics branches,
all memory slots 0..32, safely backed additional positive indices, zero/extreme
alignment, negative inDev, null kernel/config/manager early returns, callback
counter mutation and DM/extension pointer redirection. Known pointers are checked
against exact addresses before normalization. Every instruction in sixteen
selected original functions is covered. init_cp covers 47/49 instructions; its
two failure-return instructions cannot be reached with the valid concrete
initializer dependencies used by these fixtures. Original dependency paths are
checked; native paths are concrete but are not separately traced.

This is a recovered control support layer. Main control_path/commit_reg, the
remaining register/DM algorithms, T4c host adaptation and hardware playback are
separate pending work. No module or playback equivalence is claimed.
