#ifndef AM6B_BYTE_UTILS_H
#define AM6B_BYTE_UTILS_H

/* Recovered signatures, with an prefix for standalone linking. */
void mem_cpy_byte(void *destination, const void *source, int length);
int mem_eq_byte(const void *first, const void *second, int length);
int abs_int32(int value);

#endif
