/*******************************************************************************
 * Author: Ismail Kose, Ismail.Kose@maximintegrated.com
 * Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
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
 * $Date: 2015-12-04 08:35:27 -0600 (Fri, 04 Dec 2015) $
 * $Revision: 20285 $
 *
 ******************************************************************************/
#ifndef _PLATFORM_TICK_H
#define _PLATFORM_TICK_H

#define tick_to_sec()	((uint64_t)platform_get_tick() / 1000000)
#define tick_to_msec()	((uint64_t)platform_get_tick() / 1000)
#define tick_to_usec()	((uint64_t)platform_get_tick())

int32_t platform_tick_init(void);
uint64_t platform_get_tick(void);
void platform_get_time(uint32_t *sec, uint32_t *usec);
#if 0
void platform_delay_us(unsigned int us);
void platform_delay_ms(unsigned int ms);
#endif
uint32_t platform_get_time_usec(void);
uint32_t platform_get_time_ms(void);
#endif //_PLATFORM_TICK_H
