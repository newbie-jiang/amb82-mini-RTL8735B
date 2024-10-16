/******************************************************************************
 *
* Copyright(c) 2007 - 2018 Realtek Corporation. All rights reserved.
*
******************************************************************************/
//#include "example_media_framework.h"
#include "mmf2_link.h"
#include "mmf2_siso.h"
#include "mmf2_simo.h"

#include "module_audio.h"
#include "module_opusc.h"
#include "module_opusd.h"
#include "module_rtp.h"
#include "module_rtsp2.h"

static mm_context_t *rtp_ctx		= NULL;
static mm_context_t *opusd_ctx		= NULL;
static mm_context_t *audio_ctx		= NULL;
static mm_context_t *opusc_ctx		= NULL;
static mm_context_t *rtsp2_ctx		= NULL;

static mm_siso_t *siso_audio_opusc	= NULL;
static mm_siso_t *siso_opusc_rtsp	= NULL;
static mm_siso_t *siso_rtp_opusd	= NULL;
static mm_siso_t *siso_opusd_audio	= NULL;


static mm_context_t *rtsp4_ctx      = NULL;
static mm_simo_t *simo_opusc_rtsp   = NULL;

#if !USE_DEFAULT_AUDIO_SET
static audio_params_t audio_params = {
#if defined(CONFIG_PLATFORM_8721D)
	.sample_rate = SR_8K,//SR_16K,//
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
	.enable_aec  = 1
};
#endif

static opusc_params_t opusc_rtsp_params = {
	//audio	8000/16000
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
			.frame_size = 40    //equal to use_framesize in opusc_rtsp_params
		}
	}
};

static opusd_params_t opusd_params = {
	.sample_rate = 8000,
	.channel = 1,
	.bit_length = 16,         	//16 recommand
	.frame_size_in_msec = 10, 	//will not be uused
	.with_opus_enc = 0,       	//enable semaphore if the application with opus encoder
	.opus_application = OPUS_APPLICATION_AUDIO
};

static rtp_params_t rtp_opusd_params = {
	.valid_pt = 0xFFFFFFFF,
	.port = 16384,
	.frame_size = 1500,
	.cache_depth = 15
};

void mmf2_example_2way_audio_opus_init(void)
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
		goto mmf2_exmaple_2way_audio_fail;
	}
