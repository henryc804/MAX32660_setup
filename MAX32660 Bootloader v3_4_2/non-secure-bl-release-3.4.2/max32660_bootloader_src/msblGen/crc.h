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


#ifndef _CRC_H
#define _CRC_H

#include <stdint.h>

#define CHECK_BITSIZE	32
#define CHECK_BYTESIZE	(CHECK_BITSIZE / 8)

//CRC-32 standard ethernet polynomial (good for msgs with lots of continuous 0s)
#define CRC_POLYNOMIAL 0x04C11DB7

#define FIRST_CRC_BLOCK	1
#define LAST_CRC_BLOCK	2

/*
   http://www.ross.net/crc/download/crc_v3.txt is a great resource for understanding CRC theory and implementation
   Name   : "CRC-32"
   Width  : 32
   Poly   : 04C11DB7
   Init   : FFFFFFFF
   RefIn  : True
   RefOut : True
   XorOut : FFFFFFFF
   Check  : CBF43926
*/

//Suggested by Koopman et al (E&C Engineering, Carnegie Mellon):
//#define CRC_POLYNOMIAL 0xBA0DC66B;

//Returns 32-bit checksum
uint32_t crcCalc(uint8_t *msg, uint32_t size);


//Processes multiple blocks of data sequentially
//Set block = FIRST_CRC_BLOCK for the first block of data
//Set block = LAST_CRC_BLOCK for the last block of data
//Set block = 0 for all other blocks of data
//The same pointer to result is passed each time, and the value is the result
void crcMultiblockCalc(uint8_t *msg, uint32_t size,  uint32_t *result, uint32_t block);


//Return 0 if msg checksum is verified,
//Return -1 if mismatch
uint8_t crcVerifyMsg(uint8_t *msg, uint32_t size);

#endif
