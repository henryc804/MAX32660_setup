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
#include "i2c.h"

/* I2C mapping */
#define I2C_IRQn		I2C1_IRQn
static int i2c_slave_addr = I2C_DEF_SLAVE_ADDR;

static i2c_req_t i2c_slave_req;

void i2c_callback(i2c_req_t *req, int error);

//Slave interrupt handler
void I2C1_IRQHandler(void)
{
	I2C_Handler(I2C_SLAVE);
	return;
}

void i2c_slave_cb_init(void)
{
	int error;

	//Prepare SlaveAsync
	i2c_slave_req.addr = i2c_slave_addr;
	i2c_slave_req.tx_data = mxm_get_tx_buf_pointer();
	i2c_slave_req.tx_len = 32;
	i2c_slave_req.rx_data = mxm_get_rx_buf_pointer();
	i2c_slave_req.rx_len = FLC_PAGE_SIZE + CHECKBYTE_16 + 2;
	i2c_slave_req.restart = 0;
	i2c_slave_req.callback = i2c_callback;
	if ((error = I2C_SlaveAsync(I2C_SLAVE, &i2c_slave_req)) != E_NO_ERROR) {
		uart_puts("\r\nError starting async read ");
		uart_print_val(error);
	}
}

void i2c_callback(i2c_req_t *req, int error) {
	uint8_t *tx_buf = mxm_get_tx_buf_pointer();
	uint8_t *rx_buf = mxm_get_rx_buf_pointer();
	mxm_set_interface(I2C_INTERFACE);
	uart_puts("\r\n ** i2c_callback ");
	if (req->rx_num > 0) {
		tx_buf[0] = BL_ERR_UNAVAIL_CMD; // The command is not supported, initial value.
		if (bl_msg.fam_byte_received == false) {
			if (mxm_bl_find_fam_cmd(rx_buf) == 0) {
				bl_msg.fam_byte_received = true;

			} else {
				tx_buf[0] = BL_ERR_UNAVAIL_CMD; // The command is not supported, initial value.
				uart_puts("\r\n ** CMD Not found in table. ");
				uart_print_hex(rx_buf[0]);
				uart_print_hex(rx_buf[1]);
			}
		}
		if (bl_msg.fam_byte_received == true) {
			int bl_i2c_tx_len;

			if (bl_msg.cb) {
				if ((req->rx_num - 2) != bl_msg.payload_len) {
					uart_puts("\r\n expected: ");
					uart_print_val(bl_msg.payload_len);
					uart_puts("\r\n got: ");
					uart_print_val((req->rx_num - 2));
				}

				(*bl_msg.cb)(&rx_buf[2], (req->rx_num - 2), tx_buf, &bl_i2c_tx_len);
			}

			bl_msg.fam_byte_received = false;
			bl_msg.cb = NULL;
		}
	}

	i2c_slave_cb_init();
	return;
}

int mxm_bl_i2c_init(void)
{
	const sys_cfg_t sys_i2c_cfg = NULL; /* No system specific configuration needed. */
	boot_config_t* boot_config = get_pointer_to_bootconfig();


	if (check_input_value_range(boot_config->i2c_addr,
					CONFIG_I2C_ADDR_LOWER_LIMIT, CONFIG_I2C_ADDR_UPPER_LIMIT)) {
		i2c_slave_addr = boot_config->i2c_addr << 1;
	} else {
		i2c_slave_addr = I2C_DEF_SLAVE_ADDR << 1;
	}

	//Setup the I2CS
	I2C_Shutdown(I2C_SLAVE);
	if (I2C_Init(I2C_SLAVE, I2C_FASTPLUS_MODE, &sys_i2c_cfg) != E_NO_ERROR) {
		uart_puts("\r\nError initializing I2C");
		return 1;
	}
	NVIC_EnableIRQ(I2C_IRQn);
	i2c_slave_cb_init();
	return 0;
}

void mxm_bl_i2c_shutdown(void)
{
	int error;

	if ((error = I2C_Shutdown(I2C_SLAVE)) != E_NO_ERROR) {
		uart_puts("\r\nError I2C Shutdown err:");
		uart_print_val(error);
	}
	NVIC_DisableIRQ(I2C_IRQn);
	MXC_GCR->rstr1 |= (MXC_F_GCR_RSTR1_I2C1);
}
