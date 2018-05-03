/* xrit2raw.c */

/* 
   Copyright (C) 2005, 2011 Fabrice Ducos <fabrice.ducos@icare.univ-lille1.fr>
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
 * some closed-source, interactive graphic tools were available, at least
 * at the time when this tool was developed). 
 * Hopefully it exists now some well designed, official tools in replacement 
 * of this mess (that let people extract the data easily and non interactively)
 *
 * F. Ducos
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>  /* basename */
#include <fnmatch.h> /* fnmatch (pattern matching) */
#include <assert.h>
#include "xrit_swap.h"
#include "libxrit.h"
#include "math.h"

#define FILENAME_MAXLEN 256

#define OPT_PROLOGUE 1
#define OPT_UINT16 1
#define OPT_FLOAT  2
#define OPT_DOUBLE 3

#define OPT_INFO 1
#define OPT_VERBOSE 2

#define min(a,b) ( (a) <= (b) ? (a) : (b) )
#define max(a,b) ( (a) <= (b) ? (b) : (a) )

const char link_to_guide[] = "http://www.eumetsat.int/idcplg?"
  "IdcService=GET_FILE&dDocName=PDF_TEN_05105_MSG_IMG_DATA"
  "&RevisionSelectionMethod=LatestReleased";

const char *channels[] = {
  "VIS006",
  "VIS008",
  "IR016",
  "IR039",
  "WV062",
  "WV073",
  "IR087",
  "IR097",
  "IR108",
  "IR120",
  "IR134",
  "HRV"
};

void init_image(uint16_t *image, int nrows, int ncols) {
  int irow;
  int icol;
  int ipixel;
  
  assert(image != NULL);

  for (irow = 0 ; irow < nrows ; irow++) {
    for (icol = 0 ; icol < ncols ; icol++) {
      ipixel = irow*ncols + icol;
      image[ipixel] = 0;
    }
  }
}


/* sets the position of one part (upper or lower) of the HRV image in the final buffer */
/* south, north, east, west are positions from 0 to nrows-1 or ncols-1 */
void set_hrv_image_part(uint16_t *source, int nrows_src, int ncols_src, 
			uint16_t *target, int nrows_tgt, int ncols_tgt,
			int south, int north, int east, int west) {

  int irow_src;
  int icol_src;
  int irow_tgt;
  int icol_tgt;
  int ipixel_src;
  int ipixel_tgt;

  int first_row, first_col;
  int last_row, last_col;

  assert(source != NULL);
  assert(target != NULL);

  if (get_xrit_orientation() == XRIT_SOUTH_AT_TOP) {
    fprintf(stderr, "xrit2raw: in this experimental version, "
	    "the option -r is still buggy with the HRV channel and can't be used at that time, sorry.\n");
    exit (EXIT_FAILURE);
  }

  first_row = min(south, north);
  first_col = min(east, west);
  last_row = max(south, north);
  last_col = max(east, west);

  for (irow_src = first_row ; irow_src < last_row - first_row + 1; irow_src++) {
    for (icol_src = 0 ; icol_src < ncols_src ; icol_src++) {
      
      irow_tgt = irow_src;
      icol_tgt = icol_src + first_col;
      
      assert(0 <= irow_tgt && irow_tgt < nrows_tgt);
      assert(0 <= icol_tgt && icol_tgt < ncols_tgt);
      
      ipixel_src = irow_src*ncols_src + icol_src;
      ipixel_tgt = irow_tgt*ncols_tgt + icol_tgt;
      
      target[ipixel_tgt] = source[ipixel_src];
    }
  }

}

