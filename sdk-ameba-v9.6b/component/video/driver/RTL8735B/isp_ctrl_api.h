#ifndef _ISP_CTRL_API_H_
#define _ISP_CTRL_API_H_

struct ctrl_range {
	int min;
	int max;
};


void isp_ctrl_enable_log(int enable);
void isp_set_brightness(int val);
void isp_get_brightness(int *pval);
void isp_set_contrast(int val);
void isp_get_contrast(int *pval);
void isp_set_saturation(int val);
void isp_get_saturation(int *pval);
void isp_set_awb_ctrl(int val);
void isp_get_awb_ctrl(int *pval);
void isp_set_red_balance(int val);
void isp_get_red_balance(int *pval);
void isp_set_blue_balance(int val);
void isp_get_blue_balance(int *pval);
void isp_set_gamma(int val);
void isp_get_gamma(int *pval);
void isp_set_exposure_time(int val);
void isp_get_exposure_time(int *pval);
void isp_set_exposure_mode(int val);
void isp_get_exposure_mode(int *pval);
void isp_set_ae_gain(int val);
void isp_get_ae_gain(int *pval);
void isp_set_power_line_freq(int val);
void isp_get_power_line_freq(int *pval);
void isp_set_wb_temperature(int val);
void isp_get_wb_temperature(int *pval);
void isp_set_sharpness(int val);
void isp_get_sharpness(int *pval);
void isp_set_green_balance(int val);
void isp_get_green_balance(int *pval);
void isp_set_day_night(int val);
void isp_get_day_night(int *pval);
void isp_set_dynamic_iq(int val);
void isp_get_dynamic_iq(int *pval);
void isp_set_tnr(int val);
void isp_get_tnr(int *pval);
void isp_set_ldc(int val);
void isp_get_ldc(int *pval);
void isp_set_gray_mode(int val);
void isp_get_gray_mode(int *pval);
void isp_set_wdr_mode(int val);
void isp_get_wdr_mode(int *pval);
void isp_set_wdr_level(int val);
void isp_get_wdr_level(int *pval);
void isp_set_orientation(int val);
void isp_get_orientation(int *pval);
void isp_set_max_fps(int val);
void isp_get_max_fps(int *pval);
void isp_set_min_fps(int val);
void isp_get_min_fps(int *pval);

#endif