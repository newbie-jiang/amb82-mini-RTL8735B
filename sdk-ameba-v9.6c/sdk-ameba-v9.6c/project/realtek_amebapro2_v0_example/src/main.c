#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "hal.h"
#include "log_service.h"
#include "video_api.h"
#include <platform_opts.h>
#include <platform_opts_bt.h>
#include "video_boot.h"
#include "mmf2_mediatime_8735b.h"

#include "PinNames.h"
#include "gpio_api.h"
#include "netif.h"
#include "atcmd_bt.h"
#include "mDNS.h"

#if CONFIG_WLAN
#include <wifi_fast_connect.h>
extern void wlan_network(void);
#endif

extern void console_init(void);
extern void mpu_rodata_protect_init(void);


// tick count initial value used when start scheduler
uint32_t initial_tick_count = 0;

#ifdef _PICOLIBC__
int errno;
#endif

#if defined(CONFIG_FTL_ENABLED)
#include <ftl_int.h>

const u8 ftl_phy_page_num = 3;	/* The number of physical map pages, default is 3: BT_FTL_BKUP_ADDR, BT_FTL_PHY_ADDR1, BT_FTL_PHY_ADDR0 */
const u32 ftl_phy_page_start_addr = BT_FTL_BKUP_ADDR;


 TaskHandle_t  wps_Task = NULL;
 TaskHandle_t  bt_config_Task = NULL;


void app_ftl_init(void)
{
	ftl_init(ftl_phy_page_start_addr, ftl_phy_page_num);
}
#endif


/* overwrite log uart baud rate for application. ROM and bootloader will remain 115200
 * set LOGUART_TX_OFF 1 to turn off uart output from application
 */
#include "stdio_port_func.h"
extern hal_uart_adapter_t log_uart;

//#define LOGUART_TX_OFF 1

#if defined(LOGUART_TX_OFF) && (LOGUART_TX_OFF==1)
static void (*user_wputc)(phal_uart_adapter_t puart_adapter, uint8_t tx_data) = (void *)0xffffffff;
static void wputc(phal_uart_adapter_t puart_adapter, uint8_t tx_data)
{
	if ((uint32_t)user_wputc == (uint32_t)hal_uart_wputc) {
		user_wputc(puart_adapter, tx_data);
	}
}

void fUART(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	argc = parse_param(arg, argv);

	if (argc != 2)	{
		return;
	}

	if (strncmp(argv[1], "ON", 2) == 0) {
		user_wputc = hal_uart_wputc;
	} else {
		user_wputc = (void *)0xffffffff;
	}
}

static log_item_t uart_items[] = {
	{"UART", fUART,},
};

void atcmd_uart_init(void)
{
	log_service_add_table(uart_items, sizeof(uart_items) / sizeof(uart_items[0]));
}
#else
static void (*wputc)(phal_uart_adapter_t puart_adapter, uint8_t tx_data) = hal_uart_wputc;
#endif

void log_uart_port_init(int log_uart_tx, int log_uart_rx, uint32_t baud_rate)
{
	baud_rate = 115200;  //115200, 1500000, 3000000

	hal_status_t ret;
	uint8_t uart_idx;

#if defined(CONFIG_BUILD_NONSECURE) && (CONFIG_BUILD_NONSECURE == 1)
	/* prevent pin confliction */
	uart_idx = hal_uart_pin_to_idx(log_uart_rx, UART_Pin_RX);
	hal_pinmux_unregister(log_uart_rx, (PID_UART0 + uart_idx));
	hal_pinmux_unregister(log_uart_tx, (PID_UART0 + uart_idx));
#endif

	//* Init the UART port hadware
	ret = hal_uart_init(&log_uart, log_uart_tx, log_uart_rx, NULL);
	if (ret == HAL_OK) {
		hal_uart_set_baudrate(&log_uart, baud_rate);
		hal_uart_set_format(&log_uart, 8, UartParityNone, 1);

		// hook the putc function to stdio port for printf
#if defined(CONFIG_BUILD_NONSECURE) && (CONFIG_BUILD_NONSECURE == 1)
		stdio_port_init((void *)&log_uart, (stdio_putc_t)wputc, (stdio_getc_t)&hal_uart_rgetc);
#else
		stdio_port_init_s((void *)&log_uart, (stdio_putc_t)wputc, (stdio_getc_t)&hal_uart_rgetc);
		stdio_port_init_ns((void *)&log_uart, (stdio_putc_t)wputc, (stdio_getc_t)&hal_uart_rgetc);
#endif
	}
}

/* entry for the example*/
__weak void app_example(void) {}

void setup(void)
{
#if CONFIG_WLAN
#if ENABLE_FAST_CONNECT
	wifi_fast_connect_enable(1);
#else
	wifi_fast_connect_enable(0);
#endif
	wlan_network();
#endif

#if defined(CONFIG_FTL_ENABLED)
	app_ftl_init();
#endif

#if defined(LOGUART_TX_OFF) && (LOGUART_TX_OFF==1)
	atcmd_uart_init();
#endif

}

