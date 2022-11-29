/*******************************************************************************
 * Author: Ismail Kose, Ismail.Kose@maximintegrated.com
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
 ******************************************************************************/

/***** Includes *****/
#include "mxc_sys.h"
#include "platform_tick.h"

/***** Definitions *****/
#define USE_RTC_SYSTEM_CLK 0
#define SYSTICK_PERIOD_EXT_CLK 32767    //100ms with 32768Hz external RTC clock

#if MAX32660
#define SYS_RTC_CLK 32769UL
#endif

static uint32_t sysTicks;
static volatile uint32_t sys_tick_ovf = 0;

#if 0//MAX32660
uint32_t SYS_SysTick_GetFreq(void)
{
	/* Determine is using internal (SystemCoreClock) or external (32768) clock */
	if ( (SysTick->CTRL & SysTick_CTRL_CLKSOURCE_Msk) || !(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)) {
		return SystemCoreClock;
	} else {
		return SYS_RTC_CLK;
	}
}
#endif


int32_t platform_tick_init(void)
{
	uint32_t ret = 0;
#if MAX32660
	// If the SysTick is not running, configure and start it
	SysTick_Config(0x00FFFFFF);
	sysTicks = SysTick->LOAD;

#else
	sysTicks = SYSTICK_PERIOD_EXT_CLK;
	ret = SYS_SysTick_Config(sysTicks, USE_RTC_SYSTEM_CLK);
#endif
	return ret;
}

void SysTick_Handler(void)
{
	sys_tick_ovf++;
}

static int enable_debug = 0;

void platform_get_tick_set_debug(int en)
{
	enable_debug = en;
}

uint64_t platform_get_tick(void)
{
	uint32_t usec_tick;
	uint64_t usec_tick_total;
	uint32_t systick_val = SysTick->VAL;
	uint32_t _sys_tick_ovf = sys_tick_ovf;
	uint32_t sys_freq = SYS_SysTick_GetFreq();

	usec_tick = ((uint64_t)(sysTicks - systick_val) * 1000000) / sys_freq;
	if (systick_val == 0) // to protect time overflow
		_sys_tick_ovf -= 1;

	float us_per_ovf = (float)sysTicks * (float)1000000 / (float)sys_freq;
	usec_tick_total = (uint64_t)((float)_sys_tick_ovf * us_per_ovf) + usec_tick;
	return usec_tick_total;
}

uint32_t platform_get_time_usec(void)
{
	uint32_t usec_tick;
	uint64_t usec_tick_total;
	uint32_t systick_val = SysTick->VAL;
	uint32_t _sys_tick_ovf = sys_tick_ovf;
	uint32_t sys_freq = SYS_SysTick_GetFreq();

	usec_tick = ((uint64_t)(sysTicks - systick_val) * 1000000) / sys_freq;
	if (systick_val == 0) // to protect time overflow
		_sys_tick_ovf -= 1;
	float us_per_ovf = (float)sysTicks * (float)1000000 / (float)sys_freq;
	usec_tick_total = (uint64_t)((float)_sys_tick_ovf * us_per_ovf) + usec_tick;
	return usec_tick_total;
}

uint32_t platform_get_time_ms(void)
{
	uint32_t msec_tick;
	uint64_t msec_tick_total;
	uint32_t systick_val = SysTick->VAL;
	uint32_t _sys_tick_ovf = sys_tick_ovf;
	uint32_t sys_freq = SYS_SysTick_GetFreq();

	msec_tick = ((uint64_t)(sysTicks - systick_val) * 1000) / sys_freq;
	if (systick_val == 0) // to protect time overflow
		_sys_tick_ovf -= 1;
	float ms_per_ovf = (float)sysTicks * (float)1000 / (float)sys_freq;
	msec_tick_total = (uint64_t)((float)_sys_tick_ovf * ms_per_ovf) + msec_tick;
	return msec_tick_total;
}

void platform_get_time(uint32_t *sec, uint32_t *usec)
{
	uint32_t usec_tick;
	uint32_t systick_val = SysTick->VAL;
	uint32_t _sys_tick_ovf = sys_tick_ovf;
	uint32_t sys_freq = SYS_SysTick_GetFreq();

	usec_tick = ((uint64_t)(sysTicks - systick_val) * 1000000) / sys_freq;
	if (systick_val == 0) // to protect time overflow
		_sys_tick_ovf -= 1;

	float us_per_ovf = (float)sysTicks * (float)1000000 / (float)sys_freq;
	uint64_t usec_tick_total = (uint64_t)((float)_sys_tick_ovf * us_per_ovf) + usec_tick;
	*sec = (usec_tick_total / 1000000);
	*usec = usec_tick_total - 1000000 * *sec;
}

#if 0
void platform_delay_ms(unsigned int ms)
{
	uint64_t prev_tick = platform_get_tick();
	uint64_t wait_time = ms * 1000;
	uint64_t curr_tick;
	int64_t diff;

	//xprintf("prev_tick: %lld, wait_time: %lld, ms: %d\n", prev_tick, wait_time, ms);
	while(1) {
		curr_tick = platform_get_tick();
		diff = curr_tick - prev_tick;
		if (diff > wait_time) {
			//xprintf("prev_tick: %lld, curr_tick: %lld, diff: %lld, wait_time: %lld\n",
			//	prev_tick, curr_tick, diff, wait_time);
			break;
		}
	}
}

void platform_delay_us(unsigned int us)
{
	uint64_t prev_tick = platform_get_tick();
	uint64_t wait_time = us;
	uint64_t curr_tick;
	int64_t diff;

	//xprintf("prev_tick: %lld, wait_time: %lld, us: %d\n", prev_tick, wait_time, us);
	while(1) {
		curr_tick = platform_get_tick();
		diff = curr_tick - prev_tick;
		if (diff > wait_time) {
			//xprintf("prev_tick: %lld, curr_tick: %lld, diff: %lld, wait_time: %lld\n",
			// prev_tick, curr_tick, diff, wait_time);
			break;
		}
	}
}

int tick_test(void)
{
	tick_init();
	platform_delay_us(1000);
	platform_delay_ms(1000);
    while(1) {
		uint32_t sec;
		uint32_t usec;
		xprintf("tick: %lld, sec: %lld, msec: %lld, usec: %lld\n",
			platform_get_tick(), tick_to_sec(), tick_to_msec(), tick_to_usec());
		platform_get_time(&sec, &usec);
		xprintf("%d.%d\n", sec, usec);
	}
}
#endif
