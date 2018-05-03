/* xrit_swap.c */

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

/* Fabrice Ducos, CGTD Icare, fabrice.ducos@icare.univ-lille1.fr */

#include <stdint.h>
#include "xrit_swap.h"

/* FIXME: BYTE_ORDER macro is GNU/Linux specific (endian.h)
 * Thanks for any more portable way to find the endianness of the host (at compile time of course)
 */

uint64_t ntohll(uint64_t n)
{
#if BYTE_ORDER == BIG_ENDIAN
	return n;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return (((uint64_t)ntohl(n)) << 32) | ntohl(n >> 32);
#else
	#error "BYTE_ORDER not defined on this system: should be 4321 (BIG_ENDIAN) or 1234 (LITTLE_ENDIAN); PDP_ENDIAN not supported for the time being"
#endif
}

uint64_t htonll(uint64_t n)
{
#if BYTE_ORDER == BIG_ENDIAN
	return n;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return (((uint64_t)htonl(n)) << 32) | htonl(n >> 32);
#else
	#error "BYTE_ORDER not defined on this system: should be 4321 (BIG_ENDIAN) or 1234 (LITTLE_ENDIAN); PDP_ENDIAN not supported for the time being"
#endif
}



float ntoh_float(float net_float) {

  uint32_t host_int32;

  host_int32 = ntohl(*((uint32_t *) &net_float));
  return *((float *) &host_int32);
}

float hton_float(float host_float) {

  uint32_t net_int32;

  net_int32 = htonl(*((uint32_t *) &host_float));
  return *((float *) &net_int32);
}

double ntoh_double(double net_double) {

  uint64_t host_int64;

  host_int64 = ntohll(*((uint64_t *) &net_double));
  return *((double *) &host_int64);
}

double hton_double(double host_double) {

  uint64_t net_int64;

  net_int64 = htonll(*((uint64_t *) &host_double));
  return *((double *) &net_int64);
}

/* uncomment this to compile the test code */
/*
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(void) {

  float f_pi = (float) M_PI;
  double d_pi = (double) M_PI;

  float f_ip = hton_float(f_pi);
  double d_ip = hton_double(d_pi);

  printf("htonll(0x0102030405060708) = %#16.16llx\n", htonll(0x0102030405060708LL));
  printf("ntohll(0x0102030405060708) = %#16.16llx\n", ntohll(0x0102030405060708LL));
  printf("hton_float(%g) = %g\n", f_pi, f_ip);
  printf("ntoh_float(%g) = %g\n", f_ip, ntoh_float(f_ip));
  printf("hton_double(%lg) = %lg\n", d_pi, d_ip);
  printf("ntoh_double(%lg) = %lg\n", d_ip, ntoh_double(d_ip));
  
  exit(EXIT_SUCCESS);

}
*/
