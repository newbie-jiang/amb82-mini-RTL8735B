/******************************************************************************
*
* Copyright(c) 2007 - 2018 Realtek Corporation. All rights reserved.
*
******************************************************************************/
#include "mmf2_link.h"
#include "mmf2_siso.h"

#include "module_audio.h"

static mm_context_t *audio_ctx = NULL;
static mm_siso_t *siso_audio_loop       = NULL;

#if !USE_DEFAULT_AUDIO_SET
static audio_params_t audio_params = {
#if defined(CONFIG_PLATFORM_8721D)
	.sample_rate = SR_8K,
	.word_length = WL_16,
	.mono_stereo = CH_MONO,
	// .direction = APP_AMIC_IN|APP_LINE_OUT,
	.direction = APP_LINE_IN | APP_LINE_OUT,
#else
	.sample_rate    = ASR_8KHZ,
	.word_length    = WL_16BIT,
	.mic_gain       = MIC_0DB,
	.dmic_l_gain    = DMIC_BOOST_24DB,
	.dmic_r_gain    = DMIC_BOOST_24DB,
	.use_mic_type   = USE_AUDIO_AMIC,
	.channel     = 1,
#endif
	.mix_mode = 0,
	.enable_aec  = 0
};
#endif

#define AUDIO_PCM_DB 0

#if defined(AUDIO_PCM_DB) && AUDIO_PCM_DB
static float transfer_adc_gain2dB(int ADC_gain)
{
	return 30.0 - 0.375 * (0x7F - ADC_gain);
}

static void left_mic_cb(const uint8_t *data, int data_length, uint8_t bytespersample, uint32_t samplerate, audio_params_t audio_params)
{
	//User can get data, data_length(bytes), bytespersample, sample rate and all the audio parameters in this function
	//This is the fucntion is an example to transfer ADC gain into dB
	printf("The ADC Gain setting is %.04fdBFS\r\n", transfer_adc_gain2dB(audio_params.ADC_gain));
}
#endif

void mmf2_example_audioloop_init(void)
{
	audio_ctx = mm_module_open(&audio_module);
	if (audio_ctx) {
#if !USE_DEFAULT_AUDIO_SET
		mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_params);
#endif
		mm_module_ctrl(audio_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(audio_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
#if defined(AUDIO_PCM_DB) && AUDIO_PCM_DB
		mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_LEFTMIC_CB, left_mic_cb);
#endif
		mm_module_ctrl(audio_ctx, CMD_AUDIO_APPLY, 0);
	} else {
		rt_printf("audio open fail\n\r");
		goto mmf2_exmaple_audioloop_fail;
	}


	siso_audio_loop = siso_create();
	if (siso_audio_loop) {
		siso_ctrl(siso_audio_loop, MMIC_CMD_ADD_INPUT, (uint32_t)audio_ctx, 0);
		siso_ctrl(siso_audio_loop, MMIC_CMD_ADD_OUTPUT, (uint32_t)audio_ctx, 0);
		siso_start(siso_audio_loop);
	} else {
		rt_printf("siso1 open fail\n\r");
		goto mmf2_exmaple_audioloop_fail;
	}

	rt_printf("siso1 started\n\r");

	return;
mmf2_exmaple_audioloop_fail:

	return;
}