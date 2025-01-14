/******************************************************************************
*
* Copyright(c) 2007 - 2021 Realtek Corporation. All rights reserved.
*
******************************************************************************/
#include "mmf2_link.h"
#include "mmf2_siso.h"

#include "module_video.h"
#include "module_rtsp2.h"
#include "mmf2_pro2_video_config.h"
#include "video_example_media_framework.h"
#include "log_service.h"

/*****************************************************************************
* ISP channel : 0
* Video type  : H264/HEVC
*****************************************************************************/
#define V1_CHANNEL 0
#define V1_BPS 2*1024*1024
#define V1_RCMODE 2 // 1: CBR, 2: VBR
#define USE_H265 0
#if USE_H265
#include "sample_h265.h"
#define VIDEO_TYPE VIDEO_HEVC
#define VIDEO_CODEC AV_CODEC_ID_H265
#else
#include "sample_h264.h"
#define VIDEO_TYPE VIDEO_H264
#define VIDEO_CODEC AV_CODEC_ID_H264
#endif

static void atcmd_userctrl_init(void);
static mm_context_t *video_v1_ctx			= NULL;
static mm_context_t *rtsp2_v1_ctx			= NULL;

static mm_siso_t *siso_video_rtsp_v1			= NULL;

static video_params_t video_v1_params = {
	.stream_id = V1_CHANNEL,
	.type = VIDEO_TYPE,
	.bps = V1_BPS,
	.rc_mode = V1_RCMODE,
	.use_static_addr = 1
};

static rtsp2_params_t rtsp2_v1_params = {
	.type = AVMEDIA_TYPE_VIDEO,
	.u = {
		.v = {
			.codec_id = VIDEO_CODEC,
			.bps      = V1_BPS
		}
	}
};

static void change_resolution_parameter(int parm_index)
{
	if (parm_index == 0) {
		video_v1_params.resolution = VIDEO_FHD;
		video_v1_params.width = 1920;
		video_v1_params.height = 1080;
		video_v1_params.bps = 2 * 1024 * 1024;
	} else {
		video_v1_params.resolution = VIDEO_HD;
		video_v1_params.width = 1280;
		video_v1_params.height = 720;
		video_v1_params.bps = 1 * 1024 * 1024;
	}
}

