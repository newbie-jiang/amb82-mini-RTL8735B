#include "platform_opts.h"
#if FATFS_DISK_SD
#include "platform_stdlib.h"
#include "ff.h"
#include <fatfs_ext/inc/ff_driver.h>
#include "sdio_combine.h"
#include "sdio_host.h"
#include <disk_if/inc/sdcard.h>
#include "fatfs_sdcard_api.h"
#include "sd.h"
//extern phal_sdio_host_adapter_t psdioh_adapter;

static fatfs_sd_params_t fatfs_sd_param;
static uint8_t fatfs_sd_init_done = 0;
static FIL     fatfs_sd_file;
static unsigned char *fatfs_sd_buf = NULL;
static uint32_t fatfs_sd_buf_size;
static uint32_t fatfs_sd_buf_pos;

#include "gpio_api.h"
#define DEMO_Board_SD_POWER_PIN    PF_16
#define SD_POWER_RESET_DELAY_TIME 100
static gpio_t gpio_sd_power;
static int sd_gpio_init_status = 0;

void sd_gpio_init(void)
{
	if (sd_gpio_init_status == 0) {
		sd_gpio_init_status = 1;
		gpio_init(&gpio_sd_power, DEMO_Board_SD_POWER_PIN);
		gpio_dir(&gpio_sd_power, PIN_OUTPUT);    // Direction: Output
		gpio_mode(&gpio_sd_power, PullNone);     // No pull
		gpio_write(&gpio_sd_power, 0);
		vTaskDelay(SD_POWER_RESET_DELAY_TIME);
	} else {
		printf("The sd gpio pin has already init\r\n");
	}
}

void sd_gpio_deinit(void)
{
	if (sd_gpio_init_status) {
		gpio_deinit(&gpio_sd_power);
		sd_gpio_init_status = 0;
	} else {
		printf("It don't have init the sd card\r\n");
	}
}

void sd_gpio_power_reset(void)
{
	gpio_write(&gpio_sd_power, 1);
	vTaskDelay(SD_POWER_RESET_DELAY_TIME);//The time is depend on drop speed oof your gpio, or we can read the gpio status
	gpio_write(&gpio_sd_power, 0);
	vTaskDelay(SD_POWER_RESET_DELAY_TIME);
}

void sd_reset_procedure(int reason)
{
	int ret = 0;
	sd_gpio_power_reset();
	SD_DeInit();
	vTaskDelay(SD_POWER_RESET_DELAY_TIME);
	ret = SD_Init();
	printf("sd_reset_procedure ret = %d\r\n", ret);
}

void sd_gpio_power_off(void)
{
	gpio_write(&gpio_sd_power, 1);
	vTaskDelay(SD_POWER_RESET_DELAY_TIME);//The time is depend on drop speed oof your gpio, or we can read the gpio status
}

void sd_gpio_power_off_withoutdelay(void)
{
	gpio_write(&gpio_sd_power, 1);
}

void sd_gpio_power_on(void)
{
	gpio_write(&gpio_sd_power, 0);
	vTaskDelay(SD_POWER_RESET_DELAY_TIME);//The time is depend on drop speed oof your gpio, or we can read the gpio status
}

int fatfs_sd_is_inited(void)
{
	return fatfs_sd_init_done;
}

int fatfs_sd_close(void)
{
	if (fatfs_sd_init_done) {
		if (f_mount(NULL, fatfs_sd_param.drv, 1) != FR_OK) {
			printf("FATFS unmount logical drive fail.\n");
		}

		if (FATFS_UnRegisterDiskDriver(fatfs_sd_param.drv_num)) {
			printf("Unregister disk driver from FATFS fail.\n");
		}

		//sdio_deinit_host(psdioh_adapter);
		//deinit_combine(psdioh_adapter);
		fatfs_sd_init_done = 0;
	}
	return 0;
}

