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
#include "module_opusd.h"

static mm_context_t *audio_ctx            = NULL;
static mm_context_t *opusc_ctx            = NULL;
static mm_context_t *opusd_ctx            = NULL;
static mm_siso_t *siso_audio_opusc        = NULL;
static mm_siso_t *siso_opus_e2d	          = NULL;
static mm_siso_t *siso_opusd_audio     	  = NULL;

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

static opusc_params_t opusc_params = {
	//8000/16000
	.sample_rate = 8000,
	.channel = 1,
	.bit_length = 16,			//16 recommand
	.complexity = 5,			//0~10
	.bitrate = 25000,			//default 25000
	.use_framesize = 0,			//set 0 only when using audio loop
	.enable_vbr = 1,
	.vbr_constraint = 0,
	.packetLossPercentage = 0,
	.opus_application = OPUS_APPLICATION_AUDIO

};

static opusd_params_t opusd_params = {
	.sample_rate = 8000,
	.channel = 1,
	.bit_length = 16,			//16 recommand
	.frame_size_in_msec = 10,	//will not be uused
	.with_opus_enc = 0,			//enable semaphore if the application with opus encoder
	.opus_application = OPUS_APPLICATION_AUDIO
};

void mmf2_example_opusloop_init(void)
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
		goto mmf2_exmaple_opusloop_fail;
	}

	opusc_ctx = mm_module_open(&opusc_module);
	if (opusc_ctx) {
		mm_module_ctrl(opusc_ctx, CMD_OPUSC_SET_PARAMS, (int)&opusc_params);
		mm_module_ctrl(opusc_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(opusc_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(opusc_ctx, CMD_OPUSC_APPLY, 0);
	} else {
		rt_printf("OPUSC open fail\n\r");
		goto mmf2_exmaple_opusloop_fail;
	}


	opusd_ctx = mm_module_open(&opusd_module);
	if (opusd_ctx) {
		mm_module_ctrl(opusd_ctx, CMD_OPUSD_SET_PARAMS, (int)&opusd_params);
		mm_module_ctrl(opusd_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(opusd_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(opusd_ctx, CMD_OPUSD_APPLY, 0);
	} else {
		rt_printf("OPUSD open fail\n\r");
		goto mmf2_exmaple_opusloop_fail;
	}


	siso_audio_opusc = siso_create();
	if (siso_audio_opusc) {
		siso_ctrl(siso_audio_opusc, MMIC_CMD_ADD_INPUT, (uint32_t)audio_ctx, 0);
		siso_ctrl(siso_audio_opusc, MMIC_CMD_ADD_OUTPUT, (uint32_t)opusc_ctx, 0);
		siso_ctrl(siso_audio_opusc, MMIC_CMD_SET_STACKSIZE, 24 * 1024, 0);
		siso_start(siso_audio_opusc);
	} else {
		rt_printf("siso1 open fail\n\r");
		goto mmf2_exmaple_opusloop_fail;
	}

	rt_printf("siso1 started\n\r");

	siso_opus_e2d = siso_create();
	if (siso_opus_e2d) {
		siso_ctrl(siso_opus_e2d, MMIC_CMD_ADD_INPUT, (uint32_t)opusc_ctx, 0);
		siso_ctrl(siso_opus_e2d, MMIC_CMD_ADD_OUTPUT, (uint32_t)opusd_ctx, 0);
		siso_ctrl(siso_opus_e2d, MMIC_CMD_SET_STACKSIZE, 24 * 1024, 0);
		siso_start(siso_opus_e2d);
	} else {
		rt_printf("siso2 open fail\n\r");
		goto mmf2_exmaple_opusloop_fail;
	}

	rt_printf("siso2 started\n\r");

	siso_opusd_audio = siso_create();
	if (siso_opusd_audio) {
		siso_ctrl(siso_opusd_audio, MMIC_CMD_ADD_INPUT, (uint32_t)opusd_ctx, 0);
		siso_ctrl(siso_opusd_audio, MMIC_CMD_ADD_OUTPUT, (uint32_t)audio_ctx, 0);
		siso_start(siso_opusd_audio);
	} else {
		rt_printf("siso3 open fail\n\r");
		goto mmf2_exmaple_opusloop_fail;
	}

	rt_printf("siso3 started\n\r");


	return;
mmf2_exmaple_opusloop_fail:

	return;
}