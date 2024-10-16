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
#define TCP_RESUME    1
#define TCP_RESUME_MAX_PACKETS 3
#define SSL_KEEPALIVE 0
#define WOWLAN_GPIO_WDT      1

extern uint8_t rtl8735b_wowlan_wake_reason(void);
extern uint8_t rtl8735b_wowlan_wake_pattern(void);
extern uint8_t *rtl8735b_read_wakeup_packet(uint32_t *size, uint8_t wowlan_reason);
extern int rtl8735b_suspend(int mode);
extern void rtl8735b_set_lps_pg(void);
extern void rtl8735b_set_lps_dtim(uint8_t dtim);
extern void wifi_wowlan_bcntrack_stage1(uint8_t rx_bcn_window, uint8_t bcn_limit);
extern void wifi_wowlan_set_dtimtimeout(uint8_t set_dtimtimeout);

static uint8_t wowlan_wake_reason = 0;
static uint8_t wlan_resume = 0;
static uint8_t tcp_resume = 0;
static uint8_t ssl_resume = 0;
static uint8_t disconnect_pno = 0;
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

static uint8_t  pno_start_window = 5;
static uint8_t  pno_max_window = 240;
static uint8_t  pno_increment_steps = 10;
static uint8_t  pno_scan_period = 3;
static uint32_t  pno_duration = 360;

static char server_ip[16] = "192.168.0.163";
static uint16_t server_port = 5566;
__attribute__((section(".retention.data"))) uint16_t retention_local_port __attribute__((aligned(32))) = 0;

static uint8_t goto_sleep = 0;
static int keepalive = 1, keepalive_idle = 240, keepalive_interval = 10, keepalive_count = 10;

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

#include "mbedtls/version.h"
#include "mbedtls/config.h"
#include "mbedtls/ssl.h"
#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#if defined(MBEDTLS_VERSION_NUMBER) && (MBEDTLS_VERSION_NUMBER == 0x03000000)
#include "mbedtls/../../library/ssl_misc.h"
#include "mbedtls/../../library/md_wrap.h"
#else
#include "mbedtls/ssl_internal.h"
#include "mbedtls/md_internal.h"
#endif

static void *_calloc_func(size_t nmemb, size_t size)
{
	size_t mem_size;
	void *ptr = NULL;

	mem_size = nmemb * size;
	ptr = pvPortMalloc(mem_size);

	if (ptr) {
		memset(ptr, 0, mem_size);
	}

	return ptr;
}

static int _random_func(void *p_rng, unsigned char *output, size_t output_len)
{
	/* To avoid gcc warnings */
	(void) p_rng;

	rtw_get_random_bytes(output, output_len);
	return 0;
}

void tcp_app_task(void *param)
{
	while (!goto_sleep) {
		if (tcp_resume) {
			break;
		}

		if (disconnect_pno) {
			goto pno_suspend;
		}

		vTaskDelay(2000);
		u32 value32;
		value32 = HAL_READ32(0x40080000, 0x54);
		value32 = value32 & 0xFF;
		printf("beacon recv per 20 = %d\r\n", value32);
	}

	int ret = 0;
	int sock_fd = -1;
#if SSL_KEEPALIVE
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
#endif

#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
	rtw_create_secure_context(configMINIMAL_SECURE_STACK_SIZE);
#endif

	// wait for IP address
	while (!((wifi_get_join_status() == RTW_JOINSTATUS_SUCCESS) && (*(u32 *)LwIP_GetIP(0) != IP_ADDR_INVALID))) {
		vTaskDelay(10);
	}

	// socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	printf("\n\r socket(%d) \n\r", sock_fd);

	if (setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) != 0) {
		printf("ERROR: SO_KEEPALIVE\n");
	}
	if (setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_idle, sizeof(keepalive_idle)) != 0) {
		printf("ERROR: TCP_KEEPIDLE\n");
	}
	if (setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof(keepalive_interval)) != 0) {
		printf("ERROR: TCP_KEEPINTVL\n");
	}
	if (setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_count, sizeof(keepalive_count)) != 0) {
		printf("ERROR: TCP_KEEPCNT\n");
	}

	if (tcp_resume) {
		// resume on the same local port
		if (retention_local_port != 0) {
			struct sockaddr_in local_addr;
			local_addr.sin_family = AF_INET;
			local_addr.sin_addr.s_addr = INADDR_ANY;
			local_addr.sin_port = htons(retention_local_port);
			printf("bind local port:%d %s \n\r", retention_local_port, bind(sock_fd, (struct sockaddr *) &local_addr, sizeof(local_addr)) == 0 ? "OK" : "FAIL");
		}

#if TCP_RESUME
		// resume tcp pcb
		extern int lwip_resume_tcp(int s);
		printf("resume TCP pcb & seqno & ackno %s \n\r", lwip_resume_tcp(sock_fd) == 0 ? "OK" : "FAIL");
#endif
	} else {
		// connect
		struct sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr(server_ip);
		server_addr.sin_port = htons(server_port);

		if (connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == 0) {
			// retain local port
			struct sockaddr_in sin;
			socklen_t len = sizeof(sin);
			if (getsockname(sock_fd, (struct sockaddr *)&sin, &len) == -1) {
				printf("ERROR: getsockname \n\r");
			} else {
				retention_local_port = ntohs(sin.sin_port);
				dcache_clean_invalidate_by_addr((uint32_t *) &retention_local_port, sizeof(retention_local_port));
				printf("retain local port: %d \n\r", retention_local_port);
			}

			printf("connect to %s:%d OK \n\r", server_ip, server_port);
		} else {
			printf("connect to %s:%d FAIL \n\r", server_ip, server_port);
			close(sock_fd);
			goto exit1;
		}
	}

