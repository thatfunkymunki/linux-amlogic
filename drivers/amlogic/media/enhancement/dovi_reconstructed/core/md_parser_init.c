/*
 * Behavioral reconstruction of parser creation and initialization.
 * Reference SHA-256:
 * 5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b
 */
#include "md_parser_init.h"
#include "rpu_init.h"
#include "rpu_setters.h"
#include "parser_diagnostics.h"

/* Use the context member to retain the original pointer alignment/type. */
static union {
    dv_md_parser_ctx_t context;
    unsigned char bytes[MD_PARSER_SINGLETON_BYTES];
} md_parser;

dv_md_parser_ctx_t *dv_md_parser_create(void)
{
    unsigned int index;
    for (index = 0; index < sizeof(dv_md_parser_ctx_t); ++index)
        md_parser.bytes[index] = 0;
    return &md_parser.context;
}

dv_md_parser_rc_t dv_md_parser_init(dv_md_parser_ctx_t *handle,
                                      const dv_md_parser_conf_t *config)
{
    int decoder_rc;

    if (handle == 0)
        return DV_MD_PARSER_INVALID_CONTEXT_HANDLE;

    handle->last_rc = DV_MD_PARSER_OK;
    handle->f_init = 0;
    handle->p_rpu_dcd_ctx =
        (struct rpu_decoder_ctx_s *)((unsigned char *)handle + 112u);
    handle->ui64_sample = 0;
    handle->pf_md_parser_cb = config->pf_md_parser_cb;
    handle->p_md_parser_cb_ctx = config->p_md_parser_cb_ctx;
    handle->flags = config->flags;
    handle->pf_msg_log = config->pf_msg_log;
    rpu_decoder_set_msg_log(
        (struct rpu_setter_view *)handle->p_rpu_dcd_ctx,
        config->pf_msg_log, S_RPU_NOTICE);

    /* The original rechecks f_init after the setter, even though its own
     * setter never changes this byte. Preserve the branch and call order.
     */
    if (handle->f_init != 0) {
        if (handle->pf_msg_log != 0)
            handle->pf_msg_log(
                "Dolby Vision metadata parser is already initialized\n");
        return DV_MD_PARSER_OK;
    }
    if (config->pf_md_parser_cb == 0) {
        if (config->pf_msg_log != 0)
            config->pf_msg_log(
                "Dolby Vision metadata parser configured event notification callack is NULL\n");
        handle->last_rc = DV_MD_PARSER_INVALID_ARGUMENT;
        return DV_MD_PARSER_INVALID_ARGUMENT;
    }
    decoder_rc = rpu_decoder_init(handle->p_rpu_dcd_ctx,
                                     (const unsigned char *)&config->flags);
    if (decoder_rc >= 0) {
        handle->f_init = 1;
        return DV_MD_PARSER_OK;
    }
    handle->last_rc = (dv_md_parser_rc_t)decoder_rc;
    if (handle->pf_msg_log != 0)
        handle->pf_msg_log("Error: rpu_decoder_init failed: %s\n",
                           rpu_error_code_2_str(decoder_rc));
    return (dv_md_parser_rc_t)decoder_rc;
}

void metadata_parser_release(void)
{
}
