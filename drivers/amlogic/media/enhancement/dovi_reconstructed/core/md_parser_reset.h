/* parser reset wrapper recovered from dv_md_parser.c DWARF. */
#ifndef AM6B_MD_PARSER_RESET_H
#define AM6B_MD_PARSER_RESET_H

#include "../dv_md_parser_abi_recovered.h"

dv_md_parser_rc_t dv_md_parser_reset(dv_md_parser_ctx_t *h_md_parser);
dv_md_parser_rc_t dv_md_parser_destroy(dv_md_parser_ctx_t *h_md_parser);

#endif
