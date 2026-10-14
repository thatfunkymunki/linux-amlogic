# Comparative reconstruction changelog

This document records the retained STB 2.4 → STB 2.6 differences in the
source-buildable modules. STB versions identify Dolby libraries, independently
of Linux versions. Linux 4.9 and 5.15 do not explain changed fixed-point math,
metadata parsing, register packing or library state machines.

| Library | Original reference SHA-256 | Algorithm bodies | Selected host |
| --- | --- | --- | --- |
| STB 2.4 | `5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b` | 238 | P3i T4c 4.9.269 |
| STB 2.6 | `f6c26659a255447685ceac9441e399c999b1fae9c6435c48d70e14a14dd7f8f7` | 321 | CoreELEC 5.15.170 and 5.15.196 |

## Source convergence decisions

These changes improve the reconstruction while preserving each original
contract. They do not claim that different native algorithms are equivalent.

- Use the same bytewise YUV offset and scalar Kr/Kg/Kb matrix implementation; retain actual LMS/IPT factors (219/224 versus uniform 219), coefficient tables and narrowing policies.
- Use independent temporary assignments and matching literal spelling. Share inverse-matrix arithmetic with common raw-32 scale inputs interpreted through their low 16 bits. Retain the signed-16 versus raw-32 singular result and truncation versus saturation.
- Share named raw arithmetic and indexed numeric-access helpers. Unify proven same-bit signed/unsigned scalar declarations, preserving signed results where maintained callers perform signed arithmetic.
- Share the six-stage raw-word logarithm polynomial, power polynomial and bounded square-root approximation. Retain the different finishing/scale branches. Restore the meaningful original manager/scale results for InitMmg and GetRgb2LmsWpByDefV3Fxp; their former STB 2.6 void declarations were unresolved decompiler artifacts.
- Share DeGamma arithmetic and byte-identical tables through direct native-owner bindings; mask the common low-16 inputs explicitly and retain the 32-bit versus 64-bit result.
- Share the low-level parser prefix/layout, lifecycle, buffering, reset, decoder dispatch, validation, supplemental input and extension code. Select actual differences through named `parser_contract.h` policies.
- Share byte alignment, residual-coefficient traversal, ST2094 inclusive target windows, synthetic defaults and byte-published extension bookkeeping. Algebra and native comparisons show that the former spellings were equivalent under their recorded backing/callback contracts.
- Share composer packing and the constant-zero unsigned-short return with pointer/dimension parameters; retain composer bases/extents, native mode selection and source preparation. Share LUT packing with native source offsets.
- Replace opaque CLI offset names with role-based priority/flag accessors. Rename control temporaries from their audited uses; retain byte offsets, operand widths, pointer reloads, partial writes and overlapping publication order.
- Use positive unsigned 32-bit kernel-stream byte counts and wrappers, raw-word minimum-frame-size results, and a no-argument constant-zero destroy helper. These preserve original return/register bits and maintained callers.
- Replace equivalent metadata endian trees, identity-prefix initializers and trial-square-root unrolling with explicit shared loads and bounded loops, retaining checkpoints, store order, count traps and partial state.
- Replace the remaining decompiler local/parameter/label names in 20 C file/version instances with audited functional roles. Preserve reused bindings with explicit multi-role names; no types, casts, operators, evaluation/store order, control flow or source line counts change. Align the tone-control prototype parameter with its actual previous-mode argument. Function-scoped lexical proofs and final linked machine/data comparisons validate this source-only cleanup.
- Share comparative parser, control, runtime, numeric and source-guide documentation. Keep native owners, layout assertions, reference identities and matching-kernel build profiles explicit.

- Call actual DM/control function names directly instead of private endpoint aliases; retain only required OS-service/data-owner bindings. Remove unused CONCAT definitions, redundant local width guards, unused includes and proven duplicate casts.
- Use independent parser policies for application allocation and metadata-copy sign extension. Share decoder PQ table bindings through each version's existing canonical owner macros.
- Use the common bytewise BE32 reader and once-per-row white-point division core. Remove the proven unreachable STB 2.6 gamma breakpoint guard. Full-domain algebra and focused own-version original-instruction tests preserve outputs, traps and partial writes; these are source convergence, not new library functionality.

Source text uses LF line endings. The STB 2.4 already-initialized diagnostic
was removed with its unreachable branch: original initialization clears the
tested byte before the decoder setter, which cannot modify that byte or call
the logger. Repeated serialized initialization follows the same path. This
is dead-code cleanup, not a changed reachable diagnostic policy.

## Retained parser behavior

### P01: Allocated parser instances and application layout

STB 2.4 retains a static 0x780 application context, contiguous payload backing, singleton parser and persistent external counters. STB 2.6 allocates a 0x770 context, 0x10442-byte sequence and queried parser arena, stores output sizes per instance, retains the frame counter globally and frees/nulls allocations in the native failure/release order.

### P02: Host metadata processing/reset contract

STB 2.4 accepts the historical logger/init flag, ignores reset flags, leaves release empty, passes its full eos byte and resets the parser after a successful processing call. STB 2.6 accepts an explicit instance, adds flags and input_kind, uses flags bit 0 for flush, runs VUI/ST2086/T.35 ingestion for kinds 1/2, does not reset after process success, and resets the global frame counter after successful explicit reset.

### P03: Reset flags and direct-mode deep clear

STB 2.6 adds a second flags argument to both reset APIs; flags==1 clears the current header (0x136), DM payload (0x10744) and two payload records (0xe070) in direct mode. STB 2.4 resets indices/state/MEL without those payload clears. Buffered STB 2.6 reset traps after writing the first fifteen payload handles for a larger slot count; STB 2.4 retains unchecked copying.

### P04: Buffered error draining and MEL accounting

Both parser versions stop for -20/-21 queue sentinels. STB 2.4 reports the first other nonpositive output once and stops, and maps any nonzero decoder MEL byte to flag bit 0. STB 2.6 continues draining after other errors according to its native unsigned loop condition, retaining the last error result, and ORs the raw MEL byte into caller flags.

### P05: Fatal limits and native profile-16 row

STB 2.6 retains fatal checks for decoder/query level>=16; validation level 16 uses BRK #1 and larger levels BRK #0x5512, profile>16 is fatal while profile 16 uses its separate retained 38-word row, mapping pivot15 is fatal, payload IDs>=15 are fatal, spatial modes>=8 are fatal and sentinel searches retain whole-row/table bounds. Buffered byte append increments length before trapping for previous length>0xfff. STB 2.4 retains its -4/-5 validator returns and unchecked pointer/array paths, including write-before-post-check behavior in buffering.

### P06: Sparse identity fallback writes

STB 2.4 fallback initializes each complete 28-byte NLQ record. STB 2.6 preserves the native sparse/overlapping store sequence at 0x6874..0x68c7 and exposes set_default_payload as a separate native entry. Common polynomial defaults and bytewise access do not erase the differing untouched NLQ bytes.

### P07: Decoder diagnostics and API strings

Parser API text changes from 0.4.1 to 0.4.3 while algorithm text remains 1.1.2. STB 2.4 keeps algorithm text at API text+8; STB 2.6 keeps separate string objects. Diagnostic source path changes to the relative Dolby source path; header/payload/DM diagnostic lines change 0x225/0x236/0x243 to 0x22a/0x23b/0x248, and the extension diagnostic base changes 0x581 to 0x585. STB 2.4 omits the partition-count printf argument; STB 2.6 supplies it. Module diagnostics retain their native logger/import selection and existing reconstruction prefixes.

### P08: Extension callback interface and record selection

STB 2.4 extension payload reading receives explicit configuration, event-callback slot and typed reader; STB 2.6 receives the decoder and derives those dependencies from it. Both only read the byte index, which is now const in both signatures. Non-first payload diagnostics test kind==2 in STB 2.4 versus kind!=1 in STB 2.6. Unknown extension payload copy is unchecked by the 256-byte union limit in STB 2.4 and stops after byte 255 in STB 2.6. Equivalent local index-width and kind type spelling differences have been removed.

### P09: DM extension bit-count and sequence-offset accounting

STB 2.4 adds initial extension read results and second incremental-update read results to initial_bits before computing its returned bit count; STB 2.6 omits those adjustments. For unknown packed DM extension payloads, STB 2.4 addresses output through signed 32(position+5+offset), retaining wrapping32-bit arithmetic; STB 2.6 addresses block[5+offset] after selecting the block pointer. This is serialization addressing, not sequence-header parsing.

### P10: NLQ sign-mask truncation

For the retained NLQ coefficient path, STB 2.4 uses1u<<((width+1)&31); STB 2.6 uses2u<<(width&31). When width&31==31, STB 2.4 selects bit 0 while STB 2.6 drops the sign mask to 0. PARSER_NLQ_TRUNCATING_SIGN_SHIFT names this actual arithmetic difference; equivalent one-bit-reader helper selection has been removed.

### P13: Native entry visibility and historical VUI argument

STB 2.4 keeps the payload-level MEL predicate private and accepts an ignored VUI pointer in decoder/parser VUI calls. STB 2.6 exposes the payload-level MEL entry and removes that unused VUI argument. The shared implementation preserves these native API contracts through explicit declarations.

### P14: Metadata callback copy-length widening

After the same signed capacity comparison, STB 2.4 widens metadata copy length through unsigned 32 bits; STB 2.6 sign-extends it to 64 bits. Normal serializer lengths are positive, but the native distinction remains explicit for nonpositive abnormal lengths and partial-output behavior.

## Retained control behavior

### C-control-entry

STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.

### C-control-lifecycle

STB 2.4 resets on input format -1 and calls its no-op destroy helper on ordinary completion without clearing initialized. STB 2.6 resets on zero input count, clears native staging and initialization/content bytes, and retains shared control state across ordinary calls. Init_cp takes instance/configuration inputs, rejects counts >=5 and retains native zero/negative-count behavior. The destroy helper now has the same no-argument declaration and zero-result body; its old 2.4 arena argument was ignored by the native instructions.

### C-manager

The 2.4 manager has 32 request slots and signed unchecked indexing after its diagnostic callback and sizes a singleton arena. The 2.6 manager has 24 slots, reloads its count between selected writes, returns -1 at the exact threshold and traps for further/native unsigned bounds, retains partial writes, and sizes per-instance kernel/extensions. InitMmg returns the manager pointer in both native instruction streams and now has one pointer-return declaration; the prior 2.6 void spelling was a decompiler choice, not a native ABI difference. RegMemReq uses a common unsigned in-device word: each native implementation merely stores those 32 bits and maintained callers pass boolean values.

### C-getters

The 2.4 configuration getter selects pending or active validity and its stored pointer; GetHDmKs reads the singleton pointer. The 2.6 configuration getter returns context+8 when nonnull, kernel access uses the native signed index-minus-one slot, and run/signal/TMM getters select native instance records with their original failure/trap behavior. Get_dm_ctx_buf is also a native distinct pointer calculation/load. Kernel-buffer publication forwards the common unsigned byte count; only 2.6 accepts an instance index and traps at index >=4.

### C-frame-size

Native gray color is 4 in 2.4 versus 5 in 2.6; planar 2.4 sizing includes chroma while 2.6 gray omits it. Minimum sizing now uses one unsigned-word argument/result declaration and a common wrapped-product/signed-quarter-division expression. The native difference is only the gray omission condition, not a decompiler signedness artifact. Compact row-pitch sizing still halves each signed dimension before its products, unlike minimum sizing.

### C-vsvdb

The 2.4 entry writes an 88-byte target descriptor and returns -1 for null/invalid identification. The 2.6 helper writes its control-state capability fields, treats null as retained state/success and returns -8 for invalid identification/version/length. Each keeps its original capability/luminance partial writes. Luminance units and rounding differ; in particular 2.6 v1 squares/left-shifts a u32, reinterprets signed and divides toward zero. Original instructions at 0015c4b4..0015c4c4 confirm signed lowering. Proven equivalent packed-primary expressions now use the clearer common spelling.

### C-composer

Shared composer packing keeps a constant-zero unsigned-short result and pointer/dimension arguments. The native composer base/buffer extent are 176/900 bytes in 2.4 versus 200/968 in 2.6. Mode is residual_flag!=0 versus constant zero. Width/height originate from 2.4 source+12/+16 versus 2.6 source+0/+4. STB 2.6 can change selected 8-bit BL/EL configuration and coefficients to 10 bits in place before packing. Interleaved loads/stores remain valid for its overlapping views; 2.4 requires disjoint objects.

### C-lut

The four 15-bit paired LUT streams and final 32-bit input-map copy now have one implementation and pointer interface. Callers select tone/input bases kernel+1328/+200 in 2.4 versus kernel+0x540/+0xd8 in 2.6. Output order and 2.6 overlap behavior are preserved.

### C-hdr

The 2.4 infoframe builder uses its input-format/HDR10 arguments and fixed primaries. The 2.6 builder derives primaries from fixed-point target words and its publisher applies native primary-input HDR10 passthrough/debug selection. For level-6 extensions, 2.4 processes matching records in order whereas 2.6 selects the last record first. Optional output-format getters expand from required 16-byte records to native PQ/luminance/primaries records in 2.6.

### C-metadata-storage

The 2.4 control representation has a 71-byte prefix and fixed 261-byte extension slots in current/previous 66626-byte backing, optional temporal level-1 filtering, logger failure for length>256 and native HDMI trim/removal/compaction. The 2.6 outer control path consumes contiguous variable-length records in 0x1000-byte slots, retaining native signed offsets, incomplete-header reuse, per-source progress and fatal capacity checks after partial writes. It has two metadata/composer staging slots despite three private host input records.

### C-publication

The 2.4 raw HDMI publisher calculates size before payload writes, caps >512 words, logs and preserves the previous payload after availability is cleared. The 2.6 publisher retains native raw-word and extension writes interleaved with fatal capacity checks, publishes transport control data and content information, and preserves register/LUT slot selection. Core register/host field offsets and buffer extents differ. HDR publication now uses four explicit LE64 copies at offsets 0,8,16,21, retaining the final overlapping store.

### C-transport-content

STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.

### C-graphics

The 2.4 graphics path derives a separate 64-byte graphics descriptor from CLI fields and applies stored luminance offsets. The 2.6 path uses native input_mode 2, format defaults/primaries, graphic luminance words and a separate Core 2 slot. Native gamma, matrix, source/default field writes and partial-state order remain version-specific.

### C-initializers

STB 2.4 initializes pending/active DM pointers, its 168-byte tone-control fields and trim arrays with retained self-pointers/defaults. STB 2.6 initializes native multi-signal/active kernel state, a larger tone-control record and extension descriptors. InitMdsExt now has the same pointer declaration; 2.6 intentionally ignores configuration while 2.4 reads it. Counts, optional writes, identity LUT initialization and fatal paths are retained.

### C-data-ownership

STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.

### C-cli

Both modules use bytewise CLI priority accessors. Priority is byte 52 in the 112-byte 2.4 record versus byte 0x1d14 in the 7468-byte 2.6 record. The 2.6 tail additionally contains selected input count/primary, native reserved init/input flags and level-11 controls. Unknown priority-override/init/input fields remain explicitly reserved; no speculative whole-struct overlay was introduced.

## Complete conversion-path ledger

The conversion has 128 differing paths and 158 exact Git hunks.
Each hunk below has a specific library behavior, API/layout, native data
dependency, or host/compiler compatibility basis. Kernel versions do not
explain Dolby algorithm differences. Shared files are absent; F identifiers
remain stable when an incidental difference is removed. The decoder PQ
binding header is now common, so its former F097 entry is retired.

Source-only convergence above is distinct from these functional updates.
Native no-op call structure inside genuinely different algorithms is
identified explicitly and is not presented as additional functionality.

### F001: `README.md`

- **F001.H01** (functional update documentation): Identify the STB 2.6 oracle and 321-entry target; describe allocated parsers, multi-input settings, PQ cap, saturating matrix narrowing and the matching Clang/non-CFI host requirements.
- **F001.H02** (host and library contract documentation): Replace the singleton 13,468-byte output-prefix/host-owned-tail description with three private inputs, two Core 1 slots and the 112-byte host table; document allocated parser release and retained registration-result storage.
- **F001.H03** (matching-kernel build contract): Select the verified CoreELEC 5.15.170 and 5.15.196 source/header revisions instead of the T4c 4.9 build; both 5.15 kernels use the same module source/algorithm bytes with distinct release metadata and CRCs.
- **F001.H04** (compiler and callback compatibility): Explain LLVM/IAS and common-drivers include selection and direct callbacks on CONFIG_CFI_CLANG=n. These are build/host compatibility requirements, independent of Dolby arithmetic changes.

Retained details:

- STB 2.4 reference5ffe2348...130b implements238 algorithm entries; STB 2.6 referencef6c26659...8f7 implements321. These library identifiers are separate from kernel versions.
- Selected build and host boundary differs: T4c4.9.269 singleton versus CoreELEC5.15.170/.196 multi-instance, with their matching generated kernel inputs.

### F002: `build_profile.mk`

- **F002.H01** (compiler and host-header compatibility): Select the 5.15 Clang/LLVM integrated-assembler options, common-drivers host/include paths, wrapping signed arithmetic and aliasing policy instead of the 4.9 GCC/GNU11 interface; keep the common dovi.ko target.

Retained details:

- 4.9 uses AArch64 GCC, GNU11 and -fno-builtin with in-tree host declarations. 5.15 uses Clang/LLVM and integrated assembler, common-drivers includes, -fwrapv and -fno-strict-aliasing.
- 5.15 direct callbacks require CONFIG_CFI_CLANG=n; generated CRCs, instrumentation and stack protection come from the selected kernel.

### F003: `callback_registration.c`

- **F003.H01** (host compatibility): The 48-byte legacy table supplies singleton callbacks. The 112-byte current host object publishes the 96-byte multi-instance library prefix in the correct multi slots, keeps unsupported TV/HW5 slots null, and retains mutable version storage/lifetime.

Retained details:

- 4.9 publishes a const48-byte table containing version/init/reset/process/release/control. 5.15 publishes a mutable112-byte host object with 96-byte STB prefix, multi-control and multi-parser callbacks, and null TV/HW5 entries.
- 5.15 receives the version pointer at registration; the complete object remains resident even after some host registration failures.

### F004: `core/byte_utils.c`

- **F004.H01** (library byte-copy edge behavior): For negative signed counts, STB 2.4 copies zero bytes; STB 2.6 sign-extends the 32-bit count to a raw 64-bit countdown and continues until zero or a backing fault. Positive forward-copy and overlap propagation remain the same.
- **F004.H02** (library byte-compare edge behavior): For negative signed counts, STB 2.4 immediately reports equality; STB 2.6 scans with the sign-extended 64-bit countdown until mismatch, zero or a backing fault. This is native behavior, not hardening or a kernel rule.

