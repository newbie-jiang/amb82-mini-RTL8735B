#include <cmsis.h>
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include "platform_opts.h"
#include "section_config.h"
#include "flash_api.h" // Flash interface
#include "lfs.h"
#include "lfs_util.h"
#include "ftl_common_api.h"
#include "lfs_reent.h"

#define NOR_BLOCK_OFFSET 1088
#define NOR_PAGE_SIZE       4096 //SECTOR SIZE
#define NOR_BLOCK_SIZE      4096 //SECTOR SIZE
#define NOR_BLOCK_COUNT     100 //SECTOR SIZE
#define FLASH_APP_BASE  (NOR_BLOCK_OFFSET*NOR_BLOCK_SIZE)//0x440000

int nor_block_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	int ret = 0;
	ret = ftl_common_read(block * c->block_size + FLASH_APP_BASE + off, buffer, size);
	return 0;
}

int nor_block_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	int ret = 0;
	ret = ftl_common_write(block * c->block_size + FLASH_APP_BASE + off, buffer, size);
	return 0;
}
int nor_block_erase(const struct lfs_config *c, lfs_block_t block)
{
	int ret = 0;
	ret = ftl_common_erase(block * c->block_size + FLASH_APP_BASE);
	return 0;
}
int nor_block_sync(const struct lfs_config *c)
{
	return 0;
}

struct lfs_config nor_cfg = {
	// block device operations
	.read  = nor_block_read,
	.prog  = nor_block_prog,
	.erase = nor_block_erase,
	.sync  = nor_block_sync,
	.lock  =  lfs_system_lock,
	.unlock = lfs_system_unlock,

	// block device configuration
	.read_size = NOR_PAGE_SIZE,
	.prog_size = NOR_PAGE_SIZE,
	.block_size = NOR_BLOCK_SIZE,
	.block_count = NOR_BLOCK_COUNT,
	.cache_size = NOR_PAGE_SIZE,
	.block_cycles = 100,
	.lookahead_size = NOR_PAGE_SIZE
	//.lookahead_buffer = 256,
};