/* read_xrit.h */

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

/* all this does the work, but is messy and poorly designed,
 * as it was thought of as a quick (not planned) and temporary substitute
 * for missing tools from Eumetsat (who didn't provide a free, open source
 *  C library and command line tools to read their HRIT data, just 
 * some closed-source, interactive graphic tools were available). 
 * I always think of rewriting from scratch with better design and 
 * implementation decisions but of course never find the time (nor
 * get paid !) for a new iteration
 */

/**
 * @file read_xrit.h
 */

#ifndef READ_XRIT_H
#define READ_XRIT_H

#include <stdint.h>
#include "libxrit_types.h"
#include "libmsg_sp_types.h"

#define XRIT_HEADERSIZE 16
#define XRIT_NROWS 3712
#define XRIT_NROWS_PER_SEG 464
#define XRIT_NCOLUMNS 3712
#define XRIT_HRV_NROWS 11136
#define XRIT_HRV_NCOLUMNS 5568
#define XRIT_MAXROW ( XRIT_NROWS - 1 )
#define XRIT_MAXCOLUMN ( XRIT_NCOLUMNS - 1 )
#define XRIT_SEGMENTSIZE XRIT_NROWS_PER_SEG*XRIT_NCOLUMNS
#define XRIT_HRV_SEGMENTSIZE XRIT_NROWS_PER_SEG*XRIT_HRV_NCOLUMNS

enum { XRIT_LOW_RES, XRIT_HIGH_RES };

/* structures for file header data */

typedef struct xrit_header_struct xrit_header_t;

struct xrit_header_struct {
  xrit_header_t *next;
  uint8_t header_type;
  uint16_t header_record_len;
  union {
    xrit_prim_header_t primary;
    xrit_image_struct_t image_struct;
    xrit_image_navig_t image_navig;
    xrit_data_func_t data_func;
    xrit_annotation_t annotation;
    xrit_time_stamp_t time_stamp;
    xrit_ancillary_text_t ancillary_text;
    xrit_key_header_t key_header;
    xrit_segment_id_t segment_id;
  } content;
};

typedef struct xrit_rows_infos_struct xrit_rows_infos_t;

struct xrit_rows_infos_struct {
  time_t *time;
  uint8_t *validity;
  uint8_t *radiometric_validity;
  uint8_t *geometric_validity;
};

enum { XRIT_NORTH_AT_TOP, XRIT_SOUTH_AT_TOP };

/* the name xrit_orientation is obsolete, should be replaced by set_xrit_orientation ;
 * is defined here to maintain backward compatibility
 * NOTE: will produce a warning in debug mode
 */
extern void xrit_orientation(int orientation);

/** @param orientation may be
 * <dl>
 * <dt>XRIT_NORTH_AT_TOP <dd>the image will be written from north to south
 * <dt>XRIT_SOUTH_AT_TOP <dd>it will be written from south to north
 * </dl>
 *
 * This function must be used BEFORE any call to read_xrit
 */
extern void set_xrit_orientation(int orientation);

/* returns the current orientation : XRIT_NORTH_AT_TOP or XRIT_SOUTH_AT_TOP */
extern int get_xrit_orientation();

/** read_xrit: 
 * @brief loads a buffer with an MSG/SEVIRI radiances XRIT file
 * reads a file containing MSG radiances 2-bytes counts and stores them in the 'counts' buffer.
 * Only the 'filename' argument is mandatory. The other ones may be set to NULL if they are not to be used.
 * If xrit_header is not set to NULL, it shall have to be freed by free_xrit_header
 * @param filename the name of the XRIT file to read
 * @param counts must be allocated by the user (with XRIT_NROWS*XRIT_NCOLUMNS*sizeof(uint16_t) bytes)
 * @param isegment a pointer to an int that will be filled with the index of segment to extract from the file (put it to NULL to
 * extract all the segments, or set isegment to 0. Otherwise, isegment must be filled with a value between 1 and 8).
 * @param xrit_header a linked list containing the headers for each segment: give the adress to an (xrit_header_t *) object
 * @param rows_infos an xrit_rows_infos_t pointer that must be allocated by the user (or NULL if not used)
 * @param prologue a pointer to a MSG_Prologue structure that will be filled if prologue is not NULL
 * @param epilogue is a pointer to a MSG_Epilogue structure that should be filled if epilogue is not NULL. For the time
 * being, the MSG_Epilogue structure is not implemented, so don't use this argument (set it to NULL).
 * @return the number of processed segments on success, a negative value on failure
 */
extern int read_xrit(const char *filename, uint16_t *counts, int *isegment, xrit_header_t **xrit_header,
	      xrit_rows_infos_t *rows_infos, MSG_Prologue *prologue, MSG_Epilogue *epilogue);

/** @brief frees the 'xrit_header' linked list returned by read_xrit
 *  @param xrit_header a pointer to an xrit_header structure filled by read_xrit (if the xrit_header
 *  argument of read_xrit has been used) 
 */
extern void free_xrit_header(xrit_header_t *xrit_header);

/** @brief prints each element of the 'xrit_header' linked list (useful for debugging purposes)
 *
 *  a debugging purpose function that prints infos about headers found in an XRIT file
 *  @param xrit_header pointer to an xrit_header structure filled by read_xrit (if the xrit_header argument of
 *  read_xrit has been used)
 */
extern void print_xrit_header(xrit_header_t *xrit_header);

/** @brief sets the resolution of the output (deprecated)
 *  @param resolution can be XRIT_LOW_RES or XRIT_HIGH_RES (for the HRV channel)
 *
 *  this function is deprecated and shouldn't be used anymore by any client code
 *  (can be removed from the interface in a future version)
 *  read_xrit determines the right resolution itself so there is no point to force it with
 *  a specific function
 */

extern void set_xrit_resolution(int resolution);

int get_xrit_resolution();

#endif