Retained details:

- 4.9 uses signed count>0: negative counts copy nothing and compare equal. 5.15 sign-extends the 32-bit count to 64 bits and loops until zero: negative counts become huge lengths and can fault or encounter a mismatch.
- Forward-copy overlap propagation remains shared; neither lane is hardened or replaced with memmove.

### F005: `core/byte_utils.h`

- **F005.H01** (library byte-buffer contract): Document the same signed-count API's actual negative-length difference; no declaration-width or formatting difference is introduced.

Retained details:

- Negative count skips/equal in 2.4; huge sign-extended count in 2.6. Buffer extent and forward overlap requirements remain explicit.

### F006: `core/color_basis.c`

- **F006.H01** (native-library additions): The common vector/matrix copies, white-vector row sum and scale12/15 getters are unchanged.2.6 adds cross-talk/inverse-cross-talk constructors, XYZ→LMS and ICtCp IO/DM bases with their actual coefficient packing, scale results and raw-divide zero semantics.

Retained details:

- Four element-copy/row-sum operations share color_basis_impl.h with identical bodies and alias/reload rules; fixed LMS/IPT getters now also share their unsigned 32 result declarations because the results are exactly12/15.
- STB 2.6 adds cross-talk and inverse-cross-talk matrix constructors, XYZ-to-LMS constants, ICtCp IO/DM forward/inverse bases and their scale results. The arbitrary-scale inverse retains zero-divisor AArch64 behavior.

### F007: `core/color_basis.h`

- **F007.H01** (native-library additions): The common vector/matrix copies, white-vector row sum and scale12/15 getters are unchanged.2.6 adds cross-talk/inverse-cross-talk constructors, XYZ→LMS and ICtCp IO/DM bases with their actual coefficient packing, scale results and raw-divide zero semantics.

Retained details:

- Four element-copy/row-sum operations share color_basis_impl.h with identical bodies and alias/reload rules; fixed LMS/IPT getters now also share their unsigned 32 result declarations because the results are exactly12/15.
- STB 2.6 adds cross-talk and inverse-cross-talk matrix constructors, XYZ-to-LMS constants, ICtCp IO/DM forward/inverse bases and their scale results. The arbitrary-scale inverse retains zero-divisor AArch64 behavior.

### F008: `core/color_narrow.c`

- **F008.H01** (native-library arithmetic and API): LMS/IPT narrowing changes from separate219/224 row/column factors to uniform219, in addition to truncate-versus-saturate final packing.2.4 general RGB/YUV helper contracts retain their signed scale/depth interpretation;2.6 adds ICtCp IO narrow constructors. The YUV offset and scalar Kr/Kg/Kb routines remain identical.
- **F008.H02** (native-library arithmetic and API): LMS/IPT narrowing changes from separate219/224 row/column factors to uniform219, in addition to truncate-versus-saturate final packing.2.4 general RGB/YUV helper contracts retain their signed scale/depth interpretation;2.6 adds ICtCp IO narrow constructors. The YUV offset and scalar Kr/Kg/Kb routines remain identical.
- **F008.H03** (native-library arithmetic and API): LMS/IPT narrowing changes from separate219/224 row/column factors to uniform219, in addition to truncate-versus-saturate final packing.2.4 general RGB/YUV helper contracts retain their signed scale/depth interpretation;2.6 adds ICtCp IO narrow constructors. The YUV offset and scalar Kr/Kg/Kb routines remain identical.

Retained details:

- YUV offset and scalar Kr/Kg/Kb-to-YUV/RGB matrix bodies and declarations are identical across versions, with explicit bytewise stores, raw signed divide and kg=0 preservation of coefficients four/five. Narrow matrix result widths were normalized after finite shift-class bounds proved all scale results positive.
- STB 2.4 LMS/IPT narrowing applies 219 luma and 224 chroma factors; STB 2.6 applies 219 uniformly. Bits=10 matrix outputs prove this is independent of saturating narrowing.
- STB 2.4 supplies general full/narrow RGB/YUV helpers with explicit signed 16 scale/depth views; STB 2.6 adds ICtCp narrow IO matrices and retains its different unusual-depth paths. Coefficient packing remains truncate versus saturate; that is independent of the now-common bounded scale result types.

### F009: `core/color_narrow.h`

- **F009.H01** (native-library arithmetic and API): LMS/IPT narrowing changes from separate219/224 row/column factors to uniform219, in addition to truncate-versus-saturate final packing.2.4 general RGB/YUV helper contracts retain their signed scale/depth interpretation;2.6 adds ICtCp IO narrow constructors. The YUV offset and scalar Kr/Kg/Kb routines remain identical.

Retained details:

- YUV offset and scalar Kr/Kg/Kb-to-YUV/RGB matrix bodies and declarations are identical across versions, with explicit bytewise stores, raw signed divide and kg=0 preservation of coefficients four/five. Narrow matrix result widths were normalized after finite shift-class bounds proved all scale results positive.
- STB 2.4 LMS/IPT narrowing applies 219 luma and 224 chroma factors; STB 2.6 applies 219 uniformly. Bits=10 matrix outputs prove this is independent of saturating narrowing.
- STB 2.4 supplies general full/narrow RGB/YUV helpers with explicit signed 16 scale/depth views; STB 2.6 adds ICtCp narrow IO matrices and retains its different unusual-depth paths. Coefficient packing remains truncate versus saturate; that is independent of the now-common bounded scale result types.

### F010: `core/color_primaries.c`

- **F010.H01** (native-library arithmetic, constants and added APIs): The full RGB/YUV paths use different coefficient sources and signed/unsigned division paths; their ordinary coefficient differences cannot be explained by kernel changes. Narrowing keeps distinct unusual-depth policies and native packing.2.6 adds descending sort, xy/CCT, xyY/XYZ, primary lookup and custom-primary matrix entry points. Scale26 literals now match. Local fallback signed/unsigned constant spellings are representations of the same raw values, not new coefficient differences.
- **F010.H02** (native-library arithmetic, constants and added APIs): The full RGB/YUV paths use different coefficient sources and signed/unsigned division paths; their ordinary coefficient differences cannot be explained by kernel changes. Narrowing keeps distinct unusual-depth policies and native packing.2.6 adds descending sort, xy/CCT, xyY/XYZ, primary lookup and custom-primary matrix entry points. Scale26 literals now match. Local fallback signed/unsigned constant spellings are representations of the same raw values, not new coefficient differences.
- **F010.H03** (native-library arithmetic, constants and added APIs): The full RGB/YUV paths use different coefficient sources and signed/unsigned division paths; their ordinary coefficient differences cannot be explained by kernel changes. Narrowing keeps distinct unusual-depth policies and native packing.2.6 adds descending sort, xy/CCT, xyY/XYZ, primary lookup and custom-primary matrix entry points. Scale26 literals now match. Local fallback signed/unsigned constant spellings are representations of the same raw values, not new coefficient differences.

Retained details:

- The independent temporary[5]/temporary[6] stores and scale 26 literal now use matching presentation. Other RGB/YUV full matrices retain their original signed/unsigned division paths and coefficient sources.
- STB 2.4 reads a seven-definition RGB-to-XYZ table; STB 2.6 has separate Kr/Kg/Kb constants whose low-order coefficients can differ, plus19 primary records. Raw initializer values were preserved.
- STB 2.6 adds descending sort, xy/CCT and xyY/XYZ conversions, primary lookup and custom-primary matrix builders with native determinant masks, bounds traps and output widths.

### F011: `core/color_primaries.h`

- **F011.H01** (native-library arithmetic, constants and added APIs): The full RGB/YUV paths use different coefficient sources and signed/unsigned division paths; their ordinary coefficient differences cannot be explained by kernel changes. Narrowing keeps distinct unusual-depth policies and native packing.2.6 adds descending sort, xy/CCT, xyY/XYZ, primary lookup and custom-primary matrix entry points. Scale26 literals now match. Local fallback signed/unsigned constant spellings are representations of the same raw values, not new coefficient differences.

Retained details:

- The independent temporary[5]/temporary[6] stores and scale 26 literal now use matching presentation. Other RGB/YUV full matrices retain their original signed/unsigned division paths and coefficient sources.
- STB 2.4 reads a seven-definition RGB-to-XYZ table; STB 2.6 has separate Kr/Kg/Kb constants whose low-order coefficients can differ, plus19 primary records. Raw initializer values were preserved.
- STB 2.6 adds descending sort, xy/CCT and xyY/XYZ conversions, primary lookup and custom-primary matrix builders with native determinant masks, bounds traps and output widths.

### F012: `core/color_primaries_tables.h`

- **F012.H01** (native-library arithmetic, constants and added APIs): The full RGB/YUV paths use different coefficient sources and signed/unsigned division paths; their ordinary coefficient differences cannot be explained by kernel changes. Narrowing keeps distinct unusual-depth policies and native packing.2.6 adds descending sort, xy/CCT, xyY/XYZ, primary lookup and custom-primary matrix entry points. Scale26 literals now match. Local fallback signed/unsigned constant spellings are representations of the same raw values, not new coefficient differences.

Retained details:

- The independent temporary[5]/temporary[6] stores and scale 26 literal now use matching presentation. Other RGB/YUV full matrices retain their original signed/unsigned division paths and coefficient sources.
- STB 2.4 reads a seven-definition RGB-to-XYZ table; STB 2.6 has separate Kr/Kg/Kb constants whose low-order coefficients can differ, plus19 primary records. Raw initializer values were preserved.
- STB 2.6 adds descending sort, xy/CCT and xyY/XYZ conversions, primary lookup and custom-primary matrix builders with native determinant masks, bounds traps and output widths.

### F013: `core/composer_contract.h`

- **F013.H01** (library behavior and layout): Only base 176/200, enclosing extent 900/968 and residual-derived/constant-zero composer mode differ. Shared packing does not conceal a second algorithm.

Retained details:

- Shared composer packing keeps a constant-zero unsigned-short result and pointer/dimension arguments. The native composer base/buffer extent are 176/900 bytes in 2.4 versus 200/968 in 2.6. Mode is residual_flag!=0 versus constant zero. Width/height originate from 2.4 source+12/+16 versus 2.6 source+0/+4. STB 2.6 can change selected 8-bit BL/EL configuration and coefficients to 10 bits in place before packing. Interleaved loads/stores remain valid for its overlapping views; 2.4 requires disjoint objects.

### F014: `core/control_abi_recovered.h`

- **F014.H01** (library layout): The removed header specifies the actual singleton context/configuration/kernel/manager/extension/source/graphics/target layouts. The replacement multi-input layouts have different extents, members and native aliasing; retaining the 2.4 declarations as 2.6 object views would be false.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- The 2.4 manager has 32 request slots and signed unchecked indexing after its diagnostic callback and sizes a singleton arena. The 2.6 manager has 24 slots, reloads its count between selected writes, returns -1 at the exact threshold and traps for further/native unsigned bounds, retains partial writes, and sizes per-instance kernel/extensions. InitMmg returns the manager pointer in both native instruction streams and now has one pointer-return declaration; the prior 2.6 void spelling was a decompiler choice, not a native ABI difference. RegMemReq uses a common unsigned in-device word: each native implementation merely stores those 32 bits and maintained callers pass boolean values.
- STB 2.4 initializes pending/active DM pointers, its 168-byte tone-control fields and trim arrays with retained self-pointers/defaults. STB 2.6 initializes native multi-signal/active kernel state, a larger tone-control record and extension descriptors. InitMdsExt now has the same pointer declaration; 2.6 intentionally ignores configuration while 2.4 reads it. Counts, optional writes, identity LUT initialization and fatal paths are retained.
- The 2.4 control representation has a 71-byte prefix and fixed 261-byte extension slots in current/previous 66626-byte backing, optional temporal level-1 filtering, logger failure for length>256 and native HDMI trim/removal/compaction. The 2.6 outer control path consumes contiguous variable-length records in 0x1000-byte slots, retaining native signed offsets, incomplete-header reuse, per-source progress and fatal capacity checks after partial writes. It has two metadata/composer staging slots despite three private host input records.
- The 2.4 configuration getter selects pending or active validity and its stored pointer; GetHDmKs reads the singleton pointer. The 2.6 configuration getter returns context+8 when nonnull, kernel access uses the native signed index-minus-one slot, and run/signal/TMM getters select native instance records with their original failure/trap behavior. Get_dm_ctx_buf is also a native distinct pointer calculation/load. Kernel-buffer publication forwards the common unsigned byte count; only 2.6 accepts an instance index and traps at index >=4.
- The 2.4 graphics path derives a separate 64-byte graphics descriptor from CLI fields and applies stored luminance offsets. The 2.6 path uses native input_mode 2, format defaults/primaries, graphic luminance words and a separate Core 2 slot. Native gamma, matrix, source/default field writes and partial-state order remain version-specific.

### F015: `core/control_cli_contract.h`

- **F015.H01** (library layout and added controls): Priority moves from byte 52 to 0x1d14; 2.6 adds selected input count/primary and level-11 fields. Unknown init/input/override fields retain honest reserved names. The bytewise access implementation is shared.

Retained details:

- Both modules use bytewise CLI priority accessors. Priority is byte 52 in the 112-byte 2.4 record versus byte 0x1d14 in the 7468-byte 2.6 record. The 2.6 tail additionally contains selected input count/primary, native reserved init/input flags and level-11 controls. Unknown priority-override/init/input fields remain explicitly reserved; no speculative whole-struct overlay was introduced.

### F016: `core/control_graphics.c`

- **F016.H01** (library behavior): The 2.4 graphics converter fills the separate 64-byte descriptor from CLI graphics fields, selecting native gamma/EOTF/range/default values. The 2.6 input-mode-2 preparation is a different source/graphics path, not an equivalent renamed helper.

Retained details:

- The 2.4 graphics path derives a separate 64-byte graphics descriptor from CLI fields and applies stored luminance offsets. The 2.6 path uses native input_mode 2, format defaults/primaries, graphic luminance words and a separate Core 2 slot. Native gamma, matrix, source/default field writes and partial-state order remain version-specific.

### F017: `core/control_memory.c`

- **F017.H01** (library behavior and layout): Manager initialization keeps the common pointer result but writes the distinct native context/extension/kernel sizes. Request registration retains 32 signed unchecked slots versus 24 slots with count reloads, exact -1 threshold, native traps and partial-store order; 2.6 also owns its display-index/default records.
- **F017.H02** (library behavior and layout): STB 2.6 adds its OS-backed MemZeroByte and compact row-pitch updater. Packed pitch uses the native component count; planar gray omits chroma, while signed half-dimensions precede chroma products. The adjacent common minimum-size arithmetic remains wrapped u32 with signed quarter division.
- **F017.H03** (library behavior and layout): The only common minimum-size algorithm change is native 2.6 planar-gray chroma omission. Added metadata/frame synchronization, run-mode/active-kernel indexing, display pointer initialization and per-instance arena sizing preserve native slot arithmetic, null/trap behavior and store phases; singleton configuration/kernel getters select different native pointers.

Retained details:

- The 2.4 manager has 32 request slots and signed unchecked indexing after its diagnostic callback and sizes a singleton arena. The 2.6 manager has 24 slots, reloads its count between selected writes, returns -1 at the exact threshold and traps for further/native unsigned bounds, retains partial writes, and sizes per-instance kernel/extensions. InitMmg returns the manager pointer in both native instruction streams and now has one pointer-return declaration; the prior 2.6 void spelling was a decompiler choice, not a native ABI difference. RegMemReq uses a common unsigned in-device word: each native implementation merely stores those 32 bits and maintained callers pass boolean values.
- The 2.4 configuration getter selects pending or active validity and its stored pointer; GetHDmKs reads the singleton pointer. The 2.6 configuration getter returns context+8 when nonnull, kernel access uses the native signed index-minus-one slot, and run/signal/TMM getters select native instance records with their original failure/trap behavior. Get_dm_ctx_buf is also a native distinct pointer calculation/load. Kernel-buffer publication forwards the common unsigned byte count; only 2.6 accepts an instance index and traps at index >=4.
- Native gray color is 4 in 2.4 versus 5 in 2.6; planar 2.4 sizing includes chroma while 2.6 gray omits it. Minimum sizing now uses one unsigned-word argument/result declaration and a common wrapped-product/signed-quarter-division expression. The native difference is only the gray omission condition, not a decompiler signedness artifact. Compact row-pitch sizing still halves each signed dimension before its products, unlike minimum sizing.
- STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.

### F018: `core/control_memory.h`

- **F018.H01** (library API): Common pointer return and unsigned request/frame-size words are preserved. Native count/index arity and 2.6 clear/run/row-pitch/helper APIs remain distinct.

Retained details:

- The 2.4 manager has 32 request slots and signed unchecked indexing after its diagnostic callback and sizes a singleton arena. The 2.6 manager has 24 slots, reloads its count between selected writes, returns -1 at the exact threshold and traps for further/native unsigned bounds, retains partial writes, and sizes per-instance kernel/extensions. InitMmg returns the manager pointer in both native instruction streams and now has one pointer-return declaration; the prior 2.6 void spelling was a decompiler choice, not a native ABI difference. RegMemReq uses a common unsigned in-device word: each native implementation merely stores those 32 bits and maintained callers pass boolean values.
- The 2.4 configuration getter selects pending or active validity and its stored pointer; GetHDmKs reads the singleton pointer. The 2.6 configuration getter returns context+8 when nonnull, kernel access uses the native signed index-minus-one slot, and run/signal/TMM getters select native instance records with their original failure/trap behavior. Get_dm_ctx_buf is also a native distinct pointer calculation/load. Kernel-buffer publication forwards the common unsigned byte count; only 2.6 accepts an instance index and traps at index >=4.
- Native gray color is 4 in 2.4 versus 5 in 2.6; planar 2.4 sizing includes chroma while 2.6 gray omits it. Minimum sizing now uses one unsigned-word argument/result declaration and a common wrapped-product/signed-quarter-division expression. The native difference is only the gray omission condition, not a decompiler signedness artifact. Compact row-pitch sizing still halves each signed dimension before its products, unlike minimum sizing.

### F019: `core/control_metadata.c`

- **F019.H01** (library behavior): The removed 2.4 fixed-261-slot reader/filter and native HDMI trim/removal/compaction operate on 66626-byte current/previous metadata. The 2.6 path consumes variable records and obtains formatted metadata through its multi-signal DM coordinator.

Retained details:

