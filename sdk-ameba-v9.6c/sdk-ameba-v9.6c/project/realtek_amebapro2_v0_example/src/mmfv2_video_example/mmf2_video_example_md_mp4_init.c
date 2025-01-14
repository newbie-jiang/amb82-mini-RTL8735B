/******************************************************************************
*
* Copyright(c) 2007 - 2023 Realtek Corporation. All rights reserved.
*
******************************************************************************/

// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <assert.h>

#include "mmf2_link.h"
#include "mmf2_siso.h"
#include "mmf2_mimo.h"

#include "module_video.h"
#include "module_rtsp2.h"
#include "module_audio.h"
#include "module_aac.h"
#include "module_mp4.h"
#include "module_eip.h"
#include "module_queue.h"
#include "mmf2_pro2_video_config.h"
#include "video_example_media_framework.h"
#include "log_service.h"
#include "sensor.h"
#include <sntp/sntp.h>


#include "ff.h"

#include "osdep_service.h"
#include "sys_api.h"
#include "vfs.h"
// #include "ameba_sqlite_common.h"
#include "sqlite3.h"

static sqlite3 *db;

char save_file_name[128];
static char osd_tim[128];
static FILINFO _fno;


/*****************************************************************************
* ISP channel : 0,1
* Video type  : H264/HEVC
*****************************************************************************/

#define V1_CHANNEL 0
#define V1_BPS 2*1024*1024
#define V1_RCMODE 2 // 1: CBR, 2: VBR
#define V1_FPS 15

#define V2_CHANNEL 1
#define V2_BPS 2*1024*1024
#define V2_RCMODE 2 // 1: CBR, 2: VBR
#define V2_FPS 15
#define V2_WIDTH	1280
#define V2_HEIGHT	720

#define V3_CHANNEL 2
#define V3_BPS     2*1024*1024
#define V3_RCMODE  2 // 1: CBR, 2: VBR
#define V3_FPS     15
#define V3_WIDTH   1280
#define V3_HEIGHT  720

#define MD_CHANNEL 4
#define MD_GOP MD_FPS
#define MD_BPS 1024*1024
#define MD_TYPE VIDEO_RGB
#define MD_WIDTH	640
#define MD_HEIGHT	480

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
static mm_context_t *video_v2_ctx			= NULL;
static mm_context_t *video_v3_ctx			= NULL;

static mm_context_t *video_rgb_ctx			= NULL;

static mm_context_t *rtsp2_v1_ctx			= NULL;
static mm_context_t *rtsp2_v2_ctx			= NULL;
static mm_context_t *rtsp2_v3_ctx			= NULL;

static mm_context_t *audio_ctx				= NULL;
static mm_context_t *aac_ctx				= NULL;
static mm_context_t *mp4_ctx				= NULL;
static mm_context_t *queue_ctx				= NULL;
static mm_context_t *md_ctx            		= NULL;

static mm_siso_t *siso_audio_aac			= NULL;
static mm_mimo_t *mimo_2v_1a_rtsp_queue		= NULL;
static mm_siso_t *siso_queue_mp4			= NULL;
static mm_siso_t *siso_rgb_md         		= NULL;
static mm_mimo_t *mimo_2v_1a_rtsp			= NULL;

static video_params_t video_v1_params = {
	.stream_id = V1_CHANNEL,
	.type = VIDEO_TYPE,
	.bps = V1_BPS,
	.rc_mode = V1_RCMODE,
	.use_static_addr = 1,
	.fps = V1_FPS
};

static video_params_t video_v2_params = {
	.stream_id = V2_CHANNEL,
	.type = VIDEO_JPEG,
	.use_static_addr = 1
};


static video_params_t video_v3_params = {
	.stream_id = V3_CHANNEL,
	.type = VIDEO_TYPE,
	.bps = V1_BPS,
	.rc_mode = V1_RCMODE,
	.use_static_addr = 1,
};

static video_params_t video_v4_params = {
	.stream_id 		= MD_CHANNEL,
	.type 			= MD_TYPE,
	.fps 			= MD_FPS,
	.gop 			= MD_GOP,
	.direct_output 	= 0,
	.use_static_addr = 1,
	.use_roi = 1,
	.roi = {
		.xmin = 0,
		.ymin = 0,
	}
};

#if !USE_DEFAULT_AUDIO_SET
static audio_params_t audio_params = {
	.sample_rate = ASR_8KHZ,
	.word_length = WL_16BIT,
	.mic_gain    = MIC_0DB,
	.dmic_l_gain    = DMIC_BOOST_24DB,
	.dmic_r_gain    = DMIC_BOOST_24DB,
	.use_mic_type   = USE_AUDIO_AMIC,
	.channel     = 1,
	.enable_aec  = 0//1
};
#endif

