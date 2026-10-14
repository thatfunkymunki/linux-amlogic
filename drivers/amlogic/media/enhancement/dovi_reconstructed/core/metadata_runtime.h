/* Singleton runtime bridge for the recovered metadata callback ABI. */
#ifndef METADATA_RUNTIME_H
#define METADATA_RUNTIME_H

#include "metadata_bindings.h"

/* One static, initially zeroed pipeline shares the recovered parser singleton.
 * These entry points are non-reentrant; callers must serialize init, reset,
 * process and release. The logger must be nonnull, stable and non-reentrant.
 * Init returns the original app-context pointer and preserves counters. Flags
 * retain the recovered ignored-argument behavior. Release is the recovered
 * empty callback: it does not destroy, free or reset either singleton.
 *
 * Process retains metadata_bindings.h's unchecked buffer requirements:
 * composer has 8192 writable bytes and metadata has 1024; input, outputs and
 * length slots are disjoint from the pipeline and each other. Supply readable
 * input/refill padding and a nonzero payload byte. Payload-only decoder ingress
 * uses one 4096-byte slot. A successful call publishes lengths and resets the
 * parser; failure returns -1 and leaves the caller's lengths unchanged.
 * No allocation, locking, ownership or source-status policy is added here.
 */
struct metadata_app_context *metadata_runtime_init(msg_log_func_t logger,
                                                  int flag);
int metadata_runtime_reset(int flag);
int metadata_runtime_process(const char *input, int size, char *composer,
    int *composer_size, char *metadata, int *metadata_size, unsigned char eos);
void metadata_runtime_release(void);

#endif
