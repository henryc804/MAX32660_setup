 /**
 * @file    heap.c
 * @brief   System level setup help
 */
 
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
 * $Date: 2018-08-09 23:45:02 +0000 (Thu, 09 Aug 2018) $
 * $Revision: 36818 $
 *
 ******************************************************************************/
 
/* **** Includes **** */ 
#include <stdint.h>
#include <errno.h>
#include <unistd.h>

/**
 * @brief  sbrk
 * @detail Increase program data space
 * @detail Malloc and related functions depend on this
 */
 
/* **** declarations **** */
static char *heap_end = 0;
extern unsigned int __HeapBase;
extern unsigned int __HeapLimit;

/* **** functions **** */
caddr_t _sbrk(int incr)
{
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = (caddr_t)&__HeapBase;
    }
    prev_heap_end = heap_end;

    if ((unsigned int)(heap_end + incr) > (unsigned int)&__HeapLimit) {
        errno = ENOMEM;
        return  (caddr_t) -1;
    }

    heap_end += incr;

    return (caddr_t) prev_heap_end;
}

