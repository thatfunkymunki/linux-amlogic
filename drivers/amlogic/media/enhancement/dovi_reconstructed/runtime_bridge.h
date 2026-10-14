#ifndef DOVI_RUNTIME_BRIDGE_H
#define DOVI_RUNTIME_BRIDGE_H

/* Keep the recovered parser declarations separate from the host declarations. */
int dovi_runtime_configure(void);
void *dovi_metadata_init(int flag);
int dovi_metadata_reset(int flag);
int dovi_metadata_process(char *input, int size, char *composer,
    int *composer_size, char *metadata, int *metadata_size, unsigned char eos);
void dovi_metadata_release(void);

#endif
