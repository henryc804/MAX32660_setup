/**
 * @file    nbbfc_regs.h
 * @brief   Registers, Bit Masks and Bit Positions for the NBBFC Peripheral Module.
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
 * $Date: 2017-07-06 14:12:44 -0500 (Thu, 06 Jul 2017) $
 * $Revision: 28830 $
 *
 *************************************************************************** */

#ifndef _NBBFC_REGS_H_
#define _NBBFC_REGS_H_

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
 * @ingroup     nbbfc
 * @defgroup    nbbfc_registers Registers
 * @brief       Registers, Bit Masks and Bit Positions for the NBBFC Peripheral Module.
 * @description Non Battery-Backed Function Control.
 */

/**
 * @ingroup nbbfc_registers
 * Structure type to access the NBBFC Registers.
 */
typedef struct {
    __IO uint32_t reg0;                 /**< <tt>\b 0x00:<\tt> NBBFC REG0 Register */
    __IO uint32_t reg1;                 /**< <tt>\b 0x04:<\tt> NBBFC REG1 Register */
    __IO uint32_t reg2;                 /**< <tt>\b 0x08:<\tt> NBBFC REG2 Register */
    __IO uint32_t reg3;                 /**< <tt>\b 0x0C:<\tt> NBBFC REG3 Register */
} mxc_nbbfc_regs_t;

/* Register offsets for module NBBFC */
/**
 * @ingroup    nbbfc_registers
 * @defgroup   NBBFC_Register_Offsets Register Offsets
 * @brief      NBBFC Peripheral Register Offsets from the NBBFC Base Peripheral Address. 
 * @{
 */
 #define MXC_R_NBBFC_REG0                   ((uint32_t)0x00000000UL) /**< Offset from NBBFC Base Address: <tt> 0x0x000 */ 
 #define MXC_R_NBBFC_REG1                   ((uint32_t)0x00000004UL) /**< Offset from NBBFC Base Address: <tt> 0x0x004 */ 
 #define MXC_R_NBBFC_REG2                   ((uint32_t)0x00000008UL) /**< Offset from NBBFC Base Address: <tt> 0x0x008 */ 
 #define MXC_R_NBBFC_REG3                   ((uint32_t)0x0000000CUL) /**< Offset from NBBFC Base Address: <tt> 0x0x00C */ 
/**@} end of group nbbfc_registers */

/**
 * @ingroup  nbbfc_registers
 * @defgroup REG0_Register
 * @brief    Register 0.
 * @{
 */
 #define MXC_F_NBBFC_REG0_HYP_RDS_GC_SEL_POS            (0) /**< REG0_HYP_RDS_GC_SEL Position */
 #define MXC_F_NBBFC_REG0_HYP_RDS_GC_SEL                ((uint32_t)(0x3F << MXC_F_NBBFC_REG0_HYP_RDS_GC_SEL_POS)) /**< REG0_HYP_RDS_GC_SEL Mask */

 #define MXC_F_NBBFC_REG0_HYP_RDS_GC_SET_POS            (6) /**< REG0_HYP_RDS_GC_SET Position */
 #define MXC_F_NBBFC_REG0_HYP_RDS_GC_SET                ((uint32_t)(0x1 << MXC_F_NBBFC_REG0_HYP_RDS_GC_SET_POS)) /**< REG0_HYP_RDS_GC_SET Mask */

 #define MXC_F_NBBFC_REG0_HYP_CLKGEN_DLY_POS            (8) /**< REG0_HYP_CLKGEN_DLY Position */
 #define MXC_F_NBBFC_REG0_HYP_CLKGEN_DLY                ((uint32_t)(0x3F << MXC_F_NBBFC_REG0_HYP_CLKGEN_DLY_POS)) /**< REG0_HYP_CLKGEN_DLY Mask */

 #define MXC_F_NBBFC_REG0_USB_EXT_CORECLK_SEL_POS       (16) /**< REG0_USB_EXT_CORECLK_SEL Position */
 #define MXC_F_NBBFC_REG0_USB_EXT_CORECLK_SEL           ((uint32_t)(0x1 << MXC_F_NBBFC_REG0_USB_EXT_CORECLK_SEL_POS)) /**< REG0_USB_EXT_CORECLK_SEL Mask */

 #define MXC_F_NBBFC_REG0_I2C0_SDA_DGEN_POS             (20) /**< REG0_I2C0_SDA_DGEN Position */
 #define MXC_F_NBBFC_REG0_I2C0_SDA_DGEN                 ((uint32_t)(0x1 << MXC_F_NBBFC_REG0_I2C0_SDA_DGEN_POS)) /**< REG0_I2C0_SDA_DGEN Mask */

 #define MXC_F_NBBFC_REG0_I2C0_SCL_DGEN_POS             (21) /**< REG0_I2C0_SCL_DGEN Position */
 #define MXC_F_NBBFC_REG0_I2C0_SCL_DGEN                 ((uint32_t)(0x1 << MXC_F_NBBFC_REG0_I2C0_SCL_DGEN_POS)) /**< REG0_I2C0_SCL_DGEN Mask */

 #define MXC_F_NBBFC_REG0_I2C1_SDA_DGEN_POS             (22) /**< REG0_I2C1_SDA_DGEN Position */
 #define MXC_F_NBBFC_REG0_I2C1_SDA_DGEN                 ((uint32_t)(0x1 << MXC_F_NBBFC_REG0_I2C1_SDA_DGEN_POS)) /**< REG0_I2C1_SDA_DGEN Mask */

 #define MXC_F_NBBFC_REG0_I2C1_SCL_DGEN_POS             (23) /**< REG0_I2C1_SCL_DGEN Position */
 #define MXC_F_NBBFC_REG0_I2C1_SCL_DGEN                 ((uint32_t)(0x1 << MXC_F_NBBFC_REG0_I2C1_SCL_DGEN_POS)) /**< REG0_I2C1_SCL_DGEN Mask */

