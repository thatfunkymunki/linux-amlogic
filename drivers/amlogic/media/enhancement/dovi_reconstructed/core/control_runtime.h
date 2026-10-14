/* Owned singleton binding for the recovered control/register/DM closure. */
#ifndef CONTROL_RUNTIME_H
#define CONTROL_RUNTIME_H

#include "control_path.h"
#include "kernel_maps.h"

struct control_runtime_services {
    int (*cpu_version)(int level);
    control_metadata_logger logger;
    struct control_memory_ops memory;
    KernelTrimOps trim;
};

/* Configure once before the host publishes callbacks. Services are copied;
 * their function addresses must stay valid. The original control core and
 * chroma scratch state are singletons: calls must not overlap or reenter.
 * This does not allocate frame buffers: init_cp records the original opaque
 * memory request, as in the reference callback. No CPU image processing is
 * performed by this hardware-register path.
 */
int control_runtime_init(const struct control_runtime_services *services);

/* Same 17 machine arguments as the historical callback. Formats use the
 * recovered enum; host_control.h translates the T4c enum explicitly. Buffer
 * requirements are those of control_path.h. Output is the 13468-byte prefix.
 * FORMAT_INVALID (-1) retains the original reset behavior.
 */
int control_runtime_process(int input_format, int output_format,
    const char *composition, int composition_size, const char *metadata,
    int metadata_size, int priority, int bit_depth, int chroma_format,
    int yuv_range, int graphics_min, int graphics_max, int target_min,
    int target_max, int no_el, const void *hdr10, void *output);

#endif