- The 2.4 control representation has a 71-byte prefix and fixed 261-byte extension slots in current/previous 66626-byte backing, optional temporal level-1 filtering, logger failure for length>256 and native HDMI trim/removal/compaction. The 2.6 outer control path consumes contiguous variable-length records in 0x1000-byte slots, retaining native signed offsets, incomplete-header reuse, per-source progress and fatal capacity checks after partial writes. It has two metadata/composer staging slots despite three private host input records.
- The 2.4 raw HDMI publisher calculates size before payload writes, caps >512 words, logs and preserves the previous payload after availability is cleared. The 2.6 publisher retains native raw-word and extension writes interleaved with fatal capacity checks, publishes transport control data and content information, and preserves register/LUT slot selection. Core register/host field offsets and buffer extents differ. HDR publication now uses four explicit LE64 copies at offsets 0,8,16,21, retaining the final overlapping store.

### F020: `core/control_metadata.h`

- **F020.H01** (library API and backing contract): Keep 2.4 logger and fixed-record/temporal-filter/HDMI preparation contracts; these functions are not the 2.6 variable-slot ingress API.

Retained details:

- The 2.4 control representation has a 71-byte prefix and fixed 261-byte extension slots in current/previous 66626-byte backing, optional temporal level-1 filtering, logger failure for length>256 and native HDMI trim/removal/compaction. The 2.6 outer control path consumes contiguous variable-length records in 0x1000-byte slots, retaining native signed offsets, incomplete-header reuse, per-source progress and fatal capacity checks after partial writes. It has two metadata/composer staging slots despite three private host input records.

### F021: `core/control_orchestration_bindings.h`

- **F021.H01** (library layout and adapters): Active storage bindings name the native multi-input control/source/register/metadata owners and exact lvalue widths. Added transport/control APIs and high-product/packed-word expressions support new native code. Seven unconsumed CONCAT definitions and twelve simple function/callback-owner aliases have been removed; private organization alone does not justify them. The two function-like cast adapters retain their actual casts.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.
- Both modules use bytewise CLI priority accessors. Priority is byte 52 in the 112-byte 2.4 record versus byte 0x1d14 in the 7468-byte 2.6 record. The 2.6 tail additionally contains selected input count/primary, native reserved init/input flags and level-11 controls. Unknown priority-override/init/input fields remain explicitly reserved; no speculative whole-struct overlay was introduced.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.

### F022: `core/control_packets.c`

- **F022.H01** (library behavior and API): Retain target-derived HDR primaries and last-L6 selection, optional expanded output records, VSEM CRC/header handling, source/target defaults, control/VSIF and L11 content generation, indexed kernel dump and multi-kernel core3 packing. The 2.4 fixed metadata/pending-active getters are native different paths. Shared byte-count types and header spelling remain normalized.

Retained details:

- The 2.4 infoframe builder uses its input-format/HDR10 arguments and fixed primaries. The 2.6 builder derives primaries from fixed-point target words and its publisher applies native primary-input HDR10 passthrough/debug selection. For level-6 extensions, 2.4 processes matching records in order whereas 2.6 selects the last record first. Optional output-format getters expand from required 16-byte records to native PQ/luminance/primaries records in 2.6.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.
- The 2.4 configuration getter selects pending or active validity and its stored pointer; GetHDmKs reads the singleton pointer. The 2.6 configuration getter returns context+8 when nonnull, kernel access uses the native signed index-minus-one slot, and run/signal/TMM getters select native instance records with their original failure/trap behavior. Get_dm_ctx_buf is also a native distinct pointer calculation/load. Kernel-buffer publication forwards the common unsigned byte count; only 2.6 accepts an instance index and traps at index >=4.
- The 2.4 graphics path derives a separate 64-byte graphics descriptor from CLI fields and applies stored luminance offsets. The 2.6 path uses native input_mode 2, format defaults/primaries, graphic luminance words and a separate Core 2 slot. Native gamma, matrix, source/default field writes and partial-state order remain version-specific.
- The 2.4 entry writes an 88-byte target descriptor and returns -1 for null/invalid identification. The 2.6 helper writes its control-state capability fields, treats null as retained state/success and returns -8 for invalid identification/version/length. Each keeps its original capability/luminance partial writes. Luminance units and rounding differ; in particular 2.6 v1 squares/left-shifts a u32, reinterprets signed and divides toward zero. Original instructions at 0015c4b4..0015c4c4 confirm signed lowering. Proven equivalent packed-primary expressions now use the clearer common spelling.

### F023: `core/control_packets.h`

- **F023.H01** (library API): Different infoframe inputs, optional output-format buffers, indexed dump and added VSEM/VSIF/control/content interfaces match actual implementations. Common unsigned dump result is retained.

Retained details:

- The 2.4 infoframe builder uses its input-format/HDR10 arguments and fixed primaries. The 2.6 builder derives primaries from fixed-point target words and its publisher applies native primary-input HDR10 passthrough/debug selection. For level-6 extensions, 2.4 processes matching records in order whereas 2.6 selects the last record first. Optional output-format getters expand from required 16-byte records to native PQ/luminance/primaries records in 2.6.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.
- The 2.4 configuration getter selects pending or active validity and its stored pointer; GetHDmKs reads the singleton pointer. The 2.6 configuration getter returns context+8 when nonnull, kernel access uses the native signed index-minus-one slot, and run/signal/TMM getters select native instance records with their original failure/trap behavior. Get_dm_ctx_buf is also a native distinct pointer calculation/load. Kernel-buffer publication forwards the common unsigned byte count; only 2.6 accepts an instance index and traps at index >=4.

### F024: `core/control_path.c`

- **F024.H01** (library behavior and host record layout): One 17-argument source plus separate graphics/target and fixed-stride metadata is replaced by primary selection, video/graphics modes, three host inputs, two metadata/composer staging slots, transport data, variable-record progress and fatal checks after partial writes. Zero-count reset differs from FORMAT_INVALID reset. These are actual state-machine changes, not a helper-organization excuse.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- STB 2.4 resets on input format -1 and calls its no-op destroy helper on ordinary completion without clearing initialized. STB 2.6 resets on zero input count, clears native staging and initialization/content bytes, and retains shared control state across ordinary calls. Init_cp takes instance/configuration inputs, rejects counts >=5 and retains native zero/negative-count behavior. The destroy helper now has the same no-argument declaration and zero-result body; its old 2.4 arena argument was ignored by the native instructions.
- The 2.4 control representation has a 71-byte prefix and fixed 261-byte extension slots in current/previous 66626-byte backing, optional temporal level-1 filtering, logger failure for length>256 and native HDMI trim/removal/compaction. The 2.6 outer control path consumes contiguous variable-length records in 0x1000-byte slots, retaining native signed offsets, incomplete-header reuse, per-source progress and fatal capacity checks after partial writes. It has two metadata/composer staging slots despite three private host input records.
- Both modules use bytewise CLI priority accessors. Priority is byte 52 in the 112-byte 2.4 record versus byte 0x1d14 in the 7468-byte 2.6 record. The 2.6 tail additionally contains selected input count/primary, native reserved init/input flags and level-11 controls. Unknown priority-override/init/input fields remain explicitly reserved; no speculative whole-struct overlay was introduced.
- The 2.4 graphics path derives a separate 64-byte graphics descriptor from CLI fields and applies stored luminance offsets. The 2.6 path uses native input_mode 2, format defaults/primaries, graphic luminance words and a separate Core 2 slot. Native gamma, matrix, source/default field writes and partial-state order remain version-specific.
- The 2.4 raw HDMI publisher calculates size before payload writes, caps >512 words, logs and preserves the previous payload after availability is cleared. The 2.6 publisher retains native raw-word and extension writes interleaved with fatal capacity checks, publishes transport control data and content information, and preserves register/LUT slot selection. Core register/host field offsets and buffer extents differ. HDR publication now uses four explicit LE64 copies at offsets 0,8,16,21, retaining the final overlapping store.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.

### F025: `core/control_path.h`

- **F025.H01** (library API and host compatibility): Explicit singleton storage and 17 control arguments differ from a 0x4f58 multi-input host settings record. Both preserve persistent serialized control ownership.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- STB 2.4 resets on input format -1 and calls its no-op destroy helper on ordinary completion without clearing initialized. STB 2.6 resets on zero input count, clears native staging and initialization/content bytes, and retains shared control state across ordinary calls. Init_cp takes instance/configuration inputs, rejects counts >=5 and retains native zero/negative-count behavior. The destroy helper now has the same no-argument declaration and zero-result body; its old 2.4 arena argument was ignored by the native instructions.

### F026: `core/control_publish.c`

- **F026.H01** (library behavior and host output layout): Keep native register/LUT bank/slot selection and interleaved reads/stores, 2.4 preflight raw-metadata cap/log/retain policy versus 2.6 checks after partial output, expanded transport/content outputs, and HDR primary-input passthrough/debug selection. The four LE64 HDR copies preserve the final overlapping store.

Retained details:

- The 2.4 raw HDMI publisher calculates size before payload writes, caps >512 words, logs and preserves the previous payload after availability is cleared. The 2.6 publisher retains native raw-word and extension writes interleaved with fatal capacity checks, publishes transport control data and content information, and preserves register/LUT slot selection. Core register/host field offsets and buffer extents differ. HDR publication now uses four explicit LE64 copies at offsets 0,8,16,21, retaining the final overlapping store.
- The 2.4 infoframe builder uses its input-format/HDR10 arguments and fixed primaries. The 2.6 builder derives primaries from fixed-point target words and its publisher applies native primary-input HDR10 passthrough/debug selection. For level-6 extensions, 2.4 processes matching records in order whereas 2.6 selects the last record first. Optional output-format getters expand from required 16-byte records to native PQ/luminance/primaries records in 2.6.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.

### F027: `core/control_publish.h`

- **F027.H01** (library API and host output layout): 2.6 adds separate register/HDR/content publication endpoints for multi settings; byte offsets, overlapping phases and callback arguments are not interchangeable with the 2.4 packed publisher.

Retained details:

- The 2.4 raw HDMI publisher calculates size before payload writes, caps >512 words, logs and preserves the previous payload after availability is cleared. The 2.6 publisher retains native raw-word and extension writes interleaved with fatal capacity checks, publishes transport control data and content information, and preserves register/LUT slot selection. Core register/host field offsets and buffer extents differ. HDR publication now uses four explicit LE64 copies at offsets 0,8,16,21, retaining the final overlapping store.
- The 2.4 infoframe builder uses its input-format/HDR10 arguments and fixed primaries. The 2.6 builder derives primaries from fixed-point target words and its publisher applies native primary-input HDR10 passthrough/debug selection. For level-6 extensions, 2.4 processes matching records in order whereas 2.6 selects the last record first. Optional output-format getters expand from required 16-byte records to native PQ/luminance/primaries records in 2.6.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.

### F028: `core/control_runtime.c`

- **F028.H01** (library lifecycle and storage ownership): 2.4 requires separately aligned singleton current/previous metadata, composition, register and arena owners plus validated copied services. 2.6 has different-sized canonical multi-input state and dynamically allocated parser instances; overlaying the singleton runtime would change ownership/lifetime.

Retained details:

- STB 2.4 resets on input format -1 and calls its no-op destroy helper on ordinary completion without clearing initialized. STB 2.6 resets on zero input count, clears native staging and initialization/content bytes, and retains shared control state across ordinary calls. Init_cp takes instance/configuration inputs, rejects counts >=5 and retains native zero/negative-count behavior. The destroy helper now has the same no-argument declaration and zero-result body; its old 2.4 arena argument was ignored by the native instructions.
- STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.

### F029: `core/control_runtime.h`

- **F029.H01** (library lifecycle and services): Explicit singleton storage/service setup and its 17-argument processing surface are required by the 2.4 host adaptation. 2.6 binds its actual module/control service objects instead.

Retained details:

- STB 2.4 resets on input format -1 and calls its no-op destroy helper on ordinary completion without clearing initialized. STB 2.6 resets on zero input count, clears native staging and initialization/content bytes, and retains shared control state across ordinary calls. Init_cp takes instance/configuration inputs, rejects counts >=5 and retains native zero/negative-count behavior. The destroy helper now has the same no-argument declaration and zero-result body; its old 2.4 arena argument was ignored by the native instructions.
- STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.

### F030: `core/control_setting_layout.h`

- **F030.H01** (host compatibility and library layout): The 13468-byte legacy core prefix within a 13480-byte host output differs from the 0x4f58 multi settings record, three 0x68 input strides and two Core1 result banks. Packed HDR scratch assertions describe the actual byte layout, not a speculative global struct overlay.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- The 2.4 raw HDMI publisher calculates size before payload writes, caps >512 words, logs and preserves the previous payload after availability is cleared. The 2.6 publisher retains native raw-word and extension writes interleaved with fatal capacity checks, publishes transport control data and content information, and preserves register/LUT slot selection. Core register/host field offsets and buffer extents differ. HDR publication now uses four explicit LE64 copies at offsets 0,8,16,21, retaining the final overlapping store.

### F031: `core/control_state.c`

- **F031.H01** (library behavior and API): Multi-input InitDm initializes native indexed signal/kernel state, larger tone defaults, extension descriptors, identity LUT and self-pointers with original count/partial-write/trap behavior. These replace singleton pending/active configuration and trim initialization; 2.6 also adds indexed TMM access and persistent content callback storage.
- **F031.H02** (library behavior and API): Preserve mapping flag offsets, added force-CLL/L11/peak-stretch controls, indexed kernel/context getters and instance/configuration-driven initialization. The removed singleton support-EL/configuration/PQ getters have different state selection; no-op destroy is shared and argument-free.

Retained details:

- STB 2.4 resets on input format -1 and calls its no-op destroy helper on ordinary completion without clearing initialized. STB 2.6 resets on zero input count, clears native staging and initialization/content bytes, and retains shared control state across ordinary calls. Init_cp takes instance/configuration inputs, rejects counts >=5 and retains native zero/negative-count behavior. The destroy helper now has the same no-argument declaration and zero-result body; its old 2.4 arena argument was ignored by the native instructions.
- STB 2.4 initializes pending/active DM pointers, its 168-byte tone-control fields and trim arrays with retained self-pointers/defaults. STB 2.6 initializes native multi-signal/active kernel state, a larger tone-control record and extension descriptors. InitMdsExt now has the same pointer declaration; 2.6 intentionally ignores configuration while 2.4 reads it. Counts, optional writes, identity LUT initialization and fatal paths are retained.
- The 2.4 configuration getter selects pending or active validity and its stored pointer; GetHDmKs reads the singleton pointer. The 2.6 configuration getter returns context+8 when nonnull, kernel access uses the native signed index-minus-one slot, and run/signal/TMM getters select native instance records with their original failure/trap behavior. Get_dm_ctx_buf is also a native distinct pointer calculation/load. Kernel-buffer publication forwards the common unsigned byte count; only 2.6 accepts an instance index and traps at index >=4.
- STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.

### F032: `core/control_state.h`

- **F032.H01** (library API and backing contract): Distinct initializer/count/index/callback APIs and buffer extents are real contracts. No-op destroy is common and argument-free; InitMdsExt pointer declarations are common.

Retained details:

- STB 2.4 resets on input format -1 and calls its no-op destroy helper on ordinary completion without clearing initialized. STB 2.6 resets on zero input count, clears native staging and initialization/content bytes, and retains shared control state across ordinary calls. Init_cp takes instance/configuration inputs, rejects counts >=5 and retains native zero/negative-count behavior. The destroy helper now has the same no-argument declaration and zero-result body; its old 2.4 arena argument was ignored by the native instructions.
- STB 2.4 initializes pending/active DM pointers, its 168-byte tone-control fields and trim arrays with retained self-pointers/defaults. STB 2.6 initializes native multi-signal/active kernel state, a larger tone-control record and extension descriptors. InitMdsExt now has the same pointer declaration; 2.6 intentionally ignores configuration while 2.4 reads it. Counts, optional writes, identity LUT initialization and fatal paths are retained.
- The 2.4 configuration getter selects pending or active validity and its stored pointer; GetHDmKs reads the singleton pointer. The 2.6 configuration getter returns context+8 when nonnull, kernel access uses the native signed index-minus-one slot, and run/signal/TMM getters select native instance records with their original failure/trap behavior. Get_dm_ctx_buf is also a native distinct pointer calculation/load. Kernel-buffer publication forwards the common unsigned byte count; only 2.6 accepts an instance index and traps at index >=4.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.

### F033: `core/control_target.c`

- **F033.H01** (library behavior and units): 2.4 decodes an 88-byte descriptor with -1/null rejection and decimal luminance. 2.6 decodes into persistent capabilities with null-retain success, -8 rejection, fixed-point luminance/signed wrapping and native target/LL/RGB mode setup. Proven same packed-primary expressions already agree.

Retained details:

- The 2.4 entry writes an 88-byte target descriptor and returns -1 for null/invalid identification. The 2.6 helper writes its control-state capability fields, treats null as retained state/success and returns -8 for invalid identification/version/length. Each keeps its original capability/luminance partial writes. Luminance units and rounding differ; in particular 2.6 v1 squares/left-shifts a u32, reinterprets signed and divides toward zero. Original instructions at 0015c4b4..0015c4c4 confirm signed lowering. Proven equivalent packed-primary expressions now use the clearer common spelling.
- The 2.4 graphics path derives a separate 64-byte graphics descriptor from CLI fields and applies stored luminance offsets. The 2.6 path uses native input_mode 2, format defaults/primaries, graphic luminance words and a separate Core 2 slot. Native gamma, matrix, source/default field writes and partial-state order remain version-specific.
- The 2.4 infoframe builder uses its input-format/HDR10 arguments and fixed primaries. The 2.6 builder derives primaries from fixed-point target words and its publisher applies native primary-input HDR10 passthrough/debug selection. For level-6 extensions, 2.4 processes matching records in order whereas 2.6 selects the last record first. Optional output-format getters expand from required 16-byte records to native PQ/luminance/primaries records in 2.6.

### F034: `core/control_target.h`

- **F034.H01** (library API and backing contract): The standalone target descriptor conversion and persistent capability/output-target setup are different native entries with distinct destinations/error/null behavior.

Retained details:

- The 2.4 entry writes an 88-byte target descriptor and returns -1 for null/invalid identification. The 2.6 helper writes its control-state capability fields, treats null as retained state/success and returns -8 for invalid identification/version/length. Each keeps its original capability/luminance partial writes. Luminance units and rounding differ; in particular 2.6 v1 squares/left-shifts a u32, reinterprets signed and divides toward zero. Original instructions at 0015c4b4..0015c4c4 confirm signed lowering. Proven equivalent packed-primary expressions now use the clearer common spelling.

### F035: `core/control_workspace.h`