void set_initial_tick_count(void)
{
	// Check DWT_CTRL(0xe0001000) CYCCNTENA(bit 0). If DWT cycle counter is enabled, set tick count initial value based on DWT cycle counter.
	if ((*((volatile uint32_t *) 0xe0001000)) & 1) {
		(*((volatile uint32_t *) 0xe0001000)) &= (~((uint32_t) 1)); // stop DWT cycle counter
		uint32_t dwt_cyccnt = (*((volatile uint32_t *) 0xe0001004));
		uint32_t systick_load = (configCPU_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
		initial_tick_count = dwt_cyccnt / systick_load;
	}

	// Auto set the media time offset
	video_boot_stream_t *isp_fcs_info;
	video_get_fcs_info(&isp_fcs_info);  //Get the fcs info
	uint32_t media_time_ms = initial_tick_count + isp_fcs_info->fcs_start_time;
	mm_set_mediatime_in_ms(media_time_ms);
}


static void print_wifi_setting(const char *ifname, rtw_wifi_setting_t *pSetting)
{
	printf("\n\r\nWIFI  %s Setting:", ifname);
	printf("\n\r==============================");

	switch (pSetting->mode) {
	case RTW_MODE_AP:
		printf("\n\r		MODE => AP");
		break;
	case RTW_MODE_STA:
		printf("\n\r		MODE => STATION");
		break;
	default:
		printf("\n\r		MODE => UNKNOWN");
	}
	printf("\n\r		SSID => %s", pSetting->ssid);
	printf("\n\r	 CHANNEL => %d", pSetting->channel);

	switch (pSetting->security_type) {
	case RTW_SECURITY_OPEN:
		printf("\n\r	SECURITY => OPEN");
		break;
	case RTW_SECURITY_WEP_PSK:
		printf("\n\r	SECURITY => WEP");
		printf("\n\r KEY INDEX => %d", pSetting->key_idx);
		break;
	case RTW_SECURITY_WPA_TKIP_PSK:
	case RTW_SECURITY_WPA2_TKIP_PSK:
		printf("\n\r	SECURITY => TKIP");
		break;
	case RTW_SECURITY_WPA_AES_PSK:
	case RTW_SECURITY_WPA2_AES_PSK:
	case RTW_SECURITY_WPA3_AES_PSK:
		printf("\n\r	SECURITY => AES");
		break;
	default:
		printf("\n\r	SECURITY => UNKNOWN");
	}

	printf("\n\r	PASSWORD => %s", pSetting->password);
	printf("\n\r");
}

typedef struct thread_sta{
	uint8_t wps_sta;
	uint8_t bt_config_sta;	
}thread_sta;

thread_sta  s_thread_sta = {0};
gpio_t gpio_led;

void _wps_thread(void *pvParameters)
{
	s_thread_sta.wps_sta = 1;

	char *id = pvParameters;
	
	printf("\nExample: wlan_scenario \n");
	gpio_write(&gpio_led, 1);
#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
	rtw_create_secure_context(configMINIMAL_SECURE_STACK_SIZE);
#endif
	// Wait for other task stable.
	vTaskDelay(4000);

    printf("\n\n[WLAN_SCENARIO_EXAMPLE] Wi-Fi example for authentication...\n");
	rtw_network_info_t connect_param = {0};

	/*********************************************************************************
	*	1. Enable Wi-Fi with STA mode
	*********************************************************************************/
	printf("\n\r[WLAN_SCENARIO_EXAMPLE] Enable Wi-Fi\n");
	if (wifi_on(RTW_MODE_STA) < 0) {
		printf("\n\r[WLAN_SCENARIO_EXAMPLE] ERROR: wifi_on failed\n");
		return;
	}
	/*********************************************************************************
	*	2. Connect to AP by different authentications
	*********************************************************************************/
	printf("\n\r[WLAN_SCENARIO_EXAMPLE] Connect to AP\n");
	
	// By WPS-PBC.
	char *argv[2];
	argv[1] = (char *)"pbc";
	cmd_wps(2, argv);
	
	/*********************************************************************************
	*	3. Show Wi-Fi information
	*********************************************************************************/
	printf("\n\r[WLAN_SCENARIO_EXAMPLE] Show Wi-Fi information\n");
	rtw_wifi_setting_t setting;
	wifi_get_setting(WLAN0_IDX, &setting);
	print_wifi_setting(WLAN0_NAME, &setting);
}


void wps_test(char *id)
{
	if (xTaskCreate(_wps_thread, ((const char *)"_wps_thread"), 1024, (void *const) id, tskIDLE_PRIORITY + 1, &wps_Task) != pdPASS) {
		printf("\n\r%s xTaskCreate failed\n", __FUNCTION__);
	}
}

extern int bt_config_app_init(void);
extern void bt_config_app_deinit(void);
extern void set_bt_cmd_type(uint8_t cmd_type);


#define GPIO_LED_PIN       PE_6

extern struct netif xnetif[NET_IF_NUM];

void led_thread(void *pvParameters)
{
    vTaskDelay(4000); //让其他线程先运行 4s 后再检测网络连接状态

	dbg_printf("\r\n   GPIO DEMO   \r\n");

	// Init LED control pin
	gpio_init(&gpio_led, GPIO_LED_PIN);
	gpio_dir(&gpio_led, PIN_OUTPUT);        // Direction: Output
	gpio_mode(&gpio_led, PullNone);         // No pull

    //处于网络连接状态
	if(netif_is_link_up(xnetif)){

	}else{ //网络未连接
	
	     bt_config_app_init();
         set_bt_cmd_type(CONFIG_BIT | STACK_BIT);
		 s_thread_sta.bt_config_sta = 1;
	}


	while (1) {

		if(netif_is_link_up(xnetif)) //网络处于连接状态
		{   
			gpio_write(&gpio_led, 1);
		    vTaskDelay(500);
		    gpio_write(&gpio_led, 0);
		    vTaskDelay(500);

		
			//检测蓝牙配网是否已开启，开启则关闭蓝牙配网
			if(s_thread_sta.bt_config_sta == 1)
			{	
				printf("off ble config wifi......\r\n");
				bt_config_app_deinit();     //关闭蓝牙配网
                set_bt_cmd_type(0);
				s_thread_sta.bt_config_sta = 0;
			}

		}else{  //网络处于非连接状态

		      printf("wifi not connect......\r\n");
 
		      vTaskDelay(4000);
		   
		}
	}
}



void gpio_test(void)
{
	if (xTaskCreate(led_thread, ((const char *)"led_thread"), 1024, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
		printf("\n\r%s xTaskCreate failed\n", __FUNCTION__);
	}
}



extern void mDNSTest(void) ;


#define GPIO_PUSHBT_PIN    PF_15
//PF15
void wps_key_thread(void *param)
{
	gpio_t gpio_btn;
    gpio_init(&gpio_btn, GPIO_PUSHBT_PIN);
	gpio_dir(&gpio_btn, PIN_INPUT);         // Direction: Input
	gpio_mode(&gpio_btn, PullDown);           // Pull-High   
	uint32_t key_count = 0;

	while (1) {     
		
         if (gpio_read(&gpio_btn)) {
			key_count++;	
			if(key_count>=300){
			   //检测到长按3s或以上
			   //开启wps线程
			  
			    wps_test(0);
			   key_count =0;
			   
               printf("wps key long press......\r\n");
			}
		}else{
            key_count=0;
		} 
		vTaskDelay(10);    
	}
}


void wps_key_test(void)
{
	if (xTaskCreate(wps_key_thread, ((const char *)"wps_key_thread"), 1024, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
		printf("\n\r%s xTaskCreate failed\n", __FUNCTION__);
	}
}



// #define MDNS_SERVICE_NAME    "ameba"
// #define MDNS_SERVICE_TYPE    "_http._tcp"
// #define MDNS_SERVICE_PORT    5353

// // 初始化 TXT 记录
// void initTXTRecord(TXTRecordRef *txtRecord) {
//     char buffer[64];
//     TXTRecordCreate(txtRecord, sizeof(buffer), buffer);
//     TXTRecordSetValue(txtRecord, "version", 1, "1.0");
//     TXTRecordSetValue(txtRecord, "device", 1, "Ameba");
// }

// void mDNSTest(void) {
//     TXTRecordRef txtRecord;
//     DNSServiceRef serviceRef;

//     printf("mdns test start..........\r\n");

//     // 创建并初始化 TXT 记录
//     initTXTRecord(&txtRecord);

//     // 初始化 mDNS 响应器
//     if (mDNSResponderInit() != 0) {
//         printf("Failed to initialize mDNS responder.\n");
//         return;
//     }

//     // 注册 mDNS 服务
//     serviceRef = mDNSRegisterService(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, "local", MDNS_SERVICE_PORT, &txtRecord);
//     if (serviceRef == NULL) {
//         printf("Failed to register mDNS service.\n");
//         mDNSResponderDeinit();
//         return;
//     }

//     printf("mDNS service '%s' registered successfully.\n", MDNS_SERVICE_NAME);

//     // 这里可以添加其他操作，如定期更新服务或查询
//     // 例如更新 TXT 记录：
//     // TXTRecordSetValue(&txtRecord, "status", 1, "active");
//     // mDNSUpdateService(serviceRef, &txtRecord, 0);

//     // 等待一些时间（或根据需要执行其他操作）

//     // 注销 mDNS 服务
//     // mDNSDeregisterService(serviceRef);
//     // printf("mDNS service '%s' deregistered successfully.\n", MDNS_SERVICE_NAME);

//     // // 释放资源
//     // mDNSResponderDeinit();
// }



/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
	/* for debug, protect rodata*/
	//mpu_rodata_protect_init();
	console_init();

	voe_t2ff_prealloc();

	setup();

	// 进入临界区，禁止中断
    taskENTER_CRITICAL();
 
    //wps_test(NULL);
    gpio_test();

	wps_key_test();

	/* Execute application example */
	app_example();

    //退出临界区，恢复中断
    taskEXIT_CRITICAL();

	/* set tick count initial value before start scheduler */
	set_initial_tick_count();

	vTaskStartScheduler();

	while (1);
}
