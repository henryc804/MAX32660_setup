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

#ifndef _SSBOOTLOADERCOMM_H_
#define _SSBOOTLOADERCOMM_H_

#include "mbed.h"
#include "SensorComm.h"
#include "USBSerial.h"
#include "SSInterface.h"

#define SS_BOOTLOADER_ERASE_DELAY	700
#define AES_NONCE_SIZE				11
#define AES_AUTH_SIZE				16
#define MAX_PAGE_SIZE				8192
#define CHECKBYTES_SIZE				16
#define PAGE_PAYLOAD_SIZE			(MAX_PAGE_SIZE + CHECKBYTES_SIZE)

#define VALID_I2C_SLAVE_ADDR_LOW		0x08
#define VALID_I2C_SLAVE_ADDR_HIGH		0x77

#define USN_SIZE					24

#define PAGE_WRITE_DELAY_MS			170
#define MAX_PAGE_NUMBER				31
#define PAGE_ERASE_CMD_SLEEP_MS		50
#define BL_CFG_SAVE_CMD_SLEEP_MS	100

typedef struct {
	int num_allocated_pages; /* Allocated page size */
	int num_pages;
	int num_received_pages;
	int page_size;
	uint8_t auth[AES_AUTH_SIZE];
	uint8_t nonce[AES_NONCE_SIZE];
	uint8_t pages[MAX_PAGE_NUMBER * (MAX_PAGE_SIZE + CHECKBYTES_SIZE)]; //TODO: Use dynamic allocation
} app_image_t;

typedef union {
	struct {
		uint32_t enter_bl_check:1; //MFIO pin check enable
		uint32_t ebl_pin:4;
		uint32_t ebl_polarity:1; /* 0 - active low, 1 active high */
		uint32_t ebl_port:2; //rfu
		uint32_t uart_enable:1;
		uint32_t i2c_enable:1;
		uint32_t spi_enable:1;
		uint32_t i2c_addr:2;
		uint32_t res1:3;	//rfu
		uint32_t ebl_timeout:4; /* Programmable timeout to enter bootloader */
		uint32_t exit_bl_mode:2;   //timeout mode
		uint32_t res2:2;	//rfu
		uint32_t crc_check:1; //CRC check enable
		uint32_t valid_mark_check:1; //todo
		uint32_t lock_swd:1; //rfu
		uint32_t res3:5;	//rfu
	};
	uint32_t cfg;
	uint8_t v[4];
} boot_config_t;

/* Large config is valid for bootloaders with version >= 3.4.2 */
typedef union {
	struct {
		uint32_t enter_bl_check:1; 		// MFIO pin check enable
		uint32_t ebl_pin:4;				// MFIO Pin
		uint32_t ebl_polarity:1; 		// 0 - active low, 1 active high
		uint32_t res0:2;				// Reserved for future usage
		uint32_t uart_enable:1;			// Enable UART
		uint32_t i2c_enable:1;			// Enable I2C
		uint32_t spi_enable:1;			// Enable SPI
		uint32_t res1:5;				// Reserved for future usage
		uint32_t ebl_timeout:4; 		// Programmable timeout to enter bootloader
		uint32_t exit_bl_mode:2;   		// Timeout mode
		uint32_t res2:2;				// Reserved for future usage
		uint32_t crc_check:1; 			// Enable CRC Check
		uint32_t valid_mark_check:1;	// Enable Valid Mark Check
		uint32_t lock_swd:1;			// Lock SWD
		uint32_t res3:5;				// Reserved for future usage
		uint32_t i2c_addr:7;			// I2C Slave Address
		uint32_t res4:25;				// Reserved for future usage
	};
	uint64_t cfg;
	uint8_t v[8];
} boot_large_config_t;

/**
 * @brief	SSBootloaderComm Command handler class for bootloader routines on SmartSensor
 * @details
 */
class SSBootloaderComm:	public SensorComm
{
public:

	/* PUBLIC FUNCTION DECLARATIONS */
    /**
    * @brief    SSBootloaderComm constructor.
	*
	* @param[in]	USB - reference to Virtual serial port over USB
	* @param[in]	ssInterface - name of SmartSensor Interface
	* @param[in]	dsInterface - name of SmartSensor DeviceStudio Interface
    *
    */
	SSBootloaderComm(USBSerial* USB, SSInterface* ssInterface, DSInterface* dsInterface);

