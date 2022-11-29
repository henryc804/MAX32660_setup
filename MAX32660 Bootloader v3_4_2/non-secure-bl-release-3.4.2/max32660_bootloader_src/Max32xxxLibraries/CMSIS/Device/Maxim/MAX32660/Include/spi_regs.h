/**
 * @file    spi_regs.h
 * @brief   Registers, Bit Masks and Bit Positions for the SPI Peripheral Module.
 */

/* ****************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
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
 *
 * $Date: 2017-10-09 11:34:29 -0500 (Mon, 09 Oct 2017) $
 * $Revision: 31254 $
 *
 *************************************************************************** */

#ifndef _SPI_REGS_H_
#define _SPI_REGS_H_

/* **** Includes **** */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @cond
/*
    If types are not defined elsewhere (CMSIS) define them here
*/
#ifndef __IO
#define __IO volatile
#endif
#ifndef __I
#define __I  volatile const
#endif
#ifndef __O
#define __O  volatile
#endif
#ifndef __R
#define __R  volatile const
#endif
/// @endcond

/* **** Definitions **** */

/**
 * @ingroup     spi
 * @defgroup    spi_registers Registers
 * @brief       Registers, Bit Masks and Bit Positions for the SPI Peripheral Module.
 * @description SPI peripheral.
 */

/**
 * @ingroup spi_registers
 * Structure type to access the SPI Registers.
 */
typedef struct {
  union{
    __IO uint8_t  data8[4];             /**< <tt>\b 0x00:<\tt> SPI DATA8 Register */
    __IO uint16_t data16[2];            /**< <tt>\b 0x00:<\tt> SPI DATA16 Register */
    __IO uint32_t data32;               /**< <tt>\b 0x00:<\tt> SPI DATA32 Register */
  };
    __IO uint32_t ctrl0;                /**< <tt>\b 0x04:<\tt> SPI CTRL0 Register */
    __IO uint32_t ctrl1;                /**< <tt>\b 0x08:<\tt> SPI CTRL1 Register */
    __IO uint32_t ctrl2;                /**< <tt>\b 0x0C:<\tt> SPI CTRL2 Register */
    __IO uint32_t ss_time;              /**< <tt>\b 0x10:<\tt> SPI SS_TIME Register */
    __IO uint32_t clk_cfg;              /**< <tt>\b 0x14:<\tt> SPI CLK_CFG Register */
    __R  uint32_t rsv0;
    __IO uint32_t dma;                  /**< <tt>\b 0x1C:<\tt> SPI DMA Register */
    __IO uint32_t int_fl;               /**< <tt>\b 0x20:<\tt> SPI INT_FL Register */
    __IO uint32_t int_en;               /**< <tt>\b 0x24:<\tt> SPI INT_EN Register */
    __IO uint32_t wake_fl;              /**< <tt>\b 0x28:<\tt> SPI WAKE_FL Register */
    __IO uint32_t wake_en;              /**< <tt>\b 0x2C:<\tt> SPI WAKE_EN Register */
    __I  uint32_t stat;                 /**< <tt>\b 0x30:<\tt> SPI STAT Register */
} mxc_spi_regs_t;

/* Register offsets for module SPI */
/**
 * @ingroup    spi_registers
 * @defgroup   SPI_Register_Offsets Register Offsets
 * @brief      SPI Peripheral Register Offsets from the SPI Base Peripheral Address. 
 * @{
 */
 #define MXC_R_SPI_DATA8                    ((uint32_t)0x00000000UL) /**< Offset from SPI Base Address: <tt> 0x0x000 */ 
 #define MXC_R_SPI_DATA16                   ((uint32_t)0x00000000UL) /**< Offset from SPI Base Address: <tt> 0x0x000 */ 
 #define MXC_R_SPI_DATA32                   ((uint32_t)0x00000000UL) /**< Offset from SPI Base Address: <tt> 0x0x000 */ 
 #define MXC_R_SPI_CTRL0                    ((uint32_t)0x00000004UL) /**< Offset from SPI Base Address: <tt> 0x0x004 */ 
 #define MXC_R_SPI_CTRL1                    ((uint32_t)0x00000008UL) /**< Offset from SPI Base Address: <tt> 0x0x008 */ 
 #define MXC_R_SPI_CTRL2                    ((uint32_t)0x0000000CUL) /**< Offset from SPI Base Address: <tt> 0x0x00C */ 
 #define MXC_R_SPI_SS_TIME                  ((uint32_t)0x00000010UL) /**< Offset from SPI Base Address: <tt> 0x0x010 */ 
 #define MXC_R_SPI_CLK_CFG                  ((uint32_t)0x00000014UL) /**< Offset from SPI Base Address: <tt> 0x0x014 */ 
 #define MXC_R_SPI_DMA                      ((uint32_t)0x0000001CUL) /**< Offset from SPI Base Address: <tt> 0x0x01C */ 
 #define MXC_R_SPI_INT_FL                   ((uint32_t)0x00000020UL) /**< Offset from SPI Base Address: <tt> 0x0x020 */ 
 #define MXC_R_SPI_INT_EN                   ((uint32_t)0x00000024UL) /**< Offset from SPI Base Address: <tt> 0x0x024 */ 
 #define MXC_R_SPI_WAKE_FL                  ((uint32_t)0x00000028UL) /**< Offset from SPI Base Address: <tt> 0x0x028 */ 
 #define MXC_R_SPI_WAKE_EN                  ((uint32_t)0x0000002CUL) /**< Offset from SPI Base Address: <tt> 0x0x02C */ 
 #define MXC_R_SPI_STAT                     ((uint32_t)0x00000030UL) /**< Offset from SPI Base Address: <tt> 0x0x030 */ 
/**@} end of group spi_registers */

/**
 * @ingroup  spi_registers
 * @defgroup DATA8_Register
 * @brief    Register for reading and writing the FIFO.
 * @{
 */
 #define MXC_F_SPI_DATA8_DATA_POS                       (0) /**< DATA8_DATA Position */
 #define MXC_F_SPI_DATA8_DATA                           ((uint8_t)(0xFF << MXC_F_SPI_DATA8_DATA_POS)) /**< DATA8_DATA Mask */

/**@} end of group DATA8_Register */

/**
 * @ingroup  spi_registers
 * @defgroup DATA16_Register
 * @brief    Register for reading and writing the FIFO.
 * @{
 */
 #define MXC_F_SPI_DATA16_DATA_POS                      (0) /**< DATA16_DATA Position */
 #define MXC_F_SPI_DATA16_DATA                          ((uint16_t)(0xFFFF << MXC_F_SPI_DATA16_DATA_POS)) /**< DATA16_DATA Mask */

/**@} end of group DATA16_Register */

/**
 * @ingroup  spi_registers
 * @defgroup DATA32_Register
 * @brief    Register for reading and writing the FIFO.
 * @{
 */
 #define MXC_F_SPI_DATA32_DATA_POS                      (0) /**< DATA32_DATA Position */
 #define MXC_F_SPI_DATA32_DATA                          ((uint32_t)(0xFFFFFFFF << MXC_F_SPI_DATA32_DATA_POS)) /**< DATA32_DATA Mask */

/**@} end of group DATA32_Register */

