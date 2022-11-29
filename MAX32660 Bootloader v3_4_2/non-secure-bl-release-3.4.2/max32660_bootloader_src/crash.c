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

#include "utils.h"
void HardFault_Handler(void) {
    uart_puts("HardFault_Handler! (bootloader)\n");
    __asm(
        " TST LR, #4\n"
        " ITE EQ \n"
        " MRSEQ R0, MSP \n"
        " MRSNE R0, PSP \n"
        " B FaultISR_C \n");

}

void FaultISR_C(uint32_t *hardfault_args)
{
#if 0
    unsigned int stacked_r0;
    unsigned int stacked_r1;
    unsigned int stacked_r2;
    unsigned int stacked_r3;
    unsigned int stacked_r12;
    unsigned int stacked_lr;
    unsigned int stacked_pc;
    unsigned int stacked_psr;

    volatile unsigned char mmsr;
    volatile unsigned char bfsr;
    volatile unsigned short ufsr;
    volatile unsigned int hfsr;

    stacked_r0 = ((unsigned int) hardfault_args[0]);
    stacked_r1 = ((unsigned int) hardfault_args[1]);
    stacked_r2 = ((unsigned int) hardfault_args[2]);
    stacked_r3 = ((unsigned int) hardfault_args[3]);
    stacked_r12 = ((unsigned int) hardfault_args[4]);
    stacked_lr = ((unsigned int) hardfault_args[5]);
    stacked_pc = ((unsigned int) hardfault_args[6]);
    stacked_psr = ((unsigned int) hardfault_args[7]);

    printf("\n\n[Hard fault handler - all numbers in hex]\n");
    printf("R0 = 0x%08x\n", stacked_r0);
    printf("R1 = 0x%08x\n", stacked_r1);
    printf("R2 = 0x%08x\n", stacked_r2);
    printf("R3 = 0x%08x\n", stacked_r3);
    printf("R12 = 0x%08x\n", stacked_r12);
    printf("LR [R14] = 0x%08x  subroutine call return address\n", stacked_lr);
    printf("PC [R15] = 0x%08x  program counter address\n", stacked_pc);
    printf("PSR = 0x%08x\n", stacked_psr);
    printf("MMAR = 0x%08x  memory manage fault address\n", (*((volatile unsigned int *) (0xE000ED34))));
    printf("BFAR = 0x%08x  bus fault address\n", (*((volatile unsigned int *) (0xE000ED38))));


    /***********************************************************************************************
     * Memory Management Fault Status Register: (0xE000ED28)
     * Bit    Name         Description
     *  7     MMARVALID    MMAR is valid (0x40)
     *  4     MSTKERR      Stacking error (0x10)
     *  3     MUNSTKERR    Unstacking error (0x8)
     *  1     DACCVIOL     Data access violation (0x2)
     *  0     IACCVIOL     Instruction access violation (0x1)
     ***********************************************************************************************/
    mmsr = (*((volatile unsigned char *) (0xE000ED28)));
    printf("MMSR = 0x%02x  ", mmsr);
    if (mmsr & 0x40)
        printf("MMARVALID: MMAR is valid  ");
    if (mmsr & 0x10)
        printf("MSTKERR: Stacking error\n");
    else if (mmsr & 0x8)
        printf("MUNSTKERR: Unstacking error\n");
    else if (mmsr & 0x2)
        printf("DACCVIOL: Data access violation\n");
    else if (mmsr & 0x1)
        printf("IACCVIOL: Instruction access violation\n");
    else
        printf("\n");

    /***********************************************************************************************
     * Bus Fault Status Register: (0xE000ED28)
     * Bit    Name         Description
     *  7     BFARVALID    BFAR is valid (0x80)
     *  4     STKERR       Stacking error (0x10)
     *  3     UNSTKERR     Unstacking error (0x8)
     *  2     IMPREISERR  Imprecise data access violation (0x4)
     *  1     PRECISERR    Precise data access violation (0x2)
     *  0     IBUSERR      Instruction access violation (0x1)
     ***********************************************************************************************/
    bfsr = (*((volatile unsigned char *) (0xE000ED29)));
    printf("BFSR = 0x%02x  ", bfsr);
    if (bfsr & 0x80)
        printf("BFARVALID: BFAR is valid  ");
    if (bfsr & 0x10)
        printf("STKERR: Stacking error\n");
    else if (bfsr & 0x8)
        printf("UNSTKERR: Unstacking error\n");
    else if (bfsr & 0x4)
        printf("IMPREISERR: Imprecise data access violation\n");
    else if (bfsr & 0x2)
        printf("PRECISERR: Precise data access violation\n");
    else if (bfsr & 0x1)
        printf("IBUSERR: Instruction access violation\n");
    else
        printf("\n");

    /***********************************************************************************************
     * Usage Fault Status Register: (0xE000ED2A)
     * Bit    Name         Description
     *  9     DIVBYZERO    Divide by zero will take place (0x200)
     *  8     UNALIGNED    Unaligned access will take place (0x100)
     *  3     NOCP         Attempt to execute a coprocessor instruction (0x8)
     *  2     INVPC        Attempt to do exception with bad value (0x4)
     *  1     INVSTATE     Attempt to switch to invalid state (0x2)
     *  0     UNDEFINSTR   Attempt to execute an undefined instruction (0x1)
     ***********************************************************************************************/
    ufsr = (*((volatile unsigned short *) (0xE000ED2A)));
    printf("UFSR = 0x%04x  ", ufsr);
    if (ufsr & 0x200)
        printf("DIVBYZERO: Divide by zero will take place\n");
    else if (ufsr & 0x100)
        printf("UNALIGNED: Unaligned access will take place\n");
    else if (ufsr & 0x8)
        printf("NOCP: Attempt to execute a coprocessor instruction\n");
    else if (ufsr & 0x4)
        printf("INVPC: Attempt to do exception with bad value\n");
    else if (ufsr & 0x2)
        printf("INVSTATE: Attempt to switch to invalid state\n");
    else if (ufsr & 0x1)
        printf("UNDEFINSTR: Attempt to execute an undefined instruction\n");
    else
        printf("\n");

    /***********************************************************************************************
     * Usage Fault Status Register: (0xE000ED2A)
     * Bit    Name         Description
     * 31     DEBUGEVT     Hard fault caused by debug event (0x8000_0000)
     * 30     FORCED       Hard fault caused by bus/memory management/usage fault (0x4000_0000)
     *  1     VECTBL       Hard fault caused by failed vector fetch (0x1)
     ***********************************************************************************************/
    hfsr = (*((volatile unsigned int *) (0xE000ED2C)));
    printf("HFSR = 0x%08x  ", hfsr);
    if (hfsr & 0x80000000)
        printf("DEBUGEVT: Hard fault caused by debug event\n");
    else if (hfsr & 0x40000000)
        printf("FORCED: Hard fault caused by bus/memory management/usage fault\n");
    else if (hfsr & 0x1)
        printf("VECTBL: Hard fault caused by failed vector fetch\n");
    else
        printf("\n");

    //printf ("AFSR = 0x%08x\n", (*((volatile unsigned int *)(0xE000ED3C))));
    //printf ("SCB_SHCSR = %x\n", SCB->SHCSR);

#endif
    set_boot_mode_and_reset();
    while (1) ;  /* Spin so we can use a debugger to anlayzer the situation */
}
