/*******************************************************************************
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
 *******************************************************************************
 */

#include "DSInterface.h"
#include "SSBootloaderComm.h"
#include "SSInterface.h"
#include "Peripherals.h"
#include "assert.h"
#include "utils.h"

static bool flag_image_on_ram = false;

static app_image_t app_image;

static const char* const cmd_tbl[] = {
	"bootldr",
	"exit",
	"reset",
	"page_size",
	"get_usn",
	"num_pages",
	"set_iv",
	"set_auth",
	"erase",
	"page_erase",
	"flash",
	"image_flash",
	"set_cfg bl enter_mode",
	"set_cfg bl enter_pin",
	"set_cfg bl enter_pol",
	"set_cfg bl exit_mode",
	"set_cfg bl exit_to",
	"set_cfg bl valid",
	"set_cfg bl uart",
	"set_cfg bl i2c",
	"set_cfg bl spi",
	"set_cfg bl addr_i2c",
	"set_cfg bl crc",
	"set_cfg bl swd_lock",
	"set_cfg bl save",
	"get_cfg bl",
	"image_on_ram",
	"set_cfg host ebl",
	"set_cfg host cdf", /* bootloader comm_delay_factor */
	"get_cfg host",
	"set_cfg comm",
	"set_partial_size",
	"set_cfg i2c addr",
};

#define NUM_OF_SSBOOTLOADERCOMM_COMMANDS (sizeof(CMDTABLE_SSBOOTLOADERCOMM) / sizeof(*CMDTABLE_SSBOOTLOADERCOMM))

static const char* const comm_tbl[] = {
	"i2c",
	"spi",
	"uart",
};



SSBootloaderComm::SSBootloaderComm(USBSerial *USB, SSInterface* ssInterface, DSInterface* dsInterface)
	:SensorComm("bl", false), m_USB(USB), ss_int(ssInterface), ds_int(dsInterface)
{
	app_image.num_pages = -1;
	app_image.num_allocated_pages = MAX_PAGE_NUMBER;
	page_size = MAX_PAGE_SIZE;
	bl_comm_delay_factor = 2;
	page_transport_size = PAGE_PAYLOAD_SIZE;
}

SSBootloaderComm::SSBootloaderComm(USBSerial* USB, SSInterface* ssInterfaceSpi, SSInterface* ssInterfaceI2c, SSInterface* ssInterfaceUart, DSInterface* dsInterface)
	:SensorComm("bl", false), m_USB(USB), ss_int_spi(ssInterfaceSpi), ss_int_i2c(ssInterfaceI2c), ss_int_uart(ssInterfaceUart), ds_int(dsInterface)
{
	app_image.num_pages = -1;
	app_image.num_allocated_pages = MAX_PAGE_NUMBER;
	page_size = MAX_PAGE_SIZE;
	bl_comm_delay_factor = 2;
	page_transport_size = PAGE_PAYLOAD_SIZE;
	#if defined(SSINTERFACE_USES_I2C)
		ss_int = ss_int_i2c;
	#elif defined(SSINTERFACE_USES_UART)
		ss_int = ss_int_uart;
	#elif defined(SSINTERFACE_USES_SPI)
		ss_int = ss_int_spi;
	#else
		#error "SSInterface requires communication settings."
	#endif
}

int SSBootloaderComm::parse_auth(const char* cmd, uint8_t *auth_bytes)
{
	int length = strlen(cmd);
	int expected_length = sizeof("set_auth") + 2 * AES_AUTH_SIZE;
	if (length != expected_length) {
		pr_err("Couldn't parse Auth bytes, incorrect number of characters (len:%d, expected:%d)\n",
			length, expected_length);
		return COMM_INVALID_PARAM;
	}

	const char* macPtr = cmd + sizeof("set_auth");

	int num_found;
	int byteVal;
	for (int aidx = 0; aidx < AES_AUTH_SIZE; aidx++) {
		num_found = sscanf(macPtr, "%2X", &byteVal);

		if (num_found != 1 || byteVal > 0xFF) {
			pr_err("Couldn't parse byte %d of Auth\n", aidx);
			return COMM_INVALID_PARAM;
		}

		auth_bytes[aidx] = (uint8_t)byteVal;
		macPtr += 2;
	}

	return COMM_SUCCESS;
}