int fatfs_sd_init(void)
{
	int ret = 0;

	if (!fatfs_sd_init_done) {
		fatfs_sd_init_done = 1;
		int Fatfs_ok = 0;
		FRESULT res;
		sd_gpio_init();
		sdio_driver_init();
		// Register disk driver to Fatfs
		printf("Register disk driver to Fatfs.\n\r");
		fatfs_sd_param.drv_num = FATFS_RegisterDiskDriver(&SD_disk_Driver);

		if (fatfs_sd_param.drv_num < 0) {
			printf("Rigester disk driver to FATFS fail.\n\r");
		} else {
			Fatfs_ok = 1;
			fatfs_sd_param.drv[0] = fatfs_sd_param.drv_num + '0';
			fatfs_sd_param.drv[1] = ':';
			fatfs_sd_param.drv[2] = '/';
			fatfs_sd_param.drv[3] = 0;
		}
		if (!Fatfs_ok) {
			ret = -1;
			goto fatfs_init_err;
		}

		res = f_mount(&fatfs_sd_param.fs, fatfs_sd_param.drv, 1);
		if (res) {
			printf("f_mount error here, please insert SD card or use f_mkfs to format SD card to FAT32");
			//res = f_mkfs(fatfs_sd_param.drv,0,0);
			if (f_mount(&fatfs_sd_param.fs, fatfs_sd_param.drv, 1) != FR_OK) {
				printf("FATFS mount logical drive on sd card fail.\n\r");
				ret = -2;
				goto fatfs_init_err;
			} else {
				fatfs_sd_init_done = 1;
			}
		} else {
			fatfs_sd_init_done = 1;
		}
	} else {

	}
	return 0;

fatfs_init_err:
	fatfs_sd_close();
	return ret;
}

int fatfs_sd_get_param(fatfs_sd_params_t *param)
{
	if (fatfs_sd_init_done)	{
		memcpy(param, &fatfs_sd_param, sizeof(fatfs_sd_params_t));
		return 0;
	} else {
		memset(param, 0, sizeof(fatfs_sd_params_t));
		return -1;
	}
}

int fatfs_sd_format(void *parm)
{
	int ret = 0;
	(void)parm;
	if (fatfs_sd_init_done) {
		MKFS_PARM format_attr;
		format_attr.fmt = FM_EXFAT;
		format_attr.n_fat = 0;
		format_attr.align = 0;
		format_attr.n_root = 0;
		format_attr.au_size = 128;
		f_mount(&fatfs_sd_param.fs, fatfs_sd_param.drv, 0);
		ret =  f_mkfs(fatfs_sd_param.drv, &format_attr, NULL, 64 * 1024);
		if (ret) {
			printf("sd cardformat fail\r\n");
		} else {
			printf("format successful\r\n");
			ret = f_mount(&fatfs_sd_param.fs, fatfs_sd_param.drv, 1);
		}
	}
	return ret;
}

int fatfs_sd_open_file(char *filename)
{
	if (fatfs_sd_init_done)	{
		int res;
		char path[64];

		strcpy(path, fatfs_sd_param.drv);
		sprintf(&path[strlen(path)], "%s", filename);

		res = f_open(&fatfs_sd_file, path, FA_OPEN_ALWAYS | FA_WRITE);
		if (res) {
			printf("open file (%s) fail. res = %d\n\r", filename, res);
			return -1;
		}
		return 0;
	} else {
		return -2;
	}
}

int fatfs_sd_close_file(void)
{
	int res;
	res = f_close(&fatfs_sd_file);
	if (res) {
		printf("close file fail.\n\r");
		return -1;
	}

	return 0;
}

void fatfs_sd_write(char *buf, uint32_t len)
{
	int res = 0;
	uint32_t bw;
	int offset = 0;

	//printf("fatfs_sd_write length= %d\n\r",len);

	while (len > 0) {
		if (fatfs_sd_buf_pos + len >= fatfs_sd_buf_size) {
			memcpy(fatfs_sd_buf + fatfs_sd_buf_pos, buf + offset, fatfs_sd_buf_size - fatfs_sd_buf_pos);

			res = f_write(&fatfs_sd_file, fatfs_sd_buf, fatfs_sd_buf_size, &bw);
			if (res) {
				printf("Write error (%d)\n\r", res);
				f_lseek(&fatfs_sd_file, 0);
			}
			//printf("Write %d bytes.\n\r", bw);
			//vTaskDelay(1);
			offset += fatfs_sd_buf_size - fatfs_sd_buf_pos;
			len -= fatfs_sd_buf_size - fatfs_sd_buf_pos;
			fatfs_sd_buf_pos = 0;
		} else {
			memcpy(fatfs_sd_buf + fatfs_sd_buf_pos, buf + offset, len);
			fatfs_sd_buf_pos = fatfs_sd_buf_pos + len;
			len = 0;
		}
	}
}

