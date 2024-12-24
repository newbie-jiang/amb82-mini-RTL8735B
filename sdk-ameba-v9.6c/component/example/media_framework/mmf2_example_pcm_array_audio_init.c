/******************************************************************************
*
* Copyright(c) 2007 - 2018 Realtek Corporation. All rights reserved.
*
******************************************************************************/
#include "mmf2_link.h"
#include "mmf2_siso.h"

#include "module_audio.h"
#include "module_array.h"

#include "avcodec.h"

#include "sample_pcm_8k.h"
#include "sample_pcm_16k.h"

static mm_context_t *audio_ctx			= NULL;
static mm_context_t *array_pcm_ctx		= NULL;
static mm_siso_t *siso_array_audio		= NULL;

#if !USE_DEFAULT_AUDIO_SET
static audio_params_t audio_params = {
#if defined(CONFIG_PLATFORM_8721D)
	.sample_rate = SR_8K,
	.word_length = WL_16,
	.mono_stereo = CH_MONO,
	// .direction = APP_AMIC_IN|APP_LINE_OUT,
	.direction = APP_LINE_IN | APP_LINE_OUT,
#else
	.sample_rate = ASR_8KHZ, //ASR_16KHZ,
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

static array_params_t pcm_array_params = {
	.type = AVMEDIA_TYPE_AUDIO,
	.codec_id = AV_CODEC_ID_PCM_RAW,
	.mode = ARRAY_MODE_LOOP,
	.u = {
		.a = {
			.channel    = 1,
			.samplerate = 8000, //16000,
			.frame_size = 640, //suggest set the same as audio
		}
	}
};

void mmf2_example_pcm_array_audio_init(void)
{
	array_t a_array;
	a_array.data_addr = (uint32_t) pcm_sample_8k;
	a_array.data_len = (uint32_t) pcm_sample_8k_size;
	//a_array.data_addr = (uint32_t) pcm_sample_16k;
	//a_array.data_len = (uint32_t) pcm_sample_16k_size;
	array_pcm_ctx = mm_module_open(&array_module);
	if (array_pcm_ctx) {
		mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_SET_PARAMS, (int)&pcm_array_params);
		mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_SET_ARRAY, (int)&a_array);
		mm_module_ctrl(array_pcm_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(array_pcm_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);
		mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_APPLY, 0);
		mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 1);	// streamming on
	} else {
		rt_printf("ARRAY open fail\n\r");
		goto mmf2_example_pcm_audio_init;
	}
	rt_printf("ARRAY opened\n\r");

	audio_ctx = mm_module_open(&audio_module);
	if (audio_ctx) {
#if USE_DEFAULT_AUDIO_SET
		mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_PARAMS, (int)&default_audio_params);
#else
		mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_params);
#endif
		mm_module_ctrl(audio_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(audio_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(audio_ctx, CMD_AUDIO_APPLY, 0);
	} else {
		rt_printf("audio open fail\n\r");
		goto mmf2_example_pcm_audio_init;
	}
	rt_printf("AUDIO opened\n\r");

	//--------------Link---------------------------
	siso_array_audio = siso_create();
	if (siso_array_audio) {
		siso_ctrl(siso_array_audio, MMIC_CMD_ADD_INPUT, (uint32_t)array_pcm_ctx, 0);
		siso_ctrl(siso_array_audio, MMIC_CMD_ADD_OUTPUT, (uint32_t)audio_ctx, 0);
		siso_start(siso_array_audio);
	} else {
		rt_printf("siso_array_audio open fail\n\r");
		goto mmf2_example_pcm_audio_init;
	}
	rt_printf("siso_array_audio started\n\r");

	return;
mmf2_example_pcm_audio_init:

	return;
}