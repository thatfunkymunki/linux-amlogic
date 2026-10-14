/* Concrete parser dispatch with independently recovered decoder stages. */
#ifndef AM6B_PARSER_BINDINGS_H
#define AM6B_PARSER_BINDINGS_H

#include "../dv_md_parser_abi_recovered.h"

/* Original parser arguments, with concrete decoder dependencies. These
 * entry points leave the injectable md_parser_process interfaces available.
 * The parser callback and decoder/input backing retain their original
 * unchecked caller contracts. This is separate from the T4c host adapter. */
dv_md_parser_rc_t dv_md_parser_process_bound(
    dv_md_parser_ctx_t *parser, unsigned int au_flags, const timestamp_t *pts,
    const unsigned char *buffer, unsigned int size, int last, int *flags);

dv_md_parser_rc_t dv_md_parser_process_t35_bound(
    dv_md_parser_ctx_t *parser, unsigned int au_flags, const timestamp_t *pts,
    const unsigned char *buffer, unsigned int size);

#endif