int SSBootloaderComm::parse_iv(const char* cmd, uint8_t* iv_bytes)
{
	int length = strlen(cmd);
	int expected_length = sizeof("set_iv") + 2 * AES_NONCE_SIZE;
	if (length != expected_length) {
		pr_err("Couldn't parse IV, incorrect number of characters (len:%d, expected:%d)\n",
			   length, expected_length);
		return COMM_INVALID_PARAM;
	}

	const char* ivPtr = cmd + sizeof("set_iv");

	int num_found;
	int byteVal;
	for (int ividx = 0; ividx < AES_NONCE_SIZE; ividx++) {
		num_found = sscanf(ivPtr, "%2X", &byteVal);

		if (num_found != 1 || byteVal > 0xFF) {
			pr_err("Couldn't parse byte %d of IV\n", ividx);
			return COMM_INVALID_PARAM;
		}
		iv_bytes[ividx] = (uint8_t)byteVal;
		ivPtr += 2;
	}

	return COMM_SUCCESS;
}

int SSBootloaderComm::parse_comm(const char* cmd, uint8_t* comm_index, int command_index)
{
	const char ch = ' ';
	char *interface;

	interface = strrchr(cmd, ch);
	interface++;
	for(int i=0; i <NUM_COMM; i++) {
		if(strcmp(interface, comm_tbl[i]) == 0) {
			*comm_index = i;
			return COMM_SUCCESS;
		}
	}
	return COMM_INVALID_PARAM;
}

int SSBootloaderComm::bootldr_set_num_pages(int num_pages)
{
	uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SETNUMPAGES };
	uint8_t data[] = { (uint8_t)((num_pages >> 8) & 0xFF), (uint8_t)(num_pages & 0xFF) };

	SS_STATUS status = ss_int->write_cmd(
			&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
			&data[0], ARRAY_SIZE(data),
			 bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);

	pr_info("Set number of pages: %d\n", num_pages);
	if (status != SS_SUCCESS && status != SS_BTLDR_SUCCESS) {
		return COMM_GENERAL_ERROR;
	}

	sent_num_pages = true;
	return COMM_SUCCESS;
}

int SSBootloaderComm::bootldr_get_page_size(int &page_size)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_BOOTLOADER, SS_CMDIDX_PAGESIZE };
	uint8_t rxbuf[3];

	SS_STATUS status = ss_int->pInterface->read_cmd(
			&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
			0, 0,
			&rxbuf[0], ARRAY_SIZE(rxbuf));

	if (status != SS_SUCCESS && status != SS_BTLDR_SUCCESS) {
		return COMM_GENERAL_ERROR;
	}

	//rxbuf holds page size in big-endian format
	page_size = 256*rxbuf[1] + rxbuf[2];
	assert_msg(page_size <= MAX_PAGE_SIZE, "Page size exceeds maximum allowed");

	got_page_size = true;
	return COMM_SUCCESS;
}

int SSBootloaderComm::bootldr_get_usn(uint8_t* usn)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_BOOTLOADER, SS_CMDIDX_READUSN };
	uint8_t rxbuf[USN_SIZE+1];

	SS_STATUS status = ss_int->pInterface->read_cmd(
			&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
			0, 0,
			&rxbuf[0], ARRAY_SIZE(rxbuf));

	if (status != SS_SUCCESS && status != SS_BTLDR_SUCCESS) {
		return COMM_GENERAL_ERROR;
	}

	//rxbuf holds page size in big-endian format
	memcpy(usn, &rxbuf[1], USN_SIZE);

	return COMM_SUCCESS;
}

int SSBootloaderComm::bootldr_set_iv(uint8_t *iv_bytes)
{
	uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SETIV };

	SS_STATUS status = ss_int->write_cmd(
		&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
		&iv_bytes[0], AES_NONCE_SIZE,
		bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);

	if (status != SS_SUCCESS && status != SS_BTLDR_SUCCESS) {
		return COMM_GENERAL_ERROR;
	}

	return COMM_SUCCESS;
}

int SSBootloaderComm::bootldr_set_auth(uint8_t *auth_bytes)
{
	uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SETAUTH };

	SS_STATUS status = ss_int->write_cmd(
		&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
		&auth_bytes[0], AES_AUTH_SIZE,
		bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);

	if (status != SS_SUCCESS && status != SS_BTLDR_SUCCESS) {
		return COMM_GENERAL_ERROR;
	}

	return COMM_SUCCESS;
}

