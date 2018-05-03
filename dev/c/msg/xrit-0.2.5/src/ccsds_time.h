/* ccsds_time.h */

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

#ifndef _CCSDS_TIME_H
#define _CCSDS_TIME_H

/* a minimalist implementation of time_cds_t (day segmented time), since it
 * applies only to LRIT/HRIT implementation
 * the p_field (preambule) is not used here
 *
 * the day code epoch is 1-Jan-1958
 *
 */

typedef struct _ccsds_time_cds_t {
  uint16_t day;
  uint32_t ms_of_day;
} ccsds_time_cds_t;

#endif
