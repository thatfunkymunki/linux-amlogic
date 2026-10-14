#ifndef AM6B_ST2094_H
#define AM6B_ST2094_H

/* Decoder-local T35/ST2094 parsing from the binary. Valid decoder/root,
 * header, mapping payload, 264-byte extension records and padded bitstream
 * storage are required. Count/index byte wraps and appended records retain
 * the original unchecked backing contract. See st2094.recovery.md.
 */
int st2094_10_data_read(void *decoder);
int rpu_decoder_push_2094_payload(void *decoder,
                                    const unsigned char *buffer,
                                    unsigned int size);
#endif
