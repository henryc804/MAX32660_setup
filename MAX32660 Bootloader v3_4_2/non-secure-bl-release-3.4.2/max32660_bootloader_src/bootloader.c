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
#include "mxc_delay.h"
#include "gpio.h"
#include "crc32.h"
#include "uart.h"
#include "flash.h"
#include "build_info.h"
#include "bootloader.h"
#include "bootloader_helper.h"
#include "platform_tick.h"

/* Pin mapping */
#define GPIO_PORT_MFIO	PORT_0

static uint16_t num_pages = 0;
static boot_config_t boot_config_copy;
uint8_t txdata[32];
uint8_t rxdata[PAGE_PAYLOAD_SIZE + 2];
static volatile int bootldr_err_code = BTLDR_NO_ERR;
static volatile bool is_app_valid = false;
static volatile bool is_crc_ok = false;
static volatile bool flash_erased = false;
static int mxm_bl_rx_total_bytes = 0;

// Pin Mapping
static gpio_cfg_t mfio =  {PORT_0, PIN_1, GPIO_FUNC_IN, GPIO_PAD_PULL_UP}; //default mfio pin
static volatile char mxm_bl_cmd_rx_flag = false;
static unsigned long total_flash_size;
static unsigned long flash_size;
static unsigned long app_start_loc;

static int got_img_size;
static int img_page_size;

//Buffer is the sizeof one page (8KB), plus checksum bytes
//Must be 4 byte aligned for flash_program_page()
static uint8_t page_plain[PAGE_PAYLOAD_SIZE] __attribute__ ((aligned (4)));
static volatile btldr_mode_t bootldr_mode = BTLDR_IDLE;

unsigned char interface = UART_INTERFACE;

static volatile status_t btldr_status;

static int page_num_to_erase = 0;
static int16_t partial_rcv_page_size = PAGE_PAYLOAD_SIZE;
static uint32_t partial_page_offset = 0;

bl_comm_msg_t bl_msg;

sub_cmd_t fam_byte_w_bootloader_cmds[] = {
	{ BL_SUB_CMD_SET_IV, &mxm_wr_bootloader_cmd_set_iv, 11}, /* Get IV bytes */
	{ BL_SUB_CMD_SET_AUTH, &mxm_wr_bootloader_cmd_set_auth, 16}, /* Set auth bytes */
	{ BL_SUB_CMD_SET_NUM_PAGES, &mxm_wr_bootloader_cmd_set_num_pages, 2}, /* Get number of pages */
	{ BL_SUB_CMD_ERASE_APP_MEMORY, &mxm_wr_bootloader_cmd_erase_app_memory, 0}, /* Erase App memory */
	{ BL_SUB_CMD_RECEIVE_PAGE, &mxm_wr_bootloader_cmd_receive_page, PAGE_PAYLOAD_SIZE}, /* Get bootloader page */
	{ BL_SUB_CMD_ERASE_PAGE_MEMORY, &mxm_wr_bootloader_cmd_erase_page_memory, 2}, /* Erase a given page */
	{ BL_SUB_CMD_SET_PARTIAL_PAGE_SIZE, &mxm_wr_bootloader_cmd_set_pl_page_size, 2}, /* Set receive partial page size */
};

const sub_cmd_t fam_byte_r_bootloader_info[] = {
	{ BL_SUB_CMD_GET_VERSION, &mxm_bootloader_get_fw_version, 0}, /* Get bootloader fw version */
	{ BL_SUB_CMD_GET_PAGE_SIZE, &mxm_bootloader_get_page_size, 0}, /* Get page size */
	{ BL_SUB_CMD_GET_USN, &mxm_bootloader_get_usn, 0}, /* Get bootloader fw version */
};

const sub_cmd_t fam_byte_wr_bootloader_mode_cmds[] = {
	{ BL_SUB_CMD_W_DEVICE_MODE, &mxm_bootloader_write_mode, 1}, /* Set bootloader mode */
};

const sub_cmd_t fam_byte_rd_bootloader_mode_cmds[] = {
	{ BL_SUB_CMD_R_DEVICE_MODE, &mxm_bootloader_read_mode, 0}, /* Get bootloader mode */
};

const sub_cmd_t fam_byte_wr_config_bootloader_cmds[] = {
	{ BL_SUB_CMD_SAVE_SETTINGS, &mxm_bootloader_save_config, 0}, /* Save bootloader settings to flash */
	{ BL_SUB_CMD_ENTRY_CONFIG, &mxm_bootloader_config_entry, 2}, /* Update bootloader config in RAM */
	{ BL_SUB_CMD_EXIT_CONFIG, &mxm_bootloader_config_exit, 2}, /* Update bootloader config in RAM */
};

const sub_cmd_t fam_byte_rd_config_bootloader_cmds[] = {
	{ BL_SUB_CMD_RD_ENTRY_CONFIG, &mxm_bootloader_get_config_entry, 1},
	{ BL_SUB_CMD_RD_EXIT_CONFIG, &mxm_bootloader_get_config_exit, 1},
	{ BL_CMDIDX_R_ALL_CFG, &mxm_bootloader_get_config_bl, 1},
};

const sub_cmd_t fam_byte_device_info[] = {
	{ BL_SUB_CMD_PLATFORM_TYPE, &mxm_bootloader_platform_type, 0}, /*  */
};

static const cmd_table_t cmd_table[] = {
	{ FAM_BYTE_MODE_WRITE, fam_byte_wr_bootloader_mode_cmds, ARRAY_SIZE(fam_byte_wr_bootloader_mode_cmds), },
	{ FAM_BYTE_MODE_READ, fam_byte_rd_bootloader_mode_cmds, ARRAY_SIZE(fam_byte_rd_bootloader_mode_cmds), },
	{ FAM_BYTE_BOOTLOADER, fam_byte_w_bootloader_cmds, ARRAY_SIZE(fam_byte_w_bootloader_cmds), },
	{ BOOTLOADER_INFO, fam_byte_r_bootloader_info, ARRAY_SIZE(fam_byte_r_bootloader_info), },
	{ CONFIG_BYTE_MODE_WRITE, fam_byte_wr_config_bootloader_cmds, ARRAY_SIZE(fam_byte_wr_config_bootloader_cmds), },
	{ CONFIG_BYTE_MODE_READ, fam_byte_rd_config_bootloader_cmds, ARRAY_SIZE(fam_byte_rd_config_bootloader_cmds), },
	{ DEVICE_INFO, fam_byte_device_info, ARRAY_SIZE(fam_byte_device_info), },
};