int SSBootloaderComm::bootldr_erase_app_memory(void)
{

	uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_ERASE };

	SS_STATUS status = ss_int->write_cmd(
			&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
			0, 0,
			 bl_comm_delay_factor * SS_BOOTLOADER_ERASE_DELAY);

	if (status != SS_SUCCESS && status != SS_BTLDR_SUCCESS) {
		return COMM_GENERAL_ERROR;
	}

	return COMM_SUCCESS;
}

bool SSBootloaderComm::parse_command(const char* cmd)
{
	int ret = EXIT_SUCCESS;
	bool recognizedCmd = false;
	int cmd_id = 0;

	if (!ss_int) {
		pr_err("No SmartSensor Interface defined!");
		return false;
	}
	if (!ds_int) {
		pr_err("No DeviceStudio Interface defined!");
		return false;
	}

	while(cmd_id < NUM_CMDS) {
		if (starts_with(cmd, cmd_tbl[cmd_id])) {
			recognizedCmd = true;
			break;
		}

		cmd_id++;
	}

	if (!recognizedCmd)
		return false;

	switch (cmd_id) {
		case CMD_ENTER_BOOTLDR:
		{
			SS_STATUS status;
			status = ss_int->reset_to_bootloader();
			if (status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

			ds_int->set_fw_platform(ss_int->get_ss_platform_name());
			ds_int->set_fw_version(ss_int->get_ss_fw_version());
#if 0
			got_page_size = false;
			sent_num_pages = false;
#endif
		} break;

		case CMD_EXIT_BOOTLDR:
		{
			SS_STATUS status = ss_int->exit_from_bootloader();
			if (status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

			ds_int->set_fw_platform(ss_int->get_ss_platform_name());
			ds_int->set_fw_version(ss_int->get_ss_fw_version());

		} break;

		case CMD_RESET:
		{
			SS_STATUS status = ss_int->reset();
			if (status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

			ds_int->set_fw_platform(ss_int->get_ss_platform_name());
			ds_int->set_fw_version(ss_int->get_ss_fw_version());

		} break;

		case CMD_PAGE_SIZE:
		{
			if (flag_image_on_ram) {
				m_USB->printf("\r\n%s value=%d err=%d\r\n", cmd, MAX_PAGE_SIZE, COMM_SUCCESS);
				break;
			} else {
				int page_size = 0;
				int ret = bootldr_get_page_size(page_size);
				m_USB->printf("\r\n%s value=%d err=%d\r\n", cmd, page_size, ret);
			}
		} break;

		case CMD_GET_USN:
		{
			uint8_t usn[USN_SIZE];
			int ret = bootldr_get_usn(usn);
			m_USB->printf("\r\n%s value=", cmd);
			for (int i=0; i<USN_SIZE; i++)
				m_USB->printf("%02x",usn[i]);
			m_USB->printf(" err=%d\r\n", ret);
		} break;

		case CMD_NUM_PAGES:
		{
			int num_tok = sscanf(cmd, "num_pages %d", &num_pages);
			if (num_tok != 1) {
				m_USB->printf("\r\n%s value=%d err=%d\r\n", cmd, 0, COMM_INVALID_PARAM);
				break;
			}

			if (flag_image_on_ram) {
				if (num_pages > MAX_PAGE_NUMBER) {
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
					break;
				}

				app_image.num_received_pages = 0;
				app_image.num_pages = num_pages;
			} else
				ret = bootldr_set_num_pages(num_pages);
			m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);
		} break;

		case CMD_SET_IV:
		{
			uint8_t iv_bytes[AES_NONCE_SIZE];
			ret = parse_iv(cmd, &iv_bytes[0]);
			if (ret != COMM_SUCCESS) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);
				break;
			}

			if (flag_image_on_ram) {
				memcpy(app_image.nonce, iv_bytes, AES_NONCE_SIZE);
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				break;
			}

			int ret = bootldr_set_iv(iv_bytes);
			m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);
		} break;

		case CMD_SET_AUTH:
		{
			uint8_t auth_bytes[AES_AUTH_SIZE];
			ret = parse_auth(cmd, &auth_bytes[0]);
			if (ret != COMM_SUCCESS) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);
				break;
			}

			if (flag_image_on_ram) {
				memcpy(app_image.auth , auth_bytes, AES_AUTH_SIZE);
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				break;
			}

			int ret = bootldr_set_auth(auth_bytes);
			m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);
		} break;

		case CMD_ERASE:
		{
			if (flag_image_on_ram) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				break;
			}

			int ret = bootldr_erase_app_memory();
			m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);
		} break;

		case CMD_PAGE_ERASE:
		{
			int page_num_to_erase;
			int num_tok = sscanf(cmd, "page_erase %d", &page_num_to_erase);
			if (num_tok != 1) {
				m_USB->printf("\r\n%s value=%d err=%d\r\n", cmd, 0, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_ERASE_PAGE };
			uint8_t data[] = { (uint8_t)((page_num_to_erase >> 8) & 0xFF), (uint8_t)(page_num_to_erase & 0xFF) };

			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					&data[0], ARRAY_SIZE(data),
					bl_comm_delay_factor * PAGE_ERASE_CMD_SLEEP_MS);

			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				sent_num_pages = true;
			} else {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
			}
		} break;

		case CMD_FLASH_IMAGE:
		{
			if (flag_image_on_ram) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);
				int ret = flash_app_image();
				break;
			}

			m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
		} break;

		case CMD_FLASH:
		{

			if (flag_image_on_ram) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				int ret = bootldr_receive_image();
				break;
			}

			if (got_page_size && sent_num_pages) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				flash_page_data();
			} else {
				pr_info("Can't enter flash mode. Need number of pages, and size of page"
						"(num_pages, page_size, commands)\r\n");
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
			}
		} break;

		case CMD_SETCFG_BL_ENTER_MODE:
		{

			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_CFG_ENTER_BL_MODE, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

		case CMD_SETCFG_BL_EBL_PIN:
		{
			uint8_t pin[2];
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &pin[0], 2, false);
			if (ret != 2) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t ebl_pin = ((pin[0] & 0x03) << 6) | (pin[1] & 0x3F);
			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_CFG_EBL_PIN, ebl_pin };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

		case CMD_SETCFG_BL_EBL_POL:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_CFG_EBL_POL, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

		case CMD_SETCFG_BL_VALID:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_VALID_MARK_CHK, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

		case CMD_SETCFG_BL_UART:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_UART_ENABLE, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

		case CMD_SETCFG_BL_I2C:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_I2C_ENABLE, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

		case CMD_SETCFG_BL_SPI:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_SPI_ENABLE, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

		case CMD_SETCFG_BL_I2C_ADDR:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_I2C_SLAVE_ADDR, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

		case CMD_SETCFG_BL_CRC_CHK:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_CRC_CHECK, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;
		
		case CMD_SETCFG_BL_LOCK_SWD:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_CFG_LOCK_SWD, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS){
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			} else{
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
			}
		} break;

		case CMD_SETCFG_BL_EXIT_MODE:
		{
			uint8_t mode;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_EXIT, SS_BL_CFG_EXIT_BL_MODE, mode };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
		} break;

		case CMD_SETCFG_BL_TIMEOUT:
		{
			uint8_t to;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &to, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_EXIT, SS_BL_CFG_TIMEOUT, to };
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;


		case CMD_SETCFG_BL_SAVE:
		{
			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_SAVE };

			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0, bl_comm_delay_factor * BL_CFG_SAVE_CMD_SLEEP_MS);
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;


		case CMD_GETCFG_BL:
		{
			uint8_t cmd_bytes[] = { SS_FAM_R_BOOTLOADER_CFG, SS_CMDIDX_R_ALL_CFG, 0x00 };
			uint8_t rxbuf[9];
			uint8_t receive_size = ARRAY_SIZE(rxbuf);

			if (compare_versions(ds_int->get_fw_version(), "3.4.2") > 0) {
				receive_size = 5;
			}

			SS_STATUS status = ss_int->pInterface->read_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					0, 0,
					&rxbuf[0], receive_size);

			boot_config_t *boot_config = (boot_config_t *)&rxbuf[1];
			boot_config->cfg = __REV(boot_config->cfg);
			// Valid for Bootloaders with >= 3.4.2 version
			boot_large_config_t *boot_lconfig = (boot_large_config_t *)&rxbuf[1];
			boot_lconfig->cfg = __builtin_bswap64(boot_lconfig->cfg);

			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS) {
				if (compare_versions(ds_int->get_fw_version(), "3.4.2") <= 0) {
					m_USB->printf("\r\n%s enter_bl_check=%d ebl_pin=%d ebl_port=%d ebl_polarity=%d valid_mark_check=%d uart_enable=%d i2c_enable=%d spi_enable=%d i2c_addr=%d crc_check=%d ebl_timeout=%d swd_lock=%d exit_bl_mode=%d err=%d \r\n",
															cmd,
															boot_lconfig->enter_bl_check,
															boot_lconfig->ebl_pin,
															0, /* ebl_port placeholder */
															boot_lconfig->ebl_polarity,
															boot_lconfig->valid_mark_check,
															boot_lconfig->uart_enable,
															boot_lconfig->i2c_enable,
															boot_lconfig->spi_enable,
															boot_lconfig->i2c_addr,
															boot_lconfig->crc_check,
															boot_lconfig->ebl_timeout,
															boot_lconfig->lock_swd,
															boot_lconfig->exit_bl_mode,
															COMM_SUCCESS);
				} else {
					m_USB->printf("\r\n%s enter_bl_check=%d ebl_pin=%d ebl_port=%d ebl_polarity=%d valid_mark_check=%d uart_enable=%d i2c_enable=%d spi_enable=%d i2c_addr=%d crc_check=%d ebl_timeout=%d swd_lock=%d exit_bl_mode=%d err=%d \r\n",
															cmd,
															boot_config->enter_bl_check,
															boot_config->ebl_pin,
															boot_config->ebl_port,
															boot_config->ebl_polarity,
															boot_config->valid_mark_check,
															boot_config->uart_enable,
															boot_config->i2c_enable,
															boot_config->spi_enable,
															boot_config->i2c_addr,
															boot_config->crc_check,
															boot_config->ebl_timeout,
															boot_config->lock_swd,
															boot_config->exit_bl_mode,
															COMM_SUCCESS);
				}

			} else {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
			}
		}
		break;


		case CMD_IMAGE_ON_RAM:
		{
			uint8_t mode = 0;
			ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &mode, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}


			flag_image_on_ram = !!mode;
			pr_info("flag_image_on_ram: %d, mode: %d\n", flag_image_on_ram, mode);
			m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			break;
		}

		case CMD_SETCFG_HOST_EBL:
		{
			uint8_t tmp; /* 0: Command bootloader, 1: No command, trigger GPIO */

			int ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &tmp, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			if (ss_int->set_ebl_mode(tmp) == SS_SUCCESS)
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			else
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
			break;
		}
		break;

		case CMD_SETCFG_HOST_CDF:
		{
			uint8_t tmp; /* 0: Command bootloader, 1: No command, trigger GPIO */

			int ret = parse_cmd_data(cmd, cmd_tbl[cmd_id], &tmp, 1, false);
			if (ret != 1) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			bl_comm_delay_factor = tmp;
			m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			break;
		}
		break;

		case CMD_SETCFG_HOST_COMM:
		{
			uint8_t tmp; /* 0: I2C, 1: SPI, 2: UART, others: error */

			int ret = parse_comm(cmd, &tmp, CMD_SETCFG_HOST_COMM);
			if (ret != COMM_SUCCESS) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			if(tmp == COMM_I2C)
				ss_int = ss_int_i2c;
			else if(tmp == COMM_SPI)
				ss_int = ss_int_spi;
			else if(tmp == COMM_UART)
				ss_int = ss_int_uart;
			else
				assert_msg(false, "Invalid communication index!");

			m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
			break;
		}
		break;

		case CMD_GETCFG_HOST:
		{
			m_USB->printf("\r\n%s ebl=%d cdf=%d err=%d\r\n", cmd, ss_int->get_ebl_mode(), bl_comm_delay_factor, COMM_SUCCESS);
		}
		break;
		
		case CMD_SET_PARTIAL_PAGE_SIZE:
		{
			int partial_size;
			int num_tok = sscanf(cmd, "set_partial_size %d", &partial_size);
			if (num_tok != 1 || partial_size < 1 || partial_size > (PAGE_PAYLOAD_SIZE)) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SET_PARTIAL_PAGE_SIZE };
			uint8_t data[] = { (uint8_t)((partial_size >> 8) & 0xFF), (uint8_t)(partial_size & 0xFF) };
		
			SS_STATUS status = ss_int->write_cmd(
					&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
					&data[0], ARRAY_SIZE(data),
					bl_comm_delay_factor * PAGE_ERASE_CMD_SLEEP_MS);
		
			if (status == SS_SUCCESS || status == SS_BTLDR_SUCCESS) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				page_transport_size = partial_size;
			} else {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
			}
		}
		break;

		case CMD_SET_I2C_SLAVE_ADDR:
		{
			int new_addr;
			int num_tok = sscanf(cmd, "set_cfg i2c addr %d", &new_addr);
			if (num_tok != 1 || new_addr < VALID_I2C_SLAVE_ADDR_LOW || new_addr > (VALID_I2C_SLAVE_ADDR_HIGH)) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
				break;
			}

			if (ss_int_i2c->pInterface->change_slave_addr(new_addr)) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
				break;
			}

			m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
		}
		break;

		default:
		{
			assert_msg(false, "Invalid switch case!");
		}
	}

	return recognizedCmd;
}

