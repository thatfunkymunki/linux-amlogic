# parser diagnostics

Reference: `s922x-j_dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
This unit contains four functions. It does not reuse the 5.15 module's
error table or context layout.

| Function | `.text` offset | Recovered behavior |
| --- | ---: | --- |
| `dv_md_parser_get_last_err` | `0x10bec` | Null handle returns `10`; otherwise return the signed 32-bit word at handle offset `0x18`. |
| `dv_md_parser_get_errstr` | `0x10c28` | Pass the code to `rpu_error_code_2_str`, clamping codes greater than `11` to `12`. Negative codes are unchanged. |
| `dv_md_parser_get_rpu_dcd_ctx` | `0x10c70` | Null handle or zero byte at offset `0` returns null; otherwise return the 64-bit pointer at offset `0x38`. |
| `rpu_error_code_2_str` | `0x167e8` | Codes `-22` through `0` index a 23-pointer table; all other codes return `Unknown error_code`. |

Ghidra's AArch64 listing shows `ldr w0,[x19,#0x18]` for last error and
`ldrb w0,[x19]` followed by `ldr x0,[x19,#0x38]` for the decoder context.
The error-string function adds `0x16` in a 32-bit register and accepts the
result only when it is at most `0x16`. Its literal-pool relocations point to
`.rodata.str1.8+0x14e8` (fallback text) and `.rodata+0x1d2e8` (table).
The table has 23 `R_AARCH64_ABS64` relocations into `.rodata.str1.8`.
Five table entries (`-19` through `-15`) share the fallback pointer. The
wrapper's two `R_AARCH64_CALL26` relocations both target
`rpu_error_code_2_str`.

The source preserves all table strings byte for byte, including the original
wording `it's legal range` and `to large`. It performs explicit little-endian
byte loads at the context offsets so the unit can compile without a target
libc header. These produce the original values for stable ordinary memory;
they do not claim the same bus-access pattern as the original single loads.
The handle must point to a valid parser context containing enough readable
bytes for the selected operation.

Run from the project root:

```text
python reconstruction/same_kernel/am6b/core/test_parser_diagnostics.py
```

The test pins the ELF hash and four instruction hashes, checks function
offsets, wrapper calls, literal-pool targets, all 23 table relocations and
string bytes, then compiles the source and checks all table codes, nearby
out-of-range codes, 32-bit extremes, null/valid handles, signed last-error
values, and decoder pointer gating. It passed with Visual C 19.44 on
17 September 2026. The generated host DLL is only a behavior check.

The source omits `_mcount` instrumentation. It has not been linked into an
kernel module or run on an AM6B. Parser allocation, state transitions,
RPU decoding, and hardware output remain outside this unit.