boot_config_t* get_pointer_to_bootconfig(void)
{
	return (boot_config_t *)((uint8_t *)&_boot_mem_start + sizeof(app_header_t));
}

boot_mem_t* get_ptr_bootmem(void)
{
	return (boot_mem_t *)((uint8_t *)&_boot_mem_start);
}

int set_boot_mode_and_reset(void)
{
	return 0;
}

uint8_t * mxm_get_rx_buf_pointer(void)
{
	return rxdata;
}

uint8_t * mxm_get_tx_buf_pointer(void)
{
	return txdata;
}

void mxm_set_interface(unsigned char interface_type)
{
	interface = interface_type;
}

void send_response(unsigned char len)
{
#ifdef BOOTLOADER_UART
	if(interface == UART_INTERFACE) {
		mxm_bl_uart_write(txdata, len);
	}
#endif
#ifdef BOOTLOADER_SPI
	if(interface == SPI_INTERFACE) {
		mxm_bl_spi_slave_write_cb_init(len);
	}
#endif
}


//Return true if valid
//Return false if not valid
bool check_if_app_is_valid(bool valid_check_flag)
{
	//Does app have a valid isr vector table?
	if ((uint32_t)_app_isr[1] == UNINITIALIZED_MEM) {
		uart_puts("\r\n#App ISR table is not valid.\n");
		return false;
	}

	if(valid_check_flag) {
		//Get application crc from app header/footer
		app_header_t *header = (app_header_t *)&_boot_mem_start;

		if ((header->length == UNINITIALIZED_MEM) || (header->length > flash_size)) {
			uart_puts("\r\n#Reported App length is not valid\n");
			uart_puts("\r\nheader->length: ");
			uart_print_val(header->length);
			uart_puts("\r\nflash_size:");
			uart_print_val(flash_size);
			return false;
		} else if (header->valid_mark != MARK_VALID_MAGIC_VAL) {
			uart_puts("\r\nValid Mark is wrong");
			return false;
		} else if (_boot_mode == UNINITIALIZED_MEM) {
			uart_puts("\r\nBoot_mode uninitialized");
			return false;
		}
	}

	return true;
}

//Return true if valid
//Return false if not valid
bool check_app_crc(bool crc_check_flag)
{
	if(crc_check_flag) {
#if !defined(DISABLE_CRC_AT_BOOT)
		//Get application crc from app header/footer
		app_header_t *header = (app_header_t *)&_boot_mem_start;
		uint32_t app_crc = calcCrc32((uint8_t*)app_start_loc, header->length);
		if (app_crc == header->CRC32) {
			return true;
		} else {
			uart_puts("\r\nApp CRC not valid\n");
			return false;
		}
#else
		int i = 20; // 20 msec
		while (i--) {
			mxc_delay(MXC_DELAY_MSEC(1));
		}
	   #warning "*********** BOOTLOADER_CRC Check Disabled ***********"
#endif
	}
	return true;
}


int erase_boot_mem_page(void)
{
	int ret;
	uint8_t bootmem_buf[(unsigned long)&_boot_mem_len];

	memcpy(bootmem_buf, (void *)&_boot_mem_start, (unsigned long)&_boot_mem_len);

	uart_puts("\r\nErasing bootmem");
	uart_puts("\r\nPage ");
	uart_print_val(memloc >> FLC_PAGE_BIT_SIZE);
	uart_puts(": Erasing ");
	uart_print_val(num_bytes_to_clear);
	uart_puts(" bytes at ");
	uart_print_hex(memloc);

	ret = flash_erase_page((unsigned long)&_boot_mem_start);
	if (ret)
	{
		err_erase_page(ret);
		return ret;
	}

	ret = flash_program_page((unsigned long)&_boot_mem_start, (unsigned long)&_boot_mem_len, bootmem_buf);
	if (ret) {
		uart_puts("Failed!\r\n");
		uart_puts("Couldn't rewrite boot_mem values. (Error code ");
		uart_print_val(ret);
		uart_puts("), exiting flash mode\r\n");
		uart_puts("\r\n");
		txdata[0] = BL_ERR_UNKNOWN;
		send_response(BL_STATUS_BYTE_LEN);
		return ret;
	}

	return 0;
}

static int bootloader_page_erase(int page_num)
{
	int startPage = (app_start_loc >> FLC_PAGE_BIT_SIZE);
	int lastPage = (flash_size >> FLC_PAGE_BIT_SIZE) + startPage;
	int boot_mem_page = ((unsigned long)&_boot_mem_start) >> FLC_PAGE_BIT_SIZE;

	if ((page_num < startPage) || (page_num > lastPage)) {
		return -1;
	}

	if (page_num == boot_mem_page) {
		return erase_boot_mem_page();
	}

	return flash_erase_page(page_num << FLC_PAGE_BIT_SIZE);
}

/*
This function erases all pages except bootloader page and bootmem area.
*/
static int master_erase()
{
	int ret;
	int startPage;
	int currentPage;
	int lastPage;
	int memLoc;

	flash_uninit();
#ifdef MAX32660
	const sys_cfg_t sys_flash_cfg = NULL;
	FLC_Init(&sys_flash_cfg);
#else
	FLC_Init();
#endif

	startPage = (app_start_loc >> FLC_PAGE_BIT_SIZE);
	lastPage = (flash_size >> FLC_PAGE_BIT_SIZE) + startPage;

	uart_puts("\r\nErasing pages ");
	uart_print_val(startPage);
	uart_puts("[");
	uart_print_hex(FLC_PAGE_SIZE * startPage);
	uart_puts("] through ");
	uart_print_val(lastPage);
	uart_puts("[");
	uart_print_hex(FLC_PAGE_SIZE * lastPage);
	uart_puts("]\r\n");

	for (currentPage = startPage; currentPage < lastPage; currentPage++) {
		memLoc = currentPage * FLC_PAGE_SIZE;
		uart_puts("\r\nPage ");
		uart_print_val(currentPage);
		uart_puts(": Erasing ");
		uart_print_val(FLC_PAGE_SIZE);
		uart_puts(" bytes at ");
		uart_print_hex(memLoc);
		ret = flash_erase_page(memLoc);
		if (ret)
		{
			err_erase_page(ret);
			flash_uninit();
			return ret;
		}
	}

	erase_boot_mem_page();
	uart_puts("\r\nDone\n");
	flash_uninit();
	return COMM_SUCCESS;
}