#if SSL_KEEPALIVE
	mbedtls_platform_set_calloc_free(_calloc_func, vPortFree);

	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
	mbedtls_ssl_set_bio(&ssl, &sock_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

	if ((ret = mbedtls_ssl_config_defaults(&conf,
										   MBEDTLS_SSL_IS_CLIENT,
										   MBEDTLS_SSL_TRANSPORT_STREAM,
										   MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {

		printf("\nERROR: mbedtls_ssl_config_defaults %d\n", ret);
		goto exit;
	}

	static int ciphersuites[] = {MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384, 0};
	mbedtls_ssl_conf_ciphersuites(&conf, ciphersuites);
	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
	mbedtls_ssl_conf_rng(&conf, _random_func, NULL);

	if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
		printf("\nERROR: mbedtls_ssl_setup %d\n", ret);
		goto exit;
	}

	if (ssl_resume) {
		extern int mbedtls_ssl_resume(mbedtls_ssl_context * ssl);
		printf("resume SSL %s \n\r", mbedtls_ssl_resume(&ssl) == 0 ? "OK" : "FAIL");
	} else {
		// handshake
		if ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
			printf("\nERROR: mbedtls_ssl_handshake %d\n", ret);
			goto exit;
		} else {
			printf("\nUse ciphersuite %s\n", mbedtls_ssl_get_ciphersuite(&ssl));
		}
	}
#endif
	while (!goto_sleep) {
#if 0
#if SSL_KEEPALIVE
		ret = mbedtls_ssl_write(&ssl, (uint8_t *) "_APP", strlen("_APP"));
#else
		ret = write(sock_fd, "_APP", strlen("_APP"));
#endif
		printf("write application data %d \n\r", ret);
#endif
		if (ret < 0) {
#if SSL_KEEPALIVE
			mbedtls_ssl_close_notify(&ssl);
#endif
			goto exit;
		}

		vTaskDelay(5000);
	}

#if SSL_KEEPALIVE
	// retain ssl
	extern int mbedtls_ssl_retain(mbedtls_ssl_context * ssl);
	printf("retain SSL %s \n\r", mbedtls_ssl_retain(&ssl) == 0 ? "OK" : "FAIL");
#endif
#if TCP_RESUME
	// retain tcp pcb
	extern int lwip_retain_tcp(int s);
	printf("retain TCP pcb %s \n\r", lwip_retain_tcp(sock_fd) == 0 ? "OK" : "FAIL");
#endif

	// set ntp offload
	extern int wifi_set_ntp_offload(char *server_names[], uint8_t num_servers, uint32_t update_delay_ms);
	char *server_name[4] = {"pool.ntp.org"};
	wifi_set_ntp_offload(server_name, 1, 60 * 60 * 1000);

	// set keepalive
	extern int wifi_set_tcp_protocol_keepalive_offload(int socket_fd);
	wifi_set_tcp_protocol_keepalive_offload(sock_fd);

	wifi_set_dhcp_offload(); // after wifi_set_tcp_keep_alive_offload

	wifi_wowlan_set_arpreq_keepalive(0, 0); // arp0, null1

	//set tbtt interval
	//static uint8_t lps_dtim = 10;
	rtl8735b_set_lps_dtim(lps_dtim);

	wifi_wowlan_set_dtimtimeout(1);

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

	//set pno scan parmeters
	// static uint8_t  pno_start_window = 5;
	// static uint8_t  pno_max_window = 240;
	// static uint8_t  pno_increment_steps = 10;
	// static uint8_t  pno_scan_period = 3;
	// static uint32_t pno_duration = 360;
	wifi_wowlan_set_pno_scan(pno_start_window, pno_max_window, pno_increment_steps, pno_scan_period, pno_duration);

	extern int dhcp_retain(void);
	printf("retain DHCP %s \n\r", dhcp_retain() == 0 ? "OK" : "FAIL");

#if WOWLAN_GPIO_WDT
	wifi_wowlan_set_wdt(2, 5, 1, 1); //gpiof_2, io trigger interval 1 min, io pull high and trigger pull low, pulse duration 1ms
#endif

	// for wlan resume
	extern int rtw_hal_wlan_resume_backup(void);
	rtw_hal_wlan_resume_backup();

pno_suspend:
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

exit:
	printf("\n\r close(%d) \n\r", sock_fd);
	close(sock_fd);
#if SSL_KEEPALIVE
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
#endif

exit1:
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
	} else if (strcmp(argv[1], "tcp_keep_alive") == 0) {
		if (argc >= 4) {
			sprintf(server_ip, "%s", argv[2]);
			server_port = strtoul(argv[3], NULL, 10);
		}
		printf("setup tcp keep alive to %s:%d\r\n", server_ip, server_port);
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
	} else if (strcmp(argv[1], "pno_suspend") == 0) {
		disconnect_pno = atoi(argv[2]);
	}
}

