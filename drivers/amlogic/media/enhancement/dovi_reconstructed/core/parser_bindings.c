/* Typed adapters bind parser dispatch to the recovered decoder bodies. */
#include "parser_bindings.h"
#include "md_parser_process.h"
#include "decoder_bindings.h"
#include "rpu_push_data.h"
#include "rpu_last_output.h"
#include "st2094.h"

static int signed_size(unsigned int size)
{
    /* The original forwards W-register bits to signed decoder parameters. */
    return size <= 0x7fffffffu ? (int)size : -1 - (int)~size;
}

static int push_data(void *decoder, const unsigned char *buffer,
                     unsigned int size, int last)
{
    return rpu_decoder_push_data((struct rpu_push_view *)decoder,
                                 buffer, signed_size(size), last);
}

static int get_output(void *decoder, struct rpu_data_header_s **header,
    struct vdr_rpu_data_payload_s **payload, int *payload_size,
    struct vdr_dm_data_payload_s **dm, int *dm_size)
{
    void *rh = 0, *rp = 0, *display = 0;
    int result = rpu_decoder_get_output(decoder, &rh, &rp, payload_size,
                                        &display, dm_size);
    *header = (struct rpu_data_header_s *)rh;
    *payload = (struct vdr_rpu_data_payload_s *)rp;
    *dm = (struct vdr_dm_data_payload_s *)display;
    return result;
}

static int process_buffer(void *decoder, const unsigned char *buffer,
    unsigned int size, struct rpu_data_header_s **header,
    struct vdr_rpu_data_payload_s **payload, int *payload_size,
    struct vdr_dm_data_payload_s **dm, int *dm_size)
{
    void *rh = 0, *rp = 0, *display = 0;
    int result = rpu_decoder_process_buffer(decoder, (unsigned char *)buffer,
        signed_size(size), &rh, &rp, payload_size, &display, dm_size);
    *header = (struct rpu_data_header_s *)rh;
    *payload = (struct vdr_rpu_data_payload_s *)rp;
    *dm = (struct vdr_dm_data_payload_s *)display;
    return result;
}

static int last_output(const void *decoder)
{
    return rpu_decoder_last_rpu_output(
        (const struct rpu_decoder_prefix *)decoder);
}

static const struct md_parser_decoder_ops parser_ops = {
    push_data,
    rpu_decoder_push_2094_payload,
    get_output,
    process_buffer,
    last_output
};

dv_md_parser_rc_t dv_md_parser_process_bound(
    dv_md_parser_ctx_t *parser, unsigned int au_flags, const timestamp_t *pts,
    const unsigned char *buffer, unsigned int size, int last, int *flags)
{
    return dv_md_parser_process(parser, au_flags, pts, buffer, size, last,
                                 flags, &parser_ops);
}

dv_md_parser_rc_t dv_md_parser_process_t35_bound(
    dv_md_parser_ctx_t *parser, unsigned int au_flags, const timestamp_t *pts,
    const unsigned char *buffer, unsigned int size)
{
    return dv_md_parser_process_user_data_registered_itu_t_t35(
        parser, au_flags, pts, buffer, size, &parser_ops);
}
