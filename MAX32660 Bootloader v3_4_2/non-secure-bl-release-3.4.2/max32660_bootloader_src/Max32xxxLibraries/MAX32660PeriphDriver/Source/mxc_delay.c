/* *****************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
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
 * $Id: mxc_delay.c 36202 2018-07-16 21:06:02Z michael.bayern $
 *
 *************************************************************************** */

/* **** Includes **** */
#include <stdint.h>
#include "mxc_config.h"
#include "mxc_delay.h"

/* **** File Scope Variables **** */
static volatile int overflows = -1;
static uint32_t endtick;
static uint32_t ctrl_save;

static void mxc_delay_init(unsigned long us);
extern void SysTick_Handler(void);

/* ************************************************************************** */
__weak void SysTick_Handler(void) 
{
    mxc_delay_handler();
}

/* ************************************************************************** */
void mxc_delay_handler(void)
{
    // Check and clear overflow flag
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
        // Decrement overflow flag if delay is still ongoing
        if (overflows > 0) {
            overflows--;
        } else {
            mxc_delay_stop();
        }
    }
}

/* ************************************************************************** */
static void mxc_delay_init(unsigned long us)
{
    uint32_t starttick, reload, ticks, lastticks;
    
    // Record the current tick value and clear the overflow flag
    starttick = SysTick->VAL;
    
    // Save the state of control register (and clear the overflow flag)
    ctrl_save = SysTick->CTRL & ~SysTick_CTRL_COUNTFLAG_Msk;
    
    // If the SysTick is not running, configure and start it
    if (!(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)) {
        SysTick->LOAD = SysTick_LOAD_RELOAD_Msk;
        SysTick->VAL = SysTick_VAL_CURRENT_Msk;
        SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
        starttick = SysTick_VAL_CURRENT_Msk;
        reload = SysTick_LOAD_RELOAD_Msk + 1;
    } else {
        reload = SysTick->LOAD + 1; // get the current reload value
    }
    
    // Calculate the total number of ticks to delay
    ticks = (uint32_t)(((uint64_t)us * (uint64_t)SystemCoreClock) / 1000000);
    
    // How many overflows of the SysTick will occur
    overflows = ticks / reload;
    
    // How many remaining ticks after the last overflow
    lastticks = ticks % reload;
    
    // Check if there will be another overflow due to the current value of the SysTick
    if (lastticks >= starttick) {
        overflows++;
        endtick = reload - (lastticks - starttick);
    } else {
        endtick = starttick - lastticks;
    }
}

/* ************************************************************************** */
int mxc_delay_start(unsigned long us)
{
    // Check if timeout currently ongoing
    if (overflows > 0) {
        return E_BUSY;
    }
    
    // Check if there is nothing to do
    if (us == 0) {
        return E_NO_ERROR;
    }
    
    // Calculate the necessary delay and start the timer
    mxc_delay_init(us);
    
    // Enable SysTick interrupt if necessary
    if (overflows > 0) {
        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    }
    
    return E_NO_ERROR;
}

/* ************************************************************************** */
int mxc_delay_check(void)
{
    // Check if timeout currently ongoing
    if (overflows < 0) {
        return E_NO_ERROR;
    }
    
    // Check the global values
    if ((overflows == 0) && (SysTick->VAL <= endtick)) {
        mxc_delay_stop();
        return E_NO_ERROR;
    }
    
    return E_BUSY;
}

/* ************************************************************************** */
void mxc_delay_stop(void)
{
    SysTick->CTRL = ctrl_save;
    overflows = -1;
}

/* ************************************************************************** */
int mxc_delay(unsigned long us)
{
    // Check if timeout currently ongoing
    if (overflows > 0) {
        return E_BUSY;
    }
    
    // Check if there is nothing to do
    if (us == 0) {
        return E_NO_ERROR;
    }
    
    // Calculate the necessary delay and start the timer
    mxc_delay_init(us);
    
    // Wait for the number of overflows
    while (overflows > 0) {
        // If SysTick interrupts are enabled, COUNTFLAG will never be set here and
        // overflows will be decremented in the ISR. If SysTick interrupts are
        // disabled, overflows is decremented here.
        if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
            overflows--;
        }
    }
    
    // Wait for the counter value
    while (SysTick->VAL > endtick);
    
    mxc_delay_stop();
    
    return E_NO_ERROR;
}
