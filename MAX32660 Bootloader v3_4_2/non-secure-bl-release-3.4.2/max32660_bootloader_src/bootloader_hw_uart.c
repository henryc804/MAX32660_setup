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
#include "uart.h"

void bl_uart_cb(uart_req_t* req, int error);
static uart_req_t uart_req;

void init_uart_cb(int len)
{
	int error;

    uart_req.data = mxm_get_rx_buf_pointer();
    uart_req.len = len;
    uart_req.callback = bl_uart_cb;

    error = UART_ReadAsync(MXC_UART_GET_UART(BL_UART_PORT), &uart_req);
    if (error != E_NO_ERROR) {
		uart_puts("\r\n ** Error starting async read: ");
		uart_print_val(error);
        while(1) {}
    }
}


void bl_uart_cb(uart_req_t* req, int error)
{
	if (error == E_SHUTDOWN || error == E_ABORT) {
		return;
	}

	/* If there is an error, request the same amount of data from UART again. */
	if (error) {
		if (bl_msg.fam_byte_received && bl_msg.payload_len != 0) {
			init_uart_cb(bl_msg.payload_len);
		} else {
			init_uart_cb(BL_CMD_LEN);
		}
		return;
	}
	mxm_set_interface(UART_INTERFACE);
	if (bl_msg.fam_byte_received == false) {
		if (mxm_bl_find_fam_cmd(req->data) == 0) {
			bl_msg.fam_byte_received = true;

			/* Next bytes to read */
			if (bl_msg.payload_len != 0) {
				init_uart_cb(bl_msg.payload_len);
				return;
			} else {
				uint8_t bl_uart_tx[128];
				int bl_uart_tx_len = sizeof(bl_uart_tx);
				if (bl_msg.cb) {
					/* No payload for this call*/
					(*bl_msg.cb)(req->data + 2, req->num - 2, bl_uart_tx, &bl_uart_tx_len);
					if (bl_uart_tx_len) {
						UART_Write(MXC_UART_GET_UART(BL_UART_PORT), bl_uart_tx, bl_uart_tx_len);
						uart_print_array("TX: ", bl_uart_tx, bl_uart_tx_len);
					}

					bl_msg.fam_byte_received = false;
				}
			}
		} else {
			uint8_t response = BL_ERR_UNAVAIL_CMD;
			UART_Write(MXC_UART_GET_UART(BL_UART_PORT), &response, sizeof(response));
			uart_puts("\r\n ** CMD Not found in table. ");
			uart_print_hex(req->data[0]);
			uart_print_hex(req->data[1]);
		}
		init_uart_cb(BL_CMD_LEN);
	} else {
		uint8_t bl_uart_tx[128];
		int bl_uart_tx_len = sizeof(bl_uart_tx);

		if (bl_msg.cb) {
			if (req->num != bl_msg.payload_len) {
				uart_puts("\r\n expected: ");
				uart_print_val(bl_msg.payload_len);
				uart_puts("\r\n got: ");
				uart_print_val(req->num);
			}

			(*bl_msg.cb)(req->data, req->num, bl_uart_tx, &bl_uart_tx_len);
			if (bl_uart_tx_len) {
				UART_Write(MXC_UART_GET_UART(BL_UART_PORT), bl_uart_tx, bl_uart_tx_len);
				uart_print_array("TX: ", bl_uart_tx, bl_uart_tx_len);
			}
		}

		bl_msg.fam_byte_received = false;
		bl_msg.cb = NULL;
		bl_msg.payload_len = 0;
		init_uart_cb(BL_CMD_LEN);
	}
}

int mxm_bl_uart_init(void)
{
    int error;
	static const uart_cfg_t bl_uart_cfg = {
		UART_PARITY_DISABLE,
		UART_DATA_SIZE_8_BITS,
		UART_STOP_1,
		UART_FLOW_CTRL_DIS,
		UART_FLOW_POL_DIS,
		BL_UART_BAUD
	};

	static const sys_cfg_uart_t bl_uart_sys_cfg = {
		MAP_A,
		UART_FLOW_DISABLE,
	};

    /* Setup the interrupt */
    NVIC_ClearPendingIRQ(MXC_UART_GET_IRQ(BL_UART_PORT));
    NVIC_DisableIRQ(MXC_UART_GET_IRQ(BL_UART_PORT));
    NVIC_SetPriority(MXC_UART_GET_IRQ(BL_UART_PORT), 1);
    NVIC_EnableIRQ(MXC_UART_GET_IRQ(BL_UART_PORT));
    /* Initialize the UART */
    error = UART_Init(MXC_UART_GET_UART(BL_UART_PORT), &bl_uart_cfg, &bl_uart_sys_cfg);
    if (error != E_NO_ERROR) {
        uart_puts("\r\n ** Error initializing UART ");
        uart_print_val(error);
        while(1) {}
    }

	/* Initial uart callback */
    bl_msg.fam_byte_received = false;
	init_uart_cb(BL_CMD_LEN);

	return 0;
}

void mxm_bl_uart_shutdown(void)
{
	int error;
	sys_periph_clock_t uart_clock;

	if ((error = UART_Shutdown(MXC_UART_GET_UART(BL_UART_PORT))) != E_NO_ERROR) {
		uart_puts("\r\nError UART Shutdown err:");
		uart_print_val(error);
	}
	NVIC_SetPriority(MXC_UART_GET_IRQ(BL_UART_PORT), 0);
	NVIC_DisableIRQ(MXC_UART_GET_IRQ(BL_UART_PORT));
	if (MXC_UART_GET_UART(BL_UART_PORT) == MXC_UART0) {
		uart_clock = SYS_PERIPH_CLOCK_UART0;
	} else if (MXC_UART_GET_UART(BL_UART_PORT) == MXC_UART1) {
		uart_clock = SYS_PERIPH_CLOCK_UART1;
	}
	MXC_GCR->rstr0 |= (MXC_F_GCR_RSTR0_UART0 | MXC_F_GCR_RSTR0_UART1);
	SYS_ClockEnable(uart_clock);
}

int mxm_bl_uart_write(const uint8_t* data, int len)
{
	return UART_Write(MXC_UART_GET_UART(BL_UART_PORT), data, len);
}
