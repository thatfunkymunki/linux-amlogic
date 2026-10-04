#ifndef AM6B_SUPPLEMENTAL_H
#define AM6B_SUPPLEMENTAL_H

#include "../dv_md_parser_abi_recovered.h"

/* These preserve the reference's unchecked nested pointers and padded reader.
 * decoder is the 202472-byte arena, with rpu_data at +0 and bitstream +0x40.
 * VUI input is deliberately ignored by the reference; a null input is valid.
 * ST2086 inputs must have readable refill padding even if nominally truncated.
 */
int rpu_decoder_push_VUI(void *decoder, const void *vui);
int mastering_display_colour_volume_read(void *decoder);
int rpu_decoder_push_2086_payload(void *decoder,
                                    const unsigned char *buffer,
                                    unsigned int size);
dv_md_parser_rc_t dv_md_parser_process_vui(dv_md_parser_ctx_t *parser,
                                             const void *vui);
dv_md_parser_rc_t dv_md_parser_process_mastering_display_colour_volume(
    dv_md_parser_ctx_t *parser, const unsigned char *buffer, unsigned int size);

#endif
