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

#include "bootloader_helper.h"
#include "bootloader.h"
#include "utils.h"
#include "uart.h"
#if defined(BOOTLOADER_BUILD_INFO) || defined(BOOTLOADER_DEBUG_MSG)
/***** Global Variables *****/
mxc_uart_regs_t * ConsoleUart = MXC_UART_GET_UART(CONSOLE_UART);
/***** File Scope Variables *****/
//Console output is enabled by default
static int console_conn = 1;
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
#endif

/******************************************************************************/
#if defined(BOOTLOADER_BUILD_INFO) || defined(BOOTLOADER_DEBUG_MSG)
int bootloader_console_init(void)
{
    int err;

    if ((err = UART_Init(ConsoleUart, &uart_cfg, &uart_sys_cfg)) != E_NO_ERROR) {
        return err;
    }

    return E_NO_ERROR;
}

int bootloader_console_shutdown(void)
{
	int err;
	sys_periph_clock_t uart_clock;

	if ((err = UART_Shutdown(ConsoleUart)) != E_NO_ERROR) {
		return err;
	}

	if (ConsoleUart == MXC_UART0) {
		uart_clock = SYS_PERIPH_CLOCK_UART0;
	} else if (ConsoleUart == MXC_UART1) {
		uart_clock = SYS_PERIPH_CLOCK_UART1;
	}

	SYS_ClockEnable(uart_clock);
	return err;
}

void dump_app_header(void)
{
	app_header_t *header = (app_header_t *)&_boot_mem_start;

	uart_puts("\r\n\r\n== App header ==");

	uart_puts("\r\nCRC32: ");
	uart_print_hex(header->CRC32);
	uart_puts(" @");
	uart_print_hex((uint32_t)&header->CRC32);

	uart_puts("\r\nLength: ");
	uart_print_val(header->length);
	uart_puts(" @");
	uart_print_hex((uint32_t)&header->length);

	uart_puts("\r\nvalid_mark: ");
	uart_print_hex(header->valid_mark);
	uart_puts(" @");
	uart_print_hex((uint32_t)&header->valid_mark);


	uart_puts("\r\nboot_mode: ");
	uart_print_hex(header->boot_mode);
	uart_puts(" @");
	uart_print_hex((uint32_t)&header->boot_mode);

	uart_puts("\r\n\r\n");
}

void dump_bootloader_config(void)
{
	boot_config_t *pbootconfig = get_pointer_to_bootconfig();
	uart_puts("\r\n== Bootloader config ==");
	uart_puts("\r\nenter_bl_check: ");
	uart_print_val(pbootconfig->enter_bl_check);
	uart_puts("\r\nebl_pin: ");
	uart_print_val(pbootconfig->ebl_pin);
	uart_puts("\r\nebl_port: ");
	uart_print_val(pbootconfig->ebl_port);
	uart_puts("\r\nebl_polarity: ");
	uart_print_val(pbootconfig->ebl_polarity);
	uart_puts("\r\nvalid_mark_check: ");
	uart_print_val(pbootconfig->valid_mark_check);
	uart_puts("\r\nuart_enable: ");
	uart_print_val(pbootconfig->uart_enable);
	uart_puts("\r\ni2c_enable: ");
	uart_print_val(pbootconfig->i2c_enable);
	uart_puts("\r\nspi_enable: ");
	uart_print_val(pbootconfig->spi_enable);
	uart_puts("\r\ni2c_addr: ");
	uart_print_val(pbootconfig->i2c_addr);
	uart_puts("\r\nCRC_mark_check: ");
	uart_print_val(pbootconfig->crc_check);
	uart_puts("\r\nebl_timeout: ");
	uart_print_val(pbootconfig->ebl_timeout);
	uart_puts("\r\nexit_bl_mode: ");
	uart_print_val(pbootconfig->exit_bl_mode);
	uart_puts("\r\nlock_swd: ");
	uart_print_val(pbootconfig->lock_swd);
	uart_puts("\r\n\r\n");
}

void uart_puts(const char *s)
{
	uint32_t len = get_str_len(s);

	if (console_conn) {
		UART_Write(MXC_UART_GET_UART(CONSOLE_UART), (const uint8_t *)s, len);
	}
}


void uart_print_val(int val)
{
	char buf[12];
	itoa(val, buf, 10);
	uart_puts(buf);
}

void uart_print_hex(uint32_t val)
{
	char buf[12];
	itoa(val, buf, 16);
	uart_puts("0x");
	uart_puts(buf);
}


void uart_print_array(const char *label, uint8_t *buf, int len)
{

	int i;
	uart_puts("\r\n");
	uart_puts(label);
	for(i = 0; i < len; i++) {
		uart_puts(" ");
		uart_print_hex(buf[i]);
	}
	uart_puts("\r\n");
}

void err_erase_page(int code)
{
	uart_puts("\r\nCouldn't erase page! (Error code ");
	uart_print_val(code);
	uart_puts(")\r\n");
}
#endif
uint8_t check_input_value_range(uint8_t value, uint8_t lower_bound, uint8_t upper_bound)
{
	if(value >= lower_bound && value <= upper_bound)
		return 1;
	else
		return 0;
}
