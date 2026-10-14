/*
 * Identity strings and forwarding getters recovered from s922x-j_dovi.ko.
 * Reference SHA-256: 5ffe2348b2e12d682543d275f7282fb43aedfdcc7aaa8339d229ddca6abd130b
 * These functions are separate from the 5.15 STB 2.6 parser identity.
 */
#include "parser_identity.h"

/* The reference returns one .rodata base and that base plus eight bytes. */
static const char version_strings[] = "0.4.1\0\0\0" "1.1.2";
static const char build_info[] =
    "Build configuration flags:\n"
    "\tENABLE_FLOAT\t\t0\n"
    "\tNUM_RPU_BUFFERS\t\t64\n";

const char *vdr_rpu_get_api_ver(void)
{
    return version_strings;
}

const char *vdr_rpu_get_algo_ver(void)
{
    return version_strings + 8;
}

const char *vdr_rpu_get_build_info(void)
{
    return build_info;
}

const char *dv_md_parser_get_api_ver(void)
{
    return vdr_rpu_get_api_ver();
}

const char *dv_md_parser_get_algo_ver(void)
{
    return vdr_rpu_get_algo_ver();
}

const char *dv_md_parser_get_build_info(void)
{
    return vdr_rpu_get_build_info();
}