int mxm_bl_receive_page_and_flash(void)
{
	int ret;
	int startPage;
	int currentPage;
	int bootMemPage;
	int endPage;
	unsigned long memLoc;
	unsigned long flc_write_size;

	flash_uninit();
#ifdef MAX32660
	const sys_cfg_t sys_flash_cfg = NULL;
	FLC_Init(&sys_flash_cfg);
#else
	FLC_Init();
#endif

	startPage = (app_start_loc >> FLC_PAGE_BIT_SIZE);
	bootMemPage = (uint32_t)&_boot_mem_start;
	bootMemPage >>= FLC_PAGE_BIT_SIZE;
	currentPage = startPage;
	endPage = startPage + img_page_size - 1;

	while (currentPage < (startPage + img_page_size)) {
		uart_puts("\r\nWaiting for page ");
		uart_print_val(currentPage);
		uart_puts("/");
		uart_print_val(startPage + img_page_size - 1);
		uart_puts(" data (");
		uart_print_val(FLC_PAGE_SIZE);
		uart_puts(" bytes)...");
		while(!btldr_status.busy) { /* Waiting for a new page */

		}

		uart_puts("\r\nTotal bytes received: ");
		uart_print_hex(mxm_bl_rx_total_bytes);
		//Checksum
		if (crcVerifyMsg(page_plain, FLC_PAGE_SIZE + CHECK_BYTESIZE)) {
			bootldr_err_code = BTLDR_CHECKSUM_FAIL; /* Checksum failed, try Again */
			uart_puts("\r\nVerify checksum failed!\n");
			uart_puts("\r\nRequesting page ");
			uart_print_val(currentPage);
			uart_puts(" again\r\n");
			txdata[0] = BL_ERR_BTLDR_CHECKSUM;
			send_response(BL_STATUS_BYTE_LEN);
			continue;
		}

		if (currentPage < endPage) { //currentPage < bootMemPage
			memLoc = currentPage * FLC_PAGE_SIZE;
			flc_write_size = FLC_PAGE_SIZE;
			if(currentPage == bootMemPage) {
				flc_write_size = FLC_PAGE_SIZE - (unsigned long)&_boot_mem_len;
			}
		} else {
			uart_puts("\r\n**** Flashing Bootmem\n");
			app_header_t header;
			memcpy(&header, page_plain, sizeof(app_header_t));
			if (header.valid_mark) {
				master_erase();
				uart_puts("\r\nFlashing failed. Invalid application...");
				return -1;
			}
			if (header.length == 0 || header.length > (_boot_mem_start - _app_start)) {
				master_erase();
				uart_puts("\r\nFlashing failed. Invalid app length...");
				txdata[0] = BL_ERR_BTLDR_INVALID_APP;
				send_response(BL_STATUS_BYTE_LEN);
				return BL_ERR_BTLDR_INVALID_APP;
			}

			/* Back up page, and update boot memory arguments */
			memLoc = bootMemPage * FLC_PAGE_SIZE;
			memcpy(page_plain, (void *)memLoc, FLC_PAGE_SIZE);
			header.valid_mark = MARK_VALID_MAGIC_VAL;
			uint32_t offset = (uint32_t)&_boot_mem_start & (FLC_PAGE_SIZE - 1);
			memcpy(&page_plain[offset], &header, sizeof(app_header_t));
			flc_write_size = FLC_PAGE_SIZE;
			flash_erase_page((uint32_t)memLoc);
		}

		uart_puts("\r\nPage ");
		uart_print_val(currentPage);
		uart_puts("/");
		uart_print_val(startPage + img_page_size - 1);
		uart_puts(": Programming ");
		uart_print_val(FLC_PAGE_SIZE);
		uart_puts(" bytes at ");
		uart_print_hex(memLoc);

		ret = flash_program_page(memLoc, flc_write_size, page_plain);
		if (ret) {
			uart_puts("\r\nFailed! Couldn't program page. ret: ");
			uart_print_val(ret);
			uart_puts(" , exiting flash mode.");
			txdata[0] = BL_ERR_UNKNOWN;
			send_response(BL_STATUS_BYTE_LEN);
			return ret;
		}
		currentPage++;
		uart_puts("....succeed.");
		__disable_irq();
		btldr_status.busy = 0;
		__enable_irq();
		txdata[0] = BL_SUCCESS;
		send_response(BL_STATUS_BYTE_LEN);
	}

	flash_uninit();
	is_app_valid = check_if_app_is_valid(1);
	is_crc_ok = check_app_crc(1);
	if (is_crc_ok == false) {
		uart_puts("\r\nInvalid application...");
		master_erase();
		return -1;
	}
	uart_puts("\r\nFinished flashing application..");

	return 0;
}

int mfio_init(void)
{
	boot_config_t* boot_config = get_pointer_to_bootconfig();
	mfio.port = GPIO_PORT_MFIO;
	mfio.pad = GPIO_PAD_PULL_UP; // GPIO_PAD_NONE
	mfio.func = GPIO_FUNC_IN; //GPIO_PAD_NONE

	switch(boot_config->ebl_pin)
	{
		case 0x0E:
		case 0x0F:
			mfio.mask = PIN_1;
			break;
		default:
			mfio.mask = (1 << boot_config->ebl_pin);
			break;
	}

	GPIO_Config(&mfio);
	mxc_delay(MXC_DELAY_USEC(50)); /* Wait before read GPIO */
	return !!GPIO_InGet(&mfio);
}

