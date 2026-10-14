# composer and DM validators

`composer_validation.c` implements two real validator bodies:

| Original symbol | ELF `.text` offset | Ghidra address | Bytes | Instructions |
| --- | ---: | ---: | ---: | ---: |
| `validate_dm_payload` | `0x11458` | `00111480` | 1464 | 366 |
| `validate_composer_config` | `0x11a98` | `00111ac0` | 4388 | 1097 |

Reference module: `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The function byte hashes are respectively
`3d90544ac27e00cf0c1fae8a192818269f8a67f89de499779f90be3775321c86`
and `b2545d1e31251eaaafedb8556cc1d17bdccfd963d8c55563378c82b04b1e5975`.
instruction/decompiler evidence is saved in
`../evidence/composer_ghidra.json`.

## Behavior and input contract

Both functions return 0 or -2, continue checking after errors, and retain the
reference's diagnostic strings and order. The supplied frame number and all
`%d` diagnostic values are interpreted at 32-bit width. Logger returns are
ignored. Severity is compared as an unsigned 32-bit value, so negative bit
patterns enable diagnostics as they do in the original instructions.
The logger signature returns `int`, matching the original metadata callback
bindings; the helper APIs use a signed severity parameter and preserve its
bits for the unsigned comparison.

The DM validator checks metadata IDs, refresh flags, EOTF/range-dependent
parameters, bit depth, color/chroma/full-range constraints, source PQ limits
and extension count. EOTF `0xfffe` and `0xffff` require zero parameters;
`0xffff` bypasses source PQ validation. Otherwise the accepted EOTF range is
0x8ccd..0xb333 and parameters retain the modular unsigned range checks.
The level-2 extension weight accepts signed -1..4095. Count 255 is diagnosed
but the original continues through all 255 records. Caller storage therefore
must include the 0x4c-byte prefix and the indicated number of 0x108-byte
extension records, even for invalid counts.

The composer validator checks all scalar and array fields in the 0x708-byte
configuration, including inactive coefficient slots. It checks bit-depth and
denominator ranges, luma/chroma pivot counts, mapping/order choices, pivots,
polynomial/MMR/NLQ coefficients, residual/resampling flags, filter selectors,
luma pivots and explicit filter coefficients. Variable shifts use the low
five bits, exactly as ARM64 W-register shifts do. Polynomial and filter
integer coefficients use modular range tests, MMR coefficients accept
-65536..65535, and NLQ integer coefficients retain their original signed
upper-bound-only check. Full input records must be stable and readable.
Logger callbacks may record diagnostics but must not mutate the input.

The existing 5.15 validator source was used as study material and a starting
point for the loops. field accesses, checks and instruction behavior were
checked independently, and the DM parameter-0 test uses the reference module's direct modular
16-bit range comparison. Acceptance of this source is based on the oracle
and module hash, not the earlier 5.15 results.

## Validation

From `reconstruction/`:

```powershell
& ./same_kernel/am6b/build_composer.ps1
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_composer_validation.py
```

The MSVC C11 host build passes `/O2 /W4 /WX` without a default runtime library.
These source units participate in the lane's aggregate freestanding AArch64
build; consult `../tests/core_arm64_build_results.json` for the latest
source/object hashes and actual build outcome.

The suite records **7,509 matching validator calls**. It compares return values,
the complete input with 32 external guard bytes, and every ordered diagnostic
format, field name and 32-bit integer argument. Fixtures perturb every aligned
composer word, every value of the relevant byte fields, scalar/range boundaries,
all byte-sized shift values, coefficient edges, chroma mapping/pivot modes,
extension level/weight boundaries, all logging thresholds and random records.
An input mutation and an extra diagnostic are intentionally introduced into
test snapshots to verify that those comparison failures are detected.

Composer coverage is **1097/1097** instructions. DM coverage is **365/366**;
the unvisited instruction at `.text+0x11458+0x584` follows the diagnostic for
extension count 255 and returns early if the reloaded count has become zero.
That branch requires a logger that changes the count during the callback,
excluded by the stable-input contract. No feasible DM instruction in the
declared domain is left unvisited.

`../tests/composer_validation_results.json` pins module/source/header/test/
DLL hashes before and after execution and records the selected literal pools,
counts and instruction coverage. Both original validator bodies execute as
machine instructions; `_mcount` and the recording logger are the only modeled
calls. There are no injected payload/validator bodies. This does not establish
payload-reader correctness, host registration, module loading or playback.
