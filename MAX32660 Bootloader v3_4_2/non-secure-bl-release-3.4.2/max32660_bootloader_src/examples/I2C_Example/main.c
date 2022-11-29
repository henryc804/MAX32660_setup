/*============================================================================
 *
 *
 * Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
 * This software is protected by copyright laws of the United States and of
 * foreign countries. This material may also be protected by patent laws and
 * technology transfer regulations of the United States and of foreign
 * countries. This software is furnished under a license agreement and/or a
 * nondisclosure agreement and may only be used or reproduced in accordance
 * with the terms of those agreements. Dissemination of this information to
 * any party or parties not specified in the license agreement and/or
 * nondisclosure agreement is expressly prohibited.
 *
 * The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated Products,
 * Inc. shall not be used except as stated in the Maxim Integrated Products,
 * Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses of trade
 * secrets, proprietary technology, copyrights, patents, trademarks, maskwork
 * rights, or any other form of intellectual property whatsoever.
 * Maxim Integrated Products, Inc. retains all ownership rights.
 *
 *
 *==========================================================================*/

/**
 * @file    main.c
 * @brief   I2C Example!
 * @details This example flashes an LED and returns answer to some I2C Requests.
 */

/***** Includes *****/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mxc_config.h"
#include "mxc_delay.h"
#include "i2c.h"
#include "led.h"
#include "tmr.h"
#include "tmr_utils.h"

/***** Definitions *****/
#define I2C_IRQn				I2C1_IRQn
#define I2C_SLAVE	    		MXC_I2C1
#define I2C_SLAVE_ADDR          0xAA

/***** Globals *****/
uint8_t tx_buf[32];
uint8_t rx_buf[256];
i2c_req_t i2c_slave_req;
const uint8_t expected_data[] = { 0x81, 0x01 };
const uint8_t response_data[] = { 0xAA, 0x10, 0x10 };

/***** Functions *****/
void i2c_callback(i2c_req_t *req, int error);

void i2c_slave_cb_init(void)
{
	//Prepare SlaveAsync
	i2c_slave_req.addr = I2C_SLAVE_ADDR;
	i2c_slave_req.tx_data = tx_buf;
	i2c_slave_req.tx_len = 32;
	i2c_slave_req.rx_data = rx_buf;
	i2c_slave_req.rx_len = 256;
	i2c_slave_req.restart = 0;
	i2c_slave_req.callback = i2c_callback;
	if (I2C_SlaveAsync(I2C_SLAVE, &i2c_slave_req) != E_NO_ERROR) {
		printf("\r\nError starting async read\n");
	}
}

int i2c_init(void)
{
	const sys_cfg_t sys_i2c_cfg = NULL; /* No system specific configuration needed. */

	//Setup the I2CS
	I2C_Shutdown(I2C_SLAVE);
	if (I2C_Init(I2C_SLAVE, I2C_FAST_MODE, &sys_i2c_cfg) != E_NO_ERROR) {
		printf("\r\nError initializing I2C\n");
		return 1;
	}
	MXC_I2C1->ctrl |= MXC_S_I2C_CTRL_SCL_CLK_STRECH_DIS_DIS;
	MXC_I2C1->timeout = 0;
	NVIC_ClearPendingIRQ(I2C_IRQn);
	NVIC_EnableIRQ(I2C_IRQn);
	i2c_slave_cb_init();
	return 0;
}

int main(void)
{
    printf("Bootloader - I2C Example\n");
    __enable_irq();
    i2c_init();

    while(1) {
        LED_On(0);

        /* Demonstrates the TMR driver delay */
        TMR_Delay(MXC_TMR1, MSEC(500), NULL);
        LED_Off(0);

        /* Demonstrates the SysTick-based convenience delay functions */
	    mxc_delay(MXC_DELAY_MSEC(500));

    }
}

void i2c_callback(i2c_req_t *req, int error) {
    uint8_t expected_len = sizeof(expected_data) / sizeof(*expected_data);
    memset(tx_buf, 0, sizeof(tx_buf) / sizeof(*tx_buf));
	if (req->rx_num == expected_len) {
        if (memcmp(expected_data, rx_buf, expected_len) == 0) {
            uint8_t transfer_len = sizeof(response_data) / sizeof(*response_data);
            memcpy(tx_buf, response_data, transfer_len);
	        i2c_slave_req.tx_len = transfer_len;
        }
	}

	i2c_slave_cb_init();
	return;
}

void I2C1_IRQHandler(void)
{
	I2C_Handler(I2C_SLAVE);
}