int update_boot_config() {
	int ret;
	int bootMemPage;
	unsigned long memLoc;
	uint32_t offset;
	flash_uninit();
#ifdef MAX32660
	const sys_cfg_t sys_flash_cfg = NULL;
	FLC_Init(&sys_flash_cfg);
#else
	FLC_Init();
#endif
	bootMemPage = (uint32_t)&_boot_mem_start;
	bootMemPage >>= FLC_PAGE_BIT_SIZE;
	/* Back up page, and update boot memory arguments */
	memLoc = bootMemPage * FLC_PAGE_SIZE;
	memcpy(page_plain, (void *)memLoc, FLC_PAGE_SIZE);
	offset = (uint32_t)&_boot_mem_start & (FLC_PAGE_SIZE - 1);
	offset += sizeof(app_header_t);
	memcpy(&page_plain[offset], &boot_config_copy, sizeof(boot_config_t));
	/* Copying crc of boot config to page */
	*(uint32_t *)(page_plain + offset + sizeof(boot_config_t)) = calcCrc32((uint8_t*)&boot_config_copy, sizeof(boot_config_t));

	uart_puts("\r\nPage ");
	uart_puts(": Programming ");
	uart_print_val(FLC_PAGE_SIZE);
	uart_puts(" bytes at [");
	uart_print_hex(memLoc);
	uart_puts("]...");

	ret = flash_erase_page((uint32_t)memLoc);
	if (ret != 0){
		uart_puts("\r\nflash_erase_page has failed.");
	}
	ret = flash_program_page(memLoc, FLC_PAGE_SIZE, (page_plain));
	if (ret != 0){
		uart_puts("\r\nflash_program_page has failed.");
	}
	flash_uninit();
#if defined(BOOTLOADER_BUILD_INFO) || defined(BOOTLOADER_DEBUG_MSG)
	dump_bootloader_config();
#endif
	uart_puts("\r\nupdate_boot_config has finished");
	return ret;
}

// Evaluates bootloader startup routine
// returns whether or not to exit bootloader
static char should_enter_bootloader(int mfio_state)
{
	// the default wait for comm
	boot_config_t* boot_config = get_pointer_to_bootconfig();
	uint32_t wait_value = 0;
	if (mxm_bl_cmd_rx_flag) {
		return true;
	}

	wait_value = 1 << (boot_config->ebl_timeout);

	if (boot_config->enter_bl_check) {
		// check the status of EBL pin
		uart_puts("\r\nenter_bl_check mode is enabled.");
		uart_puts("\r\nebl_polarity: ");
		uart_print_val(boot_config->ebl_polarity);
		uart_puts("\r\nmfio_state: ");
		uart_print_val(mfio_state);
		uart_puts("\r\n");

		if (mfio_state == boot_config->ebl_polarity) {
			return true;
		}
	}

	if (boot_config->exit_bl_mode == BL_EXIT_INDEFINITE) {
		uart_puts("\r\nbl exit indefinite mode is enabled.");
		return true;
	} else if ((boot_config->exit_bl_mode == BL_EXIT_TIMEOUT)) {
		uart_puts("\r\nTime out mode is enabled.");
		uart_puts("\r\nSend \"0x01 0x00 0x08\" to stay in bootloader within ");
		uart_print_val(wait_value);
		uart_puts(" msec.\r\n");
		while(wait_value != 0) {
			wait_value--;
			if (mxm_bl_cmd_rx_flag) {
				break;
			}
			mxc_delay(MXC_DELAY_MSEC(1));
		}
		return mxm_bl_cmd_rx_flag;
	} else {
		// Immediate goto mainapp
		return false;
	}

	return false;
}

void bootconfig_default_init(void)
{
	boot_mem_t* boot_mem = get_ptr_bootmem();
	boot_config_t* boot_config = get_pointer_to_bootconfig();
	uint32_t boot_cfg_crc = calcCrc32((uint8_t*)boot_config, sizeof(boot_config_t));
	uart_puts("config_header_default_init\r\n");
	//if the pin number is uninitialized write the default values
	if ((boot_config->ebl_pin == 0x0F) || (boot_mem->cfg_crc != boot_cfg_crc)) {
		uart_puts("setting config_header_default_init\r\n");

		boot_config_copy.res0 = 0x0;
		boot_config_copy.res1 = 0x0;
		boot_config_copy.res2 = 0x0;
		boot_config_copy.res3 = 0x0;

		boot_config_copy.ebl_pin = 0x1;

#ifdef BL_TIME_OUT_TO_STAY_IN_BOOTLDR
		boot_config_copy.exit_bl_mode = 1;   //timeout
#else
		boot_config_copy.exit_bl_mode = 0;   //timeout
#endif

#ifdef BL_GPIO_TO_STAY_IN_BOOTLDR
		boot_config_copy.enter_bl_check = 1; //pin
#else
		boot_config_copy.enter_bl_check = 0; //pin
#endif
		boot_config_copy.ebl_polarity =  0; /* 0 - active low, 1 active high */
		boot_config_copy.i2c_enable = 1;
		boot_config_copy.spi_enable = 1;
		boot_config_copy.uart_enable = 1;
		boot_config_copy.lock_swd = 0;
		boot_config_copy.valid_mark_check = 1;
		boot_config_copy.crc_check = 0;
		boot_config_copy.i2c_addr = I2C_DEF_SLAVE_ADDR;
		boot_config_copy.ebl_timeout = 0;
		if (update_boot_config() != 0) {
			uart_puts("config_header_default_init has failed\r\n");
		} else {
			memcpy(boot_config, &boot_config_copy, sizeof(boot_config_t));
		}
	} else {
		uart_puts("\r\nebl_pin: ");
		uart_print_hex(boot_config->ebl_pin);
		memcpy(&boot_config_copy, boot_config, sizeof(boot_config_t));
	}
}

void mxm_bootloader_write_mode(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
//	uart_puts("\r\n ** Enter bootloader cmd received");
	tx_buf[0] = BL_SUCCESS; // success
	if (rx_buf[0] == (1 << MODE_BOOTLDR_MODE_BIT)) { // Enter bootloader
		uart_puts("\r\n ** Enter bootloader");
		mxm_bl_cmd_rx_flag = true;
	} else if (rx_buf[0] == 0x00) { // Exit bootloader
		if ((is_app_valid == false) || (is_crc_ok == false)) {
			tx_buf[0] = BL_ERR_BTLDR_INVALID_APP;
			uart_puts("\r\n** No valid application to jump.");
		} else {
			bootldr_mode = BTLDR_MODE_EXIT;
			uart_puts("\r\n** Exit from bootloader.");
		}
	} else if (rx_buf[0] == (1 << MODE_BOOTLDR_RESET_BIT)) { // Restart
		uart_puts("\r\n** Restart device.");
		bootldr_mode = BTLDR_RESTART;
	} else
		tx_buf[0] = BL_ERR_UNAVAIL_CMD; // Not supported. Only one config at a time can be set.

	*tx_len = 1;
	uart_puts("\r\n** Bootloader mode cmd:: ");
	uart_print_hex( rx_buf[0]);
}


