/*******************************************************************************
* Copyright (C) 2014 Maxim Integrated Products, Inc., All Rights Reserved.
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
* $Id: delay.h 29282 2017-08-10 16:01:15Z michael.bayern $
*
********************************************************************************
*/

#ifndef _DELAY_H_
#define _DELAY_H_

/***** Definitions *****/
#define SEC(s)            (((unsigned long)s) * 1000000UL)
#define MSEC(ms)          (ms * 1000UL)
#define USEC(us)          (us)

/***** Function Prototypes *****/

/**
 * \brief   Blocks and delays for the specified number of microseconds.
 * \details Uses the SysTick to time the requested delay. If the SysTick is
 *          running, the current settings will be used. If the SysTick is not
 *          running, it will be started.
 * \param   us    microseconds to delay
 * \returns #E_NO_ERROR if no errors
 */
int delay(unsigned long us);

/**
 * \brief   Starts a non-blocking delay for the specified number of microseconds.
 * \details Uses the SysTick to time the requested delay. If the SysTick is
 *          running, the current settings will be used. If the SysTick is not
 *          running, it will be started. delay_handler() must be called from the
 *          SysTick interrupt service routine or at a rate greater than the
 *          SysTick overflow rate.
 * \param   us    microseconds to delay
 * \returns #E_NO_ERROR if no errors, #E_BUSY if currently servicing a request.
 */
int delay_start(unsigned long us);

/**
 * \brief   Returns the status of a non-blocking delay request
 * \pre     Start the non-blocking delay by calling delay_start().
 * \returns #E_BUSY until the requested delay time has expired.
 */
int delay_check(void);

/**
 * \brief   Stops a non-blocking delay request
 * \pre     Start the non-blocking delay by calling delay_start().
 */
void delay_stop(void);

/**
 * \brief   Processes the delay interrupt.
 * \details This function must be called from the SysTick interrupt service
 *          routine or at a rate greater than the SysTick overflow rate.
 */
void delay_handler(void);

#endif /* _DELAY_H_ */
