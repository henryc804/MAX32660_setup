/*******************************************************************************
 * Copyright (C) 2013 Maxim Integrated Products, Inc., All Rights Reserved.
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
 *     Description: ART Road Test Utility Functions
 *             $Id$
 *
 *******************************************************************************
 */

#include "mxc_config.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mem_utils.h"

/*****************************************************************/
void memcpy32(uint32_t * dst, uint32_t * src, unsigned int len)
{

  while (len) {
    *dst = *src;
    dst++;
    src++;
    len -= 4;
  }
}

/*****************************************************************/
int memcmp32(uint32_t * dst, uint32_t * src, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < (len >> 2); i++) {
    if (src == 0) {
      if (dst[i] != 0) {
        printf("verifying failed at %x (%x != %x)\n", (unsigned int)&dst[i], (unsigned int)dst[i], (unsigned int)src[i]);
        return 1;
      }
    } else {
      if (dst[i] != src[i]) {
        printf("verifying failed at %x (%x != %x)\n", (unsigned int)&dst[i], (unsigned int)dst[i], (unsigned int)src[i]);
        return 1;
      }
    }
  }

  return 0;
}