/* sets the position of the two parts (upper and lower) of an HRV image */
void set_hrv_image(uint16_t *source, uint16_t *target, MSG_Prologue *prologue) {
  int upper_south, upper_north, upper_east, upper_west;
  int lower_south, lower_north, lower_east, lower_west;
  int nrows_src, ncols_src;
  int nrows_tgt, ncols_tgt;

  assert(source != NULL);
  assert(target != NULL);
  assert(prologue != NULL);

  nrows_src = XRIT_HRV_NROWS;
  ncols_src = XRIT_HRV_NCOLUMNS;
  nrows_tgt = XRIT_HRV_NROWS;
  ncols_tgt = 2*XRIT_HRV_NCOLUMNS;

  assert(nrows_tgt == ncols_tgt);

  init_image(target, nrows_tgt, ncols_tgt);

  lower_south = ntohl(prologue->imageDescription.plannedCoverageHRV.lowerSouthLinePlanned) - 1;
  lower_north = ntohl(prologue->imageDescription.plannedCoverageHRV.lowerNorthLinePlanned) - 1;
  lower_east  = ntohl(prologue->imageDescription.plannedCoverageHRV.lowerEastColumnPlanned) - 1;
  lower_west  = ntohl(prologue->imageDescription.plannedCoverageHRV.lowerWestColumnPlanned) - 1;

  upper_south = ntohl(prologue->imageDescription.plannedCoverageHRV.upperSouthLinePlanned) - 1;
  upper_north = ntohl(prologue->imageDescription.plannedCoverageHRV.upperNorthLinePlanned) - 1;
  upper_east  = ntohl(prologue->imageDescription.plannedCoverageHRV.upperEastColumnPlanned) - 1;
  upper_west  = ntohl(prologue->imageDescription.plannedCoverageHRV.upperWestColumnPlanned) - 1;

  if (get_xrit_orientation() == XRIT_NORTH_AT_TOP) {
    lower_south = nrows_tgt - lower_south - 1;
    lower_north = nrows_tgt - lower_north - 1;
    upper_south = nrows_tgt - upper_south - 1;
    upper_north = nrows_tgt - upper_north - 1;

    lower_east = ncols_tgt - lower_east - 1;
    lower_west = ncols_tgt - lower_west - 1;
    upper_east = ncols_tgt - upper_east - 1;
    upper_west = ncols_tgt - upper_west - 1;
    
  }

  set_hrv_image_part(source, nrows_src, ncols_src, 
		     target, nrows_tgt, ncols_tgt,
		     lower_south, lower_north, lower_east, lower_west);  
  
  set_hrv_image_part(source, nrows_src, ncols_src, 
		     target, nrows_tgt, ncols_tgt,
		     upper_south, upper_north, upper_east, upper_west);

}

