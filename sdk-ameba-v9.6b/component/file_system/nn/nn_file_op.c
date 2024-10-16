//--------------------------------------------------------------------------------------
// VIPNN file usage
//--------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nn_file_op.h"
#include "vfs.h"

#define MODEL_FROM_FLASH    0x01
#define MODEL_FROM_SD       0x02
#define MODEL_SRC           MODEL_FROM_FLASH

void *nn_f_open(char *name, int mode)
{
#if MODEL_SRC==MODEL_FROM_FLASH
	return pfw_open(name, M_NORMAL);
#elif MODEL_SRC==MODEL_FROM_SD
	vfs_init(NULL);
	vfs_user_register("sd", VFS_FATFS, VFS_INF_SD);
	char model_name[64];
	memset(model_name, 0, sizeof(model_name));
	snprintf(model_name, sizeof(model_name), "%s%s", "sd:/", name);
	return (void *)fopen(model_name, "r+");
#endif
}

void nn_f_close(void *fr)
{
#if MODEL_SRC==MODEL_FROM_FLASH
	pfw_close(fr);
#elif MODEL_SRC==MODEL_FROM_SD
	fclose((FILE *)fr);
#endif
}

int nn_f_read(void *fr, void *data, int size)
{
#if MODEL_SRC==MODEL_FROM_FLASH
	return pfw_read(fr, data, size);
#elif MODEL_SRC==MODEL_FROM_SD
	return fread(data, size, 1, (FILE *)fr);
#endif
}

int nn_f_seek(void *fr, int offset, int pos)
{
#if MODEL_SRC==MODEL_FROM_FLASH
	return pfw_seek(fr, offset, pos);
#elif MODEL_SRC==MODEL_FROM_SD
	return fseek((FILE *)fr, offset, pos);
#endif
}

int nn_f_tell(void *fr)
{
#if MODEL_SRC==MODEL_FROM_FLASH
	return pfw_tell(fr);
#elif MODEL_SRC==MODEL_FROM_SD
	return ftell((FILE *)fr);
#endif
}