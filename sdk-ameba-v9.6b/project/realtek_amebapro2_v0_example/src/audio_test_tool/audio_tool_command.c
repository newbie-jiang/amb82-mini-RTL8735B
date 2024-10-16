#include "audio_tool_command.h"
#include "log_service.h"
#include "audio_cjson_generater.h"

mm_context_t	*audio_save_ctx		= NULL;
mm_context_t 	*null_save_ctx		= NULL;
mm_context_t 	*array_pcm_ctx		= NULL;
mm_context_t 	*pcm_tone_ctx		= NULL;
mm_context_t 	*afft_test_ctx		= NULL;
mm_context_t 	*p2p_audio_ctx		= NULL;
mm_siso_t	 *siso_audio_afft	= NULL;
mm_mimo_t	 *mimo_aarray_audio	= NULL;

int set_mic_type = DEFAULT_AUDIO_MIC;

audio_params_t audio_save_params = {
	.sample_rate            = ASR_16KHZ,
	.word_length            = WL_16BIT,
	.mic_gain               = MIC_0DB,
	.dmic_l_gain            = DMIC_BOOST_12DB,
	.dmic_r_gain            = DMIC_BOOST_12DB,
	.use_mic_type           = DEFAULT_AUDIO_MIC,
	.channel                = 1,
	.mix_mode               = 0,
	.mic_bias               = 0,
	.hpf_set                = 0,
	.ADC_gain               = 0x66,	//ADC path Dgain about 20dB
	.DAC_gain               = 0xAF,
	.ADC_mute               = 0,
	.DAC_mute               = 0,
	.enable_record          = 1,
};
#if defined(CONFIG_NEWAEC) && CONFIG_NEWAEC
RX_cfg_t rx_asp_params = {
	.aec_cfg = {
		.AEC_EN = 0,
		.EchoTailLen = 64,
		.CNGEnable = 1,
		.PPLevel = 6,
		.DTControl = 1,
		.ConvergenceTime = 100,
	},
	.agc_cfg = {
		.AGC_EN = 0,
		.AGCMode = CT_ALC,
		.ReferenceLvl = 6,
		.RefThreshold = 6,
		.AttackTime = 20,
		.ReleaseTime = 20,
		.Ratio = {50, 50, 50},
		.Threshold = {20, 30, 40},
		.KneeWidth = 0,
		.NoiseFloorAdaptEnable = 1,
		.RMSDetectorEnable = 0,
	},
	.ns_cfg = {
		.NS_EN = 0,
		.NSLevel = 5,
		.HPFEnable = 0,
		.QuickConvergenceEnable = 0,
	}
};

TX_cfg_t tx_asp_params = {
	.agc_cfg = {
		.AGC_EN = 0,
		.AGCMode = CT_ALC,
		.ReferenceLvl = 6,
		.RefThreshold = 6,
		.AttackTime = 20,
		.ReleaseTime = 20,
		.Ratio = {50, 50, 50},
		.Threshold = {20, 30, 40},
		.KneeWidth = 0,
		.NoiseFloorAdaptEnable = 1,
		.RMSDetectorEnable = 0,
	},
	.ns_cfg = {
		.NS_EN = 0,
		.NSLevel = 5,
		.HPFEnable = 0,
		.QuickConvergenceEnable = 0,
	}
};
#else
RX_cfg_t rx_asp_params = {
	.aec_cfg = {
		.AEC_EN = 0,
		.aec_core = WEBRTC_AECM,
		.FilterLength = 30,
		.CNGEnable = 1,
		.AECLevel = 3,

		//for the AGC embedded in AEC
		.AGC_EN = 0,
		.AGCMode = 2,
		.TargetLevelDbfs = 0,
		.CompressionGaindB = 18,
		.LimiterEnable = 1,

		//for the NS embedded in AEC
		.NS_EN = 0,
		.NSLevel = 3,

		//howling suppression only used in webrtc
		.HOWL_EN = 0,
		.HOWL_AGC_EN = 0,
		.HOWL_AGCMode = 2,
		.HOWL_TargetLevelDbfs = 0,
		.HOWL_CompressionGaindB = 18,
		.HOWL_LimiterEnable = 1,

		.HOWL_NS_EN = 0,
		.HOWL_NSLevel = 3,
	},
	.agc_cfg = {
		.AGC_EN = 0,
		.AGCMode = 2,
		.TargetLevelDbfs = 0,
		.CompressionGaindB = 18,
		.LimiterEnable = 1,
	},
	.ns_cfg = {
		.NS_EN = 0,
		.NSLevel = 5,
	}
};

TX_cfg_t tx_asp_params = {
	.agc_cfg = {
		.AGC_EN = 0,
		.AGCMode = 2,
		.TargetLevelDbfs = 0,
		.CompressionGaindB = 18,
		.LimiterEnable = 1,
	},
	.ns_cfg = {
		.NS_EN = 0,
		.NSLevel = 5,
	},
};
#endif

array_params_t pcm16k_array_params = {
	.type = AVMEDIA_TYPE_AUDIO,
	.codec_id = AV_CODEC_ID_PCM_RAW,
	.mode = ARRAY_MODE_LOOP,
	.u = {
		.a = {
			.channel    = 1,
			.samplerate = 16000,
			.frame_size = 640,
		}
	}
};

tone_params_t pcm_tone_params = {
	.codec_id = AV_CODEC_ID_PCM_RAW,
	.mode = ARRAY_MODE_LOOP,
	.channel    = 1,
	.audiotonerate = 1000,
	.samplerate = 16000,
	.frame_size = 640,
	.ramdisk_tag = "audio_ram",
	.sdcard_tag = "audio_sd",
	.audio_filename = "playback.bin",
};

afft_params_t afft_test_params = {
	.sample_rate = 16000,
	.channel    = 1,
	.pcm_frame_size = 640,
};

p2p_audio_params_t p2p_audio_params = {
	.sample_rate = 16000,
	.channel    = 1,
};

//int NS_level = 0;
//int NS_level_SPK = 0;
int ADC_gain = 0x66;
int DAC_gain = 0xAF;
int frame_count = 0;
int record_frame_count = 0;
int record_state = 0; // 0 no record 1 start record 2 recording 3 record end
int reset_flag = 0;
int the_sample_rate = 16000;
int playing_sample_rate = 16000;
int mic_bias = 0; //0:0.9 1:0.86 2:0.75
int mic_gain = 0; //0:0dB 1:20dB 2:30dB 3:40dB
int hpf_set = 0; //0~7
int tx_mode = 0; //0: none 1: playtone 2: playback 3: playmusic
int audio_fft_show = 0;
int record_type = RECORD_RX_DATA;

uint8_t audiocopy_status = 0x0;

#define MAX_RECORD_TIME     5*60000    //5 min        
#define MIN_RECORD_TIME     40         //40 ms     

FILE  *m_record_file_RX;
FILE  *m_record_file_TX;
FILE  *m_record_file_ASP;
FILE  *m_record_file_TXASP;
char ram_record_file_RX[64];
char ram_record_file_TX[64];
char ram_record_file_ASP[64];
char ram_record_file_TXASP[64];
char file_name[20] = "test";
int recored_count = 0;
int audio_tftp_port = AUDIO_TFTP_HOST_PORT;
char audio_tftp_ip[16] = AUDIO_TFTP_HOST_IP_ADDR;

xSemaphoreHandle  ram_dump_sd_sema = NULL;
xSemaphoreHandle  ram_upload_tftp_sema = NULL;

void audio_fatfs_drv_open(void)
{
	int res;
	vfs_init(NULL);

	res = vfs_user_register("audio_ram", VFS_FATFS, VFS_INF_RAM);
	if (res < 0) {
		printf("fatfs_ram_init fail (%d)\n\r", res);
		return ;
	} else {
		printf("fatfs_ram_init open\n\r");
	}

	res = vfs_user_register("audio_sd", VFS_FATFS, VFS_INF_SD);
	if (res < 0) {
		printf("fatfs_sd_init fail (%d)\n\r", res);
		return ;
	} else {
		printf("fatfs_sd_init open\n\r");
	}
	return ;
}

void audio_fatfs_drv_close(void)
{
	vfs_user_unregister("audio_ram", VFS_FATFS, VFS_INF_RAM);
	vfs_user_unregister("audio_sd", VFS_FATFS, VFS_INF_SD);
	printf("fatfs_ram_init close\n\r");
	vfs_deinit(NULL);
}

void audio_open_record_file(void)
{
	char path[64];

	WAVE_HEADER w_header;
	w_header.w_header.fccID[0] = 'R';
	w_header.w_header.fccID[1] = 'I';
	w_header.w_header.fccID[2] = 'F';
	w_header.w_header.fccID[3] = 'F';
	w_header.w_header.fccType[0] = 'W';
	w_header.w_header.fccType[1] = 'A';
	w_header.w_header.fccType[2] = 'V';
	w_header.w_header.fccType[3] = 'E';
	w_header.w_header.dwSize = record_frame_count * 2 * FRAME_LEN + 36;
	w_header.w_fmt.fccID[0] = 'f';
	w_header.w_fmt.fccID[1] = 'm';
	w_header.w_fmt.fccID[2] = 't';
	w_header.w_fmt.fccID[3] = ' ';
	w_header.w_fmt.dwSize = 16;
	w_header.w_fmt.wFormatTag = 1;
	w_header.w_fmt.wChannels = 1;
	w_header.w_fmt.dwSamplesPerSec = playing_sample_rate;
	w_header.w_fmt.dwAvgBytesPerSec = w_header.w_fmt.dwSamplesPerSec * 2;
	w_header.w_fmt.wBlockAlign = 2;
	w_header.w_fmt.uiBitsPerSample = 16;
	w_header.w_data.fccID[0] = 'd';
	w_header.w_data.fccID[1] = 'a';
	w_header.w_data.fccID[2] = 't';
	w_header.w_data.fccID[3] = 'a';
	w_header.w_data.dwSize = record_frame_count * 2 * FRAME_LEN;

	printf("\n\r=== FATFS Example (RAM DISK) ===\n\r");
	printf("\n\r=== RAM FS Read/Write test ===\n\r");

	recored_count ++;
	//Record file in ram disk
	if (record_type & RECORD_RX_DATA) {
		//snprintf(ram_record_file_RX, 63, "%s_RX%03d.bin", file_name, recored_count);
		snprintf(ram_record_file_RX, 63, "%s_RX%03d.wav", file_name, recored_count);
		printf("record file name: %s\n\r", ram_record_file_RX);
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "audio_ram:/%s", ram_record_file_RX);
		printf("record file name: %s\n\r", path);

		m_record_file_RX = fopen(path, "w");  // if open successfully, f_open will returns 0
		printf("open record file name: %s\n\r", path);
		if (!m_record_file_RX) {
			printf("open file (%s) fail.\n\r", path);
			record_state = 0;
			return;
		}
		fwrite(&w_header, sizeof(WAVE_HEADER), 1, m_record_file_RX);
		printf("record file name: %s\n\n\r", path);
	}
	if (record_type & RECORD_TX_DATA) {
		//snprintf(ram_record_file_TX, 63, "%s_TX%03d.bin", file_name, recored_count);
		snprintf(ram_record_file_TX, 63, "%s_TX%03d.wav", file_name, recored_count);
		printf("record file name: %s\n\r", ram_record_file_TX);
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "audio_ram:/%s", ram_record_file_TX);
		printf("record file name: %s\n\r", path);

		m_record_file_TX = fopen(path, "w");  // if open successfully, f_open will returns 0
		printf("open record file name: %s\n\r", path);
		if (!m_record_file_TX) {
			printf("open file (%s) fail.\n\r", path);
			record_state = 0;
			return;
		}
		fwrite(&w_header, sizeof(WAVE_HEADER), 1, m_record_file_TX);
		printf("record file name: %s\n\n\r", path);
	}
	if (record_type & RECORD_ASP_DATA) {
		//snprintf(ram_record_file_ASP, 63, "%s_ASP%03d.bin", file_name, recored_count);
		snprintf(ram_record_file_ASP, 63, "%s_ASP%03d.wav", file_name, recored_count);
		printf("record file name: %s\n\r", ram_record_file_ASP);
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "audio_ram:/%s", ram_record_file_ASP);
		printf("record file name: %s\n\r", path);

		m_record_file_ASP = fopen(path, "w");  // if open successfully, f_open will returns 0
		printf("open record file name: %s\n\r", path);
		if (!m_record_file_ASP) {
			printf("open file (%s) fail.\n\r", path);
			record_state = 0;
			return;
		}
		fwrite(&w_header, sizeof(WAVE_HEADER), 1, m_record_file_ASP);
		printf("record file name: %s\n\n\r", path);
	}
	if (record_type & RECORD_TXASP_DATA) {
		//snprintf(ram_record_file_TXASP, 63, "%s_TXASP%03d.bin", file_name, recored_count);
		snprintf(ram_record_file_TXASP, 63, "%s_TXASP%03d.wav", file_name, recored_count);
		printf("record file name: %s\n\r", ram_record_file_TXASP);
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "audio_ram:/%s", ram_record_file_TXASP);
		printf("record file name: %s\n\r", path);

		m_record_file_TXASP = fopen(path, "w");  // if open successfully, f_open will returns 0
		printf("open record file name: %s\n\r", path);
		if (!m_record_file_TXASP) {
			printf("open file (%s) fail.\n\r", path);
			record_state = 0;
			return;
		}
		fwrite(&w_header, sizeof(WAVE_HEADER), 1, m_record_file_TXASP);
		printf("record file name: %s\n\n\r", path);
	}
	record_state = 2;


	return  ;
}

void audio_close_record_file(void)
{
	int res;
	//close Record
	if (record_type & RECORD_RX_DATA) {
		res = fclose(m_record_file_RX);
		if (res) {
			printf("close file (%s) fail.\n\r", ram_record_file_RX);
		} else {
			printf("close file (%s) success.\n\r", ram_record_file_RX);
		}
	}
	if (record_type & RECORD_TX_DATA) {
		res = fclose(m_record_file_TX);
		if (res) {
			printf("close file (%s) fail.\n\r", ram_record_file_TX);
		} else {
			printf("close file (%s) success.\n\r", ram_record_file_TX);
		}
	}
	if (record_type & RECORD_ASP_DATA) {
		res = fclose(m_record_file_ASP);
		if (res) {
			printf("close file (%s) fail.\n\r", ram_record_file_ASP);
		} else {
			printf("close file (%s) success.\n\r", ram_record_file_ASP);
		}
	}
	if (record_type & RECORD_TXASP_DATA) {
		res = fclose(m_record_file_TXASP);
		if (res) {
			printf("close file (%s) fail.\n\r", ram_record_file_TXASP);
		} else {
			printf("close file (%s) success.\n\r", ram_record_file_TXASP);
		}
	}

	if (audiocopy_status & SD_SAVE_EN) {
		xSemaphoreGive(ram_dump_sd_sema);
		audiocopy_status |= SD_SAVE_START;
	}

	if (audiocopy_status & TFTP_UPLOAD_EN) {
		xSemaphoreGive(ram_upload_tftp_sema);
		audiocopy_status |= TFTP_UPLOAD_START;
	}
	return  ;
}

void audio_record_write_file_RX(int16_t *record)
{

	fwrite(record, 1, 2 * FRAME_LEN, m_record_file_RX);

	return ;
}

void audio_record_write_file_TX(int16_t *record)
{

	fwrite(record, 1, 2 * FRAME_LEN, m_record_file_TX);

	return ;
}

void audio_record_write_file_ASP(int16_t *record)
{

	fwrite(record, 1, 2 * FRAME_LEN, m_record_file_ASP);

	return ;
}

void audio_record_write_file_TXASP(int16_t *record)
{

	fwrite(record, 1, 2 * FRAME_LEN, m_record_file_TXASP);

	return ;
}

