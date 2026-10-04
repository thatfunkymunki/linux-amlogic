# metadata callback initialization body

`metadata_parser_init_body` recovers the body of `metadata_parser_init`
at ELF `.text+0x1ac00` (324 bytes) in `p3i-t4c-dovi.ko`, SHA-256
`5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b`.
It is a helper with explicit storage, event-handler, and logger arguments.
It is not the host callback's `void *(*)(int)` ABI and must not be registered
as that callback. The original ignored `flag`, which this helper preserves.

The original uses a 1920-byte `app_ctx_t` at `.bss+0x77fb0`. DWARF CU 19
identifies input/output pointers at 0/8/16, its three-pointer RPU output
binding at 24, a 1800-byte main config at 48, DM byte sequence at 1848,
parser config at 1856, and parser handle at 1912. The header represents
payloads and main config as opaque storage and checks their exact offsets.

The supplied `metadata_init_storage` matches the original contiguous
globals from `ctx` through `rc`, including eight inter-buffer padding bytes:

| Storage | Offset from `ctx` | Cleared bytes |
| --- | ---: | ---: |
| app context | 0 | 1920 |
| RPU header | 1920 | 310 |
| composer RPU payload | 2232 | 28728 |
| DM payload | 30960 | 67396 |
| DM byte sequence | 98360 | 66626 |
| parser return code | 164988 | written after parser init |

The helper clears and binds these buffers in original order, leaving all
padding untouched. It selects main profile 1 and payload-only 1 with all
remaining flags zero, calls the original memory query (ignoring its size),
then creates and initializes the fixed parser singleton. It installs the
event handler and logger supplied by its caller. The original symbols for
these are `md_parser_evt_handler_cb` and `printk`; the event handler has not
been reconstructed and is not stubbed here.

On normal success the helper stores `rc=0`, logs the exact original
kernel-severity-prefixed success string, and returns the app context. The
original create-failure and init-failure logging/return paths are retained.
They cannot execute with the original static create implementation,
fixed nonnull handler, and decoder initializer's unconditional zero return.
The body does not allocate or destroy parser storage. Its release callback
is independently recovered as a no-op in `md_parser_init.c`.

The linked differential suite in `../tests/test_md_parser_init.py`
matched five original callback initializations with flags 0, 1, -1,
`INT32_MIN`, and `INT32_MAX`. It compared all 164,992 bytes of app/buffer
storage, including padding and return code, the complete 334,048-byte
parser singleton, normalized pointers, logger output, and every original
`memset` destination/length. There are five callback buffer clears plus
six decoder arena clears. Release leaves both regions unchanged.

Coverage is 68/81 original callback instructions. The 13 unreached
instructions belong exactly to its impossible create/init failure branches;
the JSON results record their offsets. The test executes original linked
parser/decoder code rather than substituting return values. Public kernel
integration still needs the recovered event-handler implementation,
singleton/storage bindings, and the explicit T4c ABI adapter. No runtime or
hardware compatibility claim follows from these tests.
