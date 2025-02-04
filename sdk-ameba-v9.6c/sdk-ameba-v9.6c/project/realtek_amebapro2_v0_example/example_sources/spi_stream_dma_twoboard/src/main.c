#include <string.h>
#include "spi_api.h"
#include "spi_ex_api.h"
#include "wait_api.h"
#include "sys_api.h"

#define SPI_IS_AS_MASTER    1
#define TEST_BUF_SIZE       2048
#define SCLK_FREQ           1000000

// SPI0 (S0)
#define SPI0_MOSI  PE_3
#define SPI0_MISO  PE_2
#define SPI0_SCLK  PE_1
#define SPI0_CS    PE_4

extern void hal_ssi_toggle_between_frame(phal_ssi_adaptor_t phal_ssi_adaptor, u8 ctl);

void dump_data(const u8 *start, u32 size, char *strHeader)
{
	int row, column, index, index2, max;
	u8 *buf, *line;

	if (!start || (size == 0)) {
		return;
	}

	line = (u8 *)start;

	//16 bytes per line
	if (strHeader) {
		dbg_printf("%s", strHeader);
	}

	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16) {
		buf = (u8 *)line;

		max = (index == row - 1) ? column : 16;
		if (max == 0) {
			break; /* If we need not dump this line, break it. */
		}

		dbg_printf("\r\n[%08x] ", line);

		//Hex
		for (index2 = 0; index2 < max; index2++) {
			if (index2 == 8) {
				dbg_printf("  ");
			}
			dbg_printf("%02x ", (u8)buf[index2]);
		}

		if (max != 16) {
			if (max < 8) {
				dbg_printf("  ");
			}
			for ((index2 = 16 - max); index2 > 0; index2--) {
				dbg_printf("   ");
			}
		}
	}

	dbg_printf("\r\n");
	return;
}

char TestBuf[TEST_BUF_SIZE]__attribute__((aligned(32))) = {0};
volatile int TrDone;


void master_tr_done_callback(void *pdata, SpiIrq event)
{
	TrDone = 1;
}

void slave_tr_done_callback(void *pdata, SpiIrq event)
{
	TrDone = 1;
}

#if SPI_IS_AS_MASTER
spi_t spi_master;
#else
spi_t spi_slave;
#endif



int main(void)
{
//    int TestingTimes = 10;
//    int Counter      = 0;
//    int TestData     = 0;
	int i;

	dbg_printf("\r\n   SPI Stream DMA Twoboard DEMO   \r\n");

#if SPI_IS_AS_MASTER
	spi_init(&spi_master, SPI0_MOSI, SPI0_MISO, SPI0_SCLK, SPI0_CS);
	spi_format(&spi_master, 16, ((int)SPI_SCLK_IDLE_LOW | (int)SPI_SCLK_TOGGLE_MIDDLE), 0);
	spi_frequency(&spi_master, SCLK_FREQ);
	hal_ssi_toggle_between_frame(&spi_master.hal_ssi_adaptor, ENABLE);

	for (i = 0; i < TEST_BUF_SIZE; i++) {
		TestBuf[i] = i;
	}

	spi_irq_hook(&spi_master, (spi_irq_handler)master_tr_done_callback, (uint32_t)&spi_master);
	dbg_printf("SPI Master Write Test==> \r\n");
	TrDone = 0;
	spi_master_write_stream_dma(&spi_master, TestBuf, TEST_BUF_SIZE);
	i = 0;
	dbg_printf("SPI Master Wait Write Done... \r\n");
	while (TrDone == 0) {
		wait_ms(10);
		i++;
	}
	dbg_printf("SPI Master Write Done!! \r\n");

	dbg_printf("SPI Master Read Test==> \r\n");
	dbg_printf("Wait 5 sec for SPI Slave get ready... \r\n");
	for (i = 0; i < 5; i++) {
		wait_ms(1000);
	}

	memset(TestBuf, 0, TEST_BUF_SIZE);
	spi_flush_rx_fifo(&spi_master);

	TrDone = 0;
	spi_master_read_stream_dma(&spi_master, TestBuf, TEST_BUF_SIZE);
	i = 0;
	dbg_printf("SPI Master Wait Read Done... \r\n");
	while (TrDone == 0) {
		wait_ms(10);
		i++;
	}
	dbg_printf("SPI Master Read Done!! \r\n");
	dump_data((const u8 *)TestBuf, TEST_BUF_SIZE, (char *)"SPI Master Read Data:");

	spi_free(&spi_master);
	dbg_printf("SPI Master Test <== \r\n");

#else
	spi_init(&spi_slave, SPI0_MOSI, SPI0_MISO, SPI0_SCLK, SPI0_CS);
	spi_format(&spi_slave, 16, ((int)SPI_SCLK_IDLE_LOW | (int)SPI_SCLK_TOGGLE_MIDDLE), 1);
	hal_ssi_toggle_between_frame(&spi_slave.hal_ssi_adaptor, ENABLE);

	memset(TestBuf, 0, TEST_BUF_SIZE);
	dbg_printf("SPI Slave Read Test ==> \r\n");
	spi_irq_hook(&spi_slave, (spi_irq_handler)slave_tr_done_callback, (uint32_t)&spi_slave);
	TrDone = 0;
	spi_flush_rx_fifo(&spi_slave);
	spi_slave_read_stream_dma(&spi_slave, TestBuf, TEST_BUF_SIZE);
	i = 0;
	dbg_printf("SPI Slave Wait Read Done... \r\n");
	while (TrDone == 0) {
		wait_ms(100);
		i++;
		if (i > 150) {
			dbg_printf("SPI Slave Wait Timeout \r\n");
			break;
		}
	}
	dump_data((const u8 *)TestBuf, TEST_BUF_SIZE, (char *)"SPI Slave Read Data:");

	// Slave Write Test
	dbg_printf("SPI Slave Write Test ==> \r\n");
	TrDone = 0;
	spi_slave_write_stream_dma(&spi_slave, TestBuf, TEST_BUF_SIZE);
	i = 0;
	dbg_printf("SPI Slave Wait Write Done... \r\n");
	while (TrDone == 0) {
		wait_ms(100);
		i++;
		if (i > 200) {
			dbg_printf("SPI Slave Write Timeout... \r\n");
			break;
		}
	}
	dbg_printf("SPI Slave Write Done!! \r\n");
	
	spi_free(&spi_slave);
#endif

	dbg_printf("SPI Demo finished. \r\n");
	for (;;);
}
