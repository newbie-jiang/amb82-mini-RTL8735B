<<< <<< < HEAD(5042f5 [amebapro2][isp] fix the issue of disable privacy mask)
== == == =
	/**************************************************************************//**
	 * @file     rtl8735b_wdt.c
	 * @brief    Implement flash controller ROM code functions.
	 * @version  1.00
	 * @date     2020-08-26
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
#include "hal_wdt.h"
#include "hal_wdt_nsc.h"


/// @cond DOXYGEN_ROM_HAL_API

#if !defined(CONFIG_BUILD_SECURE) && !defined(CONFIG_BUILD_NONSECURE)
#undef SECTION_NS_ENTRY_FUNC
#undef NS_ENTRY
#define SECTION_NS_ENTRY_FUNC
#define NS_ENTRY
#endif


	/**
	 *  @brief Changes the watch dog timer timeout period.
	 *
	 *  @param[in]  time_us  The timeout period in micro-second.
	 *
	 *  @returns    void
	 */
	SECTION_NS_ENTRY_FUNC
	void NS_ENTRY hal_wdt_set_timeout_nsc(uint32_t time_us)
{
	hal_wdt_set_timeout(time_us);
}

/**
 *  @brief Initials the watch dog timer and setup the timeout period.
 *         The system will be reset by the watch dog timeout event by default.
 *
 *  @param[in]  time_us  The timeout period in micro-second.
 *
 *  @returns    void
 */
SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_init_nsc(uint32_t time_us)
{
	hal_wdt_init(time_us);
}

/**
 *  @brief Enables the watch dog timer.
 *
 *  @returns    void
 */
SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_enable_nsc(void)
{
	hal_wdt_enable();
}

/**
 *  @brief Disables the watch dog timer.
 *
 *  @returns    void
 */
SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_disable_nsc(void)
{
	hal_wdt_disable();
}

/**
 *  @brief Refresh(reload) the watch dog timer counter.
 *         To prevents the watch dog timer timeout event occurred.
 *
 *  @returns    void
 */
SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_refresh_nsc(void)
{
	hal_wdt_refresh();
}

/**
 *  @brief Registers a handler for the watch dog timeout interrupt.
 *         The WDT timeout interrupt will trigger the NMI interrupt.
 *
 *  @param[in]  handler  The interrupt handler.
 *  @param[in]  arg  The application data will be passed back to the application
 *                   with the callback function.
 *
 *  @returns    void
 */
SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_reg_irq_nsc(irq_handler_t handler, void *arg)
{
	hal_wdt_reg_irq_s4ns(handler);
	hal_wdt_reg_irq(hal_wdt_s2ns_handler_call, arg);
}

/**
 *  @brief Un-Registers a handler for the watch dog timeout interrupt.
 *
 *
 *  @returns    void
 */
SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_unreg_irq_nsc(void)
{
	hal_wdt_unreg_irq();
}

/**
 *  @brief Refresh(reload) the watch dog timer counter.
 *         To prevents the watch dog timer timeout event occurred.
 *
 *  @param[in]  reset :
 *                - BIT 0: 1: Mask WDOG_RST to reset AON  block.
 *                - BIT 1: 1: Mask WDOG_RST to reset PON  block
 *                - BIT 2: 1: Mask WDOG_RST to reset WLON  block
 *                - BIT 3: 0: Mask WDOG_RST to trigger CPU wram reset
 *                            1: Mask WDOG_RST to trigger CPU cold reset
 *                - BIT 4: 1: Mask WDOG_RST to reset SYSON  block
 *  @returns    void
 */
SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_reset_nsc(u8 reset)
{
	hal_wdt_reset(reset);
}

SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_aon_enable_nsc(void)
{
	hal_wdt_aon_enable();
}

SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_aon_disable_nsc(void)
{
	hal_wdt_aon_disable();
}

SECTION_NS_ENTRY_FUNC
uint8_t NS_ENTRY hal_wdt_check_wdt_aon_reboot_nsc(void)
{
	return hal_wdt_check_wdt_aon_reboot();
}

SECTION_NS_ENTRY_FUNC
void NS_ENTRY hal_wdt_aon_clear_nsc(void)
{
	hal_wdt_aon_clear();
}

/** *@} */ /* End of group hal_wdt_rom_func */

/// @endcond /* End of condition DOXYGEN_ROM_HAL_API */

/** *@} */

>>> >>> > CHANGE(7d04ec [amebapro2][wdt] Fix watchdog interrupt mode does not bark i)
