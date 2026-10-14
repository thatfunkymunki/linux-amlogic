#ifndef AM6B_MD_PARSER_PROCESS_H
#define AM6B_MD_PARSER_PROCESS_H

#include "../dv_md_parser_abi_recovered.h"

/* Explicit decoder dependencies: the wrappers are recovered, while bindings
 * decide which decoder/payload implementations to use. No fallback is supplied.
 * Active paths retain the original unchecked decoder, event, and flags pointers.
 */
struct md_parser_decoder_ops {
    int (*push_data)(void *decoder, const unsigned char *buffer,
                     unsigned int size, int last);
    int (*push_2094_payload)(void *decoder, const unsigned char *buffer,
                             unsigned int size);
    int (*get_output)(void *decoder, struct rpu_data_header_s **header,
                       struct vdr_rpu_data_payload_s **payload, int *payload_size,
                       struct vdr_dm_data_payload_s **dm, int *dm_size);
    int (*process_buffer)(void *decoder, const unsigned char *buffer,
                           unsigned int size, struct rpu_data_header_s **header,
                           struct vdr_rpu_data_payload_s **payload,
                           int *payload_size, struct vdr_dm_data_payload_s **dm,
                           int *dm_size);
    int (*last_rpu_output)(const void *decoder);
};

dv_md_parser_rc_t dv_md_parser_process(
    dv_md_parser_ctx_t *parser, unsigned int au_flags, const timestamp_t *pts,
    const unsigned char *buffer, unsigned int size, int last, int *flags,
    const struct md_parser_decoder_ops *ops);

dv_md_parser_rc_t dv_md_parser_process_user_data_registered_itu_t_t35(
    dv_md_parser_ctx_t *parser, unsigned int au_flags, const timestamp_t *pts,
    const unsigned char *buffer, unsigned int size,
    const struct md_parser_decoder_ops *ops);

#endif
