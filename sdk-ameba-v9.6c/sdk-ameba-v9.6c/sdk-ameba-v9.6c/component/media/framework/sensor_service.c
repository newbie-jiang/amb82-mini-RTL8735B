#include "sensor_service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "platform_stdlib.h"
#include "isp_api.h"
#include "isp_ctrl_api.h"

#define SW_ALS 0
#define HW_ALS 1
#define ALS_TYPE HW_ALS
#define SS_DELAY 3000
#define ss_dprintf(level, ...) if(level <= ss_dbg_level) printf(__VA_ARGS__)

static int ss_dbg_level = 0;
static int en_auto_ir = 0;
static int sw_lux = 0;
static int ir_brightness = 0;

int sensor_external_set_gray_mode(int enable, int led_level)
{
	int ret = 0;
	if (enable == ALS_MODE_RGB) {
		ss_dprintf(1, "[SensorService] Switch to RGB Mode\n");
		ir_ctrl_set_brightness_d(led_level);
		ir_cut_enable(1 - enable);
		isp_set_day_night(enable);
		isp_get_day_night(&ret);
		if (ret != enable) {
			isp_set_day_night(enable);
		}
		vTaskDelay(200);
		isp_set_gray_mode(enable);
		if (ret != enable) {
			isp_set_gray_mode(enable);
		}
	} else if (enable == ALS_MODE_IR_Entry) {
		ss_dprintf(1, "[SensorService] Switch to IR Mode\n");
		isp_set_gray_mode(enable);
		isp_get_day_night(&ret);
		if (ret != enable) {
			isp_set_gray_mode(enable);
		}
		ir_cut_enable(1 - enable);
		ir_ctrl_set_brightness_d(led_level);
		vTaskDelay(200);
		isp_set_day_night(enable);
		if (ret != enable) {
			isp_set_day_night(enable);
		}
	} else if (enable == ALS_MODE_IR_Stable) {
		ir_ctrl_set_brightness_d(led_level);
	}
	return ret;
}

void ss_cmd(int type, int index, int *value)
{
	if (type == SS_GET_CMD) {
		if (index == SS_IDX_DBG_LEVEL) {
			*value = ss_dbg_level;
		} else if (index == SS_IDX_EN_AUTO_IR) {
			*value = en_auto_ir;
		} else if (index == SS_IDX_IR_STRENGTH) {
			*value = ir_brightness;
		} else if (index == SS_IDX_SW_LUX) {
			*value = sw_lux;
		}
	} else {
		if (index == SS_IDX_DBG_LEVEL) {
			ss_dbg_level = *value;
		} else if (index == SS_IDX_EN_AUTO_IR) {
			en_auto_ir = *value;
		} else if (index == SS_IDX_IR_STRENGTH) {
			ir_brightness = *value;
		}
	}
}

static void autoir_set_param(auto_ir_config_t *auto_ir_config)
{
	auto_ir_config->ir_led_step[0] = IR_MIN_STRENGTH;
	auto_ir_config->ir_led_step[1] = (IR_MAX_STRENGTH + IR_MIN_STRENGTH) >> 1;
	auto_ir_config->ir_led_step[2] = IR_MAX_STRENGTH;
	auto_ir_config->def_irled_idx = 2;
	auto_ir_config->thr_ir_darkder = 200;
	auto_ir_config->thr_ir_brighter = 1000;
}

void autoir_get_param(auto_ir_config_t *auto_ir_config)
{
	printf("[AutoIR] ==== Configuration ====\n");
	printf("[AutoIR] COUNT_IR_LED_STEP = %d\n", COUNT_IR_LED_STEP);
	printf("[AutoIR] def_irled_idx = %d\n", auto_ir_config->def_irled_idx);
	printf("[AutoIR] thr_ir_darkder = %d\n", auto_ir_config->thr_ir_darkder);
	printf("[AutoIR] thr_ir_brighter = %d\n", auto_ir_config->thr_ir_brighter);
	printf("[AutoIR] ==== Step ====\n");
	for (int i = 0; i < COUNT_IR_LED_STEP; i++) {
		printf("[AutoIR] Step[%d] = %d\n", i, auto_ir_config->ir_led_step[i]);
	}
}

