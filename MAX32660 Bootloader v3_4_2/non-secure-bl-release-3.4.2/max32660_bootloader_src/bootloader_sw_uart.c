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
#include "tmr.h"
#include "icc.h"

#define SERIAL_DATA_BITS (8)

typedef enum {
	SERIAL_START,
	SERIAL_EXCHANGE,
	SERIAL_STOP,
	SERIAL_END,
	SERIAL_IDLE
} serial_state_t;

typedef struct {
	uint8_t error;
	uint8_t *data;
	uint32_t data_len;
	uint8_t bit_offset;
	uint32_t data_offset;
	uint8_t xchg_data;
	serial_state_t state;
	void (*callback)(int);
} sw_serial_buf_t;

volatile uint32_t rx_int_delay = 0;
static gpio_cfg_t tx_pin_cfg = { PORT_0, PIN_8, GPIO_FUNC_OUT, GPIO_PAD_NONE };
static gpio_cfg_t rx_pin_cfg = { PORT_0, PIN_9, GPIO_FUNC_IN, GPIO_PAD_NONE };

static mxc_tmr_regs_t *uart_tx_tmr = MXC_TMR1;
static mxc_tmr_regs_t *uart_rx_tmr = MXC_TMR2;
volatile static sw_serial_buf_t rx_serial_buf, tx_serial_buf;

void bl_uart_cb(int data_len);
void init_uart_cb(int len);

int mxm_bl_sw_uart_init(int baud)
{
	uint32_t tmr_ticks = 0;
	tmr_cfg_t tmr_cfg = { TMR_MODE_CONTINUOUS, 0, 0 };

	if (baud > 500000) {
		return E_BAD_PARAM;
	}

	switch (baud) {
	case 115200:
		rx_int_delay = 101;
		break;
	default:
		/* At common values below 115200 the latency becomes insignificant */
		rx_int_delay = 0;
	}

	rx_serial_buf.state = SERIAL_IDLE;
	tx_serial_buf.state = SERIAL_IDLE;

	NVIC_DisableIRQ(GPIO0_IRQn);
	GPIO_OutSet(&tx_pin_cfg);
	GPIO_Config(&tx_pin_cfg);
	GPIO_Config(&rx_pin_cfg);

	/* Enable interrupts to detect RX start condition */
	GPIO_IntConfig(&rx_pin_cfg, GPIO_INT_EDGE, GPIO_INT_FALLING);
	GPIO_IntEnable(&rx_pin_cfg);

	tmr_ticks = PeripheralClock / baud;
	tmr_cfg.cmp_cnt = tmr_ticks;
	TMR_Init(uart_tx_tmr, TMR_PRES_1, 0);
	TMR_Init(uart_rx_tmr, TMR_PRES_1, 0);
	TMR_Config(uart_tx_tmr, &tmr_cfg);
	TMR_Config(uart_rx_tmr, &tmr_cfg);
	NVIC_EnableIRQ(TMR1_IRQn);
	NVIC_EnableIRQ(TMR2_IRQn);

	return E_NO_ERROR;
}

int mxm_bl_uart_init(void)
{
	int result;
	ICC_Enable();
	result = mxm_bl_sw_uart_init(115200);

	bl_msg.fam_byte_received = false;
	init_uart_cb(BL_CMD_LEN);
    return result;
}

void mxm_bl_uart_shutdown(void)
{
	TMR_Shutdown(uart_tx_tmr);
	TMR_Shutdown(uart_rx_tmr);
	TMR_SetCount(uart_tx_tmr, 0);
	TMR_SetCount(uart_rx_tmr, 0);
	TMR_SetCompare(uart_tx_tmr, 0);
	TMR_SetCompare(uart_rx_tmr, 0);
	NVIC_DisableIRQ(GPIO0_IRQn);
	NVIC_DisableIRQ(TMR1_IRQn);
	NVIC_DisableIRQ(TMR2_IRQn);
	// Reset GPIOs and Timers
	MXC_GCR->rstr0 |= (MXC_F_GCR_RSTR0_GPIO0 | MXC_F_GCR_RSTR0_TIMER1 | MXC_F_GCR_RSTR0_TIMER2);
	ICC_Disable();
}

void GPIO0_IRQHandler(void)
{
	if (GPIO_IntStatus(&rx_pin_cfg)) {
		NVIC_DisableIRQ(GPIO0_IRQn);
		uart_rx_tmr->cn = MXC_F_TMR_CN_TEN | MXC_S_TMR_CN_TMODE_CONTINUOUS;
	}
}


