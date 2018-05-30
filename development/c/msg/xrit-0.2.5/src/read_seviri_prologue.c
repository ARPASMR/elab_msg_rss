/* read_seviri_prologue.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stddef.h>
#include <assert.h>

#include "read_xrit.h"
#include "xrit_swap.h"
#include "libxrit.h"

xrit_rows_infos_t rows_infos;

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

int main(int argc, char *argv[]) {

  char *input_file;
  int nsegments;
  xrit_header_t *xrit_header;
  MSG_Prologue prologue;
  double cal_slope;
  double cal_offset;
  int i;
  int err;
  
  if (argc < 2) {
    fprintf(stderr, "usage: %s <SATMOS XRIT file (with \"CYCLE\" in the name)>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  input_file = argv[1];
  
  err = read_xrit(input_file, NULL, &nsegments, &xrit_header, &rows_infos, &prologue, NULL);
  if (err < 0) {
    exit (EXIT_FAILURE);
  }
  
  
  printf("prologue.satelliteStatus.satelliteDefinition.satelliteID = %hd\n", ntohs(prologue.satelliteStatus.satelliteDefinition.satelliteID));
  printf("prologue.satelliteStatus.satelliteDefinition.nominalLongitude = %f\n", ntoh_float(prologue.satelliteStatus.satelliteDefinition.nominalLongitude));
  printf("prologue.imageDescription.projectionDescription.longitudeOfSSP = %f\n", ntoh_float(prologue.imageDescription.projectionDescription.longitudeOfSSP));
  printf("prologue.satelliteStatus.satelliteDefinition.satelliteStatus = %d\n", prologue.satelliteStatus.satelliteDefinition.satelliteStatus);
  
  printf("prologue.imageDescription.plannedCoverageVIS_IR.southernLinePlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageVIS_IR.southernLinePlanned));
  printf("prologue.imageDescription.plannedCoverageVIS_IR.northernLinePlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageVIS_IR.northernLinePlanned));
  printf("prologue.imageDescription.plannedCoverageVIS_IR.easternColumnPlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageVIS_IR.easternColumnPlanned));
  printf("prologue.imageDescription.plannedCoverageVIS_IR.westernColumnPlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageVIS_IR.westernColumnPlanned));
  
  printf("prologue.imageDescription.plannedCoverageHRV.lowerSouthLinePlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageHRV.lowerSouthLinePlanned));
  printf("prologue.imageDescription.plannedCoverageHRV.lowerNorthLinePlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageHRV.lowerNorthLinePlanned));
  printf("prologue.imageDescription.plannedCoverageHRV.lowerEastColumnPlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageHRV.lowerEastColumnPlanned));
  printf("prologue.imageDescription.plannedCoverageHRV.lowerWestColumnPlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageHRV.lowerWestColumnPlanned));
  
  printf("prologue.imageDescription.plannedCoverageHRV.upperSouthLinePlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageHRV.upperSouthLinePlanned));
  printf("prologue.imageDescription.plannedCoverageHRV.upperNorthLinePlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageHRV.upperNorthLinePlanned));
  printf("prologue.imageDescription.plannedCoverageHRV.upperEastColumnPlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageHRV.upperEastColumnPlanned));
  printf("prologue.imageDescription.plannedCoverageHRV.upperWestColumnPlanned = %d\n", ntohl(prologue.imageDescription.plannedCoverageHRV.upperWestColumnPlanned));

  for (i = 0 ; i < 12 ; i++) {
    cal_slope = ntoh_double(prologue.radiometricProcessing.level1_5ImageCalibration[i].cal_Slope);
    cal_offset = ntoh_double(prologue.radiometricProcessing.level1_5ImageCalibration[i].cal_Offset);
  
    printf("prologue.radiometricProcessing.level1_5ImageCalibration[%d].cal_Slope  = %12lg (channel %s)\n", i, cal_slope, channels[i]);
    printf("prologue.radiometricProcessing.level1_5ImageCalibration[%d].cal_Offset = %12lg (channel %s)\n", i, cal_offset, channels[i]);
  }

  free_xrit_header(xrit_header); /* not used now, but may be in a later version */

  exit(EXIT_SUCCESS);

}