- **F035.H01** (library aliasing and source/packet workspace): The multi-input commit path needs metadata/primaries overlay, decoded headers, four source flags/sizes and overlapping HDR scratch. Size/offset assertions preserve the audited stack views and phases; this does not authorize reordering or blanket struct overlays.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- The 2.4 control representation has a 71-byte prefix and fixed 261-byte extension slots in current/previous 66626-byte backing, optional temporal level-1 filtering, logger failure for length>256 and native HDMI trim/removal/compaction. The 2.6 outer control path consumes contiguous variable-length records in 0x1000-byte slots, retaining native signed offsets, incomplete-header reuse, per-source progress and fatal capacity checks after partial writes. It has two metadata/composer staging slots despite three private host input records.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.

### F036: `core/data_layout.h`

- **F036.H01** (library storage and aliasing): Typed 210856-byte multi-input control storage, signature storage and native table ownership require the specified contiguous spans and reserved mirrors; compile-time assertions pin consumers. Singleton objects have a different layout and ownership contract.

Retained details:

- STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.
- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- Both modules use bytewise CLI priority accessors. Priority is byte 52 in the 112-byte 2.4 record versus byte 0x1d14 in the 7468-byte 2.6 record. The 2.6 tail additionally contains selected input count/primary, native reserved init/input flags and level-11 controls. Unknown priority-override/init/input fields remain explicitly reserved; no speculative whole-struct overlay was introduced.

### F037: `core/detect_mel.c`

- **F037.H01** (library entry visibility): P13: same shared predicate body, different native linkage; no change to MEL arithmetic.

Retained details:

- P13

### F038: `core/dm_backlight.c`

- **F038.H01** (native-library addition): 2.6 adds adaptive-backlight filtering, active-state/window restoration, asymmetric temporal weights and thresholds, target range publication and global-dimming getters. No2.4 same-signature entry is being rewritten. Getter null-output tolerance and zero/zero-extended0xffffffff results are explicit contracts.

Retained details:

- ApplyAblu selects indexed or primary state, restores target windows when inactive, derives filtered PQ/linear anchors, clamps model inputs, applies asymmetric temporal weights and frame thresholds, and updates target/output ranges.
- Result getters require an active state, permit null output pointers, return zero or zero-extended 0xffffffff, and expose filtered anchors/means, target window and backlight model input/output. No STB 2.4 standalone counterparts exist.

### F039: `core/dm_bindings.h`

- **F039.H01** (library dependencies and data-address/service bindings): The2.6 indexed/output/trim/dimming entries need the additional declarations and aggregate constant members; memset/printk/dump_stack use the control service table. The2.4-only dm_pair is a local packed-halfword construction used by its different singleton commits. No removed function alias represented a functional vendor delta.

Retained details:

- Both versions call the actual public function names directly. The remaining include sets select singleton versus indexed consolidation/metadata/trim/backlight APIs; only the genuine kernel-service and immutable-data owner bindings remain indirect.

### F040: `core/dm_commit_pipeline.c`

- **F040.H01** (library orchestration and state transitions): 2.4 selects forced/current metadata, commits pending color/graphics/video/tone/chroma state and swaps configurations.2.6 distinguishes null and all-ones metadata,700/120-byte classifiers, per-input mode/flags and metadata state; coordinates consolidation, maps, tone/inverse-curve compaction and dimming, then conditionally publishes700-byte metadata. Native return bits, early exits, diagnostics and partial state order differ.

Retained details:

- 2.4 CommitMds selects forced/current metadata, commits pending color/graphics/video/tone/chroma state, swaps active/pending configuration pointers, and dispatches dynamic metadata.
- 2.6 Commit2DmKs treats null metadata and the all-ones metadata sentinel differently, classifies700-byte records and 120-byte prefixes, maintains per-input flags and primary-mode timing, consolidates input, chooses bypass or concrete maps/tone curves, and finalizes stored metadata.
- 2.6 preserves diagnostics, temporal transitions, inverse-map de-duplication, backlight dispatch, native return bits and partial state updates; a whole-body unification would change sequencing.

### F041: `core/dm_commit_tables.inc`

- **F041.H01** (library data-address contract; equal values identified): The shared SDR and Pulsar gamma initializer files are byte-identical across lanes: this hunk does not establish a numerical gamma delta. The2.4 private15-tap filter is11,0,47,0,-262,0,1228,2048,1228,0,-262,0,47,0,11. Separate private/aggregate objects preserve the native pointer-producing bindings; owner placement is representation, not a changed coefficient algorithm or kernel semantic.

Retained details:

- The removed STB 2.4 include owns its native private filter/basis constants and gamma bindings; STB 2.6 consumes its aggregate native constant owner for the added control/display-management paths. The SDR and Pulsar gamma values are byte-identical and are shared includes, not a coefficient update. Native raw initializer values are preserved.

### F042: `core/dm_configuration.c`

- **F042.H01** (library configuration layouts/defaults/getters): 2.6 uses0x120 signal strides,0x34 alpha records,0x64 control,0x1b0 tone controls, five full signal records plus alpha storage and the separate legacy index5 byte view. Preserve clearing extents, retained pitches, primaries calls, manager writes, repeated assignments, null failures and nonnull out-of-range traps;2.4 getters select pending before active, including null pending.

Retained details:

- 2.4 initializes separate source/graphics/target layouts with sparse writes, retained Chrm/pitches and active-or-pending configuration getters. 2.6 initializes indexed 0x120-byte signals,0x34-byte alpha records,0x64-byte control and 0x1b0-byte tone control in the 0x8e4 configuration prefix.
- 2.6 adds global-dimming defaults and direct context getters: six-stride legacy signal views, null-context failure, fatal nonnull out-of-range signal access, and zero-extended 0xffffffff status. Its five full signals plus alpha region remain distinct from the legacy index5 byte view.
- Initializer clearing, primary index, transfer defaults, primaries callbacks, manager pointer storage and repeated writes retain their native order and untouched fields.

### F043: `core/dm_configuration.h`

- **F043.H01** (library API and object contracts): Argument counts/index roles, actual object layouts, copy extents and direct-versus-selected configuration behavior differ. GetTmCtrl/GetDmCtrl already share signed32 status spelling; config initialization returns its storage pointer and ignores its first2.6 LP64 argument. No recovered-width mismatch is used to justify an equal operation.

Retained details:

- 2.6 source initializers add a signal index; configuration initialization ignores its firstLP64 argument, accepts word-backed storage and returns its pointer. Getters use indexed or direct native context fields and LP64 zero-extended status values.
- 2.4 retains its typed DWARF-derived records and active/pending selection; these are different object layouts and operations, not merely alternate names for an identical structure.
- GetTmCtrl/GetDmCtrl now share signed 32 status-result spelling: both native entries return0 or zero-extended 0xffffffff. Only object/context parameter types and the different selection/copy contracts remain; their former64-bit result spelling was recovery churn.

### F044: `core/dm_extensions.c`

- **F044.H01** (added library entry): Append count0x108-byte records after the current byte count, then wrap the byte count. Preserve unchecked backing requirements and copy order; no allocation or new safety policy.

Retained details:

- The additional function copies count fixed0x108-byte records after the current byte count and then updates that byte count with native wrapping. No new allocation, capacity hardening or record conversion is introduced.

### F045: `core/dm_frame_format.c`

- **F045.H01** (library frame layout and partial writes): Gray encoding4/5 and planar-gray sizing differ; source field offsets and frame records differ.2.6 configuration conversion writes geometry/location/pitches before the flags-bit1 early return, then updates sample type/weave/depth/column pitch. Metadata conversion gates on validity and preserves sample-width/weave mapping and reload-after-write alias behavior.2.4 additionally exposes compact/singleton/secondary-input updates.

Retained details:

- 2.4 includes compact frame sizing and singleton frame-state updates; native gray value is 4 and planar sizing retains chroma. 2.6 uses gray5 and its compact sizing omits planar-gray chroma.
- CommitCdm2KsFrmFmt source field offsets differ;2.6 writes geometry/location/pitches before testing flags bit 1, then updates type/weave/bit-depth/column pitch.2.4 has no partial-update flags argument and retains its own write order.
- 2.6 metadata-to-frame conversion uses the metadata-valid bit,8-bit-versus-wider sample sizing and native weave mapping. Reloads after writes preserve overlapping views.

### F046: `core/dm_frame_format.h`

- **F046.H01** (library enum value): Other shared component/sample helpers are identical; this is a library record encoding, not a kernel enum change.
- **F046.H02** (library frame API): Added flags and distinct record dependencies reflect the respective original entry sets and partial-write contracts described in F045.

Retained details:

- RGBA2,422chroma1, U8sample1 andF32sample2 agree; gray is 4 in 2.4 and 5 in 2.6.2.6's configuration frame commit adds flags and uses indexed signal layouts.

### F047: `core/dm_input_consolidation.c`

- **F047.H01** (native-library indexed color/tone/trim orchestration): 2.4 separately commits singleton LMS/IPT matrices, video/graphics tone controls, output-map/frame state and active metadata synchronization.2.6 instead consolidates an indexed signal with primary/current-mode selection and native metadata masks, level254 version/run-mode selection, active-area crop stores with aliased dimension reloads, user contrast/chroma/white-point adjustments, L9/L11 primaries and intended-CCT normalization, followed by tone-control preparation and two-pass L2/L8 luma/chroma interpolation/defaults. The source/target primary scaling, per-signal white-point state, Q12 trim weights, user/default clamps, anchor ordering, partial stores and early return for selector2 are actual library contracts. Canonical function names and role-name changes are nonfunctional; this whole new pipeline is not a renamed2.4 helper.

Retained details:

- 2.4 commits singleton external/default LMS/IPT matrices and configuration-derived video/graphics tone targets, including native external inverse-scale behavior and HLG endpoints.
- 2.6 selects indexed versus primary state; crops exclusive active-area endpoints with required dimension reloads; clamps contrast/chroma/saturation/luminance adjustments and signed white-point offsets; selects or constructs primary matrices and color-temperature transforms.
- 2.6 coordinates level 2/8 trim interpolation, local-ms adjustments and metadata/configuration fallbacks through overlapping typed scratch views, retaining input flags, native pointer selections, table bounds, rounded/scaled arithmetic and intermediate stores.
- The top-level tail selects level-254 mode/version from metadata+0x6f/+0x70 (version clamps value+2 to 4), and the +0xd8 selector from metadata+0x71, configuration or primary state. Selector2 returns after active-area consolidation; signed control word+0xdc>127 clears flags0x210. Tone-control preparation precedes trim consolidation.

### F048: `core/dm_input_map.c`

- **F048.H01** (native-library layout and sequencing): 2.4 commits singleton source/graphics/alpha fields and typed metadata/color maps.2.6 separates indexed metadata/signal/IPT-ICtCp stages, uses per-signal white-point state, adds HLG and DM3 bypass, and writes different map/workspace records. Gamma presets, explicit matrices, offsets, source/graphics dispatch, native partial writes and cached/reloaded output owners remain per-version. Function-alias spelling is now canonical and is not a numerical change.

Retained details:

- STB 2.4 commits singleton graphics/video metadata, their color matrices and offsets through native configuration/context fields. STB 2.6 splits metadata-map, signal-map and indexed IPT/ICtCp matrix preparation, with independent color/range bypass decisions and DM3 bypass builders.
- Map workspace sizes, matrix destinations, cached versus reloaded pointer paths, packed 16/32/64 accesses and partial writes are native layout/sequencing contracts; copying either whole body to the other would change the interface and output records.

### F049: `core/dm_input_map.h`

- **F049.H01** (library input-map API): The2.4 bit-depth/range map and typed singleton graphics/video commit interface has no same-signature counterpart. Signal index, color-space/range/bypass arguments and runtime/CSC owners in2.6 select actual additional operations; canonical function aliases were removed.

Retained details:

- STB 2.4 commits singleton graphics/video metadata, their color matrices and offsets through native configuration/context fields. STB 2.6 splits metadata-map, signal-map and indexed IPT/ICtCp matrix preparation, with independent color/range bypass decisions and DM3 bypass builders.
- Map workspace sizes, matrix destinations, cached versus reloaded pointer paths, packed 16/32/64 accesses and partial writes are native layout/sequencing contracts; copying either whole body to the other would change the interface and output records.

### F050: `core/dm_kernel_maps.c`

- **F050.H01** (library kernel-state initialization/classification): InitDmKs requests0x32a0000/0xca80000 and row pitch0x1000/0x2000 with different manager/config/kernel fields. Shared CheckMds control shape compares520/514 versus700/120 and returns0/1/2 versus0/4/1, proved against each original. 'Kernel state' here is the library's register/frame record, not changed Linux kernel semantics.
- **F050.H02** (library indexed commit/finalization): 2.4 reads the pending pointer before its nominal null check and copies984 bytes.2.6 selects indexed kernel state, updates dimming weights/timing and bounded square-root result, uses frame planes+0x4380000/+0x8700000, commits input/alpha/output frames and the output-map pointer; finalization writes first/primary/last bits and0x8f8-byte config snapshot. Only demonstrated same-type/duplicate casts were removed; real conversions and stores remain.

Retained details:

- InitDmKs changes row-pitch 0x1000->0x2000, allocation 0x32a0000->0xca80000 and manager/config offsets, stores the 2.6 context handle, and initializes different native kernel maxima/storage.
- CheckMds compares 520/514 bytes and returns 0/1/2 in 2.4;2.6 compares 700/120 and returns 0/4/1. These changes are confirmed against both original binaries.
- 2.4 CommitCfg reads its pending pointer before the nominal null check, copies 984-byte pending configuration, and updates singleton frame addresses.2.6 indexes kernel/metadata state, initializes global dimming/filter timing and integer square root, writes frame-buffer planes at+0x4380000/+0x8700000, commits indexed/alpha/target frames and linked output-map pointers.
- 2.6 CommitFinalized emits first/primary/last input bits, marks initialized, copies 0x8f8 bytes, stores the committed mode and conditionally disables dimming.

### F051: `core/dm_layout.h`

- **F051.H01** (library byte layouts and enums): Offsets, extents, mode/enumeration values and owner strides are actual different library records.2.6 metadata_format is the verified0x78-byte base view within raw700-byte metadata storage, not a claim that the base struct spans every extension. Reserved fields stay uninterpreted. Full sizes/offset assertions bind the declarations to the selected ABI.

Retained details:

- Configuration, context, kernel and metadata field offsets/sizes differ.2.6 adds indexed signal/context strides, primary-mode fields, five full signals plus alpha storage, level 8/9/10/11/254 fields and global-dimming results.
- Native MDS classifier extents/results are explicitly named in both versions; asserted sizes/offsets are retained. Unconfirmed regions keep reserved/offset names rather than borrowed 2.4 semantics.

### F052: `core/dm_map_workspace.h`

- **F052.H01** (storage requirements of added indexed algorithms): These typed unions support the actual additional indexed input/output/trim APIs:16/32/64-bit coefficients, primaries, offsets and bounded17-coordinate/16-index records. Assertions preserve offsets/extents and overlapping local stores. They add no persistent state or external kernel ABI and are not a separate vendor behavior change.

Retained details:

- These locals belong to the indexed 2.6 map/commit algorithms; typed unions retain aliasing and native extents rather than represent newly added persistent state or a kernel ABI.

### F053: `core/dm_metadata.c`

- **F053.H01** (library metadata decoding and checkpoints): 2.4 uses caller-owned trim arrays, signed remaining lengths and L1/L2/L3/L5/L255 handling.2.6 resets flags/scales and primary unknown cursors, adds L4/L6/L8/L9/L10/L11/L254, length-dependent L8 fields/defaults, custom primaries, per-record tail pointers/lengths and fatal array limits. Preserve counts, aliases/reloads, duplicate/order behavior, signed/wrapped size checks and partial writes. Three equivalent BE32 trees are now fixed_load_be32; focused original2.6 proof covers the changed compiled fallback.

Retained details:

- 2.4 decodes the 71-byte base into 520-byte typed metadata with caller-owned trim arrays, signed remaining lengths, level 1/2/3/5/255 handling and count/pointer reloads that can alias.
- 2.6 resets decoded flags/scales and primary-input unknown-record cursors before parsing, fills its700-byte indexed format, and handles level 1/2/3/4/5/6/8/9/10/11/254/255 with per-level capacity checks and fatal trim-array bounds.
- Level8 optional fields depend on declared lengths and local-ms defaults; level 9/10 custom primaries and unknown tails retain their separate pointer/length arrays. Counts, duplicate/order handling, skipped bytes, zero/negative/wrapped size predicates, partial updates and diagnostics remain native.

### F054: `core/dm_metadata_output.c`

- **F054.H01** (added library output-metadata algorithm): GetMdsOutput gates on initialized context, applies configured mode/capacity fallback and optional measurement. GenerateOutputMd preserves header matrix/EOTF/primaries derivation, L1/L2/L3/L4 constraints and trim filtering, L8 optional fields/tails, indexed/custom L9/L10, L11 source/override, L254 and aggregate unknown record gates. Signed capacity checkpoints, count publication, trap-after-prior-writes and context mutation before failure are part of the contract. L255 is decoded by dm_metadata.c, not a dedicated emitter here.

Retained details:

- GetMdsOutput requires initialized context, applies configured mode/capacity fallback and optional measure-before-write; GenerateOutputMd selects indexed/primary signal and source state, exclusions and target/signal versions.
- Fixed-header generation retains matrix/EOTF/primaries/native minimum-maximum derivation and state updates before capacity failure. Legacy level 1/2/3/4 emission includes target-version constraints and trim selection; identity-prefix initialization retains signed count and a fatal16-entry bound after the first 16 writes.
- Level8 uses indexed 13-byte core plus optional16-bit/byte fields and unknown tails; level 9 chooses indexed/custom primaries; level 10 emits indexed 6/22-byte records with unknown fields; L11 supports source/override fields; L254 and aggregate unknown records keep separate gates, payload lengths and count timing.
- Null-output measurement, staged endian writes, duplicate record selection, signed capacity behavior, extension-count increments, diagnostics and trap/partial-output boundaries are preserved.
- This output generator has dedicated emitters for levels1/2/3/4/8/9/10/11/254 and aggregate unknown records. Level255 decoding belongs to dm_metadata.c; it is not a dedicated output emitter here.

### F055: `core/dm_output_map.c`

- **F055.H01** (native-library layout and sequencing): The common constant-zero status is int in both versions, but2.4 config/update-target/context versus2.6 signal-index/config/context arguments select different owners.2.6 adds ICtCp modes, cross-talk reapplication, current-mode target-range decisions, UpdateKsOMapTmm and DM3 bypass; store widths/order and owner reloads are retained. Shared offset/range/matrix calls express the unchanged operations.