void audio_mic_record(int16_t *speaker_data_TX, int16_t *speaker_data_TXASP, int16_t *mic_data_RX, int16_t *mic_data_ASP)
{
	static int record_percent;
	if (record_state == 3) {
		audio_close_record_file();
		record_state = 0;
	} else if (record_state == 2) {
		if (frame_count > 0) {
			frame_count --;
			if (record_frame_count >= 100) {
				if (((record_frame_count - frame_count) / (record_frame_count / 100)) >= (record_percent + 1)) {
					record_percent = (record_frame_count - frame_count) / (record_frame_count / 100);
					printf("*");
					if ((record_percent % 10) == 0) {
						printf(" %d%% is done!\n\r", record_percent);
					}
				}
			}
			if (record_type & RECORD_RX_DATA) {
				audio_record_write_file_RX(mic_data_RX);
			}
			if (record_type & RECORD_TX_DATA) {
				audio_record_write_file_TX(speaker_data_TX);
			}
			if (record_type & RECORD_ASP_DATA) {
				audio_record_write_file_ASP(mic_data_ASP);
			}
			if (record_type & RECORD_TXASP_DATA) {
				audio_record_write_file_TXASP(speaker_data_TXASP);
			}
		} else if (frame_count == 0) {
			record_state = 3;
		}
	} else if (record_state == 1) {
		audio_open_record_file();
		record_percent = 0;
	}
}

#include "gpio_api.h"
gpio_t gpio_amp;
int pin_initialed = 0;
int set_pin;

int pinconvert(char *pin_stream, int *amp_pin)
{
	char *pin_id_ptr;
	int amp_pin_id = 0;

	if (!strncmp("PA_", pin_stream, 3)) {
		pin_id_ptr = pin_stream + 3;
		amp_pin_id = atoi(pin_id_ptr);
		if (amp_pin_id >= 0 && amp_pin_id <= 5) {
			printf("Set pin PA_%d ", amp_pin_id);
			*amp_pin = PIN_NAME(PORT_A, amp_pin_id);
			return 1;
		} else {
			return 0;
		}
	} else if (!strncmp("PB_", pin_stream, 3)) {
		pin_id_ptr = pin_stream + 3;
		amp_pin_id = atoi(pin_id_ptr);
		if (amp_pin_id >= 0 && amp_pin_id <= 2) {
			printf("Set pin PB_%d ", amp_pin_id);
			*amp_pin = PIN_NAME(PORT_B, amp_pin_id);
			return 1;
		} else {
			return 0;
		}
	} else if (!strncmp("PC_", pin_stream, 3)) {
		pin_id_ptr = pin_stream + 3;
		amp_pin_id = atoi(pin_id_ptr);
		if (amp_pin_id >= 0 && amp_pin_id <= 5) {
			printf("Set pin PC_%d ", amp_pin_id);
			*amp_pin = PIN_NAME(PORT_C, amp_pin_id);
			return 1;
		} else {
			return 0;
		}
	} else if (!strncmp("PD_", pin_stream, 3)) {
		pin_id_ptr = pin_stream + 3;
		amp_pin_id = atoi(pin_id_ptr);
#if IS_CUT_TEST(CONFIG_CHIP_VER)
		if (amp_pin_id >= 0 && amp_pin_id <= 16) {
#else
		if (amp_pin_id >= 0 && amp_pin_id <= 20) {
#endif
			printf("Set pin PD_%d ", amp_pin_id);
			*amp_pin = PIN_NAME(PORT_D, amp_pin_id);
			return 1;
		} else {
			return 0;
		}
	} else if (!strncmp("PE_", pin_stream, 3)) {
		pin_id_ptr = pin_stream + 3;
		amp_pin_id = atoi(pin_id_ptr);
#if IS_CUT_TEST(CONFIG_CHIP_VER)
		if (amp_pin_id >= 0 && amp_pin_id <= 10) {
#else
		if (amp_pin_id >= 0 && amp_pin_id <= 6) {
#endif
			printf("Set pin PE_%d ", amp_pin_id);
			*amp_pin = PIN_NAME(PORT_E, amp_pin_id);
			return 1;
		} else {
			return 0;
		}
	} else if (!strncmp("PF_", pin_stream, 3)) {
		pin_id_ptr = pin_stream + 3;
		//printf("pin_id_ptr = %s\r\n", pin_id_ptr);
		amp_pin_id = atoi(pin_id_ptr);
		if (amp_pin_id >= 0 && amp_pin_id <= 17) {
			printf("Set pin PF_%d ", amp_pin_id);
			*amp_pin = PIN_NAME(PORT_F, amp_pin_id);
			return 1;
		} else {
			return 0;
		}
	} else if (!strncmp("PS_", pin_stream, 3)) {
		pin_id_ptr = pin_stream + 3;
		amp_pin_id = atoi(pin_id_ptr);
		if (amp_pin_id >= 0 && amp_pin_id <= 6) {
			printf("Set pin PS_%d ", amp_pin_id);
			*amp_pin = PIN_NAME(PORT_S, amp_pin_id);
			return 1;
		} else {
			return 0;
		}
	}
	return 0;
}

//********************//
//MIC setting function
//********************//

//Set the audio mic mode
void fAUMMODE(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUMMODE] Set the mic mode: AUMMODE=[mic_mode]\n");

		printf("  \r     [mic_mode]=amic/l_dmic/r_dmic/stereo_dmic\n");
		printf("  \r     Default is Amic. \r\nSet Left Dmic by AUMMODE=l_dmic\r\nSet Right Dmic by AUMMODE=r_dmic\r\nSet Stereo Dmic by AUMMODE=stereo_dmic\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		if (strncmp(argv[1], "amic", strlen("amic")) == 0) {
			printf("Set A mic \r\n");
			set_mic_type = USE_AUDIO_AMIC;
		} else if (strncmp(argv[1], "l_dmic", strlen("l_dmic")) == 0) { //Set the left dmic
			printf("Set Left D mic \r\n");
			set_mic_type = USE_AUDIO_LEFT_DMIC;
		} else if (strncmp(argv[1], "r_dmic", strlen("r_dmic")) == 0) { //Set the right dmic
			printf("Set Right D mic \r\n");
			set_mic_type = USE_AUDIO_RIGHT_DMIC;
		} else if (strncmp(argv[1], "stereo_dmic", strlen("stereo_dmic")) == 0) { //Set the stereo dmic
			printf("Set Stereo D mic \r\n");
			set_mic_type = USE_AUDIO_STEREO_DMIC;
		} else {
			printf("Unknown mic type\r\n");
		}
	}
}

//Set mic gain
void fAUMG(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUMG] Set up MIC GAIN: AUMG=[mic_gain]\n");

		printf("  \r     [mic_gain]=0~3\n");
		printf("  \r     0: 0dB 1: 20dB 2: 30dB 3: 40dB\n");
		printf("  \r     Set MIC Gain 0dB by AUMG=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		mic_gain = atoi(argv[1]);
		if (mic_gain < 0 || mic_gain > 3) {
			mic_gain = 0;
			printf("invalid mic_gain set default value: %d\r\n", mic_gain);
		} else {
			printf("Set mic gain value: %d\r\n", mic_gain);
		}
		audio_save_params.mic_gain = mic_gain;
		audio_ctx_t *ctx = (audio_ctx_t *)audio_save_ctx->priv;
		audio_mic_analog_gain(ctx->audio, 1, audio_save_params.mic_gain);
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
	}
}

//Set mic bias
void fAUMB(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUMB] Set up MIC BIAS: AUMB=[mic_bias]\n");

		printf("  \r     [mic_bias]=0~2\n");
		printf("  \r     0: 0.9 1: 0.86 2: 0.75\n");
		printf("  \r     Set MIC BIAS 0.9 by AUMB=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		mic_bias = atoi(argv[1]);
		if (mic_bias < 0 || mic_bias > 2) {
			mic_bias = 0;
			printf("invalid mic_bias set default value: %d\r\n", mic_bias);
		} else {
			printf("Set mic bias value: %d\r\n", mic_bias);
		}
		audio_save_params.mic_bias = mic_bias;
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
	}
}

//Set left dmic gain
void fAUMLG(void *arg)
{
	int argc = 0;
	int left_dmic_gain = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUMLG] Set up left dmic MIC GAIN: AUMLG=[left_dmic_gain]\n");

		printf("  \r     [left_dmic_gain]=0~3\n");
		printf("  \r     0: 0dB 1: 12dB 2: 24dB 3: 36dB\n");
		printf("  \r     Set LEFT DMIC Gain 0dB by AUMLG=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		left_dmic_gain = atoi(argv[1]);
		if (left_dmic_gain < 0 || left_dmic_gain > 3) {
			left_dmic_gain = 0;
			printf("invalid left_dmic_gain set default value: %d\r\n", left_dmic_gain);
		} else {
			printf("Set left_dmic gain value: %d\r\n", left_dmic_gain);
		}
		audio_save_params.dmic_l_gain = left_dmic_gain;
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
	}
}

//Set right dmic gain
void fAUMRG(void *arg)
{
	int argc = 0;
	int right_dmic_gain = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUMRG] Set up right dmic MIC GAIN: AUMRG=[right_dmic_gain]\n");

		printf("  \r     [right_dmic_gain]=0~3\n");
		printf("  \r     0: 0dB 1: 12dB 2: 24dB 3: 36dB\n");
		printf("  \r     Set LEFT DMIC Gain 0dB by AUMRG=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		right_dmic_gain = atoi(argv[1]);
		if (right_dmic_gain < 0 || right_dmic_gain > 3) {
			right_dmic_gain = 0;
			printf("invalid right_dmic_gain set default value: %d\r\n", right_dmic_gain);
		} else {
			printf("Set right_dmic gain value: %d\r\n", right_dmic_gain);
		}
		audio_save_params.dmic_r_gain = right_dmic_gain;
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
	}
}

//Set the mic ADC gain
void fAUADC(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	if (!arg) {
		printf("\n\r[AUADC] Set up ADC gain: AUADC=[ADC_gain]\n");

		printf("  \r     [ADC_gain]=0x00~0x7F\n");
		printf("  \r     ADC_gain need in hex= 0x7F: 30dB, 0x5F: 18dB, 0x2F: 0dB, 0x00: -17.625dB, 0.375/step\n");
		printf("  \r     Set ADC_gain 18dB by AUADC=0x5F\n");
		return;
	}

	argc = parse_param(arg, argv);

	if (argc) {
		ADC_gain = (int)strtol(argv[1], NULL, 16);
		printf("get ADC gain = %d, 0x%x\r\n", ADC_gain, ADC_gain);
		if (ADC_gain >= 0x00 && ADC_gain <= 0x7F) {
			printf("get ADC gain = %d, 0x%x\r\n", ADC_gain, ADC_gain);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_ADC_GAIN, ADC_gain);
		} else {
			ADC_gain = 0x5F;
			printf("invalid ADC set default gain 0x%x\r\n", ADC_gain);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_ADC_GAIN, ADC_gain);

		}
		audio_save_params.ADC_gain = ADC_gain;
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
	}
}

//Set the mic HPF
void fAUHPF(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUHPF] AUDIO HPF Usage: AUHPF=[cutoff num]\n");

		printf("  \r     [cutoff num]=0~8\n");
		printf("  \r     fc: cutoff frequency, fs: sample frequency \n");
		printf("  \r     fc ~= 5e-3 / (cutoff num + 1) * fs \n");
		printf("  \r     Set HPF by AUHPF=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		hpf_set = atoi(argv[1]);
		if (hpf_set < 0 || hpf_set > 7) {
			hpf_set = 5;
			printf("invalid hpf_set set default value: %d\r\n", hpf_set);
		} else {
			printf("Set HPF value: %d\r\n", hpf_set);
		}
		audio_save_params.hpf_set = hpf_set;
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
	}
}

