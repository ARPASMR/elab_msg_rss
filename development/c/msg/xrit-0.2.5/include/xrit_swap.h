/* xrit_swap.h */

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

/**
 * @file xrit_swap.h
 * an interface for the big/little endian conversion
 * of long long integers (64 bits) and floating point numbers
 */

/** Fabrice Ducos, CGTD Icare, fabrice.ducos@icare.univ-lille1.fr */

#ifndef XRIT_SWAP_H
#define XRIT_SWAP_H

#include <stdint.h>
#include <arpa/inet.h>

/* protection against possible local macro definitions */
/* TODO: should use local definitions instead of replacing them
 */
#undef ntohll
#undef htonll
#undef hton_float
#undef ntoh_float
#undef hton_double
#undef ntoh_double

/** 
 * @brief converts a 64-bits integer from its network representation (big endian) to its host representation. 
 */
uint64_t ntohll(uint64_t n);

/** @brief converts a 64-bits integer from its host representation to its network (big endian) representation. */
uint64_t htonll(uint64_t n);

/** @brief converts a simple precision floating point number (32 bits) from its network (big endian) representation
 * to its host representation.
 */
float ntoh_float(float net_float);

/** @brief converts a simple precision floating point number (32 bits) from its host representation
 * to its network (big endian) representation.
 */
float hton_float(float host_float);

/** @brief converts a double precision floating point number (64 bits) from its network (big endian) representation
 * to its host representation.
 */
double ntoh_double(double net_double);

/** @brief  converts a double precision floating point number (64 bits) from its host representation
 * to its network (big endian) representation.
 */
double hton_double(double host_double);

#endif