Retained details:

- STB 2.4 takes config/update-minmax/context for the singleton map; STB 2.6 selects a signal-indexed output owner and updates mode-specific target min/max, ICtCp IO matrices and cross-talk state.
- STB 2.6 adds UpdateKsOMapTmm and DM3 output bypass; native output formats, packed store widths and owner-pointer reloads remain distinct. Shared YUV range/offset and matrix primitives now carry equivalent suboperations.

### F056: `core/dm_output_map.h`

- **F056.H01** (library output-map API): 2.4 takes configuration/update-target-minmax/context for its singleton map;2.6 signal index replaces that role and selects a different output owner. Added TMM/DM3 operations and their buffers reflect actual distinct entry sets rather than helper organization.

Retained details:

- STB 2.4 takes config/update-minmax/context for the singleton map; STB 2.6 selects a signal-indexed output owner and updates mode-specific target min/max, ICtCp IO matrices and cross-talk state.
- STB 2.6 adds UpdateKsOMapTmm and DM3 output bypass; native output formats, packed store widths and owner-pointer reloads remain distinct. Shared YUV range/offset and matrix primitives now carry equivalent suboperations.

### F057: `core/dm_pipeline.h`

- **F057.H01** (library orchestration/API contracts): The signatures describe real different input counts, owner records, signal selectors, null/sentinel behavior and output contracts.2.4 ParseMds has signed length and no context argument;2.6 uses unsigned declared size and explicit context.2.6 adds output generation/dimming getters/finalization; private names and includes are implementation dependencies, not standalone behavioral deltas.

Retained details:

- Different argument counts, signal indices, sentinel/null handling, object extents and native scalar result widths remain explicit.2.6 adds output generators and global-dimming getters;2.4 retains its pending-pointer-before-null-check contract.

### F058: `core/dm_transfer.c`

- **F058.H01** (library transfer selection/result contract): DeGamma inputs/calculation/tables are common: four low16 inputs plus full32 overflow scale; only low32 versus full64 returned product differs.2.6 primary/indexed selection follows current/committed mode with zero-index normalization and reachable bounds checks, and adds EOTF propagation;2.4 retains separate graphics selection. Removing the private primary forwarding wrapper is representation cleanup with identical object code.

Retained details:

- 2.4 reads singleton tone-direction state and separately selects graphics configuration.2.6 selects primary/indexed context by committed/current mode and signal index, preserves zero-index normalization and conditional fatal bounds, and adds indexed/primary getters plus EOTF propagation.
- DeGamma uses one common calculation with five unsigned 32 inputs, four explicit low16 reductions, unsigned 32 log interpolation/wrap, identical18442table elements, conditional64-bit overflow scaling with 32-bit truncation, and a 64-bit final product. Wrappers bind the existing2.4private and 2.6 aggregate table owners and return low32 versus full64 bits. Wider input declarations and differing arithmetic/control shapes were recovery churn; the 2.6 fatal input-index guard was removed after proving it unreachable for every scalar input.

### F059: `core/dm_transfer.h`

- **F059.H01** (library transfer API): Only DeGamma unsigned32 versus LP64 unsigned64 result differs for the common entry.2.4 graphics direction and2.6 indexed/primary/EOTF accessors expose actual different operations. Input-width widening was proved harmless recovery churn and is gone.

Retained details:

- DeGamma has identical five input declarations: gain/offset/exponent/sample use low16 bits and overflow_scale uses full32. This removes a harmless recovered input-width discrepancy while preserving the 2.4 unsigned32 versus 2.6 unsigned64 result contract; indexed/EOTF APIs remain2.6-only.

### F060: `core/dm_trim.c`

- **F060.H01** (native-library replacement): 2.4 mutates/clamps level2 controls and commits bracket-interpolated values plus config/default adjustments.2.6 instead selects kept luminance/chroma trim indices, constructs L2/L8 defaults and resolves display/L10 target maxima. Record14/42-byte strides, duplicate/equality ordering, native limits and partial writes differ by API; this is not a renamed2.4 reduction body.

Retained details:

- STB 2.4 clamps level 2 controls and commits bracket-interpolated trim values plus default/config adjustments. STB 2.6 instead selects up to four kept indices from luma/chroma brackets, constructs L2/L8 default trims and resolves target maximum through display/L10 lookup.
- Equal-key sort ordering, level 2/8 record sizes14/42, default injection, native array limits and diagnostics/partial writes remain part of their respective vendor APIs.

### F061: `core/dm_trim.h`

- **F061.H01** (library trim API): Record selection/default/display-L10 lookup versus singleton bracket-interpolated commit are different library entry contracts. Common FindChromaDist has already aligned input/result forms. Deleted private data exports follow the different distance implementation/data owners and do not imply changed values by themselves.

Retained details:

- STB 2.4 clamps level 2 controls and commits bracket-interpolated trim values plus default/config adjustments. STB 2.6 instead selects up to four kept indices from luma/chroma brackets, constructs L2/L8 default trims and resolves target maximum through display/L10 lookup.
- Equal-key sort ordering, level 2/8 record sizes14/42, default injection, native array limits and diagnostics/partial writes remain part of their respective vendor APIs.

### F062: `core/dm_trim_chroma.c`

- **F062.H01** (native-library pipeline and addition): The final saturated-absolute32 distance sum is identical and luminance inputs share32 bits.2.4 computes its primary determinant/XYZ/LMS/cross-talk pipeline and writes116-byte persistent state;2.6 invokes its native XYZ/ICtCp helpers and accepts null primaries.2.6 also adds display/L10 chroma coordinates and default/duplicate handling.

Retained details:

- Both distance pipelines now finish with the identical shared fixed_abs32 sum, preserving INT32_MIN saturation and unsigned 32 addition. Their preceding primary/XYZ/ICtCp computation and persistent writes remain native.
- STB 2.4 writes its116-byte chroma scratch/state and computes its own primary coefficients; STB 2.6 calls native XYZ/ICtCp helpers and returns 0 for null primaries. The former64-bit luminance spelling is normalized to unsigned 32 because every path discards its high32 bits before arithmetic.
- STB 2.6 adds chroma-coordinate construction from display/L10/metadata primaries, duplicate-coordinate suppression, default exclusion and bounded17-entry scratch workspaces.

### F063: `core/dm_trim_interpolation.c`

- **F063.H01** (native-library addition): 2.6 adds interpolation bracket selection, associated-index sorting, Q12 weighting, L2/L8/default record selection and inverse-LUT uniqueness. Inverse uniqueness keeps end-of-run publication and last_index reload after writes; these have no same-signature2.4 counterparts.

Retained details:

- STB 2.6-only APIs select lower/upper interpolation indices, sort coordinates with associated indices, construct default L2/L8 records, choose native metadata-record pointers and return Q12 weights/default-use flags.
- Inverse-LUT uniqueness preserves end-of-run selection and reloads the mutable last_index after writes. These operations have no same-signature STB 2.4 conversion counterpart.

### F064: `core/dm_trim_luminance.c`

- **F064.H01** (native-library addition): 2.6 builds luminance coordinates from display/L10/L2 maxima, injects/excludes defaults, suppresses rounded/upper duplicates and publishes target PQ.17-entry scratch, count/index order, diagnostic continuation and fatal bounds after earlier stores are explicit and differ from2.4 trim reduction.

Retained details:

- STB 2.6-only luma coordinates use display lookup/L10 fallback or L2 target PQ, scale to the working domain, suppress rounded/upper duplicates and inject/exclude the default before selecting target PQ.
- The bounded17-entry scratch unions, trap after earlier stores and output index/count order are preserved; STB 2.4 trim reduction is a different entry point and storage contract.

### F065: `core/dm_types.h`

- **F065.H01** (library LP64 and fatal-path support): The new indexed/metadata/dimming algorithms operate on64-bit addresses and explicit fatal paths. This header declares/asserts their data model and makes those traps executable; private typedef names are implementation representation rather than a new vendor numerical feature.

Retained details:

- The2.6 algorithms use explicit64-bit long/pointer assumptions and fatal paths;2.4 retains typed native layouts and its own failure returns where applicable. The assertions prevent silently building a different data model.

### F066: `core/dm_workspace.h`

- **F066.H01** (storage requirements of added consolidation/output APIs): The additional consolidation/output operations require overlapping packed matrices, primaries, trim indices and level10 fields. Reserved regions and16/32/64-bit union aliases preserve intermediate writes and native extents. These are local implementation views, not additional persistent objects or a Linux ABI change.

Retained details:

- Consolidation workspace is 384 bytes and output workspace288 bytes. Reserved regions and overlapping16/32/64-bit union views preserve native stack semantics; they are not removable padding.

### F067: `core/fixed_arithmetic.c`

- **F067.H01** (native-library scalar arithmetic and extensions): Common saturating add/subtract/abs/normalization helpers remain shared.2.6 keeps meaningful full-X0 multiply/divide and low32-zero normalization results and adds signed/unsigned128-bit products and wide shifts. Negative shift policies differ: sshl(1,-1)→0x7fffffff/0; sshr(1,INT_MIN)→1/0 and sshr(-1,INT_MIN)→0xffffffff/0 in original2.4/2.6. Local scalar guards were removed in favor of the common header.
- **F067.H02** (native-library scalar arithmetic and extensions): Common saturating add/subtract/abs/normalization helpers remain shared.2.6 keeps meaningful full-X0 multiply/divide and low32-zero normalization results and adds signed/unsigned128-bit products and wide shifts. Negative shift policies differ: sshl(1,-1)→0x7fffffff/0; sshr(1,INT_MIN)→1/0 and sshr(-1,INT_MIN)→0xffffffff/0 in original2.4/2.6. Local scalar guards were removed in favor of the common header.
- **F067.H03** (native-library scalar arithmetic and extensions): Common saturating add/subtract/abs/normalization helpers remain shared.2.6 keeps meaningful full-X0 multiply/divide and low32-zero normalization results and adds signed/unsigned128-bit products and wide shifts. Negative shift policies differ: sshl(1,-1)→0x7fffffff/0; sshr(1,INT_MIN)→1/0 and sshr(-1,INT_MIN)→0xffffffff/0 in original2.4/2.6. Local scalar guards were removed in favor of the common header.
- **F067.H04** (native-library scalar arithmetic and extensions): Common saturating add/subtract/abs/normalization helpers remain shared.2.6 keeps meaningful full-X0 multiply/divide and low32-zero normalization results and adds signed/unsigned128-bit products and wide shifts. Negative shift policies differ: sshl(1,-1)→0x7fffffff/0; sshr(1,INT_MIN)→1/0 and sshr(-1,INT_MIN)→0xffffffff/0 in original2.4/2.6. Local scalar guards were removed in favor of the common header.
- **F067.H05** (native-library scalar arithmetic and extensions): Common saturating add/subtract/abs/normalization helpers remain shared.2.6 keeps meaningful full-X0 multiply/divide and low32-zero normalization results and adds signed/unsigned128-bit products and wide shifts. Negative shift policies differ: sshl(1,-1)→0x7fffffff/0; sshr(1,INT_MIN)→1/0 and sshr(-1,INT_MIN)→0xffffffff/0 in original2.4/2.6. Local scalar guards were removed in favor of the common header.

Retained details:

- Saturating add/subtract/abs/normalize primitives and same-bit scalar declarations are common. Signed add/subtract/abs/shift result views preserve signed consumers; raw input declarations plus explicit signed views preserve all32/64-bit input patterns. STB 2.6 smpy/sdiv/unorm full-X0 results remain real differences: smpy(0xffffffff,0x7fffffff)=0x1ffffffff, sdiv_sls(1,0xffffffff)=0xffffffffffffffff, and low32-zero unorm returns its original high X0 bits.
- Left shift with negative raw 32 count differs: sshl_ssu(1,-1) gives 0x7fffffff versus 0. Right-shift/negative-shift handling retains the related native behavior.
- STB 2.6 adds saturating sdiv with its separate zero result policy, signed/unsigned 128-bit product outputs and differently defined wide-shift helpers. Output order low-then-high and overlapping output slots are retained.
- Original-to-original edge evidence: sshr_sss(1, INT_MIN) returns 1 in STB 2.4 and 0 in STB 2.6; sshr_sss(-1, INT_MIN) returns 0xffffffff versus 0. These are retained raw shift semantics.

### F068: `core/fixed_arithmetic.h`

- **F068.H01** (native-library scalar arithmetic and extensions): Common saturating add/subtract/abs/normalization helpers remain shared.2.6 keeps meaningful full-X0 multiply/divide and low32-zero normalization results and adds signed/unsigned128-bit products and wide shifts. Negative shift policies differ: sshl(1,-1)→0x7fffffff/0; sshr(1,INT_MIN)→1/0 and sshr(-1,INT_MIN)→0xffffffff/0 in original2.4/2.6. Local scalar guards were removed in favor of the common header.

Retained details:

- Saturating add/subtract/abs/normalize primitives and same-bit scalar declarations are common. Signed add/subtract/abs/shift result views preserve signed consumers; raw input declarations plus explicit signed views preserve all32/64-bit input patterns. STB 2.6 smpy/sdiv/unorm full-X0 results remain real differences: smpy(0xffffffff,0x7fffffff)=0x1ffffffff, sdiv_sls(1,0xffffffff)=0xffffffffffffffff, and low32-zero unorm returns its original high X0 bits.
- Left shift with negative raw 32 count differs: sshl_ssu(1,-1) gives 0x7fffffff versus 0. Right-shift/negative-shift handling retains the related native behavior.
- STB 2.6 adds saturating sdiv with its separate zero result policy, signed/unsigned 128-bit product outputs and differently defined wide-shift helpers. Output order low-then-high and overlapping output slots are retained.

### F069: `core/fixed_transcendentals.c`

- **F069.H01** (native-library scale/finishing arithmetic and extensions): The log/power polynomial and bounded sqrt cores are shared. Taylor/exp/repeated-power wrappers retain actual invalid-scale, logical/arithmetic shift and loop-lower-bound rules; fxLOG2Taylor(0x40000000,31)→0/0xffffffff in the originals.2.6 log10 retains meaningful high result bits and adds extended power/angle APIs. fx_assert itself is the same empty function; its use does not impose an assertion policy.
- **F069.H02** (native-library scale/finishing arithmetic and extensions): The log/power polynomial and bounded sqrt cores are shared. Taylor/exp/repeated-power wrappers retain actual invalid-scale, logical/arithmetic shift and loop-lower-bound rules; fxLOG2Taylor(0x40000000,31)→0/0xffffffff in the originals.2.6 log10 retains meaningful high result bits and adds extended power/angle APIs. fx_assert itself is the same empty function; its use does not impose an assertion policy.
- **F069.H03** (native-library scale/finishing arithmetic and extensions): The log/power polynomial and bounded sqrt cores are shared. Taylor/exp/repeated-power wrappers retain actual invalid-scale, logical/arithmetic shift and loop-lower-bound rules; fxLOG2Taylor(0x40000000,31)→0/0xffffffff in the originals.2.6 log10 retains meaningful high result bits and adds extended power/angle APIs. fx_assert itself is the same empty function; its use does not impose an assertion policy.

Retained details:

- The identical six-stage logarithm polynomial now shares fixed_log_polynomial, alongside the existing fixed_power_polynomial. Taylor log/power, exp and repeated power retain different invalid-scale tests, arithmetic/logical shifts and loop lower bounds despite sharing polynomial/multiply primitives. Same-bit signed/unsigned declarations and the entire square-root core/result are now common after full-domain bounds and 4,106 original cross-version probes; log10Taylor retains its actual full-X0 result (input 0 gives 0x1dbe05bc9 in STB 2.6).
- STB 2.6 repeated-power loop uses (scale-2) modulo32 with do/while semantics; STB 2.4 uses a bounded signed loop. Both use only low32 scale bits, so the former64-bit scale spelling is removed. STB 2.6 adds extended exponent/domain functions, angle interpolation and original bounds traps. Disabled fx_assert now has one common ignored-test/message interface.
- Original-to-original edge evidence: fxLOG2Taylor(0x40000000, 31) returns 0 in STB 2.4 and 0xffffffff in STB 2.6; the finishing/invalid-scale branch remains different.

### F070: `core/fixed_transcendentals.h`

- **F070.H01** (native-library scale/finishing arithmetic and extensions): The log/power polynomial and bounded sqrt cores are shared. Taylor/exp/repeated-power wrappers retain actual invalid-scale, logical/arithmetic shift and loop-lower-bound rules; fxLOG2Taylor(0x40000000,31)→0/0xffffffff in the originals.2.6 log10 retains meaningful high result bits and adds extended power/angle APIs. fx_assert itself is the same empty function; its use does not impose an assertion policy.

Retained details:

- The identical six-stage logarithm polynomial now shares fixed_log_polynomial, alongside the existing fixed_power_polynomial. Taylor log/power, exp and repeated power retain different invalid-scale tests, arithmetic/logical shifts and loop lower bounds despite sharing polynomial/multiply primitives. Same-bit signed/unsigned declarations and the entire square-root core/result are now common after full-domain bounds and 4,106 original cross-version probes; log10Taylor retains its actual full-X0 result (input 0 gives 0x1dbe05bc9 in STB 2.6).
- STB 2.6 repeated-power loop uses (scale-2) modulo32 with do/while semantics; STB 2.4 uses a bounded signed loop. Both use only low32 scale bits, so the former64-bit scale spelling is removed. STB 2.6 adds extended exponent/domain functions, angle interpolation and original bounds traps. Disabled fx_assert now has one common ignored-test/message interface.

### F071: `core/fixed_transcendentals_tables.h`

- **F071.H01** (native-library scale/finishing arithmetic and extensions): The log/power polynomial and bounded sqrt cores are shared. Taylor/exp/repeated-power wrappers retain actual invalid-scale, logical/arithmetic shift and loop-lower-bound rules; fxLOG2Taylor(0x40000000,31)→0/0xffffffff in the originals.2.6 log10 retains meaningful high result bits and adds extended power/angle APIs. fx_assert itself is the same empty function; its use does not impose an assertion policy.

Retained details:

- The identical six-stage logarithm polynomial now shares fixed_log_polynomial, alongside the existing fixed_power_polynomial. Taylor log/power, exp and repeated power retain different invalid-scale tests, arithmetic/logical shifts and loop lower bounds despite sharing polynomial/multiply primitives. Same-bit signed/unsigned declarations and the entire square-root core/result are now common after full-domain bounds and 4,106 original cross-version probes; log10Taylor retains its actual full-X0 result (input 0 gives 0x1dbe05bc9 in STB 2.6).
- STB 2.6 repeated-power loop uses (scale-2) modulo32 with do/while semantics; STB 2.4 uses a bounded signed loop. Both use only low32 scale bits, so the former64-bit scale spelling is removed. STB 2.6 adds extended exponent/domain functions, angle interpolation and original bounds traps. Disabled fx_assert now has one common ignored-test/message interface.