void dbg_puts(char *str)
{
	Peripherals::daplinkSerial()->puts(str);
}

void dump_hex(const char *label, uint8_t *buf, int len, int num_col)
{
	dbg_puts((char *)label);
	for(int i = 0; i < len; i++) {
		char tmp_buf[12];

		if (i % num_col == 0)
			dbg_puts("\r\n");
		snprintf(tmp_buf, sizeof(tmp_buf), "%.2x ", buf[i]);
		dbg_puts(tmp_buf);
	}
}

int SSBootloaderComm::bootldr_receive_image(void)
{
	int totalBytes = 0;
	int currentPage = 1;
	uint8_t *page = &app_image.pages[0];
	uint32_t offset = 0;

	if (app_image.pages == NULL || app_image.num_allocated_pages < 0) {
		pr_info("Page addr: %p, num_allocated_pages: %d\n",
				app_image.pages, app_image.num_allocated_pages);
		return COMM_GENERAL_ERROR;
	}

	while (currentPage <= app_image.num_pages) {
		pr_info("Waiting for page %d/%d data...",
				currentPage, app_image.num_pages);

		while (totalBytes < (MAX_PAGE_SIZE + CHECKBYTES_SIZE)) {
			page[totalBytes++] = m_USB->_getc();
		}

		currentPage++;
		m_USB->printf("\r\npageFlashDone err=%d\r\n", COMM_SUCCESS);

		offset += MAX_PAGE_SIZE + CHECKBYTES_SIZE;
		page = &app_image.pages[offset];
		totalBytes = 0;
	}

	app_image.num_received_pages = currentPage;
	return COMM_SUCCESS;
}