log_item_t at_power_save_items[ ] = {
	{"PS", fPS,},
};

void main(void)
{
	uint32_t pm_reason = Get_wake_reason();
	printf("\n\rpm_reason=0x%x\n\r", pm_reason);

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
				if (wowlan_wake_reason == 0x77) {
					wlan_mcu_ok = 1;

					uint32_t packet_len = 0;
					uint8_t *wakeup_packet = rtl8735b_read_wakeup_packet(&packet_len, wowlan_wake_reason);

					// parse wakeup packet
					uint8_t *ip_header = NULL;
					uint8_t *tcp_header = NULL;
					uint8_t tcp_header_first4[4];
					tcp_header_first4[0] = (server_port & 0xff00) >> 8;
					tcp_header_first4[1] = (server_port & 0x00ff);
					tcp_header_first4[2] = (retention_local_port & 0xff00) >> 8;
					tcp_header_first4[3] = (retention_local_port & 0x00ff);

					for (int i = 0; i < packet_len - 4; i ++) {
						if ((memcmp(wakeup_packet + i, tcp_header_first4, 4) == 0) && (*(wakeup_packet + i - 20) == 0x45)) {
							ip_header = wakeup_packet + i - 20;
							tcp_header = wakeup_packet + i;
							break;
						}
					}

					if (ip_header && tcp_header) {
						if ((tcp_header[13] == 0x18) || (tcp_header[13] == 0x10)) {
							printf("PUSH + ACK or ACK with payload\n\r");
#if TCP_RESUME
							tcp_resume = 1;

							uint16_t ip_len = (((uint16_t) ip_header[2]) << 8) | ((uint16_t) ip_header[3]);

							uint32_t wakeup_eth_packet_len = 6 + 6 + (ip_len + 2);
							uint8_t *wakeup_eth_packet = (uint8_t *) malloc(wakeup_eth_packet_len);
							if (wakeup_eth_packet) {
								memcpy(wakeup_eth_packet, wakeup_packet + 4, 6);
								memcpy(wakeup_eth_packet + 6, wakeup_packet + 16, 6);
								memcpy(wakeup_eth_packet + 12, ip_header - 2, ip_len + 2);

								extern int lwip_set_resume_packet(uint8_t *packet, uint32_t packet_len);
								lwip_set_resume_packet(wakeup_eth_packet, wakeup_eth_packet_len);

								free(wakeup_eth_packet);
							}
#endif
						}
					}

					free(wakeup_packet);

#if defined(TCP_RESUME_MAX_PACKETS) && (TCP_RESUME_MAX_PACKETS > 1)
					for (int j = 1; j < TCP_RESUME_MAX_PACKETS; j ++) {
						extern uint8_t *rtl8735b_read_packet_with_index(uint32_t *size, uint8_t index);
						wakeup_packet = rtl8735b_read_packet_with_index(&packet_len, j);
						if (wakeup_packet == NULL) {
							break;
						}

						ip_header = NULL;
						tcp_header = NULL;
						for (int i = 0; i < packet_len - 4; i ++) {
							if ((memcmp(wakeup_packet + i, tcp_header_first4, 4) == 0) && (*(wakeup_packet + i - 20) == 0x45)) {
								ip_header = wakeup_packet + i - 20;
								tcp_header = wakeup_packet + i;
								break;
							}
						}

						if (ip_header && tcp_header) {
							uint16_t ip_len = (((uint16_t) ip_header[2]) << 8) | ((uint16_t) ip_header[3]);

							uint32_t wakeup_eth_packet_len = 6 + 6 + (ip_len + 2);
							uint8_t *wakeup_eth_packet = (uint8_t *) malloc(wakeup_eth_packet_len);
							if (wakeup_eth_packet) {
								memcpy(wakeup_eth_packet, wakeup_packet + 4, 6);
								memcpy(wakeup_eth_packet + 6, wakeup_packet + 16, 6);
								memcpy(wakeup_eth_packet + 12, ip_header - 2, ip_len + 2);

								extern int lwip_set_resume_packet_with_index(uint8_t *packet, uint32_t packet_len, uint8_t index);
								lwip_set_resume_packet_with_index(wakeup_eth_packet, wakeup_eth_packet_len, (uint8_t) j);

								free(wakeup_eth_packet);
							}
						}

						free(wakeup_packet);
					}
#endif
				} else if (wowlan_wake_reason == 0x7A || wowlan_wake_reason == 0x7B) {
					printf("Wakeup due to CSA !!! \n\r");
					extern int rtw_hal_wowlan_check_wlan_mcu_wakeup(void);
					if (rtw_hal_wowlan_check_wlan_mcu_wakeup() == 1) {
						wlan_mcu_ok = 1;
#if TCP_RESUME
						extern uint8_t lwip_check_tcp_resume(void);
						if (lwip_check_tcp_resume() == 1) {
							tcp_resume = 1;
						}
#endif
					} else {
						wlan_mcu_ok = 0;
						printf("\n\rERROR: rtw_hal_wowlan_check_wlan_mcu_wakeup \n\r");
					}
				} else if (wowlan_wake_reason == 0x70) {
					extern uint8_t rtw_hal_read_ch_pno_scan_from_txfifo(void);
					uint8_t channel = rtw_hal_read_ch_pno_scan_from_txfifo();
					printf("\r\nwake up from pno and camp on ch %d\r\n", channel);
				} else if (wowlan_wake_reason == 0x71) {
					printf("\r\nwake up from pno and no channel can't be scan\r\n");
				}
			}
		} else if (pm_reason & (BIT(9) | BIT(10) | BIT(11) | BIT(12))) {
			// AON GPIO wake up

			extern int rtw_hal_wowlan_check_wlan_mcu_wakeup(void);
			if (rtw_hal_wowlan_check_wlan_mcu_wakeup() == 1) {
				wlan_mcu_ok = 1;
#if TCP_RESUME
				extern uint8_t lwip_check_tcp_resume(void);
				if (lwip_check_tcp_resume() == 1) {
					tcp_resume = 1;
				}
#endif
			} else {
				wlan_mcu_ok = 0;
				printf("\n\rERROR: rtw_hal_wowlan_check_wlan_mcu_wakeup \n\r");
			}
		}

		if (wlan_mcu_ok) {
			long long current_sec = 0;
			extern void wifi_read_ntp_current_sec(long long * current_sec);
			wifi_read_ntp_current_sec(&current_sec);
			if (current_sec == 0) {
				printf("get NTP Time fail!\r\n");
			} else {
				struct tm *current_tm = localtime(&current_sec);
				current_tm->tm_year += 1900;
				current_tm->tm_mon += 1;
				printf("%d-%d-%d %d:%d:%d UTC\n\r", current_tm->tm_year, current_tm->tm_mon, current_tm->tm_mday, current_tm->tm_hour, current_tm->tm_min, current_tm->tm_sec);
			}
		}

		if (tcp_resume) {
#if TCP_RESUME
			// set tcp resume port to drop packet before tcp resume done
			// must set before lwip init
			extern void tcp_set_resume_port(uint16_t port);
			tcp_set_resume_port(retention_local_port);

			extern void lwip_recover_resume_keepalive(void);
			lwip_recover_resume_keepalive();
#if SSL_KEEPALIVE
			ssl_resume = 1;
#endif
#endif
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
