# DM metadata serialization

`dm_rpu_payload_2_byte_sequence` is the serializer invoked by the metadata
sample callback. Its three-register ABI is a readable DM payload pointer,
writable byte sequence pointer, and writable signed 32-bit capacity pointer.
The C declaration uses opaque DM storage so it preserves the established
payload layout without relying on compiler-dependent structure declarations.

The reference is `p3i-t4c-dovi.ko`, module SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Ghidra program `p3i-t4c-dovi.ko` places `.text` at `00100028`:

| Function | Ghidra address | `.text` offset | Bytes | Function SHA-256 |
| --- | --- | --- | --- | --- |
| `dm_rpu_payload_2_byte_sequence` | `001141bc` | `0x14194` | 3284 | `3754ee2ada049a9440425d071ae638a9f3b77ac86e4d770c16b316ef1eabe9b4` |

The implementation was recovered from that function's own instructions;
`evidence/dm_serialization.decompiled.txt` and
`evidence/dm_serialization.disassembly.json` preserve the corresponding
analysis. `dm_struct_2_rpu_payload` is a different conversion function and is
outside this unit's scope. The serializer has no functional dependencies;
the only original call is `_mcount`.

## Behavior

The 71-byte scalar header combines DM identifiers without additional masks,
writes the scene flag, serializes matrices/offsets/EOTF/PQ fields in big endian
order, and appends the extension count. Input padding is ignored. DM records
start at `+0x4c`, use a 264-byte stride, and hold little endian length at `+0`,
level at `+4`, padding at `+5`, and the payload union at `+6`.

Each output extension starts with a four-byte big endian output payload length
and one-byte level. Record order and duplicate levels survive serialization.
Known levels ignore their declared input length except for level 8's optional
fields; the serializer itself performs no semantic validation.

| Level | Output payload length | Packing |
| --- | --- | --- |
| 1, 3 | 6 | Three big endian 16-bit fields |
| 2 | 14 | Seven big endian 16-bit fields |
| 4 | 4 | Two big endian 16-bit fields |
| 5, 6 | 8 | Four big endian 16-bit fields |
| 8 | 13, 15, 17, 23, 29 | Display index, six 16-bit fields, then optional fields |
| 9 | 1 or 17 | Primary index; eight 16-bit coordinates when index is at least 19 |
| 10 | 6 or 22 | Display index, two 16-bit fields, primary index; eight coordinates when primary index is at least 19 |
| 11 | 4 | Four raw bytes |
| 254 | 2 | Two raw bytes |
| 255 | 6 | Six raw bytes |
| Other | Declared input length | Unchecked raw payload copy |

Level 8 thresholds are input lengths 11, 13, 14, and 20. At those thresholds
it adds mid-contrast, clipping, six saturation bytes, and six hue bytes,
respectively. Levels 8 through 10 check capacity in stages, writing the
currently available payload before checking the next optional stage. The
length header is committed after all stages succeed. Consequently a later
failure can leave payload bytes behind an untouched length header.

Capacity below 71 returns `-12` without modifying output. Later exhaustion
also returns `-12`, preserves the input capacity, and keeps the scalar header,
complete earlier extensions, and any partial current extension writes.
Success returns zero and replaces capacity with the produced byte count.
The capacity comparison and unknown-length addition retain signed/wrapping
32-bit behavior from the original instructions.

## Differential evidence

From `reconstruction/`, run:

```powershell
./same_kernel/am6b/build_dm_serialization.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_dm_serialization.py
```

`tests/dm_serialization_results.json` records 20,532 matched calls, all
821/821 original instructions visited, 12,555 matching exhaustion returns,
and 7,977 matching successes. Each call compares the return, every byte of
the output object including untouched bytes, the length object, the complete
unchanged source object, and all six surrounding guards. The suite includes
all 256 level values, arbitrary identifier bits, declared unknown lengths
through 512 bytes, malformed known-level lengths through `0xffffffff`, all
staged capacity boundaries, mixed and duplicate record ordering, signed
capacity limits, and 255-record sequences. It performs 123,192 guard checks.
Reference instructions execute under Unicorn with only `_mcount` modeled as
a no-op; no serializer, packing, length, or copying helper is stubbed.

The host DLL compiles with MSVC `/W4 /WX`. Independent LLVM compilation at
`-O2 -ffreestanding -fno-builtin -mgeneral-regs-only` also succeeds for
`aarch64-linux-gnu` with no undefined symbols, recorded in
`tests/dm_serialization_arm64_results.json`. This is freestanding object
evidence, not a loadable T4c kernel module or a live playback result.

## Caller contract and integration

Supply separate readable DM, writable output, and writable length objects.
The normal DM payload is `0x10744` bytes; unknown extensions with lengths
above the 256-byte union need additional readable backing. The original
does not protect against such lengths or signed length overflow. Tests cover
unknown lengths through 512 with accessible backing and do not execute
arbitrarily large lengths or pointer aliasing. Keep the existing DM validation
before this serializer in the metadata callback.

Add `dm_serialization.c` to metadata/decoder binding build lists and bind
`metadata_callback_ops.serialize_dm` to this function. The existing metadata
callback supplies capacity 66626, equal to the 71-byte scalar header plus
255 maximum-size 256-byte unknown records and their five-byte headers.
The shared test runner and project status documents are maintained by the
integration task.
