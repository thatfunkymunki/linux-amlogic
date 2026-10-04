/* parser dispatch wrappers recovered from the SHA-pinned T4c module. */
#include "md_parser_process.h"

static void sample(dv_md_parser_ctx_t *parser,
                   struct rpu_data_header_s *header,
                   struct vdr_rpu_data_payload_s *payload, int payload_size,
                   struct vdr_dm_data_payload_s *dm, int dm_size, int result)
{
    parser->pf_md_parser_cb(parser->p_md_parser_cb_ctx,
        MD_PARSER_SAMPLE_AVAIL_EVT, &parser->pts,
        header, payload, payload_size, dm, dm_size, result);
}

static void error(dv_md_parser_ctx_t *parser, int result)
{
    parser->last_rc = (dv_md_parser_rc_t)result;
    parser->pf_md_parser_cb(parser->p_md_parser_cb_ctx,
        MD_PARSER_ERROR_EVT, 0, 0, 0, 0, 0, 0, 0);
}

dv_md_parser_rc_t dv_md_parser_process(
    dv_md_parser_ctx_t *parser, unsigned int au_flags, const timestamp_t *pts,
    const unsigned char *buffer, unsigned int size, int last, int *flags,
    const struct md_parser_decoder_ops *ops)
{
    struct rpu_data_header_s *header;
    struct vdr_rpu_data_payload_s *payload;
    struct vdr_dm_data_payload_s *dm;
    int payload_size = 0, dm_size = 0, result;
    unsigned int flag_bits;

    if (parser == 0)
        return DV_MD_PARSER_INVALID_CONTEXT_HANDLE;
    if (parser->f_init == 0)
        return DV_MD_PARSER_UNINITIALIZE;
    if ((au_flags & 8u) != 0 && pts != 0 &&
        pts->time_val != parser->curr_pts.time_val) {
        parser->pts.time_val = parser->curr_pts.time_val;
        parser->curr_pts.time_val = pts->time_val;
    }

    if (parser->flags.input_is_RBSP != 0) {
        result = ops->process_buffer(parser->p_rpu_dcd_ctx, buffer, size,
                                      &header, &payload, &payload_size,
                                      &dm, &dm_size);
        if (result > 0) {
            sample(parser, header, payload, payload_size, dm, dm_size, result);
            result = 0;
        } else {
            error(parser, result);
        }
    } else {
        result = ops->push_data(parser->p_rpu_dcd_ctx, buffer, size, last);
        if (result != 0) {
            parser->last_rc = (dv_md_parser_rc_t)result;
        } else {
            for (;;) {
                header = 0;
                payload = 0;
                dm = 0;
                result = ops->get_output(parser->p_rpu_dcd_ctx,
                    &header, &payload, &payload_size, &dm, &dm_size);
                if (result <= 0)
                    break;
                if (ops->last_rpu_output(parser->p_rpu_dcd_ctx) != 0)
                    parser->pts.time_val = parser->curr_pts.time_val;
                sample(parser, header, payload, payload_size,
                       dm, dm_size, result);
            }
            /* ARM64 adds at 32-bit width, then compares unsigned. */
            if ((unsigned int)result + 21u <= 1u)
                result = 0;
            else
                error(parser, result);
        }
    }
    flag_bits = (unsigned int)*flags;
    if (((const unsigned char *)parser->p_rpu_dcd_ctx)[0x316e0] != 0)
        flag_bits |= 1u;
    else
        flag_bits &= ~1u;
    /* Copy bits without an implementation-defined unsigned-to-signed cast. */
    {
        unsigned char *dst = (unsigned char *)flags;
        unsigned int i;
        for (i = 0; i < sizeof(int); ++i)
            dst[i] = ((const unsigned char *)&flag_bits)[i];
    }
    return (dv_md_parser_rc_t)result;
}

dv_md_parser_rc_t dv_md_parser_process_user_data_registered_itu_t_t35(
    dv_md_parser_ctx_t *parser, unsigned int au_flags, const timestamp_t *pts,
    const unsigned char *buffer, unsigned int size,
    const struct md_parser_decoder_ops *ops)
{
    struct rpu_data_header_s *header;
    struct vdr_rpu_data_payload_s *payload;
    struct vdr_dm_data_payload_s *dm;
    dovi_u64 count;
    int payload_size = 0, dm_size = 0, result;

    if (parser == 0)
        return DV_MD_PARSER_INVALID_CONTEXT_HANDLE;
    if (parser->f_init == 0)
        return DV_MD_PARSER_UNINITIALIZE;
    count = parser->ui64_sample;
    if ((au_flags & 8u) != 0 && pts != 0) {
        if (count == 0 || pts->time_val != parser->pts.time_val) {
            parser->pts.time_val = pts->time_val;
        } else if (parser->pf_msg_log != 0) {
            parser->pf_msg_log(
                "Fatal Error: consecutive SEI user data registered ITU.T t35 message with same PTS\n");
            count = parser->ui64_sample;
        }
    }
    parser->ui64_sample = count + 1u;
    result = ops->push_2094_payload(parser->p_rpu_dcd_ctx, buffer, size);
    if (result != 0) {
        parser->last_rc = (dv_md_parser_rc_t)result;
        return (dv_md_parser_rc_t)result;
    }
    for (;;) {
        header = 0;
        payload = 0;
        dm = 0;
        result = ops->get_output(parser->p_rpu_dcd_ctx,
            &header, &payload, &payload_size, &dm, &dm_size);
        if (result <= 0)
            break;
        sample(parser, header, payload, payload_size, dm, dm_size, result);
    }
    if ((unsigned int)result + 21u <= 1u)
        return DV_MD_PARSER_OK;
    error(parser, result);
    return (dv_md_parser_rc_t)result;
}
