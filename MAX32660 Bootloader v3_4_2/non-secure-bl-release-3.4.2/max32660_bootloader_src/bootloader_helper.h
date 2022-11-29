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

#ifndef _BOOTLOADER_HELPER_H__
#define _BOOTLOADER_HELPER_H__

#include <stdint.h>
#include "bootloader.h"

uint8_t check_input_value_range(uint8_t value, uint8_t lower_bound, uint8_t upper_bound);
int bootloader_console_shutdown(void);
int bootloader_console_init(void);
void dump_app_header(void);
void dump_bootloader_config(void);
#if defined(BOOTLOADER_BUILD_INFO) || defined(BOOTLOADER_DEBUG_MSG)
	void uart_puts(const char *s);
	void uart_print_val(int val);
	void uart_print_hex(uint32_t val);
	void err_erase_page(int code);
	void uart_print_array(const char *label, uint8_t *buf, int len);
#else
	#define uart_puts(s)
	#define uart_print_val(val)
	#define uart_print_hex(val)
	#define uart_print_array(label, buf, len)
	#define err_erase_page(code)
#endif
#endif // _BOOTLOADER_HELPER_H__
