/* read_xrit.c */
#include <stdio.h>
/* 
   Copyright (C) 2005, 2010 Fabrice Ducos <fabrice.ducos@icare.univ-lille1.fr>
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

#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <alloca.h>
#include <limits.h>
#include "xrit_swap.h"
#include "ccsds_time.h"
#include "read_xrit.h"
#include "xrit_met8_channel.h"

#define UNIX_EPOCH 4383 /* Unix Epoch (1-Jan-1970) in days since TAI (Temps Atomique International) date reference (1-Jan-1958) */
#define TAI2UNIX(day) ( (day) - UNIX_EPOCH ) /* converts TAI day into UNIX day (4383 days between 1-Jan-1958 and 1-Jan-1970) */
#define UNIX_SECONDS(unix_day, ms_of_day) ( 86400*(unix_day) + (ms_of_day) / 1000) /* converts UNIX day into UNIX seconds since 1-Jan-1970 */

#define ROW_QUALITY_RECORD_SIZE ( sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t) + 3*sizeof(uint8_t) )

#define LAST_SEGMENT 7
#define LAST_HRV_SEGMENT 23

static char header_buf[65535];

static const unsigned short _xa[] = { 0, 1, 2, 3, 5, 6, 7, 8 };
static const unsigned short _xb[] = { 65472, 16368, 4092, 1023, 65472, 16368, 4092, 1023 };
static const unsigned short _xc[] = { 6, 4, 2, 0, 6, 4, 2, 0 };
#define XRIT_DECODE(_buffer, _i) ((((_buffer[_xa[_i]] << 8) \
                                 + (_buffer[_xa[_i]+1])) & _xb[_i]) \
                                 >> _xc[_i])

static int _xrit_resolution = XRIT_LOW_RES;
static int _xrit_orientation = XRIT_SOUTH_AT_TOP;
static unsigned int _xrit_nrows = XRIT_NROWS;
static unsigned int _xrit_ncolumns = XRIT_NCOLUMNS;
static int _last_segment = LAST_SEGMENT;
static int _segment_size = XRIT_SEGMENTSIZE;

int get_xrit_resolution() {
  return _xrit_resolution;
}

void set_xrit_resolution(int resolution) {
  assert(resolution == XRIT_LOW_RES || resolution == XRIT_HIGH_RES);

  _xrit_resolution = resolution;

  if (resolution == XRIT_LOW_RES) {
    _xrit_nrows = XRIT_NROWS;
    _xrit_ncolumns = XRIT_NCOLUMNS;
    _last_segment = LAST_SEGMENT;
    _segment_size = XRIT_SEGMENTSIZE;
  }
  else if (resolution == XRIT_HIGH_RES) {
    _xrit_nrows = XRIT_HRV_NROWS;
    _xrit_ncolumns = XRIT_HRV_NCOLUMNS;
    _last_segment = LAST_HRV_SEGMENT;
    _segment_size = XRIT_HRV_SEGMENTSIZE;
  }
  else {
    fprintf(stderr, "read_xrit: invalid call to set_xrit_resolution (bad argument), resolution unchanged\n");
  }
}

int get_xrit_orientation() {
  return _xrit_orientation;
}

void set_xrit_orientation(int orientation) {
  _xrit_orientation = orientation;
#ifdef DEBUG_READ_XRIT
  fprintf(stderr,"read_xrit (DEBUG): set_xrit_orientation: _xrit_orientation = %d\n", _xrit_orientation);
#endif
}

void xrit_orientation(int orientation) {
#ifdef DEBUG_READ_XRIT
  fprintf(stderr,"read_xrit (DEBUG): call to xrit_orientation obsolete, replace it by set_xrit_orientation (same prototype)\n", _xrit_orientation);
#endif
  set_xrit_orientation(orientation);
}