//set the left EQ for mic
void fAUMLEQ(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	uint8_t EQ_num = 0;
	if (!arg) {
		printf("\n\r[AUMLEQ] LEFT MIC EQ Usage: AUMLEQ=[eq num],[register h0],[register b1],[register b2],[register a1],[register a2]\n");
		printf("  \r[AUMLEQ] LEFT MIC EQ DISABLE Usage: AUMLEQ=[eq num]\n");
		printf("  \r     Enter the register coefficients obtained from biquad calculator\n");

		printf("  \r     [eq num]=0~4\n");
		printf("  \r     [register]=<num in hex> \n");
		printf("  \r     OPEN EQ by AUMLEQ=0,0x01,0x02,0x03,x04,0x05\n");
		printf("  \r     CLOSE EQ by AUMLEQ=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc == 7) {
		EQ_num = atoi(argv[1]);
		if (EQ_num >= 0 && EQ_num <= 4) {
			printf("ENABLE LEFT DMIC OR AMIC EQ: %d \r\n", EQ_num);
			audio_save_params.mic_l_eq[EQ_num].eq_enable = 1;
			audio_save_params.mic_l_eq[EQ_num].eq_h0 = (int)strtol(argv[2], NULL, 16);
			audio_save_params.mic_l_eq[EQ_num].eq_b1 = (int)strtol(argv[3], NULL, 16);
			audio_save_params.mic_l_eq[EQ_num].eq_b2 = (int)strtol(argv[4], NULL, 16);
			audio_save_params.mic_l_eq[EQ_num].eq_a1 = (int)strtol(argv[5], NULL, 16);
			audio_save_params.mic_l_eq[EQ_num].eq_a2 = (int)strtol(argv[6], NULL, 16);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else {
			printf(" Set Wrong EQ: %d Not between 0~4\r\n", EQ_num);
		}
	} else if (argc == 2) {
		EQ_num = atoi(argv[1]);
		if (EQ_num >= 0 && EQ_num <= 4) {
			printf("DISABLE LEFT DMIC OR AMIC EQ: %d \r\n", EQ_num);
			audio_save_params.mic_l_eq[EQ_num].eq_enable = 0;
			audio_save_params.mic_l_eq[EQ_num].eq_h0 = 0;
			audio_save_params.mic_l_eq[EQ_num].eq_b1 = 0;
			audio_save_params.mic_l_eq[EQ_num].eq_b2 = 0;
			audio_save_params.mic_l_eq[EQ_num].eq_a1 = 0;
			audio_save_params.mic_l_eq[EQ_num].eq_a2 = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else {
			printf(" Set Wrong EQ: %d Not between 0~4\r\n", EQ_num);
		}
	} else {
		printf(" argument number = %d \r\n", argc - 1);
		printf(" MISS some arguments AUMLEQ=[eq num],[register h0],[register a1],[register a2],[register b1],[register b2]\r\n");
	}
}

//set the right EQ for mic
void fAUMREQ(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	uint8_t EQ_num = 0;
	if (!arg) {
		printf("\n\r[AUMREQ] RIGHT MIC EQ Usage: AUMREQ=[eq num],[register h0],[register b1],[register b2],[register a1],[register a2]\n");
		printf("  \r[AUMREQ] RIGHT MIC EQ DISABLE Usage: AUMREQ=[eq num]\n");
		printf("  \r     Enter the register coefficients obtained from biquad calculator\n");

		printf("  \r     [eq num]=0~4\n");
		printf("  \r     [register]=<num in hex> \n");
		printf("  \r     OPEN EQ by AUMREQ=0,0x01,0x02,0x03,x04,0x05\n");
		printf("  \r     CLOSE EQ by AUMREQ=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc == 7) {
		EQ_num = atoi(argv[1]);
		if (EQ_num >= 0 && EQ_num <= 4) {
			printf("ENABLE RIGHT DMIC EQ: %d \r\n", EQ_num);
			audio_save_params.mic_r_eq[EQ_num].eq_enable = 1;
			audio_save_params.mic_r_eq[EQ_num].eq_h0 = (int)strtol(argv[2], NULL, 16);
			audio_save_params.mic_r_eq[EQ_num].eq_b1 = (int)strtol(argv[3], NULL, 16);
			audio_save_params.mic_r_eq[EQ_num].eq_b2 = (int)strtol(argv[4], NULL, 16);
			audio_save_params.mic_r_eq[EQ_num].eq_a1 = (int)strtol(argv[5], NULL, 16);
			audio_save_params.mic_r_eq[EQ_num].eq_a2 = (int)strtol(argv[6], NULL, 16);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else {
			printf(" Set Wrong EQ: %d Not between 0~4\r\n", EQ_num);
		}
	} else if (argc == 2) {
		EQ_num = atoi(argv[1]);
		if (EQ_num >= 0 && EQ_num <= 4) {
			printf("DISABLE RIGHT DMIC EQ: %d \r\n", EQ_num);
			audio_save_params.mic_r_eq[EQ_num].eq_enable = 0;
			audio_save_params.mic_r_eq[EQ_num].eq_h0 = 0;
			audio_save_params.mic_r_eq[EQ_num].eq_b1 = 0;
			audio_save_params.mic_r_eq[EQ_num].eq_b2 = 0;
			audio_save_params.mic_r_eq[EQ_num].eq_a1 = 0;
			audio_save_params.mic_r_eq[EQ_num].eq_a2 = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else {
			printf(" Set Wrong EQ: %d Not between 0~4\r\n", EQ_num);
		}
	} else {
		printf(" argument number = %d \r\n", argc - 1);
		printf(" MISS some arguments AUMREQ=[eq num],[register h0],[register a1],[register a2],[register b1],[register b2]\r\n");
	}
}

//Reset the MIC EQ without audio reset
void fAUMICEQR(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_MICEQ_RESET, 0);
}

//Set the AEC
void fAUAEC(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
#if defined(CONFIG_NEWAEC) && CONFIG_NEWAEC
	if (!arg) {
		printf("\n\r[AUAEC] Enable AEC or not: AUAEC=[enable],[PPLevel],[EchoTailLen],[CNGEnable],[DTControl],[ConvergenceTime]\n");

		printf("  \r     [enable]=0 or 1\n");
		printf("  \r     [PPLevel]=1~50, the fine tune value of AEC, the higher level more aggressive\n");
		printf("  \r     [EchoTailLen]=32/64/128, the length of the buffer that the echo cancel process will be rely on, the higher it set, the cpu usage is higher\n");
		printf("  \r     [CNGEnable]=0/1, enable the comfort noise generate or not\n");
		printf("  \r     [DTControl]=1~3, the coarse tune value of AEC, the higher level more aggressive\n");
		printf("  \r     [ConvergenceTime]=100~1000, AEC initialization convergence time in msec\n");

		printf("  \r     OPEN AEC by AUAEC=1,4,64,1,1,100\n");
		printf("  \r     CLOSE AEC by AUAEC=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		if (atoi(argv[1]) == 0) {
			rx_asp_params.aec_cfg.AEC_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
		} else {
			if (argc >= 7) {
				rx_asp_params.aec_cfg.PPLevel = atoi(argv[2]);
				if (rx_asp_params.aec_cfg.PPLevel < 1 || rx_asp_params.aec_cfg.PPLevel > 50) {
					rx_asp_params.aec_cfg.PPLevel = 6;
					printf("invalid AEC level set to default %d \r\n", rx_asp_params.aec_cfg.PPLevel);
				}
				rx_asp_params.aec_cfg.EchoTailLen = atoi(argv[3]);
				rx_asp_params.aec_cfg.CNGEnable = atoi(argv[4]);
				rx_asp_params.aec_cfg.DTControl = atoi(argv[5]);
				rx_asp_params.aec_cfg.ConvergenceTime = atoi(argv[6]);
				if (rx_asp_params.aec_cfg.ConvergenceTime < 100 || rx_asp_params.aec_cfg.ConvergenceTime > 1000) {
					rx_asp_params.aec_cfg.ConvergenceTime = 100;
					printf("invalid AEC ConvergenceTime set to default %d \r\n", rx_asp_params.aec_cfg.ConvergenceTime);
				}
			}
			printf("set AEC level: %d, DTControl: %d, sdelay: %d CNG: %d, ConvergenceTime: %d\r\n", rx_asp_params.aec_cfg.PPLevel, rx_asp_params.aec_cfg.DTControl,
				   rx_asp_params.aec_cfg.EchoTailLen, rx_asp_params.aec_cfg.CNGEnable, rx_asp_params.aec_cfg.ConvergenceTime);
			rx_asp_params.aec_cfg.AEC_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
		}
	}
#else
	if (!arg) {
		printf("\n\r[AUAEC] Enable AEC or not: AUAEC=[enable],[level],[sdelay],[CNG_enasble]\n");

		printf("  \r     [enable]=0 or 1\n");
		printf("  \r     OPEN AEC by AUAEC=1,3,64,1\n");
		printf("  \r     CLOSE AEC by AUAEC=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		if (atoi(argv[1]) == 0) {
			rx_asp_params.aec_cfg.AEC_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
		} else {
			if (argc >= 5) {
				rx_asp_params.aec_cfg.AECLevel = atoi(argv[2]);
				rx_asp_params.aec_cfg.FilterLength = atoi(argv[3]);
				rx_asp_params.aec_cfg.CNGEnable = atoi(argv[4]);
			}
			printf("set AEC level: %d, sdelay: %d CNG: %d\r\n", rx_asp_params.aec_cfg.AECLevel, rx_asp_params.aec_cfg.FilterLength, rx_asp_params.aec_cfg.CNGEnable);
			rx_asp_params.aec_cfg.AEC_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
		}
	}
#endif
}

//Set the AEC
void fAUAECRUN(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUAECRUN] Enable AEC run or stop: AUAECRUN=[enable]\n");

		printf("  \r     [enable]=0 or 1, 1 set to run, 0 set to close\n");

		printf("  \r     OPEN AEC by AUAEC=1,4,64,1,1,100\n");
		printf("  \r     CLOSE AEC by AUAEC=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		if (atoi(argv[1]) == 0) {
			printf("AEC run set down\r\n");
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_RUN_AEC, 0);
		} else {
			printf("AEC run set on\r\n");
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_RUN_AEC, 1);
		}
	}
}

//Set mic NS level
void fAUNS(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

#if defined(CONFIG_NEWAEC) && CONFIG_NEWAEC
	if (!arg) {
		printf("\n\r[AUNS] Set up mic NS module: AUNS=[NS_enable],[NS_level],[HPFEnable],[QuickConvergenceEnable]\n");

		printf("  \r     [NS_enable]=0 or 1\n");
		printf("  \r     [NS_level]=3~35 to set the NS level (dB), more NS level more aggressive\n");
		printf("  \r     [HPFEnable]=0/1 to enable the HPF before NS or not\n");
		printf("  \r     [QuickConvergenceEnable]=0/1 to the NS convergence speed, immediately suppress(quick) 1, smooth suppress 0\n");
		printf("  \r     Set NS in AEC process with NS level 3 by AUNS=1,3\n");
		printf("  \r     Set NS in AEC process with NS level 3, wihtout HPF and quick drop by AUNS=1,3,0,1\n");
		printf("  \r     Disable NS by AUNS=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		if (atoi(argv[1])) {
			if (argc >= 3) {
				rx_asp_params.ns_cfg.NSLevel = atoi(argv[2]);
				if (rx_asp_params.ns_cfg.NSLevel < 3 || rx_asp_params.ns_cfg.NSLevel > 35) {
					rx_asp_params.ns_cfg.NSLevel = 3;
					printf("Invalid NS level (not in 3~35), set the NS level to defualt value %d, HPFEnable %d\r\n", rx_asp_params.ns_cfg.NSLevel, rx_asp_params.ns_cfg.HPFEnable);
				}
			}
			if (argc >= 4) {
				rx_asp_params.ns_cfg.HPFEnable = atoi(argv[3]);
			}
			if (argc >= 5) {
				rx_asp_params.ns_cfg.QuickConvergenceEnable = atoi(argv[4]);
			}
			rx_asp_params.ns_cfg.NS_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
			printf("Enable mic NS = %x, level %d, HPF %d, QuickConvergence %d\r\n", rx_asp_params.ns_cfg.NS_EN, rx_asp_params.ns_cfg.NSLevel,
				   rx_asp_params.ns_cfg.HPFEnable, rx_asp_params.ns_cfg.QuickConvergenceEnable);
		} else {
			rx_asp_params.ns_cfg.NS_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
			printf("Disable mic NS = %x\r\n", rx_asp_params.ns_cfg.NS_EN);
		}
	}

#else

	if (!arg) {
		printf("\n\r[AUNS] Set up NS module: AUNS=[NS_enable],[NS_level]\n");

		printf("  \r     [NS_enable]=0 or 1 or 2\n");

		printf("  \r     [NS_level]=0~3\n");
		printf("  \r     NS_level=0~3 to set the NS level (dB), more NS level more aggressive\n");
		printf("  \r     Set extra NS with NS level 3 by AUNS=1,3\n");
		printf("  \r     Set NS in AEC process with NS level 3 by AUNS=2,3\n");
		printf("  \r     Disable NS by AUNS=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		if (atoi(argv[1]) == 1) {
			if (argc >= 3) {
				rx_asp_params.ns_cfg.NSLevel = atoi(argv[2]);
				if (rx_asp_params.ns_cfg.NSLevel < 0 || rx_asp_params.ns_cfg.NSLevel > 3) {
					rx_asp_params.ns_cfg.NSLevel = 3;
				}
			}

			rx_asp_params.ns_cfg.NS_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
			printf("Enable Extra NS = %x\r\n", rx_asp_params.ns_cfg.NS_EN);
		} else if (atoi(argv[1]) == 2) {
			if (argc >= 3) {
				rx_asp_params.aec_cfg.NSLevel = atoi(argv[2]);
				if (rx_asp_params.aec_cfg.NSLevel < 0 || rx_asp_params.aec_cfg.NSLevel > 3) {
					rx_asp_params.aec_cfg.NSLevel = 3;
				}

				rx_asp_params.aec_cfg.NS_EN = 1;
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
				printf("Enable AEC NS = %x\r\n", rx_asp_params.aec_cfg.NS_EN);
			} else {
				rx_asp_params.ns_cfg.NS_EN = 0;
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
				printf("Disable Extra NS = %x\r\n", rx_asp_params.ns_cfg.NS_EN);
			}
		}
	}
#endif
}

//Set mic AGC level
void fAUAGC(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
#if defined(CONFIG_NEWAEC) && CONFIG_NEWAEC
	if (!arg) {
		printf("\n\r[AUAGC] Set up mic AGC module: AUAGC=[AGC_enable],[AGC_mode],[AGC_referenceLvl],[AGC_ref_threshold],[AGC_AttackTime],[AGC_ReleaseTime],[AGC_Ratio1],[AGC_Ratio2],[AGC_Ratio3],[AGC_Threshold1],[AGC_Threshold2],[AGC_NoiseGateLvl],[AGC_KneeWidth],[AGC_NoiseFloorAdaptEnable],[AGC_RMSDetectorEnable]\n");
		printf("  \r     [AGC_referenceLvl], output target reference level (dBFS), support 0,1,..,30 (0,-1,…,-30dBFs)\n");
		printf("  \r     [AGC_ref_threshold], limits the output to this level, only support in CT_DRC and CT_LIMITER), support 0,1,..,30 (0,-1,…,-30dBFs)\n");
		printf("  \r     [AGC_AttackTime] is the transition time of changes to signal amplitude compression, 20~1000\n");
		printf("  \r     [AGC_ReleaseTime] is the transition time of changes to signal amplitude boost, 20~1000\n");
		printf("  \r     [AGC_Ratio1] is the compression ratio for ReferenceLvl/RefThreshold, 1,2,3….,50 (slope 1,1/2,1/3….,1/50)\n");
		printf("  \r     [AGC_Ratio2] is the compression ratio for Threshold1, 1,2,3….,50 (slope 1,1/2,1/3….,1/50)\n");
		printf("  \r     [AGC_Ratio3] is the compression ratio for Threshold2, 1,2,3….,50 (slope 1,1/2,1/3….,1/50)\n");
		printf("  \r     [AGC_Threshold1] is the parameter determines the second knee of the curve, 0,1…,40 (0,-1,…,-40dBFs)\n");
		printf("  \r     [AGC_Threshold2] is the parameter determines the third knee of the curve, 0,1…,40 (0,-1,…,-40dBFs)\n");
		printf("  \r     [AGC_NoiseGateLvl] is the noise floor level, 50,51,…,70 (-50,-51,…,-70dBFs),\n");
		printf("  \r     [AGC_KneeWidth] is the knee width, 0,1,2,...,10 (0,1,2,…,10dBFs),\n");
		printf("  \r     [AGC_NoiseFloorAdaptEnable] is to use noise detect on AGC or not, if enable the AGC will ignore some background noise\n");
		printf("  \r     [AGC_RMSDetectorEnable] '1' - RMS based signal level detector, '0' - peak input level detector\n");

		printf("  \r     Set mic AGC with attack time and release time by AUAGC=1,1,6,6,20,20,50,10,2,20,30,50\n");
		printf("  \r     Set mic AGC with attack time and release time, kneewidth 1dB, noise adaptenable 1 and detect with RMS level by AUAGC=1,1,6,6,20,20,50,10,2,20,30,50,1,1,1\n");
		printf("  \r     Disable mic AGC by AUAGC=0\n");

		return;
	}
	argc = parse_param(arg, argv);
	if (argc) {
		if (atoi(argv[1]) == 1) {
			if (argc >= 16) {
				//CT_ALC = 0, CT_LIMITER = 1
				rx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (rx_asp_params.agc_cfg.AGCMode != CT_ALC && rx_asp_params.agc_cfg.AGCMode != CT_LIMITER) {
					rx_asp_params.agc_cfg.AGCMode = CT_ALC;
				}
				rx_asp_params.agc_cfg.ReferenceLvl = atoi(argv[3]);
				if (rx_asp_params.agc_cfg.ReferenceLvl < 0 || rx_asp_params.agc_cfg.ReferenceLvl > 30) {
					rx_asp_params.agc_cfg.ReferenceLvl = 6;
				}
				rx_asp_params.agc_cfg.RefThreshold = atoi(argv[4]);
				if (rx_asp_params.agc_cfg.RefThreshold < 0 || rx_asp_params.agc_cfg.RefThreshold > 30) {
					rx_asp_params.agc_cfg.RefThreshold = 6;
				}
				rx_asp_params.agc_cfg.AttackTime = atoi(argv[5]);
				if (rx_asp_params.agc_cfg.AttackTime < 0 || rx_asp_params.agc_cfg.AttackTime > 500) {
					rx_asp_params.agc_cfg.AttackTime = 10;
				}
				rx_asp_params.agc_cfg.ReleaseTime = atoi(argv[6]);
				if (rx_asp_params.agc_cfg.ReleaseTime < 0 || rx_asp_params.agc_cfg.ReleaseTime > 500) {
					rx_asp_params.agc_cfg.ReleaseTime = 10;
				}
				rx_asp_params.agc_cfg.Ratio[0] = atoi(argv[7]);
				if (rx_asp_params.agc_cfg.Ratio[0] < 1 || rx_asp_params.agc_cfg.Ratio[0] > 50) {
					rx_asp_params.agc_cfg.Ratio[0] = 50;
				}
				rx_asp_params.agc_cfg.Ratio[1] = atoi(argv[8]);
				if (rx_asp_params.agc_cfg.Ratio[1] < 1 || rx_asp_params.agc_cfg.Ratio[1] > 50) {
					rx_asp_params.agc_cfg.Ratio[1] = 50;
				}
				rx_asp_params.agc_cfg.Ratio[2] = atoi(argv[9]);
				if (rx_asp_params.agc_cfg.Ratio[2] < 1 || rx_asp_params.agc_cfg.Ratio[2] > 50) {
					rx_asp_params.agc_cfg.Ratio[2] = 50;
				}
				rx_asp_params.agc_cfg.Threshold[0] = atoi(argv[10]);
				if (rx_asp_params.agc_cfg.Threshold[0] < 0 || rx_asp_params.agc_cfg.Threshold[0] > 40) {
					rx_asp_params.agc_cfg.Threshold[0] = 20;
				}
				rx_asp_params.agc_cfg.Threshold[1] = atoi(argv[11]);
				if (rx_asp_params.agc_cfg.Threshold[1] < 0 || rx_asp_params.agc_cfg.Threshold[1] > 40) {
					rx_asp_params.agc_cfg.Threshold[1] = 30;
				}
				rx_asp_params.agc_cfg.Threshold[2] = atoi(argv[12]);
				if (rx_asp_params.agc_cfg.Threshold[2] < 40 || rx_asp_params.agc_cfg.Threshold[2] > 100) {
					rx_asp_params.agc_cfg.Threshold[2] = 40;
				}
				rx_asp_params.agc_cfg.KneeWidth = atoi(argv[13]);
				if (rx_asp_params.agc_cfg.KneeWidth < 0 || rx_asp_params.agc_cfg.KneeWidth > 10) {
					rx_asp_params.agc_cfg.KneeWidth = 0;
				}
				rx_asp_params.agc_cfg.NoiseFloorAdaptEnable = atoi(argv[14]);
				if (rx_asp_params.agc_cfg.NoiseFloorAdaptEnable < 0 || rx_asp_params.agc_cfg.NoiseFloorAdaptEnable > 1) {
					rx_asp_params.agc_cfg.NoiseFloorAdaptEnable = 1;
				}
				rx_asp_params.agc_cfg.RMSDetectorEnable = atoi(argv[15]);
				if (rx_asp_params.agc_cfg.RMSDetectorEnable < 0 || rx_asp_params.agc_cfg.RMSDetectorEnable > 1) {
					rx_asp_params.agc_cfg.RMSDetectorEnable = 1;
				}
				printf("Enable mic Extra AGC_referenceLvl %d, AGC_ref_threshold %d, AGC_AttackTime %d, AGC_ReleaseTime %d\r\n", rx_asp_params.agc_cfg.ReferenceLvl,
					   rx_asp_params.agc_cfg.RefThreshold, rx_asp_params.agc_cfg.AttackTime, rx_asp_params.agc_cfg.ReleaseTime);
				printf("Enable mic Extra AGC_Ratio1 %d, AGC_Ratio2 %d, AGC_Ratio3 %d\r\n", rx_asp_params.agc_cfg.Ratio[0], rx_asp_params.agc_cfg.Ratio[1],
					   rx_asp_params.agc_cfg.Ratio[2]);
				printf("Enable mic Extra AGC_Threshold1 %d, AGC_Threshold2 %d, AGC_NoiseGateLvl %d, KneeWidth %d NoiseFloorAdaptEnable %d RMSDetectorEnable %d\r\n",
					   rx_asp_params.agc_cfg.Threshold[0],
					   rx_asp_params.agc_cfg.Threshold[1],
					   rx_asp_params.agc_cfg.Threshold[2], rx_asp_params.agc_cfg.KneeWidth, rx_asp_params.agc_cfg.NoiseFloorAdaptEnable, rx_asp_params.agc_cfg.RMSDetectorEnable);
			} else if (argc >= 13) {
				//CT_ALC = 0, CT_LIMITER = 1
				rx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (rx_asp_params.agc_cfg.AGCMode != CT_ALC && rx_asp_params.agc_cfg.AGCMode != CT_LIMITER) {
					rx_asp_params.agc_cfg.AGCMode = CT_ALC;
				}
				rx_asp_params.agc_cfg.ReferenceLvl = atoi(argv[3]);
				if (rx_asp_params.agc_cfg.ReferenceLvl < 0 || rx_asp_params.agc_cfg.ReferenceLvl > 30) {
					rx_asp_params.agc_cfg.ReferenceLvl = 6;
				}
				rx_asp_params.agc_cfg.RefThreshold = atoi(argv[4]);
				if (rx_asp_params.agc_cfg.RefThreshold < 0 || rx_asp_params.agc_cfg.RefThreshold > 30) {
					rx_asp_params.agc_cfg.RefThreshold = 6;
				}
				rx_asp_params.agc_cfg.AttackTime = atoi(argv[5]);
				if (rx_asp_params.agc_cfg.AttackTime < 0 || rx_asp_params.agc_cfg.AttackTime > 500) {
					rx_asp_params.agc_cfg.AttackTime = 10;
				}
				rx_asp_params.agc_cfg.ReleaseTime = atoi(argv[6]);
				if (rx_asp_params.agc_cfg.ReleaseTime < 0 || rx_asp_params.agc_cfg.ReleaseTime > 500) {
					rx_asp_params.agc_cfg.ReleaseTime = 10;
				}
				rx_asp_params.agc_cfg.Ratio[0] = atoi(argv[7]);
				if (rx_asp_params.agc_cfg.Ratio[0] < 1 || rx_asp_params.agc_cfg.Ratio[0] > 50) {
					rx_asp_params.agc_cfg.Ratio[0] = 50;
				}
				rx_asp_params.agc_cfg.Ratio[1] = atoi(argv[8]);
				if (rx_asp_params.agc_cfg.Ratio[1] < 1 || rx_asp_params.agc_cfg.Ratio[1] > 50) {
					rx_asp_params.agc_cfg.Ratio[1] = 50;
				}
				rx_asp_params.agc_cfg.Ratio[2] = atoi(argv[9]);
				if (rx_asp_params.agc_cfg.Ratio[2] < 1 || rx_asp_params.agc_cfg.Ratio[2] > 50) {
					rx_asp_params.agc_cfg.Ratio[2] = 50;
				}
				rx_asp_params.agc_cfg.Threshold[0] = atoi(argv[10]);
				if (rx_asp_params.agc_cfg.Threshold[0] < 0 || rx_asp_params.agc_cfg.Threshold[0] > 40) {
					rx_asp_params.agc_cfg.Threshold[0] = 20;
				}
				rx_asp_params.agc_cfg.Threshold[1] = atoi(argv[11]);
				if (rx_asp_params.agc_cfg.Threshold[1] < 0 || rx_asp_params.agc_cfg.Threshold[1] > 40) {
					rx_asp_params.agc_cfg.Threshold[1] = 30;
				}
				rx_asp_params.agc_cfg.Threshold[2] = atoi(argv[12]);
				if (rx_asp_params.agc_cfg.Threshold[2] < 40 || rx_asp_params.agc_cfg.Threshold[2] > 100) {
					rx_asp_params.agc_cfg.Threshold[2] = 40;
				}
				printf("Enable mic Extra AGC_referenceLvl %d, AGC_ref_threshold %d, AGC_AttackTime %d, AGC_ReleaseTime %d\r\n", rx_asp_params.agc_cfg.ReferenceLvl,
					   rx_asp_params.agc_cfg.RefThreshold, rx_asp_params.agc_cfg.AttackTime, rx_asp_params.agc_cfg.ReleaseTime);
				printf("Enable mic Extra AGC_Ratio1 %d, AGC_Ratio2 %d, AGC_Ratio3 %d\r\n", rx_asp_params.agc_cfg.Ratio[0], rx_asp_params.agc_cfg.Ratio[1],
					   rx_asp_params.agc_cfg.Ratio[2]);
				printf("Enable mic Extra AGC_Threshold1 %d, AGC_Threshold2 %d, AGC_NoiseGateLvl %d\r\n", rx_asp_params.agc_cfg.Threshold[0], rx_asp_params.agc_cfg.Threshold[1],
					   rx_asp_params.agc_cfg.Threshold[2]);
			} else if (argc >= 9) {
				//CT_ALC = 0, CT_LIMITER = 1
				rx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (rx_asp_params.agc_cfg.AGCMode != CT_ALC && rx_asp_params.agc_cfg.AGCMode != CT_LIMITER) {
					rx_asp_params.agc_cfg.AGCMode = CT_ALC;
				}
				rx_asp_params.agc_cfg.ReferenceLvl = atoi(argv[3]);
				if (rx_asp_params.agc_cfg.ReferenceLvl < 0 || rx_asp_params.agc_cfg.ReferenceLvl > 30) {
					rx_asp_params.agc_cfg.ReferenceLvl = 6;
				}
				rx_asp_params.agc_cfg.RefThreshold = atoi(argv[4]);
				if (rx_asp_params.agc_cfg.RefThreshold < 0 || rx_asp_params.agc_cfg.RefThreshold > 30) {
					rx_asp_params.agc_cfg.RefThreshold = 6;
				}
				rx_asp_params.agc_cfg.AttackTime = atoi(argv[5]);
				if (rx_asp_params.agc_cfg.AttackTime < 0 || rx_asp_params.agc_cfg.AttackTime > 500) {
					rx_asp_params.agc_cfg.AttackTime = 0;
				}
				rx_asp_params.agc_cfg.ReleaseTime = atoi(argv[6]);
				if (rx_asp_params.agc_cfg.ReleaseTime < 0 || rx_asp_params.agc_cfg.ReleaseTime > 500) {
					rx_asp_params.agc_cfg.ReleaseTime = 0;
				}
				rx_asp_params.agc_cfg.Ratio[0] = atoi(argv[7]);
				if (rx_asp_params.agc_cfg.Ratio[0] < 1 || rx_asp_params.agc_cfg.Ratio[0] > 50) {
					rx_asp_params.agc_cfg.Ratio[0] = 50;
				}
				rx_asp_params.agc_cfg.Threshold[0] = atoi(argv[8]);
				if (rx_asp_params.agc_cfg.Threshold[0] < 0 || rx_asp_params.agc_cfg.Threshold[0] > 40) {
					rx_asp_params.agc_cfg.Threshold[0] = 20;
				}
				printf("Enable mic Extra AGC_referenceLvl %d, AGC_ref_threshold %d, AGC_AttackTime %d, AGC_ReleaseTime %d\r\n", rx_asp_params.agc_cfg.ReferenceLvl,
					   rx_asp_params.agc_cfg.RefThreshold, rx_asp_params.agc_cfg.AttackTime, rx_asp_params.agc_cfg.ReleaseTime);
				printf("Enable mic Extra AGC_Ratio1 %d, AGC_Threshold1 %d\r\n", rx_asp_params.agc_cfg.Ratio[0], rx_asp_params.agc_cfg.Threshold[0]);
			} else if (argc >= 7) {
				//CT_ALC = 0, CT_LIMITER = 1
				rx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (rx_asp_params.agc_cfg.AGCMode != CT_ALC && rx_asp_params.agc_cfg.AGCMode != CT_LIMITER) {
					rx_asp_params.agc_cfg.AGCMode = CT_ALC;
				}
				rx_asp_params.agc_cfg.ReferenceLvl = atoi(argv[3]);
				if (rx_asp_params.agc_cfg.ReferenceLvl < 0 || rx_asp_params.agc_cfg.ReferenceLvl > 30) {
					rx_asp_params.agc_cfg.ReferenceLvl = 6;
				}
				rx_asp_params.agc_cfg.RefThreshold = atoi(argv[4]);
				if (rx_asp_params.agc_cfg.RefThreshold < 0 || rx_asp_params.agc_cfg.RefThreshold > 30) {
					rx_asp_params.agc_cfg.RefThreshold = 6;
				}
				rx_asp_params.agc_cfg.AttackTime = atoi(argv[5]);
				if (rx_asp_params.agc_cfg.AttackTime < 0 || rx_asp_params.agc_cfg.AttackTime > 500) {
					rx_asp_params.agc_cfg.AttackTime = 0;
				}
				rx_asp_params.agc_cfg.ReleaseTime = atoi(argv[6]);
				if (rx_asp_params.agc_cfg.ReleaseTime < 0 || rx_asp_params.agc_cfg.ReleaseTime > 500) {
					rx_asp_params.agc_cfg.ReleaseTime = 0;
				}
				printf("Enable mic Extra AGC_referenceLvl %d, AGC_ref_threshold %d, AGC_AttackTime %d, AGC_ReleaseTime %d\r\n", rx_asp_params.agc_cfg.ReferenceLvl,
					   rx_asp_params.agc_cfg.RefThreshold, rx_asp_params.agc_cfg.AttackTime, rx_asp_params.agc_cfg.ReleaseTime);
			}
			rx_asp_params.agc_cfg.AGC_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
			printf("Enable speaker Extra AGC = %x\r\n", rx_asp_params.agc_cfg.AGC_EN);
		} else {
			rx_asp_params.agc_cfg.AGC_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
			printf("Disable speaker Extra AGC = %x\r\n", rx_asp_params.agc_cfg.AGC_EN);
		}
	}
#else
	if (!arg) {
		printf("\n\r[AUAGC] Set up AGC module: AUAGC=[AGC_enable],[AGC_mode],[AGC_targetLevelDbfs],[AGC_compression_gain],[AGC_limiter_enable]\n");

		printf("  \r     [AGC_enable]=0 or 1 or 2\n");
		printf("  \r     [AGC_mode]=0~3,0 kAgcModeUnchanged, 1 kAgcModeAdaptiveAnalog, 2 kAgcModeAdaptiveDigital, 3 kAgcModeFixedDigital\n");
		printf("  \r     [AGC_targetLevelDbfs], the target target level is -[AGC_AGC_targetLevelDbfs], means the ouput will be controled under than 0dBFS\n");
		printf("  \r     [AGC_compression_gain]=AGC compression gain in dB\n");
		printf("  \r     [AGC_limiter_enable]=0 or 1, enable agc limiter or not\n");
		printf("  \r     Set extra AGC with gain 18dB, enable limiter by AUAGC=1,0,18,1\n");
		printf("  \r     Set AGC in AEC process withwith gain 18dB, disable limiter by AUAGC=2,2,0,18,1\n");
		printf("  \r     Disable extar AGC by AUAGC=0\n");
		return;
	}

	argc = parse_param(arg, argv);

	if (argc) {
		if (atoi(argv[1]) == 1) {
			if (argc >= 6) {
				rx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (rx_asp_params.agc_cfg.AGCMode < 0 || rx_asp_params.agc_cfg.AGCMode > 3) {
					rx_asp_params.agc_cfg.AGCMode = 2;
				}
				rx_asp_params.agc_cfg.TargetLevelDbfs = atoi(argv[3]);
				if (rx_asp_params.agc_cfg.TargetLevelDbfs < 0 || rx_asp_params.agc_cfg.TargetLevelDbfs > 31) {
					rx_asp_params.agc_cfg.TargetLevelDbfs = 0;
				}
				rx_asp_params.agc_cfg.CompressionGaindB = atoi(argv[4]);
				if (rx_asp_params.agc_cfg.CompressionGaindB < 0 || rx_asp_params.agc_cfg.CompressionGaindB > 90) {
					rx_asp_params.agc_cfg.CompressionGaindB = 10;
				}
				if (atoi(argv[5])) {
					rx_asp_params.agc_cfg.LimiterEnable = 1;
				} else {
					rx_asp_params.agc_cfg.LimiterEnable = 0;
				}
			}

			rx_asp_params.agc_cfg.AGC_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
			printf("Enable mic Extra AGC = %d, %d, %d, %d, %d\r\n", rx_asp_params.agc_cfg.AGC_EN, rx_asp_params.agc_cfg.AGCMode, rx_asp_params.agc_cfg.TargetLevelDbfs,
				   rx_asp_params.agc_cfg.CompressionGaindB, rx_asp_params.agc_cfg.LimiterEnable);
		} else if (atoi(argv[1]) == 2) {
			if (argc >= 6) {
				rx_asp_params.aec_cfg.AGCMode = atoi(argv[2]);
				if (rx_asp_params.aec_cfg.AGCMode < 0 || rx_asp_params.aec_cfg.AGCMode > 3) {
					rx_asp_params.aec_cfg.AGCMode = 2;
				}
				rx_asp_params.aec_cfg.TargetLevelDbfs = atoi(argv[3]);
				if (rx_asp_params.aec_cfg.TargetLevelDbfs < 0 || rx_asp_params.aec_cfg.TargetLevelDbfs > 31) {
					rx_asp_params.aec_cfg.TargetLevelDbfs = 0;
				}
				rx_asp_params.aec_cfg.CompressionGaindB = atoi(argv[4]);
				if (rx_asp_params.aec_cfg.CompressionGaindB < 0 || rx_asp_params.aec_cfg.CompressionGaindB > 90) {
					rx_asp_params.aec_cfg.CompressionGaindB = 10;
				}
				if (atoi(argv[5])) {
					rx_asp_params.aec_cfg.LimiterEnable = 1;
				} else {
					rx_asp_params.aec_cfg.LimiterEnable = 0;
				}
			}
			rx_asp_params.aec_cfg.AGC_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
			printf("Enable mic AGC in AEC = %d, %d, %d, %d, %d\r\n", rx_asp_params.aec_cfg.AGC_EN, rx_asp_params.aec_cfg.AGCMode, rx_asp_params.aec_cfg.TargetLevelDbfs,
				   rx_asp_params.aec_cfg.CompressionGaindB, rx_asp_params.aec_cfg.LimiterEnable);
		} else {
			rx_asp_params.agc_cfg.AGC_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RXASP_PARAM, (int)&rx_asp_params);
			printf("Disable mic Extra AGC = %x\r\n", rx_asp_params.agc_cfg.AGC_EN);
		}
	}

#endif
}