int SSBootloaderComm::flash_app_image(void)
{
	int currentPage = 1;
	uint32_t offset = 0;
	int ret;

	/* Put device to bootloader mode */
	SS_STATUS status;
	status = ss_int->reset_to_bootloader();
	got_page_size = true;
	sent_num_pages = true;

	wait_ms(10);
	pr_info("*** Device in bootldr mode. status: %d\n", status);
	/* Set number of pages to download */
	ret = bootldr_set_num_pages(app_image.num_pages);
	pr_info("*** set_num_page... ret: %d\n", ret);
	if (ret != 0) {
		return BL_SET_NUM_PAGES_FAIL;
	}

	ret = bootldr_set_iv(app_image.nonce);
	pr_info("*** set_iv... ret: %d\n", ret);
	if (ret != 0) {
		return BL_SET_IV_FAIL;
	}

	ret = bootldr_set_auth(app_image.auth);
	pr_info("*** set_auth... ret: %d\n", ret);
	if (ret != 0) {
		return BL_FLASH_ERR_AUTH;
	}

	ret = bootldr_erase_app_memory();
	pr_info("*** erase app memory... ret: %d\n", ret);
	if (ret != 0) {
		return BL_FLASS_ERASE_FAIL;
	}

	static uint8_t tx_buf[MAX_PAGE_SIZE + CHECKBYTES_SIZE + 2];
	tx_buf[0] = SS_FAM_W_BOOTLOADER;
	tx_buf[1] = SS_CMDIDX_SENDPAGE;
	while (currentPage <= app_image.num_pages) {
		pr_info("Transferring page %d/%d data (%d bytes)",
				currentPage, num_pages, MAX_PAGE_SIZE + CHECKBYTES_SIZE);

		memcpy(&tx_buf[2], &app_image.pages[offset], MAX_PAGE_SIZE + CHECKBYTES_SIZE);
		SS_STATUS status = ss_int->pInterface->write_cmd(tx_buf, MAX_PAGE_SIZE + CHECKBYTES_SIZE + 2,
				bl_comm_delay_factor * PAGE_WRITE_DELAY_MS);
		if (status == SS_ERR_BTLDR_CHECKSUM) {
			pr_info("Verify checksum failed!\r\n");
			m_USB->printf("\r\npageFlashDone err=%d\r\n", BL_FLASH_ERR_CHECKSUM);
			break;
		} else if (status != SS_SUCCESS && status != SS_BTLDR_SUCCESS) {
			pr_info("Page flash failed!\r\n");
			m_USB->printf("\r\npageFlashDone err=%d\r\n", BL_FLASH_ERR_GENERAL);
			break;
		} else {
			pr_info("Page flash successful!\r\n");
			m_USB->printf("\r\npageFlashDone err=%d\r\n", COMM_SUCCESS);
		}

		offset += MAX_PAGE_SIZE + CHECKBYTES_SIZE;
		currentPage++;
	}

	return COMM_SUCCESS;
}