int read_xrit(const char *filename, uint16_t *counts,
	      int *nsegments, xrit_header_t **xrit_header,
	      xrit_rows_infos_t *rows_infos, MSG_Prologue *prologue, MSG_Epilogue *epilogue) {

  FILE *stream;
  size_t nread;
  uint8_t header_type;
  uint16_t header_record_len;
  xrit_header_t *current_header, *previous_header;

  long start_of_segment = 0;
  uint8_t segment_type = 0;
  uint32_t total_header_len = 16; /* default is the primary record length (16), should be updated by the primary record itself */
  uint64_t data_field_len = 0;
  long current_pos;
  uint16_t nrows;
  int isegment = 0;
  unsigned char *segmentbuf; /* data segment buffer */
  unsigned char *pdata; /* data pointer inside segmentbuf */
  const char *input_shortname;
  int ichannel;

  assert(filename);

#ifdef DEBUG_READ_XRIT
  fprintf(stderr,"read_xrit (DEBUG): _xrit_orientation = %d\n", _xrit_orientation);
#endif

  stream = fopen(filename, "rb");
  if (! stream) {
    perror(filename);
    return -1;
  }

  input_shortname = strrchr(filename, '/');
  if (input_shortname) input_shortname++;
  else input_shortname = filename;

  ichannel = xrit_met8_channel(input_shortname);
  if (ichannel < 0) {
    fprintf(stderr, "read_xrit: no channel has been recognized in the MSG file name.\n");
    fprintf(stderr, "read_xrit: xrit2raw expects to find one of the following strings in the filename:  ");
    fprintf(stderr, "VIS006 VIS008 IR016 IR039 WV062 WV073 IR087 IR097 IR108 IR108 IR120 IR134 HRV\n");
    return -2;
  }
  if (ichannel == XRIT_CHAN_HRV) { 
    set_xrit_resolution(XRIT_HIGH_RES);
  }
  else {
    set_xrit_resolution(XRIT_LOW_RES);
  }

  previous_header = NULL;
  current_header = NULL;
  if (xrit_header) *xrit_header = NULL;

  if (nsegments) {
    if (*nsegments < 1 || *nsegments > _last_segment + 1) *nsegments = 0;
  }
  
  segmentbuf = (unsigned char *) malloc(XRIT_SEGMENTSIZE*sizeof(uint16_t));
  if (segmentbuf == NULL) {
#ifdef DEBUG_READ_XRIT
    perror("read_xrit (DEBUG): malloc");
#endif
    return -1;
  }
  
  
  if (rows_infos) {
    int irow;
    memset(rows_infos, 0, sizeof(*rows_infos));
    rows_infos->time = (time_t *) malloc(_xrit_nrows*sizeof(time_t));
    rows_infos->validity = (uint8_t *) malloc(_xrit_nrows*sizeof(uint8_t));
    rows_infos->radiometric_validity = (uint8_t *) malloc(_xrit_nrows*sizeof(uint8_t));
    rows_infos->geometric_validity = (uint8_t *) malloc(_xrit_nrows*sizeof(uint8_t));

    for (irow = 0 ; irow < _xrit_nrows ; irow++) {
      rows_infos->time[irow] = (time_t) 0;
      rows_infos->validity[irow] = 0;
      rows_infos->radiometric_validity[irow] = 0;
      rows_infos->geometric_validity[irow] = 0;
    }
  }
  
  while(1) { /* loop over segments */
    nrows = 0;
    
    while ((current_pos = ftell(stream)) - start_of_segment < (int) total_header_len) { /* loop over record headers in the current segment */
      
      /* the three first bytes describe the header type (1 byte) and length (2 bytes) */
      nread = fread(header_buf, 3, 1, stream);
      if (feof(stream)) {
	goto reached_EOF; /* reached END_OF_FILE, leaves all the loops */
      }
      assert((nread == 1) && ! ferror(stream));
      header_type = header_buf[0];
      header_record_len = ntohs(*(uint16_t *) &header_buf[1]);
      
      nread = fread(&header_buf[3], header_record_len - 3, 1, stream);
      assert((nread == 1) && ! ferror(stream) && ! feof(stream));
      
      if (xrit_header) {
	current_header = (xrit_header_t *) malloc(sizeof(xrit_header_t));
	if (previous_header) previous_header->next = current_header;
	else *xrit_header = current_header;
	current_header->header_type = header_type;
	current_header->header_record_len = header_record_len;
	current_header->next = NULL;
	previous_header = current_header;
      }
      
      switch (header_type) {
      case XRIT_PRIM_HEADER : {
	segment_type = header_buf[3];
	total_header_len = ntohl(*(uint32_t *) &header_buf[4]);
	data_field_len = ntohll(*(uint64_t *) &header_buf[8])/CHAR_BIT;
	if (current_header) {
	  current_header->content.primary.segment_type = segment_type;
	  current_header->content.primary.total_header_len = total_header_len;
	  current_header->content.primary.data_field_len = data_field_len;
	}
	break;
      }
	
      case XRIT_IMAGE_STRUCT : {
	nrows = ntohs(*(uint16_t *) &header_buf[6]);
	if (current_header) {
	  current_header->content.image_struct.nbits_per_pixel = header_buf[3];
	  current_header->content.image_struct.ncolumns = ntohs(*(uint16_t *) &header_buf[4]);
	  current_header->content.image_struct.nrows = nrows;
	  current_header->content.image_struct.compression_flag = header_buf[8];
	}
	break;
      }
	
      case XRIT_IMAGE_NAVIGATION : {
	if (current_header) {
	  strncpy(current_header->content.image_navig.projection_name, &header_buf[3], sizeof(current_header->content.image_navig.projection_name) - 1);
	  current_header->content.image_navig.projection_name[sizeof(current_header->content.image_navig.projection_name) - 1] = '\0';
	  current_header->content.image_navig.column_scaling_factor = ntohl(*(int32_t *) &header_buf[34]);
	  current_header->content.image_navig.row_scaling_factor = ntohl(*(int32_t *) &header_buf[38]);
	  current_header->content.image_navig.column_offset = ntohl(*(int32_t *) &header_buf[42]);
	  current_header->content.image_navig.row_offset = ntohl(*(int32_t *) &header_buf[46]);
	}
	break;
      }
	
      case XRIT_IMAGE_DATA_FUNC : { /* not used */
	break;
      }
	
      case XRIT_ANNOTATION : { /* not used */
	break;
      }
	
      case XRIT_TIME_STAMP : {
	
	if (current_header) {
	  /* p_field = header_buf[3]; */ /* preambule field (won't be used here) */
	  current_header->content.time_stamp.tai_day = ntohs(*(uint16_t *) &header_buf[4]);

	  /* tai_day must be AFTER beginning of UNIX Epoch (1-Jan-1970) */
	  if (current_header->content.time_stamp.tai_day <= UNIX_EPOCH) {
#ifdef XRIT_DEBUG
	    fprintf(stderr, "read_xrit (DEBUG): warning: suspicious day in %s's time_stamp: %d\n", filename, current_header->content.time_stamp.tai_day);
#endif
	  }
	  current_header->content.time_stamp.unix_day = TAI2UNIX(current_header->content.time_stamp.tai_day);
	  current_header->content.time_stamp.ms_of_day = ntohl(*(uint32_t *) &header_buf[6]);
	  current_header->content.time_stamp.unix_seconds = UNIX_SECONDS(current_header->content.time_stamp.unix_day, current_header->content.time_stamp.ms_of_day);
	}
	break;
      }

      case XRIT_ANCILLARY_TEXT : { /* 21-Feb-2005 : NOT TESTED (I've got no file with ancillary text records) */
	if (current_header) {
	  assert(current_header->header_record_len - 3 < (int) sizeof(header_buf));
	  current_header->content.ancillary_text.ancillary_text = &header_buf[3];
	  header_buf[current_header->header_record_len - 3] = '\0';
	}
	break;
      }

      case XRIT_KEY_HEADER : { /* not used */
	break;
      }

      case XRIT_SEGMENT_IDENTIFICATION : {
	if (current_header) {
	  current_header->content.segment_id.GP_SC_ID = ntohs(*(uint16_t *) &header_buf[3]);
	  current_header->content.segment_id.spectral_channel_id = header_buf[5];
	  current_header->content.segment_id.segment_sequence_number = ntohs(*(uint16_t *) &header_buf[6]);
	  current_header->content.segment_id.planned_start_segment_seq_number = ntohs(*(uint16_t *) &header_buf[8]);
	  current_header->content.segment_id.planned_end_segment_seq_number = ntohs(*(uint16_t *) &header_buf[10]);
	  current_header->content.segment_id.data_field_representation = header_buf[12];
	}
	break;
      }

      case XRIT_IMAGE_SEGMENT_ROW_QUALITY : {
	
	register uint16_t irow; /* row index */
	
	for (irow = 0 ; irow < nrows ; irow++) {
	  uint32_t row_number_in_grid;
	  uint16_t tai_day;
	  uint16_t unix_day;
	  uint32_t ms_of_day;

	  row_number_in_grid = ntohl(*(uint32_t *) &header_buf[3 + irow*ROW_QUALITY_RECORD_SIZE]) - 1;
	  if (_xrit_orientation == XRIT_NORTH_AT_TOP) row_number_in_grid = _xrit_nrows - 1 - row_number_in_grid;
	  assert(row_number_in_grid >= 0 && row_number_in_grid < _xrit_nrows);
	  tai_day = ntohs(*(uint16_t *) &header_buf[7 + irow*ROW_QUALITY_RECORD_SIZE]);
	  if (tai_day == 0) continue; // if tai_day == 0, the current row is in the outer space
	  // tai_day must be AFTER beginning of UNIX Epoch (1-Jan-1970)
	  if (tai_day <= UNIX_EPOCH) {
#ifdef XRIT_DEBUG
	    fprintf(stderr, "read_xrit (DEBUG): warning: suspicious day in %s [row %d]: %d\n", filename, irow, tai_day);
#endif
	    continue;
	  }
	  if (rows_infos) {
	    unix_day = TAI2UNIX(tai_day);
	    ms_of_day = ntohl(*(uint32_t *) &header_buf[9 + irow*ROW_QUALITY_RECORD_SIZE]);
	    rows_infos->time[row_number_in_grid] = UNIX_SECONDS(unix_day, ms_of_day);
	    rows_infos->validity[row_number_in_grid] = header_buf[13 + irow*ROW_QUALITY_RECORD_SIZE];
	    rows_infos->radiometric_validity[row_number_in_grid] = header_buf[3 + irow*ROW_QUALITY_RECORD_SIZE]; /* CHECKME */
	    rows_infos->geometric_validity[row_number_in_grid] = header_buf[3 + irow*ROW_QUALITY_RECORD_SIZE]; /* CHECKME */
	  }
	}
	
	break;
      }
	
      default : {
#ifdef DEBUG_READ_XRIT
	printf("read_xrit (DEBUG): unexpected record type: %d (byte %ld, length %hd)\n", header_type, current_pos, header_record_len);
#endif
	return -1;
	break;
      }
    
      } /* switch (header_type) */
      
    } /* while (ftell(stream) < total_header_len ) */

    if (counts != NULL && segment_type == XRIT_IMAGE_DATA_FILE) { /* reads data, decodes them and puts them into 'counts' */
      register uint32_t i,j;
      uint32_t isegment_image;
      assert(data_field_len < XRIT_SEGMENTSIZE*sizeof(uint16_t));
      nread = fread(segmentbuf, data_field_len, 1, stream);
      if (nread < 1) {
	if (! ferror(stream) && feof(stream)) break;
#ifdef DEBUG_READ_XRIT
	perror("read_xrit (DEBUG): fread(segmentbuf)");
#endif
	return -1;
      }

      isegment_image = ntohl(*(uint32_t *) &header_buf[3 + 0*ROW_QUALITY_RECORD_SIZE]) - 1;
      isegment_image /= XRIT_NROWS_PER_SEG;
      
      if (nsegments && *nsegments) {
	/* the user asked for only one segment*/
	if (*nsegments - 1 != isegment_image) goto skip_segment;
	if (_xrit_orientation == XRIT_SOUTH_AT_TOP) isegment_image = 0;
	else isegment_image = _last_segment;
      }
      
      for (i = 0 ; i < _segment_size/8 ; i++) {
	pdata = &segmentbuf[10*i];
	for (j = 0 ; j < 8 ; j++) {
	  assert(isegment_image*_segment_size+8*i+j < _xrit_nrows*_xrit_ncolumns*sizeof(uint16_t));
	  if (_xrit_orientation == XRIT_NORTH_AT_TOP) {
	    counts[_xrit_nrows*_xrit_ncolumns - 1 - (isegment_image*_segment_size+8*i+j)] = XRIT_DECODE(pdata, j); /* this way, Earth image is not reversed (upper side = north and left side = west) */
	  }
	  else if (_xrit_orientation == XRIT_SOUTH_AT_TOP) {
	    counts[isegment_image*_segment_size+8*i+j] = XRIT_DECODE(pdata, j);  /* this way, Earth image will be reversed (upper side = south and left side = east)  */
	  }
	  else {
	    fprintf(stderr, "read_xrit: fatal error (internal bug): unexpected value for _xrit_orientation: %d\n", _xrit_orientation);
	    abort();
	  }
	}
      }
    skip_segment:
      ;
    }
    else if (prologue != NULL && segment_type == XRIT_CYCLE_PROLOGUE) {
      assert(data_field_len == sizeof(MSG_Prologue));
      memset(prologue, 0, sizeof(MSG_Prologue));
      nread = fread(prologue, data_field_len, 1, stream);
      if (nread < 1) {
	if (! ferror(stream) && feof(stream)) break;
#ifdef DEBUG_READ_XRIT
	perror("read_xrit (DEBUG): fread(prologue)");
#endif
	return -1;
      }
    }
    else if (epilogue != NULL && segment_type == XRIT_CYCLE_EPILOGUE) {
      goto jumps_over;
    }
    else {/* jumps over the data */
    jumps_over: 
      if (fseek(stream, data_field_len, SEEK_CUR) != 0) {
	perror("read_xrit: fseek");
	fprintf(stderr,"read_xrit: ftell(stream) = %ld\n", ftell(stream));
	exit (EXIT_FAILURE);
      }
    }

    isegment++;
    start_of_segment = ftell(stream);
    
  } /* while (1) */
  
  if (segmentbuf != NULL) {
    free(segmentbuf);
  }

 reached_EOF:
  if (nsegments && *nsegments >=1 && *nsegments <= (_last_segment + 1)) return 1;
  return isegment;

}