    /**
    * @brief    SSBootloaderComm constructor.
	*
	* @param[in]	USB - reference to Virtual serial port over USB
	* @param[in]	ssInterfaceSpi - name of SmartSensor SPI Interface
	* @param[in]	ssInterfaceI2c - name of SmartSensor I2C Interface
	* @param[in]	ssInterfaceUart - name of SmartSensor UART Interface
	* @param[in]	dsInterface - name of SmartSensor DeviceStudio Interface
    *
    */
	SSBootloaderComm(USBSerial* USB, SSInterface* ssInterfaceSpi, SSInterface* ssInterfaceI2c, SSInterface* ssInterfaceUart, DSInterface* dsInterface);

	/**
	* @brief	Parses DeviceStudio-style commands.
	* @details  Parses and executes commands. Prints return code to i/o device.
	* @returns true if sensor acted upon the command, false if command was unknown
	*/
	bool parse_command(const char* cmd);

private:

    /* PRIVATE METHODS */
    int parse_auth(const char* cmd, uint8_t *auth_bytes);
    int parse_iv(const char* cmd, uint8_t* iv_bytes);
    int parse_comm(const char* cmd, uint8_t* comm_index, int command_index);
    void flash_page_data(void);
	int bootldr_set_num_pages(int num_pages);
	int bootldr_get_page_size(int &page_size);
	int bootldr_get_usn(uint8_t* usn);
	int bootldr_set_iv(uint8_t *iv_bytes);
	int bootldr_set_auth(uint8_t *auth_bytes);
	int bootldr_erase_app_memory(void);
	int flash_app_image(void);
	int bootldr_receive_image(void);

	/* PRIVATE TYPE DEFINITIONS */
	typedef enum _cmd_state_t {
		CMD_ENTER_BOOTLDR=0,
		CMD_EXIT_BOOTLDR,
		CMD_RESET,
		CMD_PAGE_SIZE,
		CMD_GET_USN,
		CMD_NUM_PAGES,
		CMD_SET_IV,
		CMD_SET_AUTH,
		CMD_ERASE,
		CMD_PAGE_ERASE,
		CMD_FLASH,
		CMD_FLASH_IMAGE,
		CMD_SETCFG_BL_ENTER_MODE,
		CMD_SETCFG_BL_EBL_PIN,
		CMD_SETCFG_BL_EBL_POL,
		CMD_SETCFG_BL_EXIT_MODE,
		CMD_SETCFG_BL_TIMEOUT,
		CMD_SETCFG_BL_VALID,
		CMD_SETCFG_BL_UART,
		CMD_SETCFG_BL_I2C,
		CMD_SETCFG_BL_SPI,
		CMD_SETCFG_BL_I2C_ADDR,
		CMD_SETCFG_BL_CRC_CHK,
		CMD_SETCFG_BL_LOCK_SWD,
		CMD_SETCFG_BL_SAVE,
		CMD_GETCFG_BL,
		CMD_IMAGE_ON_RAM,
		CMD_SETCFG_HOST_EBL,
		CMD_SETCFG_HOST_CDF,
		CMD_GETCFG_HOST,
		CMD_SETCFG_HOST_COMM,
		CMD_SET_PARTIAL_PAGE_SIZE,
		CMD_SET_I2C_SLAVE_ADDR,
		NUM_CMDS,
	} cmd_state_t;

	typedef enum _comm_index_t {
		COMM_I2C=0,
		COMM_SPI=1,
		COMM_UART=2,
		NUM_COMM,
	} comm_index_t;

	/* PRIVATE VARIABLES */
	USBSerial *m_USB;
	SSInterface *ss_int;
	SSInterface *ss_int_spi;
	SSInterface *ss_int_i2c;
	SSInterface *ss_int_uart;
    DSInterface *ds_int;

    char bootldr_version[128];

    int num_pages;
    int page_size;
    int page_transport_size;
    int bl_comm_delay_factor;

    bool got_page_size;
    bool sent_num_pages;
};

#endif /* _SSBOOTLOADERCOMM_H_ */
