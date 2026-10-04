#include "host_formats.h"

int host_format_to_core(int format)
{
    if (format >= -1 && format <= 3)
        return format;
    if (format == 6)
        return 4;
    return HOST_FORMAT_UNSUPPORTED;
}

int core_format_to_host(int format)
{
    if (format >= -1 && format <= 3)
        return format;
    if (format == 4)
        return 6;
    return HOST_FORMAT_UNSUPPORTED;
}
