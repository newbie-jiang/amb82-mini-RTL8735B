#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include "log_service.h"
#include "osdep_service.h"
#include <platform_opts.h>
#include <platform_opts_bt.h>
#include "sys_api.h"

#include "wifi_conf.h"
#include <lwip_netconf.h>
#include <lwip/sockets.h>

#include "power_mode_api.h"
#include "time64.h"

#define STACKSIZE     2048
#define WOWLAN_GPIO_WDT      1
//Clock, 1: 4MHz, 0: 100kHz
#define CLOCK 0
//SLEEP_DURATION, 120s
#define SLEEP_DURATION (120 * 1000 * 1000)

extern uint8_t rtl8735b_wowlan_wake_reason(void);
extern uint8_t rtl8735b_wowlan_wake_pattern(void);
extern uint8_t *rtl8735b_read_wakeup_packet(uint32_t *size, uint8_t wowlan_reason);
extern int rtl8735b_suspend(int mode);
extern void rtl8735b_set_lps_pg(void);
extern void rtl8735b_set_lps_dtim(uint8_t dtim);
extern void wifi_wowlan_bcntrack_stage1(uint8_t rx_bcn_window, uint8_t bcn_limit);
extern void wifi_wowlan_set_rxbcnlimit(uint8_t set_rxbcnlimit);
extern void wifi_wowlan_set_pstimeout(uint8_t set_pstimeout);
extern void wifi_wowlan_set_psretry(uint8_t set_psretry);
extern void rtw_hal_set_unicast_wakeup(u8 enable);

static uint8_t wowlan_wake_reason = 0;
static uint8_t wlan_resume = 0;
static uint8_t tcp_syn_wakeup = 0;
//dtim
static uint8_t lps_dtim = 10;

//stage1 setting
static uint8_t rx_bcn_window = 30;
static uint8_t bcn_limit = 5;

//stage2 setting
static uint8_t  stage2_start_window = 10;
static uint8_t  stage2_max_window = 210;
static uint8_t  stage2_increment_steps = 40;
static uint8_t  stage2_duration = 10;

static uint8_t set_rxbcnlimit = 8;
static uint8_t set_pstimeout = 16;
static uint8_t set_psretry = 7;
static uint8_t unicast_wakeup_enable = 1;

static uint16_t server_local_port = 0;
static uint16_t server_local_port0 = 4000;
static uint16_t server_local_port1 = 554;
__attribute__((section(".retention.data"))) uint32_t retention_local_ip __attribute__((aligned(32))) = 0;

static uint8_t goto_sleep = 0;

#if CONFIG_WLAN
#include <wifi_fast_connect.h>
extern void wlan_network(void);
#endif

#include "gpio_api.h"
#include "gpio_irq_api.h"
#include "gpio_irq_ex_api.h"

#define WAKUPE_GPIO_PIN PA_2
static gpio_irq_t my_GPIO_IRQ;
void gpio_demo_irq_handler(uint32_t id, gpio_irq_event event)
{
	dbg_printf("%s==> \r\n", __FUNCTION__);
}

extern void console_init(void);

