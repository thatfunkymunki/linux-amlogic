# Concrete control getters and HDMI bindings

The getter bodies were recovered independently from Ghidra program
`p3i-t4c-dovi.ko` and pinned against module SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Their decompilations and assembly are saved in
`evidence/control_getters_ghidra.json`. No implementation from another kernel
reference was used.

| Body | `.text` offset | Instruction bytes | SHA-256 |
|---|---:|---:|---|
| `GetDmCfg` | `0x6074` | 116 | `fcb0c2e304cda5e17c81e4d905aa45f5a2b52d8371aa10053682bfeecac2e66e` |
| `GetStbCc` | `0xa2d0` | 276 | `829ff9d1c11d7a5df40df6f8a8615722c0bf1dcf61a72ed6a985e560687a57d0` |
| `MemCpyByte` dependency | `0xad84` | 92 | `a1efba460790cd979c29ba5cc1b84a3510c51c38df7220df96ef003f12220063` |

`control_get_dm_cfg` first rejects a null handle or combined zero pending/active
validity flags. It chooses `pDmCfgToBe` when the pending flag is any nonzero
32-bit value, otherwise `pDmCfgAct`. A null selected pointer fails even when
the other source exists. Success copies exactly 984 bytes and returns zero;
failure returns -1 without destination writes. Its actual dependency is
`MemCpyByte`: the C binds the existing `mem_cpy_byte` and preserves forward
byte-copy behavior, including overlapping buffers. Tests execute the original
getter and actual `MemCpyByte` instructions, including both overlap directions
and source==destination.

`control_get_stb_cc` requires a non-null handle and nonzero active validity.
It writes optional outputs in order: crush PQ, crush linear, clip PQ, clip
linear. The PQ fields are at handle offsets `0xa94`/`0xa96`. Linear conversion
clamps PQ to 4095 and interpolates the binary's `depq1024Lut` at each quarter
step. C binds the independently recovered `control_pq12_to_l`; the original
getter executes its inlined table instructions. Source fields are reread
between writes, preserving effects when outputs overlap the handle's PQ fields.
Invalid/null handles return -1 with every output unchanged.

`control_bindings.c` closes all HDMI getter dependencies. Its
`update_dm_l0_for_hdmi` and `update_dm_ext_for_hdmi` bind the recovered bodies
to real `control_get_dm_cfg`, `control_get_stb_tmm`, and `control_get_stb_cc`.
The existing TMM body, matrices, extension filtering, metadata size handling,
and LUT source remain independently pinned to the same module. The
`update_md_for_hdmi_bound` entry accepts only the still-unrecovered
`reduce_trims` callback; it needs that binding only when its size branch calls
it. The callback receives the target-pointer field address, and its changes
to count, lengths, target version, and unrelated base bytes are preserved.

Build `build_control_bindings.ps1`, then from `reconstruction/` run:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_control_bindings.py
```

`tests/control_bindings_results.json` records **6,284 matched comparisons** and
**113,112 guard comparisons**. It covers pending/active flag combinations,
null handles/selected pointers, the exact copy extent and overlapping copies,
all 4096 PQ codes and interpolation fractions, clamp endpoints, all optional
output masks, output aliases, and preservation of complete objects/unused tails.
Concrete preparation tests cover all 256 levels, graphics-priority modes,
configuration selection, output colors/ranges, metadata versions, CC clamp
boundaries, active-area offset/gain rules, and counts through 255.

There are **180 complete update pipelines** with actual original/recovered
update, l0, extension, configuration getter, CC getter, TMM getter, and byte-copy
dependencies. Only `reduce_trims` is modeled. Other tests invoke each concrete
l0 or extension binding directly. The reference records actual dependency
entry counts; native equivalence compares full objects and reduce callback
arguments/snapshots, without claiming instrumentation of native internal calls.

Instruction coverage is configuration getter 29/29, CC getter 69/69,
byte copy 23/23, l0 261/261, extensions 161/161, and update 225/227. The TMM
dependency reaches 30/95 here because these callers request only PQ outputs;
the comprehensive `control_infoframe_results.json` establishes the other TMM
branches separately. Update's two unvisited zero-count removal instructions
remain recorded explicitly in the metadata recovery note.

The original CC and TMM getter literals follow their instruction symbols at
`.text+0xa3e8` and `.text+0xa2c8`. Tests apply the actual ABS64 relocations to
ELF section 8 plus 66176. The CC instruction adds 0x120 to reach its LUT;
all interpolation comparisons therefore use original relocated table bytes.

Getter failures are tested directly. Bound HDMI helper tests require valid
active DM handles and selected configuration pointers: their original callers
ignore failure returns and otherwise consume uninitialized local PQ/config
bytes. The C retains that unchecked caller contract rather than adding a
different fallback. Inputs must cover every original access. `_mcount` is a
no-op and metadata-compaction `memcpy` is a checked byte-copy import model;
all getters and the actual `MemCpyByte` dependency execute machine instructions.
This is host DLL versus original ARM64 evidence, not kernel load/playback testing.