//Set mic mute
void fAUMICM(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	int mic_mute = 0;
	if (!arg) {
		printf("\n\r[AUMICM] Mute MIC or not: AUMICM=[enable_mute]\n");

		printf("  \r     [enable_mute]=0 or 1\n");
		printf("  \r     MUTE MIC by AUMICM=1\n");
		printf("  \r     UNMUTE MIC by AUMICM=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		mic_mute = atoi(argv[1]);
		if (mic_mute == 0) {
			printf("Disable MIC Mute\r\n");
			audio_save_params.ADC_mute = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_MIC_ENABLE, 1);
		} else {
			printf("Enable MIC Mute\r\n");
			audio_save_params.ADC_mute = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_MIC_ENABLE, 0);
		}
	}
}

#if P2P_ENABLE
extern int gProcessRun;
//open audio RX p2p streaming
void fAURXP2P(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	if (!arg) {
		printf("\n\r[AURXP2P] Enable P2P RX streaming: AURXP2P=[RX_P2P_enable]\n");

		printf("  \r     [RX_P2P_enable]=0 or 1\n");
		printf("  \r     enable RX_P2P \n");
		printf("  \r     Enable RX P2P streaming by AURXP2P=1\n");
		printf("  \r     Disable RX P2P streaming by AURXP2P=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		if (gProcessRun) {
			if (atoi(argv[1])) {
				mm_module_ctrl(p2p_audio_ctx, CMD_P2P_AUDIO_STREAMING, 1);
				printf("RX P2P STREAMING ON\r\n");
			} else {
				mm_module_ctrl(p2p_audio_ctx, CMD_P2P_AUDIO_STREAMING, 0);
				printf("RX P2P STREAMING OFF\r\n");
			}
		} else {
			printf("skynet net open\r\n");
		}
	}
}
extern int skyNetModuleDeinit(void);
extern void resetClients(void);
extern int getIPNotice;
extern void deinitAVInfo(void);
extern void skynet_device_run(void);
//enable p2p streaming
int test_i = 1;
void fP2PEN(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	if (!arg) {
		printf("\n\r	Enable P2P client: P2PEN=[P2P_enable]\n");

		printf("  \r     [P2P_enable]=0 or 1\n");
		printf("  \r     enable P2P_enable \n");
		printf("  \r     Enable P2P client by P2PEN=1\n");
		printf("  \r     Disable P2P client by P2PEN=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		if (atoi(argv[1])) {
			if (getIPNotice == 0) {
				gProcessRun = 1;
				getIPNotice = 1;
				skynet_device_run();
				printf("Turn on P2P\r\n");
			} else {
				printf("P2P is already ON\r\n");
			}
		} else {
			if (gProcessRun == 1) {
				resetClients();
				gProcessRun = 0;
				vTaskDelay(5000);
				skyNetModuleDeinit();
				deinitAVInfo();
			} else {
				printf("P2P is already OFF\r\n");
			}
		}
	}
}

