#include <linux/kernel.h>
#include <linux/amlogic/cpu_version.h>

#include "runtime_bridge.h"
#include "core/control_runtime.h"
#include "core/metadata_runtime.h"

static int metadata_log(const char *format, ...)
{
    va_list arguments;
    int result;
    va_start(arguments, format);
    result = vprintk(format, arguments);
    va_end(arguments);
    return result;
}

static int control_log(const char *format, unsigned int length)
{
    return printk(format, length);
}

static int memory_log(const char *format, const char *file,
    const char *function, unsigned int line, const char *expression)
{
    return printk(format, file, function, line, expression);
}

static void trim_assertion(const char *file, const char *function,
    int line, const char *expression)
{
    pr_err("dovi reconstructed: %s:%d %s: %s\n", file, line,
           function, expression);
}

static int cpu_version(int level)
{
    return get_meson_cpu_version(level);
}

int dovi_runtime_configure(void)
{
    const struct control_runtime_services services = {
        .cpu_version = cpu_version,
        .logger = control_log,
        .memory = { .logger = memory_log, .dump_stack = dump_stack },
        .trim = { .assertion = trim_assertion, .dump_stack = dump_stack }
    };
    return control_runtime_init(&services);
}

void *dovi_metadata_init(int flag)
{
    return metadata_runtime_init(metadata_log, flag);
}

int dovi_metadata_reset(int flag)
{
    return metadata_runtime_reset(flag);
}

int dovi_metadata_process(char *input, int size, char *composer,
    int *composer_size, char *metadata, int *metadata_size, unsigned char eos)
{
    return metadata_runtime_process(input, size, composer, composer_size,
                                    metadata, metadata_size, eos);
}

void dovi_metadata_release(void)
{
    metadata_runtime_release();
}
