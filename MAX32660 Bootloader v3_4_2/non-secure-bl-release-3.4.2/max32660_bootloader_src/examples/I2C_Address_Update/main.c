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
 * @brief   I2C SLave Address Changer!
 * @details This example changes Bootloader I2C Slave address to value that defined as I2C_ADDR
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "mxc_delay.h"
#include "flc.h"
#include "led.h"
#include "tmr.h"
#include "tmr_utils.h"

/***** Definitions *****/

#define PAGE_SIZE 	8192
#define I2C_ADDR 	0x70 /* 7 bit I2C value, valid between 0x08 - 0x77 */
#define I2C_ADDR_UPPER_LIMIT 0x77
#define I2C_ADDR_LOWER_LIMIT 0x8

typedef struct {
	uint32_t CRC32;
	uint32_t length;
	uint32_t valid_mark;
	uint32_t boot_mode;
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

typedef struct {
	app_header_t header;
	boot_config_t boot_cfg;
	uint32_t cfg_crc;
	uint8_t rfu[4];
	uint8_t usn[32];
} boot_mem_t;

/***** Globals *****/

static const uint32_t crc32_table[256] =
{
  0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU,
  0x076DC419U, 0x706AF48FU, 0xE963A535U, 0x9E6495A3U,
  0x0EDB8832U, 0x79DCB8A4U, 0xE0D5E91EU, 0x97D2D988U,
  0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U, 0x90BF1D91U,
  0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU,
  0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U,
  0x136C9856U, 0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU,
  0x14015C4FU, 0x63066CD9U, 0xFA0F3D63U, 0x8D080DF5U,
  0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U, 0xA2677172U,
  0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,
  0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U,
  0x32D86CE3U, 0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U,
  0x26D930ACU, 0x51DE003AU, 0xC8D75180U, 0xBFD06116U,
  0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U, 0xB8BDA50FU,
  0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U,
  0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU,
  0x76DC4190U, 0x01DB7106U, 0x98D220BCU, 0xEFD5102AU,
  0x71B18589U, 0x06B6B51FU, 0x9FBFE4A5U, 0xE8B8D433U,
  0x7807C9A2U, 0x0F00F934U, 0x9609A88EU, 0xE10E9818U,
  0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,
  0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU,
  0x6C0695EDU, 0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U,
  0x65B0D9C6U, 0x12B7E950U, 0x8BBEB8EAU, 0xFCB9887CU,
  0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U, 0xFBD44C65U,
  0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U,
  0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU,
  0x4369E96AU, 0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U,
  0x44042D73U, 0x33031DE5U, 0xAA0A4C5FU, 0xDD0D7CC9U,
  0x5005713CU, 0x270241AAU, 0xBE0B1010U, 0xC90C2086U,
  0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,
  0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U,
  0x59B33D17U, 0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU,
  0xEDB88320U, 0x9ABFB3B6U, 0x03B6E20CU, 0x74B1D29AU,
  0xEAD54739U, 0x9DD277AFU, 0x04DB2615U, 0x73DC1683U,
  0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U,
  0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U,
  0xF00F9344U, 0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU,
  0xF762575DU, 0x806567CBU, 0x196C3671U, 0x6E6B06E7U,
  0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU, 0x67DD4ACCU,
  0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,
  0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U,
  0xD1BB67F1U, 0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU,
  0xD80D2BDAU, 0xAF0A1B4CU, 0x36034AF6U, 0x41047A60U,
  0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU, 0x4669BE79U,
  0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U,
  0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU,
  0xC5BA3BBEU, 0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U,
  0xC2D7FFA7U, 0xB5D0CF31U, 0x2CD99E8BU, 0x5BDEAE1DU,
  0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU, 0x026D930AU,
  0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,
  0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U,
  0x92D28E9BU, 0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U,
  0x86D3D2D4U, 0xF1D4E242U, 0x68DDB3F8U, 0x1FDA836EU,
  0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U, 0x18B74777U,
  0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU,
  0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U,
  0xA00AE278U, 0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U,
  0xA7672661U, 0xD06016F7U, 0x4969474DU, 0x3E6E77DBU,
  0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U, 0x37D83BF0U,
  0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,
  0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U,
  0xBAD03605U, 0xCDD70693U, 0x54DE5729U, 0x23D967BFU,
  0xB3667A2EU, 0xC4614AB8U, 0x5D681B02U, 0x2A6F2B94U,
  0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU, 0x2D02EF8DU
};


extern int _boot_mem_start;

/***** Functions *****/

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

uint32_t calc_crc32(const uint8_t *pBuf, int size)
{
	uint32_t crc = 0xFFFFFFFFU;

	while (size > 0)
	{
		crc = crc32_table[*pBuf ^ (uint8_t)crc] ^ (crc >> 8);
		pBuf++;
		size--;
	}

	crc = crc ^ 0xFFFFFFFFU;

	return crc;
}

void set_i2c_address(int new_addr)
{
	uint8_t page_data[PAGE_SIZE];
	boot_mem_t new_boot_memory;
	unsigned long conf_page_loc = ((unsigned long)&_boot_mem_start) & ~(PAGE_SIZE - 1);

	if (new_addr < I2C_ADDR_LOWER_LIMIT || new_addr > I2C_ADDR_UPPER_LIMIT) {
		printf("Invalid I2C Address\n");
		return;
	}

	memcpy(&new_boot_memory, ((unsigned long)&_boot_mem_start), sizeof(boot_mem_t));
	memcpy(page_data, (uint8_t *)conf_page_loc, PAGE_SIZE);

	new_boot_memory.boot_cfg.i2c_addr = new_addr;
	new_boot_memory.cfg_crc = calc_crc32(&new_boot_memory.boot_cfg, sizeof(boot_config_t));
	memcpy(page_data + ((unsigned long)&_boot_mem_start - conf_page_loc), &new_boot_memory, sizeof(boot_mem_t));

	const sys_cfg_t sys_flash_cfg = NULL;
	int ret = FLC_Init(&sys_flash_cfg);
	if (ret) {
		printf("Could not init flash\n");
		FLC_ClearFlags(MXC_F_FLC_INTR_DONE | MXC_F_FLC_INTR_AF);
		return;
	}

	ret = FLC_PageErase(conf_page_loc);
	if (ret) {
		printf("Could not erase fpage\n");
		FLC_ClearFlags(MXC_F_FLC_INTR_DONE | MXC_F_FLC_INTR_AF);
		return;
	}

	ret = flash_program_page(conf_page_loc, PAGE_SIZE, page_data);
	if (ret) {
		printf("Could not program flash\n");
		FLC_ClearFlags(MXC_F_FLC_INTR_DONE | MXC_F_FLC_INTR_AF);
		return;
	}

	printf("Updated I2C Address Successfully!\n\n");
	FLC_ClearFlags(MXC_F_FLC_INTR_DONE | MXC_F_FLC_INTR_AF);
}

//******************************************************************************

const gpio_cfg_t gpio_0_9 = {PORT_0, PIN_9, GPIO_FUNC_OUT, GPIO_PAD_NONE};

int main(void)
{
    int count = 0;

    printf("Bootloader - I2C Address Changer!\n");

    set_i2c_address(I2C_ADDR);

	GPIO_Config(&gpio_0_9);
    while(1) {
        LED_On(0);
		GPIO_OutClr(&gpio_0_9);

        /* Demonstrates the TMR driver delay */
        TMR_Delay(MXC_TMR0, MSEC(500), NULL);
        LED_Off(0);
		GPIO_OutSet(&gpio_0_9);

        /* Demonstrates the SysTick-based convenience delay functions */
	    mxc_delay(MXC_DELAY_MSEC(500));
        printf("count = %d\n", count++);

    }
}
