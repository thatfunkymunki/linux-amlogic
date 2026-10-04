# DM metadata commit recovery

`dm_commit.c` recovers the state and numerical orchestration of `CommitMds`,
its metadata color path `CommitMdsCsc`, and the `CheckMds` change classifier
from the explicit Ghidra program `p3i-t4c-dovi.ko`. The reference SHA-256 is
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
Its Ghidra `.text` base is `0x100028`.

| Original body | ELF `.text` offset | Bytes | SHA-256 |
| --- | --- | --- | --- |
| CommitMds | 0x8c38 | 5372 | 9b2b3708f4dc61bc5ba5d9a2612e334281acf843ffab8b04015fb0e7b8671bca |
| CommitMdsCsc | 0x8908 | 804 | 3ec9cbc37eace0a46a32d8904f48d889a11822c73a03231a2c81600cc84b6816 |
| CheckMds | 0x88a8 | 96 | 3cedbb5089bd3c4fb42d8c91bcaf63ec0338c668da9b049db7528d300e0928d1 |

`tools/extract_dm_commit_abi.py` selects the actual CommitMds compilation unit
and emits 26 recursively resolved DWARF layouts, member-offset and size
assertions, enums, and exact filter/SDR/Pulsar tables. The resulting objects
include DM4400, configuration984, metadata520, kernel10928, color-control168,
tone-control168, input-map1128 and tone-map4104 bytes. The generated header
requires eight-byte pointers and naturally aligned complete distinct backing.
The source retains the original unchecked caller contract.

`check_dm_metadata` compares all 520 metadata bytes first. Equal objects return
zero; a difference confined to bytes 514..519 returns tone change 1; any
difference in the first 514 bytes returns configuration change 2. Metadata
pointer representations are part of those byte comparisons.

The main entry preserves active/pending validity, metadata reuse and change
classification, forced curve updates, secondary input, graphics/video
priority, matrix overrides, EOTF/range conversion, alpha ranges, output maps,
trim application and configuration-pointer exchange. Kernel pointers are
reloaded at the original call boundaries, including after the two upsample
calls. Curve return bits and output-map return bits are combined in the
original order. The color path snapshots both LMS/IPT matrices before
writing the input map and overwrites intermediate offset outputs exactly as
the original does.

Optimized machine signatures take precedence over stale DWARF prototypes.
CreateToneCurve receives three object arguments. CommitUs receives a chroma
enum, a pointer to the bit-depth scalar, and output. Primaries conversion
receives eight coordinates and one output pointer. The scalar divisions
retain the distinction between a positive 64-bit `0xffffffff` numerator and
the sign-extended shifted 32-bit denominator. Variable shifts are masked,
packed halfwords preserve their 32-bit bit patterns, and scale additions
wrap at 32 bits. The source uses 64-bit integers for the original AArch64
`long` intermediates despite Windows' 32-bit `long`.

`dm_commit.h` exposes all 27 calls as typed audit operations with explicit
caller context. `dm_commit_bindings.c` connects them to recovered
`color_matrix`, `kernel_maps`, `tone_controls`, fixed-point math and byte
helpers. Its public `commit_dm_metadata` takes caller-owned bindings for the
optional tone-curve override and trim diagnostics. `commit_dm_color` needs no
numeric dependency operations. There are no placeholder color/map/math
algorithms in these concrete bindings.

`CreateToneCurve3pxx` and its parameter, LUT and auxiliary-map closure are now
fully recovered in the five tone-curve C units. A null `tone_curve.build`
member selects the real builder through `tone_curve_concrete_ops()`; the
bindings object itself remains caller-owned and valid. An explicit callback
remains available for the historical boundary-audit fixture. The recovered
wrapper supplies the exact source/display/pair structures, slope output and
four 512-entry LUT buffers. The direct builder's formal finite count/capacity
domain and tested pair/cache aliases are recorded in `tone_curve.recovery.md`;
arbitrary source/display or DM/kernel overlap is not claimed. `sdiv_sls` is
shared once from `tone_math.c`, retaining its independently verified body.

`test_dm_commit.py` isolates commit state and call ordering with deterministic
typed dependencies; byte equality/copy and PQ conversion execute concrete
bodies. `test_dm_commit_review.py` independently audits metadata-byte
classification and expands state/branch coverage. `test_dm_commit_pipeline.py`
is the historical concrete numeric/helper boundary audit and deliberately
injects the deep curve builder. `test_dm_tone_pipeline.py` separately executes
the real complete DM-to-tone closure, including curve fitting, generated LUTs,
and all auxiliary maps. In the full path only instrumentation, host byte-memory
services and diagnostic sinks are modeled; no numeric dependency or builder
is modeled. Reports compare complete guarded arenas, returns and relevant
ordered callback data against relocated
original ARM64. The integration summary records current source/native hashes,
original pins, combined instruction coverage and finite comparison counts.
The previously recorded combined commit audit reaches 1,341 of 1,343 original main instructions;
the color path reaches all 201 and metadata classification all 24. Main offsets
`0x13f4` and `0x13f8` follow a chroma reload that can change only when metadata
overlaps kernel output. They cannot execute under the declared distinct-object
contract. This exclusion is recorded separately from exercised instructions.

The standalone strict MSVC `/W4 /WX /NODEFAULTLIB` builds and the optimized
freestanding AArch64 object are source validation. The aggregate currently
contains 62 C units, 201 public functions and 14 public data symbols with no
unresolved symbols. The previous outer control/register test remains a
separate CommitMds audit-producer fixture; its injected curve outputs are not
the full numerical pipeline evidence. Neither the finite concrete tests nor
the relocatable build establish a functional kernel replacement or Dolby
playback. The T4c 48-byte adapter, complete Kbuild module, live module identity
and AM6B+ hardware output remain pending.
