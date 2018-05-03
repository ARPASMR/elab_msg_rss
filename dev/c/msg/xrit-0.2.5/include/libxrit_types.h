/* libxrit_types.h */

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

#ifndef _LIBXRIT_TYPES_H
#define _LIBXRIT_TYPES_H

#include <stdint.h>
#include <time.h>
#include "ccsds_time.h"

typedef enum _xrit_file_types_t {
  XRIT_IMAGE_DATA_FILE = 0,
  XRIT_GTS_MESSAGE = 1,
  XRIT_ALPHANUM_TEXT_FILE = 2,
  XRIT_ENCRYPT_KEY_MESSAGE = 3,
  /* 4..127 reserved for future global usage */
  /* 128..255 for mission specific use */
  XRIT_CYCLE_PROLOGUE = 128, /* MSG specific */
  XRIT_CYCLE_EPILOGUE = 129  /* MSG specific */
} xrit_file_types_t;

typedef enum _xrit_header_types_t {
  XRIT_PRIM_HEADER = 0,
  XRIT_IMAGE_STRUCT = 1,
  XRIT_IMAGE_NAVIGATION = 2,
  XRIT_IMAGE_DATA_FUNC = 3,
  XRIT_ANNOTATION = 4,
  XRIT_TIME_STAMP = 5,
  XRIT_ANCILLARY_TEXT = 6,
  XRIT_KEY_HEADER = 7,
  /* 8..127 reserved for future global usage */
  /* 128..255 for mission specific use */
  XRIT_SEGMENT_IDENTIFICATION = 128,
  XRIT_IMAGE_SEGMENT_ROW_QUALITY = 129
} xrit_header_types_t;

typedef struct _xrit_prim_header_t {
  uint8_t segment_type;
  uint32_t total_header_len;
  uint64_t data_field_len;
} xrit_prim_header_t;

typedef struct _xrit_image_struct_t {
  uint8_t nbits_per_pixel;
  uint16_t ncolumns;
  uint16_t nrows;
  uint8_t compression_flag;
} xrit_image_struct_t;

typedef struct _xrit_image_navig_t {
  char projection_name[33];
  int32_t column_scaling_factor;
  int32_t row_scaling_factor;
  int32_t column_offset;
  int32_t row_offset;
} xrit_image_navig_t;

typedef struct _xrit_data_func_t {
  char *data_def_block; /* up to 65532 bytes */
} xrit_data_func_t;

typedef struct _xrit_annotation_t {
  char *annotation_text; /* up to 64 bytes */
} xrit_annotation_t;

typedef struct _xrit_time_stamp_t {
  // char time_stamp[7]; /* data type is in fact 'CCSDS time' (7 bytes); obsolete; replaced by the following fields */
  /* uint8_t p_field; */ /* preambule field from CCSDS Recommandation for Time Formats (won't be used here) */
  uint16_t tai_day; /* count of days since 1-Jan-1958 (TAI Epoch, Temps Atomique International) */
  uint16_t unix_day; /* count of days since 1-Jan-1970 (Unix Epoch) */
  uint32_t ms_of_day; /* ms in the current day */
  time_t unix_seconds; /* count of seconds since 1-Jan-1970 (Unix Epoch) */
} xrit_time_stamp_t;

typedef struct _xrit_ancillary_text_t {
  char *ancillary_text; /* up to 65532 bytes */
} xrit_ancillary_text_t;

typedef struct _xrit_key_header_t {
  char *key_header_info; /* mission specific, up to 65532 */
} xrit_key_header_t;

typedef struct _xrit_segment_id_t {
  uint16_t GP_SC_ID;
  uint8_t spectral_channel_id;
  uint16_t segment_sequence_number;
  uint16_t planned_start_segment_seq_number;
  uint16_t planned_end_segment_seq_number;
  uint8_t data_field_representation;
} xrit_segment_id_t;

#endif
