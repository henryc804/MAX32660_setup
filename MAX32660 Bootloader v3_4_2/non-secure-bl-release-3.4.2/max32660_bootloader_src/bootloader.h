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

#ifndef _BOOTLOADER_H__
#define _BOOTLOADER_H__

#define BOOTLOADER_VERSION_MAJOR		3
#define BOOTLOADER_VERSION_MINOR		4
#define BOOTLOADER_VERSION_REVISION		2

#define BOOTLOADER_VERSION "3.4.2"

/* Must start with "Bootloader" */
#if defined(TARGET_STR)
#define BOOTLOADER_PLATFORM "Bootloader_" TARGET_STR
#else
#error "TARGET_STR undefined"
#endif

#define I2C_INTERFACE				0
#define UART_INTERFACE				1
#define SPI_INTERFACE				2

#define SPI							BL_SPI_PORT
#define I2C_SLAVE	    			MXC_I2C1
#define I2C_DEF_SLAVE_ADDR			0x55

#define MARK_VALID_MAGIC_VAL    0x4D41524B //'MARK'

#define FLC_PAGE_BIT_SIZE	13
#define FLC_PAGE_SIZE		(1 << FLC_PAGE_BIT_SIZE)
#define UNINITIALIZED_MEM	0xFFFFFFFF
#define BOOTMODE_SET		0xAAAAAAAA

#define BOOTMEM_SIZE				64

#define CHECKBYTE_16				16
#define USN_SIZE					24
#define PAGE_PAYLOAD_SIZE			(FLC_PAGE_SIZE + CHECKBYTE_16)

#define BL_SUCCESS					0xAA
#define BL_PARTIAL_ACK				0xAB
#define BL_ERR_UNAVAIL_CMD			0x01
#define BL_ERR_UNAVAIL_FUNC			0x02
#define BL_ERR_ERR_DATA_FORMAT		0x03
#define BL_ERR_INPUT_VALUE			0x04
#define BL_ERR_BTLDR_GENERAL		0x80
#define BL_ERR_BTLDR_CHECKSUM		0x81
#define BL_ERR_BTLDR_AUTH			0x82
#define BL_ERR_BTLDR_INVALID_APP	0x83
#define BL_ERR_BTLDR_APP_NOT_ERASED	0x84
#define BL_ERR_TRY_AGAIN			0xFE
#define BL_ERR_UNKNOWN				0xFF

#define BL_OFFSET_STATUS		0
#define BL_STATUS_BYTE_LEN 		1

#define BL_STATUS_BUSY			1

#define COMM_SUCCESS				0

#define MODE_BOOTLDR_MODE_BIT		3
#define MODE_BOOTLDR_RESET_BIT		1
#define MODE_BOOTLDR_SHUTDOWN_BIT	0

#define BL_START_DELAY_MS 20

// command related defines
#define WAIT_I2C_DEFAULT_VALUE_MS 10

#define BL_EXIT_IMMEDIATE 0
#define BL_EXIT_TIMEOUT 1
#define BL_EXIT_INDEFINITE 2

#define CONFIG_EBL_TIMEOUT_UPPER_LIMIT 15
#define CONFIG_EBL_TIMEOUT_LOWER_LIMIT 0

#define CONFIG_EBL_PIN_UPPER_LIMIT 13
#define CONFIG_EBL_PIN_LOWER_LIMIT 0

#define CONFIG_EBL_PORT_UPPER_LIMIT 0
#define CONFIG_EBL_PORT_LOWER_LIMIT 0

#define CONFIG_EBL_CHECK_UPPER_LIMIT 1
#define CONFIG_EBL_CHECK_LOWER_LIMIT 0

#define CONFIG_EBL_POLARITY_UPPER_LIMIT 1
#define CONFIG_EBL_POLARITY_LOWER_LIMIT 0

#define CONFIG_EXIT_BL_UPPER_LIMIT 2
#define CONFIG_EXIT_BL_LOWER_LIMIT 0

#define CONFIG_LOCK_SWD_UPPER_LIMIT 1
#define CONFIG_LOCK_SWD_LOWER_LIMIT 0

#define CONFIG_VALID_MARK_UPPER_LIMIT 1
#define CONFIG_VALID_MARK_LOWER_LIMIT 0

#define CONFIG_INTERFACE_UPPER_LIMIT 1
#define CONFIG_INTERFACE_LOWER_LIMIT 0

#define CONFIG_INTERFACE_UPPER_LIMIT 1
#define CONFIG_INTERFACE_LOWER_LIMIT 0

