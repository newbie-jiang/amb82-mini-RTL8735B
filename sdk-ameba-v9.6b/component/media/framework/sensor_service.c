#include "sensor_service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "platform_stdlib.h"
#include "isp_api.h"

#define SW_ALS 0
#define HW_ALS 1
#define ALS_TYPE SW_ALS

static int ss_dbg_level = 0;
#define ss_dprintf(level, ...) if(level <= ss_dbg_level) printf(__VA_ARGS__)

typedef enum {
	ALS_MODE_RGB = 0,
	ALS_MODE_IR_Entry,
	ALS_MODE_IR_Stable,
} Als_mode_t;

int sensor_external_set_gray_mode(int enable, int led_level)
{
	int ret;
	if (enable == ALS_MODE_RGB) {
		ss_dprintf(1, "[SensorService] Switch to RGB Mode\n");
		ir_ctrl_set_brightness_d(led_level);
		ir_cut_enable(1 - enable);
		isp_set_day_night(enable);
		ret = isp_get_day_night();
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
}

void ss_set_dbglevel(int enable)
{
	ss_dbg_level = enable;
}

#if(ALS_TYPE == HW_ALS)
#define THR_COLOR_TO_GRAY	5
#define THR_GRAY_TO_COLOR	20

void sensor_thread(void *param)
{
	int gray_mode = 0;
	float lux;
	int scale = 180;
	ambient_light_sensor_init(NULL);
	ambient_light_sensor_power(1);

	while (1) {
		lux = (ambient_light_sensor_get_lux(50) * scale) / 100;
		if (!gray_mode && (lux <= THR_COLOR_TO_GRAY)) {
			ss_dprintf(1, "[SensorService] RGB2IR:Mode(%d), Lux(%3.3f) <= THR_COLOR_TO_GRAY(%d)\n", gray_mode, lux, (int)THR_COLOR_TO_GRAY);
			gray_mode = ALS_MODE_IR_Entry;
			sensor_external_set_gray_mode(gray_mode, 100);
		} else if (gray_mode && (lux > THR_GRAY_TO_COLOR)) {
			ss_dprintf(1, "[SensorService] IR2RGB:Mode(%d), Lux(%3.3f) >= THR_GRAY_TO_COLOR(%d)\r\n", gray_mode, lux, (int)THR_GRAY_TO_COLOR);
			gray_mode = ALS_MODE_RGB;
			sensor_external_set_gray_mode(gray_mode, 0);
		}
		vTaskDelay(3000);
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
	int lux = 0;
	char ifAEStable = ALS_AE_UNSTABLE;
	char gray_mode = ALS_MODE_RGB;
	als_set_param(&als_config);
	als_get_param(&als_config);
	while (1) {
		ifAEStable = isp_get_ifAEstable(&lux, 500);
		ss_dprintf(1, "[SensorService] ifAEStable(%d), Lux(%d)\n", ifAEStable, lux);
		if (ifAEStable == ALS_AE_UNSTABLE) {
		} else if (!gray_mode && (lux >= als_config.Thr_Color_to_Gray)) {
			ss_dprintf(1, "[SensorService] RGB2IR:Mode(%d), Lux(%d) >= Thr_Color_to_Gray(%d)\n", gray_mode, lux, als_config.Thr_Color_to_Gray);
			gray_mode = ALS_MODE_IR_Entry;
			sensor_external_set_gray_mode(gray_mode, 100);
		} else if (gray_mode && (lux <= als_config.Thr_Gray_to_Color)) {
			als_get_statist(&als_data);
			if (als_ifSwitch(&als_config, &als_data)) {
				gray_mode = ALS_MODE_RGB;
				ss_dprintf(1, "[SensorService] IR2RGB:Mode(%d), Lux(%d) <= Thr_Gray_to_Color(%d), als_ifSwitch(1)\n", gray_mode, lux, als_config.Thr_Gray_to_Color);
				sensor_external_set_gray_mode(gray_mode, 0);
			} else {
				ss_dprintf(1, "[SensorService] IRStable:Mode(%d), Lux(%d) <= Thr_Gray_to_Color(%d), als_ifSwitch(0)\n", gray_mode, lux, als_config.Thr_Gray_to_Color);
			}
		}
		vTaskDelay(3000);
	}
}
#endif

void init_sensor_service(void)
{
	if (xTaskCreate(sensor_thread, ((const char *)"sensor_thread"), 384, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
		printf("\r\n sensor_thread: Create Task Error\n");
	}
}
