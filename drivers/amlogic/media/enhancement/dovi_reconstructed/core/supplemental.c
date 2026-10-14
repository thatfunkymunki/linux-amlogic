/* VUI/ST2086 ingress recovered against the P3i T4c reference binary. */
#include "supplemental.h"
#include "bitstream.h"
#include "luminance_pq.h"

typedef unsigned char byte;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char require_abi[(sizeof(void *) == 8 && sizeof(u32) == 4 &&
                         sizeof(u64) == 8 && sizeof(int) == 4) ? 1 : -1];

static byte *pointer_at(const byte *p)
{
    byte *value;
    byte *target = (byte *)&value;
    u32 i;
    for (i = 0; i < 8; ++i)
        target[i] = p[i];
    return value;
}

static void write16(byte *p, u32 value)
{
    p[0] = (byte)value;
    p[1] = (byte)(value >> 8);
}

static void write32(byte *p, u32 value)
{
    p[0] = (byte)value;
    p[1] = (byte)(value >> 8);
    p[2] = (byte)(value >> 16);
    p[3] = (byte)(value >> 24);
}

static int signed32(u32 value)
{
    return value < 0x80000000u ? (int)value : -1 - (int)~value;
}

int rpu_decoder_push_VUI(void *opaque, const void *vui)
{
    byte *decoder = (byte *)opaque;
    byte *metadata;
    static const u32 ycc[9] = {
        0x2566u, 0u, 0x35eau, 0x2566u, 0xf9fcu, 0xeb1cu,
        0x2566u, 0x44cau, 0u
    };
    static const u32 lms[9] = {
        0x1c36u, 0x2243u, 0x186u, 0xa5eu, 0x308eu, 0x514u,
        0u, 0x1a6u, 0x3e5au
    };
    u32 i;
    (void)vui;
    if (decoder == 0)
        return -7;
    metadata = pointer_at(pointer_at(decoder) + 0x10);
    for (i = 0; i < 9; ++i) {
        write16(metadata + 4 + i * 2, ycc[i]);
        write16(metadata + 0x24 + i * 2, lms[i]);
    }
    write32(metadata + 0x18, 0x1000000u);
    write32(metadata + 0x1c, 0x8000000u);
    write32(metadata + 0x20, 0x8000000u);
    write16(metadata + 0x36, 0xffffu);
    write16(metadata + 0x38, 0);
    write16(metadata + 0x3a, 0);
    write32(metadata + 0x3c, 0);
    metadata[0x40] = 12;
    metadata[0x41] = 0;
    metadata[0x42] = 0;
    metadata[0x43] = 1;
    write16(metadata + 0x48, 42);
    write32(decoder + 0x315c0, 1);
    return 0;
}

int mastering_display_colour_volume_read(void *opaque)
{
    byte *decoder = (byte *)opaque;
    byte *metadata = pointer_at(pointer_at(decoder) + 0x10);
    struct bitstream *bits = (struct bitstream *)(decoder + 0x40);
    u32 i, maximum, minimum;
    /* The eight primary/white-point coordinates are consumed, not stored. */
    for (i = 0; i < 8; ++i)
        (void)read_u_v(16, bits);
    maximum = read_u_v(32, bits);
    minimum = read_u_v(32, bits);
    /* Preserve the reference's 32-bit shift for minimum and 64-bit maximum. */
    write16(metadata + 0x44,
            luminance_to_pq12((minimum << 18) / 10000u));
    write16(metadata + 0x46,
            luminance_to_pq12((u32)(((u64)maximum << 18) / 10000u)));
    return signed32(bits->bits_read);
}

int rpu_decoder_push_2086_payload(void *opaque,
                                    const unsigned char *buffer, u32 size)
{
    byte *decoder = (byte *)opaque;
    int result;
    if (decoder == 0)
        return -7;
    if (size == 0)
        return 0;
    if (buffer == 0)
        return -7;
    bitstream_init((struct bitstream *)(decoder + 0x40), buffer, size);
    result = mastering_display_colour_volume_read(decoder);
    if (result < 0)
        return result;
    write32(decoder + 0x315bc, 1);
    return 0;
}

dv_md_parser_rc_t dv_md_parser_process_vui(dv_md_parser_ctx_t *parser,
                                             const void *vui)
{
    int result;
    if (parser == 0)
        return DV_MD_PARSER_INVALID_CONTEXT_HANDLE;
    result = rpu_decoder_push_VUI(parser->p_rpu_dcd_ctx, vui);
    if (result != 0)
        parser->last_rc = (dv_md_parser_rc_t)result;
    return (dv_md_parser_rc_t)result;
}

dv_md_parser_rc_t dv_md_parser_process_mastering_display_colour_volume(
    dv_md_parser_ctx_t *parser, const unsigned char *buffer, u32 size)
{
    int result;
    if (parser == 0)
        return DV_MD_PARSER_INVALID_CONTEXT_HANDLE;
    result = rpu_decoder_push_2086_payload(parser->p_rpu_dcd_ctx, buffer, size);
    if (result != 0)
        parser->last_rc = (dv_md_parser_rc_t)result;
    return (dv_md_parser_rc_t)result;
}
