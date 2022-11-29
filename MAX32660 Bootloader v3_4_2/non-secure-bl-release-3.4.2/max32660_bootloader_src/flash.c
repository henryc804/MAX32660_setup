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
#include "flash.h"
#ifdef MAX32620
#include "max32620.h"
#elif MAX32625
#include "max32625.h"
#elif MAX3263x
#include "max3263x.h"
#elif MAX32660
#include "max32660.h"
#endif


/*
APIs

*/
int flash_verify(uint32_t address, uint32_t length, uint8_t *data)
{
  volatile uint8_t *ptr;

  for (ptr = (uint8_t *)address; ptr < (uint8_t *)(address + length); ptr++, data++) {
    if (*ptr != *data) {
//        printf("Verify failed at 0x%x (0x%x != 0x%x)\n", (unsigned int)ptr, (unsigned int)*ptr, (unsigned int)*data);
        return E_UNKNOWN;
    }
  }

  return E_NO_ERROR;
}

/******************************************************************************/
int flash_uninit(void)
{
	FLC_ClearFlags(MXC_F_FLC_INTR_DONE | MXC_F_FLC_INTR_AF);
    return  0;  // Finished without Errors
}

/******************************************************************************/
/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */
int flash_erase_chip(void)
{
	return FLC_MassErase();
}

/******************************************************************************/
/*
 *  Erase given page address in Flash Memory
 *    Parameter:      address:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */
int flash_erase_page(unsigned long address)
{
	return FLC_PageErase(address);
}

int flash_program_page(unsigned long address, unsigned long size, unsigned char *buffer8)
{
	uint32_t dest_addr;
	int i = 0;
	uint32_t *buffer32 = (uint32_t *)buffer8;

    for (dest_addr = address; dest_addr < (address + size); dest_addr += 4) {
        // Write a word
        if (FLC_Write32(dest_addr, buffer32[i]) != E_NO_ERROR) {
          //  printf("Failure in writing a word.\n");
            break;
        }

        i++;
    }

	// Verify if page is written properly
	if (flash_verify(address, size, (uint8_t*)buffer8) != E_NO_ERROR) {
		//printf("%ld bytes are not written properly.\n", size);
		return -1;
	}

//	printf("* %d data succefully written to address %X, and verified.\n", size, address);
	return 0;
}