#endif
//********************//
//Speaker setting function
//********************//
//Set speaker NS level
void fAUSPNS(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

#if defined(CONFIG_NEWAEC) && CONFIG_NEWAEC
	if (!arg) {
		printf("\n\r[AUSPNS] Set up speaker NS module: AUSPNS=[NS_enable],[NS_level],[HPFEnable],[QuickConvergenceEnable]\n");

		printf("  \r     [NS_enable]=0 or 1\n");
		printf("  \r     [NS_level_SPK]=3~35 to set the NS level (dB), more NS level more aggressive\n");
		printf("  \r     [NS_HPFEnable]=0/1 to enable the HPF before NS or not]");
		printf("  \r     Set extra speaker NS with NS level 3 by AUSPNS=1,3\n");
		printf("  \r     Set extra speaker NS with NS level 3, wihtout HPF and quick drop by AUSPNS=1,3,0,1\n");
		printf("  \r     Disable speaker NS by AUSPNS=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		if (atoi(argv[1])) {
			if (argc >= 3) {
				tx_asp_params.ns_cfg.NSLevel = atoi(argv[2]);
				if (tx_asp_params.ns_cfg.NSLevel < 3 || tx_asp_params.ns_cfg.NSLevel > 35) {
					tx_asp_params.ns_cfg.NSLevel = 9;
					printf("Invalid NS level (not in 3~35), set the NS level to defualt value %d\r\n", tx_asp_params.ns_cfg.NSLevel);
				}
			}
			if (argc >= 4) {
				tx_asp_params.ns_cfg.HPFEnable = atoi(argv[3]);
			}
			if (argc >= 5) {
				tx_asp_params.ns_cfg.QuickConvergenceEnable = atoi(argv[4]);
			}
			tx_asp_params.ns_cfg.NS_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TXASP_PARAM, (int)&tx_asp_params);
			printf("Enable speaker NS = %x, level %d, HPF %d, QuickConvergence %d\r\n", tx_asp_params.ns_cfg.NS_EN, tx_asp_params.ns_cfg.NSLevel,
				   tx_asp_params.ns_cfg.HPFEnable, tx_asp_params.ns_cfg.QuickConvergenceEnable);
		} else {
			tx_asp_params.ns_cfg.NS_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TXASP_PARAM, (int)&tx_asp_params);
			printf("Disable speaker NS = %x\r\n", tx_asp_params.ns_cfg.NS_EN);
		}
	}
#else
	if (!arg) {
		printf("\n\r[AUSPNS] Set up NS module: AUSPNS=[NS_enable],[NS_level_SPK]\n");

		printf("  \r     [NS_enable]=0 or 1\n");
		printf("  \r     [NS_level_SPK]=0~3\n");
		printf("  \r     NS_level_SPK=0~3 to set the NS level, more NS level more aggressive\n");

		printf("  \r     Set extra speaker NS with NS level 3 by AUSPNS=1,3\n");
		printf("  \r     Disable speaker NS by AUSPNS=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		if (atoi(argv[1])) {
			if (argc >= 3) {
				tx_asp_params.ns_cfg.NSLevel = atoi(argv[2]);
				if (tx_asp_params.ns_cfg.NSLevel >= 0 && tx_asp_params.ns_cfg.NSLevel <= 3) {
					tx_asp_params.ns_cfg.NSLevel = 3;
				}
			}

			tx_asp_params.ns_cfg.NS_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TXASP_PARAM, (int)&tx_asp_params);
			printf("Enable Extra Speaker NS = %x, %d\r\n", tx_asp_params.ns_cfg.NS_EN, tx_asp_params.ns_cfg.NSLevel);
		} else {
			tx_asp_params.ns_cfg.NS_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TXASP_PARAM, (int)&tx_asp_params);
			printf("Disable Extra Speaker NS = %x\r\n", tx_asp_params.ns_cfg.NS_EN);
		}
	}
#endif
}

//Set speaker AGC level
void fAUSPAGC(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
#if defined(CONFIG_NEWAEC) && CONFIG_NEWAEC
	if (!arg) {

		printf("\n\r[AUSPAGC] Set up speaker AGC module: AUSPAGC=[AGC_enable],[AGC_mode],[AGC_referenceLvl],[AGC_ref_threshold],[AGC_AttackTime],[AGC_ReleaseTime],[AGC_Ratio1],[AGC_Ratio2],[AGC_Ratio3],[AGC_Threshold1],[AGC_Threshold2],[AGC_NoiseGateLvl],[AGC_KneeWidth],[AGC_NoiseFloorAdaptEnable],[AGC_RMSDetectorEnable]\n");
		printf("  \r     [AGC_referenceLvl], output target reference level (dBFS), support 0,1,..,30 (0,-1,…,-30dBFs)\n");
		printf("  \r     [AGC_ref_threshold], limits the output to this level, only support in CT_DRC and CT_LIMITER), support 0,1,..,30 (0,-1,…,-30dBFs)\n");
		printf("  \r     [AGC_AttackTime] is the transition time of changes to signal amplitude compression, 1~500\n");
		printf("  \r     [AGC_ReleaseTime] is the transition time of changes to signal amplitude boost, 1~500\n");
		printf("  \r     [AGC_Ratio1] is the compression ratio for ReferenceLvl/RefThreshold, 1,2,3….,50 (slope 1,1/2,1/3….,1/50)\n");
		printf("  \r     [AGC_Ratio2] is the compression ratio for Threshold1, 1,2,3….,50 (slope 1,1/2,1/3….,1/50)\n");
		printf("  \r     [AGC_Ratio3] is the compression ratio for Threshold2, 1,2,3….,50 (slope 1,1/2,1/3….,1/50)\n");
		printf("  \r     [AGC_Threshold1] is the parameter determines the second knee of the curve, 0,1,…,40 (0,-1,…,-40dBFs)\n");
		printf("  \r     [AGC_Threshold2] is the parameter determines the third knee of the curve, 0,1,…,40 (0,-1,…,-40dBFs)\n");
		printf("  \r     [AGC_NoiseGateLvl] is the noise floor level, 50,51,…,70 (-50,-51,…,-70dBFs),\n");
		printf("  \r     [AGC_KneeWidth] is the knee width, 0,1,2,...,10 (0,1,2,…,10dBFs),\n");
		printf("  \r     [AGC_NoiseFloorAdaptEnable] is to use noise detect on AGC or not, if enable the AGC will ignore some background noise\n");
		printf("  \r     [AGC_RMSDetectorEnable] '1' - RMS based signal level detector, '0' - peak input level detector\n");

		printf("  \r     Set speaker AGC with attack time and release time by AUSPAGC=1,1,6,6,20,20,50,10,2,20,30,50\n");
		printf("  \r     Set speaker AGC with attack time and release time, kneewidth 1dB, noise adaptenable 1 and detect with RMS level by AUSPAGC=1,1,6,6,20,20,50,10,2,20,30,50,1,1,1\n");
		printf("  \r     Disable speaker AGC by AUSPAGC=0\n");

	}

	argc = parse_param(arg, argv);

	if (argc) {
		if (atoi(argv[1]) == 1) {
			if (argc >= 16) {
				//CT_ALC = 0, CT_LIMITER = 1
				tx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (tx_asp_params.agc_cfg.AGCMode != CT_ALC && tx_asp_params.agc_cfg.AGCMode != CT_LIMITER) {
					tx_asp_params.agc_cfg.AGCMode = CT_ALC;
				}
				tx_asp_params.agc_cfg.ReferenceLvl = atoi(argv[3]);
				if (tx_asp_params.agc_cfg.ReferenceLvl < 0 || tx_asp_params.agc_cfg.ReferenceLvl > 30) {
					tx_asp_params.agc_cfg.ReferenceLvl = 6;
				}
				tx_asp_params.agc_cfg.RefThreshold = atoi(argv[4]);
				if (tx_asp_params.agc_cfg.RefThreshold < 0 || tx_asp_params.agc_cfg.RefThreshold > 30) {
					tx_asp_params.agc_cfg.RefThreshold = 6;
				}
				tx_asp_params.agc_cfg.AttackTime = atoi(argv[5]);
				if (tx_asp_params.agc_cfg.AttackTime < 0 || tx_asp_params.agc_cfg.AttackTime > 500) {
					tx_asp_params.agc_cfg.AttackTime = 10;
				}
				tx_asp_params.agc_cfg.ReleaseTime = atoi(argv[6]);
				if (tx_asp_params.agc_cfg.ReleaseTime < 0 || tx_asp_params.agc_cfg.ReleaseTime > 500) {
					tx_asp_params.agc_cfg.ReleaseTime = 10;
				}
				tx_asp_params.agc_cfg.Ratio[0] = atoi(argv[7]);
				if (tx_asp_params.agc_cfg.Ratio[0] < 1 || tx_asp_params.agc_cfg.Ratio[0] > 50) {
					tx_asp_params.agc_cfg.Ratio[0] = 50;
				}
				tx_asp_params.agc_cfg.Ratio[1] = atoi(argv[8]);
				if (tx_asp_params.agc_cfg.Ratio[1] < 1 || tx_asp_params.agc_cfg.Ratio[1] > 50) {
					tx_asp_params.agc_cfg.Ratio[1] = 50;
				}
				tx_asp_params.agc_cfg.Ratio[2] = atoi(argv[9]);
				if (tx_asp_params.agc_cfg.Ratio[2] < 1 || tx_asp_params.agc_cfg.Ratio[2] > 50) {
					tx_asp_params.agc_cfg.Ratio[2] = 50;
				}
				tx_asp_params.agc_cfg.Threshold[0] = atoi(argv[10]);
				if (tx_asp_params.agc_cfg.Threshold[0] < 0 || tx_asp_params.agc_cfg.Threshold[0] > 40) {
					tx_asp_params.agc_cfg.Threshold[0] = 20;
				}
				tx_asp_params.agc_cfg.Threshold[1] = atoi(argv[11]);
				if (tx_asp_params.agc_cfg.Threshold[1] < 0 || tx_asp_params.agc_cfg.Threshold[1] > 40) {
					tx_asp_params.agc_cfg.Threshold[1] = 30;
				}
				tx_asp_params.agc_cfg.Threshold[2] = atoi(argv[12]);
				if (tx_asp_params.agc_cfg.Threshold[2] < 40 || tx_asp_params.agc_cfg.Threshold[2] > 100) {
					tx_asp_params.agc_cfg.Threshold[2] = 40;
				}
				tx_asp_params.agc_cfg.KneeWidth = atoi(argv[13]);
				if (tx_asp_params.agc_cfg.KneeWidth < 0 || tx_asp_params.agc_cfg.KneeWidth > 10) {
					tx_asp_params.agc_cfg.KneeWidth = 0;
				}
				tx_asp_params.agc_cfg.NoiseFloorAdaptEnable = atoi(argv[14]);
				if (tx_asp_params.agc_cfg.NoiseFloorAdaptEnable < 0 || tx_asp_params.agc_cfg.NoiseFloorAdaptEnable > 1) {
					tx_asp_params.agc_cfg.NoiseFloorAdaptEnable = 1;
				}
				tx_asp_params.agc_cfg.RMSDetectorEnable = atoi(argv[15]);
				if (tx_asp_params.agc_cfg.RMSDetectorEnable < 0 || tx_asp_params.agc_cfg.RMSDetectorEnable > 1) {
					tx_asp_params.agc_cfg.RMSDetectorEnable = 1;
				}
				printf("Enable speaker Extra AGC_referenceLvl %d, AGC_ref_threshold %d, AGC_AttackTime %d, AGC_ReleaseTime %d\r\n", tx_asp_params.agc_cfg.ReferenceLvl,
					   tx_asp_params.agc_cfg.RefThreshold, tx_asp_params.agc_cfg.AttackTime, tx_asp_params.agc_cfg.ReleaseTime);
				printf("Enable speaker Extra AGC_Ratio1 %d, AGC_Ratio2 %d, AGC_Ratio3 %d\r\n", tx_asp_params.agc_cfg.Ratio[0], tx_asp_params.agc_cfg.Ratio[1],
					   tx_asp_params.agc_cfg.Ratio[2]);
				printf("Enable speaker Extra AGC_Threshold1 %d, AGC_Threshold2 %d, AGC_NoiseGateLvl %d, KneeWidth %d, NoiseFloorAdaptEnable %d, RMSDetectorEnable %d\r\n",
					   tx_asp_params.agc_cfg.Threshold[0],
					   tx_asp_params.agc_cfg.Threshold[1], tx_asp_params.agc_cfg.Threshold[2], tx_asp_params.agc_cfg.KneeWidth, tx_asp_params.agc_cfg.NoiseFloorAdaptEnable,
					   tx_asp_params.agc_cfg.RMSDetectorEnable);
			} else if (argc >= 13) {
				//CT_ALC = 0, CT_LIMITER = 1
				tx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (tx_asp_params.agc_cfg.AGCMode != CT_ALC && tx_asp_params.agc_cfg.AGCMode != CT_LIMITER) {
					tx_asp_params.agc_cfg.AGCMode = CT_ALC;
				}
				tx_asp_params.agc_cfg.ReferenceLvl = atoi(argv[3]);
				if (tx_asp_params.agc_cfg.ReferenceLvl < 0 || tx_asp_params.agc_cfg.ReferenceLvl > 30) {
					tx_asp_params.agc_cfg.ReferenceLvl = 6;
				}
				tx_asp_params.agc_cfg.RefThreshold = atoi(argv[4]);
				if (tx_asp_params.agc_cfg.RefThreshold < 0 || tx_asp_params.agc_cfg.RefThreshold > 30) {
					tx_asp_params.agc_cfg.RefThreshold = 6;
				}
				tx_asp_params.agc_cfg.AttackTime = atoi(argv[5]);
				if (tx_asp_params.agc_cfg.AttackTime < 0 || tx_asp_params.agc_cfg.AttackTime > 500) {
					tx_asp_params.agc_cfg.AttackTime = 10;
				}
				tx_asp_params.agc_cfg.ReleaseTime = atoi(argv[6]);
				if (tx_asp_params.agc_cfg.ReleaseTime < 0 || tx_asp_params.agc_cfg.ReleaseTime > 500) {
					tx_asp_params.agc_cfg.ReleaseTime = 10;
				}
				tx_asp_params.agc_cfg.Ratio[0] = atoi(argv[7]);
				if (tx_asp_params.agc_cfg.Ratio[0] < 1 || tx_asp_params.agc_cfg.Ratio[0] > 50) {
					tx_asp_params.agc_cfg.Ratio[0] = 50;
				}
				tx_asp_params.agc_cfg.Ratio[1] = atoi(argv[8]);
				if (tx_asp_params.agc_cfg.Ratio[1] < 1 || tx_asp_params.agc_cfg.Ratio[1] > 50) {
					tx_asp_params.agc_cfg.Ratio[1] = 50;
				}
				tx_asp_params.agc_cfg.Ratio[2] = atoi(argv[9]);
				if (tx_asp_params.agc_cfg.Ratio[2] < 1 || tx_asp_params.agc_cfg.Ratio[2] > 50) {
					tx_asp_params.agc_cfg.Ratio[2] = 50;
				}
				tx_asp_params.agc_cfg.Threshold[0] = atoi(argv[10]);
				if (tx_asp_params.agc_cfg.Threshold[0] < 0 || tx_asp_params.agc_cfg.Threshold[0] > 40) {
					tx_asp_params.agc_cfg.Threshold[0] = 20;
				}
				tx_asp_params.agc_cfg.Threshold[1] = atoi(argv[11]);
				if (tx_asp_params.agc_cfg.Threshold[1] < 0 || tx_asp_params.agc_cfg.Threshold[1] > 40) {
					tx_asp_params.agc_cfg.Threshold[1] = 30;
				}
				tx_asp_params.agc_cfg.Threshold[2] = atoi(argv[12]);
				if (tx_asp_params.agc_cfg.Threshold[2] < 40 || tx_asp_params.agc_cfg.Threshold[2] > 100) {
					tx_asp_params.agc_cfg.Threshold[2] = 40;
				}
				printf("Enable speaker Extra AGC_referenceLvl %d, AGC_ref_threshold %d, AGC_AttackTime %d, AGC_ReleaseTime %d\r\n", tx_asp_params.agc_cfg.ReferenceLvl,
					   tx_asp_params.agc_cfg.RefThreshold, tx_asp_params.agc_cfg.AttackTime, tx_asp_params.agc_cfg.ReleaseTime);
				printf("Enable speaker Extra AGC_Ratio1 %d, AGC_Ratio2 %d, AGC_Ratio3 %d\r\n", tx_asp_params.agc_cfg.Ratio[0], tx_asp_params.agc_cfg.Ratio[1],
					   tx_asp_params.agc_cfg.Ratio[2]);
				printf("Enable speaker Extra AGC_Threshold1 %d, AGC_Threshold2 %d, AGC_NoiseGateLvl %d\r\n", tx_asp_params.agc_cfg.Threshold[0],
					   tx_asp_params.agc_cfg.Threshold[1], tx_asp_params.agc_cfg.Threshold[2]);
			} else if (argc >= 9) {
				//CT_ALC = 0, CT_LIMITER = 1
				tx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (tx_asp_params.agc_cfg.AGCMode != CT_ALC && tx_asp_params.agc_cfg.AGCMode != CT_LIMITER) {
					tx_asp_params.agc_cfg.AGCMode = CT_ALC;
				}
				tx_asp_params.agc_cfg.ReferenceLvl = atoi(argv[3]);
				if (tx_asp_params.agc_cfg.ReferenceLvl < 0 || tx_asp_params.agc_cfg.ReferenceLvl > 30) {
					tx_asp_params.agc_cfg.ReferenceLvl = 6;
				}
				tx_asp_params.agc_cfg.RefThreshold = atoi(argv[4]);
				if (tx_asp_params.agc_cfg.RefThreshold < 0 || tx_asp_params.agc_cfg.RefThreshold > 30) {
					tx_asp_params.agc_cfg.RefThreshold = 6;
				}
				tx_asp_params.agc_cfg.AttackTime = atoi(argv[5]);
				if (tx_asp_params.agc_cfg.AttackTime < 0 || tx_asp_params.agc_cfg.AttackTime > 500) {
					tx_asp_params.agc_cfg.AttackTime = 10;
				}
				tx_asp_params.agc_cfg.ReleaseTime = atoi(argv[6]);
				if (tx_asp_params.agc_cfg.ReleaseTime < 0 || tx_asp_params.agc_cfg.ReleaseTime > 500) {
					tx_asp_params.agc_cfg.ReleaseTime = 10;
				}
				tx_asp_params.agc_cfg.Ratio[0] = atoi(argv[7]);
				if (tx_asp_params.agc_cfg.Ratio[0] < 1 || tx_asp_params.agc_cfg.Ratio[0] > 50) {
					tx_asp_params.agc_cfg.Ratio[0] = 50;
				}
				tx_asp_params.agc_cfg.Threshold[0] = atoi(argv[8]);
				if (tx_asp_params.agc_cfg.Threshold[0] < 0 || tx_asp_params.agc_cfg.Threshold[0] > 40) {
					tx_asp_params.agc_cfg.Threshold[0] = 20;
				}
				printf("Enable speaker Extra AGC_referenceLvl %d, AGC_ref_threshold %d, AGC_AttackTime %d, AGC_ReleaseTime %d\r\n", tx_asp_params.agc_cfg.ReferenceLvl,
					   tx_asp_params.agc_cfg.ReferenceLvl, tx_asp_params.agc_cfg.AttackTime, tx_asp_params.agc_cfg.ReleaseTime);
				printf("Enable speaker Extra AGC_Ratio1 %d, AGC_Threshold1 %d\r\n", tx_asp_params.agc_cfg.Ratio[0], tx_asp_params.agc_cfg.Threshold[0]);
			} else if (argc >= 7) {
				//CT_ALC = 0, CT_LIMITER = 1
				tx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (tx_asp_params.agc_cfg.AGCMode != CT_ALC && tx_asp_params.agc_cfg.AGCMode != CT_LIMITER) {
					tx_asp_params.agc_cfg.AGCMode = CT_ALC;
				}
				tx_asp_params.agc_cfg.ReferenceLvl = atoi(argv[3]);
				if (tx_asp_params.agc_cfg.ReferenceLvl < 0 || tx_asp_params.agc_cfg.ReferenceLvl > 30) {
					tx_asp_params.agc_cfg.ReferenceLvl = 6;
				}
				tx_asp_params.agc_cfg.RefThreshold = atoi(argv[4]);
				if (tx_asp_params.agc_cfg.RefThreshold < 0 || tx_asp_params.agc_cfg.RefThreshold > 30) {
					tx_asp_params.agc_cfg.RefThreshold = 6;
				}
				tx_asp_params.agc_cfg.AttackTime = atoi(argv[5]);
				if (tx_asp_params.agc_cfg.AttackTime < 0 || tx_asp_params.agc_cfg.AttackTime > 500) {
					tx_asp_params.agc_cfg.AttackTime = 10;
				}
				tx_asp_params.agc_cfg.ReleaseTime = atoi(argv[6]);
				if (tx_asp_params.agc_cfg.ReleaseTime < 0 || tx_asp_params.agc_cfg.ReleaseTime > 500) {
					tx_asp_params.agc_cfg.ReleaseTime = 10;
				}
				printf("Enable speaker Extra AGC_referenceLvl_SPK %d, AGC_ref_threshold_SPK %d, AGC_AttackTime_SPK %d, AGC_ReleaseTime_SPK %d\r\n",
					   tx_asp_params.agc_cfg.ReferenceLvl, tx_asp_params.agc_cfg.RefThreshold, tx_asp_params.agc_cfg.AttackTime, tx_asp_params.agc_cfg.ReleaseTime);
			}
			tx_asp_params.agc_cfg.AGC_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TXASP_PARAM, (int)&tx_asp_params);
			printf("Enable speaker Extra AGC = %x\r\n", tx_asp_params.agc_cfg.AGC_EN);
		} else {
			tx_asp_params.agc_cfg.AGC_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TXASP_PARAM, (int)&tx_asp_params);
			printf("Disable speaker Extra AGC = %x\r\n", tx_asp_params.agc_cfg.AGC_EN);
		}
	}