void SSBootloaderComm::flash_page_data(void)
{
	int totalBytes = 0;
	int currentPage = 1;
	int delay = bl_comm_delay_factor * PAGE_WRITE_DELAY_MS;
	int data_len_flash = 0;

	static uint8_t rx_buf[1] = { 0 };
	static uint8_t page_buf[PAGE_PAYLOAD_SIZE] = { 0 };
	static uint8_t tx_buf[PAGE_PAYLOAD_SIZE + 2] = { 0 };
	static uint8_t packet_header[2] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SENDPAGE };

	while (currentPage <= num_pages) {
		int write_offset = 0;
		int delay = bl_comm_delay_factor * SS_DEFAULT_CMD_SLEEP_MS;
		SS_STATUS status = SS_ERR_UNKNOWN;
		pr_info("Waiting for page %d/%d data (%d bytes)...", currentPage, num_pages, page_size);

		while (totalBytes < (page_size + CHECKBYTES_SIZE)) {
			page_buf[totalBytes++] = m_USB->_getc();
		}

		pr_info("Done\r\n");

		memcpy(tx_buf + 2, page_buf, PAGE_PAYLOAD_SIZE);

		while (write_offset < PAGE_PAYLOAD_SIZE) {
			int write_size = page_transport_size;
			if (write_offset + write_size >= PAGE_PAYLOAD_SIZE) {
				delay = bl_comm_delay_factor * PAGE_WRITE_DELAY_MS;
				write_size = PAGE_PAYLOAD_SIZE - write_offset;
				if(currentPage == num_pages) {
					delay *= 2; // wait for CRC check at last page
				}
			}
			memcpy(tx_buf + write_offset, packet_header, 2);
			status = ss_int->pInterface->write_cmd(tx_buf + write_offset, write_size + 2, delay);
			if (write_offset + write_size < PAGE_PAYLOAD_SIZE && status != SS_BTLDR_PARTIAL_ACK) {
				break;
			}
			write_offset += write_size;
		}

		//Inform PC/Andoid of status
		if (status == SS_ERR_BTLDR_CHECKSUM) {
			pr_info("Verify checksum failed!\r\n");
			m_USB->printf("\r\npageFlashDone err=%d\r\n", BL_FLASH_ERR_CHECKSUM);
			break;
		} else if (status != SS_SUCCESS && status != SS_BTLDR_SUCCESS) {
			pr_info("Page flash failed!\r\n");
			m_USB->printf("\r\npageFlashDone err=%d\r\n", BL_FLASH_ERR_GENERAL);
			break;
		} else {
			currentPage++;
			pr_info("Page flash successful!\r\n");
			m_USB->printf("\r\npageFlashDone err=%d\r\n", COMM_SUCCESS);
		}

		totalBytes = 0;
	}
}