/**
 * @ingroup  spi_registers
 * @defgroup CTRL0_Register
 * @brief    Register for controlling SPI peripheral.
 * @{
 */
 #define MXC_F_SPI_CTRL0_EN_POS                         (0) /**< CTRL0_EN Position */
 #define MXC_F_SPI_CTRL0_EN                             ((uint32_t)(0x1 << MXC_F_SPI_CTRL0_EN_POS)) /**< CTRL0_EN Mask */
 #define MXC_V_SPI_CTRL0_EN_DIS                         ((uint32_t)0x0) /**< CTRL0_EN_DIS Value */
 #define MXC_S_SPI_CTRL0_EN_DIS                         (MXC_V_SPI_CTRL0_EN_DIS << MXC_F_SPI_CTRL0_EN_POS) /**< CTRL0_EN_DIS Setting */
 #define MXC_V_SPI_CTRL0_EN_EN                          ((uint32_t)0x1) /**< CTRL0_EN_EN Value */
 #define MXC_S_SPI_CTRL0_EN_EN                          (MXC_V_SPI_CTRL0_EN_EN << MXC_F_SPI_CTRL0_EN_POS) /**< CTRL0_EN_EN Setting */

 #define MXC_F_SPI_CTRL0_MASTER_POS                     (1) /**< CTRL0_MASTER Position */
 #define MXC_F_SPI_CTRL0_MASTER                         ((uint32_t)(0x1 << MXC_F_SPI_CTRL0_MASTER_POS)) /**< CTRL0_MASTER Mask */
 #define MXC_V_SPI_CTRL0_MASTER_DIS                     ((uint32_t)0x0) /**< CTRL0_MASTER_DIS Value */
 #define MXC_S_SPI_CTRL0_MASTER_DIS                     (MXC_V_SPI_CTRL0_MASTER_DIS << MXC_F_SPI_CTRL0_MASTER_POS) /**< CTRL0_MASTER_DIS Setting */
 #define MXC_V_SPI_CTRL0_MASTER_EN                      ((uint32_t)0x1) /**< CTRL0_MASTER_EN Value */
 #define MXC_S_SPI_CTRL0_MASTER_EN                      (MXC_V_SPI_CTRL0_MASTER_EN << MXC_F_SPI_CTRL0_MASTER_POS) /**< CTRL0_MASTER_EN Setting */

 #define MXC_F_SPI_CTRL0_SS_IO_POS                      (4) /**< CTRL0_SS_IO Position */
 #define MXC_F_SPI_CTRL0_SS_IO                          ((uint32_t)(0x1 << MXC_F_SPI_CTRL0_SS_IO_POS)) /**< CTRL0_SS_IO Mask */
 #define MXC_V_SPI_CTRL0_SS_IO_OUTPUT                   ((uint32_t)0x0) /**< CTRL0_SS_IO_OUTPUT Value */
 #define MXC_S_SPI_CTRL0_SS_IO_OUTPUT                   (MXC_V_SPI_CTRL0_SS_IO_OUTPUT << MXC_F_SPI_CTRL0_SS_IO_POS) /**< CTRL0_SS_IO_OUTPUT Setting */
 #define MXC_V_SPI_CTRL0_SS_IO_INPUT                    ((uint32_t)0x1) /**< CTRL0_SS_IO_INPUT Value */
 #define MXC_S_SPI_CTRL0_SS_IO_INPUT                    (MXC_V_SPI_CTRL0_SS_IO_INPUT << MXC_F_SPI_CTRL0_SS_IO_POS) /**< CTRL0_SS_IO_INPUT Setting */

 #define MXC_F_SPI_CTRL0_START_POS                      (5) /**< CTRL0_START Position */
 #define MXC_F_SPI_CTRL0_START                          ((uint32_t)(0x1 << MXC_F_SPI_CTRL0_START_POS)) /**< CTRL0_START Mask */
 #define MXC_V_SPI_CTRL0_START_START                    ((uint32_t)0x1) /**< CTRL0_START_START Value */
 #define MXC_S_SPI_CTRL0_START_START                    (MXC_V_SPI_CTRL0_START_START << MXC_F_SPI_CTRL0_START_POS) /**< CTRL0_START_START Setting */

 #define MXC_F_SPI_CTRL0_SS_CTRL_POS                    (8) /**< CTRL0_SS_CTRL Position */
 #define MXC_F_SPI_CTRL0_SS_CTRL                        ((uint32_t)(0x1 << MXC_F_SPI_CTRL0_SS_CTRL_POS)) /**< CTRL0_SS_CTRL Mask */
 #define MXC_V_SPI_CTRL0_SS_CTRL_DEASSERT               ((uint32_t)0x0) /**< CTRL0_SS_CTRL_DEASSERT Value */
 #define MXC_S_SPI_CTRL0_SS_CTRL_DEASSERT               (MXC_V_SPI_CTRL0_SS_CTRL_DEASSERT << MXC_F_SPI_CTRL0_SS_CTRL_POS) /**< CTRL0_SS_CTRL_DEASSERT Setting */
 #define MXC_V_SPI_CTRL0_SS_CTRL_ASSERT                 ((uint32_t)0x1) /**< CTRL0_SS_CTRL_ASSERT Value */
 #define MXC_S_SPI_CTRL0_SS_CTRL_ASSERT                 (MXC_V_SPI_CTRL0_SS_CTRL_ASSERT << MXC_F_SPI_CTRL0_SS_CTRL_POS) /**< CTRL0_SS_CTRL_ASSERT Setting */

 #define MXC_F_SPI_CTRL0_SS_POS                         (16) /**< CTRL0_SS Position */
 #define MXC_F_SPI_CTRL0_SS                             ((uint32_t)(0xF << MXC_F_SPI_CTRL0_SS_POS)) /**< CTRL0_SS Mask */
 #define MXC_V_SPI_CTRL0_SS_SS0                         ((uint32_t)0x1) /**< CTRL0_SS_SS0 Value */
 #define MXC_S_SPI_CTRL0_SS_SS0                         (MXC_V_SPI_CTRL0_SS_SS0 << MXC_F_SPI_CTRL0_SS_POS) /**< CTRL0_SS_SS0 Setting */
 #define MXC_V_SPI_CTRL0_SS_SS1                         ((uint32_t)0x2) /**< CTRL0_SS_SS1 Value */
 #define MXC_S_SPI_CTRL0_SS_SS1                         (MXC_V_SPI_CTRL0_SS_SS1 << MXC_F_SPI_CTRL0_SS_POS) /**< CTRL0_SS_SS1 Setting */
 #define MXC_V_SPI_CTRL0_SS_SS2                         ((uint32_t)0x4) /**< CTRL0_SS_SS2 Value */
 #define MXC_S_SPI_CTRL0_SS_SS2                         (MXC_V_SPI_CTRL0_SS_SS2 << MXC_F_SPI_CTRL0_SS_POS) /**< CTRL0_SS_SS2 Setting */
 #define MXC_V_SPI_CTRL0_SS_SS3                         ((uint32_t)0x8) /**< CTRL0_SS_SS3 Value */
 #define MXC_S_SPI_CTRL0_SS_SS3                         (MXC_V_SPI_CTRL0_SS_SS3 << MXC_F_SPI_CTRL0_SS_POS) /**< CTRL0_SS_SS3 Setting */

/**@} end of group CTRL0_Register */

/**
 * @ingroup  spi_registers
 * @defgroup CTRL1_Register
 * @brief    Register for controlling SPI peripheral.
 * @{
 */
 #define MXC_F_SPI_CTRL1_TX_NUM_CHAR_POS                (0) /**< CTRL1_TX_NUM_CHAR Position */
 #define MXC_F_SPI_CTRL1_TX_NUM_CHAR                    ((uint32_t)(0xFFFF << MXC_F_SPI_CTRL1_TX_NUM_CHAR_POS)) /**< CTRL1_TX_NUM_CHAR Mask */

 #define MXC_F_SPI_CTRL1_RX_NUM_CHAR_POS                (16) /**< CTRL1_RX_NUM_CHAR Position */
 #define MXC_F_SPI_CTRL1_RX_NUM_CHAR                    ((uint32_t)(0xFFFF << MXC_F_SPI_CTRL1_RX_NUM_CHAR_POS)) /**< CTRL1_RX_NUM_CHAR Mask */

/**@} end of group CTRL1_Register */

