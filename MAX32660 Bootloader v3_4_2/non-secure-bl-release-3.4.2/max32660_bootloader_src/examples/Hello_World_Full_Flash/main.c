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
 * @brief   Hello World Full Flash!
 * @details This example uses the UART to print to a terminal and flashes an LED.
 * 			The aim of this example is to generate an application covering all Flash memory.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "mxc_delay.h"
#include "led.h"
#include "tmr.h"
#include "tmr_utils.h"

/***** Definitions *****/

/***** Globals *****/

/***** Functions *****/

void init_big_table(void);
//******************************************************************************

const gpio_cfg_t gpio_0_9 = {PORT_0, PIN_9, GPIO_FUNC_OUT, GPIO_PAD_NONE};

#if defined (__ICCARM__)
#pragma section=".text"
#endif
#if defined ( __GNUC__ )
__attribute__ ((section(".text")))
#endif
static const char huge_table[150000] = {"Maxim Integrated Products Inc."};

#if defined (__ICCARM__)
#pragma section=".app_sign"
#endif
#if defined ( __GNUC__ )
__attribute__ ((section (".app_sign"))) __attribute__ ((__used__))
#endif
static volatile char huge_app_sign[FULL_FLASH_APP_SIGN_SIZE] = {"FULL_APPLICATION"};

void init_big_table(void)
{
	printf("Init big table to fill up flash memory. Initial text: %.30s\n", huge_table);
}

int main(void)
{
    int count = 0;

    printf("Hello World from main application!\n");
    // Prevent optimization
    for (int i=0; i < FULL_FLASH_APP_SIGN_SIZE; i++) {
		huge_app_sign[i] = huge_app_sign[i];
	}	
	init_big_table();
	GPIO_Config(&gpio_0_9);
    while(1) {
        LED_On(0);
		GPIO_OutClr(&gpio_0_9);

        /* Demonstrates the TMR driver delay */
        TMR_Delay(MXC_TMR0, MSEC(500), NULL);
        LED_Off(0);
		GPIO_OutSet(&gpio_0_9);

        /* Demonstrates the SysTick-based convenience delay functions */
	    mxc_delay(MXC_DELAY_MSEC(500));
        printf("count = %d\n", count++);
    }
}
