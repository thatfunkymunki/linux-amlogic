/* SHA-256: 5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b */
#include "st2094.h"
#include "bitstream.h"
#include "align_to_byte.h"
#include "dm_payload.h"

typedef unsigned char byte;
typedef unsigned int u32;
typedef char require_abi[(sizeof(void *) == 8 && sizeof(u32) == 4 && sizeof(int) == 4) ? 1 : -1];

static byte *pointer(const byte *source)
{
    byte *result;
    unsigned int i;
    for (i = 0; i < 8; ++i)
        ((byte *)&result)[i] = source[i];
    return result;
}

static void set_pointer(byte *destination, const byte *value)
{
    unsigned int i;
    for (i = 0; i < 8; ++i)
        destination[i] = ((const byte *)&value)[i];
}

static u32 read16(const byte *p)
{
    return (u32)p[0] | ((u32)p[1] << 8);
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

int st2094_10_data_read(void *opaque)
{
    byte *decoder = (byte *)opaque;
    struct bitstream *bits = (struct bitstream *)(decoder + 0x40);
    byte *root = pointer(decoder);
    byte *dm = pointer(root + 16);
    byte *header = pointer(root);
    byte *payload = pointer(root + 8);
    u32 baseline = bits->bits_read;
    u32 value, provider, user;
    unsigned int component;
    write32(decoder + 0x315b8, 0);
    value = read_u_v(8, bits);
    if (value == 255) {
        (void)read_u_v(8, bits);
        return 0;
    }
    if (value != 181)
        return 0;
    provider = read_u_v(8, bits) << 8;
    provider |= read_u_v(8, bits);
    if (provider != 59 && provider != 49)
        return 0;
    user = read_u_v(32, bits);
    if (user != 0 && user != 0x47413934u)
        return 0;
    if (read_u_v(8, bits) != 9)
        return 0;
    write32(decoder + 0x315b8, 1);
    (void)read_ue_v(bits);
    (void)read_ue_v(bits);
    if (read_u_v(1, bits) != 0) {
        byte count = (byte)read_ue_v(bits);
        byte current = 0;
        int has_8 = 0, has_254 = 0;
        dm[0x4a] = count;
        align_to_byte(bits);
        if (count != 0) {
            do {
                byte *record = dm + 0x4c + (u32)current * 264u;
                int result;
                write32(record, read_ue_v(bits));
                record[4] = (byte)read_u_v(8, bits);
                result = ext_block_payload_read(pointer(decoder + 8),
                    decoder + 0x316d8, record, bits, &current);
                baseline += (u32)result;
                record = dm + 0x4c + (u32)current * 264u;
                if (record[4] == 3) {
                    write16(record + 6, read16(record + 6) >> 1);
                    write16(record + 8, read16(record + 8) >> 1);
                    write16(record + 10, read16(record + 10) >> 1);
                } else if (record[4] == 8) {
                    has_8 = 1;
                } else if (record[4] == 254) {
                    record[6] = 0;
                    record[7] = 2;
                    has_254 = 1;
                }
                current = (byte)(current + 1u);
            } while (dm[0x4a] > current);
            if (!has_8 && dm[0x4a] != 0) {
                byte append_index = current;
                byte index = 0;
                int appended = 0;
                do {
                    byte *record = dm + 0x4c + (u32)index * 264u;
                    if (record[4] == 2) {
                        u32 pq = read16(record + 6);
                        record[4] = 8;
                        write32(record, 10);
                        if (((pq - 0x81cu) & 0xffffu) <= 9u)
                            record[6] = 1;
                        else if (((pq - 0xb1fu) & 0xffffu) <= 9u)
                            record[6] = 27;
                        else if (((pq - 0xc02u) & 0xffffu) <= 9u)
                            record[6] = 48;
                        else if (((pq - 0xd38u) & 0xffffu) <= 9u)
                            record[6] = 37;
                        else {
                            byte *extra = dm + 0x4c + (u32)append_index * 264u;
                            write32(extra, 5);
                            extra[4] = 10;
                            extra[6] = 255;
                            write16(extra + 8, read16(record + 6));
                            write16(extra + 10, 62);
                            extra[12] = 0;
                            record[6] = 255;
                            dm[0x4a] = (byte)(dm[0x4a] + 1u);
                            append_index = (byte)(append_index + 1u);
                            appended = 1;
                        }
                        if (read16(record + 18) == 65535u)
                            write16(record + 18, 2048);
                    }
                    index = (byte)(index + 1u);
                } while (dm[0x4a] > index);
                if (appended)
                    current = append_index;
            }
            if (!has_254) {
                byte *record = dm + 0x4c + (u32)current * 264u;
                write32(record, 2);
                record[4] = 254;
                record[6] = 0;
                record[7] = 2;
                dm[0x4a] = (byte)(dm[0x4a] + 1u);
            }
        }
    }
    header[9] = 0;
    header[15] = 10;
    header[10] = 23;
    header[16] = 8;
    header[17] = 12;
    header[0xf0] = 0;
    header[0x133] = 1;
    header[0x134] = 1;
    header[6] = 1;
    header[22] = 1;
    header[21] = 0;
    header[18] = 0;
    for (component = 0; component < 3; ++component) {
        byte *poly = payload + component * 0x30cu;
        header[0x25 + component] = 2;
        write16(header + 0x88 + component * 32u, 0);
        write16(header + 0x8a + component * 32u, (1u << (header[15] & 31u)) - 1u);
        payload[component * 15u] = 0;
        poly[0x5c] = 1;
        write32(poly + 0x68, 0);
        write32(poly + 0x7c, 0);
        write32(poly + 0x6c, 1);
        write32(poly + 0x80, 0x1806);
    }
    return signed32(bits->bits_read - baseline);
}

int rpu_decoder_push_2094_payload(void *opaque, const byte *buffer, u32 size)
{
    byte *decoder = (byte *)opaque;
    int result;
    if (decoder == 0)
        return -7;
    if (size == 0)
        return 0;
    if (buffer == 0)
        return -7;
    set_pointer(pointer(decoder) + 8, pointer(decoder + 0x316d0));
    bitstream_init((struct bitstream *)(decoder + 0x40), buffer, size);
    result = st2094_10_data_read(decoder);
    return result <= 0 ? result : 0;
}