/**
 * @ingroup  spi_registers
 * @defgroup CTRL2_Register
 * @brief    Register for controlling SPI peripheral.
 * @{
 */
 #define MXC_F_SPI_CTRL2_CPHA_POS                       (0) /**< CTRL2_CPHA Position */
 #define MXC_F_SPI_CTRL2_CPHA                           ((uint32_t)(0x1 << MXC_F_SPI_CTRL2_CPHA_POS)) /**< CTRL2_CPHA Mask */
 #define MXC_V_SPI_CTRL2_CPHA_RISING_EDGE               ((uint32_t)0x0) /**< CTRL2_CPHA_RISING_EDGE Value */
 #define MXC_S_SPI_CTRL2_CPHA_RISING_EDGE               (MXC_V_SPI_CTRL2_CPHA_RISING_EDGE << MXC_F_SPI_CTRL2_CPHA_POS) /**< CTRL2_CPHA_RISING_EDGE Setting */
 #define MXC_V_SPI_CTRL2_CPHA_FALLING_EDGE              ((uint32_t)0x1) /**< CTRL2_CPHA_FALLING_EDGE Value */
 #define MXC_S_SPI_CTRL2_CPHA_FALLING_EDGE              (MXC_V_SPI_CTRL2_CPHA_FALLING_EDGE << MXC_F_SPI_CTRL2_CPHA_POS) /**< CTRL2_CPHA_FALLING_EDGE Setting */

 #define MXC_F_SPI_CTRL2_CPOL_POS                       (1) /**< CTRL2_CPOL Position */
 #define MXC_F_SPI_CTRL2_CPOL                           ((uint32_t)(0x1 << MXC_F_SPI_CTRL2_CPOL_POS)) /**< CTRL2_CPOL Mask */
 #define MXC_V_SPI_CTRL2_CPOL_NORMAL                    ((uint32_t)0x0) /**< CTRL2_CPOL_NORMAL Value */
 #define MXC_S_SPI_CTRL2_CPOL_NORMAL                    (MXC_V_SPI_CTRL2_CPOL_NORMAL << MXC_F_SPI_CTRL2_CPOL_POS) /**< CTRL2_CPOL_NORMAL Setting */
 #define MXC_V_SPI_CTRL2_CPOL_INVERTED                  ((uint32_t)0x1) /**< CTRL2_CPOL_INVERTED Value */
 #define MXC_S_SPI_CTRL2_CPOL_INVERTED                  (MXC_V_SPI_CTRL2_CPOL_INVERTED << MXC_F_SPI_CTRL2_CPOL_POS) /**< CTRL2_CPOL_INVERTED Setting */

 #define MXC_F_SPI_CTRL2_SCLK_INV_POS                   (4) /**< CTRL2_SCLK_INV Position */
 #define MXC_F_SPI_CTRL2_SCLK_INV                       ((uint32_t)(0x1 << MXC_F_SPI_CTRL2_SCLK_INV_POS)) /**< CTRL2_SCLK_INV Mask */

 #define MXC_F_SPI_CTRL2_NUMBITS_POS                    (8) /**< CTRL2_NUMBITS Position */
 #define MXC_F_SPI_CTRL2_NUMBITS                        ((uint32_t)(0xF << MXC_F_SPI_CTRL2_NUMBITS_POS)) /**< CTRL2_NUMBITS Mask */
 #define MXC_V_SPI_CTRL2_NUMBITS_0                      ((uint32_t)0x0) /**< CTRL2_NUMBITS_0 Value */
 #define MXC_S_SPI_CTRL2_NUMBITS_0                      (MXC_V_SPI_CTRL2_NUMBITS_0 << MXC_F_SPI_CTRL2_NUMBITS_POS) /**< CTRL2_NUMBITS_0 Setting */

 #define MXC_F_SPI_CTRL2_DATA_WIDTH_POS                 (12) /**< CTRL2_DATA_WIDTH Position */
 #define MXC_F_SPI_CTRL2_DATA_WIDTH                     ((uint32_t)(0x3 << MXC_F_SPI_CTRL2_DATA_WIDTH_POS)) /**< CTRL2_DATA_WIDTH Mask */
 #define MXC_V_SPI_CTRL2_DATA_WIDTH_MONO                ((uint32_t)0x0) /**< CTRL2_DATA_WIDTH_MONO Value */
 #define MXC_S_SPI_CTRL2_DATA_WIDTH_MONO                (MXC_V_SPI_CTRL2_DATA_WIDTH_MONO << MXC_F_SPI_CTRL2_DATA_WIDTH_POS) /**< CTRL2_DATA_WIDTH_MONO Setting */
 #define MXC_V_SPI_CTRL2_DATA_WIDTH_DUAL                ((uint32_t)0x1) /**< CTRL2_DATA_WIDTH_DUAL Value */
 #define MXC_S_SPI_CTRL2_DATA_WIDTH_DUAL                (MXC_V_SPI_CTRL2_DATA_WIDTH_DUAL << MXC_F_SPI_CTRL2_DATA_WIDTH_POS) /**< CTRL2_DATA_WIDTH_DUAL Setting */
 #define MXC_V_SPI_CTRL2_DATA_WIDTH_QUAD                ((uint32_t)0x2) /**< CTRL2_DATA_WIDTH_QUAD Value */
 #define MXC_S_SPI_CTRL2_DATA_WIDTH_QUAD                (MXC_V_SPI_CTRL2_DATA_WIDTH_QUAD << MXC_F_SPI_CTRL2_DATA_WIDTH_POS) /**< CTRL2_DATA_WIDTH_QUAD Setting */

 #define MXC_F_SPI_CTRL2_THREE_WIRE_POS                 (15) /**< CTRL2_THREE_WIRE Position */
 #define MXC_F_SPI_CTRL2_THREE_WIRE                     ((uint32_t)(0x1 << MXC_F_SPI_CTRL2_THREE_WIRE_POS)) /**< CTRL2_THREE_WIRE Mask */
 #define MXC_V_SPI_CTRL2_THREE_WIRE_DIS                 ((uint32_t)0x0) /**< CTRL2_THREE_WIRE_DIS Value */
 #define MXC_S_SPI_CTRL2_THREE_WIRE_DIS                 (MXC_V_SPI_CTRL2_THREE_WIRE_DIS << MXC_F_SPI_CTRL2_THREE_WIRE_POS) /**< CTRL2_THREE_WIRE_DIS Setting */
 #define MXC_V_SPI_CTRL2_THREE_WIRE_EN                  ((uint32_t)0x1) /**< CTRL2_THREE_WIRE_EN Value */
 #define MXC_S_SPI_CTRL2_THREE_WIRE_EN                  (MXC_V_SPI_CTRL2_THREE_WIRE_EN << MXC_F_SPI_CTRL2_THREE_WIRE_POS) /**< CTRL2_THREE_WIRE_EN Setting */

 #define MXC_F_SPI_CTRL2_SS_POL_POS                     (16) /**< CTRL2_SS_POL Position */
 #define MXC_F_SPI_CTRL2_SS_POL                         ((uint32_t)(0xFF << MXC_F_SPI_CTRL2_SS_POL_POS)) /**< CTRL2_SS_POL Mask */
 #define MXC_V_SPI_CTRL2_SS_POL_SS0_HIGH                ((uint32_t)0x1) /**< CTRL2_SS_POL_SS0_HIGH Value */
 #define MXC_S_SPI_CTRL2_SS_POL_SS0_HIGH                (MXC_V_SPI_CTRL2_SS_POL_SS0_HIGH << MXC_F_SPI_CTRL2_SS_POL_POS) /**< CTRL2_SS_POL_SS0_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SS_POL_SS1_HIGH                ((uint32_t)0x2) /**< CTRL2_SS_POL_SS1_HIGH Value */
 #define MXC_S_SPI_CTRL2_SS_POL_SS1_HIGH                (MXC_V_SPI_CTRL2_SS_POL_SS1_HIGH << MXC_F_SPI_CTRL2_SS_POL_POS) /**< CTRL2_SS_POL_SS1_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SS_POL_SS2_HIGH                ((uint32_t)0x4) /**< CTRL2_SS_POL_SS2_HIGH Value */
 #define MXC_S_SPI_CTRL2_SS_POL_SS2_HIGH                (MXC_V_SPI_CTRL2_SS_POL_SS2_HIGH << MXC_F_SPI_CTRL2_SS_POL_POS) /**< CTRL2_SS_POL_SS2_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SS_POL_SS3_HIGH                ((uint32_t)0x8) /**< CTRL2_SS_POL_SS3_HIGH Value */
 #define MXC_S_SPI_CTRL2_SS_POL_SS3_HIGH                (MXC_V_SPI_CTRL2_SS_POL_SS3_HIGH << MXC_F_SPI_CTRL2_SS_POL_POS) /**< CTRL2_SS_POL_SS3_HIGH Setting */

 #define MXC_F_SPI_CTRL2_SRPOL_POS                      (24) /**< CTRL2_SRPOL Position */
 #define MXC_F_SPI_CTRL2_SRPOL                          ((uint32_t)(0xFF << MXC_F_SPI_CTRL2_SRPOL_POS)) /**< CTRL2_SRPOL Mask */
 #define MXC_V_SPI_CTRL2_SRPOL_SR0_HIGH                 ((uint32_t)0x1) /**< CTRL2_SRPOL_SR0_HIGH Value */
 #define MXC_S_SPI_CTRL2_SRPOL_SR0_HIGH                 (MXC_V_SPI_CTRL2_SRPOL_SR0_HIGH << MXC_F_SPI_CTRL2_SRPOL_POS) /**< CTRL2_SRPOL_SR0_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SRPOL_SR1_HIGH                 ((uint32_t)0x2) /**< CTRL2_SRPOL_SR1_HIGH Value */
 #define MXC_S_SPI_CTRL2_SRPOL_SR1_HIGH                 (MXC_V_SPI_CTRL2_SRPOL_SR1_HIGH << MXC_F_SPI_CTRL2_SRPOL_POS) /**< CTRL2_SRPOL_SR1_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SRPOL_SR2_HIGH                 ((uint32_t)0x4) /**< CTRL2_SRPOL_SR2_HIGH Value */
 #define MXC_S_SPI_CTRL2_SRPOL_SR2_HIGH                 (MXC_V_SPI_CTRL2_SRPOL_SR2_HIGH << MXC_F_SPI_CTRL2_SRPOL_POS) /**< CTRL2_SRPOL_SR2_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SRPOL_SR3_HIGH                 ((uint32_t)0x8) /**< CTRL2_SRPOL_SR3_HIGH Value */
 #define MXC_S_SPI_CTRL2_SRPOL_SR3_HIGH                 (MXC_V_SPI_CTRL2_SRPOL_SR3_HIGH << MXC_F_SPI_CTRL2_SRPOL_POS) /**< CTRL2_SRPOL_SR3_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SRPOL_SR4_HIGH                 ((uint32_t)0x10) /**< CTRL2_SRPOL_SR4_HIGH Value */
 #define MXC_S_SPI_CTRL2_SRPOL_SR4_HIGH                 (MXC_V_SPI_CTRL2_SRPOL_SR4_HIGH << MXC_F_SPI_CTRL2_SRPOL_POS) /**< CTRL2_SRPOL_SR4_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SRPOL_SR5_HIGH                 ((uint32_t)0x20) /**< CTRL2_SRPOL_SR5_HIGH Value */
 #define MXC_S_SPI_CTRL2_SRPOL_SR5_HIGH                 (MXC_V_SPI_CTRL2_SRPOL_SR5_HIGH << MXC_F_SPI_CTRL2_SRPOL_POS) /**< CTRL2_SRPOL_SR5_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SRPOL_SR6_HIGH                 ((uint32_t)0x40) /**< CTRL2_SRPOL_SR6_HIGH Value */
 #define MXC_S_SPI_CTRL2_SRPOL_SR6_HIGH                 (MXC_V_SPI_CTRL2_SRPOL_SR6_HIGH << MXC_F_SPI_CTRL2_SRPOL_POS) /**< CTRL2_SRPOL_SR6_HIGH Setting */
 #define MXC_V_SPI_CTRL2_SRPOL_SR7_HIGH                 ((uint32_t)0x80) /**< CTRL2_SRPOL_SR7_HIGH Value */
 #define MXC_S_SPI_CTRL2_SRPOL_SR7_HIGH                 (MXC_V_SPI_CTRL2_SRPOL_SR7_HIGH << MXC_F_SPI_CTRL2_SRPOL_POS) /**< CTRL2_SRPOL_SR7_HIGH Setting */

/**@} end of group CTRL2_Register */

