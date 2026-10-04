#ifndef AM6B_LUMINANCE_PQ_H
#define AM6B_LUMINANCE_PQ_H

/* Decoder-local LToPQ12 (.text+0x16840); the control path has another copy. */
unsigned int luminance_to_pq12(unsigned int value);

#endif