static void autoir_flow(auto_ir_config_t auto_ir_config, int *gray_mode, short *irled_idx)
{
	if ((en_auto_ir && gray_mode) && (*gray_mode >= ALS_MODE_IR_Entry)) {
		if (sw_lux < auto_ir_config.thr_ir_darkder) {
			*gray_mode = ALS_MODE_IR_Stable;
			if (*irled_idx == 0) {
				ss_dprintf(1, "[SensorService][Stable] EN_SmartIR(%d), irled_idx(%d), ir_brightness(%d)\n", en_auto_ir, *irled_idx, ir_brightness);
			} else {
				*irled_idx -= 1;
				if (*irled_idx <= 0) {
					*irled_idx = 0;
				}
				ir_brightness = auto_ir_config.ir_led_step[*irled_idx] ;
				if (!en_auto_ir) {
					ir_brightness = IR_MAX_STRENGTH;
				}
				sensor_external_set_gray_mode(*gray_mode, ir_brightness);
				ss_dprintf(1, "[SensorService] EN_SmartIR(%d), irled_idx(%d), ir_brightness(%d)\n", en_auto_ir, *irled_idx, ir_brightness);
			}
		} else if (sw_lux > auto_ir_config.thr_ir_brighter) {
			*gray_mode = ALS_MODE_IR_Stable;
			if (*irled_idx == (COUNT_IR_LED_STEP - 1)) {
				ss_dprintf(1, "[SensorService][Stable] EN_SmartIR(%d), irled_idx(%d), ir_brightness(%d)\n", en_auto_ir, *irled_idx, ir_brightness);
			} else {
				*irled_idx += 1;
				if (*irled_idx >= COUNT_IR_LED_STEP) {
					*irled_idx = COUNT_IR_LED_STEP - 1;
				}
				ir_brightness = auto_ir_config.ir_led_step[*irled_idx] ;
				if (!en_auto_ir) {
					ir_brightness = IR_MAX_STRENGTH;
				}
				sensor_external_set_gray_mode(*gray_mode, ir_brightness);
				ss_dprintf(1, "[SensorService] EN_SmartIR(%d), irled_idx(%d), ir_brightness(%d)\n", en_auto_ir, *irled_idx, ir_brightness);
			}
		}
	} else if ((!en_auto_ir && *gray_mode) && (ir_brightness != IR_MAX_STRENGTH)) {
		ir_brightness = IR_MAX_STRENGTH;
		sensor_external_set_gray_mode(ALS_MODE_IR_Stable, ir_brightness);
		ss_dprintf(1, "[SensorService] EN_SmartIR(%d), irled_idx(%d), ir_brightness(%d)\n", en_auto_ir, *irled_idx, ir_brightness);
	}
}

#if(ALS_TYPE == HW_ALS)
#define THR_COLOR_TO_GRAY	5
#define THR_GRAY_TO_COLOR	20

void sensor_thread(void *param)
{
	int gray_mode = 0;
	float hw_lux;
	int scale = 180;
	auto_ir_config_t auto_ir_config;
	short irled_idx;

	char ifAEStable = ALS_AE_UNSTABLE;
	ambient_light_sensor_init(NULL);
	ambient_light_sensor_power(1);
	autoir_set_param(&auto_ir_config);
	autoir_get_param(&auto_ir_config);
	irled_idx = auto_ir_config.def_irled_idx;

	while (1) {
		hw_lux = (ambient_light_sensor_get_lux(50) * scale) / 100;
		ifAEStable = isp_get_ifAEstable(&sw_lux, 500);
		ss_dprintf(1, "[SensorService] ifAEStable(%d), Lux(%d)\n", ifAEStable, sw_lux);
		if (ifAEStable == ALS_AE_UNSTABLE && gray_mode == ALS_MODE_IR_Stable) {
			gray_mode = ALS_MODE_IR_Entry;
			ss_dprintf(1, "[SensorService] unstable\r\n");
		}
		if (!gray_mode && (hw_lux <= THR_COLOR_TO_GRAY)) {
			ss_dprintf(1, "[SensorService] RGB2IR:Mode(%d), Lux(%3.3f) <= THR_COLOR_TO_GRAY(%d)\n", gray_mode, hw_lux, (int)THR_COLOR_TO_GRAY);
			gray_mode = ALS_MODE_IR_Entry;
			if (!en_auto_ir) {
				sensor_external_set_gray_mode(gray_mode, IR_MAX_STRENGTH);
			} else {
				sensor_external_set_gray_mode(gray_mode, auto_ir_config.ir_led_step[auto_ir_config.def_irled_idx]);
			}
			irled_idx = auto_ir_config.def_irled_idx;
		} else if (gray_mode && (hw_lux > THR_GRAY_TO_COLOR)) {
			ss_dprintf(1, "[SensorService] IR2RGB:Mode(%d), Lux(%3.3f) >= THR_GRAY_TO_COLOR(%d)\r\n", gray_mode, hw_lux, (int)THR_GRAY_TO_COLOR);
			gray_mode = ALS_MODE_RGB;
			if (en_auto_ir) {
				auto_ir_config.def_irled_idx = irled_idx;
			}
			sensor_external_set_gray_mode(gray_mode, 0);
		}
		autoir_flow(auto_ir_config, &gray_mode, &irled_idx);
		vTaskDelay(SS_DELAY);
	}
}
#else

