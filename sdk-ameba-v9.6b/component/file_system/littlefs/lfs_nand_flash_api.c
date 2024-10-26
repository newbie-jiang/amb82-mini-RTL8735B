#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <cmsis.h>
#include "FreeRTOS.h"
#include "task.h"
#include "basic_types.h"
#include "section_config.h"
#include "lfs.h"
#include "lfs_util.h"
#include "snand_api.h"
#include "ftl_common_api.h"
#include "lfs_reent.h"

#define NAND_FLASH_BASE 0X200
#define NAND_FLASH_BLOCK_PAGE_SIZE 64
#define NAND_PAGE_SIZE 2048
#define NAND_SPARE_SIZE 0X08
#define NAND_FLASH_BLOCK_SIZE (NAND_PAGE_SIZE*NAND_FLASH_BLOCK_PAGE_SIZE)
#define NAND_FLASH_BLOCK_COUNT 100

static int nand_block_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	int ret = 0;
	ret = ftl_common_read((NAND_FLASH_BASE + block) * c->block_size + off, buffer, size);
}
static int nand_block_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	int ret = 0;
	ret = ftl_common_write((NAND_FLASH_BASE + block) * c->block_size + off, buffer, size);
	if (ret == 0) {
		ret = LFS_ERR_OK;
	} else {
		ret = LFS_ERR_IO;
	}
}
static int nand_block_erase(const struct lfs_config *c, lfs_block_t block)
{
	int ret = 0;
	ret = ftl_common_erase((NAND_FLASH_BASE + block) * c->block_size);
	if (ret == 0) {
		ret = LFS_ERR_OK;
	} else {
		ret = LFS_ERR_IO;
	}
}
static int nand_block_sync(const struct lfs_config *c)
{
	return 0;
}


struct lfs_config nand_cfg = {
	// block device operations
	.read  = nand_block_read,
	.prog  = nand_block_prog,
	.erase = nand_block_erase,
	.sync  = nand_block_sync,
	.lock  =  lfs_system_lock,
	.unlock = lfs_system_unlock,

	// block device configuration
	.read_size = NAND_PAGE_SIZE,
	.prog_size = NAND_PAGE_SIZE,
	.block_size = NAND_FLASH_BLOCK_SIZE,
	.block_count = NAND_FLASH_BLOCK_COUNT,
	.cache_size = NAND_PAGE_SIZE,
	.block_cycles = 100,
	.lookahead_size = NAND_PAGE_SIZE
	//.lookahead_buffer = 256,
};