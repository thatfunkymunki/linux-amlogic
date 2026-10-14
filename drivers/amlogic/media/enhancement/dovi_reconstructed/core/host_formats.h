#ifndef HOST_FORMATS_H
#define HOST_FORMATS_H

/* T4c FORMAT_INVALID is -1; this separate result means no core mapping. */
#define HOST_FORMAT_UNSUPPORTED (-2)

/* The pinned T4c host uses 6 for SDR BT.2020; the recovered core uses 4.
 * Invalid, Dolby Vision, HDR10, SDR and Dolby Vision LL retain -1..3.
 * Other host/core values have no supported translation. */
int host_format_to_core(int format);
int core_format_to_host(int format);

#endif
