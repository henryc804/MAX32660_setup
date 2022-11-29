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

/*
FUNCTION
<<itoa>>---integer to string

INDEX
	itoa

SYNOPSIS
	#include <stdlib.h>
	char *itoa(int <[value]>, char *<[str]>, int <[base]>);
	char *__itoa(int <[value]>, char *<[str]>, int <[base]>);

DESCRIPTION
<<itoa>> converts the integer <[value]> to a null-terminated string
using the specified base, which must be between 2 and 36, inclusive.
If <[base]> is 10, <[value]> is treated as signed and the string will be
prefixed with '-' if negative. For all other bases, <[value]> is treated as
unsigned. <[str]> should be an array long enough to contain the converted
value, which in the worst case is sizeof(int)*8+1 bytes.

RETURNS
A pointer to the string, <[str]>, or NULL if <[base]> is invalid.

PORTABILITY
<<itoa>> is non-ANSI.

No supporting OS subroutine calls are required.
*/

#include "utils.h"

/*
This function is from libc.
*/

char *
__utoa (unsigned value,
        char *str,
        int base);

char *
__itoa (int value,
        char *str,
        int base)
{
  unsigned uvalue;
  int i = 0;

  /* Check base is supported. */
  if ((base < 2) || (base > 36))
    {
      str[0] = '\0';
      return NULL;
    }

  /* Negative numbers are only supported for decimal.
   * Cast to unsigned to avoid overflow for maximum negative value.  */
  if ((base == 10) && (value < 0))
    {
      str[i++] = '-';
      uvalue = (unsigned)-value;
    }
  else
    uvalue = (unsigned)value;

  __utoa (uvalue, &str[i], base);
  return str;
}

char *
itoa (int value,
        char *str,
        int base)
{
  return __itoa (value, str, base);
}

char *custom_strstr(const char *searchee,
	const char *lookfor)
{

// defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  /* Less code size, but quadratic performance in the worst case.  */
  if (*searchee == 0)
    {
      if (*lookfor)
	return (char *) NULL;
      return (char *) searchee;
    }

  while (*searchee)
    {
      size_t i;
      i = 0;

      while (1)
	{
	  if (lookfor[i] == 0)
	    {
	      return (char *) searchee;
	    }

	  if (lookfor[i] != searchee[i])
	    {
	      break;
	    }
	  i++;
	}
      searchee++;
    }

  return (char *) NULL;
}

uint32_t get_str_len(const char *s)
{
	uint32_t len = 0;
	while(*s++ != '\0')
		len++;

	return len;
}
