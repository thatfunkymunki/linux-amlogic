#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/amlogic/media/amvecm/amvecm.h>
#include <linux/amlogic/media/amdolbyvision/dolby_vision.h>
#include <linux/amlogic/media/vpu/vpu.h>
#include "amdolby_vision.h"

#include "runtime_bridge.h"
#include "core/host_control.h"
#include "core/control_output_abi.h"

#define ABI_OFFSET(type, member, expected) \
    _Static_assert(offsetof(type, member) == (expected), #member " offset")

_Static_assert(sizeof(void *) == 8, "T4c requires AArch64 pointers");
_Static_assert(FORMAT_INVALID == -1 && FORMAT_DOVI == 0 && FORMAT_HDR10 == 1 &&
               FORMAT_SDR == 2 && FORMAT_DOVI_LL == 3 && FORMAT_SDR_2020 == 6,
               "T4c host signal formats");
_Static_assert(sizeof(struct dolby_vision_func_s) == 48, "T4c callback table");
ABI_OFFSET(struct dolby_vision_func_s, version_info, 0);
ABI_OFFSET(struct dolby_vision_func_s, metadata_parser_init, 8);
ABI_OFFSET(struct dolby_vision_func_s, metadata_parser_reset, 16);
ABI_OFFSET(struct dolby_vision_func_s, metadata_parser_process, 24);
ABI_OFFSET(struct dolby_vision_func_s, metadata_parser_release, 32);
ABI_OFFSET(struct dolby_vision_func_s, control_path, 40);
_Static_assert(sizeof(struct dovi_setting_s) == 13480, "T4c output size");
ABI_OFFSET(struct dovi_setting_s, comp_reg, CONTROL_OUTPUT_COMP_REG_OFFSET);
ABI_OFFSET(struct dovi_setting_s, dm_reg1, CONTROL_OUTPUT_DM_REG1_OFFSET);
ABI_OFFSET(struct dovi_setting_s, dm_reg2, CONTROL_OUTPUT_DM_REG2_OFFSET);
ABI_OFFSET(struct dovi_setting_s, dm_reg3, CONTROL_OUTPUT_DM_REG3_OFFSET);
ABI_OFFSET(struct dovi_setting_s, dm_lut1, CONTROL_OUTPUT_DM_LUT1_OFFSET);
ABI_OFFSET(struct dovi_setting_s, dm_lut2, CONTROL_OUTPUT_DM_LUT2_OFFSET);
ABI_OFFSET(struct dovi_setting_s, md_reg3, CONTROL_OUTPUT_MD_REG3_OFFSET);
ABI_OFFSET(struct dovi_setting_s, hdr_info, CONTROL_OUTPUT_HDR_INFO_OFFSET);
ABI_OFFSET(struct dovi_setting_s, src_format, CONTROL_OUTPUT_SRC_FORMAT_OFFSET);
ABI_OFFSET(struct dovi_setting_s, dst_format, CONTROL_OUTPUT_DST_FORMAT_OFFSET);
ABI_OFFSET(struct dovi_setting_s, el_flag, CONTROL_OUTPUT_EL_FLAG_OFFSET);
ABI_OFFSET(struct dovi_setting_s, el_halfsize_flag, CONTROL_OUTPUT_EL_HALFSIZE_FLAG_OFFSET);
ABI_OFFSET(struct dovi_setting_s, video_width, CONTROL_OUTPUT_VIDEO_WIDTH_OFFSET);
ABI_OFFSET(struct dovi_setting_s, video_height, CONTROL_OUTPUT_VIDEO_HEIGHT_OFFSET);
ABI_OFFSET(struct dovi_setting_s, g_format, CONTROL_OUTPUT_G_FORMAT_OFFSET);
ABI_OFFSET(struct dovi_setting_s, g_bitdepth, CONTROL_OUTPUT_G_BITDEPTH_OFFSET);
ABI_OFFSET(struct dovi_setting_s, dovi2hdr10_nomapping, CONTROL_OUTPUT_DOVI2HDR10_NOMAPPING_OFFSET);
ABI_OFFSET(struct dovi_setting_s, use_ll_flag, CONTROL_OUTPUT_USE_LL_FLAG_OFFSET);
ABI_OFFSET(struct dovi_setting_s, ll_rgb_desired, CONTROL_OUTPUT_LL_RGB_DESIRED_OFFSET);
ABI_OFFSET(struct dovi_setting_s, diagnostic_enable, CONTROL_OUTPUT_DIAGNOSTIC_ENABLE_OFFSET);
ABI_OFFSET(struct dovi_setting_s, diagnostic_mux_select, CONTROL_OUTPUT_DIAGNOSTIC_MUX_SELECT_OFFSET);
ABI_OFFSET(struct dovi_setting_s, dovi_ll_enable, CONTROL_OUTPUT_DOVI_LL_ENABLE_OFFSET);
ABI_OFFSET(struct dovi_setting_s, vout_width, CONTROL_OUTPUT_VOUT_WIDTH_OFFSET);
ABI_OFFSET(struct dovi_setting_s, vout_height, CONTROL_OUTPUT_VOUT_HEIGHT_OFFSET);
ABI_OFFSET(struct dovi_setting_s, vsvdb_tbl, CONTROL_OUTPUT_VSVDB_TBL_OFFSET);
ABI_OFFSET(struct dovi_setting_s, ext_md, CONTROL_OUTPUT_EXT_MD_OFFSET);
ABI_OFFSET(struct dovi_setting_s, vsvdb_len, CONTROL_OUTPUT_BYTES);
ABI_OFFSET(struct dovi_setting_s, vsvdb_changed, CONTROL_OUTPUT_BYTES + 4);
ABI_OFFSET(struct dovi_setting_s, mode_changed, CONTROL_OUTPUT_BYTES + 8);
_Static_assert(sizeof(struct hdr10_parameter) == CONTROL_HDR10_BYTES,
               "T4c HDR10 input size");
ABI_OFFSET(struct hdr10_parameter, min_display_mastering_lum, CONTROL_HDR10_MIN_DISPLAY_MASTERING_LUMINANCE_OFFSET);
ABI_OFFSET(struct hdr10_parameter, max_display_mastering_lum, CONTROL_HDR10_MAX_DISPLAY_MASTERING_LUMINANCE_OFFSET);
ABI_OFFSET(struct hdr10_parameter, r_x, CONTROL_HDR10_RX_OFFSET);
ABI_OFFSET(struct hdr10_parameter, r_y, CONTROL_HDR10_RY_OFFSET);
ABI_OFFSET(struct hdr10_parameter, g_x, CONTROL_HDR10_GX_OFFSET);
ABI_OFFSET(struct hdr10_parameter, g_y, CONTROL_HDR10_GY_OFFSET);
ABI_OFFSET(struct hdr10_parameter, b_x, CONTROL_HDR10_BX_OFFSET);
ABI_OFFSET(struct hdr10_parameter, b_y, CONTROL_HDR10_BY_OFFSET);
ABI_OFFSET(struct hdr10_parameter, w_x, CONTROL_HDR10_WX_OFFSET);
ABI_OFFSET(struct hdr10_parameter, w_y, CONTROL_HDR10_WY_OFFSET);
ABI_OFFSET(struct hdr10_parameter, max_content_light_level, CONTROL_HDR10_MAX_CONTENT_LIGHT_LEVEL_OFFSET);
ABI_OFFSET(struct hdr10_parameter, max_frame_avg_light_level, CONTROL_HDR10_MAX_PIC_AVERAGE_LIGHT_LEVEL_OFFSET);
_Static_assert(sizeof(struct ext_md_s) == CONTROL_OUTPUT_EXT_BYTES, "T4c extension size");
ABI_OFFSET(struct ext_md_s, avail_level_mask, CONTROL_OUTPUT_EXT_AVAILABLE_LEVEL_MASK_OFFSET);
ABI_OFFSET(struct ext_md_s, level_1, CONTROL_OUTPUT_EXT_LEVEL_1_OFFSET);
ABI_OFFSET(struct ext_md_s, level_2, CONTROL_OUTPUT_EXT_LEVEL_2_OFFSET);
ABI_OFFSET(struct ext_md_s, level_4, CONTROL_OUTPUT_EXT_LEVEL_4_OFFSET);
ABI_OFFSET(struct ext_md_s, level_5, CONTROL_OUTPUT_EXT_LEVEL_5_OFFSET);
ABI_OFFSET(struct ext_md_s, level_6, CONTROL_OUTPUT_EXT_LEVEL_6_OFFSET);
ABI_OFFSET(struct ext_md_s, level_255, CONTROL_OUTPUT_EXT_LEVEL_255_OFFSET);

static int metadata_process(char *input, int size, char *composer,
    int *composer_size, char *metadata, int *metadata_size, bool eos)
{
    return dovi_metadata_process(input, size, composer, composer_size,
                                 metadata, metadata_size, (unsigned char)eos);
}

static int control_process(enum signal_format_enum input_format,
    enum signal_format_enum output_format, char *composition,
    int composition_size, char *metadata, int metadata_size,
    enum priority_mode_enum priority, int bit_depth, int chroma_format,
    int yuv_range, int graphics_min, int graphics_max, int target_min,
    int target_max, int no_el, struct hdr10_parameter *hdr10,
    struct dovi_setting_s *output)
{
    return host_control_process((int)input_format, (int)output_format,
        composition, composition_size, metadata, metadata_size, (int)priority,
        bit_depth, chroma_format, yuv_range, graphics_min, graphics_max,
        target_min, target_max, no_el, hdr10, output);
}

/* The provider retains this pointer and does not protect readers on removal. */
static const struct dolby_vision_func_s callbacks = {
    .version_info = "dovi reconstructed P3i T4c scalar core",
    .metadata_parser_init = dovi_metadata_init,
    .metadata_parser_reset = dovi_metadata_reset,
    .metadata_parser_process = metadata_process,
    .metadata_parser_release = dovi_metadata_release,
    .control_path = control_process
};

static int __init reconstructed_init(void)
{
    int result = dovi_runtime_configure();
    if (result != 0)
        return result;
    return register_dv_functions(&callbacks);
}

module_init(reconstructed_init);
/* No module_exit: the T4c provider has no callback-reader lifetime protocol. */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Reconstructed scalar Dolby Vision core for P3i T4c");
MODULE_VERSION("0.1");