void fatfs_sd_flush_buf(void)
{
	int res = 0;
	if (fatfs_sd_buf_pos != 0) {
		uint32_t bw;
		//printf("flush %d bytes before close file\n\r",fatfs_sd_buf_pos);
		res = f_write(&fatfs_sd_file, fatfs_sd_buf, fatfs_sd_buf_pos, &bw);
		if (res) {
			printf("Write error (%d)\n\r", res);
			f_lseek(&fatfs_sd_file, 0);
		}
		fatfs_sd_buf_pos = 0;
	}
}

void fatfs_sd_free_write_buf(void)
{
	if (fatfs_sd_buf) {
		free(fatfs_sd_buf);
	}
}

int fatfs_sd_create_write_buf(uint32_t buf_size)
{
	if (buf_size == 0) {
		printf("ERROR: buf_size can't be 0\n\r");
		return -1;
	}
	fatfs_sd_free_write_buf();
	fatfs_sd_buf = (unsigned char *)malloc(buf_size);
	if (fatfs_sd_buf == NULL) {
		printf("allocate fatfs_sd_buf fail\r\n");
		return -2;
	}
	memset(fatfs_sd_buf, 0, buf_size);
	fatfs_sd_buf_size = buf_size;
	fatfs_sd_buf_pos = 0;
	return 0;
}

FRESULT scan_files(
	char *path        /* Start node to be scanned (also used as work area) */
)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
	static char lfn[_MAX_LFN + 1];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
#endif
	char cur_path[512] = {0};
	res = f_opendir(&dir, path);                       /* Open the directory */
	if (res == FR_OK) {
		sprintf(cur_path, path);
		for (;;) {
			res = f_readdir(&dir, &fno);                   /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0) {
				break;    /* Break on error or end of dir */
			}
			if (fno.fname[0] == '.') {
				continue;    /* Ignore dot entry */
			}
#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif
			if (fno.fattrib & AM_DIR) {                    /* It is a directory */
				sprintf(&cur_path[strlen(path)], "/%s", fn);
				scan_files(cur_path);
			} else {                                       /* It is a file. */
				printf("%s/%s\r\n", path, fn);
			}
		}
	}
	f_closedir(&dir);
	return res;
}

int fatfs_get_free_space(void)
{
	FATFS *pfs = NULL;
	DWORD fre_clust;
	unsigned char  res;
	float sector_size = 0;
	float free_size = 0;
	sector_size = 512;

	if (fatfs_sd_init_done) {
		res = f_getfree(fatfs_sd_param.drv, &fre_clust, &pfs);
		if (res) {
			return res;
		} else {
			//TOT_SIZE = (pfs->n_fatent - 2) * pfs->csize/2; //
			free_size = (fre_clust * pfs->csize) * (sector_size / 1024 / 1024); //
		}
	}

	return (int)free_size;//MB
}


//打印所有文件
int printf_all_file(void)
{
	FRESULT res;        // 函数返回结果
    DIR dir;            // 目录对象
    FILINFO fno;        // 文件信息对象
    FATFS fs;           // 文件系统对象

    // 假设前面已经进行了 f_mount(&fs, "", 1) 挂载文件系统的操作
    // 若尚未挂载，请执行以下操作：
    // f_mount(&fs, "", 1);
    
    // 打开根目录
    res = f_opendir(&dir, "/");
    if (res == FR_OK) {
        // 循环读取根目录下的文件信息
        for (;;) {
            res = f_readdir(&dir, &fno);  // 读取下一个文件或目录项
            if (res != FR_OK || fno.fname[0] == 0) {
                // 出错或者没有更多文件时退出循环
               return -1;
            }    
            // 判断是文件还是目录
            if (fno.fattrib & AM_DIR) {
                // 这是一个目录
                printf("Directory: %s\n", fno.fname);
            } else {
                // 这是一个文件
                // 可以打印更多属性，比如文件大小、日期、时间
                // FatFs 默认使用短文件名 fno.fname 和长文件名 fno.altname 
                // （或在使用LFN配置时 fno.lfname），根据配置情况选择使用。
                // 此处仅举例使用 fno.fname。
                // printf("File: %s, Size: %lu bytes\r\n", fno.fname, (unsigned long)fno.fsize);

                // 日期时间信息
                // FatFs日期格式：bit15:9 = year from 1980, bit8:5 = month, bit4:0 = day
                // FatFs时间格式：bit15:11 = hour, bit10:5 = minute, bit4:0 = second/2
                // UINT year = (fno.fdate >> 9) + 1980;
                // UINT month = (fno.fdate >> 5) & 0x0F;
                // UINT day = fno.fdate & 0x1F;
                // UINT hour = (fno.ftime >> 11);
                // UINT minute = (fno.ftime >> 5) & 0x3F;
                // UINT second = (fno.ftime & 0x1F) * 2;

                // printf("  Date: %04u-%02u-%02u\n", year, month, day);
                // printf("  Time: %02u:%02u:%02u\r\n", hour, minute, second);

                //删除文件
                 res = f_unlink(fno.fname);
    			if (res == FR_OK) {
      			  printf("Deleted file successfully: %s\n", fno.fname);
    			} else {
    			  printf("Failed to delete file: %s, error: %d\n", fno.fname, res);
   				 }

				//当剩余空间大于100M时，不删除，返回
                int free_spase_byte = fatfs_get_free_space_byte();

	            int free_spase_MB = free_spase_byte/1024/1024;

				if(free_spase_MB>=70){
					  printf("release sd card space success\r\n");
					  return 0;

				}
            }
        }

        f_closedir(&dir);
    } else {
        printf("Failed to open root directory, error: %d\n", res);
		return -1;
    }

    return 0;
}