#else
	if (!arg) {
		printf("\n\r[AUSPAGC] Set up AGC module: AUSPAGC=[AGC_enable],[AGC_mode],[AGC_targetLevelDbfs],[AGC_compression_gain],[AGC_limiter_enable]\n");

		printf("  \r     [AGC_enable]=0 or 1 or 2\n");
		printf("  \r     [AGC_mode]=0~3,0 kAgcModeUnchanged, 1 kAgcModeAdaptiveAnalog, 2 kAgcModeAdaptiveDigital, 3 kAgcModeFixedDigital\n");
		printf("  \r     [AGC_targetLevelDbfs], the target target level is -[AGC_targetLevelDbfs], means the ouput will be controled under than 0dBFS\n");
		printf("  \r     [AGC_compression_gain]=AGC compression gain in dB\n");
		printf("  \r     [AGC_limiter_enable]=0 or 1, enable agc limiter or not\n");
		printf("  \r     Set extra AGC with gain 18dB, enable limiter by AUASPGC=1,2,0,18,1\n");
		printf("  \r     Disable extar AGC by AUSPAGC=0\n");
		return;
	}

	argc = parse_param(arg, argv);

	if (argc) {
		if (atoi(argv[1]) == 1) {
			if (argc >= 6) {
				tx_asp_params.agc_cfg.AGCMode = atoi(argv[2]);
				if (tx_asp_params.agc_cfg.AGCMode < 0 || tx_asp_params.agc_cfg.AGCMode > 3) {
					tx_asp_params.agc_cfg.AGCMode = 2;
				}
				tx_asp_params.agc_cfg.TargetLevelDbfs = atoi(argv[3]);
				if (tx_asp_params.agc_cfg.TargetLevelDbfs < 0 || tx_asp_params.agc_cfg.TargetLevelDbfs > 31) {
					tx_asp_params.agc_cfg.TargetLevelDbfs = 0;
				}
				tx_asp_params.agc_cfg.CompressionGaindB = atoi(argv[4]);
				if (tx_asp_params.agc_cfg.CompressionGaindB < 0 || tx_asp_params.agc_cfg.CompressionGaindB > 90) {
					tx_asp_params.agc_cfg.CompressionGaindB = 10;
				}
				if (atoi(argv[5])) {
					tx_asp_params.agc_cfg.LimiterEnable = 1;
				} else {
					tx_asp_params.agc_cfg.LimiterEnable = 0;
				}
			}

			tx_asp_params.agc_cfg.AGC_EN = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TXASP_PARAM, (int)&tx_asp_params);
			printf("Enable speaker Extra AGC = %d, %d, %d, %d, %d\r\n", tx_asp_params.agc_cfg.AGC_EN, tx_asp_params.agc_cfg.AGCMode,
				   tx_asp_params.agc_cfg.TargetLevelDbfs, tx_asp_params.agc_cfg.CompressionGaindB, tx_asp_params.agc_cfg.LimiterEnable);
		} else {
			tx_asp_params.agc_cfg.AGC_EN = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TXASP_PARAM, (int)&tx_asp_params);
			printf("Disable speaker Extra AGC = %x\r\n", tx_asp_params.agc_cfg.AGC_EN);
		}
	}
#endif
}

//Set the speaker EQ
void fAUSPEQ(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	uint8_t EQ_num = 0;
	if (!arg) {
		printf("\n\r[AUSPEQ] Speaker EQ Usage: AUSPEQ=[eq num],[register h0],[register a1],[register a2],[register b1],[register b2]\n");
		printf("  \r[AUSPEQ] Speaker EQ DISABLE Usage: AUSPEQ=[eq num]\n");
		printf("  \r     Enter the register coefficients obtained from biquad calculator\n");

		printf("  \r     [eq num]=0~4\n");
		printf("  \r     [register]=<num in hex> \n");
		printf("  \r     OPEN EQ by AUSPEQ=0,0x01,0x02,0x03,x04,0x05\n");
		printf("  \r     CLOSE EQ by AUSPEQ=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc == 7) {
		EQ_num = atoi(argv[1]);
		if (EQ_num >= 0 && EQ_num <= 4) {
			printf("ENABLE EQ: %d \r\n", EQ_num);
			audio_save_params.spk_l_eq[EQ_num].eq_enable = 1;
			audio_save_params.spk_l_eq[EQ_num].eq_h0 = (int)strtol(argv[2], NULL, 16);
			audio_save_params.spk_l_eq[EQ_num].eq_b1 = (int)strtol(argv[3], NULL, 16);
			audio_save_params.spk_l_eq[EQ_num].eq_b2 = (int)strtol(argv[4], NULL, 16);
			audio_save_params.spk_l_eq[EQ_num].eq_a1 = (int)strtol(argv[5], NULL, 16);
			audio_save_params.spk_l_eq[EQ_num].eq_a2 = (int)strtol(argv[6], NULL, 16);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else {
			printf(" Set Wrong EQ: %d Not between 0~4\r\n", EQ_num);
		}
	} else if (argc == 2) {
		EQ_num = atoi(argv[1]);
		if (EQ_num >= 0 && EQ_num <= 4) {
			printf("DISABLE EQ: %d \r\n", EQ_num);
			audio_save_params.spk_l_eq[EQ_num].eq_enable = 0;
			audio_save_params.spk_l_eq[EQ_num].eq_h0 = 0;
			audio_save_params.spk_l_eq[EQ_num].eq_b1 = 0;
			audio_save_params.spk_l_eq[EQ_num].eq_b2 = 0;
			audio_save_params.spk_l_eq[EQ_num].eq_a1 = 0;
			audio_save_params.spk_l_eq[EQ_num].eq_a2 = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else {
			printf(" Set Wrong EQ: %d Not between 0~4\r\n", EQ_num);
		}
	} else {
		printf(" argument number = %d \r\n", argc - 1);
		printf(" MISS some arguments AUSPEQ=[eq num],[register h0],[register a1],[register a2],[register b1],[register b2]\r\n");
	}
}

//Reset the SPEAKER EQ without audio reset
void fAUSPKEQR(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SPKEQ_RESET, 0);
}

//Set the speaker DAC gain
void fAUDAC(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	if (!arg) {
		printf("\n\r[AUDAC] Set up DAC gain: AUDAC=[DAC_gain]\n");

		printf("  \r     [DAC_gain]=0x00~0x7F\n");
		printf("  \r     DAC_gain need in hex= 0xAF: 0dB, 0x87: -15dB, 0x00: -65dB, 0.375/step\n");
		printf("  \r     Set DAC_gain -15dB by AUADC=0x87\n");
		return;
	}

	argc = parse_param(arg, argv);

	if (argc) {
		//ADC_gain = atohex(argv[1]);
		DAC_gain = (int)strtol(argv[1], NULL, 16);
		printf("get DAC gain = %d, 0x%x\r\n", DAC_gain, DAC_gain);
		if (DAC_gain >= 0x00 && DAC_gain <= 0xAF) {
			printf("get DAC gain = %d, 0x%x\r\n", DAC_gain, DAC_gain);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);
		} else {
			DAC_gain = 0xAF;
			printf("invalid DAC set default gain = %d, 0x%x\r\n", DAC_gain, DAC_gain);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);
		}
		audio_save_params.DAC_gain = DAC_gain;
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
	}
}