/* for getopt (FIXME: not declared with unistd when -std=c99 is enabled, check why) */
int getopt(int argc, char * const argv[],
	   const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;

static char *app_version = "0.2.5";
static unsigned char spare[19814];

static uint32_t opt_flags = 0;
static double fill_value = NAN;

static uint16_t *counts = NULL;
static uint16_t *counts_displayable = NULL;
static float *spectral_radiances_flt = NULL;
static double *spectral_radiances_dbl = NULL;

char prologue_file[FILENAME_MAXLEN] = "";

int data_type = OPT_UINT16;
int isegment = 0;
const char *str_data_type = "uint16 (unsigned integer 16 bits)";

int is_valid_filename(const char *filename) {
  const char *pattern1 = "H-???-MSG?__-MSG?________-?????????-CYCLE____-????????????";
  const char *pattern2 = "H-???-MSG?__-MSG?________-?????????-CYCLE____-????????????-__";
  int match1;
  int match2;
  
  char filename_[FILENAME_MAXLEN]; /* basename may modify its argument so we work on a modifiable copy of it */
  strncpy(filename_, filename, FILENAME_MAXLEN);
  
  match1 = fnmatch(pattern1, basename(filename_), 0);
  match2 = fnmatch(pattern2, basename(filename_), 0);
  
  return (match1 == 0 || match2 == 0);
}

/* parses options from command line */
void parse_options(int *argc, char **argv[]) {
  int option;
  const char *options = "P:n:sfdvIF:"; /* option -r disabled */

  opterr = 0;

  while ((option = getopt(*argc, *argv, options)) != -1) {
    switch (option) {
    case 'P' : {
      strncpy(prologue_file, optarg, FILENAME_MAXLEN - 1);
      break;
    }
    case 'n' : {
      isegment = atoi(optarg);
      if (isegment < 1) {
	fprintf(stderr, "xrit2raw: invalid argument for -n (should be between 1 and 8 in low res or 1 and 24 in high res)\n");
	exit (EXIT_FAILURE);
      }
      break;
    }
    case 'r' : {
      set_xrit_orientation(XRIT_SOUTH_AT_TOP);
      break;
    }
    case 's' : {
      data_type = OPT_UINT16;
      str_data_type = "uint16 (unsigned integer 16 bits)";
      break;
    }
    case 'f' : {
      data_type = OPT_FLOAT;
      str_data_type = "float (real 32 bits)";
      break;
    }
    case 'd' : {
      data_type = OPT_DOUBLE;
      str_data_type = "double (real 64 bits)";
      break;
    }
    case 'v' : {
      opt_flags |= OPT_VERBOSE;
      break;
    }
    case 'I' : {
      opt_flags |= OPT_INFO;
      break;
    }
    case 'F' : {
      fill_value = atof(optarg);
      break;
    }
    case '?' : {
      break;
    }
    } /* switch */
  } /* while option */
  
  *argc -= optind - 1;
  *argv += optind - 1;
  
} /* parse_options */



int main(int argc, char *argv[]) {

  FILE *output_stream;
  char *input_file;
  char output_file[FILENAME_MAXLEN];
  size_t nwritten;
  MSG_Prologue prologue;
  double cal_slope[12];
  double cal_offset[12];
  const unsigned char version = 0;
  int ichannel;
  register int irow, icol;
  int nsegments = 0;
  int nsegments_per_slot = 0;
  int i;
  int nrows;
  int ncols;
  char *input_shortname;

  set_xrit_orientation(XRIT_NORTH_AT_TOP);
  parse_options(&argc, &argv);

  if (argc < 2) {
    fprintf(stderr, "xrit2raw version %s\n\n", app_version);
    fprintf(stderr, "usage: %s [OPTIONS] <input xrit file> [<output raw file>]\n\n", argv[0]);
    fprintf(stderr, "By default, the tool extracts numeric integer counts (16 bits); to obtain physical values, use -f or -d (see below).\n");
    fprintf(stderr, "For more details on the data conversion, see MSG Level 1.5 Image Data Format Description, p 25 (v6, 23 february 2010) available here:\n");
    fprintf(stderr, "%s (at the time of this writing)\n\n", link_to_guide);
    fprintf(stderr, 
	    "OPTIONS:\n"
	    "  -P <prologue_file>                 saves the msg prologue into 'prologue_file' (a binary file with the prologue structure only, can be read with read_seviri_prologue)\n"
	    "  -n <segment>                       extracts only one segment (between 1 and 8 in low res or 1 and 24 in high res) of 464 rows (experimental option)\n"
	    /* "  -r                                 the image will be reversed\n" */ /* disabled for the time being */
	    "  -s                                 stores msg counts (short integer) into the raw file (default behavior)\n"
	    "  -f                                 stores msg calibrated data as float into the raw file, in physical units: mW.m^-2.sr^-1.(cm^-1)^-1\n"
	    "  -d                                 stores msg calibrated data as double into the raw file, in physical units: mW.m^-2.sr^-1.(cm^-1)^-1\n"
	    "  -I                                 displays some informations from the input file, namely the central longitude of the satellite and\n"
            "                                     the calibration coefficients (no raw file will be created)\n"
	    "  -v                                 verbose mode. Displays the raw file newly created, and a brief description of its content (dimensions, data type ...)\n"
	    "  -F fill_value                      default value, when no measure is available (for calibrated data only, e.g. in the outer space), nan by default\n"
	    "\n"
	    "xrit2raw will dump the data available from a HRIT CYCLE file (*) into a raw file \n"
	    "(raw array of values, without header, whose type and dimensions can be displayed by the -v option, that you should always use\n"
	    "interactively.)\n\n"
	    "(*) You can build HRIT CYCLE files yourself with the tool put_xrit_segments_together.pl provided by this suite. Type put_xrit_segments_together.pl\n"
	    "without argument to have all the explanations about this command.\n"
	    );
    exit(EXIT_FAILURE);
  }

  input_file = argv[1];

  input_shortname = strrchr(input_file, '/');
  if (input_shortname) input_shortname++;
  else input_shortname = input_file;

  if (! is_valid_filename(input_shortname)) {
    fprintf(stderr, "xrit2raw: %s: wrong filename, a HRIT_CYCLE file was expected "
                    "(such a file can be produced by the command put_xrit_segments_together.pl from the EUMETCAST broadcast files, look at this command help for details)\n", input_file);
    exit(EXIT_FAILURE);
  }

  ichannel = xrit_met8_channel(input_shortname);
  if (ichannel < 0) {
    fprintf(stderr, "xrit2raw: no channel has been recognized in the MSG file name.\n");
    fprintf(stderr, "xrit2raw: xrit2raw expects to find one of the following channels in the filename:  ");
    fprintf(stderr, "VIS006 VIS008 IR016 IR039 WV062 WV073 IR087 IR097 IR108 IR120 IR134 HRV\n");
    exit(EXIT_FAILURE);
  }
  if (ichannel == XRIT_CHAN_HRV) { 
    set_xrit_resolution(XRIT_HIGH_RES);
    counts = malloc(XRIT_HRV_NROWS*XRIT_HRV_NCOLUMNS*sizeof(uint16_t));
    nrows = XRIT_HRV_NROWS;
    ncols = 2*XRIT_HRV_NCOLUMNS; /* the raw data are in a 11136*5568 array, one puts the final data in a 11136*11136 array */
    assert(nrows == ncols);
    nsegments_per_slot = 24;
  }
  else {
    set_xrit_resolution(XRIT_LOW_RES);
    counts = malloc(XRIT_NROWS*XRIT_NCOLUMNS*sizeof(uint16_t));
    nrows = XRIT_NROWS;
    ncols = XRIT_NCOLUMNS;
    assert(nrows == ncols);
    nsegments_per_slot = 8;
  }
  
  if (argc > 2) {
    strncpy(output_file, argv[2], FILENAME_MAXLEN - 1);
  }
  else {
    snprintf(output_file, FILENAME_MAXLEN - 1, "%s.raw", input_shortname);
  }

  if ((nsegments = read_xrit(input_file, counts, &isegment, NULL, NULL, &prologue, NULL)) < 0) {
    exit (EXIT_FAILURE);
  }

  if (ichannel == XRIT_CHAN_HRV) {
    counts_displayable = malloc(nrows*ncols*sizeof(uint16_t));
    set_hrv_image(counts, counts_displayable, &prologue);
  }
  else {
    counts_displayable = counts;
  }
  assert(counts_displayable != NULL);


  if (*prologue_file) { /* writes the prologue into a file */
    FILE *prologue_stream;
    prologue_stream = fopen(prologue_file, "w");
    if (! prologue_stream) perror(prologue_file);
    else {
      nwritten = fwrite(&version, sizeof(version), 1, prologue_stream);
      if (nwritten != 1) {
	perror(prologue_file);
	exit(EXIT_FAILURE);
      }
      nwritten = fwrite(&prologue, sizeof(prologue), 1, prologue_stream);
      if (nwritten != 1) {
	perror(prologue_file);
	exit(EXIT_FAILURE);
      }
      /* fills 0 in place of IMPFConfiguration record (currently not implemented in prologue) */
      nwritten = fwrite(spare, sizeof(spare), 1, prologue_stream);
      if (nwritten != 1) {
	perror(prologue_file);
	exit(EXIT_FAILURE);
      }
      
      if (fclose(prologue_stream) != 0) {
	perror(output_file);
	exit(EXIT_FAILURE);
      }
    }
  }


  for (i = 0 ; i < 12 ; i++) {
    cal_slope[i] = ntoh_double(prologue.radiometricProcessing.level1_5ImageCalibration[i].cal_Slope);
    cal_offset[i] = ntoh_double(prologue.radiometricProcessing.level1_5ImageCalibration[i].cal_Offset);
  }

  if (opt_flags & OPT_INFO) {
    float nominal_longitude = ntoh_float(prologue.satelliteStatus.satelliteDefinition.nominalLongitude);
    float longitude_of_SSP  = ntoh_float(prologue.imageDescription.projectionDescription.longitudeOfSSP);
    printf("For explanations about the following quantities, please refer to the documentation from Eumetsat about SEVIRI, available there (at the time of this writing): \n");
    printf("%s\n", link_to_guide);
    printf("the author of the software won't provide any assistance on this.\n\n");
    printf("Nominal Longitude: %f (actual longitude of the satellite in the space, counted positively towards the east from Greenwich)\n", nominal_longitude);
    printf("Longitude of subsatellite point: %f (it's the actual term from Eumetsat documentation, to make it clear it is the longitude of the central pixel of the image after reprojection)\n", longitude_of_SSP);
    for (i = 0 ; i < 12 ; i++) {
      if (opt_flags & OPT_INFO) {
	printf("calibration coefficients : %-12s (%d)\tslope: %12lg    offset: %12lg\n",
	       channels[i], i+1, cal_slope[i], cal_offset[i]);
      }
    }
    exit (EXIT_SUCCESS);
  }

  output_stream = fopen(output_file, "w");
  if (! output_stream) {
    perror(output_file);
    exit(EXIT_FAILURE);
  }

  switch (data_type) {
  case OPT_UINT16 :
    if (nsegments > 1) nwritten = fwrite(counts_displayable, nrows*ncols*sizeof(uint16_t), 1, output_stream);
    else  nwritten = fwrite(counts_displayable, nrows*ncols*sizeof(uint16_t)/nsegments_per_slot, 1, output_stream);
    break;
  case OPT_FLOAT :
    spectral_radiances_flt = malloc(nrows*ncols*sizeof(float));
    assert(spectral_radiances_flt != NULL);
    for (irow = 0 ; irow < nrows ; irow++) {
      for (icol = 0 ; icol < ncols ; icol++) {
	if (counts_displayable[irow*ncols+icol] == 0) { 
	  spectral_radiances_flt[irow*ncols+icol] = fill_value; 
	}
	else {
	  spectral_radiances_flt[irow*ncols+icol] = cal_slope[ichannel]*counts_displayable[irow*ncols+icol] + cal_offset[ichannel];
	}
      }
    }
    if (nsegments > 1) nwritten = fwrite(spectral_radiances_flt, nrows*ncols*sizeof(float), 1, output_stream);
    else nwritten = fwrite(spectral_radiances_flt, nrows*ncols*sizeof(float)/nsegments_per_slot, 1, output_stream);
    break;
  case OPT_DOUBLE :
    spectral_radiances_dbl = malloc(nrows*ncols*sizeof(double));
    assert(spectral_radiances_dbl != NULL);
    for (irow = 0 ; irow < nrows ; irow++) {
      for (icol = 0 ; icol < ncols ; icol++) {
	if (counts_displayable[irow*ncols+icol] == 0) {
	  spectral_radiances_dbl[irow*ncols+icol] = fill_value;
	}
	else {
	  spectral_radiances_dbl[irow*ncols+icol] = cal_slope[ichannel]*counts_displayable[irow*ncols+icol] + cal_offset[ichannel];
	}
      }
    }
    if (nsegments > 1) nwritten = fwrite(spectral_radiances_dbl, nrows*ncols*sizeof(double), 1, output_stream);
    else nwritten = fwrite(spectral_radiances_dbl, sizeof(spectral_radiances_dbl)/nsegments_per_slot, 1, output_stream);
    break;
  default :
    fprintf(stderr, "%s: fatal error: unexpected value for variable data_type: %d\n", argv[0], data_type);
    exit (EXIT_FAILURE);
    break;
  }
   
  if (nwritten != 1) {
    perror(output_file);
    exit(EXIT_FAILURE);
  }

  if (fclose(output_stream) != 0) {
    perror(output_file);
    exit(EXIT_FAILURE);
  }

  if (opt_flags & OPT_VERBOSE) {
    if (nsegments > 1) {
      fprintf(stdout, "xrit2raw: %s: height: %d width: %d, data type: %s\n", output_file, nrows, ncols, str_data_type);
    }
    else {
      fprintf(stdout, "xrit2raw: %s: height: %d width: %d, data type: %s\n", output_file, nrows/nsegments_per_slot, ncols, str_data_type);
    }
  }

  if (counts != NULL) { free(counts); }
  if (spectral_radiances_flt != NULL) { free(spectral_radiances_flt); }
  if (spectral_radiances_dbl != NULL) { free(spectral_radiances_dbl); }

  exit(EXIT_SUCCESS);

}
