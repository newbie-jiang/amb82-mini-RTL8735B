#include <platform_opts.h>
#include <platform_stdlib.h>
#include "usbh.h"
#include "osdep_service.h"
#include "usbh_cdc_ecm.h"
#include "usbh_cdc_ecm_hal.h"
#include "log_service.h"
#include "ethernet_usb.h"

#ifdef PLATFORM_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif
#include "osdep_service.h"
#include "lwip_netconf.h"
#include "lwip_intf.h"
#include "platform_opts.h"

#if(CONFIG_ETHERNET == 1 && ETHERNET_INTERFACE == USB_INTERFACE)

#define ETHERNET_IDX (NET_IF_NUM - 1)

extern struct netif  xnetif[NET_IF_NUM];

static u8 TX_BUFFER[1536];
static u8 RX_BUFFER[1536];

static _mutex mii_tx_mutex;
static _mutex mii_link_mutex;

static struct task_struct task_ecm_attach;
static struct task_struct task_ecm_detach;

extern int lwip_init_done;

#define USBH_ECM_THREAD_STACK_SIZE 2048

void usb_ethernet_ecm_cb(u8 *buf, u32 len);

static _sema ecm_detach_sema = NULL;
static _sema ecm_attach_sema = NULL;

static u32 rx_buffer_saved_data = 0;
static u32 ip_total_len = 0;
static int ecm_status = ECM_STATUS_NONE;
u8 multi_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

//#define ECM_MONITOR_MODE //If the usb is detach that it will initial the ecm to get the attach

static void usb_cdc_ecm_attach_cb(void)
{
	rtw_up_sema(&ecm_attach_sema);
}

static void usb_cdc_ecm_detach_cb(void)
{
	rtw_up_sema(&ecm_detach_sema);
}

static void ecm_lwip_init(void)//Connect to lwip
{
	int link_is_up = 0;
	int dhcp_status = 0;
	printf("ecm lwip init\r\n");
	//Wait the ethernet ready
	while (1) {
		link_is_up = usbh_cdc_ecm_get_connect_status();
		if (link_is_up) {
			break;
		} else {
			vTaskDelay(100);
		}
	}

	u8 *mac = (unsigned char *)usbh_cdc_ecm_process_mac_str();
	//If no mac address that we will set the fake mac address
	if (mac[0] == 0 && mac[1] == 0 && mac[2] == 0 && mac[3] == 0 && mac[4] == 0 && mac[5] == 0) {
		mac[0] = 0x00;
		mac[1] = 0xe0;
		mac[2] = 0x4c;
		mac[3] = 0x36;
		mac[4] = 0x00;
		mac[5] = 0x02;
	}
	printf("mac[%02x %02x %02x %02x %02x %02x]\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	for (int i = 0; i < 6; i++) {
		multi_mac[i] = mac[i];
	}

	memcpy(xnetif[NET_IF_NUM - 1].hwaddr, mac, 6);

	if (!netif_is_link_up(&xnetif[ETHERNET_IDX])) {
		printf("Lwip link up\r\n");
		netif_set_link_up(&xnetif[ETHERNET_IDX]);
		netif_set_up(&xnetif[ETHERNET_IDX]);
	}
	dhcp_status = LwIP_DHCP(NET_IF_NUM - 1, DHCP_START);
	if (DHCP_ADDRESS_ASSIGNED == dhcp_status) {
		netif_set_default(&xnetif[NET_IF_NUM - 1]);	//Set default gw to ether netif
	} else {
		printf("It can't get the dchp\r\n");
	}
}

static void ecm_lwip_deinit(void)//disconnect to lwip
{
	netif_set_default(&xnetif[0]);
	LwIP_ReleaseIP(ETHERNET_IDX);
	if (netif_is_link_up(&xnetif[ETHERNET_IDX])) {
		printf("Lwip link down\r\n");
		netif_set_link_down(&xnetif[ETHERNET_IDX]);
		netif_set_down(&xnetif[ETHERNET_IDX]);
	}
}

bool ecm_on(void)
{
	bool status = false;
	usbh_cdc_ecm_user_cb_t usb_cb;
	memset(&usb_cb, 0x00, sizeof(usb_cb));
	rx_buffer_saved_data = 0;
	ip_total_len = 0;
	usb_cb.report_data = usb_ethernet_ecm_cb;//Recevie the packet
	usb_cb.usb_attach = usb_cdc_ecm_attach_cb;//Attach the ECM
	usb_cb.usb_detach = usb_cdc_ecm_detach_cb;//Detach the ECM
	status = usbh_cdc_ecm_on(&usb_cb);
	if (status == true) {
		printf("ecm init ok\r\n");
	} else {
		printf("ecm init fail\r\n");
	}
	return status;
}