void mmf2_video_example_v1_param_change_init(void)
{
	int sw = 0;
	int i = 0;

	atcmd_userctrl_init();

	/*sensor capacity check & video parameter setting*/
	video_v1_params.resolution = VIDEO_FHD;
	video_v1_params.width = sensor_params[USE_SENSOR].sensor_width;
	video_v1_params.height = sensor_params[USE_SENSOR].sensor_height;
	video_v1_params.fps = sensor_params[USE_SENSOR].sensor_fps;
	video_v1_params.gop = sensor_params[USE_SENSOR].sensor_fps;
	/*rtsp parameter setting*/
	rtsp2_v1_params.u.v.fps = sensor_params[USE_SENSOR].sensor_fps;
#if (USE_UPDATED_VIDEO_HEAP == 0)
	int voe_heap_size = video_voe_presetting(1, video_v1_params.width, video_v1_params.height, V1_BPS, 0,
						0, 0, 0, 0, 0,
						0, 0, 0, 0, 0,
						0, 0, 0);
#else
	int voe_heap_size = video_voe_presetting_by_params(&video_v1_params, 0, NULL, 0, NULL, 0, NULL);
#endif
	printf("\r\n voe heap size = %d\r\n", voe_heap_size);

	video_v1_ctx = mm_module_open(&video_module);
	if (video_v1_ctx) {
		mm_module_ctrl(video_v1_ctx, CMD_VIDEO_SET_PARAMS, (int)&video_v1_params);
		mm_module_ctrl(video_v1_ctx, MM_CMD_SET_QUEUE_LEN, video_v1_params.fps * 3);
		mm_module_ctrl(video_v1_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);
	} else {
		rt_printf("video open fail\n\r");
		goto mmf2_video_exmaple_v1_param_change_fail;
	}

	rtsp2_v1_ctx = mm_module_open(&rtsp2_module);
	if (rtsp2_v1_ctx) {
		mm_module_ctrl(rtsp2_v1_ctx, CMD_RTSP2_SELECT_STREAM, 0);
		mm_module_ctrl(rtsp2_v1_ctx, CMD_RTSP2_SET_PARAMS, (int)&rtsp2_v1_params);
		mm_module_ctrl(rtsp2_v1_ctx, CMD_RTSP2_SET_APPLY, 0);
		mm_module_ctrl(rtsp2_v1_ctx, CMD_RTSP2_SET_STREAMMING, ON);
	} else {
		rt_printf("RTSP2 open fail\n\r");
		goto mmf2_video_exmaple_v1_param_change_fail;
	}

	siso_video_rtsp_v1 = siso_create();
	if (siso_video_rtsp_v1) {
#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
		siso_ctrl(siso_video_rtsp_v1, MMIC_CMD_SET_SECURE_CONTEXT, 1, 0);
#endif
		siso_ctrl(siso_video_rtsp_v1, MMIC_CMD_ADD_INPUT, (uint32_t)video_v1_ctx, 0);
		siso_ctrl(siso_video_rtsp_v1, MMIC_CMD_ADD_OUTPUT, (uint32_t)rtsp2_v1_ctx, 0);
		siso_start(siso_video_rtsp_v1);
	} else {
		rt_printf("siso2 open fail\n\r");
		goto mmf2_video_exmaple_v1_param_change_fail;
	}

	mm_module_ctrl(video_v1_ctx, CMD_VIDEO_APPLY, V1_CHANNEL);

	rt_printf("changing resolution and bps test\n\r");
	for (i = 0; i < 2; i++) {
		sw++;
		sw &= 1;
		// wait 30 seconds, change resolution
		vTaskDelay(30000);
		mm_module_ctrl(video_v1_ctx, CMD_VIDEO_STREAM_STOP, V1_CHANNEL);
		change_resolution_parameter(sw);


		siso_pause(siso_video_rtsp_v1);

		mm_module_ctrl(video_v1_ctx, CMD_VIDEO_SET_PARAMS, (int)&video_v1_params);
		mm_module_ctrl(video_v1_ctx, CMD_VIDEO_APPLY, V1_CHANNEL);	// start channel 0

		siso_resume(siso_video_rtsp_v1);
	}

	for (i = 0; i < 10; i++) {
		vTaskDelay(10000);
		rt_printf("changing bit rate test = %d\n\r", (1024 * 1024 + 1024 * (1 + i)));
		mm_module_ctrl(video_v1_ctx, CMD_VIDEO_BPS, (1024 * 1024 + 1024 * (1 + i)));
	}
	for (i = 0; i < 5; i++) {
		vTaskDelay(10000);
		rt_printf("changing QP test = %d, %d\n\r", (25 + i * 2), (35 + i * 2));
		encode_rc_parm_t rc_parm;
		memset(&rc_parm, 0, sizeof(encode_rc_parm_t));
		rc_parm.minQp = (25 + i * 2);
		rc_parm.maxQp = (35 + i * 2);

		mm_module_ctrl(video_v1_ctx, CMD_VIDEO_SET_RCPARAM, (int)&rc_parm);
	}


	for (i = 0; i < 10; i++) {
		vTaskDelay(500);
		rt_printf("changing forcei test\n\r");
		mm_module_ctrl(video_v1_ctx, CMD_VIDEO_FORCE_IFRAME, 0);
	}

	return;
mmf2_video_exmaple_v1_param_change_fail:

	return;
}

static const char *example = "mmf2_video_example_v1_param_change";
static void example_deinit(void)
{
	//Pause Linker
	siso_pause(siso_video_rtsp_v1);

	//Stop module
	mm_module_ctrl(rtsp2_v1_ctx, CMD_RTSP2_SET_STREAMMING, OFF);
	mm_module_ctrl(video_v1_ctx, CMD_VIDEO_STREAM_STOP, V1_CHANNEL);

	//Delete linker
	siso_delete(siso_video_rtsp_v1);

	//Close module
	mm_module_close(rtsp2_v1_ctx);
	mm_module_close(video_v1_ctx);

	//Video Deinit
	video_deinit();
}

static void fUC(void *arg)
{
	static uint32_t user_cmd = 0;

	if (!strcmp(arg, "TD")) {
		if (user_cmd & USR_CMD_EXAMPLE_DEINIT) {
			printf("invalid state, can not do %s deinit!\r\n", example);
		} else {
			example_deinit();
			user_cmd = USR_CMD_EXAMPLE_DEINIT;
			printf("deinit %s\r\n", example);
		}
	} else if (!strcmp(arg, "TSR")) {
		if (user_cmd & USR_CMD_EXAMPLE_DEINIT) {
			printf("reinit %s\r\n", example);
			sys_reset();
		} else {
			printf("invalid state, can not do %s reinit!\r\n", example);
		}
	} else {
		printf("invalid cmd");
	}

	printf("user command 0x%lx\r\n", user_cmd);
}

static log_item_t userctrl_items[] = {
	{"UC", fUC, },
};

static void atcmd_userctrl_init(void)
{
	log_service_add_table(userctrl_items, sizeof(userctrl_items) / sizeof(userctrl_items[0]));
}
