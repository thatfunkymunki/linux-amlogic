/*
 * metadata_parser_init callback-body recovery from the pinned module.
 * Reference SHA-256:
 * 5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b
 */
#include "metadata_init_body.h"
#include "memory_query.h"
#include "parser_diagnostics.h"

static void metadata_clear(void *memory, unsigned int bytes)
{
    unsigned char *out = (unsigned char *)memory;
    unsigned int index;
    for (index = 0; index < bytes; ++index)
        out[index] = 0;
}

struct metadata_app_context *metadata_parser_init_body(
    struct metadata_init_storage *storage,
    md_parser_evt_cb_func_t event_handler, msg_log_func_t logger, int flag)
{
    struct metadata_app_context *ctx = &storage->ctx;
    (void)flag;
    metadata_clear(ctx, 1920u);
    ctx->rpu_data_header = (struct rpu_data_header_s *)storage->rpu_data_header;
    metadata_clear(storage->rpu_data_header, 310u);
    ctx->vdr_rpu_data_payload =
        (struct vdr_rpu_data_payload_s *)storage->vdr_rpu_data_payload;
    metadata_clear(storage->vdr_rpu_data_payload, 28728u);
    ctx->vdr_dm_data_payload =
        (struct vdr_dm_data_payload_s *)storage->vdr_dm_data_payload;
    metadata_clear(storage->vdr_dm_data_payload, 67396u);
    ctx->dm_byte_sequence = (char *)storage->dm_byte_sequence;
    metadata_clear(storage->dm_byte_sequence, 66626u);
    ctx->conf.flags.profile = RPU_MAIN_PROFILE;
    ctx->conf.flags.payload_only = 1;
    (void)md_parser_mem_query((const unsigned char *)&ctx->conf);
    ctx->h_md_parser = dv_md_parser_create();
    if (ctx->h_md_parser == 0) {
        logger("\001" "6Error creating Dolby Vision metadata parser instance\n");
        return 0;
    }
    ctx->conf.pf_md_parser_cb = event_handler;
    ctx->conf.pf_msg_log = logger;
    ctx->conf.p_md_parser_cb_ctx = ctx;
    storage->rc = (int)dv_md_parser_init(ctx->h_md_parser, &ctx->conf);
    if (storage->rc != DV_MD_PARSER_OK) {
        logger("\001" "6dv_md_parser_init(rc='%s') failed)\n",
               dv_md_parser_get_errstr(storage->rc));
        return 0;
    }
    logger("\001" "6Creating Dolby Vision metadata parser instance success\n");
    return ctx;
}
