#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#include "xrit_swap.h"
#include "libxrit.h"
#include "ccsds_time.h"

#define DEBUG_READ_XRIT_HEADER

#define UNIX_EPOCH 4383 /* Unix Epoch (1-Jan-1970) in days since TAI (Temps Atomique International) date reference (1-Jan-1958) */
#define TAI2UNIX(day) ( (day) - UNIX_EPOCH ) /* converts TAI day into UNIX day (4383 days between 1-Jan-1958 and 1-Jan-1970) */
#define UNIX_SECONDS(unix_day, ms_of_day) ( 86400*(unix_day) + (ms_of_day) / 1000) /* converts UNIX day into UNIX seconds since 1-Jan-1970 */

#define ROW_QUALITY_RECORD_SIZE ( sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t) + 3*sizeof(uint8_t) )

char header_buf[65535];

int main(int argc, char *argv[]) {

  char *filename;
  FILE *stream;
  uint8_t header_type;
  uint16_t header_record_len;
  size_t nread;

  uint8_t file_type_code;
  uint32_t total_header_len = 16; /* default is the primary record length (16), should be updated by the primary record itself */
  uint64_t data_field_len = 0;

  long start_of_segment = 0;
  long current_pos;

  /* filled by XRIT_IMAGE_STRUCT */
  uint8_t nbits_per_pixel;
  uint16_t ncolumns = 0;
  uint16_t nlines = 0;
  uint8_t compression_flag;

  if (argc != 2) {
    fprintf(stderr, "usage: %s [LRIT/HRIT file]\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  filename = argv[1];
  stream = fopen(filename, "r");
  assert(stream != NULL);

  while(1) { /* loop over segments */
    while ((current_pos = ftell(stream)) - start_of_segment < total_header_len) { /* loop over record headers in the current segment */

      /* the three first bytes describe the header type (1 byte) and length (2 bytes) */
      nread = fread(header_buf, 3, 1, stream);
      if (feof(stream)) {
#ifdef DEBUG_READ_XRIT_HEADER
	fprintf(stderr, "reached end of file at byte %ld\n", current_pos);
#endif
	goto reached_EOF; /* reached END_OF_FILE, leaves all the loops */
      }
      assert((nread == 1) && ! ferror(stream));
      header_type = header_buf[0];
      header_record_len = ntohs(*(uint16_t *) &header_buf[1]);
  
      nread = fread(&header_buf[3], header_record_len - 3, 1, stream);
      assert((nread == 1) && ! ferror(stream) && ! feof(stream));

      switch (header_type) {
      case XRIT_PRIM_HEADER : {
	file_type_code = header_buf[3];
	total_header_len = ntohl(*(uint32_t *) &header_buf[4]);
	data_field_len = ntohll(*(uint64_t *) &header_buf[8])/CHAR_BIT;
#ifdef DEBUG_READ_XRIT_HEADER
	printf("primary header record (byte %ld, length %hd)\n"
	       "  file_type_code: %hd\n"
	       "  total_header_len: %u\n"
	       "  data_field_len: %lld\n",
	       current_pos, header_record_len, (uint16_t) file_type_code, total_header_len, data_field_len);
#endif
	break;
      }

      case XRIT_IMAGE_STRUCT : {

	nbits_per_pixel = header_buf[3];
	ncolumns = ntohs(*(uint16_t *) &header_buf[4]);
	nlines = ntohs(*(uint16_t *) &header_buf[6]);
	compression_flag = header_buf[8];
#ifdef DEBUG_READ_XRIT_HEADER
	printf("image structure record (byte %ld, length %hd)\n", current_pos, header_record_len);
	printf("  nbits_per_pixel: %hu\n"
	       "  ncolumns: %hu\n"
	       "  nlines: %hu\n"
	       "  compression_flag: %hu\n",
	       (unsigned short) nbits_per_pixel, ncolumns, nlines, compression_flag);
#endif
	break;
      }

      case XRIT_IMAGE_NAVIGATION : {
	char projection_name[33];
	int32_t column_scaling_factor;
	int32_t line_scaling_factor;
	int32_t column_offset;
	int32_t line_offset;

	strncpy(projection_name, &header_buf[3], sizeof(projection_name) - 1);
	projection_name[sizeof(projection_name) - 1] = '\0';
	column_scaling_factor = ntohl(*(int32_t *) &header_buf[34]);
	line_scaling_factor = ntohl(*(int32_t *) &header_buf[38]);
	column_offset = ntohl(*(int32_t *) &header_buf[42]);
	line_offset = ntohl(*(int32_t *) &header_buf[46]);

#ifdef DEBUG_READ_XRIT_HEADER
	printf("image navigation record (byte %ld, length %hd)\n", current_pos, header_record_len);
	printf("  projection_name: %s\n"
	       "  column_scaling_factor: %ld\n"
	       "  line_scaling_factor: %ld\n"
	       "  column_offset: %ld\n"
	       "  line_offset: %ld\n",
	       projection_name, (long) column_scaling_factor, (long) line_scaling_factor,
	       (long) column_offset, (long) line_offset);
#endif
	break;
      }

      case XRIT_IMAGE_DATA_FUNC : {
#ifdef DEBUG_READ_XRIT_HEADER
	printf("image data function record (byte %ld, length %hd)\n", current_pos, header_record_len);
#endif
	break;
      }

      case XRIT_ANNOTATION : {
#ifdef DEBUG_READ_XRIT_HEADER
	char annotation_text[65];
	printf("annotation record (byte %ld, length %hd)\n", current_pos, header_record_len);
	assert(header_record_len - 3 < sizeof(annotation_text));
	strncpy(annotation_text, &header_buf[3], header_record_len - 3);
	annotation_text[header_record_len - 3] = '\0';
	printf("  annotation_text: %s\n", annotation_text);
#endif
	break;
      }

      case XRIT_TIME_STAMP : {
	/* uint8_t p_field; */ /* preambule field from CCSDS Recommandation for Time Formats (won't be used here) */
	uint16_t tai_day; /* count of days since 1-Jan-1958 (TAI Epoch, Temps Atomique International) */
	uint16_t unix_day; /* count of days since 1-Jan-1970 (Unix Epoch) */
	uint32_t ms_of_day; /* ms in the current day */
	time_t unix_seconds; /* count of seconds since 1-Jan-1970 (Unix Epoch) */

	/* p_field = header_buf[3]; */
	tai_day = ntohs(*(uint16_t *) &header_buf[4]);
	/*assert(tai_day > UNIX_EPOCH);*/
	unix_day = TAI2UNIX(tai_day);
	ms_of_day = ntohl(*(uint32_t *) &header_buf[6]);
	unix_seconds = UNIX_SECONDS(unix_day, ms_of_day);

#ifdef DEBUG_READ_XRIT_HEADER
	printf("image time stamp (byte %ld, length %hd)\n", current_pos, header_record_len);
	printf("  day %hu (since 1-Jan-1958, TAI Epoch) %hu (since 1-Jan-1970, Unix Epoch), ms_of_day: %lu\n", tai_day, unix_day, (unsigned long) ms_of_day);
	printf("  calendar date: %s\n", asctime(gmtime(&unix_seconds)));
#endif

	if (tai_day <= UNIX_EPOCH) { 
	  fprintf(stderr, "read_xrit_header: warning: suspicious date in the time stamp: %s\n", asctime(gmtime(&unix_seconds))); 
	} /* tai_day must be AFTER beginning of UNIX Epoch (1-Jan-1970) */

	break;
      }

      case XRIT_ANCILLARY_TEXT : { /* 21-Feb-2005 : NOT TESTED (I've got no file with ancillary text records) */
	char *ancillary_text;

	assert(header_record_len - 3 < sizeof(header_buf));
	ancillary_text = &header_buf[3];
	header_buf[header_record_len - 3] = '\0';
	printf("ancillary text record (byte %ld, length %hd)\n", current_pos, header_record_len);
	printf("  ancillary_text: %s\n", ancillary_text);
	break;
      }

      case XRIT_KEY_HEADER : {
	printf("key header record (byte %ld, length %hd)\n", current_pos, header_record_len);
	break;
      }

      case XRIT_SEGMENT_IDENTIFICATION : {
	uint16_t GP_SC_ID;
	uint8_t spectral_channel_id;
	uint16_t segment_sequence_number;
	uint16_t planned_start_segment_seq_number;
	uint16_t planned_end_segment_seq_number;
	uint8_t data_field_representation;

	GP_SC_ID = ntohs(*(uint16_t *) &header_buf[3]);
	spectral_channel_id = header_buf[5];
	segment_sequence_number = ntohs(*(uint16_t *) &header_buf[6]);
	planned_start_segment_seq_number = ntohs(*(uint16_t *) &header_buf[8]);
	planned_end_segment_seq_number = ntohs(*(uint16_t *) &header_buf[10]);
	data_field_representation = header_buf[12];

#ifdef DEBUG_READ_XRIT_HEADER
	printf("segment identification (byte %ld, length %hd)\n", current_pos, header_record_len);
	printf("  GP_SC_ID: %hd\n"
	       "  spectral_channel_id: %hu\n"
	       "  segment_sequence_number: %hu\n"
	       "  planned_start_segment_seq_number: %hu\n"
	       "  planned_end_segment_seq_number: %hu\n"
	       "  data_field_representation: %hu\n",
	       GP_SC_ID, spectral_channel_id, segment_sequence_number,
	       planned_start_segment_seq_number, planned_end_segment_seq_number,
	       data_field_representation);
#endif
	break;
      }

      case XRIT_IMAGE_SEGMENT_ROW_QUALITY : {
	uint16_t tai_day; /* count of days since 1-Jan-1958 (TAI Epoch, Temps Atomique International) */
	uint16_t unix_day; /* count of days since 1-Jan-1970 (Unix Epoch) */
	uint32_t ms_of_day; /* ms in the current day */
	time_t unix_seconds; /* count of seconds since 1-Jan-1970 (Unix Epoch) */
	uint32_t line_number_in_grid;

	uint8_t line_validity;
	uint8_t line_radiometric_validity;
	uint8_t line_geometric_validity;

	register uint16_t l; /* line index */

#ifdef DEBUG_READ_XRIT_HEADER
	printf("image segment line quality (byte %ld, length %hd)\n", current_pos, header_record_len);
#endif
	assert(nlines > 0);

	for (l = 0 ; l < nlines ; l++) {
	  line_number_in_grid = ntohl(*(uint32_t *) &header_buf[3 + l*ROW_QUALITY_RECORD_SIZE]);
	  tai_day = ntohs(*(uint16_t *) &header_buf[7 + l*ROW_QUALITY_RECORD_SIZE]);
	  if (tai_day == 0) continue; /* if tai_day == 0, the current line is in the outer space */
	  assert(tai_day > UNIX_EPOCH); /* tai_day must be AFTER beginning of UNIX Epoch (1-Jan-1970) */
	  unix_day = TAI2UNIX(tai_day);
	  ms_of_day = ntohl(*(uint32_t *) &header_buf[9 + l*ROW_QUALITY_RECORD_SIZE]);
	  unix_seconds = UNIX_SECONDS(unix_day, ms_of_day);
	  
	  line_validity = header_buf[13 + l*ROW_QUALITY_RECORD_SIZE];
	  line_radiometric_validity = header_buf[3 + l*ROW_QUALITY_RECORD_SIZE];
	  line_geometric_validity = header_buf[3 + l*ROW_QUALITY_RECORD_SIZE];
	  
	} /* for (l < nlines) */

	break;
      }

      default : {
#ifdef DEBUG_READ_XRIT_HEADER
	printf("unexpected record type: %d (byte %ld, length %hd)\n", header_type, current_pos, header_record_len);
#endif
	break;
      }
    
      } /* switch (header_type) */

#ifdef DEBUG_READ_XRIT_HEADER
      printf("\n");
#endif

    } /* while (ftell(stream) < total_header_len ) */

    /* here begin the real data */
    if (fseek(stream, data_field_len, SEEK_CUR) != 0) {
      perror("read_xrit_header: main.fseek");
      fprintf(stderr,"read_xrit_header: ftell(stream) = %ld\n", ftell(stream));
      exit (EXIT_FAILURE);
    }
    
    start_of_segment = ftell(stream);

  } /* while (1) */

 reached_EOF:
  return (EXIT_SUCCESS);

}
