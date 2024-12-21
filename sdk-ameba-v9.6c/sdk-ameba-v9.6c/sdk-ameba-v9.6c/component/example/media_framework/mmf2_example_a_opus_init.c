/******************************************************************************
 *
* Copyright(c) 2007 - 2018 Realtek Corporation. All rights reserved.
*
******************************************************************************/
//#include "example_media_framework.h"
#include "mmf2_link.h"
#include "mmf2_siso.h"

#include "module_audio.h"
#include "module_opusc.h"
#include "module_rtsp2.h"

static mm_context_t *audio_ctx		= NULL;
static mm_context_t *opusc_ctx		= NULL;
static mm_context_t *rtsp2_ctx		= NULL;

static mm_siso_t *siso_audio_opusc	= NULL;
static mm_siso_t *siso_opusc_rtsp   = NULL;

#if !USE_DEFAULT_AUDIO_SET
static audio_params_t audio_params = {
#if defined(CONFIG_PLATFORM_8721D)
	.sample_rate = SR_8K,
	.word_length = WL_16,
	.mono_stereo = CH_MONO,
	// .direction = APP_AMIC_IN|APP_LINE_OUT,
	.direction = APP_LINE_IN | APP_LINE_OUT,
#else
	.sample_rate = ASR_8KHZ,
	.word_length = WL_16BIT,
	.mic_gain    = MIC_0DB,
	.dmic_l_gain    = DMIC_BOOST_24DB,
	.dmic_r_gain    = DMIC_BOOST_24DB,
	.use_mic_type   = USE_AUDIO_AMIC,
	.channel     = 1,
#endif
	.mix_mode = 0,
	.enable_aec  = 0
};
#endif

static opusc_params_t opusc_rtsp_params = {
	.sample_rate = 8000,
	.channel = 1,
	.bit_length = 16,			//16 recommand
	.complexity = 5,			//0~10
	.bitrate = 25000,			//default 25000
	.use_framesize = 40,		//needs to the same or bigger than AUDIO_DMA_PAGE_SIZE/(sample_rate/1000)/2 but less than 60
	.enable_vbr = 1,
	.vbr_constraint = 0,
	.packetLossPercentage = 0,
	.opus_application = OPUS_APPLICATION_AUDIO

};

static rtsp2_params_t rtsp2_a_opus_params = {
	.type = AVMEDIA_TYPE_AUDIO,
	.u = {
		.a = {
			.codec_id   = AV_CODEC_ID_OPUS,
			.channel    = 1,
			.samplerate = 8000,
			.frame_size = 40	//equal to use_framesize in opusc_rtsp_params
		}
	}
};

void mmf2_example_a_opus_init(void)
{
	audio_ctx = mm_module_open(&audio_module);
	if (audio_ctx) {
#if !USE_DEFAULT_AUDIO_SET
		mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_params);
#endif
		mm_module_ctrl(audio_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(audio_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(audio_ctx, CMD_AUDIO_APPLY, 0);
	} else {
		rt_printf("audio open fail\n\r");
		goto mmf2_exmaple_a_opus_fail;
	}

	opusc_ctx = mm_module_open(&opusc_module);
	if (opusc_ctx) {
		mm_module_ctrl(opusc_ctx, CMD_OPUSC_SET_PARAMS, (int)&opusc_rtsp_params);
		mm_module_ctrl(opusc_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(opusc_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(opusc_ctx, CMD_OPUSC_APPLY, 0);
	} else {
		rt_printf("OPUSC open fail\n\r");
		goto mmf2_exmaple_a_opus_fail;
	}

	rtsp2_ctx = mm_module_open(&rtsp2_module);
	if (rtsp2_ctx) {
		mm_module_ctrl(rtsp2_ctx, CMD_RTSP2_SELECT_STREAM, 0);
		mm_module_ctrl(rtsp2_ctx, CMD_RTSP2_SET_PARAMS, (int)&rtsp2_a_opus_params);
		mm_module_ctrl(rtsp2_ctx, CMD_RTSP2_SET_APPLY, 0);
		mm_module_ctrl(rtsp2_ctx, CMD_RTSP2_SET_STREAMMING, ON);
		//mm_module_ctrl(rtsp2_ctx, MM_CMD_SET_QUEUE_LEN, 3);
		//mm_module_ctrl(rtsp2_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
	} else {
		rt_printf("RTSP2 open fail\n\r");
		goto mmf2_exmaple_a_opus_fail;
	}

	rt_printf("RTSP2 opened\n\r");

	siso_audio_opusc = siso_create();
	if (siso_audio_opusc) {
		siso_ctrl(siso_audio_opusc, MMIC_CMD_ADD_INPUT, (uint32_t)audio_ctx, 0);
		siso_ctrl(siso_audio_opusc, MMIC_CMD_ADD_OUTPUT, (uint32_t)opusc_ctx, 0);
		siso_ctrl(siso_audio_opusc, MMIC_CMD_SET_STACKSIZE, 24 * 1024, 0);
		siso_start(siso_audio_opusc);
	} else {
		rt_printf("siso1 open fail\n\r");
		goto mmf2_exmaple_a_opus_fail;
	}

	rt_printf("siso1 started\n\r");

	siso_opusc_rtsp = siso_create();
	if (siso_opusc_rtsp) {
		siso_ctrl(siso_opusc_rtsp, MMIC_CMD_ADD_INPUT, (uint32_t)opusc_ctx, 0);
		siso_ctrl(siso_opusc_rtsp, MMIC_CMD_ADD_OUTPUT, (uint32_t)rtsp2_ctx, 0);
		siso_start(siso_opusc_rtsp);
	} else {
		rt_printf("siso2 open fail\n\r");
		goto mmf2_exmaple_a_opus_fail;
	}

	rt_printf("siso2 started\n\r");

	return;
mmf2_exmaple_a_opus_fail:

	return;
}