/**
 * @ingroup  spi_registers
 * @defgroup SS_TIME_Register
 * @brief    Register for controlling SPI peripheral/Slave Select Timing.
 * @{
 */
 #define MXC_F_SPI_SS_TIME_PRE_POS                      (0) /**< SS_TIME_PRE Position */
 #define MXC_F_SPI_SS_TIME_PRE                          ((uint32_t)(0xFF << MXC_F_SPI_SS_TIME_PRE_POS)) /**< SS_TIME_PRE Mask */
 #define MXC_V_SPI_SS_TIME_PRE_256                      ((uint32_t)0x0) /**< SS_TIME_PRE_256 Value */
 #define MXC_S_SPI_SS_TIME_PRE_256                      (MXC_V_SPI_SS_TIME_PRE_256 << MXC_F_SPI_SS_TIME_PRE_POS) /**< SS_TIME_PRE_256 Setting */

 #define MXC_F_SPI_SS_TIME_POST_POS                     (8) /**< SS_TIME_POST Position */
 #define MXC_F_SPI_SS_TIME_POST                         ((uint32_t)(0xFF << MXC_F_SPI_SS_TIME_POST_POS)) /**< SS_TIME_POST Mask */
 #define MXC_V_SPI_SS_TIME_POST_256                     ((uint32_t)0x0) /**< SS_TIME_POST_256 Value */
 #define MXC_S_SPI_SS_TIME_POST_256                     (MXC_V_SPI_SS_TIME_POST_256 << MXC_F_SPI_SS_TIME_POST_POS) /**< SS_TIME_POST_256 Setting */

 #define MXC_F_SPI_SS_TIME_INACT_POS                    (16) /**< SS_TIME_INACT Position */
 #define MXC_F_SPI_SS_TIME_INACT                        ((uint32_t)(0xFF << MXC_F_SPI_SS_TIME_INACT_POS)) /**< SS_TIME_INACT Mask */
 #define MXC_V_SPI_SS_TIME_INACT_256                    ((uint32_t)0x0) /**< SS_TIME_INACT_256 Value */
 #define MXC_S_SPI_SS_TIME_INACT_256                    (MXC_V_SPI_SS_TIME_INACT_256 << MXC_F_SPI_SS_TIME_INACT_POS) /**< SS_TIME_INACT_256 Setting */

/**@} end of group SS_TIME_Register */

/**
 * @ingroup  spi_registers
 * @defgroup CLK_CFG_Register
 * @brief    Register for controlling SPI clock rate.
 * @{
 */
 #define MXC_F_SPI_CLK_CFG_LO_POS                       (0) /**< CLK_CFG_LO Position */
 #define MXC_F_SPI_CLK_CFG_LO                           ((uint32_t)(0xFF << MXC_F_SPI_CLK_CFG_LO_POS)) /**< CLK_CFG_LO Mask */
 #define MXC_V_SPI_CLK_CFG_LO_DIS                       ((uint32_t)0x0) /**< CLK_CFG_LO_DIS Value */
 #define MXC_S_SPI_CLK_CFG_LO_DIS                       (MXC_V_SPI_CLK_CFG_LO_DIS << MXC_F_SPI_CLK_CFG_LO_POS) /**< CLK_CFG_LO_DIS Setting */

 #define MXC_F_SPI_CLK_CFG_HI_POS                       (8) /**< CLK_CFG_HI Position */
 #define MXC_F_SPI_CLK_CFG_HI                           ((uint32_t)(0xFF << MXC_F_SPI_CLK_CFG_HI_POS)) /**< CLK_CFG_HI Mask */
 #define MXC_V_SPI_CLK_CFG_HI_DIS                       ((uint32_t)0x0) /**< CLK_CFG_HI_DIS Value */
 #define MXC_S_SPI_CLK_CFG_HI_DIS                       (MXC_V_SPI_CLK_CFG_HI_DIS << MXC_F_SPI_CLK_CFG_HI_POS) /**< CLK_CFG_HI_DIS Setting */

 #define MXC_F_SPI_CLK_CFG_SCALE_POS                    (16) /**< CLK_CFG_SCALE Position */
 #define MXC_F_SPI_CLK_CFG_SCALE                        ((uint32_t)(0xF << MXC_F_SPI_CLK_CFG_SCALE_POS)) /**< CLK_CFG_SCALE Mask */

/**@} end of group CLK_CFG_Register */

/**
 * @ingroup  spi_registers
 * @defgroup DMA_Register
 * @brief    Register for controlling DMA.
 * @{
 */
 #define MXC_F_SPI_DMA_TX_FIFO_LEVEL_POS                (0) /**< DMA_TX_FIFO_LEVEL Position */
 #define MXC_F_SPI_DMA_TX_FIFO_LEVEL                    ((uint32_t)(0x1F << MXC_F_SPI_DMA_TX_FIFO_LEVEL_POS)) /**< DMA_TX_FIFO_LEVEL Mask */

 #define MXC_F_SPI_DMA_TX_FIFO_EN_POS                   (6) /**< DMA_TX_FIFO_EN Position */
 #define MXC_F_SPI_DMA_TX_FIFO_EN                       ((uint32_t)(0x1 << MXC_F_SPI_DMA_TX_FIFO_EN_POS)) /**< DMA_TX_FIFO_EN Mask */
 #define MXC_V_SPI_DMA_TX_FIFO_EN_DIS                   ((uint32_t)0x0) /**< DMA_TX_FIFO_EN_DIS Value */
 #define MXC_S_SPI_DMA_TX_FIFO_EN_DIS                   (MXC_V_SPI_DMA_TX_FIFO_EN_DIS << MXC_F_SPI_DMA_TX_FIFO_EN_POS) /**< DMA_TX_FIFO_EN_DIS Setting */
 #define MXC_V_SPI_DMA_TX_FIFO_EN_EN                    ((uint32_t)0x1) /**< DMA_TX_FIFO_EN_EN Value */
 #define MXC_S_SPI_DMA_TX_FIFO_EN_EN                    (MXC_V_SPI_DMA_TX_FIFO_EN_EN << MXC_F_SPI_DMA_TX_FIFO_EN_POS) /**< DMA_TX_FIFO_EN_EN Setting */

 #define MXC_F_SPI_DMA_TX_FIFO_CLEAR_POS                (7) /**< DMA_TX_FIFO_CLEAR Position */
 #define MXC_F_SPI_DMA_TX_FIFO_CLEAR                    ((uint32_t)(0x1 << MXC_F_SPI_DMA_TX_FIFO_CLEAR_POS)) /**< DMA_TX_FIFO_CLEAR Mask */
 #define MXC_V_SPI_DMA_TX_FIFO_CLEAR_CLEAR              ((uint32_t)0x1) /**< DMA_TX_FIFO_CLEAR_CLEAR Value */
 #define MXC_S_SPI_DMA_TX_FIFO_CLEAR_CLEAR              (MXC_V_SPI_DMA_TX_FIFO_CLEAR_CLEAR << MXC_F_SPI_DMA_TX_FIFO_CLEAR_POS) /**< DMA_TX_FIFO_CLEAR_CLEAR Setting */

 #define MXC_F_SPI_DMA_TX_FIFO_CNT_POS                  (8) /**< DMA_TX_FIFO_CNT Position */
 #define MXC_F_SPI_DMA_TX_FIFO_CNT                      ((uint32_t)(0x3F << MXC_F_SPI_DMA_TX_FIFO_CNT_POS)) /**< DMA_TX_FIFO_CNT Mask */

 #define MXC_F_SPI_DMA_TX_DMA_EN_POS                    (15) /**< DMA_TX_DMA_EN Position */
 #define MXC_F_SPI_DMA_TX_DMA_EN                        ((uint32_t)(0x1 << MXC_F_SPI_DMA_TX_DMA_EN_POS)) /**< DMA_TX_DMA_EN Mask */
 #define MXC_V_SPI_DMA_TX_DMA_EN_DIS                    ((uint32_t)0x0) /**< DMA_TX_DMA_EN_DIS Value */
 #define MXC_S_SPI_DMA_TX_DMA_EN_DIS                    (MXC_V_SPI_DMA_TX_DMA_EN_DIS << MXC_F_SPI_DMA_TX_DMA_EN_POS) /**< DMA_TX_DMA_EN_DIS Setting */
 #define MXC_V_SPI_DMA_TX_DMA_EN_EN                     ((uint32_t)0x1) /**< DMA_TX_DMA_EN_EN Value */
 #define MXC_S_SPI_DMA_TX_DMA_EN_EN                     (MXC_V_SPI_DMA_TX_DMA_EN_EN << MXC_F_SPI_DMA_TX_DMA_EN_POS) /**< DMA_TX_DMA_EN_EN Setting */

 #define MXC_F_SPI_DMA_RX_FIFO_LEVEL_POS                (16) /**< DMA_RX_FIFO_LEVEL Position */
 #define MXC_F_SPI_DMA_RX_FIFO_LEVEL                    ((uint32_t)(0x1F << MXC_F_SPI_DMA_RX_FIFO_LEVEL_POS)) /**< DMA_RX_FIFO_LEVEL Mask */

 #define MXC_F_SPI_DMA_RX_FIFO_EN_POS                   (22) /**< DMA_RX_FIFO_EN Position */
 #define MXC_F_SPI_DMA_RX_FIFO_EN                       ((uint32_t)(0x1 << MXC_F_SPI_DMA_RX_FIFO_EN_POS)) /**< DMA_RX_FIFO_EN Mask */
 #define MXC_V_SPI_DMA_RX_FIFO_EN_DIS                   ((uint32_t)0x0) /**< DMA_RX_FIFO_EN_DIS Value */
 #define MXC_S_SPI_DMA_RX_FIFO_EN_DIS                   (MXC_V_SPI_DMA_RX_FIFO_EN_DIS << MXC_F_SPI_DMA_RX_FIFO_EN_POS) /**< DMA_RX_FIFO_EN_DIS Setting */
 #define MXC_V_SPI_DMA_RX_FIFO_EN_EN                    ((uint32_t)0x1) /**< DMA_RX_FIFO_EN_EN Value */
 #define MXC_S_SPI_DMA_RX_FIFO_EN_EN                    (MXC_V_SPI_DMA_RX_FIFO_EN_EN << MXC_F_SPI_DMA_RX_FIFO_EN_POS) /**< DMA_RX_FIFO_EN_EN Setting */

 #define MXC_F_SPI_DMA_RX_FIFO_CLEAR_POS                (23) /**< DMA_RX_FIFO_CLEAR Position */
 #define MXC_F_SPI_DMA_RX_FIFO_CLEAR                    ((uint32_t)(0x1 << MXC_F_SPI_DMA_RX_FIFO_CLEAR_POS)) /**< DMA_RX_FIFO_CLEAR Mask */
 #define MXC_V_SPI_DMA_RX_FIFO_CLEAR_CLEAR              ((uint32_t)0x1) /**< DMA_RX_FIFO_CLEAR_CLEAR Value */
 #define MXC_S_SPI_DMA_RX_FIFO_CLEAR_CLEAR              (MXC_V_SPI_DMA_RX_FIFO_CLEAR_CLEAR << MXC_F_SPI_DMA_RX_FIFO_CLEAR_POS) /**< DMA_RX_FIFO_CLEAR_CLEAR Setting */

 #define MXC_F_SPI_DMA_RX_FIFO_CNT_POS                  (24) /**< DMA_RX_FIFO_CNT Position */
 #define MXC_F_SPI_DMA_RX_FIFO_CNT                      ((uint32_t)(0x3F << MXC_F_SPI_DMA_RX_FIFO_CNT_POS)) /**< DMA_RX_FIFO_CNT Mask */

 #define MXC_F_SPI_DMA_RX_DMA_EN_POS                    (31) /**< DMA_RX_DMA_EN Position */
 #define MXC_F_SPI_DMA_RX_DMA_EN                        ((uint32_t)(0x1 << MXC_F_SPI_DMA_RX_DMA_EN_POS)) /**< DMA_RX_DMA_EN Mask */
 #define MXC_V_SPI_DMA_RX_DMA_EN_DIS                    ((uint32_t)0x0) /**< DMA_RX_DMA_EN_DIS Value */
 #define MXC_S_SPI_DMA_RX_DMA_EN_DIS                    (MXC_V_SPI_DMA_RX_DMA_EN_DIS << MXC_F_SPI_DMA_RX_DMA_EN_POS) /**< DMA_RX_DMA_EN_DIS Setting */
 #define MXC_V_SPI_DMA_RX_DMA_EN_EN                     ((uint32_t)0x1) /**< DMA_RX_DMA_EN_EN Value */
 #define MXC_S_SPI_DMA_RX_DMA_EN_EN                     (MXC_V_SPI_DMA_RX_DMA_EN_EN << MXC_F_SPI_DMA_RX_DMA_EN_POS) /**< DMA_RX_DMA_EN_EN Setting */

