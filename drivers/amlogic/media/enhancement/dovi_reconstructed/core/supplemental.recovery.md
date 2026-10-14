# VUI and ST2086 ingress

Reference: `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Ghidra analysis explicitly targeted this program. Its `.text` maps at
`0x100028`; the ELF offsets below exclude that import base.

| Original function | ELF `.text` offset | Bytes | C entry |
| --- | --- | ---: | --- |
| Decoder-local `LToPQ12` | `0x16840` | 156 | `luminance_to_pq12` |
| `rpu_decoder_push_VUI` | `0x1598c` | 268 | `rpu_decoder_push_VUI` |
| `mastering_display_colour_volume_read` | `0x17354` | 212 | `mastering_display_colour_volume_read` |
| `rpu_decoder_push_2086_payload` | `0x15908` | 132 | `rpu_decoder_push_2086_payload` |
| `dv_md_parser_process_vui` | `0x10710` | 80 | `dv_md_parser_process_vui` |
| `dv_md_parser_process_mastering_display_colour_volume` | `0x10760` | 88 | `dv_md_parser_process_mastering_display_colour_volume` |

## Contract and preserved behavior

The VUI handler ignores its second argument, including null or an unreadable
non-null pointer. It writes fixed YCC/RGB/LMS coefficients, offsets, signal
defaults, and diagonal into the DM metadata prefix, then sets `vui_available`
at decoder offset `0x315c0`. It preserves both source PQ fields. A null decoder
returns `-7`; valid nested `rpu_data` and DM metadata pointers are required.

The ST2086 reader consumes eight 16-bit primary/white-point coordinates without
storing them. It then reads maximum and minimum 32-bit luminance values and
writes the corresponding PQ fields at metadata offsets `0x46` and `0x44`.
Minimum luminance shifts at 32-bit width, wrapping before division by 10000.
Maximum luminance shifts at 64-bit width, divides, then truncates to 32 bits.
The reader returns the wrapping signed bit counter, including negative values
when called directly with a preinitialized counter near overflow.

The push wrapper first rejects a null decoder, then accepts zero-size input
without touching state; a null buffer with nonzero size returns `-7`. Otherwise
it initializes the actual recovered bitstream reader and calls the actual
recovered ST2086 reader. Successful parsing sets `st2086_available` at
`0x315bc`. The original bit reader is unchecked: callers must supply readable
refill padding and valid nested pointers, even for nominally truncated input.

The two parser wrappers check only their parser handle, not `f_init`. They
pass through raw decoder statuses and update `last_rc` only on failure. A
successful operation preserves an older `last_rc`. These are reference
semantics, not proposed robustness fixes.

`luminance_pq.c` implements the decoder-local copy of the original binary
search and fixed-point LUT interpolation. Another `LToPQ12` exists in the
control path; its implementation is outside this unit. The three decoder LUTs
are hash-pinned at `.rodata+0x1d3a0`, `+0x1d5a0`, `+0x1d7a0` and regenerated
using `tools/extract_luminance_tables.py`.

## Validation

Run from `reconstruction/`:

```powershell
& ./same_kernel/am6b/build_supplemental.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_supplemental.py
```

`tests/supplemental_results.json` records 5,453 matching calls, including 4,737
PQ values around every LUT threshold and random 32-bit values; 88 cases per VUI
function; 228 cases per ST2086 wrapper; and 84 direct reader cases covering bit
counter overflow. The reference executes original ARM64 instructions for every
selected function and its bitstream dependencies. Only `_mcount` is modeled.
The decoder-local PQ pointer literal lies beyond its ELF `st_size`; the harness
applies its exact ABS64 relocation and preserves the instruction bytes.

Comparisons include return values, all 202472 decoder bytes plus 32 guard bytes,
all 112 parser bytes, 512 metadata bytes including untouched state, and read-only
input/root snapshots. Native and emulated pointers are normalized explicitly.
All instructions in five target functions are reached. The ST2086 push wrapper
reaches 28/33; its five remaining instructions handle a negative reader return,
which cannot occur after its bitstream initialization resets the counter and
the reader consumes exactly 192 bits. Both branches are implemented.

These tests do not establish whole-module equivalence, live playback,
ST2094 parsing, decoder output, or T4c callback integration.
