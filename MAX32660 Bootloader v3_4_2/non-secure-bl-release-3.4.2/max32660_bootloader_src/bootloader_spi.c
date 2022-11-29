/*******************************************************************************
* Copyright (C) 2018 Maxim Integrated Products, Inc., All rights Reserved.
*
* This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "bootloader.h"
#include "bootloader_helper.h"
#include "spi.h"

#if BL_SPI_PORT == SPIOA
#define SPI_IRQ 		SPI0_IRQn
#elif BL_SPI_PORT == SPI1A || BL_SPI_PORT == SPI1B
#define SPI_IRQ 		SPI1_IRQn
#endif
#define SPI_SPEED		10000000

spi_req_t bl_spi_slave_req;
static volatile int spis_rd_flag;
static volatile int spis_wr_flag;

unsigned char spi_txbuf[FLC_PAGE_SIZE + CHECKBYTE_16 + 2];
void mxm_bl_spi_slave_cb(spi_req_t *req, int error);
void mxm_bl_spi_slave_write_cb_init(int nbytes);
void mxm_bl_spi_slave_read_cb_init(int nbytes);

void SPI0_IRQHandler(void)
{
    SPI_Handler(SPI0A);
}

void SPI1_IRQHandler(void)
{
    SPI_Handler(SPI1A);
}

void mxm_bl_spi_slave_write_cb_init(int nbytes)
{
	int error;

	// Initialize spi_request struct for slave
	bl_spi_slave_req.tx_data = mxm_get_tx_buf_pointer();
	bl_spi_slave_req.rx_data = NULL;
	bl_spi_slave_req.len = nbytes;

	spis_wr_flag = 1;

	if ((error = SPI_SlaveTransAsync(SPI, &bl_spi_slave_req)) != E_NO_ERROR) {
		uart_puts("\r\nError starting async read. err:");
		uart_print_val(error);
	}
}

void mxm_bl_spi_slave_read_cb_init(int nbytes)
{
	int error;

	// Initialize spi_request struct for slave
	bl_spi_slave_req.tx_data = spi_txbuf;
	bl_spi_slave_req.rx_data = mxm_get_rx_buf_pointer();;
	bl_spi_slave_req.len = nbytes;

	spis_rd_flag = 1;

	if ((error = SPI_SlaveTransAsync(SPI, &bl_spi_slave_req)) != E_NO_ERROR) {
		uart_puts("\r\nError starting async read. err:");
		uart_print_val(error);
	}
}
extern bl_comm_msg_t bl_msg;

void mxm_bl_spi_slave_cb(spi_req_t *req, int error)
{
	uint8_t *tx_buf = mxm_get_tx_buf_pointer();
	uint8_t *rx_buf = mxm_get_rx_buf_pointer();
	spis_rd_flag = 0;
	spis_wr_flag = 0;
	mxm_set_interface(SPI_INTERFACE);
	if (req->rx_num != 0) { /* Data received */
		if (bl_msg.fam_byte_received == false) {
			if (mxm_bl_find_fam_cmd(rx_buf) == 0) {
				bl_msg.fam_byte_received = true;

				/* Next bytes to read */
				if (bl_msg.payload_len != 0) {
					mxm_bl_spi_slave_read_cb_init(bl_msg.payload_len);
					return;
				} else {
					uint8_t bl_tx[128];
					int bl_tx_len = sizeof(bl_tx);
					if (bl_msg.cb) {
						/* No payload for this call*/
						(*bl_msg.cb)(rx_buf + 2, req->rx_num - 2, bl_tx, &bl_tx_len);
						if (bl_tx_len) {
							memcpy(tx_buf, bl_tx, bl_tx_len);
							mxm_bl_spi_slave_write_cb_init(bl_tx_len);
							return;
						}

						bl_msg.fam_byte_received = false;
					}
				}
			} else {
				uint8_t response = BL_ERR_UNAVAIL_CMD;
				memcpy(tx_buf, &response, sizeof(response));
				mxm_bl_spi_slave_write_cb_init(sizeof(response));
				uart_puts("\r\n ** CMD Not found in table. ");
				uart_print_hex(rx_buf[0]);
				uart_print_hex(rx_buf[1]);
				return;
			}
			mxm_bl_spi_slave_read_cb_init(BL_CMD_LEN);
		} else {
			uint8_t bl_tx[128];
			int bl_tx_len = sizeof(bl_tx);

			if (bl_msg.cb) {
				if (req->rx_num != bl_msg.payload_len) {
					uart_puts("\r\n expected: ");
					uart_print_val(bl_msg.payload_len);
					uart_puts("\r\n got: ");
					uart_print_val(req->rx_num);
				}

				(*bl_msg.cb)(rx_buf, req->rx_num, bl_tx, &bl_tx_len);
				if (bl_tx_len) {
					memcpy(tx_buf, bl_tx, bl_tx_len);
					mxm_bl_spi_slave_write_cb_init(bl_tx_len);
					return;
				}
			}

			bl_msg.fam_byte_received = false;
			bl_msg.cb = NULL;
			mxm_bl_spi_slave_read_cb_init(BL_CMD_LEN);
		};
		return;
	}

	// Data TX completed
	if (req->rx_num == 0 && req->tx_num != 0) {
		if(tx_buf[0] == BL_ERR_TRY_AGAIN) {
			mxm_bl_spi_slave_write_cb_init(1);
			return;
		}
		bl_msg.fam_byte_received = false;
		bl_msg.cb = NULL;

		mxm_bl_spi_slave_read_cb_init(BL_CMD_LEN);
		return;
	}
}

int mxm_bl_spi_init(void)
{
	NVIC_EnableIRQ(SPI_IRQ);
	// Configure the peripheral
	if (SPI_Init(SPI, 0, SPI_SPEED) != 0) {
		while (1) {}
	}
	memset(spi_txbuf, 0xff, sizeof(spi_txbuf));

	bl_spi_slave_req.ssel = 0;
	bl_spi_slave_req.ssel_pol = SPI_POL_LOW;// SPI_POL_HIGH;
	bl_spi_slave_req.width = SPI17Y_WIDTH_1;
	bl_spi_slave_req.bits = 8;
	bl_spi_slave_req.tx_num = 0;
	bl_spi_slave_req.rx_num = 0;
	bl_spi_slave_req.callback =  (spi_callback_fn)mxm_bl_spi_slave_cb;

	bl_msg.fam_byte_received = false;
	mxm_bl_spi_slave_read_cb_init(2);
	return 0;
}


void mxm_bl_spi_shutdown(void)
{
	int error;

	if ((error = SPI_Shutdown(SPI)) != E_NO_ERROR) {
		uart_puts("\r\nError SPI Shutdown err:");
		uart_print_val(error);
	}
	NVIC_DisableIRQ(SPI_IRQ);
	MXC_GCR->rstr0 |= (MXC_F_GCR_RSTR0_SPI0);
	NVIC_ClearPendingIRQ(SPI17Y_IRQn);
	SYS_ClockEnable(SYS_PERIPH_CLOCK_SPI17Y);
}
