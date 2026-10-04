# parser identity getters

Reference: `s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
This unit covers six getters in that binary only. The 5.15 STB 2.6 module has
a different API version (`0.4.3`) and is not the source for these constants.

The `dv_md_parser_get_*` entries at `.text+0x10450`, `+0x1046c`, and
`+0x10488` are 28 bytes each. After the instrumentation call to `_mcount`,
each branches to the corresponding `vdr_rpu_get_*` entry. The `R_AARCH64_CALL26`
relocations at wrapper offset `+0x10` identify those three targets. The C
wrappers retain the call relationship and return the exact same pointer as
their corresponding `vdr_rpu` getter.

The direct getters are at `.text+0x10cb8` (API), `+0x10ce0` (algorithm), and
`+0x10d08` (build information). Their literal-pool `R_AARCH64_ABS64`
relocations point to `.rodata+0x1c210` for the version pair and
`.rodata.str1.8+0x6b8` for build information. The algorithm getter adds eight
bytes to the shared version base. Its exact returned bytes are `1.1.2\0`;
the API bytes are `0.4.1\0`. The build information is:

```text
Build configuration flags:
	ENABLE_FLOAT		0
	NUM_RPU_BUFFERS		64
```

Each displayed line, including the last, ends in a newline; the entire string
ends in a NUL byte. The source keeps the two version strings eight bytes apart
and shares the direct getter pointers with the wrappers.

Run `python reconstruction/same_kernel/am6b/core/test_parser_identity.py`
from the project root. The test verifies the reference hash, function offsets
and wrapper instruction hashes, relocation targets, exact string bytes, and
compiled C results. It also verifies pointer identity between each wrapper
and direct getter, and the eight-byte version pointer difference. The test
passed on Windows with Visual C 19.44 on 17 September 2026.

The C source omits `_mcount`, which is compiler instrumentation rather than
parser behavior. This unit does not implement parser state, RPU decoding,
kernel registration, or hardware behavior. No Kbuild integration or device
execution has been validated for it.
