/* libmsg_gp_types.h : general purpose types */

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

/* Fabrice Ducos, CGTD Icare 2005, fabrice.ducos@icare.univ-lille1.fr*/

/** @file libmsg_gp_types.h
 *  a partial implementation
 *  of Design Specification Standard Volume F: Data Types and Encoding Rules (from EUMETSAT)
 */

#ifndef _LIBMSG_GP_TYPES_H
#define _LIBMSG_GP_TYPES_H

#include <stdint.h>

#define PACKED __attribute((packed)) /* FIXME: gcc specific */

/* BOOLEAN */
typedef uint8_t BOOLEAN_BYTE;
typedef uint16_t BOOLEAN_SHORT;
typedef uint32_t BOOLEAN_LONG;

/* ENUMERATED */
typedef uint8_t ENUMERATED_BYTE;
typedef uint16_t ENUMERATED_SHORT;
typedef uint32_t ENUMERATED_LONG;
#define ENUMERATED_SIZE(nbits) struct { uint8_t val[nbits/8]; } PACKED

/* UNSIGNED INTEGER */
typedef uint8_t UNSIGNED_BYTE;
typedef uint16_t UNSIGNED_SHORT;
typedef uint32_t UNSIGNED;
typedef uint64_t UNSIGNED_DOUBLE;
#define UNSIGNED_SIZE(nbits) struct { uint8_t val[nbits/8]; } PACKED

/* SIGNED INTEGER */
typedef int8_t INTEGER_BYTE;
typedef int16_t INTEGER_SHORT;
typedef int32_t INTEGER;
typedef int64_t INTEGER_DOUBLE;
#define INTEGER_SIZE(nbits) struct { uint8_t val[nbits/8]; } PACKED /* usage of unsigned type uint8_t is NOT buggy, we could use any byte-long type (important here is type length) */

/* REAL */
typedef float REAL;
typedef double REAL_DOUBLE;

/* BIT_STRING */
#define BITSTRING_SIZE(nbits) struct { unsigned val : (nbits); } PACKED

/* OCTETSTRING */
// VARIABLE_OCTETSTRING_SIZE(maxlength)

/* CHARACTER_STRING */
// VARIABLE_CHARACTERSTRING_SIZE(maxlength)
// CHARACTERSTRING_SIZE(pfc)

/* ABSOLUTE TIME */
typedef struct _TIME_CDS_SHORT {
  uint8_t val[6];
} TIME_CDS_SHORT;
#define TIME_CUC_SIZE(coarse, fine) struct { uint8_t val[coarse+fine]; } PACKED
typedef struct _TIME_CDS_EXPANDED {
  uint8_t val[10];
} PACKED TIME_CDS_EXPANDED;
typedef struct _TIME_GENERALIZED {
  uint8_t val[15];
} PACKED TIME_GENERALIZED;
typedef struct _TIME_GENERALIZED_EXPANDED {
  uint8_t val[25];
} PACKED TIME_GENERALIZED_EXPANDED;

typedef ENUMERATED_SHORT GP_SC_ID; /* spacecraft identification */

#endif
