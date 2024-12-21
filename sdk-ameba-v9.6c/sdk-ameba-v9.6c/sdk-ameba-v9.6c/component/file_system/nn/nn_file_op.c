//--------------------------------------------------------------------------------------
// VIPNN file usage
//--------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nn_file_op.h"
#include "vfs.h"

#define MODEL_FROM_FLASH    0x01
#define MODEL_FROM_SD       0x02
#define MODEL_SRC           MODEL_FROM_FLASH

//----- AES NN Model decryption --------------------------------------------------------
#include "crypto_api.h"
#include "hal_crypto.h"
#include "device_lock.h"

typedef enum aes_mode {
	AES_MODE_ECB = 0,
	AES_MODE_CBC = 1
} aes_mode_t;

/**** user configuration ****/
#define MDL_ENC_SIZE    512  // should be 16bytes-aligned, user should set how many bytes of model be encrypted from head
static uint8_t AESkey[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};  // user should set their key here
static uint8_t AESkeylen = 0;  // [16] user should set their key length here
static uint8_t AESiv[16] = {0x1A, 0x1B, 0x1C, 0x1D, 0xA6, 0xB6, 0xC6, 0xD6, 0xAA, 0xBB, 0xCC, 0xDD, 0xFA, 0xFB, 0xFC, 0xFD};  // user should set the IV here
static aes_mode_t AESmode = AES_MODE_CBC;  // user should choose mode here: AES_MODE_ECB, AES_MODE_CBC
/**** user configuration ****/

typedef struct nn_aes_ctx {
	int nr;                     // The number of rounds.
	uint32_t *rk;               // AES round keys.
	uint32_t buf[68];           // Unaligned data buffer.
	uint8_t *pDecBuf;           // Decrypt data buffer.
	aes_mode_t mode;            // AES mode
} nn_aes_ctx_t;

static nn_aes_ctx_t *pAes_ctx = NULL;

static bool nn_keylen_valid(void)
{
	if (AESkeylen != 16 && AESkeylen != 24 && AESkeylen != 32) {
		return 0;
	}
	return 1;
}

static int nn_aes_set_key(nn_aes_ctx_t *ctx, uint8_t *key, int keybits)
{
	int keyBytes = 1;

	switch (keybits) {
	case 128:
		ctx->nr = 10;
		keyBytes = 128 / 8;
		break;
	case 192:
		ctx->nr = 12;
		keyBytes = 192 / 8;
		break;
	case 256:
		ctx->nr = 14;
		keyBytes = 256 / 8;
		break;
	default :
		return -1;
	}

	ctx->rk = ctx->buf;
	memcpy(ctx->rk, key, keyBytes);

	return 0;
}

static int nn_aes_set_mode(nn_aes_ctx_t *ctx, aes_mode_t mode)
{
	ctx->mode = mode;

	return 0;
}

static int nn_aes_crypt_ecb_decrypt(nn_aes_ctx_t *ctx, size_t length, const uint8_t *input, uint8_t *output)
{
	if (length % 16) {
		printf("[NN decrypt] invalid input length for ecb decrypt\r\n");
		return -1;
	}

	if (length > 0) {
		uint8_t key_buf[32 + 32 + 32], *key_buf_aligned;
		key_buf_aligned = (uint8_t *)(((uint32_t) key_buf + 32) / 32 * 32);
		memcpy(key_buf_aligned, ctx->rk, ((ctx->nr - 6) * 4));
		size_t length_done = 0;

		device_mutex_lock(RT_DEV_LOCK_CRYPTO);
		crypto_aes_ecb_init(key_buf_aligned, ((ctx->nr - 6) * 4));
		while ((length - length_done) > CRYPTO_MAX_MSG_LENGTH) {
			crypto_aes_ecb_decrypt(input + length_done, CRYPTO_MAX_MSG_LENGTH, NULL, 0, output + length_done);
			length_done += CRYPTO_MAX_MSG_LENGTH;
		}
		crypto_aes_ecb_decrypt(input + length_done, length - length_done, NULL, 0, output + length_done);
		device_mutex_unlock(RT_DEV_LOCK_CRYPTO);
	}

	return 0;
}