void update_tim(char *file_name, FILINFO *fno)
{
	f_utime(file_name, fno);	
}





long long int fatfs_get_free_space_byte(void)
{
	FATFS *pfs = NULL;
	DWORD fre_clust;
	long long int fre_clust_temp, csize_temp;
	long long int free_size = 0;
	unsigned char  res;
	long long int sector_size = 0;

	sector_size = 512;

	if (fatfs_sd_init_done) {
		res = f_getfree(fatfs_sd_param.drv, &fre_clust, &pfs);
		if (res) {
			return res;
		} else {
			//TOT_SIZE = (pfs->n_fatent - 2) * pfs->csize/2; //
			fre_clust_temp = fre_clust;
			csize_temp     = pfs->csize;
			free_size = (fre_clust_temp * csize_temp) * sector_size; //*(sector_size); //
		}
	}

	return free_size;//Byte
}

long long int fatfs_get_used_space_byte(void)
{
	FATFS *pfs = NULL;
	DWORD fre_clust;
	long long int fre_clust_temp, csize_temp;
	long long int free_size = 0;
	unsigned char  res;
	long long int sector_size = 0;
	long long int tot_sect = 0;
	long long int used_size = -1;
	sector_size = 512;

	if (fatfs_sd_init_done) {
		res = f_getfree(fatfs_sd_param.drv, &fre_clust, &pfs);
		if (res) {
			printf("Error f_getfree res = %d\r\n", res);
			return used_size;
		} else {
			tot_sect = (pfs->n_fatent - 2) * pfs->csize; //
			tot_sect = tot_sect * sector_size;//Bytes
			fre_clust_temp = fre_clust;
			csize_temp     = pfs->csize;
			free_size = (fre_clust_temp * csize_temp) * sector_size;
			used_size = tot_sect - free_size;
		}
	} else {
		return used_size;
	}

	return used_size;//Byte
}

void fatfs_list_files(void)
{
	char buff[256] = {0};
	char	logical_drv[4] = {0}; /* root diretor */
	logical_drv[0] = fatfs_sd_param.drv_num + '0';
	logical_drv[1] = ':';
	logical_drv[2] = '/';
	logical_drv[3] = 0;
	if (fatfs_sd_init_done) {
		strcpy(buff, logical_drv);
		scan_files(buff);
	}
}

/*For usb operation.........................................*/
int usb_sd_init(void)
{
	SD_RESULT ret;
	ret = SD_Init();
	if (ret != SD_OK) {
		return 1;
	}
	return ret;
}
int usb_sd_deinit(void)
{
	SD_RESULT ret;
	ret = SD_DeInit();
	if (ret != SD_OK) {
		return 1;
	}
	return ret;
}
int usb_sd_getcapacity(uint32_t *sector_count)
{
	SD_RESULT ret;
	ret = SD_GetCapacity(sector_count);
	if (ret != SD_OK) {
		return 1;
	}
	return 0;
}
int usb_sd_readblocks(uint32_t sector, uint8_t *data, uint32_t count)
{
	SD_RESULT ret;
	ret = SD_ReadBlocks(sector, data, count);
	if (ret != SD_OK) {
		return 1;
	}
	return 0;
}
int usb_sd_writeblocks(uint32_t sector, const uint8_t *data, uint32_t count)
{
	SD_RESULT ret;
	ret = SD_WriteBlocks(sector, data, count);
	if (ret != SD_OK) {
		return 1;
	}
	return 0;
}
/*For usb operation.........................................*/
#endif //FATFS_DISK_SD