void TMR1_IRQHandler(void)
{
	switch (tx_serial_buf.state) {
	case SERIAL_IDLE:
		break;
	case SERIAL_START:
		GPIO_OutClr(&tx_pin_cfg);
		TMR_IntClear(uart_tx_tmr);
		TMR_SetCount(uart_tx_tmr, 0);
		tx_serial_buf.state = SERIAL_EXCHANGE;
		break;
	case SERIAL_EXCHANGE:
		if (tx_serial_buf.xchg_data & 0x1) {
			GPIO_OutSet(&tx_pin_cfg);
		} else {
			GPIO_OutClr(&tx_pin_cfg);
		}
		TMR_IntClear(uart_tx_tmr);
		tx_serial_buf.xchg_data >>= 1;
		tx_serial_buf.bit_offset++;
		if (tx_serial_buf.bit_offset >= SERIAL_DATA_BITS) {
			tx_serial_buf.state = SERIAL_STOP;
		}
		break;
	case SERIAL_STOP:
		GPIO_OutSet(&tx_pin_cfg);
		TMR_IntClear(uart_tx_tmr);
		tx_serial_buf.state = SERIAL_END;
		break;
	case SERIAL_END:
		TMR_IntClear(uart_tx_tmr);
		tx_serial_buf.bit_offset = 0;
		tx_serial_buf.data_offset++;
		if (tx_serial_buf.data_offset >= tx_serial_buf.data_len) {
			if (tx_serial_buf.callback != NULL) {
				tx_serial_buf.callback(tx_serial_buf.data_offset);
			}
			tx_serial_buf.data_offset = 0;
			tx_serial_buf.state = SERIAL_IDLE;
			TMR_Disable(uart_tx_tmr);
		} else {
			tx_serial_buf.xchg_data = tx_serial_buf.data[tx_serial_buf.data_offset];
			tx_serial_buf.state = SERIAL_START;
		}
		break;
	}
}

void TMR2_IRQHandler(void)
{
	switch(rx_serial_buf.state) {
	case SERIAL_IDLE:
		break;
	case SERIAL_START:
		TMR_IntClear(uart_rx_tmr);
		if (GPIO_InGet(&rx_pin_cfg)) {
			rx_serial_buf.error = 1;
			rx_serial_buf.state = SERIAL_IDLE;
		}
		rx_serial_buf.state = SERIAL_EXCHANGE;
		break;
	case SERIAL_EXCHANGE:
		if (GPIO_InGet(&rx_pin_cfg)) {
			rx_serial_buf.xchg_data |= (1 << rx_serial_buf.bit_offset);
		}
		rx_serial_buf.bit_offset++;
		TMR_IntClear(uart_rx_tmr);
		if (rx_serial_buf.bit_offset >= SERIAL_DATA_BITS) {
			rx_serial_buf.state = SERIAL_STOP;
			TMR_SetCount(uart_rx_tmr, rx_int_delay + (TMR_GetCompare(uart_rx_tmr) >> 2));
		}
		break;
	case SERIAL_STOP:
		TMR_IntClear(uart_rx_tmr);
		if (!GPIO_InGet(&rx_pin_cfg)) {
			rx_serial_buf.error = 2;
			rx_serial_buf.state = SERIAL_IDLE;
		}
		TMR_Disable(uart_rx_tmr);
		rx_serial_buf.data[rx_serial_buf.data_offset] = rx_serial_buf.xchg_data & 0xFF;
		rx_serial_buf.data_offset++;
		rx_serial_buf.bit_offset = 0;
		rx_serial_buf.xchg_data = 0;
		if (rx_serial_buf.data_offset >= rx_serial_buf.data_len) {
			rx_serial_buf.data_offset = 0;
			rx_serial_buf.state = SERIAL_IDLE;
			if(rx_serial_buf.callback != NULL) {
				rx_serial_buf.callback(rx_serial_buf.data_len);
			}
		} else {
			rx_serial_buf.state = SERIAL_START;
			TMR_SetCount(uart_rx_tmr, rx_int_delay + (TMR_GetCompare(uart_rx_tmr) >> 1));
			GPIO_IntClr(&rx_pin_cfg);
			NVIC_EnableIRQ(GPIO0_IRQn);
		}
		break;
	case SERIAL_END:
		uart_puts("\r\n ** There is no end step in rx serial. \r\n");
		break;
	}
}