/**@} end of group DMA_Register */

/**
 * @ingroup  spi_registers
 * @defgroup INT_FL_Register
 * @brief    Register for reading and clearing interrupt flags. All bits are write 1 to
 *           clear.
 * @{
 */
 #define MXC_F_SPI_INT_FL_TX_THRESH_POS                 (0) /**< INT_FL_TX_THRESH Position */
 #define MXC_F_SPI_INT_FL_TX_THRESH                     ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_TX_THRESH_POS)) /**< INT_FL_TX_THRESH Mask */
 #define MXC_V_SPI_INT_FL_TX_THRESH_CLEAR               ((uint32_t)0x1) /**< INT_FL_TX_THRESH_CLEAR Value */
 #define MXC_S_SPI_INT_FL_TX_THRESH_CLEAR               (MXC_V_SPI_INT_FL_TX_THRESH_CLEAR << MXC_F_SPI_INT_FL_TX_THRESH_POS) /**< INT_FL_TX_THRESH_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_TX_EMPTY_POS                  (1) /**< INT_FL_TX_EMPTY Position */
 #define MXC_F_SPI_INT_FL_TX_EMPTY                      ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_TX_EMPTY_POS)) /**< INT_FL_TX_EMPTY Mask */
 #define MXC_V_SPI_INT_FL_TX_EMPTY_CLEAR                ((uint32_t)0x1) /**< INT_FL_TX_EMPTY_CLEAR Value */
 #define MXC_S_SPI_INT_FL_TX_EMPTY_CLEAR                (MXC_V_SPI_INT_FL_TX_EMPTY_CLEAR << MXC_F_SPI_INT_FL_TX_EMPTY_POS) /**< INT_FL_TX_EMPTY_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_RX_THRESH_POS                 (2) /**< INT_FL_RX_THRESH Position */
 #define MXC_F_SPI_INT_FL_RX_THRESH                     ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_RX_THRESH_POS)) /**< INT_FL_RX_THRESH Mask */
 #define MXC_V_SPI_INT_FL_RX_THRESH_CLEAR               ((uint32_t)0x1) /**< INT_FL_RX_THRESH_CLEAR Value */
 #define MXC_S_SPI_INT_FL_RX_THRESH_CLEAR               (MXC_V_SPI_INT_FL_RX_THRESH_CLEAR << MXC_F_SPI_INT_FL_RX_THRESH_POS) /**< INT_FL_RX_THRESH_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_RX_FULL_POS                   (3) /**< INT_FL_RX_FULL Position */
 #define MXC_F_SPI_INT_FL_RX_FULL                       ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_RX_FULL_POS)) /**< INT_FL_RX_FULL Mask */
 #define MXC_V_SPI_INT_FL_RX_FULL_CLEAR                 ((uint32_t)0x1) /**< INT_FL_RX_FULL_CLEAR Value */
 #define MXC_S_SPI_INT_FL_RX_FULL_CLEAR                 (MXC_V_SPI_INT_FL_RX_FULL_CLEAR << MXC_F_SPI_INT_FL_RX_FULL_POS) /**< INT_FL_RX_FULL_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_SSA_POS                       (4) /**< INT_FL_SSA Position */
 #define MXC_F_SPI_INT_FL_SSA                           ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_SSA_POS)) /**< INT_FL_SSA Mask */
 #define MXC_V_SPI_INT_FL_SSA_CLEAR                     ((uint32_t)0x1) /**< INT_FL_SSA_CLEAR Value */
 #define MXC_S_SPI_INT_FL_SSA_CLEAR                     (MXC_V_SPI_INT_FL_SSA_CLEAR << MXC_F_SPI_INT_FL_SSA_POS) /**< INT_FL_SSA_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_SSD_POS                       (5) /**< INT_FL_SSD Position */
 #define MXC_F_SPI_INT_FL_SSD                           ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_SSD_POS)) /**< INT_FL_SSD Mask */
 #define MXC_V_SPI_INT_FL_SSD_CLEAR                     ((uint32_t)0x1) /**< INT_FL_SSD_CLEAR Value */
 #define MXC_S_SPI_INT_FL_SSD_CLEAR                     (MXC_V_SPI_INT_FL_SSD_CLEAR << MXC_F_SPI_INT_FL_SSD_POS) /**< INT_FL_SSD_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_FAULT_POS                     (8) /**< INT_FL_FAULT Position */
 #define MXC_F_SPI_INT_FL_FAULT                         ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_FAULT_POS)) /**< INT_FL_FAULT Mask */
 #define MXC_V_SPI_INT_FL_FAULT_CLEAR                   ((uint32_t)0x1) /**< INT_FL_FAULT_CLEAR Value */
 #define MXC_S_SPI_INT_FL_FAULT_CLEAR                   (MXC_V_SPI_INT_FL_FAULT_CLEAR << MXC_F_SPI_INT_FL_FAULT_POS) /**< INT_FL_FAULT_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_ABORT_POS                     (9) /**< INT_FL_ABORT Position */
 #define MXC_F_SPI_INT_FL_ABORT                         ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_ABORT_POS)) /**< INT_FL_ABORT Mask */
 #define MXC_V_SPI_INT_FL_ABORT_CLEAR                   ((uint32_t)0x1) /**< INT_FL_ABORT_CLEAR Value */
 #define MXC_S_SPI_INT_FL_ABORT_CLEAR                   (MXC_V_SPI_INT_FL_ABORT_CLEAR << MXC_F_SPI_INT_FL_ABORT_POS) /**< INT_FL_ABORT_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_M_DONE_POS                    (11) /**< INT_FL_M_DONE Position */
 #define MXC_F_SPI_INT_FL_M_DONE                        ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_M_DONE_POS)) /**< INT_FL_M_DONE Mask */
 #define MXC_V_SPI_INT_FL_M_DONE_CLEAR                  ((uint32_t)0x1) /**< INT_FL_M_DONE_CLEAR Value */
 #define MXC_S_SPI_INT_FL_M_DONE_CLEAR                  (MXC_V_SPI_INT_FL_M_DONE_CLEAR << MXC_F_SPI_INT_FL_M_DONE_POS) /**< INT_FL_M_DONE_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_TX_OVR_POS                    (12) /**< INT_FL_TX_OVR Position */
 #define MXC_F_SPI_INT_FL_TX_OVR                        ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_TX_OVR_POS)) /**< INT_FL_TX_OVR Mask */
 #define MXC_V_SPI_INT_FL_TX_OVR_CLEAR                  ((uint32_t)0x1) /**< INT_FL_TX_OVR_CLEAR Value */
 #define MXC_S_SPI_INT_FL_TX_OVR_CLEAR                  (MXC_V_SPI_INT_FL_TX_OVR_CLEAR << MXC_F_SPI_INT_FL_TX_OVR_POS) /**< INT_FL_TX_OVR_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_TX_UND_POS                    (13) /**< INT_FL_TX_UND Position */
 #define MXC_F_SPI_INT_FL_TX_UND                        ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_TX_UND_POS)) /**< INT_FL_TX_UND Mask */
 #define MXC_V_SPI_INT_FL_TX_UND_CLEAR                  ((uint32_t)0x1) /**< INT_FL_TX_UND_CLEAR Value */
 #define MXC_S_SPI_INT_FL_TX_UND_CLEAR                  (MXC_V_SPI_INT_FL_TX_UND_CLEAR << MXC_F_SPI_INT_FL_TX_UND_POS) /**< INT_FL_TX_UND_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_RX_OVR_POS                    (14) /**< INT_FL_RX_OVR Position */
 #define MXC_F_SPI_INT_FL_RX_OVR                        ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_RX_OVR_POS)) /**< INT_FL_RX_OVR Mask */
 #define MXC_V_SPI_INT_FL_RX_OVR_CLEAR                  ((uint32_t)0x1) /**< INT_FL_RX_OVR_CLEAR Value */
 #define MXC_S_SPI_INT_FL_RX_OVR_CLEAR                  (MXC_V_SPI_INT_FL_RX_OVR_CLEAR << MXC_F_SPI_INT_FL_RX_OVR_POS) /**< INT_FL_RX_OVR_CLEAR Setting */

 #define MXC_F_SPI_INT_FL_RX_UND_POS                    (15) /**< INT_FL_RX_UND Position */
 #define MXC_F_SPI_INT_FL_RX_UND                        ((uint32_t)(0x1 << MXC_F_SPI_INT_FL_RX_UND_POS)) /**< INT_FL_RX_UND Mask */
 #define MXC_V_SPI_INT_FL_RX_UND_CLEAR                  ((uint32_t)0x1) /**< INT_FL_RX_UND_CLEAR Value */
 #define MXC_S_SPI_INT_FL_RX_UND_CLEAR                  (MXC_V_SPI_INT_FL_RX_UND_CLEAR << MXC_F_SPI_INT_FL_RX_UND_POS) /**< INT_FL_RX_UND_CLEAR Setting */