static int nn_aes_crypt_cbc_decrypt(nn_aes_ctx_t *ctx, size_t length, uint8_t iv[16], const uint8_t *input, uint8_t *output)
{
	if (length % 16) {
		printf("[NN decrypt] invalid input length for cbc decrypt\r\n");
		return -1;
	}

	if (length > 0) {
		uint8_t key_buf[32 + 32 + 32], *key_buf_aligned;
		uint8_t iv_buf[32 + 16 + 32], *iv_buf_aligned, iv_tmp[16];
		size_t length_done = 0;

		key_buf_aligned = (uint8_t *)(((unsigned int) key_buf + 32) / 32 * 32);
		memcpy(key_buf_aligned, ctx->rk, ((ctx->nr - 6) * 4));
		iv_buf_aligned = (uint8_t *)(((unsigned int) iv_buf + 32) / 32 * 32);
		memcpy(iv_buf_aligned, iv, 16);

		device_mutex_lock(RT_DEV_LOCK_CRYPTO);
		crypto_aes_cbc_init(key_buf_aligned, ((ctx->nr - 6) * 4));
		while ((length - length_done) > CRYPTO_MAX_MSG_LENGTH) {
			memcpy(iv_tmp, (input + length_done + CRYPTO_MAX_MSG_LENGTH - 16), 16);
			crypto_aes_cbc_decrypt(input + length_done, CRYPTO_MAX_MSG_LENGTH, iv_buf_aligned, 16, output + length_done);
			memcpy(iv_buf_aligned, iv_tmp, 16);
			length_done += CRYPTO_MAX_MSG_LENGTH;
		}
		memcpy(iv_tmp, (input + length - 16), 16);
		crypto_aes_cbc_decrypt(input + length_done, length - length_done, iv_buf_aligned, 16, output + length_done);
		memcpy(iv, iv_tmp, 16);
		device_mutex_unlock(RT_DEV_LOCK_CRYPTO);
	}

	return 0;
}

static int nn_model_decrypt(const uint8_t *input, uint8_t *output)
{
	int ret = -1;
	if (pAes_ctx->mode == AES_MODE_ECB) {
		printf("[NN decrypt] decrypt model in ECB mode\r\n");
		ret = nn_aes_crypt_ecb_decrypt(pAes_ctx, MDL_ENC_SIZE, input, output);
	} else if (pAes_ctx->mode == AES_MODE_CBC) {
		printf("[NN decrypt] decrypt model in CBC mode\r\n");
		ret = nn_aes_crypt_cbc_decrypt(pAes_ctx, MDL_ENC_SIZE, AESiv, input, output);
	} else {
		printf("[NN decrypt] unsupported AES mode\r\n");
	}
	return ret;
}

static uint8_t crypto_inited = 0;

static int is_vfs_path(void *fr);
static void nn_aes_deinit(void);

static int nn_aes_init(void *fp)
{
	printf("[NN decrypt] fisrt %ld bytes of model are encrypted.\r\n", MDL_ENC_SIZE);

	int ret = 0;

	if (!crypto_inited) {
		device_mutex_lock(RT_DEV_LOCK_CRYPTO);
		crypto_init();
		device_mutex_unlock(RT_DEV_LOCK_CRYPTO);
		crypto_inited = 1;
	}

	pAes_ctx = (nn_aes_ctx_t *)malloc(sizeof(nn_aes_ctx_t));
	if (!pAes_ctx) {
		printf("[NN decrypt] fail to allocate memory for pAes_ctx\r\n");
		goto error;
	}
	memset(pAes_ctx, 0, sizeof(nn_aes_ctx_t));

	pAes_ctx->pDecBuf = (uint8_t *)malloc(MDL_ENC_SIZE);
	if (!pAes_ctx->pDecBuf) {
		printf("[NN decrypt] fail to allocate memory for pAes_ctx->pDecBuf\r\n");
		goto error;
	}

	ret = nn_aes_set_mode(pAes_ctx, AESmode);
	if (ret != 0) {
		printf("[NN decrypt] nn_aes_set_mode fail\r\n");
		goto error;
	}

	ret = nn_aes_set_key(pAes_ctx, AESkey, AESkeylen * 8);
	if (ret != 0) {
		printf("[NN decrypt] nn_aes_set_key fail\r\n");
		goto error;
	}

	nn_f_seek((FILE *)fp, 0, SEEK_SET);
	ret = nn_f_read(fp, pAes_ctx->pDecBuf, MDL_ENC_SIZE);
	if (ret < 0) {
		printf("[NN decrypt] nn_f_read fail\r\n");
		goto error;
	}
	//pfw_dump_mem((uint8_t *)pAes_ctx->pDecBuf, 128);

	uint32_t t0 = xTaskGetTickCount();
	ret = nn_model_decrypt(pAes_ctx->pDecBuf, pAes_ctx->pDecBuf);
	if (ret != 0) {
		printf("[NN decrypt] nn_model_decrypt fail\r\n");
		goto error;
	}
	printf("[NN decrypt] decrypt model done, take %ld ms.\r\n", xTaskGetTickCount() - t0);
	//pfw_dump_mem((uint8_t *)pAes_ctx->pDecBuf, 128);
	nn_f_seek((FILE *)fp, 0, SEEK_SET);

	return 0;

error:
	nn_aes_deinit();
	return -1;
}