void mxm_bootloader_read_mode(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS; // success
	tx_buf[1] = (1 << MODE_BOOTLDR_MODE_BIT);
	*tx_len = 2;

	uart_puts("\r\nMode reg: ");
	uart_print_hex((1 << MODE_BOOTLDR_MODE_BIT));
}

void mxm_wr_bootloader_cmd_set_iv(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS;
	uart_puts("\r\n ** SET IV dummy call");
	*tx_len = 1;
}


void mxm_wr_bootloader_cmd_set_auth(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS;
	uart_puts("\r\n ** SET AUTH dummy call");
	*tx_len = 1;
}

void mxm_wr_bootloader_cmd_set_num_pages(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	num_pages = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
	if (num_pages == 0 || (num_pages - 1) > (total_flash_size >> FLC_PAGE_BIT_SIZE))
		tx_buf[0] = BL_ERR_INPUT_VALUE; // Incorrect page size
	else {
		img_page_size = num_pages;
		got_img_size = 1;
		tx_buf[0] = BL_SUCCESS; // success
	}
	uart_puts("\r\n ** Number of pages: ");
	uart_print_val(num_pages);
	*tx_len = 1;
}

void mxm_wr_bootloader_cmd_erase_app_memory(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_ERR_TRY_AGAIN; // Busy
	uart_puts("\r\n ** Erasing App memory\n");
	btldr_status.busy = 1;
	bootldr_mode = BTLDR_ERASE_APP_MEMORY;

	*tx_len = interface == UART_INTERFACE ? 0 : 1;
}

void mxm_wr_bootloader_cmd_erase_page_memory(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS; // success

	uart_puts("\r\n** Erasing page memory");
	if (btldr_status.busy == 1) { // Check if ready to receive new page
		tx_buf[0] = BL_ERR_TRY_AGAIN; // Busy
	} else {
		page_num_to_erase = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
		btldr_status.busy = 1;
		bootldr_mode = BTLDR_ERASE_PAGE_MEMORY;

		int startPage = (app_start_loc >> FLC_PAGE_BIT_SIZE);
		int lastPage = (flash_size >> FLC_PAGE_BIT_SIZE) + startPage;

		if (page_num_to_erase < startPage || page_num_to_erase > lastPage) {
			tx_buf[0] = BL_ERR_INPUT_VALUE;
		}
	}
	*tx_len = 1;
}

void mxm_wr_bootloader_cmd_set_pl_page_size(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	if (rx_buf == NULL || tx_buf == NULL) {
		uart_puts("Received null buffer at " __func__ "\n");
		return;
	}

	tx_buf[BL_OFFSET_STATUS] = BL_SUCCESS; // success
	uart_puts("\r\n** Setting partial page size");
	if (btldr_status.busy == BL_STATUS_BUSY) { // Check if ready to receive new page
		tx_buf[BL_OFFSET_STATUS] = BL_ERR_TRY_AGAIN; // Busy
	} else {
		partial_rcv_page_size = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];

		if (partial_rcv_page_size < 1 || partial_rcv_page_size > PAGE_PAYLOAD_SIZE) {
			tx_buf[BL_OFFSET_STATUS] = BL_ERR_INPUT_VALUE;
		} else {
			fam_byte_w_bootloader_cmds[BL_SUB_CMD_RECEIVE_PAGE].expected_num_bytes = partial_rcv_page_size;
		}
	}
	*tx_len = BL_STATUS_BYTE_LEN;
}

void mxm_wr_bootloader_cmd_receive_page(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS; // success
	*tx_len = 1;

	uart_puts("\n **** Received page. len: ");
	uart_print_val(rx_len);
	if (!got_img_size) {
		uart_puts("Can't enter flash mode, need number of pages,"
			" IV, and MAC first (num_pages, set_iv, and set_auth commands)\n");
		tx_buf[0] = BL_ERR_UNKNOWN; // Failed
		return;
	}

	if (!flash_erased) {
		txdata[0] = BL_ERR_BTLDR_APP_NOT_ERASED;
		return;
	}

	if (btldr_status.busy == 1) { // Check if ready to receive new page
		tx_buf[0] = BL_ERR_TRY_AGAIN; // Busy
		return;
	}

	if (bootldr_err_code == BTLDR_CHECKSUM_FAIL) { // Check if ready to receive new page
		tx_buf[0] = BL_ERR_BTLDR_CHECKSUM; // Checksum failed
		bootldr_err_code = BTLDR_NO_ERR; // Clear error message
		return;
	}

	if ((rx_len >= 0) && (rx_len <= partial_rcv_page_size)) {
		memset(page_plain + partial_page_offset, 0, rx_len);
		mxm_bl_rx_total_bytes += rx_len;
		memcpy(page_plain + partial_page_offset, rx_buf, rx_len); /* Double check the size to copy */
		partial_page_offset += rx_len;
		fam_byte_w_bootloader_cmds[BL_SUB_CMD_RECEIVE_PAGE].expected_num_bytes = partial_rcv_page_size;
		if (partial_page_offset == PAGE_PAYLOAD_SIZE) {
			partial_page_offset = 0;
			bootldr_mode = BTLDR_FLASHING;
			tx_buf[0] = BL_ERR_TRY_AGAIN; // Busy
			btldr_status.busy = 1; // Bootloader busy until received page flashed in the main loop
		} else if (partial_page_offset > PAGE_PAYLOAD_SIZE){
			partial_page_offset = 0;
			tx_buf[0] = BL_ERR_INPUT_VALUE; // Page size is incorrect
			uart_puts("\r\n ** Partial page size is incorrect: ");
			uart_print_val(partial_page_offset + rx_len);
		} else {
			int remaining = PAGE_PAYLOAD_SIZE - partial_page_offset;
			tx_buf[0] = BL_PARTIAL_ACK;
			if (remaining < partial_rcv_page_size) {
				fam_byte_w_bootloader_cmds[BL_SUB_CMD_RECEIVE_PAGE].expected_num_bytes = remaining;
			}
		}
	} else {
		tx_buf[0] = BL_ERR_INPUT_VALUE; // Page size is incorrect
		uart_puts("\r\n ** Page size is incorrect: ");
		uart_print_val(rx_len);
	}

}