//Set speaker mute
void fAUSPM(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	int spk_mute = 0;
	if (!arg) {
		printf("\n\r[AUSPM] Mute SPEAKER or not: AUSPM=[enable_mute]\n");

		printf("  \r     [enable_mute]=0 or 1\n");
		printf("  \r     MUTE SPEAKER by AUSPM=1\n");
		printf("  \r     UNMUTE SPEAKER by AUSPM=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		spk_mute = atoi(argv[1]);
		if (spk_mute == 0) {
			printf("Disable Speaker Mute\r\n");
			audio_save_params.DAC_mute = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_SPK_ENABLE, 1);
		} else {
			printf("Enable Speaker Mute\r\n");
			audio_save_params.DAC_mute = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_SPK_ENABLE, 0);
		}
	}
}

//Set speaker tx mode
void fAUTXMODE(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	array_t array;
	int ret = 0;

	if (!arg) {
		printf("\n\r[AUTXMODE] Mute SPEAKER or not: AUTXMODE=[tx_mode],[audio_tone(Hz)],[targetlevel(dB)]\n");

		printf("  \r     [tx_mode]=playmusic/playback/playtone/playmusic\n");
		printf("  \r     [audio_tone(Hz)]=audio tone for play tone mode, only for play tone mode. Default is 1k\n");
		printf("  \r     playmusic mode by AUTXMODE=playmusic\n");
		printf("  \r     playback mode by AUTXMODE=playback\n");
		printf("  \r     playmusic mode (the array in the ram) by AUTXMODE=playmusic\n");
		printf("  \r     playspeech mode (play data in SD card, file name: playback.bin) by AUTXMODE=playspeech\n");
		printf("  \r     playramdisk mode (play data in ram disk, file name: playback.bin) by AUTXMODE=playramdisk\n");
		printf("  \r     playtone mode 2K tone by AUTXMODE=playtone,2000\n");
		printf("  \r     play a signal tone(default 1k) by AUTXMODE=playtone\n");


		printf("  \r     no mode by AUTXMODE=noplay\n");
		return;
	}
	//reset array data
	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);

		if (strcmp(argv[1], "playmusic") == 0) {
			if (tx_mode != TXPLAYMUSIC) {
				if (playing_sample_rate <= 16000) {
					printf("Set playmusic mode\r\n");
					mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
					mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
					//printf("mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0)\r\n");
					mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);
					//printf("mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0)\r\n");

					//set the array audio data
					if (playing_sample_rate == 16000) {
						array.data_addr = (uint32_t) music_sr16k;
						array.data_len = (uint32_t) music_sr16k_pcm_len;
						pcm16k_array_params.u.a.samplerate = 16000;
					} else if (playing_sample_rate == 8000) {
						array.data_addr = (uint32_t) music_sr8k;
						array.data_len = (uint32_t) music_sr8k_pcm_len;
						pcm16k_array_params.u.a.samplerate = 8000;
					}
					mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_SET_PARAMS, (int)&pcm16k_array_params);
					mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_SET_ARRAY, (int)&array);
					mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_RECOUNT_PERIOD, 0);
					vTaskDelay(10);
					mimo_resume(mimo_aarray_audio);
					//printf("mimo_resume(mimo_aarray_audio)\r\n");
					mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT3);
					//printf("mimo_pause(mimo_aarray_audio, MM_OUTPUT1)\r\n");
					mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, 0x00);
					mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
					mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 1);	// streamming on
					vTaskDelay(40);
					mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);

					tx_mode = TXPLAYMUSIC;
				} else {
					printf("Playmusic mode is not supported sample rate: %d \r\n", playing_sample_rate);
				}
			} else {
				printf("Now is in playmusic mode\r\n");
			}

		} else if (strcmp(argv[1], "playspeech") == 0) {
			if (tx_mode != TXPLAYSPEECH) {
				mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
				//printf("mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0)\r\n");
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);
				//printf("mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0)\r\n");

				vTaskDelay(10);
				mimo_resume(mimo_aarray_audio);
				//printf("mimo_resume(mimo_aarray_audio)\r\n");
				mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT2);

				//printf("mimo_pause(mimo_aarray_audio, MM_OUTPUT1)\r\n");
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, 0x00);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
				ret = mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SET_PLAY_MODE, PLAY_SD_DATA);
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 1);	// streamming on
				vTaskDelay(40);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);
				if (ret == 0) {
					tx_mode = TXPLAYSPEECH;
				} else {
					tx_mode = TXPLAYTONE;
				}
			} else {
				printf("Now is in playspeech mode\r\n");
			}
		} else if (strcmp(argv[1], "playramdisk") == 0) {
			if (tx_mode != TXPLAYRAMDISK) {
				mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
				//printf("mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0)\r\n");
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);
				//printf("mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0)\r\n");

				vTaskDelay(10);
				mimo_resume(mimo_aarray_audio);
				//printf("mimo_resume(mimo_aarray_audio)\r\n");
				mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT2);

				//printf("mimo_pause(mimo_aarray_audio, MM_OUTPUT1)\r\n");
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, 0x00);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
				ret = mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SET_PLAY_MODE, PLAY_RAMDISK_DATA);
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 1);	// streamming on
				vTaskDelay(40);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);
				if (ret == 0) {
					tx_mode = TXPLAYRAMDISK;
				} else {
					tx_mode = TXPLAYTONE;
				}
			} else {
				printf("Now is in playramdisk mode\r\n");
			}
		} else if (strcmp(argv[1], "playback") == 0) {
			if (tx_mode != TXPLAYBACK) {
				printf("Change playback mode\r\n");
				mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
				//printf("mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0)\r\n");
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);
				//printf("mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0)\r\n");
				vTaskDelay(10);
				mimo_resume(mimo_aarray_audio);
				//printf("mimo_resume(mimo_aarray_audio)\r\n");
				mimo_pause(mimo_aarray_audio, MM_OUTPUT0 | MM_OUTPUT2 | MM_OUTPUT3);
				//printf("mimo_pause(mimo_aarray_audio, MM_OUTPUT0 | MM_OUTPUT2)\r\n");
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, 0x00);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
				vTaskDelay(40);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);

				tx_mode = TXPLAYBACK;
			} else {
				printf("Now is in playback mode\r\n");
			}
		} else if (strcmp(argv[1], "playtone") == 0) {
			printf("Set playtone mode\r\n");
			mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
			mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
			//printf("mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0)\r\n");
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);
			//printf("mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0)\r\n");

			if (argv[2]) {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SET_AUDIOTONE, atoi(argv[2]));
			} else {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SET_AUDIOTONE, 1000);
			}

			if (argv[3]) {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_TARGET_DB, atoi(argv[3]));
			} else {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_TARGET_DB, 0);
			}

			vTaskDelay(10);
			mimo_resume(mimo_aarray_audio);
			//printf("mimo_resume(mimo_aarray_audio)\r\n");
			mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT2);
			//printf("mimo_pause(mimo_aarray_audio, MM_OUTPUT1)\r\n");
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, 0x00);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
			mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SET_PLAY_MODE, PLAY_TONE);
			mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 1);	// streamming on
			vTaskDelay(40);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);

			tx_mode = TXPLAYTONE;
		} else if (strcmp(argv[1], "playsweep") == 0) {
			printf("Set playtone mode\r\n");
			mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
			mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
			//printf("mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0)\r\n");
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);
			//printf("mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0)\r\n");

			if (argv[2]) {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SWEEP_TONE, atoi(argv[2]));
			} else {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SWEEP_TONE, 350);
			}

			if (argv[3]) {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_TARGET_DB, atoi(argv[3]));
			} else {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_TARGET_DB, 0);
			}

			vTaskDelay(10);
			mimo_resume(mimo_aarray_audio);
			//printf("mimo_resume(mimo_aarray_audio)\r\n");
			mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT2);
			//printf("mimo_pause(mimo_aarray_audio, MM_OUTPUT1)\r\n");
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, 0x00);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
			mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SET_PLAY_MODE, PLAY_TONE);
			mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 1);	// streamming on
			vTaskDelay(40);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);

			tx_mode = TXPLAYTONE;
		} else if (strcmp(argv[1], "noplay") == 0 || strcmp(argv[1], "playstream") == 0) {
			if (tx_mode != TXNOPLAY) {
				printf("Set noplay or playstream mode\r\n");
				mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
				//printf("mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0)\r\n");
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_SPK_ENABLE, 0);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);
				//printf("mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0)\r\n");
				vTaskDelay(10);
				mimo_resume(mimo_aarray_audio);
				//printf("mimo_resume(mimo_aarray_audio)\r\n");
				mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT2 | MM_OUTPUT3); //enable audio playtone
				//printf("mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT2)\r\n");
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, 0x00);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
				vTaskDelay(40);
				mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);

				tx_mode = TXNOPLAY;
			} else {
				printf("Now is in noplay or playstream mode\r\n");
			}
		} else {
			if (tx_mode == TXPLAYMUSIC) {
				printf("Unknown mode keep playmusic mode\r\n");
			} else if (tx_mode == TXPLAYBACK) {
				printf("Unknown mode keep playback mode\r\n");
			} else if (tx_mode == TXPLAYTONE) {
				printf("Unknown mode keep playtone or playsweep mode\r\n");
			} else if (tx_mode == TXPLAYRAMDISK) {
				printf("Unknown mode keep playramdisk\r\n");
			} else {
				printf("Unknown mode keep noplay mode\r\n");
			}
		}
	}
}

//Set tone generator db sweep
void fTONEDBSW(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	if (!arg) {
		printf("\n\r[TONEDBSW] TONEDBSW : TONEDBSW=[sweep_DB_interval(ms)]\n");

		printf("  \r     [sweep_DB_interval(ms)]=playmusic/playback/playtone/playmusic\n");
		printf("  \r     enable tone DB sweep with interval 200 ms by TONEDBSW=200\n");
		printf("  \r     disable tone DB sweep by TONEDBSW=0\n");
		return;
	}
	//reset array data
	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);

		if (argv[1]) {
			mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SWEEP_DB, atoi(argv[1]));
			mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_PCM_SWEEP, atoi(argv[1]));
		}

	}
}

//Set speaker amplifier pin
extern void gpio_deinit(gpio_t *obj);
void fAUAMPIN(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	int res = 0;
	int a_amp_pin;


	if (!arg) {
		printf("\n\r[AUAMPIN] set amp pin on or not: AUAMPIN=[pin_name],[on/off]\n");

		printf("  \r     [pin_name]=pin name\n");
		printf("  \r     [on/off]=1/0\n");
		printf("  \r     Set PF_15 on by AUAMPIN=PF_15,1\n");
		printf("  \r     Set PF_15 off by AUAMPIN=PF_15,0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		res = pinconvert(argv[1], &a_amp_pin);
		if (res) {
			// Init LED control pin
			if (pin_initialed == 0) {
				gpio_init(&gpio_amp, a_amp_pin);
				pin_initialed = 1;
				set_pin = a_amp_pin;
			}
			if (set_pin == a_amp_pin) {
				gpio_dir(&gpio_amp, PIN_OUTPUT);        // Direction: Output
				gpio_mode(&gpio_amp, PullNone);         // No pull
				gpio_write(&gpio_amp, !!atoi(argv[2]));
				if (!!atoi(argv[2])) {
					printf("to ON \r\n");
				} else {
					printf("to OFF \r\n");
				}
			} else {
				if (!!atoi(argv[2])) {
					printf("to ON \r\n");
				} else {
					printf("to OFF \r\n");
				}
				printf("Not the same. Deinit previous pin\r\n");
				gpio_deinit(&gpio_amp);
				gpio_init(&gpio_amp, a_amp_pin);
				gpio_dir(&gpio_amp, PIN_OUTPUT);        // Direction: Output
				gpio_mode(&gpio_amp, PullNone);         // No pull
				gpio_write(&gpio_amp, !!atoi(argv[2]));
				pin_initialed = 1;
				set_pin = a_amp_pin;
			}
			//printf("\r\na_amp_pin = %d\r\n", a_amp_pin);

		} else {
			printf("Unkown pin\r\n");
		}
	}
}

//********************//
//Audio setting function
//********************//
//Set audio sample rate
void fAUSR(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUSR] Set up SAMPLE RATE: AUSR=[sample_rate]\n");

		printf("  \r     [sample_rate]=8000,16000,32000,44100,48000,88200,96000\n");
		printf("  \r     sample rate = 8000, 16000\n");
		printf("  \r     Set SAMPLE RATE 8000 by AUSR=8000\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		the_sample_rate = atoi(argv[1]);
		if (the_sample_rate == 96000) {
			printf("Set sample rate 96K\r\n");
			audio_save_params.sample_rate = ASR_96KHZ;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else if (the_sample_rate == 88200) {
			printf("Set sample rate 88.2K\r\n");
			audio_save_params.sample_rate = ASR_88p2KHZ;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else if (the_sample_rate == 48000) {
			printf("Set sample rate 48K\r\n");
			audio_save_params.sample_rate = ASR_48KHZ;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else if (the_sample_rate == 44100) {
			printf("Set sample rate 44.1K\r\n");
			audio_save_params.sample_rate = ASR_44p1KHZ;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else if (the_sample_rate == 32000) {
			printf("Set sample rate 32K\r\n");
			audio_save_params.sample_rate = ASR_32KHZ;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else if (the_sample_rate == 16000) {
			printf("Set sample rate 16K\r\n");
			audio_save_params.sample_rate = ASR_16KHZ;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else if (the_sample_rate == 8000) {
			printf("Set sample rate 8K\r\n");
			audio_save_params.sample_rate = ASR_8KHZ;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
		} else {
			printf("Set sample rate 8K\r\n");
			audio_save_params.sample_rate = ASR_8KHZ;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
			the_sample_rate = 8000;
		}
	}
}

//Set audio TRX enable
void fAUTRX(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AUTRX] Enable TRX or not: AUTRX=[enable]\n");

		printf("  \r     [enable]=0 or 1\n");
		printf("  \r     OPEN TRX by AUTRX=1\n");
		printf("  \r     CLOSE TRX by AUTRX=0\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		if (atoi(argv[1]) == 0) {
			if (tx_mode == TXPLAYMUSIC) {
				mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
			} else if (tx_mode == TXPLAYTONE) {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
			}
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);

		} else {
			if (tx_mode == TXPLAYMUSIC) {
				mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 1);	// streamming on
			} else if (tx_mode == TXPLAYTONE) {
				mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 1);	// streamming on
			}
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
		}
	}
}

void fAURES(void *arg)
{
	//int argc = 0;
	//char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[AURES] Reset the audio for setting the previous audio configuration: AURES=1\n");

		printf("  \r     Reset Audio by AURES=1\n");
		return;
	}
	//argc = parse_param(arg, argv);
	printf("Reset audio flow \r\n");
	reset_flag = 1;

	//choose new array data
	array_t array;
	if (the_sample_rate == 16000) {
		if (tx_mode == TXPLAYMUSIC) {//play music mode
			array.data_addr = (uint32_t) music_sr16k;
			array.data_len = (uint32_t) music_sr16k_pcm_len;
		}
		pcm16k_array_params.u.a.samplerate = 16000;
	} else if (the_sample_rate == 8000) {
		if (tx_mode == TXPLAYMUSIC) {//play music mode
			array.data_addr = (uint32_t) music_sr8k;
			array.data_len = (uint32_t) music_sr8k_pcm_len;
		}
		pcm16k_array_params.u.a.samplerate = 8000;
	}
	//reset array data
	mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
	mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_SET_PARAMS, (int)&pcm16k_array_params);
	mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_SET_ARRAY, (int)&array);
	mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_RECOUNT_PERIOD, 0);

	//reset audio
	mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 0);	// streamming off
	mm_module_ctrl(pcm_tone_ctx, CMD_TONE_SET_SAMPLERATE, the_sample_rate);
	mm_module_ctrl(pcm_tone_ctx, CMD_TONE_RECOUNT_PERIOD, 0);
	audio_save_params.use_mic_type = set_mic_type;
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_RESET, 0);

	//reset p2p audio sample rate
	mm_module_ctrl(p2p_audio_ctx, CMD_P2P_AUDIO_SAMPLERATE, the_sample_rate);

	//restart if play music mode
	if (tx_mode == TXPLAYMUSIC) {
		if (the_sample_rate <= 16000) {
			mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 1);	// streamming on
		} else {
			printf("playmusic ot playtone mode are not support in sample rate %d. Set to no play mode\r\n", the_sample_rate);
			mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0);	// streamming off
			//printf("mm_module_ctrl(array_pcm_ctx, CMD_ARRAY_STREAMING, 0)\r\n");
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_SPK_ENABLE, 0);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0);
			//printf("mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 0)\r\n");
			vTaskDelay(10);
			mimo_resume(mimo_aarray_audio);
			//printf("mimo_resume(mimo_aarray_audio)\r\n");
			mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT2); //enable audio playtone
			//printf("mimo_pause(mimo_aarray_audio, MM_OUTPUT1 | MM_OUTPUT2)\r\n");
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, 0x00);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_TRX, 1);
			vTaskDelay(40);
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);

			tx_mode = TXNOPLAY;
		}
	} else if (tx_mode == TXPLAYTONE || tx_mode == TXPLAYSPEECH) { //if play tone mode
		mm_module_ctrl(pcm_tone_ctx, CMD_TONE_STREAMING, 1);	// streamming off
	}
	playing_sample_rate = the_sample_rate;

	if (tx_mode != TXPLAYBACK) {
		mm_module_ctrl(afft_test_ctx, CMD_AFFT_SAMPLERATE, playing_sample_rate);
	}
	//reset the audio ADC and DAC gain
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_ADC_GAIN, ADC_gain);
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_DAC_GAIN, DAC_gain);
}