/**@} end of group REG0_Register */

/**
 * @ingroup  nbbfc_registers
 * @defgroup REG1_Register
 * @brief    Register 1.
 * @{
 */
 #define MXC_F_NBBFC_REG1_HIRC96M_AC_EN_POS             (0) /**< REG1_HIRC96M_AC_EN Position */
 #define MXC_F_NBBFC_REG1_HIRC96M_AC_EN                 ((uint32_t)(0x1 << MXC_F_NBBFC_REG1_HIRC96M_AC_EN_POS)) /**< REG1_HIRC96M_AC_EN Mask */

 #define MXC_F_NBBFC_REG1_HIRC96M_AC_RUN_POS            (1) /**< REG1_HIRC96M_AC_RUN Position */
 #define MXC_F_NBBFC_REG1_HIRC96M_AC_RUN                ((uint32_t)(0x1 << MXC_F_NBBFC_REG1_HIRC96M_AC_RUN_POS)) /**< REG1_HIRC96M_AC_RUN Mask */

 #define MXC_F_NBBFC_REG1_HIRC96M_AC_LOADTRM_POS        (2) /**< REG1_HIRC96M_AC_LOADTRM Position */
 #define MXC_F_NBBFC_REG1_HIRC96M_AC_LOADTRM            ((uint32_t)(0x1 << MXC_F_NBBFC_REG1_HIRC96M_AC_LOADTRM_POS)) /**< REG1_HIRC96M_AC_LOADTRM Mask */

 #define MXC_F_NBBFC_REG1_HIRC96M_AC_GAIN_INV_POS       (3) /**< REG1_HIRC96M_AC_GAIN_INV Position */
 #define MXC_F_NBBFC_REG1_HIRC96M_AC_GAIN_INV           ((uint32_t)(0x1 << MXC_F_NBBFC_REG1_HIRC96M_AC_GAIN_INV_POS)) /**< REG1_HIRC96M_AC_GAIN_INV Mask */

 #define MXC_F_NBBFC_REG1_HIRC96M_AC_ATOMIC_POS         (4) /**< REG1_HIRC96M_AC_ATOMIC Position */
 #define MXC_F_NBBFC_REG1_HIRC96M_AC_ATOMIC             ((uint32_t)(0x1 << MXC_F_NBBFC_REG1_HIRC96M_AC_ATOMIC_POS)) /**< REG1_HIRC96M_AC_ATOMIC Mask */

 #define MXC_F_NBBFC_REG1_HIRC96M_AC_MU_POS             (8) /**< REG1_HIRC96M_AC_MU Position */
 #define MXC_F_NBBFC_REG1_HIRC96M_AC_MU                 ((uint32_t)(0xFFF << MXC_F_NBBFC_REG1_HIRC96M_AC_MU_POS)) /**< REG1_HIRC96M_AC_MU Mask */

/**@} end of group REG1_Register */

/**
 * @ingroup  nbbfc_registers
 * @defgroup REG2_Register
 * @brief    Register 2.
 * @{
 */
 #define MXC_F_NBBFC_REG2_HIRC96M_AC_INITTRM_POS        (0) /**< REG2_HIRC96M_AC_INITTRM Position */
 #define MXC_F_NBBFC_REG2_HIRC96M_AC_INITTRM            ((uint32_t)(0x1FF << MXC_F_NBBFC_REG2_HIRC96M_AC_INITTRM_POS)) /**< REG2_HIRC96M_AC_INITTRM Mask */

 #define MXC_F_NBBFC_REG2_HIRC96M_AC_MINTRM_POS         (10) /**< REG2_HIRC96M_AC_MINTRM Position */
 #define MXC_F_NBBFC_REG2_HIRC96M_AC_MINTRM             ((uint32_t)(0x1FF << MXC_F_NBBFC_REG2_HIRC96M_AC_MINTRM_POS)) /**< REG2_HIRC96M_AC_MINTRM Mask */

 #define MXC_F_NBBFC_REG2_HIRC96M_AC_MAXTRM_POS         (20) /**< REG2_HIRC96M_AC_MAXTRM Position */
 #define MXC_F_NBBFC_REG2_HIRC96M_AC_MAXTRM             ((uint32_t)(0x1FF << MXC_F_NBBFC_REG2_HIRC96M_AC_MAXTRM_POS)) /**< REG2_HIRC96M_AC_MAXTRM Mask */

/**@} end of group REG2_Register */

/**
 * @ingroup  nbbfc_registers
 * @defgroup REG3_Register
 * @brief    Register 3.
 * @{
 */
 #define MXC_F_NBBFC_REG3_HIRC96M_AC_DONECNT_POS        (0) /**< REG3_HIRC96M_AC_DONECNT Position */
 #define MXC_F_NBBFC_REG3_HIRC96M_AC_DONECNT            ((uint32_t)(0xFF << MXC_F_NBBFC_REG3_HIRC96M_AC_DONECNT_POS)) /**< REG3_HIRC96M_AC_DONECNT Mask */

/**@} end of group REG3_Register */

#ifdef __cplusplus
}
#endif

#endif /* _NBBFC_REGS_H_ */
  