void mxm_bootloader_get_fw_version(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	uart_puts("\r\n** Get FW Version");
	tx_buf[0] = BL_SUCCESS;
	tx_buf[1] = BOOTLOADER_VERSION_MAJOR; /* Major version */
	tx_buf[2] = BOOTLOADER_VERSION_MINOR; /* Minor version */
	tx_buf[3] = BOOTLOADER_VERSION_REVISION; /* Revision */

	*tx_len = 4;
}


void mxm_bootloader_get_page_size(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	uart_puts("\r\n** Get page size");
	tx_buf[0] = BL_SUCCESS;
	tx_buf[1] = FLC_PAGE_SIZE >> 8;
	tx_buf[2] = FLC_PAGE_SIZE & 0xFF;

	*tx_len = 3;
}

void mxm_bootloader_get_usn(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	uart_puts("\r\n** Get USN");
	uint32_t * ptr;
	uint32_t * buf = (uint32_t*)&tx_buf[1];
    ptr = (uint32_t*)MXC_INFO_MEM_BASE;
    tx_buf[0] = BL_SUCCESS;
    for(int i = 0; i < (USN_SIZE/4); i++)
    {
		buf[i] = ((ptr[i]  << 8) & 0xFF00FF00 ) | ((ptr[i]  >> 8) & 0xFF00FF );
		buf[i] = (buf[i] << 16) | (buf[i] >> 16);
    }
	*tx_len = USN_SIZE + 1;
}

__attribute__((section(".ram_code"))) void mxm_bootloader_lock_swd(void)
{
	uint32_t lock_swd_pattern = 0x5a5aa5a5;
	MXC_FLC->cn = 0x20000000;

	MXC_FLC->acntl = 0x3a7f5ca3;
	MXC_FLC->acntl = 0xa1e34f20;
	MXC_FLC->acntl = 0x9608b2c1;
	FLC_Write(0x40000 + 0x0030, 4, (uint8_t*)&lock_swd_pattern);
}

void mxm_bootloader_save_config(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	boot_config_t *pbootconfig = get_pointer_to_bootconfig();
	uart_puts("\r\n** Save config");
	tx_buf[0] = BL_ERR_TRY_AGAIN; // Busy
	if (!pbootconfig->lock_swd && boot_config_copy.lock_swd) {
		mxm_bootloader_lock_swd();
	} else if (pbootconfig->lock_swd && !boot_config_copy.lock_swd) {
		// SWD Lock cannot be unlocked!
		boot_config_copy.lock_swd = pbootconfig->lock_swd;
	}
	bootldr_mode = BTLDR_UPDATE_BOOT_CONFIG;
	btldr_status.busy = 1;
	*tx_len = interface == UART_INTERFACE ? 0 : 1;
}