bool ecm_off(void)
{
	bool status = false;
	status = usbh_cdc_ecm_off();
	if (status == true) {
		printf("ecm deinit ok\r\n");
	} else {
		printf("ecm deinit fail\r\n");
	}
	return status;
}

void ecm_deinit_resoure(void)
{
	rtw_mutex_free(&mii_tx_mutex);
	rtw_mutex_free(&mii_link_mutex);
	rtw_free_sema(&ecm_attach_sema);
	rtw_free_sema(&ecm_detach_sema);
	rtw_delete_task(&task_ecm_attach);
	ecm_status = ECM_STATUS_DEINIT;
	printf("ecm deinit\r\n");
}

static void ecm_detach_thread(void *parm)
{
	while (1) {
		rtw_down_sema(ecm_detach_sema);
		ecm_lwip_deinit();
		ecm_off();
		printf("link to unlink !!\n");
#ifdef ECM_MONITOR_MODE
		ecm_on();
#else
		ecm_deinit_resoure();
		break;
#endif
	}
	rtw_thread_exit();
}

static void ecm_attach_thread(void *parm)
{
	while (1) {
		rtw_down_sema(ecm_attach_sema);
		ecm_lwip_init();
		printf("unlink to link !!\n");
	}
	rtw_thread_exit();
}