static void nn_aes_deinit(void)
{
	if (pAes_ctx) {
		if (pAes_ctx->pDecBuf) {
			free(pAes_ctx->pDecBuf);
		}
		free(pAes_ctx);
	}
}

//--------------------------------------------------------------------------------------

void *nn_f_open(char *name, int mode)
{
	void *fp = NULL;

	if (strstr(name, ":/")) {
		fp = (void *)fopen(name, "r");
	} else {
#if MODEL_SRC==MODEL_FROM_FLASH
		fp = pfw_open(name, M_NORMAL);
#elif MODEL_SRC==MODEL_FROM_SD
		vfs_init(NULL);
		vfs_user_register("sd", VFS_FATFS, VFS_INF_SD);
		char model_name[64];
		memset(model_name, 0, sizeof(model_name));
		snprintf(model_name, sizeof(model_name), "%s%s", "sd:/", name);
		fp = (void *)fopen(model_name, "r");
#endif
	}
	if (fp == NULL) {
		printf("[NN decrypt] nn open fail\r\n");
		goto error;
	}

	if (nn_keylen_valid()) {
		if (nn_aes_init(fp) != 0) {
			printf("[NN decrypt] fail to init nn AES\r\n");
			goto error;
		}
	}

	return fp;

error:
	if (fp) {
		nn_f_close(fp);
	}
	return NULL;
}

static int is_vfs_path(void *fr)
{
	vfs_file *fp = (vfs_file *)fr;
	/* If "fr" is a file descriptor of fwfs, it doesn't have "name" member
	 * In case there is no terminating null-character in "name" after type casting, use strnstr to check first 16 bytes */
	if (strnstr(fp->name, ":/", 16)) {
		return 1;
	} else {
		return 0;
	}
}

void nn_f_close(void *fr)
{
	if (is_vfs_path(fr)) {
		fclose((FILE *)fr);
	} else {
#if MODEL_SRC==MODEL_FROM_FLASH
		pfw_close(fr);
#elif MODEL_SRC==MODEL_FROM_SD
		fclose((FILE *)fr);
#endif
	}

	if (nn_keylen_valid()) {
		nn_aes_deinit();
	}
}

int nn_f_read(void *fr, void *data, int size)
{
	int crr = nn_f_tell(fr);
	int ret_size = 0;

	if (is_vfs_path(fr)) {
		ret_size = fread(data, size, 1, (FILE *)fr);
	} else {
#if MODEL_SRC==MODEL_FROM_FLASH
		ret_size = pfw_read(fr, data, size);
#elif MODEL_SRC==MODEL_FROM_SD
		ret_size = fread(data, size, 1, (FILE *)fr);
#endif
	}

	if (nn_keylen_valid()) {
		if (crr < MDL_ENC_SIZE) {
			int res_size = (crr + size) > MDL_ENC_SIZE ? MDL_ENC_SIZE - crr : size;
			memcpy(data, pAes_ctx->pDecBuf + crr, res_size);
		}
	}

	return ret_size;
}

int nn_f_seek(void *fr, int offset, int pos)
{
	int ret = 0;
	if (is_vfs_path(fr)) {
		ret = fseek((FILE *)fr, offset, pos);
	} else {
#if MODEL_SRC==MODEL_FROM_FLASH
		ret = pfw_seek(fr, offset, pos);
#elif MODEL_SRC==MODEL_FROM_SD
		ret = fseek((FILE *)fr, offset, pos);
#endif
	}
	return ret;
}

int nn_f_tell(void *fr)
{
	int ret = 0;
	if (is_vfs_path(fr)) {
		ret = ftell((FILE *)fr);
	} else {
#if MODEL_SRC==MODEL_FROM_FLASH
		ret = pfw_tell(fr);
#elif MODEL_SRC==MODEL_FROM_SD
		ret = ftell((FILE *)fr);
#endif
	}
	return ret;
}