/* Packed control metadata and HDR infoframe helpers from the pinned T4c ELF. */
#ifndef CONTROL_INFOFRAME_H
#define CONTROL_INFOFRAME_H

/* All objects use the binary's recovered opaque layout. Distinct objects are
 * required. ctx is a cp_context_t; target is target_param_t; hdr10 is the
 * 28-byte hdr10_param_t; dm is the packed 71-byte base followed by count
 * 261-byte extension records. infoframe has 29 writable bytes. Unrecognized
 * input formats change only the first five infoframe bytes. */
int get_info_frame(const void *ctx, const void *target, int input_format,
                   const void *hdr10, const void *dm, void *infoframe);

/* Uses ctx->h_dm. This caller requires a valid DM configuration: the original
 * ignores GetStbTmm failure and otherwise copies uninitialized local PQ words.
 * Only bytes 0..70 of the packed metadata base are changed. */
void get_fixed_metadata(const void *ctx, void *dm);

/* Full GetStbTmm recovery. Outputs are optional and are written in this order:
 * min_pq, min, max_pq, max. Null or invalid dm returns -1 without output writes.
 * Valid dm requires readable pDmCfgAct/hDmKs for requested luminance outputs. */
int control_get_stb_tmm(unsigned short *min_pq, unsigned int *min,
                        unsigned short *max_pq, unsigned int *max,
                        const void *dm);

#endif