static void als_set_param(als_config_t *als_config)
{
	char mask[ALS_MAX_COL * ALS_MAX_ROW] = {
		0, 0, 1, 0, 0,
		0, 1, 1, 1, 0,
		1, 1, 1, 1, 1,
		0, 1, 1, 1, 0,
		0, 0, 1, 0, 0
	};
	als_config->Thr_Color_to_Gray = 12000;
	als_config->Thr_Gray_to_Color = 4000;
	als_config->Thr_Color_Ratio = 192;
	als_config->Thr_Valid_Block = 6;
	for (int i = 0; i < ALS_MAX_COL * ALS_MAX_ROW; i++) {
		als_config->Mask[i] = mask[i];
	}
}

void als_get_param(als_config_t *als_config)
{
	printf("[ALS] ==== Configuration ====\n");
	printf("[ALS] THR_COLOR_TO_GRAY=%d\n", als_config->Thr_Color_to_Gray);
	printf("[ALS] THR_GRAY_TO_COLOR=%d\n", als_config->Thr_Gray_to_Color);
	printf("[ALS] THR_COLOR_RATIO  =%d\n", als_config->Thr_Color_Ratio);
	printf("[ALS] THR_VALID_BLOCK  =%d\n", als_config->Thr_Valid_Block);
	printf("[ALS] ==== MASK ====\n");
	for (int i = 0; i < ALS_MAX_ROW; i++) {
		printf("[ALS] ");
		for (int j = 0; j < ALS_MAX_ROW; j++) {
			printf("%2d ", als_config->Mask[i * 5 + j]);
		}
		printf("\n");
	}
}

void sensor_thread(void *param)
{
	als_data_t als_data;
	als_config_t als_config;
	auto_ir_config_t auto_ir_config;
	short irled_idx;
	char ifAEStable = ALS_AE_UNSTABLE;
	char gray_mode = ALS_MODE_RGB;
	als_set_param(&als_config);
	als_get_param(&als_config);
	autoir_set_param(&auto_ir_config);
	autoir_get_param(&auto_ir_config);
	irled_idx = auto_ir_config.def_IRLED_idx;

	while (1) {
		ifAEStable = isp_get_ifAEstable(&sw_lux, 500);
		ss_dprintf(1, "[SensorService] ifAEStable(%d), Lux(%d)\n", ifAEStable, sw_lux);
		if (ifAEStable == ALS_AE_UNSTABLE && gray_mode == ALS_MODE_IR_Stable) {
			gray_mode = ALS_MODE_IR_Entry;
			ss_dprintf(1, "[SensorService] unstable\r\n");
		} else if (!gray_mode && (sw_lux >= als_config.Thr_Color_to_Gray)) {
			ss_dprintf(1, "[SensorService] RGB2IR:Mode(%d), Lux(%d) >= Thr_Color_to_Gray(%d)\n", gray_mode, sw_lux, als_config.Thr_Color_to_Gray);
			gray_mode = ALS_MODE_IR_Entry;
			if (!en_auto_iR) {
				sensor_external_set_gray_mode(gray_mode, IR_MAX_STRENGTH);
			} else {
				sensor_external_set_gray_mode(gray_mode, auto_ir_config.IR_LED_STEP[auto_ir_config.def_IRLED_idx]);
			}
		} else if (gray_mode && (sw_lux <= als_config.Thr_Gray_to_Color)) {
			als_get_statist(&als_data);
			if (als_ifSwitch(&als_config, &als_data)) {
				gray_mode = ALS_MODE_RGB;
				ss_dprintf(1, "[SensorService] IR2RGB:Mode(%d), Lux(%d) <= Thr_Gray_to_Color(%d), als_ifSwitch(1)\n", gray_mode, sw_lux, als_config.Thr_Gray_to_Color);
				if (en_auto_iR) {
					auto_ir_config.def_IRLED_idx = irled_idx;
				}
				sensor_external_set_gray_mode(gray_mode, 0);
			} else {
				ss_dprintf(1, "[SensorService] IRStable:Mode(%d), Lux(%d) <= Thr_Gray_to_Color(%d), als_ifSwitch(0)\n", gray_mode, sw_lux, als_config.Thr_Gray_to_Color);
			}
		}
		autoir_flow(auto_ir_config, &gray_mode, &irled_idx);
		vTaskDelay(SS_DELAY);
	}
}
#endif

void init_sensor_service(void)
{
	if (xTaskCreate(sensor_thread, ((const char *)"sensor_thread"), 1024, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
		printf("\r\n sensor_thread: Create Task Error\n");
	}
}
