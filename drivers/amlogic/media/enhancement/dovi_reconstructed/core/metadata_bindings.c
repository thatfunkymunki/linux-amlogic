/* Connect the recovered callback bodies to their recovered dependencies. */
#include "metadata_bindings.h"
#include "composer_conversion.h"
#include "composer_validation.h"
#include "dm_serialization.h"
#include "parser_bindings.h"
#include "md_parser_reset.h"

static int process_parser(dv_md_parser_ctx_t *parser, unsigned int au_flags,
    const timestamp_t *pts, const unsigned char *input, unsigned int size,
    int last, int *flags)
{
    return (int)dv_md_parser_process_bound(parser, au_flags, pts, input,
                                          size, last, flags);
}

static int reset_parser(dv_md_parser_ctx_t *parser)
{
    return (int)dv_md_parser_reset(parser);
}

static struct metadata_callback_ops callback_ops(msg_log_func_t logger)
{
    struct metadata_callback_ops ops = {
        comp_rpu_2_main_cfg,
        validate_composer_config,
        validate_dm_payload,
        dm_rpu_payload_2_byte_sequence,
        process_parser,
        reset_parser,
        logger
    };
    return ops;
}

void metadata_pipeline_event(void *context, dv_md_parser_evt_t event,
    timestamp_t *pts, struct rpu_data_header_s *header,
    struct vdr_rpu_data_payload_s *rpu, int rpu_size,
    struct vdr_dm_data_payload_s *dm, int dm_size, int status)
{
    struct metadata_pipeline *pipeline = (struct metadata_pipeline *)context;
    const struct metadata_callback_ops ops = callback_ops(pipeline->logger);
    md_parser_evt_handler_body(&pipeline->storage.ctx, &pipeline->counters,
        (unsigned int)event, pts, header, rpu, rpu_size, dm, dm_size, status,
        &ops);
}

struct metadata_app_context *metadata_pipeline_init(
    struct metadata_pipeline *pipeline, msg_log_func_t logger, int flag)
{
    pipeline->logger = logger;
    return metadata_parser_init_body(&pipeline->storage,
                                     metadata_pipeline_event, logger, flag);
}

int metadata_pipeline_process(struct metadata_pipeline *pipeline,
    const char *input, int size, char *composer, int *composer_size,
    char *metadata, int *metadata_size, unsigned char eos)
{
    const struct metadata_callback_ops ops = callback_ops(pipeline->logger);
    return metadata_parser_process_body(&pipeline->storage, &pipeline->counters,
        input, size, composer, composer_size, metadata, metadata_size, eos, &ops);
}

int metadata_pipeline_reset(struct metadata_pipeline *pipeline, int flag)
{
    const struct metadata_callback_ops ops = callback_ops(pipeline->logger);
    return metadata_parser_reset_body(&pipeline->storage, flag, &ops);
}