//should parse the data to get the ip header
u8 rltk_mii_recv_data(u8 *buf, u32 total_len, u32 *frame_length)
{
	u8 *pbuf ;

	if (0 == ip_total_len) { //first packet
		pbuf = RX_BUFFER;
		rtw_memcpy((void *)pbuf, buf, total_len);
		if (total_len != 512) { //should finish
			*frame_length = total_len;
			return 1;
		} else { //get the total length
			rx_buffer_saved_data = total_len;
			//should check the vlan header
			//should check it is IP packet 0x0800
			ip_total_len = buf[14 + 2] * 256 + buf[14 + 2 + 1];
			//printf("ip packet len = %d\n", ip_total_len);
			if (512 - 14 == ip_total_len) { //the total length is 512
				*frame_length = total_len;
				ip_total_len = 0;
				return 1;
			}
		}
	} else {
		pbuf = RX_BUFFER + rx_buffer_saved_data;
		rtw_memcpy((void *)pbuf, buf, total_len);
		rx_buffer_saved_data += total_len;
		if (total_len != 512) {
			//should finish
			*frame_length = rx_buffer_saved_data;
			ip_total_len = 0;
			return 1;
		} else {
			//should check the vlan header
			//should check it is IP packet 0x0800
			if (rx_buffer_saved_data - 14 == ip_total_len) {
				//should finish
				*frame_length = rx_buffer_saved_data;
				ip_total_len = 0;
				return 1;
			}
		}
	}

	return 0;
}
u8 rltk_mii_recv_data_check(u8 *mac, u32 frame_length)
{
	u8 *pbuf = RX_BUFFER;
	u8 checklen = 0;
	u8 multi[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	//printf("[usb]get framelen=%d\n",frame_length);

	if (memcmp(mac, pbuf, 6) == 0 || memcmp(multi, pbuf, 6) == 0) {
		checklen = 7 ;
		//printf("\n[rx data header]");
	} else {
		checklen = 6;
		//printf("\n[rx data header][exit]");
	}

	if (1) {
		u32 index = 0 ;
		u32 max = frame_length;

		if (frame_length >= checklen) {
			max = checklen;
		}
	}

	return (checklen == 6) ? (0) : (1);
}

void usb_ethernet_ecm_cb(u8 *buf, u32 len)
{
	u8 *pbuf = RX_BUFFER;
	u32 frame_len = 0;

	if (0 == rltk_mii_recv_data(buf, len, &frame_len)) {
		return;
	}

	if (0 == rltk_mii_recv_data_check(multi_mac, frame_len)) {
		return;
	}
	ethernetif_mii_recv(&xnetif[ETHERNET_IDX], frame_len);
}

void rltk_mii_recv(struct eth_drv_sg *sg_list, int sg_len)
{
	struct eth_drv_sg *last_sg;
	u8 *pbuf = RX_BUFFER;

	for (last_sg = &sg_list[sg_len]; sg_list < last_sg; ++sg_list) {
		if (sg_list->buf != 0) {
			rtw_memcpy((void *)(sg_list->buf), pbuf, sg_list->len);
			pbuf += sg_list->len;
		}
	}
}

s8 rltk_mii_send(struct eth_drv_sg *sg_list, int sg_len, int total_len)
{
	int ret = 0;
	struct eth_drv_sg *last_sg;
	u8 *pdata = TX_BUFFER;
	u8	retry_cnt = 0;
	u32 size = 0;
	for (last_sg = &sg_list[sg_len]; sg_list < last_sg; ++sg_list) {
		rtw_memcpy(pdata, (void *)(sg_list->buf), sg_list->len);
		pdata += sg_list->len;
		size += sg_list->len;
	}
	pdata = TX_BUFFER;
	if (NULL == mii_tx_mutex) {
		rtw_mutex_init(&mii_tx_mutex);
	}
	rtw_mutex_get(&mii_tx_mutex);
	while (1) {
		ret = usbh_cdc_ecm_senddata(pdata, size);
		if (ret == 0) {
			//ethernet_send();
			//ret = 0;
			break;
		}
		if (++retry_cnt > 3) {
			printf("TX drop\n\r");
			ret = -1;
			break;
		} else {
			rtw_udelay_os(1);
		}
	}
	rtw_mutex_put(&mii_tx_mutex);

	//wait reply success,wait signal
	while (!usbh_cdc_ecm_get_sendflag()) {
		rtw_msleep_os(1);
	}
	return ret;
}

void atcmd_usb_ecm_init(void);
void example_usbh_ecm_thread(void *param)
{
	int ret = 0;
	bool status = false;

	rtw_mutex_init(&mii_tx_mutex);
	rtw_mutex_init(&mii_link_mutex);
	rtw_init_sema(&ecm_attach_sema, 0);
	rtw_init_sema(&ecm_detach_sema, 0);

	if (!lwip_init_done) {
		LwIP_Init();
	}

	ret = rtw_create_task(&task_ecm_attach, "ecm_attach_thread", USBH_ECM_THREAD_STACK_SIZE, tskIDLE_PRIORITY + 2, ecm_attach_thread, NULL);

	if (ret != pdPASS) {
		printf("\n[ECM] Fail to create USB host ECM thread\n");
		goto EXIT_FAIL_ATTACH;
	}

	ret = rtw_create_task(&task_ecm_detach, "ecm_detach_thread", USBH_ECM_THREAD_STACK_SIZE, tskIDLE_PRIORITY + 2, ecm_detach_thread, NULL);

	if (ret != pdPASS) {
		printf("\n[ECM] Fail to create USB host ECM thread\n");
		goto EXIT_FAIL_DETACH;
	}

	vTaskDelay(100);

	status = ecm_on();//It don't have the usb signal
	atcmd_usb_ecm_init();
	if (status == false) {
		rtw_delete_task(&task_ecm_detach);
		ecm_off();
		ecm_status = ECM_STATUS_DEINIT;
		printf("ecm_status %d\r\n", ecm_status);
		goto EXIT_FAIL_DETACH;
	} else {
		ecm_status = ECM_STATUS_START;
		printf("ecm_status %d\r\n", ecm_status);
	}
	goto EXIT;
EXIT_FAIL_DETACH:
	rtw_delete_task(&task_ecm_attach);
EXIT_FAIL_ATTACH:
	rtw_mutex_free(&mii_tx_mutex);
	rtw_mutex_free(&mii_link_mutex);
	rtw_free_sema(&ecm_attach_sema);
	rtw_free_sema(&ecm_detach_sema);
EXIT:
	vTaskDelete(NULL);
}

int ethernet_ecm_status(void)
{
	return ecm_status;
}
int ethernet_is_linked(void)
{
	return (int)usbd_cdc_ecm_ethernt_status();
}

int ethernet_is_unplug(void)
{
	return (int)usbd_cdc_ecm_ethernt_status();
}

void ethernet_usb_init(void)
{
	int ret;
	struct task_struct task;

	printf("\n[ECM] USB host ECM demo started...\n");

	ret = rtw_create_task(&task, "example_usbh_ecm_thread", USBH_ECM_THREAD_STACK_SIZE, tskIDLE_PRIORITY + 2, example_usbh_ecm_thread, NULL);
	if (ret != pdPASS) {
		printf("\n[MSC] Fail to create USB host ECM thread\n");
	}
}

void example_usb_ecm_deinit(void)
{
	rtw_up_sema(&ecm_detach_sema);//Trigger to close the ecm driver
	int count = 0;
	ecm_status = ECM_STATUS_DEINIT;
	while (1) {
		count++;
		vTaskDelay(100);
		if (ecm_status == ECM_STATUS_DEINIT) {
			printf("ecm deinit finish\r\n");
			break;
		} else if (count >= 50) {
			printf("timeout 5s\r\n");
			break;
		}
	}
}

void AECMD(void *arg)
{
	if (ecm_status == ECM_STATUS_START) {
		example_usb_ecm_deinit();
	} else {
		printf("It has already deinit\r\n");
	}
}

void AECME(void *arg)
{
	if (ecm_status == ECM_STATUS_DEINIT) {
		ethernet_usb_init();
	} else {
		printf("The example is running\r\n");
	}
}

log_item_t usb_ecm_items[] = {
	{"ECMD", AECMD,},
	{"ECME", AECME,},
};

void atcmd_usb_ecm_init(void)
{
	log_service_add_table(usb_ecm_items, sizeof(usb_ecm_items) / sizeof(usb_ecm_items[0]));
}
#endif