### F072: `core/host_control.c`

- **F072.H01** (host compatibility): 4.9 host SDR2020 enum 6 must translate to legacy core 4; -1..3 remain direct. Reset bypasses ordinary format validation. 5.15 uses the multi settings callback and does not need this translation wrapper.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.

### F073: `core/host_control.h`

- **F073.H01** (host compatibility): The legacy 17-argument format-translation boundary is replaced by the host-typed multi callback surface.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- STB 2.4 resets on input format -1 and calls its no-op destroy helper on ordinary completion without clearing initialized. STB 2.6 resets on zero input count, clears native staging and initialization/content bytes, and retains shared control state across ordinary calls. Init_cp takes instance/configuration inputs, rejects counts >=5 and retains native zero/negative-count behavior. The destroy helper now has the same no-argument declaration and zero-result body; its old 2.4 arena argument was ignored by the native instructions.

### F074: `core/kernel_map_composition.c`

- **F074.H01** (native-library inverse-map addition): 2.6 adds primary backward-map concatenation with no2.4 counterpart. It copies tone-control parameters before testing the signed inverse last index, then iterates inclusively through that mutable index, trapping at entry512 after earlier entries were published. The first luma plane interpolates/end-scales32-bit inverse-luma values and narrows directly to short WITHOUT clamping. The other planes apply SMI and signed-divide saturation ratios with their separate rounding and0..0x2000 clamps, native coordinate/denominator selection and offsets. A negative initial count skips map writes after parameter copying; the native entry returns1. Store/read order, partial outputs and first-plane wrapping are required.

Retained details:

- The added ConcatenatePriBwdMapping path copies tone controls, interpolates primary inverse points, calls SMIcore, stores the interpolated inverse-luma plane by narrowing to short without a clamp, then clamps the other three tone/saturation planes to 0..0x2000 and retains signed division/masked index behavior.
- A mapping exceeding512 entries traps after earlier writes; the native inclusive endpoint loop, cached/selected tables and constant-success return remain intact.

### F075: `core/kernel_map_composition.h`

- **F075.H01** (native-library inverse-map addition): 2.6 adds primary backward-map concatenation with no2.4 counterpart. It copies tone-control parameters before testing the signed inverse last index, then iterates inclusively through that mutable index, trapping at entry512 after earlier entries were published. The first luma plane interpolates/end-scales32-bit inverse-luma values and narrows directly to short WITHOUT clamping. The other planes apply SMI and signed-divide saturation ratios with their separate rounding and0..0x2000 clamps, native coordinate/denominator selection and offsets. A negative initial count skips map writes after parameter copying; the native entry returns1. Store/read order, partial outputs and first-plane wrapping are required.

Retained details:

- No2.4 counterpart is removed; this is part of the extended2.6 tone/map orchestration.

### F076: `core/kernel_map_serialization.c`

- **F076.H01** (native library stream layout, restore behavior and API): STB 2.4 packs native control/upsample/downsample/primary records, then conditionally appends secondary/graphics records and a color-selected alpha record before output map/frame data; the stream reaches 10872 bytes and the separate hardware packer reaches 10850 bytes. STB 2.6 uses its changed control/resampling/frame layout, fixed 0x15d0 stream, and 112-byte output map reached through kernel+0x15f8. On load it preserves primary and secondary dimensions/pitch words, secondary type/depth bytes and output pitches; both retain output dimensions from primary input. Before reloading, 2.6 clears the existing 112-byte indirect output map, restores stream map bytes at kernel+0x1568 and sets kernel+0x15f8 to that internal map. 2.4 restores four native frame geometry/pitch sets and uses the existing graphics-map color to choose its optional alpha record. The different clearing, load/store/copy order, sparse writes and restore phases are native behavior, not helper-organization justification. Both length results already use one positive unsigned 32-bit byte-count type.

Retained details:

- 2.4 packs a variable stream up to 10872 bytes with optional secondary/alpha/graphics records and hardware-interface output.2.6 packs a fixed0x15d0-byte stream with changed control/up/downsample/frame fields and indirect output-map storage.
- Loading preserves native dimensions/pitches and selected type/depth fields, clears the specified storage, reconstructs output-map ownership and restores output dimensions from primary input where required. Native copy/store order is retained.
- All produced lengths are nonnegative and fit 32 bits; the unified unsigned-int result preserves every valid return bit pattern, including the former64-bit 0x15d0 return's zero upper bits.

### F077: `core/kernel_map_serialization.h`

- **F077.H01** (native library stream/buffer and entry-point contract): The 2.4 API requires a full 10928-byte kernel object, up to 10872 stream bytes and up to 10850 hardware-output bytes; 2.6 uses its native 0x1608-byte kernel allocation and exactly 0x15d0 stream bytes, including a 112-byte indirectly owned map. Dump needs that old map readable; Load needs it writable before retargeting to the internal map. Neither version accepts a capacity or validates generic input storage. Packed LE fields accept unaligned byte storage; source and destination must be distinct, and every accessed backing region must be complete. Native dimension/pitch/type/depth restoration differs. Only 2.4 exposes dovi_SetHardwareInterface. No return-type or cosmetic source change is presented as a native delta.

Retained details:

- 2.4 exposes hardware-interface packing and variable stream extent;2.6 uses fixed0x15d0 bytes and its own kernel/output-map layout. Both retain explicit unaligned little-endian access.

### F078: `core/matrix_arithmetic.c`

- **F078.H01** (native-library packing and scalar result domains): Precision is one bounded0..64 common declaration/body. General matrix scaling keeps2.4 signed16 scale/result versus2.6 raw32 and truncate/saturate packing;2.6 adds rounded narrowing. Inverse cofactor/determinant/low16-scale core is identical, but singular returns -1/65535 and coefficient packing differ.2.4-only M33TimesM33 retains its32-bit operands and16-bit stores.

Retained details:

- Precision/narrowing shares matrix_scale_impl.h; precision is bounded0..64 and now has the same unsigned 32 declaration/body. General narrowing retains signed 16 versus raw 32 scale/result domains and truncate versus saturate policies. STB 2.6 additionally exposes rounded narrowing.
- InvM33 delegates to an identical common inverse core in matrix_scale_impl.h, preserving raw determinant/cofactor arithmetic, explicit low16 scale interpretation and all input reads before output stores. Inputs now share raw 32 declarations because high16 bits are ignored in both versions. Native signed 16 versus raw 32 singular results (-1 versus 65535) and final truncate versus saturate packing remain different.
- Original/published/new inverse checks cover1024 matrices pertarget including singular, signed 16/raw 32 scale extremes, unaligned and overlapping input/output. STB 2.4 alone exposes M33TimesM33 with its fixed operand access widths and scale result.

### F079: `core/matrix_arithmetic.h`

- **F079.H01** (native-library packing and scalar result domains): Precision is one bounded0..64 common declaration/body. General matrix scaling keeps2.4 signed16 scale/result versus2.6 raw32 and truncate/saturate packing;2.6 adds rounded narrowing. Inverse cofactor/determinant/low16-scale core is identical, but singular returns -1/65535 and coefficient packing differ.2.4-only M33TimesM33 retains its32-bit operands and16-bit stores.

Retained details:

- Precision/narrowing shares matrix_scale_impl.h; precision is bounded0..64 and now has the same unsigned 32 declaration/body. General narrowing retains signed 16 versus raw 32 scale/result domains and truncate versus saturate policies. STB 2.6 additionally exposes rounded narrowing.
- InvM33 delegates to an identical common inverse core in matrix_scale_impl.h, preserving raw determinant/cofactor arithmetic, explicit low16 scale interpretation and all input reads before output stores. Inputs now share raw 32 declarations because high16 bits are ignored in both versions. Native signed 16 versus raw 32 singular results (-1 versus 65535) and final truncate versus saturate packing remain different.
- Original/published/new inverse checks cover1024 matrices pertarget including singular, signed 16/raw 32 scale extremes, unaligned and overlapping input/output. STB 2.4 alone exposes M33TimesM33 with its fixed operand access widths and scale result.

### F080: `core/metadata_init_body.c`

- **F080.H01** (library lifetime/application interface): P01/P02/P07:2.4 rebinds singleton logger/context and clears its static payload records;2.6 allocates0x770 app context,0x10442 sequence and queried parser arena, initializes per-instance callback context, omits parser logger and frees/nulls in the native order. Diagnostics and allocation failure checkpoints differ.

Retained details:

- P01
- P02
- P07

### F081: `core/metadata_runtime.c`

- **F081.H01** (library process/reset/release contract): P02:2.4 uses full eos byte, ignores flags and publishes lengths only on success before resetting parser.2.6 accepts9 arguments, kinds1/2 run VUI/ST2086/T.35, flags bit0 flushes raw processing, success retains parser state, explicit reset deep-clears with flag1 and resets global frame count; release frees/nulls sequence/parser/app. Error diagnostics and partial length publication remain native.

Retained details:

- P01
- P02
- P07

### F082: `core/module_parser.h`

- **F082.H01** (host-facing library callback API): P01/P02: argument counts, context ownership and release(void**) are incompatible native interfaces. Imports and output capacities bind the host adapter to the selected library; changing these is not explained by Linux pointer semantics.

Retained details:

- P01
- P02

### F083: `core/module_parser_context.h`

- **F083.H01** (library application byte layout/lifetime): P01: composer block starts0x30 versus0x18; metadata sequence/config/parser/counter offsets differ and are asserted.2.4 additionally stores header/RPU/DM payloads and persistent counters/logger outside the app;2.6 owns arenas separately. Assertions now use the same unconditional GNU11 form.

Retained details:

- P01

### F084: `core/numeric_color_matrix.c`

- **F084.H01** (native-library matrix pipeline and result domains): YUV offset and bounded white-vector getters are identical. Custom-primary determinant/table coefficients, cross-talk ordering, intermediate rounding and final saturation remain per-version;2.6 adds CT reapplication and V8/diagonal APIs. Inverse getters retain real signed16/raw32 result differences (definition7→32/0xffff0020). The white-point division core now matches exactly; only its scale/result/packing finish differs.2.6 Lms2RgbCtWp retains the extra reserved64-bit slot before white_point.
- **F084.H02** (native-library matrix pipeline and result domains): YUV offset and bounded white-vector getters are identical. Custom-primary determinant/table coefficients, cross-talk ordering, intermediate rounding and final saturation remain per-version;2.6 adds CT reapplication and V8/diagonal APIs. Inverse getters retain real signed16/raw32 result differences (definition7→32/0xffff0020). The white-point division core now matches exactly; only its scale/result/packing finish differs.2.6 Lms2RgbCtWp retains the extra reserved64-bit slot before white_point.
- **F084.H03** (native-library matrix pipeline and result domains): YUV offset and bounded white-vector getters are identical. Custom-primary determinant/table coefficients, cross-talk ordering, intermediate rounding and final saturation remain per-version;2.6 adds CT reapplication and V8/diagonal APIs. Inverse getters retain real signed16/raw32 result differences (definition7→32/0xffff0020). The white-point division core now matches exactly; only its scale/result/packing finish differs.2.6 Lms2RgbCtWp retains the extra reserved64-bit slot before white_point.
- **F084.H04** (native-library matrix pipeline and result domains): YUV offset and bounded white-vector getters are identical. Custom-primary determinant/table coefficients, cross-talk ordering, intermediate rounding and final saturation remain per-version;2.6 adds CT reapplication and V8/diagonal APIs. Inverse getters retain real signed16/raw32 result differences (definition7→32/0xffff0020). The white-point division core now matches exactly; only its scale/result/packing finish differs.2.6 Lms2RgbCtWp retains the extra reserved64-bit slot before white_point.
- **F084.H05** (native-library matrix pipeline and result domains): YUV offset and bounded white-vector getters are identical. Custom-primary determinant/table coefficients, cross-talk ordering, intermediate rounding and final saturation remain per-version;2.6 adds CT reapplication and V8/diagonal APIs. Inverse getters retain real signed16/raw32 result differences (definition7→32/0xffff0020). The white-point division core now matches exactly; only its scale/result/packing finish differs.2.6 Lms2RgbCtWp retains the extra reserved64-bit slot before white_point.

Retained details:

- YUV-to-RGB offset calculation now has an identical explicit-width common body, including unaligned reads, signed 64 arithmetic shift and row-by-row reload across overlapping output.
- Definition/custom-primary XYZ/LMS transforms retain coefficient-table layout, determinant truncation/masks and fixed XYZ-to-LMS operand differences. Cross-talk/white-point composition differs in intermediate rounding, division order, row/column scaling and saturation.
- STB 2.6 adds DE_CT/APPLY_CT and reapplication helpers, V8-primary wrappers and diagonal getters. Its Lms2RgbCtWp boundary reserves an extra64-bit argument slot before the white-point pointer; that slot is intentionally retained.
- Rgb2LmsWpByDefV3 now explicitly returns its bounded native scale in both versions. The original STB 2.6 epilogue preserves W0 after GetRgb2LmsM33; the former void declaration was an unsupported decompiler choice, removed using instruction and native-return evidence.
- GetLms2RgbByDefM33Fxp(definition7) is an explicit result-domain difference: original STB 2.4 returns signed 16 value32, while original STB 2.6 returns raw 32 value0xffff0020. GetRgb2LmsByDefM33 and forward custom-primary scale results are bounded positive and now share unsigned 32 declarations; inverse getters retain their meaningful narrowed result domains.
- GetRgb2LmsWpM33Fxp now shares its once-per-row signed-white and explicit-zero division core. A signed16 coefficient multiplied by 65536 lies in [-2^31, 2^31-65536], so division by signed32 has no overflow case. The different scale/result domains and truncate-versus-saturate finishing remain native updates.

### F085: `core/numeric_color_matrix.h`

- **F085.H01** (native-library matrix pipeline and result domains): YUV offset and bounded white-vector getters are identical. Custom-primary determinant/table coefficients, cross-talk ordering, intermediate rounding and final saturation remain per-version;2.6 adds CT reapplication and V8/diagonal APIs. Inverse getters retain real signed16/raw32 result differences (definition7→32/0xffff0020). The white-point division core now matches exactly; only its scale/result/packing finish differs.2.6 Lms2RgbCtWp retains the extra reserved64-bit slot before white_point.

Retained details:

- YUV-to-RGB offset calculation now has an identical explicit-width common body, including unaligned reads, signed 64 arithmetic shift and row-by-row reload across overlapping output.
- Definition/custom-primary XYZ/LMS transforms retain coefficient-table layout, determinant truncation/masks and fixed XYZ-to-LMS operand differences. Cross-talk/white-point composition differs in intermediate rounding, division order, row/column scaling and saturation.
- STB 2.6 adds DE_CT/APPLY_CT and reapplication helpers, V8-primary wrappers and diagonal getters. Its Lms2RgbCtWp boundary reserves an extra64-bit argument slot before the white-point pointer; that slot is intentionally retained.
- Rgb2LmsWpByDefV3 now explicitly returns its bounded native scale in both versions. The original STB 2.6 epilogue preserves W0 after GetRgb2LmsM33; the former void declaration was an unsupported decompiler choice, removed using instruction and native-return evidence.
- GetLms2RgbByDefM33Fxp(definition7) is an explicit result-domain difference: original STB 2.4 returns signed 16 value32, while original STB 2.6 returns raw 32 value0xffff0020. GetRgb2LmsByDefM33 and forward custom-primary scale results are bounded positive and now share unsigned 32 declarations; inverse getters retain their meaningful narrowed result domains.

### F086: `core/numeric_helpers.c`

- **F086.H01** (native-library range/layout and added APIs): GetRange(10,3) returns0/1023 versus64/959 and retains distinct minimum/extent alias reload behavior. Bypass divisor, signed-depth and zero/full-range rules differ. CommitKsUs has different arguments/packed store sequence;2.6 adds level bits, stable ascending sort, downscale setup, inverse/interpolated LUTs and display/primary pointer lookups. Four unused2.4 includes are gone.

Retained details:

- Range fallback and exceptional depths differ natively: GetRange(10,3) produces 0/1023 versus 64/959. Minimum/extent alias reload semantics are retained.
- Bypass offset stores now use the common YUV helper; each version retains divisor, signed-depth and zero/full-range rules. CommitKsUs has a different native input interface and output-store sequence.
- STB 2.6 adds level-bit operations, stable ascending sort, downscale/filter setup, inverse/interpolated LUT helpers and display/primary pointer lookups. Their signed comparisons, last-index domains and output ordering remain explicit.

### F087: `core/numeric_helpers.h`

- **F087.H01** (native-library range/layout and added APIs): GetRange(10,3) returns0/1023 versus64/959 and retains distinct minimum/extent alias reload behavior. Bypass divisor, signed-depth and zero/full-range rules differ. CommitKsUs has different arguments/packed store sequence;2.6 adds level bits, stable ascending sort, downscale setup, inverse/interpolated LUTs and display/primary pointer lookups. Four unused2.4 includes are gone.

Retained details:

- Range fallback and exceptional depths differ natively: GetRange(10,3) produces 0/1023 versus 64/959. Minimum/extent alias reload semantics are retained.
- Bypass offset stores now use the common YUV helper; each version retains divisor, signed-depth and zero/full-range rules. CommitKsUs has a different native input interface and output-store sequence.
- STB 2.6 adds level-bit operations, stable ascending sort, downscale/filter setup, inverse/interpolated LUT helpers and display/primary pointer lookups. Their signed comparisons, last-index domains and output ordering remain explicit.

### F088: `core/numeric_tables.h`

- **F088.H01** (native-library added coefficient data): 2.6-only XYZ definition and white-point/CCT constants serve the new color-temperature/primary APIs. The63 signed-bit-pattern64-bit coefficients and15 white-point entries are a concrete new data dependency, not a generic header-organization reason.

Retained details:

- STB 2.6 table aliases and XYZ-definition/white-point-CCT constants bind its custom-primary and color-temperature implementation. Values and storage ownership were preserved; no matching STB 2.4 standalone header exists.

### F089: `core/numeric_tone.c`

- **F089.H01** (native-library addition): 2.6 exposes forward/backward tone/saturation wrappers over its new parameter block at0x1650, plus HLG OOTF composition. Shift/scale inputs, cached sqrt values, ratio operations and full64 versus32 return domains follow those new entry points;2.4 does not expose these signatures.

Retained details:

- STB 2.6-only tone/saturation forward/backward wrappers select parameter block0x1650 and native Q11 domain conversions. Reference/value parameter names now match declarations and exact underlying types replace opaque aliases.
- HLG OOTF uses extended PQ/power helpers, including native repeated loads and zero/negative-ratio behavior. These are additional vendor API bodies, not alternate spellings of STB 2.4 vector-map helpers.

### F090: `core/numeric_tone.h`

- **F090.H01** (native-library addition): 2.6 exposes forward/backward tone/saturation wrappers over its new parameter block at0x1650, plus HLG OOTF composition. Shift/scale inputs, cached sqrt values, ratio operations and full64 versus32 return domains follow those new entry points;2.4 does not expose these signatures.

Retained details:

- STB 2.6-only tone/saturation forward/backward wrappers select parameter block0x1650 and native Q11 domain conversions. Reference/value parameter names now match declarations and exact underlying types replace opaque aliases.
- HLG OOTF uses extended PQ/power helpers, including native repeated loads and zero/negative-ratio behavior. These are additional vendor API bodies, not alternate spellings of STB 2.4 vector-map helpers.

### F091: `core/numeric_transfer.c`

- **F091.H01** (native-library transfer functions and slope division): 2.4-only97-point target EOTF/lerp/alpha-beta helpers are replaced by2.6 CCT/XYZ/ICtCp/white-point/gamma-parameter stages with native state writes. GenerateL2GLut shares its entire sampling/store structure and32-bit gamma; its slope denominator is zero-extended in2.4 and sign-extended in2.6. The former2.6 breakpoint guard was unreachable under immutable breaks and was removed; it is not a functional assertion delta.
- **F091.H02** (native-library transfer functions and slope division): 2.4-only97-point target EOTF/lerp/alpha-beta helpers are replaced by2.6 CCT/XYZ/ICtCp/white-point/gamma-parameter stages with native state writes. GenerateL2GLut shares its entire sampling/store structure and32-bit gamma; its slope denominator is zero-extended in2.4 and sign-extended in2.6. The former2.6 breakpoint guard was unreachable under immutable breaks and was removed; it is not a functional assertion delta.

Retained details:

- GenerateL2GLut common sampling/store structure matches and both now take unsigned 32 gamma: all original STB 2.6 gamma uses explicitly discard high32 bits. The zero-extended (STB 2.4) versus sign-extended (STB 2.6) slope denominator remains a real behavior difference.
- STB 2.4 additionally implements97-point target-EOTF LUT/lerp and gamma alpha/beta calculation. STB 2.6 instead implements CCT/xy/XYZ/ICtCp conversion, intended white-point CCT and native gamma-parameter preparation with its own state writes.
- The original STB 2.6 breakpoint guard is unreachable with immutable breaks={27,16,12,9,2,0}: the 26 loop iterations advance sections only at 16,12,9,2 and use indices 1..5. The dead guard is omitted; this does not create an assertion-policy difference.

### F092: `core/numeric_transfer.h`

- **F092.H01** (native-library transfer functions and slope division): 2.4-only97-point target EOTF/lerp/alpha-beta helpers are replaced by2.6 CCT/XYZ/ICtCp/white-point/gamma-parameter stages with native state writes. GenerateL2GLut shares its entire sampling/store structure and32-bit gamma; its slope denominator is zero-extended in2.4 and sign-extended in2.6. The former2.6 breakpoint guard was unreachable under immutable breaks and was removed; it is not a functional assertion delta.

Retained details:

- GenerateL2GLut common sampling/store structure matches and both now take unsigned 32 gamma: all original STB 2.6 gamma uses explicitly discard high32 bits. The zero-extended (STB 2.4) versus sign-extended (STB 2.6) slope denominator remains a real behavior difference.
- STB 2.4 additionally implements97-point target-EOTF LUT/lerp and gamma alpha/beta calculation. STB 2.6 instead implements CCT/xy/XYZ/ICtCp conversion, intended white-point CCT and native gamma-parameter preparation with its own state writes.

### F093: `core/numeric_transfer_tables.h`

- **F093.H01** (native-library transfer functions and slope division): 2.4-only97-point target EOTF/lerp/alpha-beta helpers are replaced by2.6 CCT/XYZ/ICtCp/white-point/gamma-parameter stages with native state writes. GenerateL2GLut shares its entire sampling/store structure and32-bit gamma; its slope denominator is zero-extended in2.4 and sign-extended in2.6. The former2.6 breakpoint guard was unreachable under immutable breaks and was removed; it is not a functional assertion delta.

Retained details:

- STB 2.4-only target-EOTF/logarithmic gamma tables belong to its97-point helper family. Gamma breakpoint, pulsar and SDR values already share common include files across both versions; raw values remain unchanged.

### F094: `core/original_data.c`

- **F094.H01** (library values and data ownership): The new typed control/signature/constant owners preserve native version/chip/build/diagnostic strings, trim tables, primary definitions and shared table values at asserted offsets. It is source-defined data, not linked binary algorithm content. Owner names remain historical archaeology but do not themselves justify a behavior delta.

Retained details:

- STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.

### F095: `core/parser_contract.h`

- **F095.H01** (explicit library policies and diagnostics): P01 uses PARSER_ALLOCATED_APPLICATION_STORAGE; P14 uses PARSER_SIGN_EXTEND_METADATA_COPY_LENGTH, no longer coupled to reset flags. P03/P04/P05/P08/P09/P10/P13 cover reset/argument shapes, error draining, fatal bounds, extension callback/unknown limits, accounting/address wrapping and NLQ sign truncation. P07 covers source paths/line constants. No cosmetic alignment/residual-filter/one-bit-reader selectors remain.

Retained details:

- P03
- P04
- P05
- P07
- P08
- P09
- P10
- P13

### F096: `core/parser_info.c`

- **F096.H01** (library identity data/address contract): P07:2.4 algorithm text is API text+8;2.6 has separate string owners. Returned contents and native object relationships remain explicit; this is not a numerical parser change or a kernel version consequence.

Retained details:

- P07

### F098: `core/pq_conversion.c`

- **F098.H01** (native-library PQ endpoint/result domains and additions): Shared12/16-bit inverse interpolation and bounded unsigned32 declarations remain common.2.4 control LToPQ12 permits rounded4096;2.6 caps4095. Signed/extended L2PqX keeps different range branches/rounding and meaningful32/full64 results.2.6 adds16/18-bit, signed inverse and extended table conversions. The inverse table values are common; added extended tables are real new algorithm data.
- **F098.H02** (native-library PQ endpoint/result domains and additions): Shared12/16-bit inverse interpolation and bounded unsigned32 declarations remain common.2.4 control LToPQ12 permits rounded4096;2.6 caps4095. Signed/extended L2PqX keeps different range branches/rounding and meaningful32/full64 results.2.6 adds16/18-bit, signed inverse and extended table conversions. The inverse table values are common; added extended tables are real new algorithm data.

Retained details:

- Common12/16-bit inverse interpolation bodies use pq12_impl.h and now share unsigned 32 input declarations because both explicitly mask low16 bits. LToPQ12 now has one bounded unsigned 32 result declaration; STB 2.4 still permits rounded4096 while STB 2.6 caps4095. Both decoder conversions cap4095.
- Signed/extended L2PqX retains native range branches, rounding and meaningful return widths: raw 64 input 0xffffffffffffffff gives signed 32 -108 in STB 2.4 versus full64 0x7ffffffffff94 in STB 2.6. STB 2.6 adds16/18-bit LUT conversions, signed forward/inverse extended PQ and table-interpolated extended conversions with original fatal bounds.
- The private STB 2.6 asr64 implementation was replaced by the shared fixed_asr64 for its existing constant1..63 shift callers; duplicate shift presentation is removed without changing their results.

### F099: `core/pq_conversion.h`

- **F099.H01** (native-library PQ endpoint/result domains and additions): Shared12/16-bit inverse interpolation and bounded unsigned32 declarations remain common.2.4 control LToPQ12 permits rounded4096;2.6 caps4095. Signed/extended L2PqX keeps different range branches/rounding and meaningful32/full64 results.2.6 adds16/18-bit, signed inverse and extended table conversions. The inverse table values are common; added extended tables are real new algorithm data.

Retained details:

- Common12/16-bit inverse interpolation bodies use pq12_impl.h and now share unsigned 32 input declarations because both explicitly mask low16 bits. LToPQ12 now has one bounded unsigned 32 result declaration; STB 2.4 still permits rounded4096 while STB 2.6 caps4095. Both decoder conversions cap4095.
- Signed/extended L2PqX retains native range branches, rounding and meaningful return widths: raw 64 input 0xffffffffffffffff gives signed 32 -108 in STB 2.4 versus full64 0x7ffffffffff94 in STB 2.6. STB 2.6 adds16/18-bit LUT conversions, signed forward/inverse extended PQ and table-interpolated extended conversions with original fatal bounds.
- The private STB 2.6 asr64 implementation was replaced by the shared fixed_asr64 for its existing constant1..63 shift callers; duplicate shift presentation is removed without changing their results.

### F100: `core/pq_conversion_tables.h`

- **F100.H01** (native-library PQ endpoint/result domains and additions): Shared12/16-bit inverse interpolation and bounded unsigned32 declarations remain common.2.4 control LToPQ12 permits rounded4096;2.6 caps4095. Signed/extended L2PqX keeps different range branches/rounding and meaningful32/full64 results.2.6 adds16/18-bit, signed inverse and extended table conversions. The inverse table values are common; added extended tables are real new algorithm data.

Retained details:

- Common12/16-bit inverse interpolation bodies use pq12_impl.h and now share unsigned 32 input declarations because both explicitly mask low16 bits. LToPQ12 now has one bounded unsigned 32 result declaration; STB 2.4 still permits rounded4096 while STB 2.6 caps4095. Both decoder conversions cap4095.
- Signed/extended L2PqX retains native range branches, rounding and meaningful return widths: raw 64 input 0xffffffffffffffff gives signed 32 -108 in STB 2.4 versus full64 0x7ffffffffff94 in STB 2.6. STB 2.6 adds16/18-bit LUT conversions, signed forward/inverse extended PQ and table-interpolated extended conversions with original fatal bounds.
- The private STB 2.6 asr64 implementation was replaced by the shared fixed_asr64 for its existing constant1..63 shift callers; duplicate shift presentation is removed without changing their results.

### F101: `core/register_commit.c`

- **F101.H01** (library behavior, native layout and store phases): Singleton source/graphics/target commit differs from primary-ordered multi-source preparation, VSEM/VSIF/HDR handling, CommitFinalized, source flags and L11 callback publication. Retain the selected 8-to-10 in-place pivot/NLQ conversion, native synthetic source setup, different core register maps/LUT bases and overlap-sensitive phases. Shared composer/LUT packing and result/prototypes are identical.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- Shared composer packing keeps a constant-zero unsigned-short result and pointer/dimension arguments. The native composer base/buffer extent are 176/900 bytes in 2.4 versus 200/968 in 2.6. Mode is residual_flag!=0 versus constant zero. Width/height originate from 2.4 source+12/+16 versus 2.6 source+0/+4. STB 2.6 can change selected 8-bit BL/EL configuration and coefficients to 10 bits in place before packing. Interleaved loads/stores remain valid for its overlapping views; 2.4 requires disjoint objects.
- The four 15-bit paired LUT streams and final 32-bit input-map copy now have one implementation and pointer interface. Callers select tone/input bases kernel+1328/+200 in 2.4 versus kernel+0x540/+0xd8 in 2.6. Output order and 2.6 overlap behavior are preserved.
- The 2.4 entry writes an 88-byte target descriptor and returns -1 for null/invalid identification. The 2.6 helper writes its control-state capability fields, treats null as retained state/success and returns -8 for invalid identification/version/length. Each keeps its original capability/luminance partial writes. Luminance units and rounding differ; in particular 2.6 v1 squares/left-shifts a u32, reinterprets signed and divides toward zero. Original instructions at 0015c4b4..0015c4c4 confirm signed lowering. Proven equivalent packed-primary expressions now use the clearer common spelling.
- The 2.4 control representation has a 71-byte prefix and fixed 261-byte extension slots in current/previous 66626-byte backing, optional temporal level-1 filtering, logger failure for length>256 and native HDMI trim/removal/compaction. The 2.6 outer control path consumes contiguous variable-length records in 0x1000-byte slots, retaining native signed offsets, incomplete-header reuse, per-source progress and fatal capacity checks after partial writes. It has two metadata/composer staging slots despite three private host input records.
- STB 2.6 adds VSEM/VSIF packet decoding, packet header/OUI validation and -5 failure paths, transport-derived signal configuration, optional Dolby control packets, and content/white-point/level-11 callback notification. Packet/type/status writes and callback sequences remain ordered. The 2.4 callback path instead uses the native packed metadata and separate HDR10/source arguments.
- The 2.4 graphics path derives a separate 64-byte graphics descriptor from CLI fields and applies stored luminance offsets. The 2.6 path uses native input_mode 2, format defaults/primaries, graphic luminance words and a separate Core 2 slot. Native gamma, matrix, source/default field writes and partial-state order remain version-specific.
- The 2.4 raw HDMI publisher calculates size before payload writes, caps >512 words, logs and preserves the previous payload after availability is cleared. The 2.6 publisher retains native raw-word and extension writes interleaved with fatal capacity checks, publishes transport control data and content information, and preserves register/LUT slot selection. Core register/host field offsets and buffer extents differ. HDR publication now uses four explicit LE64 copies at offsets 0,8,16,21, retaining the final overlapping store.

### F102: `core/register_commit.h`

- **F102.H01** (library API): The native singleton commit takes formats, metadata/composition/graphics/source and two LUTs; the multi commit takes input configuration, source array, target/VSVDB and multi-bank LUT storage. All buffers already use pointer interfaces.

Retained details:

- STB 2.4 has one source, separate graphics/target descriptors and an explicit runtime storage binding. STB 2.6 accepts three 0x68-byte private inputs in a 0x4f58-byte settings record, selects a primary, stages multiple signal/kernel contexts and publishes two Core 1 result slots. This changes source selection and callback shape independently of the kernel.
- Shared composer packing keeps a constant-zero unsigned-short result and pointer/dimension arguments. The native composer base/buffer extent are 176/900 bytes in 2.4 versus 200/968 in 2.6. Mode is residual_flag!=0 versus constant zero. Width/height originate from 2.4 source+12/+16 versus 2.6 source+0/+4. STB 2.6 can change selected 8-bit BL/EL configuration and coefficients to 10 bits in place before packing. Interleaved loads/stores remain valid for its overlapping views; 2.4 requires disjoint objects.
- The 2.4 raw HDMI publisher calculates size before payload writes, caps >512 words, logs and preserves the previous payload after availability is cleared. The 2.6 publisher retains native raw-word and extension writes interleaved with fatal capacity checks, publishes transport control data and content information, and preserves register/LUT slot selection. Core register/host field offsets and buffer extents differ. HDR publication now uses four explicit LE64 copies at offsets 0,8,16,21, retaining the final overlapping store.

### F103: `core/resampling_filters.c`

- **F103.H01** (native-library address/reload and API behavior): The prediction reader shares coefficient sign extension and scalar declarations, but component/direction are sign-extended for2.4 versus zero-extended for2.6 address arithmetic. Log capture timing, flag/delta reloads and payload-owner reload paths differ.2.4 keeps its spatial phase reader private entry;2.6 has no matching original body to expose.

Retained details:

- Read-log decoding and coefficient sign extension now use matching representation; equivalent signed counter>=2/unsigned log-level>=3 comparisons and required64-bit-offset checks also match.
- Component/direction declarations now share raw 32 inputs with explicit signed(STB 2.4)/unsigned(STB 2.6) views inside their different native address arithmetic. Diagnostics capture timing, prediction-flag/delta reloads and cached versus reloaded payload owners remain native.
- STB 2.4 additionally exposes the spatial filter/phase reader; STB 2.6 has no same original exported body in this unit. Its removal is an API-family difference, not a reason to remove the working STB 2.4 implementation.

### F104: `core/resampling_filters.h`

- **F104.H01** (native-library address/reload and API behavior): The prediction reader shares coefficient sign extension and scalar declarations, but component/direction are sign-extended for2.4 versus zero-extended for2.6 address arithmetic. Log capture timing, flag/delta reloads and payload-owner reload paths differ.2.4 keeps its spatial phase reader private entry;2.6 has no matching original body to expose.

Retained details:

- Read-log decoding and coefficient sign extension now use matching representation; equivalent signed counter>=2/unsigned log-level>=3 comparisons and required64-bit-offset checks also match.
- Component/direction declarations now share raw 32 inputs with explicit signed(STB 2.4)/unsigned(STB 2.6) views inside their different native address arithmetic. Diagnostics capture timing, prediction-flag/delta reloads and cached versus reloaded payload owners remain native.
- STB 2.4 additionally exposes the spatial filter/phase reader; STB 2.6 has no same original exported body in this unit. Its removal is an API-family difference, not a reason to remove the working STB 2.4 implementation.

### F105: `core/shared_data.h`

- **F105.H01** (library data ownership): Canonical 2.6 callback/display/frame/matrix objects are declared once and referenced from the new multi-input algorithms. They are not alternate owners for the singleton layouts.

Retained details:

- STB 2.6 owns canonical contiguous 210856-byte control storage, signature storage and constant/table members with explicit reserved spans and offset/size assertions. STB 2.4 keeps its separately aligned runtime owners and table objects. Storage owners and native relative aliases remain separate; identical table includes retain shared values without merging instances. Version/build/diagnostic strings remain the pinned native 2.4/2.6 identities.

### F106: `core/tone_controls.c`

- **F106.H01** (native-library control-layout replacement): 2.4 maps source/target display and level2 trims into its typed tone control.2.6 prepares indexed controls from different metadata/config/state records, applies native luma/chroma/default/HLG/backlight rules and preserves repeated loads and partial stores. previous_mode names the actual argument; canonical dovi_* calls replace alias noise. Neither naming change alters policy.

Retained details:

- STB 2.4 updates level-adjustment controls, maps tone control and derives target/HLG signal fields through small singleton helpers. STB 2.6 prepares indexed source/target color state, metadata overrides, adaptive backlight, PQ bounds and tone-LUT sizes in a distinct consolidation path.
- Native context offsets, secondary-signal propagation, repeated reads after packed stores, LUT bound traps and per-signal update ordering remain version-specific. Audited role names now describe the local bindings, including their multiple lifetimes; existing native object types and offsets remain separate recovery work.

### F107: `core/tone_controls.h`