/**@} end of group INT_FL_Register */

/**
 * @ingroup  spi_registers
 * @defgroup INT_EN_Register
 * @brief    Register for enabling interrupts.
 * @{
 */
 #define MXC_F_SPI_INT_EN_TX_THRESH_POS                 (0) /**< INT_EN_TX_THRESH Position */
 #define MXC_F_SPI_INT_EN_TX_THRESH                     ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_TX_THRESH_POS)) /**< INT_EN_TX_THRESH Mask */
 #define MXC_V_SPI_INT_EN_TX_THRESH_DIS                 ((uint32_t)0x0) /**< INT_EN_TX_THRESH_DIS Value */
 #define MXC_S_SPI_INT_EN_TX_THRESH_DIS                 (MXC_V_SPI_INT_EN_TX_THRESH_DIS << MXC_F_SPI_INT_EN_TX_THRESH_POS) /**< INT_EN_TX_THRESH_DIS Setting */
 #define MXC_V_SPI_INT_EN_TX_THRESH_EN                  ((uint32_t)0x1) /**< INT_EN_TX_THRESH_EN Value */
 #define MXC_S_SPI_INT_EN_TX_THRESH_EN                  (MXC_V_SPI_INT_EN_TX_THRESH_EN << MXC_F_SPI_INT_EN_TX_THRESH_POS) /**< INT_EN_TX_THRESH_EN Setting */

 #define MXC_F_SPI_INT_EN_TX_EMPTY_POS                  (1) /**< INT_EN_TX_EMPTY Position */
 #define MXC_F_SPI_INT_EN_TX_EMPTY                      ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_TX_EMPTY_POS)) /**< INT_EN_TX_EMPTY Mask */
 #define MXC_V_SPI_INT_EN_TX_EMPTY_DIS                  ((uint32_t)0x0) /**< INT_EN_TX_EMPTY_DIS Value */
 #define MXC_S_SPI_INT_EN_TX_EMPTY_DIS                  (MXC_V_SPI_INT_EN_TX_EMPTY_DIS << MXC_F_SPI_INT_EN_TX_EMPTY_POS) /**< INT_EN_TX_EMPTY_DIS Setting */
 #define MXC_V_SPI_INT_EN_TX_EMPTY_EN                   ((uint32_t)0x1) /**< INT_EN_TX_EMPTY_EN Value */
 #define MXC_S_SPI_INT_EN_TX_EMPTY_EN                   (MXC_V_SPI_INT_EN_TX_EMPTY_EN << MXC_F_SPI_INT_EN_TX_EMPTY_POS) /**< INT_EN_TX_EMPTY_EN Setting */

 #define MXC_F_SPI_INT_EN_RX_THRESH_POS                 (2) /**< INT_EN_RX_THRESH Position */
 #define MXC_F_SPI_INT_EN_RX_THRESH                     ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_RX_THRESH_POS)) /**< INT_EN_RX_THRESH Mask */
 #define MXC_V_SPI_INT_EN_RX_THRESH_DIS                 ((uint32_t)0x0) /**< INT_EN_RX_THRESH_DIS Value */
 #define MXC_S_SPI_INT_EN_RX_THRESH_DIS                 (MXC_V_SPI_INT_EN_RX_THRESH_DIS << MXC_F_SPI_INT_EN_RX_THRESH_POS) /**< INT_EN_RX_THRESH_DIS Setting */
 #define MXC_V_SPI_INT_EN_RX_THRESH_EN                  ((uint32_t)0x1) /**< INT_EN_RX_THRESH_EN Value */
 #define MXC_S_SPI_INT_EN_RX_THRESH_EN                  (MXC_V_SPI_INT_EN_RX_THRESH_EN << MXC_F_SPI_INT_EN_RX_THRESH_POS) /**< INT_EN_RX_THRESH_EN Setting */

 #define MXC_F_SPI_INT_EN_RX_FULL_POS                   (3) /**< INT_EN_RX_FULL Position */
 #define MXC_F_SPI_INT_EN_RX_FULL                       ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_RX_FULL_POS)) /**< INT_EN_RX_FULL Mask */
 #define MXC_V_SPI_INT_EN_RX_FULL_DIS                   ((uint32_t)0x0) /**< INT_EN_RX_FULL_DIS Value */
 #define MXC_S_SPI_INT_EN_RX_FULL_DIS                   (MXC_V_SPI_INT_EN_RX_FULL_DIS << MXC_F_SPI_INT_EN_RX_FULL_POS) /**< INT_EN_RX_FULL_DIS Setting */
 #define MXC_V_SPI_INT_EN_RX_FULL_EN                    ((uint32_t)0x1) /**< INT_EN_RX_FULL_EN Value */
 #define MXC_S_SPI_INT_EN_RX_FULL_EN                    (MXC_V_SPI_INT_EN_RX_FULL_EN << MXC_F_SPI_INT_EN_RX_FULL_POS) /**< INT_EN_RX_FULL_EN Setting */

 #define MXC_F_SPI_INT_EN_SSA_POS                       (4) /**< INT_EN_SSA Position */
 #define MXC_F_SPI_INT_EN_SSA                           ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_SSA_POS)) /**< INT_EN_SSA Mask */
 #define MXC_V_SPI_INT_EN_SSA_DIS                       ((uint32_t)0x0) /**< INT_EN_SSA_DIS Value */
 #define MXC_S_SPI_INT_EN_SSA_DIS                       (MXC_V_SPI_INT_EN_SSA_DIS << MXC_F_SPI_INT_EN_SSA_POS) /**< INT_EN_SSA_DIS Setting */
 #define MXC_V_SPI_INT_EN_SSA_EN                        ((uint32_t)0x1) /**< INT_EN_SSA_EN Value */
 #define MXC_S_SPI_INT_EN_SSA_EN                        (MXC_V_SPI_INT_EN_SSA_EN << MXC_F_SPI_INT_EN_SSA_POS) /**< INT_EN_SSA_EN Setting */

 #define MXC_F_SPI_INT_EN_SSD_POS                       (5) /**< INT_EN_SSD Position */
 #define MXC_F_SPI_INT_EN_SSD                           ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_SSD_POS)) /**< INT_EN_SSD Mask */
 #define MXC_V_SPI_INT_EN_SSD_DIS                       ((uint32_t)0x0) /**< INT_EN_SSD_DIS Value */
 #define MXC_S_SPI_INT_EN_SSD_DIS                       (MXC_V_SPI_INT_EN_SSD_DIS << MXC_F_SPI_INT_EN_SSD_POS) /**< INT_EN_SSD_DIS Setting */
 #define MXC_V_SPI_INT_EN_SSD_EN                        ((uint32_t)0x1) /**< INT_EN_SSD_EN Value */
 #define MXC_S_SPI_INT_EN_SSD_EN                        (MXC_V_SPI_INT_EN_SSD_EN << MXC_F_SPI_INT_EN_SSD_POS) /**< INT_EN_SSD_EN Setting */

 #define MXC_F_SPI_INT_EN_FAULT_POS                     (8) /**< INT_EN_FAULT Position */
 #define MXC_F_SPI_INT_EN_FAULT                         ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_FAULT_POS)) /**< INT_EN_FAULT Mask */
 #define MXC_V_SPI_INT_EN_FAULT_DIS                     ((uint32_t)0x0) /**< INT_EN_FAULT_DIS Value */
 #define MXC_S_SPI_INT_EN_FAULT_DIS                     (MXC_V_SPI_INT_EN_FAULT_DIS << MXC_F_SPI_INT_EN_FAULT_POS) /**< INT_EN_FAULT_DIS Setting */
 #define MXC_V_SPI_INT_EN_FAULT_EN                      ((uint32_t)0x1) /**< INT_EN_FAULT_EN Value */
 #define MXC_S_SPI_INT_EN_FAULT_EN                      (MXC_V_SPI_INT_EN_FAULT_EN << MXC_F_SPI_INT_EN_FAULT_POS) /**< INT_EN_FAULT_EN Setting */

 #define MXC_F_SPI_INT_EN_ABORT_POS                     (9) /**< INT_EN_ABORT Position */
 #define MXC_F_SPI_INT_EN_ABORT                         ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_ABORT_POS)) /**< INT_EN_ABORT Mask */
 #define MXC_V_SPI_INT_EN_ABORT_DIS                     ((uint32_t)0x0) /**< INT_EN_ABORT_DIS Value */
 #define MXC_S_SPI_INT_EN_ABORT_DIS                     (MXC_V_SPI_INT_EN_ABORT_DIS << MXC_F_SPI_INT_EN_ABORT_POS) /**< INT_EN_ABORT_DIS Setting */
 #define MXC_V_SPI_INT_EN_ABORT_EN                      ((uint32_t)0x1) /**< INT_EN_ABORT_EN Value */
 #define MXC_S_SPI_INT_EN_ABORT_EN                      (MXC_V_SPI_INT_EN_ABORT_EN << MXC_F_SPI_INT_EN_ABORT_POS) /**< INT_EN_ABORT_EN Setting */

 #define MXC_F_SPI_INT_EN_M_DONE_POS                    (11) /**< INT_EN_M_DONE Position */
 #define MXC_F_SPI_INT_EN_M_DONE                        ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_M_DONE_POS)) /**< INT_EN_M_DONE Mask */
 #define MXC_V_SPI_INT_EN_M_DONE_DIS                    ((uint32_t)0x0) /**< INT_EN_M_DONE_DIS Value */
 #define MXC_S_SPI_INT_EN_M_DONE_DIS                    (MXC_V_SPI_INT_EN_M_DONE_DIS << MXC_F_SPI_INT_EN_M_DONE_POS) /**< INT_EN_M_DONE_DIS Setting */
 #define MXC_V_SPI_INT_EN_M_DONE_EN                     ((uint32_t)0x1) /**< INT_EN_M_DONE_EN Value */
 #define MXC_S_SPI_INT_EN_M_DONE_EN                     (MXC_V_SPI_INT_EN_M_DONE_EN << MXC_F_SPI_INT_EN_M_DONE_POS) /**< INT_EN_M_DONE_EN Setting */

 #define MXC_F_SPI_INT_EN_TX_OVR_POS                    (12) /**< INT_EN_TX_OVR Position */
 #define MXC_F_SPI_INT_EN_TX_OVR                        ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_TX_OVR_POS)) /**< INT_EN_TX_OVR Mask */
 #define MXC_V_SPI_INT_EN_TX_OVR_DIS                    ((uint32_t)0x0) /**< INT_EN_TX_OVR_DIS Value */
 #define MXC_S_SPI_INT_EN_TX_OVR_DIS                    (MXC_V_SPI_INT_EN_TX_OVR_DIS << MXC_F_SPI_INT_EN_TX_OVR_POS) /**< INT_EN_TX_OVR_DIS Setting */
 #define MXC_V_SPI_INT_EN_TX_OVR_EN                     ((uint32_t)0x1) /**< INT_EN_TX_OVR_EN Value */
 #define MXC_S_SPI_INT_EN_TX_OVR_EN                     (MXC_V_SPI_INT_EN_TX_OVR_EN << MXC_F_SPI_INT_EN_TX_OVR_POS) /**< INT_EN_TX_OVR_EN Setting */

 #define MXC_F_SPI_INT_EN_TX_UND_POS                    (13) /**< INT_EN_TX_UND Position */
 #define MXC_F_SPI_INT_EN_TX_UND                        ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_TX_UND_POS)) /**< INT_EN_TX_UND Mask */
 #define MXC_V_SPI_INT_EN_TX_UND_DIS                    ((uint32_t)0x0) /**< INT_EN_TX_UND_DIS Value */
 #define MXC_S_SPI_INT_EN_TX_UND_DIS                    (MXC_V_SPI_INT_EN_TX_UND_DIS << MXC_F_SPI_INT_EN_TX_UND_POS) /**< INT_EN_TX_UND_DIS Setting */
 #define MXC_V_SPI_INT_EN_TX_UND_EN                     ((uint32_t)0x1) /**< INT_EN_TX_UND_EN Value */
 #define MXC_S_SPI_INT_EN_TX_UND_EN                     (MXC_V_SPI_INT_EN_TX_UND_EN << MXC_F_SPI_INT_EN_TX_UND_POS) /**< INT_EN_TX_UND_EN Setting */

 #define MXC_F_SPI_INT_EN_RX_OVR_POS                    (14) /**< INT_EN_RX_OVR Position */
 #define MXC_F_SPI_INT_EN_RX_OVR                        ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_RX_OVR_POS)) /**< INT_EN_RX_OVR Mask */
 #define MXC_V_SPI_INT_EN_RX_OVR_DIS                    ((uint32_t)0x0) /**< INT_EN_RX_OVR_DIS Value */
 #define MXC_S_SPI_INT_EN_RX_OVR_DIS                    (MXC_V_SPI_INT_EN_RX_OVR_DIS << MXC_F_SPI_INT_EN_RX_OVR_POS) /**< INT_EN_RX_OVR_DIS Setting */
 #define MXC_V_SPI_INT_EN_RX_OVR_EN                     ((uint32_t)0x1) /**< INT_EN_RX_OVR_EN Value */
 #define MXC_S_SPI_INT_EN_RX_OVR_EN                     (MXC_V_SPI_INT_EN_RX_OVR_EN << MXC_F_SPI_INT_EN_RX_OVR_POS) /**< INT_EN_RX_OVR_EN Setting */

 #define MXC_F_SPI_INT_EN_RX_UND_POS                    (15) /**< INT_EN_RX_UND Position */
 #define MXC_F_SPI_INT_EN_RX_UND                        ((uint32_t)(0x1 << MXC_F_SPI_INT_EN_RX_UND_POS)) /**< INT_EN_RX_UND Mask */
 #define MXC_V_SPI_INT_EN_RX_UND_DIS                    ((uint32_t)0x0) /**< INT_EN_RX_UND_DIS Value */
 #define MXC_S_SPI_INT_EN_RX_UND_DIS                    (MXC_V_SPI_INT_EN_RX_UND_DIS << MXC_F_SPI_INT_EN_RX_UND_POS) /**< INT_EN_RX_UND_DIS Setting */
 #define MXC_V_SPI_INT_EN_RX_UND_EN                     ((uint32_t)0x1) /**< INT_EN_RX_UND_EN Value */
 #define MXC_S_SPI_INT_EN_RX_UND_EN                     (MXC_V_SPI_INT_EN_RX_UND_EN << MXC_F_SPI_INT_EN_RX_UND_POS) /**< INT_EN_RX_UND_EN Setting */

