/* Canonical interfaces used by the display-management implementations. */
#ifndef DOVI_DM_BINDINGS_H
#define DOVI_DM_BINDINGS_H
#include "fixed_access.h"
#include "dm_pipeline.h"
#include "dm_configuration.h"
#include "dm_frame_format.h"
#include "dm_transfer.h"
#include "color_basis.h"
#include "matrix_arithmetic.h"
#include "color_narrow.h"
#include "color_primaries.h"
#include "numeric_color_matrix.h"
#include "fixed_arithmetic.h"
#include "fixed_transcendentals.h"
#include "pq_conversion.h"
#include "numeric_transfer.h"
#include "numeric_helpers.h"
#include "tone_curve.h"
#include "tone_controls.h"
#include "byte_utils.h"

static inline int dm_pair(unsigned short lo, unsigned short hi)
{
    return fixed_signed32((unsigned int)lo | ((unsigned int)hi << 16));
}

#define MemCpyByte dovi_MemCpyByte
#define LToPQ12 dovi_LToPQ12
#define GetLms2IptM33Fxp dovi_GetLms2IptM33Fxp
#define GetIpt2LmsM33Fxp dovi_GetIpt2LmsM33Fxp
#define InvM33 dovi_InvM33
#define AssignM33I2I dovi_AssignM33I2I
#define AssignV3I2I dovi_AssignV3I2I
#define Yuv2RgbOffsetRgbFxp dovi_Yuv2RgbOffsetRgbFxp
#define GetRgb2LmsCtWpM33Fxp dovi_GetRgb2LmsCtWpM33Fxp
#define GetYuv2RgbM33NarrowFxp dovi_GetYuv2RgbM33NarrowFxp
#define GetYuv2RgbM33Fxp dovi_GetYuv2RgbM33Fxp
#define GetYuvRgbOffFxp dovi_GetYuvRgbOffFxp
#define GetRange dovi_GetRange
#define CreateToneCurve dovi_CreateToneCurve

#define CheckMds dovi_dm_CheckMds
#define CommitMdsCsc dovi_dm_CommitMdsCsc
#define SigEnv2KsFrmFmt dovi_dm_CommitCdm2KsFrmFmt
#define CommitIptKsIMap dovi_dm_CommitIpt2KsIMap
#define calcTaTb calc_ta_tb
#define fxPOW2Taylor dovi_fxPOW2Taylor
#define DeGamma dovi_dm_DeGamma
#define GetRgb2LmsByDefM33 dovi_GetRgb2LmsByDefM33
#define GetRgb2LmsByPrimsM33Fxp dovi_GetRgb2LmsByPrimsM33Fxp
#define GetIptNormParam dovi_GetCvmBypassNormParam
#define TgtSigEnv2TcCtrl target_signal_to_tc_ctrl
#define UpdateTcSigHlgA update_tc_signal_hlg_a
#define TmCtrl2TcCtrl tm_ctrl_to_tc_ctrl
#define UpdateTcCtrlLevel update_tc_ctrl_level
#define CommitKsOMap dovi_dm_CommitSigEnv2KsOMap
#define CommitUs dovi_CommitKsUs
#define CommitTrim dovi_dm_CommitTrim

#endif