#define CONFIG_I2C_ADDR_UPPER_LIMIT 0x77
#define CONFIG_I2C_ADDR_LOWER_LIMIT 0x8

#define CONFIG_CRC_CHECK_UPPER_LIMIT 1
#define CONFIG_CRC_CHECK_LOWER_LIMIT 0
// end of command related defines

#define btldr_assert(x) { \
	if (!(x)) \
	while(!(x)); \
}

#define enter_critical_section()		__disable_irq()
#define exit_critical_section()			__enable_irq()


#define DEBUG_LINE()	uart_puts("\r\n# line: "); uart_print_val(__LINE__);uart_puts("\r\n");

#define bool	int
#ifndef true
# define true               (1 == 1)
#endif
#ifndef false
# define false              (!true)
#endif

#ifndef BL_TIME_OUT_VALUE
#define BL_TIME_OUT_VALUE		100 // 1000 ms
#endif

#if !defined(BL_TIME_OUT_TO_STAY_IN_BOOTLDR) && !defined(BL_GPIO_TO_STAY_IN_BOOTLDR)
#warning "Uses GPIO as a default to stay in bootloader"
//#define BL_TIME_OUT_TO_STAY_IN_BOOTLDR
#define BL_GPIO_TO_STAY_IN_BOOTLDR
#endif

#define BL_SUB_CMD_W_DEVICE_MODE	0x00
#define BL_SUB_CMD_R_DEVICE_MODE	0x00

#define BUILD_BUG_ON_ZERO(exp)	(sizeof(struct { int:-!!(exp); }))
#define __same_type(a, b)		__builtin_types_compatible_p(typeof(a), typeof(b))
#define __must_be_array(a)		BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))
#define ARRAY_SIZE(arr)	(sizeof(arr)/sizeof((arr)[0]) + __must_be_array(arr))

#define BL_CMD_LEN	2
typedef enum {
	FAM_BYTE_MODE_WRITE = 0x01,
	FAM_BYTE_MODE_READ = 0x02,
	FAM_BYTE_BOOTLOADER = 0x80,
	BOOTLOADER_INFO = 0x81,
	CONFIG_BYTE_MODE_WRITE = 0x82,
	CONFIG_BYTE_MODE_READ = 0x83,
	DEVICE_INFO = 0xFF,
} fam_byte_t;

typedef enum {
	BL_SUB_CMD_SET_IV = 0x00,
	BL_SUB_CMD_SET_AUTH = 0x01,
	BL_SUB_CMD_SET_NUM_PAGES = 0x02,
	BL_SUB_CMD_ERASE_APP_MEMORY = 0x03,
	BL_SUB_CMD_RECEIVE_PAGE = 0x04,
	BL_SUB_CMD_ERASE_PAGE_MEMORY = 0x05,
	BL_SUB_CMD_SET_PARTIAL_PAGE_SIZE = 0x06
} bootldr_flash_enum_t;

typedef enum {
	BL_SUB_CMD_GET_VERSION = 0x00,
	BL_SUB_CMD_GET_PAGE_SIZE = 0x01,
	BL_SUB_CMD_GET_USN = 0x02,
} bootldr_info_enum_t;

typedef enum {
	BL_SUB_CMD_SAVE_SETTINGS = 0x00,
	BL_SUB_CMD_ENTRY_CONFIG = 0x01,
	BL_SUB_CMD_EXIT_CONFIG = 0x02,
} bootldr_wr_config_enum_t;

typedef enum {
	BL_CFG_EBL_CHECK = 0x00,
	BL_CFG_EBL_PIN = 0x01,
	BL_CFG_EBL_POL = 0x02,
	BL_CFG_VALID_MARK_CHK = 0x03,
	BL_CFG_UART_ENABLE = 0x04,
	BL_CFG_I2C_ENABLE = 0x05,
	BL_CFG_SPI_ENABLE = 0x06,
	BL_CFG_I2C_SLAVE_ADDR = 0x07,
	BL_CFG_CRC_CHECK = 0x08,
	BL_CFG_LOCK_SWD = 0x09,
} bootldr_bl_entry_config_enum_t;

typedef enum {
	BL_SUB_CMD_RD_ENTRY_CONFIG = 0x01,
	BL_SUB_CMD_RD_EXIT_CONFIG = 0x02,
	BL_CMDIDX_R_ALL_CFG = 0xFF,
} bootldr_rd_config_enum_t;