void free_xrit_header(xrit_header_t *xrit_header) {

  xrit_header_t *current_header = xrit_header, *next_header;
  
  assert(xrit_header);

  do {
    next_header = current_header->next;
   
    free(current_header);
  } while ((current_header = next_header)); /* yes, this is being an assignment, not a comparison between current_header and next_header
					     * (double parenthesis are there to avoid compiler warnings)
					     */

}

void print_xrit_header(xrit_header_t *xrit_header) {

  xrit_header_t *current_header = xrit_header, *next_header;
  int nsegments = 0;

  assert(xrit_header);

  do {
    next_header = current_header->next;
    switch (current_header->header_type) {
    case XRIT_PRIM_HEADER : { printf("primary header (segment %d)\n", ++nsegments); break; }
    case XRIT_IMAGE_STRUCT : { printf("image struct\n"); break; }
    case XRIT_IMAGE_NAVIGATION : { printf("image navig\n"); break; }
    case XRIT_IMAGE_DATA_FUNC : { printf("image data func\n"); break; }
    case XRIT_ANNOTATION : { printf("annotation\n"); break; }
    case XRIT_TIME_STAMP : { printf("time stamp\n"); break; }
    case XRIT_ANCILLARY_TEXT : { printf("ancillary text\n"); break; }
    case XRIT_KEY_HEADER : { printf("key header\n"); break; }
    case XRIT_SEGMENT_IDENTIFICATION : { printf("segment identification\n"); break; }
    case XRIT_IMAGE_SEGMENT_ROW_QUALITY : { printf("image segment row quality\n"); break; }
    default : { printf("unknown\n"); break; }
    }
  } while ((current_header = next_header)); /* yes, this is being an assignment, not a comparison between current_header and next_header
					     * (double parenthesis are there to avoid compiler warnings)
					     */

}