- **F107.H01** (native-library control-layout replacement): 2.4 maps source/target display and level2 trims into its typed tone control.2.6 prepares indexed controls from different metadata/config/state records, applies native luma/chroma/default/HLG/backlight rules and preserves repeated loads and partial stores. previous_mode names the actual argument; canonical dovi_* calls replace alias noise. Neither naming change alters policy.

Retained details:

- STB 2.4 updates level-adjustment controls, maps tone control and derives target/HLG signal fields through small singleton helpers. STB 2.6 prepares indexed source/target color state, metadata overrides, adaptive backlight, PQ bounds and tone-LUT sizes in a distinct consolidation path.
- Native context offsets, secondary-signal propagation, repeated reads after packed stores, LUT bound traps and per-signal update ordering remain version-specific. Audited role names now describe the local bindings, including their multiple lifetimes; existing native object types and offsets remain separate recovery work.

### F108: `core/tone_curve.c`

- **F108.H01** (native-library tone algorithm replacement): 2.4 compares160 bytes, builds2/3-point parameters and ratio/auxiliary maps with its168-byte control layout; mode3 can return untouched.2.6 compares0x1a0 control plus0xf8 parameters at0x1650, prepares/builds/packs its new pipeline and copies the first sample into four control slots. Native change flags and buffers differ.

Retained details:

- STB 2.4 exposes2/3-point generation, source/display parameter objects and a 160-byte comparison of 168-byte control. Ratio-pointer selection can leave three auxiliary maps untouched; mode3 two-point calls make no output writes.
- STB 2.6 coordinator compares 0x1a0 control and 0xf8 parameter bytes at 0x1650, prepares/builds/packs, stores first sample into four slots and returns native change flags; its mode/context slots remain part of the call boundary.

### F109: `core/tone_curve.h`

- **F109.H01** (native-library tone algorithm replacement): 2.4 compares160 bytes, builds2/3-point parameters and ratio/auxiliary maps with its168-byte control layout; mode3 can return untouched.2.6 compares0x1a0 control plus0xf8 parameters at0x1650, prepares/builds/packs its new pipeline and copies the first sample into four control slots. Native change flags and buffers differ.

Retained details:

- STB 2.4 exposes2/3-point generation, source/display parameter objects and a 160-byte comparison of 168-byte control. Ratio-pointer selection can leave three auxiliary maps untouched; mode3 two-point calls make no output writes.
- STB 2.6 coordinator compares 0x1a0 control and 0xf8 parameter bytes at 0x1650, prepares/builds/packs, stores first sample into four slots and returns native change flags; its mode/context slots remain part of the call boundary.

### F110: `core/tone_curve_lut.c`

- **F110.H01** (native-library LUT sampling/layout replacement): 2.4 samples its4096 log-coordinate table using floor/ceil stride and optional515-word padded interpolation into4096 outputs; finite size/untouched cells are documented.2.6 samples512 values, applies HLG when selected and packs register LUTs/control state. Distinct sizes/offsets and source reloads prohibit substituting one sampler for the other.

Retained details:

- STB 2.4 samples a 4096-step log-coordinate curve, optionally interpolates into 4096 outputs from a padded515-element work buffer and retains its finite size/untouched-cell rules.
- STB 2.6 samples512 primary/identity values, applies HLG where required and packs register-ready output plus control state with native reloads. The algorithm and buffer layouts differ, not just their names.

### F111: `core/tone_curve_lut.h`

- **F111.H01** (native-library LUT sampling/layout replacement): 2.4 samples its4096 log-coordinate table using floor/ceil stride and optional515-word padded interpolation into4096 outputs; finite size/untouched cells are documented.2.6 samples512 values, applies HLG when selected and packs register LUTs/control state. Distinct sizes/offsets and source reloads prohibit substituting one sampler for the other.

Retained details:

- STB 2.4 samples a 4096-step log-coordinate curve, optionally interpolates into 4096 outputs from a padded515-element work buffer and retains its finite size/untouched-cell rules.
- STB 2.6 samples512 primary/identity values, applies HLG where required and packs register-ready output plus control state with native reloads. The algorithm and buffer layouts differ, not just their names.

### F112: `core/tone_curve_lut_table.inc`

- **F112.H01** (native-library LUT sampling/layout replacement): 2.4 samples its4096 log-coordinate table using floor/ceil stride and optional515-word padded interpolation into4096 outputs; finite size/untouched cells are documented.2.6 samples512 values, applies HLG when selected and packs register LUTs/control state. Distinct sizes/offsets and source reloads prohibit substituting one sampler for the other.

Retained details:

- STB 2.4-only 4096-element log-coordinate table feeds its native curve sampler. STB 2.6 uses different tone-core sampling and does not need this table; its raw values were preserved.

### F113: `core/tone_mapping_math.c`

- **F113.H01** (native-library addition): 2.6 introduces local-saturation records, HLG inverse-OETF construction, Hermite tone/saturation cores and cutoff scoring with signed128-bit product reduction. HLG writes256 words at+0x30 and can trap after partial writes; TMScore can publish sqrt before later config loads. These are concrete new algorithms, not decompiler spellings of2.4.

Retained details:

- STB 2.6-only tone/saturation primitives implement local-saturation patching, HLG EOTF parameter generation, Hermite-based tone/saturation curves and cutoff scoring with native fixed-point scales and 128-bit product reduction.
- The HLG stage writes256 words at output+0x30 and can trap after partial writes; TMScore's optional sqrt-output store precedes later config reads. Aliasing, saturation bounds and native BRK behavior are preserved.

### F114: `core/tone_mapping_math.h`

- **F114.H01** (native-library addition): 2.6 introduces local-saturation records, HLG inverse-OETF construction, Hermite tone/saturation cores and cutoff scoring with signed128-bit product reduction. HLG writes256 words at+0x30 and can trap after partial writes; TMScore can publish sqrt before later config loads. These are concrete new algorithms, not decompiler spellings of2.4.

Retained details:

- STB 2.6-only tone/saturation primitives implement local-saturation patching, HLG EOTF parameter generation, Hermite-based tone/saturation curves and cutoff scoring with native fixed-point scales and 128-bit product reduction.
- The HLG stage writes256 words at output+0x30 and can trap after partial writes; TMScore's optional sqrt-output store precedes later config reads. Aliasing, saturation bounds and native BRK behavior are preserved.

### F115: `core/tone_mapping_math_tables.h`

- **F115.H01** (native-library addition): 2.6 introduces local-saturation records, HLG inverse-OETF construction, Hermite tone/saturation cores and cutoff scoring with signed128-bit product reduction. HLG writes256 words at+0x30 and can trap after partial writes; TMScore can publish sqrt before later config loads. These are concrete new algorithms, not decompiler spellings of2.4.

Retained details:

- STB 2.6-only tone/saturation primitives implement local-saturation patching, HLG EOTF parameter generation, Hermite-based tone/saturation curves and cutoff scoring with native fixed-point scales and 128-bit product reduction.
- The HLG stage writes256 words at output+0x30 and can trap after partial writes; TMScore's optional sqrt-output store precedes later config reads. Aliasing, saturation bounds and native BRK behavior are preserved.

### F116: `core/tone_maps.c`

- **F116.H01** (native-library auxiliary-map replacement): 2.4 produces independent intensity/saturation maps from scalar curve controls;2.6 builds primary plus three auxiliary maps with its new tone/saturation cores and square-root scratch, native rounding/clamping and packed endpoints. The two2.4 fx_assert calls are no-ops retained as original call structure; their absence is not a2.6 policy change.

Retained details:

- STB 2.4 vector maps generate independent intensity/saturation LUTs from curve pairs and display/ratio controls. STB 2.6 builds primary plus three auxiliary maps from parameter blocks and tone/saturation core calls.
- Native mode dispatch, square-root scratch slots, clamping/rounding constants,511/512 endpoints, packed output offsets and repeated loads distinguish the bodies. Audited role names now describe the local bindings and their multiple lifetimes; the distinct parameter layouts and native object typing remain explicit.
- The two STB 2.4 fx_assert call sites are retained native call structure inside the different tone algorithm. The common assertion function ignores its arguments and returns; their absence in STB 2.6 is not a functional assertion-policy update.

### F117: `core/tone_maps.h`

- **F117.H01** (native-library auxiliary-map replacement): 2.4 produces independent intensity/saturation maps from scalar curve controls;2.6 builds primary plus three auxiliary maps with its new tone/saturation cores and square-root scratch, native rounding/clamping and packed endpoints. The two2.4 fx_assert calls are no-ops retained as original call structure; their absence is not a2.6 policy change.

Retained details:

- STB 2.4 vector maps generate independent intensity/saturation LUTs from curve pairs and display/ratio controls. STB 2.6 builds primary plus three auxiliary maps from parameter blocks and tone/saturation core calls.
- Native mode dispatch, square-root scratch slots, clamping/rounding constants,511/512 endpoints, packed output offsets and repeated loads distinguish the bodies. Audited role names now describe the local bindings and their multiple lifetimes; the distinct parameter layouts and native object typing remain explicit.

### F118: `core/tone_parameters.c`

- **F118.H01** (native-library parameter algorithm replacement): 2.4 derives2/3-point mapped parameters and curve segments with ordered output aliases.2.6 copies/derives a different control/parameter record, maps low/mid/high points and applies HLG/native-luminance/trim scaling. Its thresholds, intermediate shifts and stores are the actual algorithm delta. The23 retained2.4 fx_assert calls impose no assertion policy and are not counted as functional differences.

Retained details:

- STB 2.4 derives2/3-point mapped parameters and curve segments from source/display structures with ordered output aliases. STB 2.6 copies a different control block into its parameter record, computes mode/HLG/native-luminance transforms and stores mapped low/mid/high fields.
- Native shifts, source clamps, white-point/trim scales, denominator handling and parameter destinations remain distinct; no algebraic unification was attempted without original-instruction evidence.
- The 23 STB 2.4 fx_assert call sites are retained native call structure inside the different parameter algorithm. Arguments are side-effect-free and the common assertion function is empty; no observable assertion-policy difference is claimed.

### F119: `core/tone_parameters.h`

- **F119.H01** (native-library parameter algorithm replacement): 2.4 derives2/3-point mapped parameters and curve segments with ordered output aliases.2.6 copies/derives a different control/parameter record, maps low/mid/high points and applies HLG/native-luminance/trim scaling. Its thresholds, intermediate shifts and stores are the actual algorithm delta. The23 retained2.4 fx_assert calls impose no assertion policy and are not counted as functional differences.

Retained details:

- STB 2.4 derives2/3-point mapped parameters and curve segments from source/display structures with ordered output aliases. STB 2.6 copies a different control block into its parameter record, computes mode/HLG/native-luminance transforms and stores mapped low/mid/high fields.
- Native shifts, source clamps, white-point/trim scales, denominator handling and parameter destinations remain distinct; no algebraic unification was attempted without original-instruction evidence.

### F120: `core/tone_tables.inc`

- **F120.H01** (nonfunctional shared-value owner binding): The2.4 translation unit defines log10/rate/power arrays from common value includes;2.6 binds those same values through its typed constant aggregate. DeGamma now uses one shared calculation over those bindings. Removing the2.4 owner declarations from the2.6 conversion is source/data binding, not changed table values or arithmetic.

Retained details:

- STB 2.4 table owners bind the already-common tone log10/rate/power include values. STB 2.6 obtains corresponding arrays from its constant aggregate; values are shared while ownership remains native.

### F121: `core/trim_reduction.c`

- **F121.H01** (native-library API-family replacement): 2.4 compacts packed big-endian L2/L8/L10/254 records, sorts keys with equal-key swaps and selects endpoints/brackets with16-record work arrays.2.6 instead uses its separate coordinate/default/interpolation APIs and layouts; the removed API is not discarded as dead or cosmetically renamed.

Retained details:

- STB 2.4-only reduction reads packed big-endian metadata trim records, compacts L2/L8/L10 levels, sorts and selects target/display brackets before copying native records.
- Equal keys swap and can reverse duplicate-key ordering; fixed arrays hold16 records. STB 2.6 replaces this entry-point family with separate selection/default/interpolation builders, not the same reduction contract.

### F122: `core/trim_reduction.h`

- **F122.H01** (native-library API-family replacement): 2.4 compacts packed big-endian L2/L8/L10/254 records, sorts keys with equal-key swaps and selects endpoints/brackets with16-record work arrays.2.6 instead uses its separate coordinate/default/interpolation APIs and layouts; the removed API is not discarded as dead or cosmetically renamed.

Retained details:

- STB 2.4-only reduction reads packed big-endian metadata trim records, compacts L2/L8/L10 levels, sorts and selects target/display brackets before copying native records.
- Equal keys swap and can reverse duplicate-key ordering; fixed arrays hold16 records. STB 2.6 replaces this entry-point family with separate selection/default/interpolation builders, not the same reduction contract.

### F123: `core/trim_tables.inc`

- **F123.H01** (native-library lookup layout replacement): The2.4 display/primary records feed its trim-reduction entry points.2.6 uses its distinct display objects and19-primary table through new lookup APIs; raw constants and lookup extents remain separately pinned. Common tone/PQ values have already been factored into shared include files.

Retained details:

- STB 2.4-only display and primary lookup records support trim reduction, while STB 2.6 uses its own display storage and 19-primary table. Native initializer values and ownership were preserved.

### F124: `core_objects.mk`

- **F124.H01** (multi-input implementation dependencies): Remove singleton graphics/metadata/runtime objects replaced by the STB 2.6 multi-input control and statically bound services; add the adaptive backlight implementation.
- **F124.H02** (extended metadata implementation dependency): Link the new STB 2.6 metadata-output selection, measurement and staged serialization implementation.
- **F124.H03** (mapping and trim implementation dependencies): Link STB 2.6 luminance/interpolated trim and kernel-map composition; remove the historical many-argument host-control enum adapter because the host now passes multi-input settings.
- **F124.H04** (new numerical and data dependencies): Link the STB 2.6 numerical tone operations and native aggregate constants/state required by its added control/display-management paths.
- **F124.H05** (native tone algorithm dependencies): Link the STB 2.6 tone-mapping mathematics instead of the STB 2.4 trim-reduction implementation. Every listed object has its included C source and no processing binary is linked.

Retained details:

- 2.4 retains single-input host/control orchestration and legacy trim helpers.2.6 adds backlight, output metadata, interpolation/luminance trim and primary-map composition units, and removes superseded single-input runtime/metadata/graphics units.
- The restored common align_to_byte object replaces equivalent inline code in 2.6; all module algorithms remain source-compiled.

### F125: `host_abi.h`

- **F125.H01** (host compatibility): Match the actual host headers, input-kind/signal enums, 48-byte legacy versus 112-byte complete callback table, three 0x68-byte input records and 0x4f58 multi settings. The asserted fields are actual callback/buffer compatibility requirements, not library algorithm changes.
- **F125.H02** (host compatibility): Assert the 0x1720 Core1 result-bank layout and declare the actual multi-control, explicit-context parser/reset/process and registration-version signatures. Legacy many-argument control and singleton parser declarations remain in 2.4.

Retained details:

- 4.9 callback table 48 bytes and single settings/output ownership differ from the 5.15 table 112 bytes,0x4f58 multi-input settings, three0x68-byte input records and two Core1 outputs.
- Parser input-kind enums, signal values, register/LUT/HDR/extension fields and offsets retain native host semantics; assertions fail compilation for a mismatched provider.

### F126: `host_adapter.c`

- **F126.H01** (host compatibility and lifetime): 4.9 supplies logger/flags and legacy format adaptation plus configured services. 5.15 supplies explicit parser context/input kind and multi settings, formats native version storage, records registration status and remains resident if the host may have retained callbacks. Failure-lifetime policy is explicit, not an algorithm-equivalence claim.

Retained details:

- 4.9 provides vprintk-backed logging, singleton parser calls and the many-argument control adapter with host/core format translation.5.15 passes explicit contexts and input kind, invokes multi-control settings, builds the version/chip strings, and registers multi callbacks.
- 4.9 propagates configure/registration failures.5.15 records the registration result in a read-only parameter, logs failure/success and returns successful insertion to keep possibly retained callbacks resident; replacing the attempted registration requires reboot.

### F127: `host_adapter.h`

- **F127.H01** (module identity and loader metadata): Description/version and GPL versus AML metadata are explicitly recorded declarations. They are not attributed to Linux semantics and no new relicensing inference is made.

Retained details:

- 2.4 declares GPL/version0.1 and 2.6 declares AML/version0.2 with matching descriptions. These are retained module metadata declarations, not inferred kernel semantics or a legal relicensing decision.

### F128: `runtime_bridge.c`

- **F128.H01** (host service compatibility): Bind the actual logger/CPU/memory/diagnostic service shapes: copied legacy runtime services versus 2.6 parser vmalloc/vfree, OS memset and _printk. Actual imports/generated CRCs remain kernel-build inputs.

Retained details:

- 4.9 configures copied control-runtime services and printk/vprintk adapters.5.15 supplies vmalloc/vfree-backed parser ownership, OS memset and _printk through its module/control service objects.
- Kernel-provided symbol names, CRCs and instrumentation are rebuilt from the selected generated inputs; service tables remain separate from algorithm/state ownership.

### F129: `runtime_bridge.h`

- **F129.H01** (host service compatibility): The explicit legacy configure hook is removed because the new native services are bound by static module/control service objects before callback registration.

Retained details:

- The2.6 host adapter does not call the 2.4 configuration hook; kernel services are bound in runtime_bridge.c before registration instead.

## Validation and remaining work

Validation combines matching-kernel source builds, host-layout/CRC checks,
original-instruction comparisons and published/new linked-module native
differentials. Cases cover numerical boundaries, unaligned/disjoint backing,
audited overlapping views, parser lifecycle/streams, native traps, observer
callbacks and partial output. Only the recorded OS/observer services are
modeled; algorithm calls execute AArch64 instructions.

The STB 2.4 tranche additionally has physical AM6B+ playback evidence on
21.3-p3i-tcpfix-f0bfd922 and complete local replay of all four primary device
captures: 20,023 callbacks per comparison pass, with matching return values,
output backing and captured-provider results. Both identical-input and
verified direct-copy parser-chained passes succeed.
See the README validation record for artifact hashes.

The finite native and physical tests establish the baseline evidence within
their recorded input/lifecycle coverage. The isolated trace-off missing-EL
warning remains a timing/repeatability investigation; no algorithm discrepancy
has been established. STB 2.6 matching-device playback, broader edge/error
coverage and sustained transitions remain separate work. The 5.15 targets
share algorithm/data sections and source; kernel metadata and symbol CRCs
differ. Direct STB 2.6 callbacks require a non-CFI host configuration.

Native object typing for distinct STB 2.6 algorithms is further source work.
Any intentional hardening or behavior change must receive its own changelog
entry and validation.
