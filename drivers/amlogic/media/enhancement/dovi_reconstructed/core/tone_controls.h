#ifndef DOVI_TONE_CONTROLS_H
#define DOVI_TONE_CONTROLS_H

/* DWARF-sized inputs: TmCtrl168, DmCtrl104, TcCtrl168, TcSig28,
 * TgtSigEnv204, DmKsTMap4104 bytes. Functions retain all unrelated bytes. */
struct tone_curve_source {
    int crush, mid, clip, min_pq, max_pq, key_weight;
    int trim_slope, trim_offset, trim_power, chroma_weight;
    int saturation_gain, diagonal_inches, intensity_weight, chroma_vector_weight;
};
struct tone_curve_display {
    int min_pq, max_pq, min_bias, mid_bias, max_bias;
    int contrast, brightness, diagonal_inches, bright_preserve, rolloff, rolloff_inverse;
    unsigned short mode;
};
struct tone_curve_pair { int value; unsigned short norm; };
struct tone_curve_ops {
    void *opaque;
    /* Optional explicit builder for dependency audits. A null ops pointer or
     * build callback selects the concrete recovered builder. All pointers
     * valid; ratio is null, lut_size512. Local structure padding unspecified. */
    void (*build)(void *opaque, struct tone_curve_source *source,
                  struct tone_curve_display *display, struct tone_curve_pair *param1,
                  struct tone_curve_pair *param2, struct tone_curve_pair *param3,
                  int *slope_over_rolloff, short *tone_i, short *tone_s,
                  short *saturation_i, short *saturation_s, int *ratio,
                  short lut_size, short clamp_output);
};
void clamp_level2(int *level2);
void update_tc_ctrl_level(const void *tone_mapping, void *tone_curve);
void tm_ctrl_to_tc_ctrl(const void *tone_mapping, const void *dm_control,
                        void *tone_curve, int graphics_source);
void target_signal_to_tc_ctrl(const void *target_signal, void *tone_curve);
void update_tc_signal_hlg_a(int minimum, int maximum, void *tone_signal);
int log10_taylor(int value);
int sdiv_sls(long long numerator, int denominator);
unsigned int de_gamma(unsigned short a, unsigned short b, unsigned short gamma,
                      unsigned int gain, unsigned short y);
void calc_ta_tb(int gamma, unsigned int maximum, unsigned int minimum, int *ta, int *tb);
/* Optimized original has three object arguments; the obsolete fourth DWARF
 * argument is unused. Compare/copy covers160 bytes out of TcCtrl's168. */
int create_tone_curve(const void *tone_curve, void *kernel_tmap, void *used_curve,
                      const struct tone_curve_ops *ops);
int create_tone_curve_changed(const void *tone_curve, void *kernel_tmap, void *used_curve,
                              const struct tone_curve_ops *ops);
#endif
