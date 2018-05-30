/* xrit_met8_channel.h */

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

#ifndef XRIT_MET8_CHANNEL_H
#define XRIT_MET8_CHANNEL_H

/** @file xrit_met8_channel.h */

enum {
  XRIT_CHAN_VIS006 = 0,
  XRIT_CHAN_VIS008 = 1,
  XRIT_CHAN_IR016  = 2,
  XRIT_CHAN_IR039  = 3,
  XRIT_CHAN_WV062  = 4,
  XRIT_CHAN_WV073  = 5,
  XRIT_CHAN_IR087  = 6,
  XRIT_CHAN_IR097  = 7,
  XRIT_CHAN_IR108  = 8,
  XRIT_CHAN_IR120  = 9,
  XRIT_CHAN_IR134  = 10,
  XRIT_CHAN_HRV    = 11
};


/**
 * @param filename a filename expected to contain a SEVIRI channel name
 * @return a positive channel index on success, or -1 on failure
 * <ul>
 * <li>VIS006 returns 0
 * <li>VIS008 returns 1
 * <li>IR016  (or IR_016) returns 2
 * <li>IR039  (or IR_039) returns 3
 * <li>WV062  (or WV_062) returns 4
 * <li>WV073  (or WV_073) returns 5
 * <li>IR087  (or IR_087) returns 6
 * <li>IR097  (or IR_097) returns 7
 * <li>IR108  (or IR_108) returns 8
 * <li>IR120  (or IR_120) returns 9
 * <li>IR134  (or IR_134) returns 10
 * <li>HRV returns 11
 * </ul>
 */
int xrit_met8_channel(const char *filename);

#endif