void mxm_bootloader_config_entry(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS;

	uart_puts("\r\n ** Config entry. cmd: ");
	uart_print_val(rx_buf[0]);
	switch(rx_buf[0]) {
		case BL_CFG_EBL_CHECK: /* Enter BL Check */
			if (check_input_value_range(rx_buf[1], CONFIG_EBL_CHECK_LOWER_LIMIT, CONFIG_EBL_CHECK_UPPER_LIMIT)) {
				boot_config_copy.enter_bl_check = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;

		case BL_CFG_EBL_PIN: /* EBL pin */
			if (check_input_value_range(rx_buf[1] >> 6, CONFIG_EBL_PORT_LOWER_LIMIT, CONFIG_EBL_PORT_UPPER_LIMIT)
				&& check_input_value_range(rx_buf[1] & 0xC0, CONFIG_EBL_PIN_LOWER_LIMIT, CONFIG_EBL_PIN_UPPER_LIMIT)) {
					boot_config_copy.ebl_pin = rx_buf[1] & 0x0FU;
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;

		case BL_CFG_EBL_POL: /* EBL Polarity */
			if (check_input_value_range(rx_buf[1], CONFIG_EBL_POLARITY_LOWER_LIMIT, CONFIG_EBL_POLARITY_UPPER_LIMIT)) {
				boot_config_copy.ebl_polarity = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;

		case BL_CFG_VALID_MARK_CHK: /* Valid Mark Check */
			if (check_input_value_range(rx_buf[1], CONFIG_VALID_MARK_LOWER_LIMIT, CONFIG_VALID_MARK_UPPER_LIMIT)) {
				boot_config_copy.valid_mark_check = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;
		case BL_CFG_UART_ENABLE: /* UART Enable */
			if (check_input_value_range(rx_buf[1], CONFIG_INTERFACE_LOWER_LIMIT, CONFIG_INTERFACE_UPPER_LIMIT)) {
				boot_config_copy.uart_enable = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;
		case BL_CFG_I2C_ENABLE: /* I2C Enable */
			if (check_input_value_range(rx_buf[1], CONFIG_INTERFACE_LOWER_LIMIT, CONFIG_INTERFACE_UPPER_LIMIT)) {
				boot_config_copy.i2c_enable = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;
		case BL_CFG_SPI_ENABLE: /* SPI Enable */
			if (check_input_value_range(rx_buf[1], CONFIG_INTERFACE_LOWER_LIMIT, CONFIG_INTERFACE_UPPER_LIMIT)) {
				boot_config_copy.spi_enable = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;

		case BL_CFG_I2C_SLAVE_ADDR: /* I2C Slave Address */
			if (check_input_value_range(rx_buf[1], CONFIG_I2C_ADDR_LOWER_LIMIT, CONFIG_I2C_ADDR_UPPER_LIMIT)) {
				boot_config_copy.i2c_addr = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;
		case BL_CFG_CRC_CHECK: /* CRC Check */
			if (check_input_value_range(rx_buf[1], CONFIG_CRC_CHECK_LOWER_LIMIT, CONFIG_CRC_CHECK_UPPER_LIMIT)) {
				boot_config_copy.crc_check = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;
		case BL_CFG_LOCK_SWD: /* Lock SWD */
			if (check_input_value_range(rx_buf[1], CONFIG_LOCK_SWD_LOWER_LIMIT, CONFIG_LOCK_SWD_UPPER_LIMIT)) {
				boot_config_copy.lock_swd = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;


		default:
			tx_buf[0] = BL_ERR_UNAVAIL_CMD;
			break;
	}

	*tx_len = 1;
}


void mxm_bootloader_config_exit(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS;
	uart_puts("\r\n ** Config Exit. cmd: ");
	uart_print_val(rx_buf[0]);
	switch(rx_buf[0]) {
		case 0x00: /* Exit BL Mode*/
			if (check_input_value_range(rx_buf[1], CONFIG_EXIT_BL_LOWER_LIMIT, CONFIG_EXIT_BL_UPPER_LIMIT)) {
				if (rx_buf[1] == BL_EXIT_IMMEDIATE) {
					boot_config_copy.exit_bl_mode = BL_EXIT_IMMEDIATE;
				} else if (rx_buf[1] == BL_EXIT_TIMEOUT) {
					boot_config_copy.exit_bl_mode = BL_EXIT_TIMEOUT;
				} else if (rx_buf[1] == BL_EXIT_INDEFINITE) {
					boot_config_copy.exit_bl_mode = BL_EXIT_INDEFINITE;
				} else
					tx_buf[0] = BL_ERR_INPUT_VALUE;
			}
			break;

		case 0x01: /* Exit Timeout */
			if (check_input_value_range(rx_buf[1], CONFIG_EBL_TIMEOUT_LOWER_LIMIT, CONFIG_EBL_TIMEOUT_UPPER_LIMIT)) {
				boot_config_copy.ebl_timeout = rx_buf[1];
			} else
				tx_buf[0] = BL_ERR_INPUT_VALUE;
			break;

		default:
			tx_buf[0] = BL_ERR_UNAVAIL_CMD;
			break;
	}

	*tx_len = 1;
}

void mxm_bootloader_get_config_entry(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS;
	boot_config_t *pbootconfig = get_pointer_to_bootconfig();
	uart_puts("\r\n ** Get entry configs. cmd: ");
	uart_print_val(rx_buf[0]);
	switch(rx_buf[0]) {
		case BL_CFG_EBL_CHECK: /* Enter BL Check */
			tx_buf[1] = pbootconfig->enter_bl_check;
			break;
		case BL_CFG_EBL_PIN: /* EBL Pin */
			tx_buf[1] = pbootconfig->ebl_pin;
			break;
		case BL_CFG_EBL_POL: /* EBL Polarity */
			tx_buf[1] = pbootconfig->ebl_polarity;
			break;
		case BL_CFG_VALID_MARK_CHK: /* Valid Mark Check */
			tx_buf[1] = pbootconfig->valid_mark_check;
			break;
		case BL_CFG_UART_ENABLE: /* UART Enable */
			tx_buf[1] = pbootconfig->uart_enable;
			break;
		case BL_CFG_I2C_ENABLE: /* I2C Enable */
			tx_buf[1] = pbootconfig->i2c_enable;
			break;
		case BL_CFG_SPI_ENABLE: /* SPI Enable */
			tx_buf[1] = pbootconfig->spi_enable;
			break;
		case BL_CFG_I2C_SLAVE_ADDR: /* I2C Slave Address */
			tx_buf[1] = pbootconfig->i2c_addr;
			break;
		case BL_CFG_CRC_CHECK: /* CRC Check */
			tx_buf[1] = pbootconfig->crc_check;
			break;
		case BL_CFG_LOCK_SWD: /* Lock SWD */
			tx_buf[1] = pbootconfig->lock_swd;
			break;
		default:
			tx_buf[0] = BL_ERR_UNAVAIL_CMD;
			break;
	}

	*tx_len = 2;
}

void mxm_bootloader_get_config_exit(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	tx_buf[0] = BL_SUCCESS;

	uart_puts("\r\n ** Get exit configs. cmd: ");
	uart_print_val(rx_buf[0]);
	boot_config_t *pbootconfig = get_pointer_to_bootconfig();
	/* Read Bootloader Exit Configuration */
	switch(rx_buf[0]) {
		case 0x00: /* Exit BL Mode*/
			tx_buf[1] = pbootconfig->exit_bl_mode;
			break;
		case 0x01: /* Exit Timeout */
			tx_buf[1] = pbootconfig->ebl_timeout;
			break;

		default:
			tx_buf[0] = BL_ERR_UNAVAIL_CMD;
			break;
	}

	*tx_len = 2;
}


void mxm_bootloader_get_config_bl(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
    tx_buf[0] = BL_SUCCESS;
    boot_config_t *pbootconfig = get_pointer_to_bootconfig();
    for(int i = 0; i < sizeof(boot_config_t); i++) {
    	tx_buf[1+i] = pbootconfig->v[sizeof(boot_config_t) - 1 -i];
    }

	*tx_len = sizeof(boot_config_t) + 1;
}

void mxm_bootloader_platform_type(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len)
{
	uart_puts("\n ** Get platform type.\n");
	tx_buf[0] = BL_SUCCESS; // success
	tx_buf[1] = 0x01; // ME11

	*tx_len = 2;
}

int mxm_bl_find_fam_cmd(uint8_t *packet)
{
	int lo = 0;
	int hi = ARRAY_SIZE(cmd_table) - 1;
	uint8_t cmd = packet[0];
	int idx = -1;

	/* Binary search to find family byte */
	while (lo <= hi) {
		int mid = (lo + hi) >> 1;

		if (cmd_table[mid].family_byte == cmd) {
			idx = mid;
			break;
		}

		if (cmd_table[mid].family_byte < cmd) {
			lo = mid + 1;
		} else {
			hi = mid - 1;
		}
	}

	/* Not found */
	if (idx == -1) {
		return -1;
	}

	uint8_t sub_cmd = packet[1];
	for (int i = 0; i < cmd_table[idx].len; i++) {
		if (cmd_table[idx].cmd[i].sub_cmd == sub_cmd) {
			bl_msg.cb = cmd_table[idx].cmd[i].cb;
			bl_msg.payload_len = cmd_table[idx].cmd[i].expected_num_bytes;
			return 0;
		}
	}

	return -1;
}

int bootloader(void)
{
	/**********************************************************
	* variables created by linker or in assembly are represented as addresses
	* but c compiler treats it as the value pointed to by that address.
	* it must be referenced to get the actual address we want.
	***********************************************************/
	int mfio_state;
	int ret;
	volatile boot_config_t* boot_config = get_pointer_to_bootconfig();
	platform_tick_init();
	flash_size = (unsigned long)&_app_end - (unsigned long)&_app_start;
	app_start_loc = (unsigned long)&_app_start;
	total_flash_size = flash_size + (unsigned long)&_boot_mem_end - (unsigned long)&_boot_mem_start;

	bootconfig_default_init();

#if defined(SWD_LOCK)
	MXC_GCR->scon |= MXC_S_GCR_SCON_CHKRD_READY;
	#warning "*********** SWD Locked...***********"
#endif
	memset(rxdata, 0, sizeof(rxdata));
	memset(txdata, 0, sizeof(txdata));
	got_img_size = 0;
#ifdef BOOTLOADER_UART
	if(boot_config->uart_enable) {
		mxm_bl_uart_init();
		uart_puts("\n\n UART initialized\r\n");
	}
#endif
#ifdef BOOTLOADER_SPI
	if(boot_config->spi_enable) {
		mxm_bl_spi_init();
		uart_puts("\n SPI initialized\r\n");
	}
#endif
#ifdef BOOTLOADER_I2C
	if(boot_config->i2c_enable) {
		mxm_bl_i2c_init();
		uart_puts("\n I2C initialized\r\n");
	}
#endif

	mfio_state = mfio_init();


#if defined(BOOTLOADER_BUILD_INFO) || defined(BOOTLOADER_DEBUG_MSG)
	bootloader_console_init();

	mxc_delay(MXC_DELAY_MSEC(100));
	uart_puts("\r\n\r\n----- Maxim Integrated -----\r\n");
	uart_puts("Board name: ");
	uart_puts(BOOTLOADER_PLATFORM);
	uart_puts(", built: ");
	uart_puts(__BUILD_TIME__);

	uart_puts("\r\nCommit-Id: ");
	uart_puts(__BUILD_COMMIT_ID__);
	uart_puts("\r\nBoard Init...\r\n");
	uart_puts("Version: ");
	uart_puts(BOOTLOADER_VERSION);

	uart_puts("\r\nBoot_mode addr: ");
	uart_print_hex((uint32_t)&_boot_mode);

	dump_app_header();
	dump_bootloader_config();
#endif

	is_app_valid = check_if_app_is_valid(boot_config->valid_mark_check);
	if (is_app_valid == true)
		is_crc_ok = check_app_crc(boot_config->crc_check);

	uint32_t time_passed = platform_get_time_ms();
	int start_delay = BL_START_DELAY_MS - time_passed; // 20 msec
	if (!mxm_bl_cmd_rx_flag && start_delay > 0 && start_delay <= BL_START_DELAY_MS){
		while (!mxm_bl_cmd_rx_flag && start_delay--) {
			mxc_delay(MXC_DELAY_MSEC(1));
		}
	}

	if (is_app_valid == false) {
		uart_puts("\r\nNo valid application. Stay in bootloader mode");
	} else if (is_crc_ok == false) {
		uart_puts("\r\nApplication CRC is wrong. Stay in bootloader mode");
	} else if (_boot_mode == BOOTMODE_SET) {
		uart_puts("\r\nStaying in bootloader mode (boot_mode set)");
	} else if (should_enter_bootloader(mfio_state)) {
		uart_puts("\r\nShould stay in bootloader\r\n");
	} else {
		uart_puts("\r\nFound valid application. Exit bootloader mode");
		goto exit;
	}

	uart_puts("\r\n*** Enter Bootloader Mode\n");
	uart_puts("\r\n");

	while (bootldr_mode != BTLDR_MODE_EXIT) {
		if (bootldr_mode == BTLDR_ERASE_APP_MEMORY) {
			ret = master_erase();
			if (ret) {
				uart_puts("\r\nFailed Erasing application memory.");
				flash_erased = false;
				txdata[0] = BL_ERR_UNKNOWN;
			} else {
				flash_erased = true;
				txdata[0] = BL_SUCCESS;
			}
			send_response(BL_STATUS_BYTE_LEN);
			__disable_irq();
			btldr_status.busy = 0;
			bootldr_mode = BTLDR_IDLE;
			__enable_irq();
		} else if (bootldr_mode == BTLDR_ERASE_PAGE_MEMORY) {
			uart_puts("\r\n** Erasing page number: ");
			uart_print_val(page_num_to_erase);
			int ret = bootloader_page_erase(page_num_to_erase);
			if (ret == 0)
				uart_puts("\r\nErased page.");
			__disable_irq();
			bootldr_mode = BTLDR_IDLE;
			btldr_status.busy = 0;
			__enable_irq();
		} else if (bootldr_mode == BTLDR_FLASHING) {
			uart_puts("\r\nEntered flash mode.");
			mxm_bl_receive_page_and_flash();
			bootldr_mode = BTLDR_IDLE;
			flash_erased = false;
		} else if (bootldr_mode == BTLDR_UPDATE_BOOT_CONFIG) {
			uart_puts("\r\nUpdate boot config");
			ret = update_boot_config(); // save settings mode
			if (ret) {
				uart_puts("\r\nFailed updating boot config.");
				txdata[0] = BL_ERR_UNKNOWN;
			} else {
				txdata[0] = BL_SUCCESS;
			}
			send_response(BL_STATUS_BYTE_LEN);
			__disable_irq();
			btldr_status.busy = 0;
			bootldr_mode = BTLDR_IDLE;
			__enable_irq();
		} else if (bootldr_mode == BTLDR_RESTART) {
			bootldr_mode = BTLDR_IDLE;
			mxc_delay(MXC_DELAY_MSEC(50));
			NVIC_SystemReset();
		}
	}

	mxc_delay(MXC_DELAY_MSEC(5));
exit:
	uart_puts("\r\nJumping to Application's Reset Handler...\r\n\r\n");
	__disable_irq();

#ifdef BOOTLOADER_UART
	mxm_bl_uart_shutdown();
#endif
#ifdef BOOTLOADER_SPI
	mxm_bl_spi_shutdown();
#endif
#ifdef BOOTLOADER_I2C
	mxm_bl_i2c_shutdown();
#endif
#if defined(BOOTLOADER_BUILD_INFO) || defined(BOOTLOADER_DEBUG_MSG)
	mxc_delay(MXC_DELAY_MSEC(10));
	bootloader_console_shutdown();
#endif
	__enable_irq();
	((void(*)())_app_isr[1])();
	return 0;
}
