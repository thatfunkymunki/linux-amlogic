/* Reference SHA-256: 5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b */
#include "metadata_callbacks.h"
#include "parser_diagnostics.h"

static int metadata_signed(unsigned int value)
{
    return value <= 0x7fffffffu ? (int)value : -1 - (int)~value;
}

static void metadata_copy(unsigned char *dst, const unsigned char *src,
                             unsigned int count)
{
    unsigned int index;
    for (index = 0; index < count; ++index)
        dst[index] = src[index];
}

void md_parser_evt_handler_body(
    struct metadata_app_context *ctx, struct metadata_counters *counters,
    unsigned int event, const timestamp_t *pts, const void *header,
    const void *rpu, int rpu_size, const void *dm, int dm_size, int status,
    const struct metadata_callback_ops *ops)
{
    int error;
    int count;
    int previous;
    unsigned int index;
    volatile unsigned char *clear;
    (void)pts;
    (void)rpu_size;
    (void)dm_size;
    (void)status;
    counters->frame_nr += 1u;
    if (event == MD_PARSER_SAMPLE_AVAIL_EVT) {
        clear = ctx->main_cfg;
        for (index = 0; index < sizeof(ctx->main_cfg); ++index)
            clear[index] = 0;
        ops->compose(header, rpu, ctx->main_cfg);
        error = ops->validate_composer(ctx->main_cfg, counters->frame_nr,
                                       ctx->conf.pf_msg_log, 7);
        if (error == 0) {
            previous = counters->comp_size;
            if ((unsigned int)previous > 0x18f8u) {
                ops->logger("\001" "6comp_size too big\n");
                return;
            }
            metadata_copy((unsigned char *)ctx->comp_out + previous,
                             ctx->main_cfg, 1800u);
            counters->comp_size = previous + 1800;
            error = ops->validate_dm(dm, counters->frame_nr,
                                     ctx->conf.pf_msg_log, 7);
            if (error == 0) {
                count = 66626;
                error = ops->serialize_dm(dm, ctx->dm_byte_sequence, &count);
                if (error == 0) {
                    previous = counters->md_size;
                    if (metadata_signed(1024u - (unsigned int)count) < previous) {
                        ops->logger("\001" "6md_size too big\n");
                        return;
                    }
                    metadata_copy((unsigned char *)ctx->dm_out + previous,
                                     (const unsigned char *)ctx->dm_byte_sequence,
                                     (unsigned int)count);
                    counters->md_size = metadata_signed(
                        (unsigned int)previous + (unsigned int)count);
                    return;
                }
            }
        }
        ops->logger("\001" "6%s\n", rpu_error_code_2_str(error));
    } else if (event == MD_PARSER_ERROR_EVT) {
        error = dv_md_parser_get_last_err(ctx->h_md_parser);
        ops->logger("\001" "6%s\n", dv_md_parser_get_errstr(error));
    }
}

int metadata_parser_process_body(
    struct metadata_init_storage *storage,
    struct metadata_counters *counters, const char *input, int size,
    char *composer, int *composer_size, char *metadata, int *metadata_size,
    unsigned char eos, const struct metadata_callback_ops *ops)
{
    int index = metadata_signed((unsigned int)size - 1u);
    int limit = metadata_signed((unsigned int)size - 64u);
    int flags = 0;
    unsigned int length;
    struct metadata_app_context *ctx = &storage->ctx;
    counters->comp_size = 0;
    counters->md_size = 0;
    ctx->md_in = (char *)input;
    ctx->comp_out = composer;
    ctx->dm_out = metadata;
    while (index > limit && input[index] == 0)
        index = metadata_signed((unsigned int)index - 1u);
    length = (unsigned int)index - 4u;
    storage->rc = ops->process(ctx->h_md_parser, 0, 0,
                               (const unsigned char *)input + 5,
                               length, (int)eos, &flags);
    if (storage->rc == 0) {
        *composer_size = counters->comp_size;
        *metadata_size = counters->md_size;
        (void)ops->reset(ctx->h_md_parser);
        return flags;
    }
    ops->logger("\001" "6dv_md_parser_process %d:%02x,%02x (rc='%s') failed\n",
                metadata_signed(length), (unsigned char)input[5],
                (unsigned char)input[6], dv_md_parser_get_errstr(storage->rc));
    return -1;
}

int metadata_parser_reset_body(
    struct metadata_init_storage *storage, int flag,
    const struct metadata_callback_ops *ops)
{
    (void)flag;
    storage->rc = ops->reset(storage->ctx.h_md_parser);
    if (storage->rc == 0)
        return 0;
    ops->logger("\001" "6dv_md_parser_reset(rc='%s') failed\n",
                rpu_error_code_2_str(storage->rc));
    return -1;
}