void tcp_app_task(void *param)
{
	while (!goto_sleep) {
		if (tcp_syn_wakeup) {
			break;
		}

		vTaskDelay(2000);
		u32 value32;
		value32 = HAL_READ32(0x40080000, 0x54);
		value32 = value32 & 0xFF;
		printf("beacon recv per 20 = %d\r\n", value32);
	}

#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
	rtw_create_secure_context(configMINIMAL_SECURE_STACK_SIZE);
#endif

	// wait for IP address
	while (!((wifi_get_join_status() == RTW_JOINSTATUS_SUCCESS) && (*(u32 *)LwIP_GetIP(0) != IP_ADDR_INVALID))) {
		vTaskDelay(10);
	}

	if (tcp_syn_wakeup) {
		int server_fd = -1;

		// server socket
		server_fd = socket(AF_INET, SOCK_STREAM, 0);
		printf("\n\r socket(%d) \n\r", server_fd);

		// bind server port
		struct sockaddr_in local_addr;
		local_addr.sin_family = AF_INET;
		local_addr.sin_addr.s_addr = INADDR_ANY;
		local_addr.sin_port = htons(server_local_port);
		printf("bind server local port:%d %s \n\r", server_local_port, bind(server_fd, (struct sockaddr *) &local_addr, sizeof(local_addr)) == 0 ? "OK" : "FAIL");

		printf("listen %s \n\r", listen(server_fd, 3) == 0 ? "OK" : "FAIL");

		// fill wakeup packet
		extern int lwip_fill_wakeup_packet(void);
		lwip_fill_wakeup_packet();

		// wait for input
		while (1) {
			struct timeval timeout;
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
			fd_set read_fds;
			FD_ZERO(&read_fds);
			FD_SET(server_fd, &read_fds);

			if (select(server_fd + 1, &read_fds, NULL, NULL, &timeout)) {
				if (FD_ISSET(server_fd, &read_fds)) {
					struct sockaddr_in client_addr;
					unsigned int client_addr_size = sizeof(client_addr);
					int client_fd = -1;

					if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size)) >= 0) {
						printf("accept client_fd %d \n\r", client_fd);

						int ret = 0;
						uint8_t buf[16];
						while (1) {
							ret = read(client_fd, buf, sizeof(buf));
							printf("read application data %d \n\r", ret);

							if (ret < 0) {
								close(client_fd);
								break;
							}
						}
					} else {
						printf("ERROR: accept \n\r");
						break;
					}
				} else {
					printf("ERROR: select \n\r");
					break;
				}
			} else if (goto_sleep) {
				break;
			}
		}

		if (server_fd >= 0) {
			close(server_fd);
		}
	}

	retention_local_ip = *(uint32_t *) LwIP_GetIP(0);
	dcache_clean_invalidate_by_addr((uint32_t *) &retention_local_ip, sizeof(retention_local_ip));
	printf("retain local ip: %08x \n\r", retention_local_ip);


	//set tbtt interval
	//static uint8_t lps_dtim = 10;
	rtl8735b_set_lps_dtim(lps_dtim);

	//set bcn track stage2 parmeters
	//static uint8_t rx_bcn_window = 30;
	//static uint8_t bcn_limit = 5;
	wifi_wowlan_bcntrack_stage1(rx_bcn_window, bcn_limit);

	//set bcn track stage2 parmeters
	// static uint8_t  stage2_start_window = 10;
	// static uint8_t  stage2_max_window = 210;
	// static uint8_t  stage2_increment_steps = 40;
	// static uint8_t  stage2_duration = 10;
	wifi_wowlan_set_bcn_track(stage2_start_window, stage2_max_window, stage2_increment_steps, stage2_duration);

	rtw_hal_set_unicast_wakeup(unicast_wakeup_enable);

	extern int dhcp_retain(void);
	printf("retain DHCP %s \n\r", dhcp_retain() == 0 ? "OK" : "FAIL");

#if WOWLAN_GPIO_WDT
	wifi_wowlan_set_wdt(2, 5, 1, 1); //gpiof_2, io trigger interval 1 min, io pull high and trigger pull low, pulse duration 1ms