//Set show the mic audio fft result in function
void fAUFFTS(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	if (!arg) {
		printf("\n\r[AUFFTS] Enable Print FFT result (playback mode will not print): AUFFTS=[FFT_EN]\n");

		printf("  \r     [FFT_EN]=0 or 1\n");
		printf("  \r     Enable Print FFT result (playback mode will not print) by AUFFTS=1\n");
		return;
	}

	argc = parse_param(arg, argv);

	if (argc) {
		if (atoi(argv[1])) {
			printf("Start FFT Result Print\r\n");
			audio_fft_show = 1;
			mm_module_ctrl(afft_test_ctx, CMD_AFFT_SHOWN, 1);
		} else {
			printf("Stop FFT Result Print\r\n");
			audio_fft_show = 0;
			mm_module_ctrl(afft_test_ctx, CMD_AFFT_SHOWN, 0);
		}
	}
}

//********************//
//Audio record function
//********************//
//Set audio save file name
void fAUFILE(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	if (!arg) {
		printf("\n\r[AUFILE] Set up record file name: AUFILE=[filename]\n");

		printf("  \r     [filename] length needs to smaller than 20\n");
		printf("  \r     Set FILE NAME by AUFILE=AUDIO_TEST\n");
		return;
	}

	argc = parse_param(arg, argv);

	if (argc) {
		memset(file_name, 0, 20);
		strncpy(file_name, argv[1], 20);
	}
}

//record file time
void fAUREC(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	int rtime = 0;
	int p_min = 0;
	int p_sec = 0;
	int p_msec = 0;

	if (!arg) {
		printf("\n\r[fAUREC] SET AUDIO RECORD TIME: AUREC=[record_time],[RECORD_TYPE1],[RECORD_TYPE2],[RECORD_TYPE3],[RECORD_TYPE4]\n");

		printf("  \r     %d <= [record_time] <= %d in ms\n", MIN_RECORD_TIME, MAX_RECORD_TIME);
		printf("  \r     [RECORD_TYPE] = RX, TX, ASP\n");
		printf("  \r     RX: record mic RX data before ASP, TX: speaker data, ASP: mic data after ASP, TXASP: speaker data after ASP \n");
		printf("  \r     if [RECORD_TYPE] not set, it will record mic RX data\n");
		printf("  \r     record 1 min data fAUREC=600000\n");
		printf("  \r     record 1 min data for RX, TX, ASP, TXASP fAUREC=600000,RX,TX,ASP,TXASP\n");
		return;
	}
	argc = parse_param(arg, argv);
	rtime = atoi(argv[1]);
	if (record_state == 0 && !((audiocopy_status & SD_SAVE_START) || (audiocopy_status & TFTP_UPLOAD_START))) {
		if (argc) {
			record_type = 0;
			if (argc > 2) {
				for (int i = 2; i < argc; i++) {
					if ((strncmp(argv[i], "TXASP", 5) == 0) && (strlen(argv[i]) == 5)) {
						printf("Record speaker TXASP DATA\r\n");
						record_type |= RECORD_TXASP_DATA;
					} else if ((strncmp(argv[i], "RX", 2) == 0) && (strlen(argv[i]) == 2)) {
						printf("Record mic RX DATA\r\n");
						record_type |= RECORD_RX_DATA;
					} else if ((strncmp(argv[i], "TX", 2) == 0) && (strlen(argv[i]) == 2)) {
						printf("Record speaker TX DATA\r\n");
						record_type |= RECORD_TX_DATA;
					} else if ((strncmp(argv[i], "ASP", 3) == 0) && (strlen(argv[i]) == 3)) {
						printf("Record mic ASP DATA\r\n");
						record_type |= RECORD_ASP_DATA;
					}
				}
			}

			if (record_type == 0) {
				printf("Default Record mic RX DATA\r\n");
				record_type |= RECORD_RX_DATA;
			}
			if (rtime > MAX_RECORD_TIME) {
				printf("The record time > MAX_RECORD_TIME(%d), set to max record time \r\n", MAX_RECORD_TIME);
				rtime = MAX_RECORD_TIME;
			} else if (rtime < MIN_RECORD_TIME) {
				printf("The record time > MIN_RECORD_TIME(%d), set to min record time \r\n", MIN_RECORD_TIME);
				rtime = MIN_RECORD_TIME;
			} else {
				printf("Set the record time (%d) \r\n", rtime);
			}
			p_min = rtime / 60000;
			p_sec = (rtime - p_min * 60000) / 1000;
			p_msec = rtime - p_min * 60000 - p_sec * 1000;

			printf("Record %d m %d s %d ms length data\r\n", p_min, p_sec, p_msec);
			//count the number of frame need to save
			frame_count = rtime / (FRAME_LEN / (the_sample_rate / 1000));
			record_frame_count = frame_count;

			printf("Sample rate: %dkHZ, frame length: %d\r\n", the_sample_rate / 1000, frame_count);
			record_state = 1;
		}
	} else {
		printf("previous record not end\r\n");
	}
}

void fAUCOPY(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[fAUCOPY] download to sd or not: fAUCOPY=[mode],[tftp_ip],[tftp_port]\n");

		printf("  \r     [mode]=NOCOPY, SD, TFTP\n");
		printf("  \r     NOCOPY: just save in RAM\n");
		printf("  \r     SD: download data to SD\n");
		printf("  \r     TFTP: upload data to TFTP server\n");
		printf("  \r     [tftp_ip],[tftp_port]: set TFTP server IP and port\n");
		printf("  \r     if not set it will use the defualt value\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		if (strcmp(argv[1], "NOCOPY") == 0) {
			printf("NOCOPY mode, just save in RAM\r\n");
			audiocopy_status &= ~SD_SAVE_EN;
			audiocopy_status &= ~SD_SAVE_START;
			audiocopy_status &= ~TFTP_UPLOAD_EN;
			audiocopy_status &= ~TFTP_UPLOAD_START;
		} else if (strcmp(argv[1], "SD") == 0) {
			printf("SD dowmload mode, downlaod to SD every record\r\n");
			audiocopy_status |= SD_SAVE_EN;
			audiocopy_status &= ~SD_SAVE_START;
			audiocopy_status &= ~TFTP_UPLOAD_EN;
			audiocopy_status &= ~TFTP_UPLOAD_START;
		} else if (strcmp(argv[1], "TFTP") == 0) {
			printf("TFTP upload mode, upload to TFTP server every record\r\n");
			audiocopy_status &= ~SD_SAVE_EN;
			audiocopy_status &= ~SD_SAVE_START;
			audiocopy_status |= TFTP_UPLOAD_EN;
			audiocopy_status &= ~TFTP_UPLOAD_START;
			if (argc >= 4) {
				snprintf(audio_tftp_ip, 16, "%s", argv[2]);
				audio_tftp_port = strtoul(argv[3], NULL, 10);
			}
			printf("Upload to TFTP server %s:%d\r\n", audio_tftp_ip, audio_tftp_port);
		} else {
			printf("Ubkown mode, just save in RAM\r\n");
			audiocopy_status &= ~SD_SAVE_EN;
			audiocopy_status &= ~SD_SAVE_START;
			audiocopy_status &= ~TFTP_UPLOAD_EN;
			audiocopy_status &= ~TFTP_UPLOAD_START;
		}
	}
}

void fAUMSGS(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[fAUMSGS] set the audio message show level: fAUCOPY=[MSGLevel]\n");

		printf("  \r     [MSGLevel]=0,1,2,3\n");
		printf("  \r     0: no message\n");
		printf("  \r     1: inf, warn and err\n");
		printf("  \r     2: warn, err\n");
		printf("  \r     3: err\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		int msglevel = atoi(argv[1]);
		if (msglevel < 0 || msglevel > 3) {
			msglevel = 2;
		}
		printf("set the msglevel to %d\r\n", msglevel);
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_MESSAGE_LEVEL, msglevel);

	}
}

void fAUINFO(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	argc = parse_param(arg, argv);
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_GET_PARAMS, (int)&audio_save_params);
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_GET_TXASP_PARAM, (int)&tx_asp_params);
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_GET_RXASP_PARAM, (int)&rx_asp_params);
	char *audio_json = Get_Audio_CJSON(audio_save_params, rx_asp_params, tx_asp_params);
	printf("%s\r\n", audio_json);
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_PRINT_ASP_INFO, (int)&rx_asp_params);
	printf("%s\r\n", libVQE_get_version());
	free(audio_json);
}

void fAUDRST(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	argc = parse_param(arg, argv);
	extern void fatfs_ram_reset(void);
	fatfs_ram_reset();
}

// for sync enable testing, will set parameter fcs_avsync_en to enable
void fAUSYNCEN(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[fAUSYNCEN] set the fcs_avsync_en testing to enable: AUSYNCEN=[enable]\n");

		printf("  \r     [enable]=0,1\n");
		printf("  \r     0: disable avsync process\n");
		printf("  \r     1: enable avsync process\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		if (atoi(argv[1]) == 0) {
			audio_save_params.avsync_en = 0;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
			printf("Set the AV Sync testing to off\r\n");
		} else {
			audio_save_params.avsync_en = 1;
			mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_save_params);
			printf("Set the AV Sync testing to on\r\n");
		}
	}
}

// for settiing the expecting ms (video) timestamp is before current time
// Use it with AUSYNCEN
uint32_t avsynctime = 0;
void fAUSYNCTS(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		printf("\n\r[fAUSYNCTS] set the fcs_avsync_en testing to enable: AUSYNCTS=[back_timestamp]\n");

		printf("  \r     [back_timestamp]\n");
		printf("  \r     AUSYNCTS=30: set avsync time to current_time - 30\n");
		return;
	}

	argc = parse_param(arg, argv);
	if (argc) {
		printf("argc = %d\r\n", argc);
		int back_timestamp = atoi(argv[1]);
		uint32_t current_time = mm_read_mediatime_ms();
		avsynctime = current_time - back_timestamp;
		mm_module_ctrl(audio_save_ctx, CMD_AUDIO_SET_AVSYNC_TIMESTAMP, avsynctime);
		printf("Set the AV Sync time to %d\r\n", avsynctime);
	}
}

void fAUGETSYNC(void *arg)
{
	int fcs_audio_data_starttime = 0;
	int fcs_audio_dummy_starttime = 0;
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_GET_FIRST_DATA_TS, (int)&fcs_audio_data_starttime);
	mm_module_ctrl(audio_save_ctx, CMD_AUDIO_GET_FIRST_DUMMY_TS, (int)&fcs_audio_dummy_starttime);
	printf("v: %d, ad: %d, a: %d\r\n", avsynctime, fcs_audio_dummy_starttime, fcs_audio_data_starttime);
}

log_item_t at_audio_save_items[ ] = {
	//For Audio mic
	{"AUMMODE",     fAUMMODE,   {NULL, NULL}}, //set the mic mode
	{"AUMG",        fAUMG,      {NULL, NULL}}, //adjust analog mic gain
	{"AUMB",        fAUMB,      {NULL, NULL}}, //adjust analog mic bias
	{"AUMLG",       fAUMLG,     {NULL, NULL}}, //adjust left digital mic gain
	{"AUMRG",       fAUMRG,     {NULL, NULL}}, //adjust right digital mic gain
	{"AUADC",       fAUADC,     {NULL, NULL}}, //adjust the ADC gain for mic
	{"AUHPF",       fAUHPF,     {NULL, NULL}}, //adjust the mic HPF (this HPF is before EQ)
	{"AUMLEQ",      fAUMLEQ,    {NULL, NULL}}, //adjust the left mic (analog mic) EQ
	{"AUMREQ",      fAUMREQ,    {NULL, NULL}}, //adjust the right mic EQ
	{"AUMICEQR",    fAUMICEQR,  {NULL, NULL}}, //reset mic eq parameters without audio reset
	{"AUAEC",       fAUAEC,     {NULL, NULL}}, //open/close the SW AEC
	{"AUAECRUN",    fAUAECRUN,  {NULL, NULL}}, //run/stop the SW AEC process if the AEC is initialed
	{"AUNS",        fAUNS,      {NULL, NULL}}, //open and adjust the SW NS for mic
	{"AUAGC",       fAUAGC,     {NULL, NULL}}, //open/close the SW AGC
	{"AUMICM",      fAUMICM,    {NULL, NULL}}, //enable/disable to mute the mic
	//For Audio speaker
	{"AUSPNS",      fAUSPNS,    {NULL, NULL}}, //open and adjust the SW NS for speaker
	{"AUSPAGC",     fAUSPAGC,   {NULL, NULL}}, //open and adjust the SW NS for speaker
	{"AUSPEQ",      fAUSPEQ,    {NULL, NULL}}, //adjust the speaker EQ
	{"AUSPKEQR",    fAUSPKEQR,  {NULL, NULL}}, //reset speaker eq parameters without audio reset
	{"AUDAC",       fAUDAC,     {NULL, NULL}}, //adjust the DAC dain for speaker
	{"AUSPM",       fAUSPM,     {NULL, NULL}}, //enable/disable to mute the speaker
	{"AUTXMODE",    fAUTXMODE,  {NULL, NULL}}, //adjust speaker output to playtone/playback/noplay
	{"TONEDBSW",    fTONEDBSW,  {NULL, NULL}}, //enable tone DB sweep with interval
	{"AUAMPIN",     fAUAMPIN,   {NULL, NULL}}, //select the speaker amplifier pin
	//For Audio TRX
	{"AUSR",        fAUSR,      {NULL, NULL}}, //set the audio sample rate for both TRX
	{"AUTRX",       fAUTRX,     {NULL, NULL}}, //enable/disable the audio TRX
	{"AURES",       fAURES,     {NULL, NULL}}, //reset the audio TRX to enable the previous setting
	{"AUFFTS",      fAUFFTS,    {NULL, NULL}}, //set shown the audio fft result
	//For Audio Recording
	{"AUFILE",      fAUFILE,    {NULL, NULL}}, //set the record file name (need less than 23)
	{"AUCOPY",      fAUCOPY,    {NULL, NULL}}, //set the how user copy the data by SD or TFTP every record
	{"AUREC",       fAUREC,     {NULL, NULL}}, //record file for the setting time
	//For Audio Message
	{"AUMSGS",      fAUMSGS,    {NULL, NULL}}, //set the audio message show level 0: no message, 1: inf, warn and err, 2: warn, err, 3: err
	{"AUINFO",      fAUINFO,    {NULL, NULL}},
	{"AUDRST",      fAUDRST,    {NULL, NULL}},
	//For AVsync testing
	{"AUSYNCEN",    fAUSYNCEN,  {NULL, NULL}},
	{"AUSYNCTS",    fAUSYNCTS,  {NULL, NULL}},
	{"AUGETSYNC",   fAUGETSYNC, {NULL, NULL}},
#if P2P_ENABLE
	{"AURXP2P",     fAURXP2P,   {NULL, NULL}}, //enable rx stream (expect TX is playback mode)
	{"P2PEN",       fP2PEN,     {NULL, NULL}}, //enable P2P
#endif
};

void audio_save_log_init(void)
{
	log_service_add_table(at_audio_save_items, sizeof(at_audio_save_items) / sizeof(at_audio_save_items[0]));
}

log_module_init(audio_save_log_init);