int mxm_bl_uart_writeasync(uint8_t *data, int len, void(*callback)(int))
{
	if (data == NULL || len <= 0)
		return E_BAD_PARAM;

	if (tx_serial_buf.state != SERIAL_IDLE)
		return E_BUSY;

	tx_serial_buf.data = data;
	tx_serial_buf.error = 0;
	tx_serial_buf.data_len = len;
	tx_serial_buf.bit_offset = 0;
	tx_serial_buf.data_offset = 0;
	tx_serial_buf.xchg_data = data[0];
	tx_serial_buf.callback = callback;
	tx_serial_buf.state = SERIAL_START;

	TMR_SetCount(uart_tx_tmr, 0);
	TMR_IntClear(uart_tx_tmr);
	TMR_Enable(uart_tx_tmr);
	return E_NO_ERROR;
}

int mxm_bl_uart_write(uint8_t *data, int len)
{
	int result;
	result = mxm_bl_uart_writeasync(data, len, NULL);
	uart_puts("\r\n ** After write async \r\n");
	if (result != E_NO_ERROR)
		return result;

	while(tx_serial_buf.state != SERIAL_IDLE);
	uart_puts("\r\n ** After while state check \r\n");
	return result;
}

int mxm_bl_uart_readasync(uint8_t *data, int len, void(*callback)(int))
{
	if (data == NULL || len <= 0)
		return E_BAD_PARAM;

	if (rx_serial_buf.state != SERIAL_IDLE)
		return E_BUSY;

	rx_serial_buf.data = data;
	rx_serial_buf.error = 0;
	rx_serial_buf.data_len = len;
	rx_serial_buf.bit_offset = 0;
	rx_serial_buf.data_offset = 0;
	rx_serial_buf.xchg_data = 0;
	rx_serial_buf.callback = callback;
	rx_serial_buf.state = SERIAL_START;

	TMR_SetCount(uart_rx_tmr, rx_int_delay + (TMR_GetCompare(uart_rx_tmr) >> 1));
	GPIO_IntClr(&rx_pin_cfg);
	NVIC_EnableIRQ(GPIO0_IRQn);
	return E_NO_ERROR;
}

int mxm_bl_uart_read(uint8_t *data, int len)
{
	int result;
	result = mxm_bl_uart_readasync(data, len, NULL);
	if (result != E_NO_ERROR)
		return result;

	while(rx_serial_buf.state != SERIAL_IDLE);
	return rx_serial_buf.error ? rx_serial_buf.error : result;
}

void init_uart_cb(int len)
{
	int error;

    error = mxm_bl_uart_readasync(mxm_get_rx_buf_pointer(), len, bl_uart_cb);
    if (error != E_NO_ERROR) {
		uart_puts("\r\n ** Error starting async read: ");
		uart_print_val(error);
        while(1) {}
    }
}


void bl_uart_cb(int data_len)
{
	if (data_len < 0) {
		return;
	}

	mxm_set_interface(UART_INTERFACE);
	//uart_print_array("RX: ", req->data, req->num);
	if (bl_msg.fam_byte_received == false) {
		if (mxm_bl_find_fam_cmd(rx_serial_buf.data) == 0) {
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
					(*bl_msg.cb)(rx_serial_buf.data + 2, data_len - 2, bl_uart_tx, &bl_uart_tx_len);
					if (bl_uart_tx_len) {
						mxm_bl_uart_writeasync(bl_uart_tx, bl_uart_tx_len, NULL);
						uart_print_array("TX: ", bl_uart_tx, bl_uart_tx_len);
					}

					bl_msg.fam_byte_received = false;
				}
			}
		} else {
			uint8_t response = BL_ERR_UNAVAIL_CMD;
			mxm_bl_uart_write(&response, sizeof(response));
			uart_puts("\r\n ** CMD Not found in table. \r\n");
			uart_print_hex(rx_serial_buf.data[0]);
			uart_puts("\r\n");
			uart_print_hex(rx_serial_buf.data[1]);
			uart_puts("\r\n");
		}
		init_uart_cb(BL_CMD_LEN);
	} else {
		uint8_t bl_uart_tx[128];
		int bl_uart_tx_len = sizeof(bl_uart_tx);

		if (bl_msg.cb) {
			if (data_len != bl_msg.payload_len) {
				uart_puts("\r\n expected: ");
				uart_print_val(bl_msg.payload_len);
				uart_puts("\r\n got: ");
				uart_print_val(data_len);
			}

			(*bl_msg.cb)(rx_serial_buf.data, data_len, bl_uart_tx, &bl_uart_tx_len);
			if (bl_uart_tx_len) {
				mxm_bl_uart_writeasync(bl_uart_tx, bl_uart_tx_len, NULL);
				uart_print_array("TX: ", bl_uart_tx, bl_uart_tx_len);
			}
		}

		bl_msg.fam_byte_received = false;
		bl_msg.cb = NULL;
		bl_msg.payload_len = 0;
		init_uart_cb(BL_CMD_LEN);
	}
}
