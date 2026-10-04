#ifndef AM6B_RPU_PAYLOAD_H
#define AM6B_RPU_PAYLOAD_H

/* The SHA-pinned decoder is 202472 bytes, with root/context at +0/+8
 * and initialized bitstream at +0x40. Root contains header/payload at +0/+8.
 * Supply disjoint writable objects and padded readable bitstream input.
 * Payload partition stride is 0x7038; the original has no general index or
 * coefficient-array bounds checks. See the scoped differential evidence.
 */
int rpu_payload_read(void *decoder);

#endif
