# Control trim reduction and chroma distance

Recovered from `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
The matching `s922x-j_dovi.ko` is byte-identical. This is concrete control
metadata code, including the original fixed-point chroma distance chain.

| Original function | .text offset | Bytes | SHA-256 |
| --- | ---: | ---: | --- |
| reduce_trims.isra.2 | 0xde0 | 3052 | d980c833b721faf585af2113651f297a6dc476aee7a316e89afc9934fa00eb10 |
| sort_L2_by_tMaxLum | 0 | 288 | 7f92791f93fe75c773925e9e55ef99b4489c1ea0166d4ac8e3b9e900bfac219f |
| FindChromaDist | 0xac8 | 776 | 32c1f6611f4723cd96487ce15948f72ba0eb60c2ef5af108ba1ece7d35a5ae35 |
| L2PqX | 0x85c | 620 | 3e4471bd428aeb990ed9a4d7adc401e6aea578f48717ae4fbe9e4712b938b793 |
| _powInSTaylor.constprop.3 | 0x7b8 | 164 | 1747daafe8ae03cc7e341a66916d43ef0a4adf1563ac20c324756a22f489d022 |
| powTaylor | 0xc450 | 84 | f5b801505b1b317bb903a7384b0e9d8114bd44098818717ee3c9ad0e10d269dd |
| fxLOG2Taylor | 0xd2c8 | 348 | b2a2471d08dd6c61c4d071505d56b06de94843cbd7ce74af3c5bd8a038d142e6 |
| fxPOW2Taylor | 0xd440 | 236 | 50611d62c78f981349bd3b56ebf6dd58df033120c293c87e20a1dd4f765610d9 |
| unorm_s | 0xcf88 | 112 | 3135aeecfc62e8a1c0e276d92ca0174c1343bc402304827fecd3c3f4ce610d13 |
| smpy_ss | 0xcf54 | 52 | 5b94f7e8382e7d8494ac8aa35debfad410ef62a4d798e7f578ab31c7bbfc681b |
| sshr_sss | 0xd1e4 | 120 | c838fe985239323758e2f3543b2659ed813d0254eaf157ee8aa8704444625070 |
| fx_assert | 0xd2ac | 24 | 4d368941421d8f2f6ca2b537fb536d4854da07d22c969ea1f6f95901dc8e7953 |

Existing `control_pq12_to_l`, `mem_cpy_byte`, and `abs_int32` are reused.
The reference executes the actual `PQ12ToL`, `MemCpyByte` and `AssignM33I2I`
instructions as dependencies. `fx_assert` is a no-op in this module, even for
failed conditions; the C preserves that behavior.

## ABI and behavior

Assembly corrects Ghidra's mispropagated context type: the first argument to
`reduce_trims` is the **address of the target pointer**, usually context+0x1048.
It reads sink DM version at target+24, signed maximum PQ at target+52, and eight
Q26 primary coordinates at target+56. The metadata count is byte70, followed by
261-byte packed records at byte71. Each record has a four-byte big-endian length,
one-byte level, and 256-byte payload backing.

Legacy sinks remove levels3,8,9,10,11,254 before selecting level2 trims. Modern
metadata with the first level254 payload version at least2 removes level2,
selects level8/level10 pairs by luminance, and then clears additional removal
marks according to chroma distance. Built-in display IDs use the exact display
and primary tables; custom level10 primary coordinates are read big-endian and
scaled by2048. A missing matching level10 keeps the binary's existing l10 slot
value and uses key -1. Compaction copies the full261-byte record and preserves
the bytes after the resulting record count.

Sorting swaps equal adjacent keys during shrinking bubble passes. Equal keys
therefore reverse; changing this to a stable sort changes retained records.
`FindChromaDist` reproduces 32-bit differences, 64-bit multiply/add wrapping,
signed division, the signed32 extraction in RGB-to-XYZ scaling, two exact
matrix tables, cross-talk constants, and saturated absolute values. It updates
the original116-byte scratch state with the same matrix and vector writes;
untouched padding is preserved. The recovered `chroma_distance_state` mirrors
that state and the function retains the original non-reentrant scratch design.

Four constant tables are extracted by `tools/extract_trim_tables.py` from the
SHA-pinned ELF, recorded in `evidence/trim_tables.json`, emitted to
`trim_tables.inc`, and checked byte-for-byte in the compiled DLL:

| Table | .rodata offset | Bytes | SHA-256 |
| --- | ---: | ---: | --- |
| chroma_xyz_to_lms | 0x50 | 36 | e66c2337b1b13d26534f614a9b7f36749b3fdd721384159bc70b69a7eca53ffb |
| chroma_lms_to_ictcp | 0x78 | 18 | 6474f8433c9755a409dd39aeee844f2fdbd4aa3129cf4882c50c68551150c7ac |
| trim_display_table | 0x90 | 112 | 19d61af62c0adbb1de7085821f23e9d21f5a10d240dac15be3497cc756a71ac0 |
| trim_primaries_table | 0x100 | 640 | c13d07a7fefc62a4c9068e72c748a3f97fef89361ec411d02621d1436a65438f |

## Validation

`build_trim_reduction.ps1` compiles with MSVC `/W4 /WX` and links a standalone
DLL without CRT dependencies. Run with the locked uv environment:

```powershell
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_trim_reduction.py
uv --cache-dir .cache/uv run --locked python same_kernel/am6b/tests/test_control_hdmi_pipeline.py
```

The unit suite passed **11,064 comparisons**, including552 `reduce_trims`,
187 `FindChromaDist`,36 sort calls,1061 `L2PqX`, and the primitive Taylor chain.
All15 selected functions reached full instruction coverage: reduce763/763,
distance194/194, sort72/72, and L2PqX155/155. Every object call compares the whole
guarded caller arena, verifies the target pointer before address normalization,
and compares all116 scratch bytes. There are9300 object guard checks. Direct
sorting tests caller-owned arrays through255 entries, including equal keys.
Scalar tests include signed extrema, shift saturation, Taylor scale edge cases,
and every L2PqX magnitude branch threshold. Results bind function/table pins,
source/header/inc/test/generator/evidence hashes and the actual DLL hash.

The separate concrete HDMI suite retains the historical modeled boundary suite.
It runs original `update_md_for_hdmi`, both concrete HDMI metadata helpers,
GetDmCfg/GetStbTmm/GetStbCc, real trim selection, and the numeric chain against
the concrete native wrapper. Only `_mcount`, bounded `memcpy`, and bounded
`memset` are substituted. Its364 full preparation comparisons include built-in,
custom and missing level10 pairs, legacy sinks, version fallback, unrelated
vendor records and data exceeding509 serialized bytes. Full arena/guards,
pointer values and scratch state are compared; reports are recorded separately.

## Caller domain and remaining limits

`reduce_trims` has four fixed16-slot local arrays and a255-slot removal list.
Its caller must provide at most16 level2 and16 level8 records, a valid target
pointer to88 bytes, PQ code0..4095, and complete66626-byte metadata backing.
The original does not validate excessive trim counts; overflowing its stack is
outside this reconstruction's supported domain. The standalone sorting helper
instead requires caller arrays large enough for its actual number of level2
records. Arbitrary invalid pointers are excluded.

`pow_in_s_taylor` uses the low32 signed exponent as a loop bound; tests cover
0..16, and real L2PqX callers use4..16. Pathological huge exponent loops are
excluded. Signed right shifts rely on the supported two's-complement MSVC and
Clang targets. Full visited-instruction coverage and finite comparisons do not
prove equivalence for every possible input combination.

The concrete HDMI callers require valid active handles and selected config
pointers because the binary ignores getter failures. This recovery establishes
host instruction equivalence, not kernel module loading or hardware playback.
