/*******************************************************************************
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
 * $Date: 2016-03-23 15:12:09 -0700 (Wed, 23 Mar 2016) $
 * $Revision: 22072 $
 *
 ******************************************************************************/

MEMORY
{
    BOOTLOADER (rx) : ORIGIN = 0x00000000, LENGTH = 0x000C000   /* 49 kb for bootloader */
    MAINAPP (rwx) : ORIGIN = 0x0000C000, LENGTH = 0x00033FC0 /* Main application */
    BOOTMEM (rw) : ORIGIN = 0x0003FFC0, LENGTH = 0x00000040 /* 64 bytes of memory for bootloader */
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00018000 /* full-size SRAM, 98K */
}

_bl_start = ORIGIN(BOOTLOADER);
_bl_len = LENGTH(BOOTLOADER);

_app_isr = ORIGIN(MAINAPP);
_app_start = ORIGIN(MAINAPP);                   /* Start of application */
_app_len = LENGTH(MAINAPP);
_app_end = ORIGIN(MAINAPP) + LENGTH(MAINAPP);   /* End of application */

_boot_mem_start = ORIGIN(BOOTMEM);                  /* Start of non-volatile memory for bootloader */
_boot_mem_len = LENGTH(BOOTMEM);  					/* Length of non-volatile memory for bootloader */
_boot_mem_end = ORIGIN(BOOTMEM) + LENGTH(BOOTMEM);  /* End of non-volatile memory for bootloader */
_boot_mode = _boot_mem_start + 12; /* Tells the bootloader whether it should jump directly to app or flash a new app */

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.isr_vector))
        *(.text*)    /* program code */
        *(.rodata*)  /* read-only data: "const" */

        KEEP(*(.init))
        KEEP(*(.fini))

        /* C++ Exception handling */
        KEEP(*(.eh_frame*))
        _etext = .;
    } > BOOTLOADER

	. = ORIGIN(MAINAPP);

	.main_app :
	{
		_main_app = .;
		KEEP(*(.main_app))
		_emain_app = .;
	} > MAINAPP

	.boot_mem :
	{
		_boot_mem = .;
		KEEP(*(.bootmem))
		_eboot_mem = .;
	} > BOOTMEM

	.text :
	{
		_text = .;
		*(.text*)    /* program code */
        *(.rodata*)  /* read-only data: "const" variable */
        __exidx_start = .;
	} > BOOTSEC

    /* it's used for C++ exception handling      */
    /* we need to keep this to avoid overlapping */
    .ARM.exidx :
	{
        *(.ARM.exidx*)
        __exidx_end = .;
    } > BOOTSEC

    _etext = ALIGN(__exidx_end, 4);

    .data : AT(_etext)
    {
        _data = .;
        *(.data*)    /*read-write initialized data: initialized global variable*/
        _edata = ALIGN(., 4);
    } > SRAM
    __load_data = LOADADDR(.data);

    .bss :
    {
        . = ALIGN(4);
        _bss = .;
        *(.bss*)     /*read-write zero initialized data: uninitialzed global variable*/
        *(COMMON)
        _ebss = ALIGN(., 4);
    } > SRAM

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
     __StackTop = ORIGIN(SRAM) + LENGTH(SRAM);
     __StackLimit = __StackTop - SIZEOF(.stack_dummy);

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (COPY):
    {
        *(.stack*)
    } > SRAM

    .heap (COPY):
    {
        . = ALIGN(4);
        *(.heap*)
        __HeapLimit = ABSOLUTE(__StackLimit);
    } > SRAM

    PROVIDE(__stack = __StackTop);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= _ebss, "region RAM overflowed with stack")
}
