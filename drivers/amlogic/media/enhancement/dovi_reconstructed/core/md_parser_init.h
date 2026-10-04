/* Parser creation/initialization from the pinned CoreELEC module. */
#ifndef AM6B_MD_PARSER_INIT_H
#define AM6B_MD_PARSER_INIT_H

#include "../dv_md_parser_abi_recovered.h"

/* Original .bss md_parser object: context followed by decoder storage.
 * create clears only the 112-byte parser prefix and always returns this
 * singleton. The fixed singleton is suitable for the module's level-0,
 * payload-only callback configuration; arbitrary flags need their own arena.
 */
#define MD_PARSER_SINGLETON_BYTES 334048u
dv_md_parser_ctx_t *dv_md_parser_create(void);

/* A nonnull handle needs decoder backing storage at handle+112. Config and
 * handle must not overlap. A null handle returns 2 without reading config.
 */
dv_md_parser_rc_t dv_md_parser_init(dv_md_parser_ctx_t *handle,
                                      const dv_md_parser_conf_t *config);

/* The historical callback is an empty function, not parser destruction. */
void metadata_parser_release(void);

#endif
