/* xrit_met8_channel.c */

/* 
   Copyright (C) 2005 Fabrice Ducos <fabrice.ducos@icare.univ-lille1.fr>
   This file is part of the LibXRIT Library.

   The LibXRIT Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The LibXRIT Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the LibXRIT Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xrit_met8_channel.h"

int xrit_met8_channel(const char *filename) {

  /* channel ordering is specified in MSG Level 1.5 Image Data Format Description 6.2 (p. 63 in 4 feb 2005 release) */

  if (strstr(filename, "VIS006")) return XRIT_CHAN_VIS006;
  else if (strstr(filename, "VIS008")) return XRIT_CHAN_VIS008;
  else if (strstr(filename, "IR016") || strstr(filename, "IR_016")) return XRIT_CHAN_IR016;
  else if (strstr(filename, "IR039") || strstr(filename, "IR_039")) return XRIT_CHAN_IR039;
  else if (strstr(filename, "WV062") || strstr(filename, "WV_062")) return XRIT_CHAN_WV062;
  else if (strstr(filename, "WV073") || strstr(filename, "WV_073")) return XRIT_CHAN_WV073;
  else if (strstr(filename, "IR087") || strstr(filename, "IR_087")) return XRIT_CHAN_IR087;
  else if (strstr(filename, "IR097") || strstr(filename, "IR_097")) return XRIT_CHAN_IR097;
  else if (strstr(filename, "IR108") || strstr(filename, "IR_108")) return XRIT_CHAN_IR108;
  else if (strstr(filename, "IR120") || strstr(filename, "IR_120")) return XRIT_CHAN_IR120;
  else if (strstr(filename, "IR134") || strstr(filename, "IR_134")) return XRIT_CHAN_IR134;
  else if (strstr(filename, "HRV")) return XRIT_CHAN_HRV;
  else return -1; /* default */

}