#endif

	// for wlan resume
	extern int rtw_hal_wlan_resume_backup(void);
	rtw_hal_wlan_resume_backup();

	// sleep
	rtl8735b_set_lps_pg();
	rtw_enter_critical(NULL, NULL);
	if (rtl8735b_suspend(0) == 0) { // should stop wifi application before doing rtl8735b_suspend(
		if (((wifi_get_join_status() == RTW_JOINSTATUS_SUCCESS) && (*(u32 *)LwIP_GetIP(0) != IP_ADDR_INVALID))) {
			printf("rtl8735b_suspend\r\n");

			// wakeup GPIO
			gpio_irq_init(&my_GPIO_IRQ, WAKUPE_GPIO_PIN, gpio_demo_irq_handler, (uint32_t)&my_GPIO_IRQ);
			gpio_irq_pull_ctrl(&my_GPIO_IRQ, PullDown);
			gpio_irq_set(&my_GPIO_IRQ, IRQ_RISE, 1);
#if WOWLAN_GPIO_WDT

			//set gpio pull control
			HAL_WRITE32(0x40009850, 0x0, 0x4f004f); //GPIOF_1/GPIOF_0
			// HAL_WRITE32(0x40009854, 0x0, 0x8f004f); //GPIOF_3/GPIOF_2
			HAL_WRITE32(0x40009854, 0x0, 0x8f0000 | (0xffff & HAL_READ32(0x40009854, 0x0))); //GPIOF_3/GPIOF_2 keep wowlan wdt setting
			HAL_WRITE32(0x40009858, 0x0, 0x4f008f); //GPIOF_5/GPIOF_4
			HAL_WRITE32(0x4000985c, 0x0, 0x4f004f); //GPIOF_7/GPIOF_6
			HAL_WRITE32(0x40009860, 0x0, 0x4f004f); //GPIOF_9/GPIOF_8
			HAL_WRITE32(0x40009864, 0x0, 0x4f004f); //GPIOF_11/GPIOF_10
			HAL_WRITE32(0x40009868, 0x0, 0x4f004f); //GPIOF_13/GPIOF_12
			HAL_WRITE32(0x4000986C, 0x0, 0x4f004f); //GPIOF_15/GPIOF_14
			HAL_WRITE32(0x40009870, 0x0, 0x4f004f); //GPIOF_17/GPIOF_16
			rtw_exit_critical(NULL, NULL);
			Standby(DSTBY_AON_GPIO | DSTBY_PON_GPIO | DSTBY_WLAN | SLP_GTIMER, 0, 0, 1);
#else
			// standby with retention: add SLP_GTIMER and set SramOption to retention mode(1)
			rtw_exit_critical(NULL, NULL);
			Standby(DSTBY_AON_GPIO | DSTBY_WLAN | SLP_GTIMER, 0, 0, 1);
#endif
		} else {
			rtw_exit_critical(NULL, NULL);
			printf("wifi disconnect\r\n");
		}
	} else {
		rtw_exit_critical(NULL, NULL);
		printf("rtl8735b_suspend fail\r\n");
	}
	//rtw_exit_critical(NULL, NULL);

	while (1) {
		vTaskDelay(2000);
		printf("sleep fail!!!\r\n");
		sys_reset();
	}

	vTaskDelete(NULL);
}

int wlan_do_resume(void)
{
	extern int rtw_hal_wlan_resume_restore(void);
	rtw_hal_wlan_resume_restore();

	wifi_fast_connect_enable(1);
	wifi_fast_connect_load_fast_dhcp();

	extern uint8_t lwip_check_dhcp_resume(void);
	if (lwip_check_dhcp_resume() == 1) {
		extern int dhcp_resume(void);
		printf("\n\rresume DHCP %s\n\r", dhcp_resume() == 0 ? "OK" : "FAIL");
	} else {
		LwIP_DHCP(0, DHCP_START);
	}

	if (wowlan_wake_reason == 0x7A || wowlan_wake_reason == 0x7B) { // CSA
		if (p_store_fast_connect_info) {
			uint8_t *ip = LwIP_GetIP(0);
			uint8_t *sv = LwIP_GetDHCPSERVER(0);
			uint32_t offer_ip = *((u32_t *)ip);
			uint32_t server_ip = *((u32_t *)sv);
			printf("CSA new channel\n\r");
			p_store_fast_connect_info(offer_ip, server_ip); // store new channel in flash
		}
	}
	return 0;
}

