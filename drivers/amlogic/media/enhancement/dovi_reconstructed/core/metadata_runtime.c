/* Bind the recovered host callback shapes to one concrete static pipeline. */
#include "metadata_runtime.h"

static struct metadata_pipeline metadata_state;

struct metadata_app_context *metadata_runtime_init(msg_log_func_t logger,
                                                  int flag)
{
    return metadata_pipeline_init(&metadata_state, logger, flag);
}

int metadata_runtime_reset(int flag)
{
    return metadata_pipeline_reset(&metadata_state, flag);
}

int metadata_runtime_process(const char *input, int size, char *composer,
    int *composer_size, char *metadata, int *metadata_size, unsigned char eos)
{
    return metadata_pipeline_process(&metadata_state, input, size, composer,
        composer_size, metadata, metadata_size, eos);
}

void metadata_runtime_release(void)
{
    metadata_parser_release();
}