static aac_params_t aac_params = {
	.sample_rate = 8000,
	.channel = 1,
	.trans_type = AAC_TYPE_ADTS,
	.object_type = AAC_AOT_LC,
	.bitrate = 32000,

	.mem_total_size = 10 * 1024,
	.mem_block_size = 128,
	.mem_frame_size = 1024
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


static rtsp2_params_t rtsp2_v2_params = {
	.type = AVMEDIA_TYPE_VIDEO,
	.u = {
		.v = {
			.codec_id = AV_CODEC_ID_MJPEG,
		}
	}
};


static rtsp2_params_t rtsp2_v3_params = {
	.type = AVMEDIA_TYPE_VIDEO,
	.u = {
		.v = {
			.codec_id = VIDEO_CODEC,
			.bps      = V1_BPS
		}
	}
};

static rtsp2_params_t rtsp2_a_params = {
	.type = AVMEDIA_TYPE_AUDIO,
	.u = {
		.a = {
			.codec_id   = AV_CODEC_ID_MP4A_LATM,
			.channel    = 1,
			.samplerate = 8000
		}
	}
};

static mp4_params_t mp4_v1_params = {
	.sample_rate = 8000,
	.channel = 1,
	.record_length = 305, //5 min 5sec
	.record_type = STORAGE_ALL,
	.record_file_num = 1,
	.record_file_name = "AmebaPro_recording",
	.fatfs_buf_size = 224 * 1024, /* 32kb multiple */
};

#define MD_COL 32
#define MD_ROW 32

static eip_param_t md_param = {
	.image_width = MD_WIDTH,
	.image_height = MD_HEIGHT,
	.eip_row = MD_ROW,
	.eip_col = MD_COL
};

//--------------------------------------------
// Draw Rect
//--------------------------------------------
#define MD_DRAW 1

#if MD_DRAW
#include "osd_render.h"
#endif

static int event_threshold = 7;
static int md_sensitivity = 75;
static char mp4_filename[128];



static TimerHandle_t mp4_stop_record_timer = NULL;
void print_sqlite_db(void);
static void mp4_stop_record_callback(TimerHandle_t xTimer)
{
	(void)xTimer;
	printf("no motion for 10 seconds\r\n");
	mm_module_ctrl(mp4_ctx, CMD_MP4_STOP, 0);
	update_tim(save_file_name,&_fno);	
	// print_sqlite_db();//打印数据库
}


static int mp4_stop_cb(void *parm)
{
	printf("Record stop\r\n");
	return 0;
}

static int mp4_end_cb(void *parm)
{
	printf("Record end\r\n");
	return 0;
}

 // 回调函数用于打印查询结果
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    for(int i = 0; i < argc; i++) {
        printf("%s = %s\t", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}




// 创建表的函数
static int create_table(sqlite3 *db){
    char *err_msg = 0;
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS file_info ("
        "file_time TEXT NOT NULL, "
        "filename TEXT NOT NULL"
        ");";

    int rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    if(rc != SQLITE_OK ){
        fprintf(stderr, "创建表 SQL 错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    printf("表 'file_info' 已创建或已存在。\n");
    return SQLITE_OK;
}

// // 插入数据的函数
static int insert_data(sqlite3 *db, const char *file_time, const char *filename) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO file_info (file_time, filename) VALUES (?, ?);";
    int rc;

    // 预编译 SQL 语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "预编译SQL失败: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // 绑定参数，将第1个问号绑定为 file_time，将第2个问号绑定为 filename
    sqlite3_bind_text(stmt, 1, file_time, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, filename, -1, SQLITE_TRANSIENT);

    // 执行插入
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "插入数据失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }

    // 清理资源
    sqlite3_finalize(stmt);

    printf("成功插入记录: %s, %s\n", file_time, filename);
    return SQLITE_OK;
}



static int delete_file(const char *filename) {
    FRESULT res;

    // 尝试删除文件
    res = f_unlink(filename);
    if (res == FR_OK) {
        printf("成功删除文件: %s\n", filename);
    } else {
        // 打印错误信息
        printf("删除文件失败: %s, 错误代码: %d\n", filename, res);
        return res;  // 返回错误代码
    }

    return FR_OK;
}

//蓝牙配网,WPS配网,RTSP双通道串流，视频存储至tf卡，动态侦测，后面也会结合其他厂家的nvr存储
//目前需要实现的一个功能，如何通过局域网查看tf卡中的视频文件？
//一般的监控设备，手机app就可以查看存储在tf卡中的视频(非局域网)，这又是如何做到的？监控设备带有反向代理？


// 获取最老记录的 filename 并删除该文件
int get_oldest_record(sqlite3 *db, char **filename) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT filename FROM file_info ORDER BY file_time ASC LIMIT 1;";
    int rc;

    // 准备 SQL 查询
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "预编译 SQL 失败: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // 执行查询并获取结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // 获取查询结果中的 filename 字段
        *filename = (char *)sqlite3_column_text(stmt, 0);
        if (*filename != NULL) {
            printf("最老记录的 filename: %s\n", *filename);
			delete_file(*filename);
        } else {
            printf("没有找到最老记录的 filename\n");
        }
    } else {
        fprintf(stderr, "获取最老记录失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }

    // 清理资源
    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

//删除最老记录的函数
static int delete_oldest_record(sqlite3 *db){
    char *err_msg = 0;
    const char *sql = 
        "WITH oldest AS ("
        "    SELECT filename FROM file_info ORDER BY file_time ASC LIMIT 1"
        ") "
        "DELETE FROM file_info WHERE filename IN (SELECT filename FROM oldest);";

    int rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    if(rc != SQLITE_OK ){
        fprintf(stderr, "删除最老记录 SQL 错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    return SQLITE_OK;
}


// 验证删除结果的函数
static int verify_deletion(sqlite3 *db){
    char *err_msg = 0;
    const char *sql = "SELECT * FROM file_info ORDER BY file_time ASC;";

    int rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    if(rc != SQLITE_OK ){
        fprintf(stderr, "验证删除结果 SQL 错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    return SQLITE_OK;
}

//存储空间满时将清除数据
void sd_card_full_process(void)
{
	//获得SD剩余空间
    long long int free_spase_byte = fatfs_get_free_space_byte(); 

	long long int free_spase_MB = free_spase_byte/1024/1024;

	printf(".....................................sd free spase is %lldMB \r\n",free_spase_MB);

	//SD的剩余空间较小删除最早视频文件
	if(free_spase_MB<80) //删除最早的视频文件
	{
		while(free_spase_MB<90)
		{
          char *oldest_filename = NULL;
          if (get_oldest_record(db, &oldest_filename) != SQLITE_OK) 
		    {
  		         fprintf(stderr, "获取最老记录失败\n");
			} else {
    		     // 现在可以使用 `oldest_filename` 执行文件系统删除操作
   		         printf("要删除的最老记录的文件: %s\n", oldest_filename);
			}

          // 删除最老记录
    	  if(delete_oldest_record(db) != SQLITE_OK){
             fprintf(stderr, "删除最老记录失败\n");
    	  } else {
       		 printf("\n已删除最老的记录。\n");
          }

		 free_spase_byte = fatfs_get_free_space_byte();

	     free_spase_MB = free_spase_byte/1024/1024;

		 printf("..while...................................sd free spase is %lldMB \r\n",free_spase_MB);
		}
	} 
}


extern test_sqlite3(void);
// 数据库文件名
#define DATABASE_NAME "file_database.db"

//初始化
void sqlite3_init_create_video_database(void)
{
	 int rc;
     vfs_init(NULL);
     vfs_user_register("sd", VFS_FATFS, VFS_INF_SD);

     sqlite3_initialize();

   // 打开连接到数据库文件（如果不存在，将创建一个新的）
    rc = sqlite3_open(DATABASE_NAME, &db);
    if(rc){
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stdout, "成功打开数据库\n");
    }

	// 创建表
    if(create_table(db) != SQLITE_OK){
        fprintf(stderr, "创建表失败\n");
        sqlite3_close(db);
        return 1;
    }
}

void print_sqlite_db(void)
{
   verify_deletion(db);
}

char save_file_name[128];


static void md_process(void *md_result)
{
    int rc;
	md_result_t *md_res = (md_result_t *) md_result;

	struct tm now_tim = sntp_gen_system_time_s(8 * 3600);
    snprintf(osd_tim, sizeof(osd_tim), "%04d-%02d-%02d %02d:%02d:%02d", now_tim.tm_year, now_tim.tm_mon, now_tim.tm_mday, now_tim.tm_hour, now_tim.tm_min,
					 now_tim.tm_sec);

   	if (md_res->event_cnt >= event_threshold) {
		int record_status = 0;
		mm_module_ctrl(mp4_ctx, CMD_MP4_GET_STATUS, (int)&record_status);
		if (record_status) {
			xTimerReset(mp4_stop_record_timer, 10);
		} else {
			struct tm tm_now = sntp_gen_system_time_s(8 * 3600);
			snprintf(mp4_filename, sizeof(mp4_filename), "%04d%02d%02d%02d%02d%02d", tm_now.tm_year, tm_now.tm_mon, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min,
					 tm_now.tm_sec);
			printf("start recording %s\r\n", mp4_filename);
			snprintf(save_file_name, sizeof(save_file_name), "%s.mp4", mp4_filename);

			sd_card_full_process();                // SD卡溢出检测

			mm_module_ctrl(mp4_ctx, CMD_MP4_SET_RECORD_FILE_NAME, (int)mp4_filename);
			insert_data(db,osd_tim,save_file_name); //插入数据
          
			mm_module_ctrl(mp4_ctx, CMD_MP4_START, 1);
			xTimerStart(mp4_stop_record_timer, 10);
		}
	}

#if MD_DRAW

	    //mp4 only draw text
	  	int motion = md_res->motion_cnt;  
        //显示时间
     
	    canvas_create_bitmap(V1_CHANNEL, 0, RTS_OSD2_BLK_FMT_1BPP);
        canvas_set_text(V1_CHANNEL, 0, 5, 5, osd_tim, COLOR_WHITE);  // 将文本设置为白色，并显示在(10,10)位置

	    if (motion) {
		int xmin = (int)(md_res->md_pos[0].xmin * V2_WIDTH);
		int ymin = (int)(md_res->md_pos[0].ymin * V2_HEIGHT);
		int xmax = (int)(md_res->md_pos[0].xmax * V2_WIDTH);
		int ymax = (int)(md_res->md_pos[0].ymax * V2_HEIGHT);

		canvas_set_rect(V1_CHANNEL, 0, xmin, ymin, xmax, ymax, 2, COLOR_WHITE);
	   }
	    canvas_update(V1_CHANNEL, 0, 1);

        canvas_create_bitmap(V2_CHANNEL, 0, RTS_OSD2_BLK_FMT_1BPP);
        canvas_set_text(V2_CHANNEL, 0, 5, 5, osd_tim, COLOR_WHITE);  // 将文本设置为白色，并显示在(10,10)位置
	  if (motion) {

		int xmin = (int)(md_res->md_pos[0].xmin * V2_WIDTH);
		int ymin = (int)(md_res->md_pos[0].ymin * V2_HEIGHT);
		int xmax = (int)(md_res->md_pos[0].xmax * V2_WIDTH);
		int ymax = (int)(md_res->md_pos[0].ymax * V2_HEIGHT);

		canvas_set_rect(V2_CHANNEL, 0, xmin, ymin, xmax, ymax, 2, COLOR_WHITE);
	   }
    canvas_update(V2_CHANNEL, 0, 1);

	canvas_create_bitmap(V3_CHANNEL, 0, RTS_OSD2_BLK_FMT_1BPP);
    canvas_set_text(V3_CHANNEL, 0, 5, 5, osd_tim, COLOR_WHITE);  // 将文本设置为白色，并显示在(10,10)位置
	  if (motion) {

		int xmin = (int)(md_res->md_pos[0].xmin * V2_WIDTH);
		int ymin = (int)(md_res->md_pos[0].ymin * V2_HEIGHT);
		int xmax = (int)(md_res->md_pos[0].xmax * V2_WIDTH);
		int ymax = (int)(md_res->md_pos[0].ymax * V2_HEIGHT);

		canvas_set_rect(V3_CHANNEL, 0, xmin, ymin, xmax, ymax, 2, COLOR_WHITE);
	   }
    canvas_update(V3_CHANNEL, 0, 1);
#endif
}


void mmf2_video_example_md_mp4_init(void)
{
  atcmd_userctrl_init();

  sqlite3_init_create_video_database();  //数据库初始化

#if defined(ENABLE_META_INFO)
	unsigned char uuid[16] = {0xc7, 0x98, 0x2c, 0x28, 0x0a, 0xfc, 0x49, 0xe6, 0xaa, 0xe4, 0x7f, 0x8f, 0x64, 0xee, 0x65, 0x01};
	video_pre_init_params_t init_params;
	memset(&init_params, 0x00, sizeof(video_pre_init_params_t));
	init_params.meta_enable = 1;
	init_params.meta_size = VIDEO_META_USER_SIZE;
	memcpy(init_params.video_meta_uuid, uuid, VIDEO_META_UUID_SIZE);
	video_pre_init_setup_parameters(&init_params);//It only setup one time.
	video_v1_params.meta_enable = 1;
	video_v2_params.meta_enable = 1;
	for (int i = 0; i < 64; i++) {
		meta_user_buf[i] = i;
	}
#endif

	video_v1_params.resolution = VIDEO_FHD;
	video_v1_params.width = 1280;
	video_v1_params.height = 720;
	video_v1_params.fps = V1_FPS;
	video_v1_params.gop = V1_FPS;

	video_v2_params.resolution = VIDEO_HD;
	video_v2_params.width = 1280;
	video_v2_params.height = 720;
	video_v2_params.fps = 15;
	video_v2_params.gop = 15;

	video_v3_params.resolution = VIDEO_HD;
	video_v3_params.width = 1280;
	video_v3_params.height = 720;
    video_v3_params.fps = 15 ;
	video_v3_params.gop = 15 ;

	video_v4_params.width = MD_WIDTH;
	video_v4_params.height = MD_HEIGHT;
	video_v4_params.fps = MD_FPS;
	video_v4_params.gop = MD_FPS;
	video_v4_params.roi.xmax = sensor_params[USE_SENSOR].sensor_width;
	video_v4_params.roi.ymax = sensor_params[USE_SENSOR].sensor_height;

 	/*mp4 parameter setting*/
 	mp4_v1_params.fps = V1_FPS;
	mp4_v1_params.gop = V1_FPS;
	mp4_v1_params.width = 1920;
	mp4_v1_params.height = 1080;

	/* rtsp parameter setting */
    rtsp2_v1_params.u.v.fps = 15;
	rtsp2_v2_params.u.v.fps = 15;
    rtsp2_v3_params.u.v.fps = 15;

	int voe_heap_size = video_voe_presetting(1, video_v1_params.width, video_v1_params.height, V1_BPS, 0,
						1, video_v2_params.width, video_v2_params.height, V2_BPS, 0,
						1, video_v3_params.width, video_v3_params.height, V3_BPS, 0,
						1, MD_WIDTH, MD_HEIGHT);  

	printf("\r\n voe heap size = %d\r\n", voe_heap_size);
	if (voe_boot_fsc_status()) {
		video_v1_params.fcs = 1;
	}

	// ---------- Channel 1--------------
	video_v1_ctx = mm_module_open(&video_module);
	if (video_v1_ctx) {
		mm_module_ctrl(video_v1_ctx, CMD_VIDEO_SET_PARAMS, (int)&video_v1_params);
		mm_module_ctrl(video_v1_ctx, MM_CMD_SET_QUEUE_LEN, video_v1_params.fps);
		mm_module_ctrl(video_v1_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);

	} else {
		rt_printf("video open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

	// ----------- Channel 2--------------
	video_v2_ctx = mm_module_open(&video_module);
	if (video_v2_ctx) {
		mm_module_ctrl(video_v2_ctx, CMD_VIDEO_SET_PARAMS, (int)&video_v2_params);
		mm_module_ctrl(video_v2_ctx, MM_CMD_SET_QUEUE_LEN, video_v2_params.fps * 3);
		mm_module_ctrl(video_v2_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);

	} else {
		rt_printf("video open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

    //------------ Channel 3--------------
	video_v3_ctx = mm_module_open(&video_module);
	if (video_v3_ctx) {
		mm_module_ctrl(video_v3_ctx, CMD_VIDEO_SET_PARAMS, (int)&video_v3_params);
		mm_module_ctrl(video_v3_ctx, MM_CMD_SET_QUEUE_LEN, video_v3_params.fps);
		mm_module_ctrl(video_v3_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);

	} else {
		rt_printf("video open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

	//------ Channel 4--------------
	video_rgb_ctx = mm_module_open(&video_module);
	if (video_rgb_ctx) {
		mm_module_ctrl(video_rgb_ctx, CMD_VIDEO_SET_PARAMS, (int)&video_v4_params);
		mm_module_ctrl(video_rgb_ctx, MM_CMD_SET_QUEUE_LEN, 2);
		mm_module_ctrl(video_rgb_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);
	} else {
		printf("video open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

	md_ctx  = mm_module_open(&eip_module);
	if (md_ctx) {
		md_config_t md_config;
		mm_module_ctrl(md_ctx, CMD_EIP_GET_MD_CONFIG, (int)&md_config); //get default md config
		md_config.md_obj_sensitivity = md_sensitivity;
		mm_module_ctrl(md_ctx, CMD_EIP_SET_PARAMS, (int)&md_param);
		mm_module_ctrl(md_ctx, CMD_EIP_SET_MD_DISPPOST, (int)md_process);
		mm_module_ctrl(md_ctx, CMD_EIP_SET_MD_CONFIG, (int)&md_config);
		mm_module_ctrl(md_ctx, CMD_EIP_SET_MD_EN, 1);
		mm_module_ctrl(md_ctx, CMD_EIP_SET_STATUS, EIP_STATUS_START);
	} else {
		printf("md_ctx open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

   	//--------------MP4---------------
	mp4_ctx = mm_module_open(&mp4_module);
	if (mp4_ctx) {
		mm_module_ctrl(mp4_ctx, CMD_MP4_SET_PARAMS, (int)&mp4_v1_params);
		mm_module_ctrl(mp4_ctx, CMD_MP4_LOOP_MODE, 0); 
		mm_module_ctrl(mp4_ctx, CMD_MP4_SET_STOP_CB, (int)mp4_stop_cb);
		mm_module_ctrl(mp4_ctx, CMD_MP4_SET_END_CB, (int)mp4_end_cb);
	} else {
		printf("MP4 open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}
	
	mp4_stop_record_timer = xTimerCreate("mp4_stop_record_timer", 10000 / portTICK_PERIOD_MS, pdFALSE, NULL, mp4_stop_record_callback);

    sntp_init();

	queue_ctx = mm_module_open(&queue_module);
	if (queue_ctx) {

		int audio_queue_len = mp4_v1_params.sample_rate / 1024; 

		mm_module_ctrl(queue_ctx, MM_CMD_SET_QUEUE_LEN, (15 + audio_queue_len) * 20);          //mp4 init takes 20s, buffer 10 seconds video and audio
		mm_module_ctrl(queue_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);   
		
		//此处为 15*15时 测试时存储的视频开始存在6s左右的延迟
		mm_module_ctrl(queue_ctx, CMD_QUEUE_SET_VQUEUE_LEN, 15 * 10);                         // buffer 10 second video
		mm_module_ctrl(queue_ctx, CMD_QUEUE_SET_AQUEUE_LEN, audio_queue_len * 10);            // buffer 10 second audio
		
	} else {
		printf("QUEUE open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

	//--------------Audio --------------
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
		goto mmf2_video_example_md_mp4_fail;
	}

	aac_ctx = mm_module_open(&aac_module);
	if (aac_ctx) {
		mm_module_ctrl(aac_ctx, CMD_AAC_SET_PARAMS, (int)&aac_params);
		mm_module_ctrl(aac_ctx, MM_CMD_SET_QUEUE_LEN, 16);
		mm_module_ctrl(aac_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);
		mm_module_ctrl(aac_ctx, CMD_AAC_INIT_MEM_POOL, 0);
		mm_module_ctrl(aac_ctx, CMD_AAC_APPLY, 0);
	} else {
		rt_printf("AAC open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

	//--------------RTSP---------------
    rtsp2_v2_ctx = mm_module_open(&rtsp2_module);
	if (rtsp2_v2_ctx) {
		
		mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SELECT_STREAM, 0);
		mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SET_PARAMS, (int)&rtsp2_v2_params);
		mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SET_APPLY, 0);
	    mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SET_STREAMMING, ON);

        //mjpeg audio --> rtsp2
		mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SELECT_STREAM, 1);
		mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SET_PARAMS, (int)&rtsp2_a_params);
		mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SET_APPLY, 0);

		mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SET_STREAMMING, ON);
	} else {
		rt_printf("RTSP2 open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

	rtsp2_v3_ctx = mm_module_open(&rtsp2_module); 
	if (rtsp2_v3_ctx) {
		mm_module_ctrl(rtsp2_v3_ctx, CMD_RTSP2_SELECT_STREAM, 0);
		mm_module_ctrl(rtsp2_v3_ctx, CMD_RTSP2_SET_PARAMS, (int)&rtsp2_v3_params);
		mm_module_ctrl(rtsp2_v3_ctx, CMD_RTSP2_SET_APPLY, 0);

		mm_module_ctrl(rtsp2_v3_ctx, CMD_RTSP2_SELECT_STREAM, 1);
		mm_module_ctrl(rtsp2_v3_ctx, CMD_RTSP2_SET_PARAMS, (int)&rtsp2_a_params);
		mm_module_ctrl(rtsp2_v3_ctx, CMD_RTSP2_SET_APPLY, 0);

		mm_module_ctrl(rtsp2_v3_ctx, CMD_RTSP2_SET_STREAMMING, ON);
	} else {
		rt_printf("RTSP2 open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

	//--------------Link---------------------------
	siso_audio_aac = siso_create();
	if (siso_audio_aac) {
		siso_ctrl(siso_audio_aac, MMIC_CMD_ADD_INPUT, (uint32_t)audio_ctx, 0);
		siso_ctrl(siso_audio_aac, MMIC_CMD_ADD_OUTPUT, (uint32_t)aac_ctx, 0);
		siso_ctrl(siso_audio_aac, MMIC_CMD_SET_STACKSIZE, 44 * 1024, 0);
		siso_start(siso_audio_aac);
	} else {
		rt_printf("siso1 open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

	rt_printf("siso started\n\r");

	mimo_2v_1a_rtsp_queue = mimo_create();
	if (mimo_2v_1a_rtsp_queue) {
#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
		mimo_ctrl(mimo_2v_1a_rtsp_queue, MMIC_CMD_SET_SECURE_CONTEXT, 1, 0);
#endif
        mimo_ctrl(mimo_2v_1a_rtsp_queue, MMIC_CMD_ADD_INPUT0, (uint32_t)video_v1_ctx, 0);
		mimo_ctrl(mimo_2v_1a_rtsp_queue, MMIC_CMD_ADD_INPUT1, (uint32_t)video_v2_ctx, 0);
		mimo_ctrl(mimo_2v_1a_rtsp_queue, MMIC_CMD_ADD_INPUT2, (uint32_t)video_v3_ctx, 0);
		mimo_ctrl(mimo_2v_1a_rtsp_queue, MMIC_CMD_ADD_INPUT3, (uint32_t)aac_ctx, 0);

		mimo_ctrl(mimo_2v_1a_rtsp_queue, MMIC_CMD_ADD_OUTPUT0, (uint32_t)queue_ctx, MMIC_DEP_INPUT0 | MMIC_DEP_INPUT3);
		mimo_ctrl(mimo_2v_1a_rtsp_queue, MMIC_CMD_ADD_OUTPUT1, (uint32_t)rtsp2_v2_ctx, MMIC_DEP_INPUT1 | MMIC_DEP_INPUT3);
        mimo_ctrl(mimo_2v_1a_rtsp_queue, MMIC_CMD_ADD_OUTPUT2, (uint32_t)rtsp2_v3_ctx, MMIC_DEP_INPUT2 | MMIC_DEP_INPUT3);
	
		mimo_start(mimo_2v_1a_rtsp_queue);

	} else {
		rt_printf("mimo open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}

    mm_module_ctrl(video_v1_ctx, CMD_VIDEO_APPLY, V1_CHANNEL);
    mm_module_ctrl(video_v2_ctx, CMD_VIDEO_APPLY, V2_CHANNEL);   
	mm_module_ctrl(video_v2_ctx, CMD_VIDEO_SNAPSHOT, 2);      // jpeg串流不可删除
	mm_module_ctrl(video_v3_ctx, CMD_VIDEO_APPLY, V3_CHANNEL);
	 
	rt_printf("mimo started\n\r");

// ------------ siso rgb md ------------
	siso_rgb_md = siso_create();
	if (siso_rgb_md) {
#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
		siso_ctrl(siso_rgb_md, MMIC_CMD_SET_SECURE_CONTEXT, 1, 0);
#endif
		siso_ctrl(siso_rgb_md, MMIC_CMD_ADD_INPUT, (uint32_t)video_rgb_ctx, 0);
		siso_ctrl(siso_rgb_md, MMIC_CMD_SET_STACKSIZE, (uint32_t)1024 * 64, 0);
		siso_ctrl(siso_rgb_md, MMIC_CMD_SET_TASKPRIORITY, 3, 0);
		siso_ctrl(siso_rgb_md, MMIC_CMD_ADD_OUTPUT, (uint32_t)md_ctx, 0);
		siso_start(siso_rgb_md);

	} else {
		printf("siso_rgb_md open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}
	printf("siso_rgb_md started\n\r");
	mm_module_ctrl(video_rgb_ctx, CMD_VIDEO_APPLY, MD_CHANNEL);	// start channel 4
	mm_module_ctrl(video_rgb_ctx, CMD_VIDEO_YUV, 2);
    
    // ------------siso  mp4--------------
   	siso_queue_mp4 = siso_create();
	
	if (siso_queue_mp4) {
		siso_ctrl(siso_queue_mp4, MMIC_CMD_ADD_INPUT, (uint32_t)queue_ctx, 0);
		siso_ctrl(siso_queue_mp4, MMIC_CMD_ADD_OUTPUT, (uint32_t)mp4_ctx, 0);
		siso_start(siso_queue_mp4);	
	
	} else {
		printf("siso2 open fail\n\r");
		goto mmf2_video_example_md_mp4_fail;
	}
	printf("siso_queue_mp4 started\n\r");

	#if MD_DRAW

	 int ch_enable[4] = {1, 1, 1,0};
	 int char_resize_w[4] = {16, 16, 16,0}, char_resize_h[4] = {32, 32, 32,0};
	 int ch_width[4] = {video_v1_params.width, video_v2_params.width, video_v3_params.width,0};
	 int ch_height[4] = {video_v1_params.height, video_v2_params.height, video_v3_params.height,0};

	 osd_render_dev_init(ch_enable, char_resize_w, char_resize_h);
	 osd_render_task_start(ch_enable, ch_width, ch_height);
	

    #endif

	return;
mmf2_video_example_md_mp4_fail:


	return;
}


static const char *example = "mmf2_video_example_md_mp4_init";
static void example_deinit(void)
{
	sntp_stop();
	xTimerDelete(mp4_stop_record_timer, 0xFFFFFFFF);
	mm_module_ctrl(md_ctx, CMD_EIP_SET_STATUS, EIP_STATUS_STOP);

#if MD_DRAW
	osd_render_task_stop();
	osd_render_dev_deinit_all();  
#endif

	//Pause Linker
	siso_pause(siso_audio_aac);
	mimo_pause(mimo_2v_1a_rtsp_queue, MM_OUTPUT0 | MM_OUTPUT1);
	siso_pause(siso_rgb_md);
	siso_pause(siso_queue_mp4);

	//Stop module
	mm_module_ctrl(rtsp2_v2_ctx, CMD_RTSP2_SET_STREAMMING, OFF);
	mm_module_ctrl(aac_ctx, CMD_AAC_STOP, 0);
	mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_TRX, 0);
	mm_module_ctrl(mp4_ctx, CMD_MP4_STOP, 0);
	mm_module_ctrl(video_rgb_ctx, CMD_VIDEO_STREAM_STOP, 0);
	mm_module_ctrl(video_v1_ctx, CMD_VIDEO_STREAM_STOP, 0);
	mm_module_ctrl(video_v2_ctx, CMD_VIDEO_STREAM_STOP, 0);

	//Delete linker
	siso_delete(siso_queue_mp4);
	siso_delete(siso_rgb_md);
	mimo_delete(mimo_2v_1a_rtsp_queue);
	siso_delete(siso_audio_aac);

	//Close module
	rtsp2_v2_ctx = mm_module_close(rtsp2_v2_ctx);
	aac_ctx = mm_module_close(aac_ctx);
	audio_ctx = mm_module_close(audio_ctx);
	mp4_ctx = mm_module_close(mp4_ctx);
	queue_ctx = mm_module_close(queue_ctx);
	md_ctx = mm_module_close(md_ctx);
	video_rgb_ctx = mm_module_close(video_rgb_ctx);
	video_v2_ctx = mm_module_close(video_v2_ctx);
	video_v1_ctx = mm_module_close(video_v1_ctx);
    
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
			printf("invalid state, can not do %s init!\r\n", example);
		}
	} else {
		printf("invalid cmd");
	}
	printf("user command 0x%x\r\n", user_cmd);
}

#define MAX_ARGC 18

static void fMD(void *arg)
{
	char *argv[MAX_ARGC] = {0};
	if (!arg) {
		return;
	}

	parse_param(arg, argv);

	if (strcmp(argv[1], "sen") == 0) {
		int sen = atoi(argv[2]);
		if (md_ctx) {
			mm_module_ctrl(md_ctx, CMD_EIP_SET_MD_SENSITIVITY, sen);
			printf("Set MD sensitivity %d\r\n", sen);
		}
	} else if (strcmp(argv[1], "thr") == 0) {
		int event = atoi(argv[2]);
		if (event < MD_FPS) {
			event_threshold = event;
			printf("Set event threshold %d\r\n", event_threshold);
		} else {
			printf("Event threshold out of range (0-%d)\r\n", MD_FPS);
		}
	}
	return;
}

static log_item_t userctrl_items[] = {
	{"UC", fUC, },
	{"MD", fMD, },
};

static void atcmd_userctrl_init(void)
{
	log_service_add_table(userctrl_items, sizeof(userctrl_items) / sizeof(userctrl_items[0]));
}