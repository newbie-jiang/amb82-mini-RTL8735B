/**************************************************************************//**
 * @file     hal_trng_sec.c
 * @brief    Implement TRNG IP (SEC) RAM code functions.
 *
 * @version  V1.00
 * @date     2021-12-23
 *
 * @note
 *
 ******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#include "cmsis.h"
#include "hal_trng_sec.h"


#if defined(CONFIG_BUILD_NONSECURE)

#include "hal_trng_sec_nsc.h"

#endif

// TODO add doxygen
/**
  \brief todo
*/
#if CONFIG_TRNG_EN

hal_trng_sec_adapter_t hal_trng_sec_adtr;

extern const hal_trng_sec_func_stubs_t hal_trng_sec_stubs;

hal_status_t hal_trng_sec_init(void)
{
	hal_status_t ret = HAL_OK;
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		if (ENABLE == phal_trng_sec_adtr->isInit) {
			ret = HAL_OK;
			return ret;
		}
		ret = hal_trng_sec_stubs.hal_trng_sec_init(phal_trng_sec_adtr);
	} else {
		ret = hal_rtl_trng_sec_init_patch(phal_trng_sec_adtr);
	}
	return ret;
}

hal_status_t hal_trng_sec_deinit(void)
{
	hal_status_t ret = HAL_OK;
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		if (ENABLE != phal_trng_sec_adtr->isInit) {
			ret = _ERRNO_TRNG_SEC_ENGINE_NOT_INIT;
			return ret;
		}
		ret = hal_trng_sec_stubs.hal_trng_sec_deinit(phal_trng_sec_adtr);
	} else {
		ret = hal_rtl_trng_sec_deinit_patch(phal_trng_sec_adtr);
	}
	return ret;
}

hal_status_t hal_trng_sec_set_clk(uint8_t sel_val)
{
	hal_status_t ret = HAL_OK;
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		ret = hal_trng_sec_stubs.hal_trng_sec_set_clk(phal_trng_sec_adtr, sel_val);
	} else {
		ret = hal_rtl_trng_sec_set_clk_patch(phal_trng_sec_adtr, sel_val);
	}
	return ret;
}

hal_status_t hal_trng_sec_swrst_en(void)
{
	hal_status_t ret = HAL_OK;
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		ret = hal_trng_sec_stubs.hal_trng_sec_swrst_en(phal_trng_sec_adtr);
	} else {
		ret = hal_rtl_trng_sec_swrst_en_patch(phal_trng_sec_adtr);
	}
	return ret;
}

uint32_t hal_trng_sec_get_rand(void)
{
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint32_t rng_v;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		if (ENABLE != phal_trng_sec_adtr->isInit) {
			rng_v = 0x0;
			return rng_v;
		}
		rng_v = hal_trng_sec_stubs.hal_trng_sec_get_rand(phal_trng_sec_adtr);
	} else {
		rng_v = hal_rtl_trng_sec_get_rand_patch(phal_trng_sec_adtr);
	}
	return rng_v;
}

hal_status_t hal_trng_sec_set_normal_ctrl(uint8_t rng_mode, uint8_t rbc_sel, uint8_t hspeed_sel)
{
	hal_status_t ret = HAL_OK;
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		if (ENABLE != (phal_trng_sec_adtr->isInit)) {
			ret = _ERRNO_TRNG_SEC_ENGINE_NOT_INIT;
			return ret;
		}
		ret = hal_trng_sec_stubs.hal_trng_sec_set_normal_ctrl(phal_trng_sec_adtr, rng_mode, rbc_sel, hspeed_sel);
	} else {
		ret = hal_rtl_trng_sec_set_normal_ctrl_patch(phal_trng_sec_adtr, rng_mode,  rbc_sel, hspeed_sel);
	}
	return ret;
}

hal_status_t hal_trng_sec_set_lfsr_ctrl(uint8_t lfsr_mod, uint32_t poly_lsb, uint32_t poly_msb)
{
	hal_status_t ret = HAL_OK;
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		if (ENABLE != (phal_trng_sec_adtr->isInit)) {
			ret = _ERRNO_TRNG_SEC_ENGINE_NOT_INIT;
			return ret;
		}
		ret = hal_trng_sec_stubs.hal_trng_sec_set_lfsr_ctrl(phal_trng_sec_adtr, lfsr_mod, poly_lsb, poly_msb);
	} else {
		ret =  hal_rtl_trng_sec_set_lfsr_ctrl_patch(phal_trng_sec_adtr, lfsr_mod,  poly_lsb, poly_msb);
	}
	return ret;
}

hal_status_t hal_trng_sec_set_selft_ctrl(uint8_t selft_en, uint8_t cmp_rep_mode, uint8_t cmp_adpt_mode, uint8_t adpt1_window_sel, uint8_t adpt2_window_sel)
{
	hal_status_t ret = HAL_OK;
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		if (ENABLE != (phal_trng_sec_adtr->isInit)) {
			ret = _ERRNO_TRNG_SEC_ENGINE_NOT_INIT;
			return ret;
		}
		ret = hal_trng_sec_stubs.hal_trng_sec_set_selft_ctrl(phal_trng_sec_adtr, selft_en, cmp_rep_mode, cmp_adpt_mode, adpt1_window_sel, adpt2_window_sel);
	} else {
		ret =  hal_rtl_trng_sec_set_selft_ctrl_patch(phal_trng_sec_adtr, selft_en, cmp_rep_mode, cmp_adpt_mode, adpt1_window_sel, adpt2_window_sel);

	}
	return ret;
}

hal_status_t hal_trng_sec_load_default_setting(uint8_t selft_en)
{
	hal_status_t ret = HAL_OK;
	hal_trng_sec_adapter_t *phal_trng_sec_adtr = &hal_trng_sec_adtr;
	uint8_t v_main = 0;
	v_main =  hal_sys_get_rom_ver();
	if (IS_AFTER_CUT_C(v_main)) {
		if (ENABLE != (phal_trng_sec_adtr->isInit)) {
			ret = _ERRNO_TRNG_SEC_ENGINE_NOT_INIT;
			return ret;
		}
		ret = hal_trng_sec_stubs.hal_trng_sec_load_default_setting(phal_trng_sec_adtr, selft_en);
	} else {
		ret =  hal_rtl_trng_sec_load_default_setting_patch(phal_trng_sec_adtr, selft_en);
	}
	return ret;
}
#endif