typedef enum {
	BL_SUB_CMD_PLATFORM_TYPE = 0x00,
} bootldr_rd_device_config_enum_t;

typedef enum {
	BTLDR_IDLE,
	BTLDR_FLASHING,
	BTLDR_MODE_EXIT,
	BTLDR_SHUTDOWN,
	BTLDR_RESTART,
	BTLDR_ERASE_APP_MEMORY,
	BTLDR_ERASE_PAGE_MEMORY,
	BTLDR_UPDATE_BOOT_CONFIG,
} btldr_mode_t;

typedef enum {
	BTLDR_NO_ERR = 0,
	BTLDR_CHECKSUM_FAIL,
} btldr_err_t;

typedef struct {
	uint32_t CRC32;
	uint32_t length;
	uint32_t valid_mark;
	uint32_t boot_mode;
	/*
	Please note that when app_header_t is modified,
	bootloader linker files also need to be modifed.
	Becase Linker exposes boot_mode varible in the flash memory.
	*/
} app_header_t;

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
} boot_config_t;

/* boot mem header is 64 bytes */
typedef struct {
	app_header_t header;
	boot_config_t boot_cfg;
	uint32_t cfg_crc;
	uint8_t rfu[4];
	uint8_t usn[32];
} boot_mem_t;

_Static_assert(sizeof(boot_mem_t) == BOOTMEM_SIZE, "Boot memory is not equal to BOOTMEM_SIZE definition!");

/********************************************
rx_buf: received bytes
rx_len: Number of received bytes
tx_buf: Buffer to fill up for response
tx_len: Initial value is tx_buf's maximum size,
		it must be set to number of bytes to transfer.
return: error message. 0: success, non-zero is an error code
*********************************************/
typedef void (*fp_callback_t)(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);

typedef struct {
	uint8_t sub_cmd;
	fp_callback_t cb;
	int expected_num_bytes;
} sub_cmd_t;

typedef struct {
	const uint8_t family_byte;
	const sub_cmd_t *cmd;
	const uint8_t len;
} cmd_table_t;

typedef struct {
	fp_callback_t cb;
	uint8_t *msg;
	uint16_t max_len;
	uint16_t payload_len;
	bool fam_byte_received;
} bl_comm_msg_t;

typedef	union {
	struct {
		uint8_t err:3;
		uint8_t : 3;
		uint8_t busy: 1;
	};
	uint8_t val;
} status_t;

int mxm_bl_find_fam_cmd(uint8_t *packet);
int uart_main(void);
boot_config_t *get_pointer_to_bootconfig(void);
int set_boot_mode_and_reset(void);
void mxm_bl_spi_slave_write_cb_init(int nbytes);
int mxm_bl_spi_init(void);
void mxm_bl_spi_shutdown(void);
int mxm_bl_uart_init(void);
void mxm_bl_uart_shutdown(void);
int mxm_bl_uart_write(uint8_t* data, int len);
int mxm_bl_i2c_init(void);
void mxm_bl_i2c_shutdown(void);
uint8_t * mxm_get_rx_buf_pointer(void);
uint8_t * mxm_get_tx_buf_pointer(void);
void mxm_set_interface(unsigned char interface_type);

void mxm_bootloader_write_mode(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_read_mode(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_wr_bootloader_cmd_set_iv(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_wr_bootloader_cmd_set_auth(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_wr_bootloader_cmd_set_num_pages(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_wr_bootloader_cmd_erase_app_memory(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_wr_bootloader_cmd_erase_page_memory(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);

/**
 * @brief	Set partial page packet size
 *
 * @param[in] rx_buf communication receive buffer
 * @param[in] rx_len communication received data length
 * @param[in] tx_buf communication transfer buffer
 * @param[in] tx_len communication transfer data length
 *
 * @details	Sets partial page packet size and sends response.
 *
 */
void mxm_wr_bootloader_cmd_set_pl_page_size(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_wr_bootloader_cmd_receive_page(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_get_fw_version(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_get_page_size(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_get_usn(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_save_config(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_config_entry(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_config_exit(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_get_config_entry(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_get_config_exit(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_get_config_bl(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);
void mxm_bootloader_platform_type(const uint8_t *rx_buf, int rx_len, uint8_t *tx_buf, int *tx_len);

extern void *_app_isr[];
extern int _app_start;
extern int _app_end;
extern int _boot_mem_start;
extern int _boot_mode;
extern int _boot_mem_end;
extern int _boot_mem_len;
extern bl_comm_msg_t bl_msg;
#endif