/**@} end of group INT_EN_Register */

/**
 * @ingroup  spi_registers
 * @defgroup WAKE_FL_Register
 * @brief    Register for wake up flags. All bits in this register are write 1 to clear.
 * @{
 */
 #define MXC_F_SPI_WAKE_FL_TX_THRESH_POS                (0) /**< WAKE_FL_TX_THRESH Position */
 #define MXC_F_SPI_WAKE_FL_TX_THRESH                    ((uint32_t)(0x1 << MXC_F_SPI_WAKE_FL_TX_THRESH_POS)) /**< WAKE_FL_TX_THRESH Mask */
 #define MXC_V_SPI_WAKE_FL_TX_THRESH_CLEAR              ((uint32_t)0x1) /**< WAKE_FL_TX_THRESH_CLEAR Value */
 #define MXC_S_SPI_WAKE_FL_TX_THRESH_CLEAR              (MXC_V_SPI_WAKE_FL_TX_THRESH_CLEAR << MXC_F_SPI_WAKE_FL_TX_THRESH_POS) /**< WAKE_FL_TX_THRESH_CLEAR Setting */

 #define MXC_F_SPI_WAKE_FL_TX_EMPTY_POS                 (1) /**< WAKE_FL_TX_EMPTY Position */
 #define MXC_F_SPI_WAKE_FL_TX_EMPTY                     ((uint32_t)(0x1 << MXC_F_SPI_WAKE_FL_TX_EMPTY_POS)) /**< WAKE_FL_TX_EMPTY Mask */
 #define MXC_V_SPI_WAKE_FL_TX_EMPTY_CLEAR               ((uint32_t)0x1) /**< WAKE_FL_TX_EMPTY_CLEAR Value */
 #define MXC_S_SPI_WAKE_FL_TX_EMPTY_CLEAR               (MXC_V_SPI_WAKE_FL_TX_EMPTY_CLEAR << MXC_F_SPI_WAKE_FL_TX_EMPTY_POS) /**< WAKE_FL_TX_EMPTY_CLEAR Setting */

 #define MXC_F_SPI_WAKE_FL_RX_THRESH_POS                (2) /**< WAKE_FL_RX_THRESH Position */
 #define MXC_F_SPI_WAKE_FL_RX_THRESH                    ((uint32_t)(0x1 << MXC_F_SPI_WAKE_FL_RX_THRESH_POS)) /**< WAKE_FL_RX_THRESH Mask */
 #define MXC_V_SPI_WAKE_FL_RX_THRESH_CLEAR              ((uint32_t)0x1) /**< WAKE_FL_RX_THRESH_CLEAR Value */
 #define MXC_S_SPI_WAKE_FL_RX_THRESH_CLEAR              (MXC_V_SPI_WAKE_FL_RX_THRESH_CLEAR << MXC_F_SPI_WAKE_FL_RX_THRESH_POS) /**< WAKE_FL_RX_THRESH_CLEAR Setting */

 #define MXC_F_SPI_WAKE_FL_RX_FULL_POS                  (3) /**< WAKE_FL_RX_FULL Position */
 #define MXC_F_SPI_WAKE_FL_RX_FULL                      ((uint32_t)(0x1 << MXC_F_SPI_WAKE_FL_RX_FULL_POS)) /**< WAKE_FL_RX_FULL Mask */
 #define MXC_V_SPI_WAKE_FL_RX_FULL_CLEAR                ((uint32_t)0x1) /**< WAKE_FL_RX_FULL_CLEAR Value */
 #define MXC_S_SPI_WAKE_FL_RX_FULL_CLEAR                (MXC_V_SPI_WAKE_FL_RX_FULL_CLEAR << MXC_F_SPI_WAKE_FL_RX_FULL_POS) /**< WAKE_FL_RX_FULL_CLEAR Setting */

