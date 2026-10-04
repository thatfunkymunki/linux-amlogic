/* Behavioral reconstruction of dv_md_parser_reset. */
#include "md_parser_reset.h"
#include "rpu_lifecycle.h"

dv_md_parser_rc_t dv_md_parser_reset(dv_md_parser_ctx_t *h_md_parser)
{
    if (h_md_parser == 0)
        return DV_MD_PARSER_INVALID_CONTEXT_HANDLE;
    if (h_md_parser->f_init == 0)
        return DV_MD_PARSER_UNINITIALIZE;
    if (h_md_parser->p_rpu_dcd_ctx != 0)
        rpu_decoder_reset(
            (struct rpu_decoder_reset_view *)h_md_parser->p_rpu_dcd_ctx);
    h_md_parser->ui64_sample = 0;
    return DV_MD_PARSER_OK;
}

dv_md_parser_rc_t dv_md_parser_destroy(dv_md_parser_ctx_t *h_md_parser)
{
    if (h_md_parser == 0)
        return DV_MD_PARSER_INVALID_CONTEXT_HANDLE;
    if (h_md_parser->p_rpu_dcd_ctx != 0) {
        rpu_decoder_cleanup(
            (struct rpu_decoder_reset_view *)h_md_parser->p_rpu_dcd_ctx);
        h_md_parser->p_rpu_dcd_ctx = 0;
    }
    h_md_parser->f_init = 0;
    return DV_MD_PARSER_OK;
}
