# Control metadata ingress and HDMI preparation

All five new bodies are independently pinned to `p3i-t4c-dovi.ko`, module
SHA-256 `5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The Ghidra `.text` base is `0x100028`; addresses must include that displacement
when translating ELF section-relative offsets. No 5.15 implementation was used.

| Original body | `.text` offset | Instruction bytes | SHA-256 |
|---|---:|---:|---|
| `read_md` | `0x4948` | 452 | `d20f2e076d9037b95f1009bd2247ec2bfd6375fe731f5e944d4e9021b98fced9` |
| `filter_md` | `0x1e4c` | 276 | `ce93798291beb77027b7c00b426a3b54accff0750f177613e0b9aebddb1941de` |
| `update_md_for_hdmi` | `0x2124` | 908 | `c84aa4a62ce86d923bc3989ff3badd29a60bd024d6ef4892d0cc1de9fb328a38` |
| `update_dm_l0_for_hdmi.isra.1` | `0x3a4` | 1044 | `816a837773f1c7db5a15638fa1edd78d680b08e34a083fc28cf358438f776578` |
| `update_dm_ext_for_hdmi` | `0x120` | 644 | `942f80b305fbf70a4692a705a0fc97134815446935037bb2adee73a52a83e5ea` |

The local `.isra.1` body has the two-pointer context/metadata ABI recovered in
Ghidra. C exports the plain functional name with `_with_ops` for its explicit
getter bindings. The assembly and helper decompilations are saved in
`evidence/control_metadata_ghidra.json`; shared DWARF constants live in
`core/control_abi_recovered.h` and `evidence/control_abi_dwarf.json`.

## Representation and preserved behavior

Control metadata uses a packed 71-byte big-endian base, followed by 261-byte
records: four big-endian length bytes, one level byte, and 256 payload bytes.
This is distinct from the parser serializer's 76-byte base / 264-byte records.

`read_md` copies a short base directly, leaving its old extension count and
records intact. An empty input returns immediately without changing chroma.
It copies an extension header only with at least five remaining bytes. Missing
headers still validate and use the old record's declared length. Lengths above
256 fail after previous writes and offset updates, before the final chroma
write. Insufficient payload leaves its old bytes and does not advance the
offset; later declared records are still examined. The function never adjusts
the declared count. Its return counts copied base and extension headers,
excluding payload bytes; the offset includes payload bytes. The initial
remaining-size comparison is unsigned while extension comparisons are signed.
The C retains the 32-bit wrapping arithmetic and requires accessible backing.
`read_md_with_logger` records the original error callback; `read_md` is its
convenience entry with logging omitted.

`filter_md` processes only current level-1 records, using previous values at
the same index regardless of previous level or count. Each big-endian PQ value
becomes `(previous * 0xf333 + current * 0xccd) >> 16`, with 32-bit arithmetic.
All six current/previous input bytes are read before writes. Tests include
`current == previous` and preserve all other fields and previous storage.

`update_md_for_hdmi_with_ops` calls l0 then extension preparation before reading
count/version. It passes the address of `context.target_param` (`ctx+0x1048`)
to `reduce_trims`, then rereads count and version. Serialized-size accumulation
uses signed 32-bit results, including overflow from unusual declared lengths.
The binary's version-specific 512-byte checks and generic 2048/509-byte checks
ultimately trigger reduction/removal above 509 for stable metadata. Removal
compacts complete records while retaining unused trailing records. It discards
level 7 and levels 12..253, preserves 254/255, and sets payload byte 1 to 2 for
surviving level-254 records. It does not recompute declared lengths or enforce
another capacity check after removal.

The actual l0 helper selects the binary's four YUV/IPT narrow/full matrices,
preserves matrices for other colors, and substitutes the fixed RGB-to-LMS
matrix for IPT only when old color space differs from 2. It sets the PQ EOTF
fields, calls `GetStbTmm`, then rereads output color from its configuration.
It writes source min/max PQ, 12-bit signal depth, chroma 1 and full-range 1.

The actual extension helper clears count for graphics-priority mode (configuration
StbMode 0). Otherwise it removes levels 5/6. For level 1 it obtains `GetStbCc`
values and normally clamps minimum to at most 614, maximum to at least 2081,
and average to `[minimum+700, maximum-1]`. An existing level-254 payload byte 1
at least 2 and sink DM version at least 2 bypass these clamps. Critically, level-1
compaction writes only its six PQ bytes: the old destination header and payload
tail survive. Other levels copy entire records. Level-2 gain is set to 0x0800
when level-5 active-area availability is nonzero, right/top/bottom offsets are
nonzero, and its high gain byte exceeds 7; the left offset alone does not trigger
this adjustment. Callback count/handle mutations and unrelated bytes survive.

## Differential evidence

Build with `build_control_metadata.ps1`, then from `reconstruction/` run:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_control_metadata.py
```

The saved `tests/control_metadata_results.json` records **4,070 matched cases**
and **29,010 guard comparisons**. It compares full metadata/source/previous
objects, exact return/offset, callback order, arguments, input snapshots, and
callback changes to count, lengths, target version, DM handle, and configuration.
It includes all 256 levels, lengths 0..256 and invalid 32-bit declarations,
truncation at base/header/payload boundaries, count 255, same-object filtering,
HDMI size thresholds/overflow, active-area fields, and getter clamp boundaries.

There are **554 concrete serializer-to-reader pipelines**. Both the original
AArch64 serializer and recovered C serializer feed their corresponding actual
reader, with known/unknown levels, specialized length/endian encoding, full
count and truncation. Another **18 complete HDMI preparation pipelines** use
the original/recovered l0 and extension bodies inside their actual update body.

Instruction coverage is `read_md` 113/113, `filter_md` 69/69, l0 261/261,
extensions 161/161, and update 225/227. Update's two unvisited instructions
(`+0x384`, `+0x388`) clear count on the removal branch with zero count; the
stable-state paths return earlier when count is zero. They are not claimed as
executed. Serializer coverage here is 801/821; its separate comprehensive
serialization evidence remains authoritative for all packing/failure paths.
`read_md` has an error-format literal at `.text+0x4b10` after its instruction
size; the test applies its real ABS64 `.rodata+264` relocation.

## Remaining boundaries

`GetDmCfg`, `GetStbTmm`, `GetStbCc`, and `reduce_trims` are explicit injected
dependencies. Their callback models establish orchestration and writes, not
equivalence of those getter/trim algorithms. Standalone update tests additionally
inject arbitrary l0/ext mutations; the concrete pipelines execute their actual
bodies with only the lower getter/trim boundary modeled. `_mcount` is a no-op;
`memcpy` uses checked byte copying; `printk` records its format/length argument.

Reader source/metadata/offset objects are distinct; partial overlaps and invalid
pointers are outside the checked caller contract. Caller storage must cover the
original unchecked reads, even with invalid signed sizes. The C units are
freestanding and do not link unrecovered helpers implicitly. This is host DLL
versus original ARM64 evidence, not a kernel load or playback validation.

The subsequent concrete getter closure is now in `control_getters.c` and
`control_bindings.c`, with separate evidence in `control_getters.recovery.md`
and `tests/control_bindings_results.json`. That bound preparation entry executes
real configuration/TMM/CC getters and leaves only `reduce_trims` injected. The
older injectable tests above remain useful for deliberate callback mutations.
