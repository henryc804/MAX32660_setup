/*******************************************************************************
 * Copyright (C) 2015 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
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
 *
 * $Date: 2017-08-10 11:01:15 -0500 (Thu, 10 Aug 2017) $
 * $Revision: 29282 $
 *
 ******************************************************************************/

#include <stdio.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "mxc_assert.h"
#include "board.h"
#include "uart.h"
#include "gpio.h"
#include "mxc_pins.h"
#include "led.h"
#include "pb.h"

/***** Global Variables *****/
extern uint32_t SystemCoreClock;

const gpio_cfg_t pb_pin[] = {
    //{PORT_0, PIN_12, GPIO_FUNC_IN, GPIO_PAD_PULL_UP},
};
const unsigned int num_pbs = (sizeof(pb_pin) / sizeof(gpio_cfg_t));

const gpio_cfg_t led_pin[] = {
    //{PORT_0, PIN_13, GPIO_FUNC_OUT, GPIO_PAD_NONE},
};
const unsigned int num_leds = (sizeof(led_pin) / sizeof(gpio_cfg_t));

/***** File Scope Variables *****/
static const uart_cfg_t uart_cfg = {
    UART_PARITY_DISABLE,
    UART_DATA_SIZE_8_BITS,
    UART_STOP_1,
    UART_FLOW_CTRL_DIS,
    UART_FLOW_POL_DIS,
    CONSOLE_BAUD
};
static const sys_cfg_uart_t uart_sys_cfg = {
    MAP_A,
    UART_FLOW_DISABLE,
};

/******************************************************************************/
void mxc_assert(const char *expr, const char *file, int line)
{
    printf("MXC_ASSERT %s #%d: (%s)\n", file, line, expr);
    while (1);
}

/******************************************************************************/
#if 0
int Board_Init(void)
{
    int err;

    if ((err = Console_Init(CONSOLE_UART)) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

	/* Additional debug port */
    if ((err = Console_Init(0)) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    if ((err = PB_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    if ((err = LED_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    return E_NO_ERROR;
}
#endif

/******************************************************************************/
int Console_Init(int uart_num)
{
    int err;

    if ((err = UART_Init(MXC_UART_GET_UART(uart_num), &uart_cfg, &uart_sys_cfg)) != E_NO_ERROR) {
        return err;
    }

	// Setup the interrupt
	NVIC_ClearPendingIRQ(MXC_UART_GET_IRQ(uart_num));
	NVIC_DisableIRQ(MXC_UART_GET_IRQ(uart_num));
	NVIC_SetPriority(MXC_UART_GET_IRQ(uart_num), 1);
	NVIC_EnableIRQ(MXC_UART_GET_IRQ(uart_num));

	MXC_UART_GET_UART(uart_num)->int_en |= MXC_F_UART_INT_EN_RX_FIFO_THRESH | MXC_F_UART_INT_EN_RX_FRAME_ERROR;

    return E_NO_ERROR;
}
/******************************************************************************/
int Console_Shutdown(int uart_num)
{
    int err;

    if ((err = UART_Shutdown(MXC_UART_GET_UART(uart_num))) != E_NO_ERROR) {
        return err;
    }

    return E_NO_ERROR;
}
/******************************************************************************/
__weak void NMI_Handler(void)
{
    __NOP();
}

#if defined(BOOTLOADER_UART) && !defined(BL_USE_BITBANG_UART)
void UART0_IRQHandler(void)
{
	UART_Handler(MXC_UART_GET_UART(0));
}

void UART1_IRQHandler(void)
{
    UART_Handler(MXC_UART_GET_UART(1));
}
#endif

