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
 * @brief   Entering bootloader mode from application!
 * @details This example show how to enter bootloader mode from application.
 */

/*
 * Caution: It may break bootloader settings and behave unexpectedly
 *			if something goes wrong while updating _boot_mode flag.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mxc_delay.h"
#include "led.h"
#include "flc.h"
#include "tmr_utils.h"

/***** Definitions *****/

#define PAGE_SIZE 8192
#define BOOTMODE_SET		0xAAAAAAAA
/***** Globals *****/
extern int _boot_mode;

/***** Functions *****/

int flash_uninit(void)
{
	FLC_ClearFlags(MXC_F_FLC_INTR_DONE | MXC_F_FLC_INTR_AF);
	return  0;  // Finished without Errors
}


int flash_verify(uint32_t address, uint32_t length, uint8_t *data)
{
  volatile uint8_t *ptr;

  for (ptr = (uint8_t *)address; ptr < (uint8_t *)(address + length); ptr++, data++) {
	if (*ptr != *data) {
		printf("Verify failed at 0x%x (0x%x != 0x%x)\n", (unsigned int)ptr, (unsigned int)*ptr, (unsigned int)*data);
		return E_UNKNOWN;
	}
	}

	return E_NO_ERROR;
}


int flash_program_page(unsigned long address, unsigned long size, unsigned char *buffer8)
{
	uint32_t dest_addr;
	int i = 0;
	uint32_t *buffer32 = (uint32_t *)buffer8;

	for (dest_addr = address; dest_addr < (address + size); dest_addr += 4) {
		if (FLC_Write32(dest_addr, buffer32[i]) != E_NO_ERROR) {
			break;
		}
		i++;
	}

	if (flash_verify(address, size, (uint8_t*)buffer8) != E_NO_ERROR) {
		return -1;
	}

	return 0;
}


int set_boot_mode_and_reset()
{
	//Check if we HAVE a bootloader first by looking at ISR vector table location
	//If bootloader is present, then the firmware's vector table will be somewhere other than 0x00
	if (SCB->VTOR == 0x00) {
		printf("No bootloader mode available\n");
		return -1;
	}

	if(_boot_mode == BOOTMODE_SET) {
		printf("Boot mode already set\n");
		return 0;
	}

	uint8_t page_buffer[PAGE_SIZE + 3];
	uint8_t *aligned_buf_ptr = &page_buffer[0];

	//Get the beginning of the page that _boot_mode is located at
	unsigned long boot_mode_page_loc = ((unsigned long)&_boot_mode) & ~(PAGE_SIZE - 1);

	if (((int)&page_buffer[0] & 3) != 0) {
		int alignOffset = 4 - ((int)&page_buffer[0] & 3);
		aligned_buf_ptr += alignOffset;
	}

	//Copy contents of page
	unsigned long offset = (unsigned long)&_boot_mode - boot_mode_page_loc;
	memcpy(aligned_buf_ptr, (uint8_t *)boot_mode_page_loc, PAGE_SIZE);

	//Set 4 bytes of _boot_mode to 0xAA to keep bootloader in bootloader mode
	aligned_buf_ptr[offset] = 0xAA;
	aligned_buf_ptr[offset + 1] = 0xAA;
	aligned_buf_ptr[offset + 2] = 0xAA;
	aligned_buf_ptr[offset + 3] = 0xAA;

	//Update contents of page
	const sys_cfg_t sys_flash_cfg = NULL;
	int ret = FLC_Init(&sys_flash_cfg);
	if (ret) {
		flash_uninit();
		return ret;
	}

	ret = FLC_PageErase(boot_mode_page_loc);
	if (ret) {
		flash_uninit();
		return ret;
	}

	ret = flash_program_page(boot_mode_page_loc, PAGE_SIZE, aligned_buf_ptr);
	if (ret) {
		flash_uninit();
		return ret;
	}

	//Jump to bootloader
	printf("Set boot_mode, rebooting...\n\n");
	flash_uninit();
	TMR_Delay(MXC_TMR0, MSEC(100), NULL);
	NVIC_SystemReset();
	return 0;
}


int clear_boot_mode()
{
	//Check if we HAVE a bootloader first by looking at ISR vector table location
	//If bootloader is present, then the firmware's vector table will be somewhere other than 0x00
	if (SCB->VTOR == 0x00) {
		printf("No bootloader mode available\n");
		return -1;
	}

	if(_boot_mode == 0) {
		printf("Boot mode already cleared\n");
		return 0;
	}

	uint8_t page_buffer[PAGE_SIZE + 3];
	uint8_t *aligned_buf_ptr = &page_buffer[0];

	//Get the beginning of the page that _boot_mode is located at
	unsigned long boot_mode_page_loc = ((unsigned long)&_boot_mode) & ~(PAGE_SIZE - 1);

	if (((int)&page_buffer[0] & 4) != 0) {
		int alignOffset = 4 - ((int)&page_buffer[0] & 4);
		aligned_buf_ptr += alignOffset;
	}

	//Copy contents of page
	unsigned long offset = (unsigned long)&_boot_mode - boot_mode_page_loc;
	memcpy(aligned_buf_ptr, (uint8_t *)boot_mode_page_loc, PAGE_SIZE);

	//Set 4 bytes of _boot_mode to 0x00 signal bootloader to skip flashing next time it is entered
	aligned_buf_ptr[offset] = 0x00;
	aligned_buf_ptr[offset + 1] = 0x00;
	aligned_buf_ptr[offset + 2] = 0x00;
	aligned_buf_ptr[offset + 3] = 0x00;

	//Update contents of page
	const sys_cfg_t sys_flash_cfg = NULL;
	int ret = FLC_Init(&sys_flash_cfg);
	if (ret) {
		flash_uninit();
		return ret;
	}

	ret = FLC_PageErase(boot_mode_page_loc);
	if (ret) {
		flash_uninit();
		return ret;
	}

	ret = flash_program_page(boot_mode_page_loc, PAGE_SIZE, aligned_buf_ptr);
	if (ret) {
		flash_uninit();
		return ret;
	}

	flash_uninit();
	printf("Cleared boot_mode\n");
	return 0;
}


int main(void)
{
	int count = 5;

	printf("*** Maxim Integrated ***\n\n");
	printf("Enter Bootloader Example!\n");
	printf("Will enter to the bootloader mode after 5 sec!\n");
	clear_boot_mode();
	while(count--) {
		LED_On(0);

		TMR_Delay(MXC_TMR0, MSEC(500), NULL);
		LED_Off(0);

		TMR_Delay(MXC_TMR0, MSEC(500), NULL);
		printf("Remained = %d sec...\n", count);
	}

	set_boot_mode_and_reset();
	return 0;
}