/**@} end of group WAKE_FL_Register */

/**
 * @ingroup  spi_registers
 * @defgroup WAKE_EN_Register
 * @brief    Register for wake up enable.
 * @{
 */
 #define MXC_F_SPI_WAKE_EN_TX_THRESH_POS                (0) /**< WAKE_EN_TX_THRESH Position */
 #define MXC_F_SPI_WAKE_EN_TX_THRESH                    ((uint32_t)(0x1 << MXC_F_SPI_WAKE_EN_TX_THRESH_POS)) /**< WAKE_EN_TX_THRESH Mask */
 #define MXC_V_SPI_WAKE_EN_TX_THRESH_DIS                ((uint32_t)0x0) /**< WAKE_EN_TX_THRESH_DIS Value */
 #define MXC_S_SPI_WAKE_EN_TX_THRESH_DIS                (MXC_V_SPI_WAKE_EN_TX_THRESH_DIS << MXC_F_SPI_WAKE_EN_TX_THRESH_POS) /**< WAKE_EN_TX_THRESH_DIS Setting */
 #define MXC_V_SPI_WAKE_EN_TX_THRESH_EN                 ((uint32_t)0x1) /**< WAKE_EN_TX_THRESH_EN Value */
 #define MXC_S_SPI_WAKE_EN_TX_THRESH_EN                 (MXC_V_SPI_WAKE_EN_TX_THRESH_EN << MXC_F_SPI_WAKE_EN_TX_THRESH_POS) /**< WAKE_EN_TX_THRESH_EN Setting */

 #define MXC_F_SPI_WAKE_EN_TX_EMPTY_POS                 (1) /**< WAKE_EN_TX_EMPTY Position */
 #define MXC_F_SPI_WAKE_EN_TX_EMPTY                     ((uint32_t)(0x1 << MXC_F_SPI_WAKE_EN_TX_EMPTY_POS)) /**< WAKE_EN_TX_EMPTY Mask */
 #define MXC_V_SPI_WAKE_EN_TX_EMPTY_DIS                 ((uint32_t)0x0) /**< WAKE_EN_TX_EMPTY_DIS Value */
 #define MXC_S_SPI_WAKE_EN_TX_EMPTY_DIS                 (MXC_V_SPI_WAKE_EN_TX_EMPTY_DIS << MXC_F_SPI_WAKE_EN_TX_EMPTY_POS) /**< WAKE_EN_TX_EMPTY_DIS Setting */
 #define MXC_V_SPI_WAKE_EN_TX_EMPTY_EN                  ((uint32_t)0x1) /**< WAKE_EN_TX_EMPTY_EN Value */
 #define MXC_S_SPI_WAKE_EN_TX_EMPTY_EN                  (MXC_V_SPI_WAKE_EN_TX_EMPTY_EN << MXC_F_SPI_WAKE_EN_TX_EMPTY_POS) /**< WAKE_EN_TX_EMPTY_EN Setting */

 #define MXC_F_SPI_WAKE_EN_RX_THRESH_POS                (2) /**< WAKE_EN_RX_THRESH Position */
 #define MXC_F_SPI_WAKE_EN_RX_THRESH                    ((uint32_t)(0x1 << MXC_F_SPI_WAKE_EN_RX_THRESH_POS)) /**< WAKE_EN_RX_THRESH Mask */
 #define MXC_V_SPI_WAKE_EN_RX_THRESH_DIS                ((uint32_t)0x0) /**< WAKE_EN_RX_THRESH_DIS Value */
 #define MXC_S_SPI_WAKE_EN_RX_THRESH_DIS                (MXC_V_SPI_WAKE_EN_RX_THRESH_DIS << MXC_F_SPI_WAKE_EN_RX_THRESH_POS) /**< WAKE_EN_RX_THRESH_DIS Setting */
 #define MXC_V_SPI_WAKE_EN_RX_THRESH_EN                 ((uint32_t)0x1) /**< WAKE_EN_RX_THRESH_EN Value */
 #define MXC_S_SPI_WAKE_EN_RX_THRESH_EN                 (MXC_V_SPI_WAKE_EN_RX_THRESH_EN << MXC_F_SPI_WAKE_EN_RX_THRESH_POS) /**< WAKE_EN_RX_THRESH_EN Setting */

 #define MXC_F_SPI_WAKE_EN_RX_FULL_POS                  (3) /**< WAKE_EN_RX_FULL Position */
 #define MXC_F_SPI_WAKE_EN_RX_FULL                      ((uint32_t)(0x1 << MXC_F_SPI_WAKE_EN_RX_FULL_POS)) /**< WAKE_EN_RX_FULL Mask */
 #define MXC_V_SPI_WAKE_EN_RX_FULL_DIS                  ((uint32_t)0x0) /**< WAKE_EN_RX_FULL_DIS Value */
 #define MXC_S_SPI_WAKE_EN_RX_FULL_DIS                  (MXC_V_SPI_WAKE_EN_RX_FULL_DIS << MXC_F_SPI_WAKE_EN_RX_FULL_POS) /**< WAKE_EN_RX_FULL_DIS Setting */
 #define MXC_V_SPI_WAKE_EN_RX_FULL_EN                   ((uint32_t)0x1) /**< WAKE_EN_RX_FULL_EN Value */
 #define MXC_S_SPI_WAKE_EN_RX_FULL_EN                   (MXC_V_SPI_WAKE_EN_RX_FULL_EN << MXC_F_SPI_WAKE_EN_RX_FULL_POS) /**< WAKE_EN_RX_FULL_EN Setting */

/**@} end of group WAKE_EN_Register */

/**
 * @ingroup  spi_registers
 * @defgroup STAT_Register
 * @brief    SPI Status register.
 * @{
 */
 #define MXC_F_SPI_STAT_BUSY_POS                        (0) /**< STAT_BUSY Position */
 #define MXC_F_SPI_STAT_BUSY                            ((uint32_t)(0x1 << MXC_F_SPI_STAT_BUSY_POS)) /**< STAT_BUSY Mask */
 #define MXC_V_SPI_STAT_BUSY_NOT                        ((uint32_t)0x0) /**< STAT_BUSY_NOT Value */
 #define MXC_S_SPI_STAT_BUSY_NOT                        (MXC_V_SPI_STAT_BUSY_NOT << MXC_F_SPI_STAT_BUSY_POS) /**< STAT_BUSY_NOT Setting */
 #define MXC_V_SPI_STAT_BUSY_ACTIVE                     ((uint32_t)0x1) /**< STAT_BUSY_ACTIVE Value */
 #define MXC_S_SPI_STAT_BUSY_ACTIVE                     (MXC_V_SPI_STAT_BUSY_ACTIVE << MXC_F_SPI_STAT_BUSY_POS) /**< STAT_BUSY_ACTIVE Setting */

/**@} end of group STAT_Register */

#ifdef __cplusplus
}
#endif

#endif /* _SPI_REGS_H_ */
  