#if 1
	opusc_ctx = mm_module_open(&opusc_module);
	if (opusc_ctx) {
		mm_module_ctrl(opusc_ctx, CMD_OPUSC_SET_PARAMS, (int)&opusc_rtsp_params);
		mm_module_ctrl(opusc_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(opusc_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(opusc_ctx, CMD_OPUSC_INIT_MEM_POOL, 0);
		mm_module_ctrl(opusc_ctx, CMD_OPUSC_APPLY, 0);
	} else {
		rt_printf("OPUSC open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
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
		goto mmf2_exmaple_2way_audio_fail;
	}

	rt_printf("RTSP2 opened\n\r");

#if 0
	rtsp4_ctx = mm_module_open(&rtsp2_module);
	if (rtsp4_ctx) {
		rtsp2_a_opus_params.u.a_opus.samplerate = 16000;
		mm_module_ctrl(rtsp4_ctx, CMD_RTSP2_SELECT_STREAM, 0);
		mm_module_ctrl(rtsp4_ctx, CMD_RTSP2_SET_PARAMS, (int)&rtsp2_a_opus_params);
		mm_module_ctrl(rtsp4_ctx, CMD_RTSP2_SET_APPLY, 0);
		mm_module_ctrl(rtsp4_ctx, CMD_RTSP2_SET_STREAMMING, ON);
		//mm_module_ctrl(rtsp3_ctx, MM_CMD_SET_QUEUE_LEN, 3);
		//mm_module_ctrl(rtsp3_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
	} else {
		rt_printf("RTSP2 open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
	}

	rt_printf("RTSP2 opened\n\r");
#endif
#endif
	rtp_ctx = mm_module_open(&rtp_module);
	if (rtp_ctx) {
		mm_module_ctrl(rtp_ctx, CMD_RTP_SET_PARAMS, (int)&rtp_opusd_params);
		mm_module_ctrl(rtp_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(rtp_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(rtp_ctx, CMD_RTP_APPLY, 0);
		mm_module_ctrl(rtp_ctx, CMD_RTP_STREAMING, 1);	// streamming on
	} else {
		rt_printf("RTP open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
	}

	opusd_ctx = mm_module_open(&opusd_module);
	if (opusd_ctx) {
		mm_module_ctrl(opusd_ctx, CMD_OPUSD_SET_PARAMS, (int)&opusd_params);
		mm_module_ctrl(opusd_ctx, MM_CMD_SET_QUEUE_LEN, 16);
		mm_module_ctrl(opusd_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(opusd_ctx, CMD_OPUSD_APPLY, 0);
	} else {
		rt_printf("OPUSD open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
	}
#if 1
	siso_audio_opusc = siso_create();
	if (siso_audio_opusc) {
		siso_ctrl(siso_audio_opusc, MMIC_CMD_ADD_INPUT, (uint32_t)audio_ctx, 0);
		siso_ctrl(siso_audio_opusc, MMIC_CMD_ADD_OUTPUT, (uint32_t)opusc_ctx, 0);
		//siso_ctrl(siso_audio_opusc, MMIC_CMD_SET_TASKNANE, (uint32_t)"audio_opc", 0);
		siso_ctrl(siso_audio_opusc, MMIC_CMD_SET_STACKSIZE, 24 * 1024, 0);
		siso_start(siso_audio_opusc);
	} else {
		rt_printf("siso1 open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
	}

	rt_printf("siso1 started\n\r");

#if 1
	siso_opusc_rtsp = siso_create();
	if (siso_opusc_rtsp) {
		siso_ctrl(siso_opusc_rtsp, MMIC_CMD_ADD_INPUT, (uint32_t)opusc_ctx, 0);
		siso_ctrl(siso_opusc_rtsp, MMIC_CMD_ADD_OUTPUT, (uint32_t)rtsp2_ctx, 0);
		//siso_ctrl(siso_opusc_rtsp, MMIC_CMD_SET_TASKNANE, (uint32_t)"opusc_rtsp", 0);
		siso_start(siso_opusc_rtsp);
	} else {
		rt_printf("siso2 open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
	}

	rt_printf("siso2 started\n\r");
#else
	simo_opusc_rtsp = simo_create();

	if (simo_opusc_rtsp) {
		simo_ctrl(simo_opusc_rtsp, MMIC_CMD_ADD_INPUT, (uint32_t)opusc_ctx, 0);
		simo_ctrl(simo_opusc_rtsp, MMIC_CMD_ADD_OUTPUT0, (uint32_t)rtsp2_ctx, 0);
		simo_ctrl(simo_opusc_rtsp, MMIC_CMD_ADD_OUTPUT1, (uint32_t)rtsp4_ctx, 0);
		simo_start(simo_opusc_rtsp);
	} else {
		rt_printf("simo open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
	}
	rt_printf("simo started\n\r");
#endif
#endif

	siso_rtp_opusd = siso_create();
	if (siso_rtp_opusd) {
		siso_ctrl(siso_rtp_opusd, MMIC_CMD_ADD_INPUT, (uint32_t)rtp_ctx, 0);
		siso_ctrl(siso_rtp_opusd, MMIC_CMD_ADD_OUTPUT, (uint32_t)opusd_ctx, 0);
		siso_ctrl(siso_rtp_opusd, MMIC_CMD_SET_STACKSIZE, 24 * 1024, 0);
		//siso_ctrl(siso_rtp_opusd, MMIC_CMD_SET_TASKNANE, (uint32_t)"rtp_opd", 0);
		siso_start(siso_rtp_opusd);
	} else {
		rt_printf("siso1 open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
	}

	rt_printf("siso3 started\n\r");

	siso_opusd_audio = siso_create();
	if (siso_opusd_audio) {
		siso_ctrl(siso_opusd_audio, MMIC_CMD_ADD_INPUT, (uint32_t)opusd_ctx, 0);
		siso_ctrl(siso_opusd_audio, MMIC_CMD_ADD_OUTPUT, (uint32_t)audio_ctx, 0);
		//siso_ctrl(siso_opusd_audio, MMIC_CMD_SET_TASKNANE, (uint32_t)"opusd_audio", 0);
		siso_start(siso_opusd_audio);
	} else {
		rt_printf("siso2 open fail\n\r");
		goto mmf2_exmaple_2way_audio_fail;
	}

	rt_printf("siso4 started\n\r");



	return;
mmf2_exmaple_2way_audio_fail:

	return;

}