void fPS(void *arg)
{
	int argc;
	char *argv[MAX_ARGC] = {0};

	argc = parse_param(arg, argv);

	if (strcmp(argv[1], "wowlan") == 0) {
		goto_sleep = 1;
	} else if (strcmp(argv[1], "dtim") == 0) {
		if (argc == 3 && atoi(argv[2]) >= 1 && atoi(argv[2]) <= 15) {
			printf("dtim=%02d", atoi(argv[2]));
			lps_dtim = atoi(argv[2]);
		}
	} else if (strcmp(argv[1], "stage1") == 0) {
		if (argc >= 4) {

			rx_bcn_window = atoi(argv[2]);
			bcn_limit = atoi(argv[3]);
		}
		printf("setup stage1 to %d:%d\r\n", atoi(argv[2]), atoi(argv[3]));
	} else if (strcmp(argv[1], "stage2") == 0) {
		if (argc >= 6) {
			stage2_start_window = atoi(argv[2]);
			stage2_max_window = atoi(argv[3]);
			stage2_increment_steps = atoi(argv[4]);
			stage2_duration = atoi(argv[5]);
		}
		printf("setup stage2 to %d:%d:%d:%d\r\n", atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	} else if (strcmp(argv[1], "standby") == 0) {
		printf("into standby mode %d second\r\n", (SLEEP_DURATION / 1000000));
		rtl8735b_set_lps_pg();
		rtl8735b_suspend(0);
		Standby(DSTBY_AON_TIMER, SLEEP_DURATION, CLOCK, 0);
	} else if (strcmp(argv[1], "rx_bcn_limit") == 0) {
		if (argc == 3) {
			printf("rx_bcn_limit=%02d", atoi(argv[2]));
			set_rxbcnlimit = atoi(argv[2]);
			wifi_wowlan_set_rxbcnlimit(set_rxbcnlimit);
		}
	} else if (strcmp(argv[1], "ps_timeout") == 0) {
		if (argc == 3) {
			printf("ps_timeout=%02d", atoi(argv[2]));
			set_pstimeout = atoi(argv[2]);
			wifi_wowlan_set_pstimeout(set_pstimeout);
		}
	} else if (strcmp(argv[1], "ps_retry") == 0) {
		if (argc == 3) {
			printf("ps_retry=%02d", atoi(argv[2]));
			set_psretry = atoi(argv[2]);
			wifi_wowlan_set_psretry(set_psretry);
		}
	}
}

log_item_t at_power_save_items[ ] = {
	{"PS", fPS,},
};

void main(void)
{
	uint32_t pm_reason = Get_wake_reason();
	printf("\n\rpm_reason=0x%x\n\r", pm_reason);

	hal_xtal_divider_enable(1);
	hal_32k_s1_sel(2);
	HAL_WRITE32(0x40009000, 0x18, 0x1 | HAL_READ32(0x40009000, 0x18)); //SWR 1.35V

	if (pm_reason) {
		uint32_t tcp_resume_seqno = 0, tcp_resume_ackno = 0;
		uint8_t by_wlan = 0, wlan_mcu_ok = 0;

		if (pm_reason & BIT(3)) {
			// WLAN wake up
			by_wlan = 1;

			/* *************************************
				RX_DISASSOC = 0x04,
				RX_DEAUTH = 0x08,
				FW_DECISION_DISCONNECT = 0x10,
				RX_PATTERN_PKT = 0x23,
				TX_TCP_SEND_LIMIT = 0x69,
				RX_DHCP_NAK = 0x6A,
				DHCP_RETRY_LIMIT = 0x6B,
				RX_MQTT_PATTERN_MATCH = 0x6C,
				RX_MQTT_PUBLISH_WAKE = 0x6D,
				RX_MQTT_MTU_LIMIT_PACKET = 0x6E,
				RX_TCP_FROM_SERVER_TO  = 0x6F,
				FW_BCN_TO_WAKEUP = 0x74,
				RX_TCP_RST_FIN_PKT = 0x75,
				RX_TCP_ALL_PKT = 0x77,
			*************************************** */

			wowlan_wake_reason = rtl8735b_wowlan_wake_reason();
			if (wowlan_wake_reason != 0) {
				printf("\r\nwake fom wlan: 0x%02X\r\n", wowlan_wake_reason);

				extern uint8_t *read_rf_conuter_report(void);
				read_rf_conuter_report();
				if (wowlan_wake_reason == 0x22) {
					wlan_mcu_ok = 1;
					printf("\r\nunicast wakeup!!\r\n");

					uint32_t packet_len = 0;
					uint8_t *wakeup_packet = rtl8735b_read_wakeup_packet(&packet_len, wowlan_wake_reason);

					// parse wakeup packet
					uint8_t *ip_header = NULL;
					uint8_t *tcp_header = NULL;
					uint16_t dst_port = 0;

					for (int i = 0; i < packet_len - 4; i ++) {
						if ((memcmp(wakeup_packet + i, &retention_local_ip, 4) == 0) && (*(wakeup_packet + i - 16) == 0x45)) {
							ip_header = wakeup_packet + i - 16;
							tcp_header = wakeup_packet + i + 4;
							break;
						}
					}

					if (ip_header && tcp_header) {
						if (tcp_header[13] == 0x02) {
							printf("SYN\n\r");

							dst_port = ntohs(*(uint16_t *)(tcp_header + 2));
							printf("dst port=%u\n\r", dst_port);

							if ((dst_port == server_local_port0) || (dst_port == server_local_port1)) {
								tcp_syn_wakeup = 1;
								server_local_port = dst_port;

								uint16_t ip_len = (((uint16_t) ip_header[2]) << 8) | ((uint16_t) ip_header[3]);

								uint32_t wakeup_eth_packet_len = 6 + 6 + (ip_len + 2);
								uint8_t *wakeup_eth_packet = (uint8_t *) malloc(wakeup_eth_packet_len);
								if (wakeup_eth_packet) {
									memcpy(wakeup_eth_packet, wakeup_packet + 4, 6);
									memcpy(wakeup_eth_packet + 6, wakeup_packet + 16, 6);
									memcpy(wakeup_eth_packet + 12, ip_header - 2, ip_len + 2);

									extern int lwip_set_wakeup_packet(uint8_t *packet, uint32_t packet_len);
									lwip_set_wakeup_packet(wakeup_eth_packet, wakeup_eth_packet_len);

									free(wakeup_eth_packet);
								}
							}
						}
					}

					free(wakeup_packet);

				}
			}
		} else if (pm_reason & (BIT(9) | BIT(10) | BIT(11) | BIT(12))) {
			// AON GPIO wake up

			extern int rtw_hal_wowlan_check_wlan_mcu_wakeup(void);
			if (rtw_hal_wowlan_check_wlan_mcu_wakeup() == 1) {
				wlan_mcu_ok = 1;
			} else {
				wlan_mcu_ok = 0;
				printf("\n\rERROR: rtw_hal_wowlan_check_wlan_mcu_wakeup \n\r");
			}
		}

		if (tcp_syn_wakeup) {
			extern void tcp_set_wakeup_port(uint16_t port);
			tcp_set_wakeup_port(server_local_port);

		}

		extern int rtw_hal_wlan_resume_check(void);
		if (wlan_mcu_ok && (rtw_hal_wlan_resume_check() == 1)) {
			wlan_resume = 1;

			extern int rtw_hal_read_aoac_rpt_from_txfifo(u8 * buf, u16 addr, u16 len);
			rtw_hal_read_aoac_rpt_from_txfifo(NULL, 0, 0);
		}
	}

	console_init();
	log_service_add_table(at_power_save_items, sizeof(at_power_save_items) / sizeof(at_power_save_items[0]));

	if (wlan_resume) {
		p_wifi_do_fast_connect = wlan_do_resume;
		p_store_fast_connect_info = NULL;
	} else {
		wifi_fast_connect_enable(1);
	}

	wlan_network();

	if (xTaskCreate(tcp_app_task, ((const char *)"tcp_app_task"), STACKSIZE, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
		printf("\n\r%s xTaskCreate(tcp_app_task) failed\n", __FUNCTION__);
	}

	vTaskStartScheduler();
	while (1);
}
