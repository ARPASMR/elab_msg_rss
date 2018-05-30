/* libmsg_sp_types.h : specific purpose types */

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

/** @file libmsg_sp_types.h The MSG/SEVIRI specific-purpose types.
 * See <i>MSG LeveL 1.5 Image Data Format Description</i> document (from EUMETSAT) for details.
 *
 * This is not a complete implementation of MSG Level 1.5 Image Data Format Description.
 * For the time being, nor IMPFConfiguration structure neither MSG/SEVIRI Epilogue are implemented
 */

#ifndef _LIBMSG_SP_TYPES_H
#define _LIBMSG_SP_TYPES_H

#include "libmsg_gp_types.h"

/* SATELLITE STATUS */
typedef struct _SatelliteStatus {
  struct /* SatelliteDefinition */ {
    GP_SC_ID satelliteID;
    REAL nominalLongitude;
    ENUMERATED_BYTE satelliteStatus;
  } PACKED satelliteDefinition;
  
  struct /* SatelliteOperations */ {
    BOOLEAN_BYTE lastManoeuvreFlag;
    TIME_CDS_SHORT lastManoeuvreStartTime;
    TIME_CDS_SHORT lastManoeuvreEndTime;
    ENUMERATED_BYTE lastManoeuvreType;
    
    BOOLEAN_BYTE nextManoeuvreFlag;
    TIME_CDS_SHORT nextManoeuvreStartTime;
    TIME_CDS_SHORT nextManoeuvreEndTime;
    ENUMERATED_BYTE nextManoeuvreType;
  } PACKED satelliteOperations;

  struct /* Orbit */ {
    TIME_CDS_SHORT periodStartTime;
    TIME_CDS_SHORT periodEndTime;
    struct /* OrbitPolynomial */ {
      TIME_CDS_SHORT startTime;
      TIME_CDS_SHORT endTime;
      REAL_DOUBLE X[8];
      REAL_DOUBLE Y[8];
      REAL_DOUBLE Z[8];
      REAL_DOUBLE VX[8];
      REAL_DOUBLE VY[8];
      REAL_DOUBLE VZ[8];
    } PACKED orbitPolynomial[100];
  } PACKED orbit;

  struct /* Attitude */ {
    TIME_CDS_SHORT periodStartTime;
    TIME_CDS_SHORT periodEndTime;
    REAL_DOUBLE principleAxisOffsetAngle;
    struct /* AttitudeCoef */ {
      TIME_CDS_SHORT startTime;
      TIME_CDS_SHORT endTime;
      REAL_DOUBLE XofSpinAxis[8];
      REAL_DOUBLE YofSpinAxis[8];
      REAL_DOUBLE ZofSpinAxis[8];
    } PACKED attitudeCoef[100];
  } PACKED attitude;

  REAL_DOUBLE spinRateatRCStart;

  struct /* UTCCorrelation */ {
    TIME_CDS_SHORT periodStartTime;
    TIME_CDS_SHORT periodEndTime;
    TIME_CUC_SIZE(4,3) onBoardTimeStart;
    REAL_DOUBLE varOnBoardTimeStart;
    REAL_DOUBLE A1;
    REAL_DOUBLE varA1;
    REAL_DOUBLE A2;
    REAL_DOUBLE varA2;
  } PACKED UTCCorrelation;

} PACKED SatelliteStatus;

typedef struct _ImageAcquisition {
  struct /* PlannedAcquisitionTime */ {
    TIME_CDS_EXPANDED trueRepeatCycleStart;
    TIME_CDS_EXPANDED plannedForwardScanEnd;
    TIME_CDS_EXPANDED plannedRepeatCycleEnd;
  } PACKED plannedAcquisitionTime;

  struct /* RadiometerStatus */ {
    ENUMERATED_BYTE channelStatus[12];
    ENUMERATED_BYTE detectorStatus[42];
  } PACKED radiometerStatus;

  struct /* RadiometerSettings */ {
    UNSIGNED_SHORT MDUSamplingDelays[42];
    
    struct /* HRVFrameOffsets */ {
      UNSIGNED_SHORT MDUNomHRVDelay1;
      UNSIGNED_SHORT MDUNomHRVDelay2;
      BITSTRING_SIZE(16) spare;
      UNSIGNED_SHORT MDUNomHRVBreakline;
    } PACKED HRVFrameOffsets;
    
    ENUMERATED_BYTE DHSSSynchSelection;
    UNSIGNED_SHORT MDUOutGain[42];
    UNSIGNED_BYTE MDUCoarseGain[42]; /* specified as CourseGain (erroneous) in MSG Level 1.5 Image Data Format Description (4 February 2005)  */
    UNSIGNED_SHORT MDUFineGain[42];
    UNSIGNED_SHORT MDUNumericalOffset[42];
    UNSIGNED_SHORT PUGain[42];
    UNSIGNED_SHORT PUOffset[27];
    UNSIGNED_SHORT PUBias[15];
    
    struct /* OperationParameters */ {
      UNSIGNED_SHORT L0_LineCounter;
      UNSIGNED_SHORT K1_RetraceLines;
      UNSIGNED_SHORT K2_PauseDeciseconds;
      UNSIGNED_SHORT K3_RetraceLines;
      UNSIGNED_SHORT K4_PauseDeciseconds;
      UNSIGNED_SHORT K5_RetraceLines;
      ENUMERATED_BYTE X_DeepSpaceWindowPosition;
    } PACKED operationParameters;

    UNSIGNED_SHORT refocusingLines;
    ENUMERATED_BYTE refocusingDirection;
    UNSIGNED_SHORT refocusingPosition;
    BOOLEAN_BYTE scanRefPosFlag;
    UNSIGNED_SHORT scanRefPosNumber;
    REAL scanRefPotVal;
    UNSIGNED_SHORT scanFirstLine;
    UNSIGNED_SHORT scanLastLine;
    UNSIGNED_SHORT retraceStartLine;
  } PACKED radiometerSettings;

  struct /* RadiometerOperations */ {
    BOOLEAN_BYTE lastGainChangeFlag;
    TIME_CDS_SHORT lastGainChangTime;
    struct /* Decontamination */ {
      BOOLEAN_BYTE decontaminationNow;
      TIME_CDS_SHORT decontaminationStart;
      TIME_CDS_SHORT decontaminationEnd;
    } PACKED decontamination;
    
    BOOLEAN_BYTE BBCalScheduled;
    ENUMERATED_BYTE BBCalibrationType;
    
    UNSIGNED_SHORT BBFirstLine;
    UNSIGNED_SHORT BBLastLine;
    UNSIGNED_SHORT coldFocalPlaneOpTemp;
    UNSIGNED_SHORT warmFocalPlaneOpTemp;
  } PACKED radiometerOperations;
  
} PACKED ImageAcquisition;

typedef struct _EARTHMOONSUNCOEF {
  TIME_CDS_SHORT startTime;
  TIME_CDS_SHORT endTime;
  REAL_DOUBLE alphaCoef[8];
  REAL_DOUBLE betaCoef[8];
} PACKED EARTHMOONSUNCOEF;

typedef struct _STARCOEF_ELEMENT {
  UNSIGNED_SHORT starId;
  TIME_CDS_SHORT startTime;
  TIME_CDS_SHORT endTime;
  REAL_DOUBLE alphaCoef[8];
  REAL_DOUBLE betaCoef[8];
} PACKED STARCOEF_ELEMENT;

typedef struct _STARCOEF {
  STARCOEF_ELEMENT element[20];
} PACKED STARCOEF;


typedef struct _EPHEMERIS {
  TIME_CDS_SHORT periodStartTime;
  TIME_CDS_SHORT periodEndTime;
  TIME_GENERALIZED relatedOrbitFileTime;
  TIME_GENERALIZED relatedAttitudeFileTime;
  EARTHMOONSUNCOEF earthEphemeris[100];
  EARTHMOONSUNCOEF moonEphemeris[100];
  EARTHMOONSUNCOEF sunEphemeris[100];
  STARCOEF starEphemeris[100];
} PACKED EPHEMERIS;


typedef struct _CelestialEvents {

  EPHEMERIS celestialBodiesPosition;
  struct /* RelationToImage */ {
    ENUMERATED_BYTE typeofEclipse;
    TIME_CDS_SHORT eclipseStartTime;
    TIME_CDS_SHORT eclipseEndTime;
    ENUMERATED_BYTE visibleBodiesInImage;
    ENUMERATED_BYTE bodiesClosetoFOV;
    ENUMERATED_BYTE impactOnImageQuality;
  } PACKED relationToImage;
} PACKED CelestialEvents;

typedef struct _IMPF_CAL_Data_Element {
  ENUMERATED_BYTE imageQualityFlag;
  ENUMERATED_BYTE referenceDataFlag;
  ENUMERATED_BYTE absCalMethod;
  char Pad1; /* FIXME: should be CHARACTERSTRING_SIZE(1), this type is not yet implemented */
  REAL absCalWeightVic;
  REAL absCalWeightXsat;
  REAL absCalCoeff;
  REAL absCalError;
  REAL calMonBias;
  REAL calMonRMS;
  REAL offsetCount;
} PACKED IMPF_CAL_Data_Element;

typedef struct _IMPF_CAL_Data {
  IMPF_CAL_Data_Element element[12];
} PACKED IMPF_CAL_Data;

typedef struct _ImageDescription {
  struct /* ProjectionDescription */ {
    ENUMERATED_BYTE typeOfProjection;
    REAL longitudeOfSSP;
  } PACKED projectionDescription;
  struct /* ReferenceGridVIS_IR */ {
    INTEGER numberOfLines;
    INTEGER numberOfColumns;
    REAL lineDirGridStep;
    REAL columnDirGridStep;
    ENUMERATED_BYTE gridOrigin;
  } PACKED referenceGridVIS_IR;
  struct /* ReferenceGridHRV */ {
    INTEGER numberOfLines;
    INTEGER numberOfColumns;
    REAL lineDirGridStep;
    REAL columnDirGridStep;
    ENUMERATED_BYTE gridOrigin;
  } PACKED referenceGridHRV;
  struct /* PlannedCoverageVIS_IR */ {
    INTEGER southernLinePlanned;
    INTEGER northernLinePlanned;
    INTEGER easternColumnPlanned;
    INTEGER westernColumnPlanned;
  } PACKED plannedCoverageVIS_IR;
  struct /* PlannedCoverageHRV */ {
    INTEGER lowerSouthLinePlanned;
    INTEGER lowerNorthLinePlanned;
    INTEGER lowerEastColumnPlanned;
    INTEGER lowerWestColumnPlanned;
    INTEGER upperSouthLinePlanned;
    INTEGER upperNorthLinePlanned;
    INTEGER upperEastColumnPlanned;
    INTEGER upperWestColumnPlanned;
  } PACKED plannedCoverageHRV;
  struct /* Level_1_5_ImageProduction */ {
    ENUMERATED_BYTE imageProcDirection;
    ENUMERATED_BYTE pixelGenDirection;
    BOOLEAN_BYTE plannedChanProcessing[12];
  } PACKED level_1_5_ImageProduction;
} PACKED ImageDescription;

typedef struct _RadiometricProcessing {
  struct /* RPSummary  */ {
    BOOLEAN_BYTE radianceLinearization[12];
    BOOLEAN_BYTE detectorEqualization[12];
    BOOLEAN_BYTE onboardCalibrationResult[12];
    BOOLEAN_BYTE MPEFCalFeedback[12];
    BOOLEAN_BYTE MTFAdaptation[12];
    BOOLEAN_BYTE straylightCorrectionFlag[12];
  } PACKED RPSummary;

  struct /* Level1_5ImageCalibration */ {
    REAL_DOUBLE cal_Slope;
    REAL_DOUBLE cal_Offset;
  } PACKED level1_5ImageCalibration[12];

  struct /* BlackBodyDataUsed */ {
    TIME_CDS_EXPANDED BBObservationUTC;
    struct /* BBRelatedData */ {
      TIME_CUC_SIZE(4,3) onBoardBBTime;
      UNSIGNED_SHORT MDUOutGain[42];
      UNSIGNED_BYTE MDUCoarseGain[42];
      UNSIGNED_SHORT MDUFineGain[42];
      UNSIGNED_SHORT MDUNumericalOffset[42];
      UNSIGNED_SHORT PUGain[42];
      UNSIGNED_SHORT PUOffset[27];
      UNSIGNED_SHORT PUBias[15];
      // BITSTRING_SIZE(12) DCRValues[42]; /* CHECKME: maybe portability issues due to the use of BITSTRING (implemented as a bitfield)  */
      unsigned char DCRValues[63]; /* should be defined as BITSTRING_SIZE(12), but BITSTRING_SIZE implementation is not yet satisfactory
				    * (each BITSTRING_SIZE(12) occupies 16 bits in memory and not really 12 bits)
				    */
      ENUMERATED_BYTE X_DeepSpaceWindowPosition;
      struct /* ColdFPTemperature */ {
	UNSIGNED_SHORT FCUNominalColdFocalPlaneTemp;
	UNSIGNED_SHORT FCURedundantColdFocalPlaneTemp;
      } PACKED coldFPTemperature;
      struct /* WarmFPTemperature */ {
	UNSIGNED_SHORT FCUNominalWarmFocalPlaneVHROTemp;
	UNSIGNED_SHORT FCURedundantWarmFocalPlaneVHROTemp;
      } PACKED warmFPTemperature;
      struct /* ScanMirrorTemperature */ {
	UNSIGNED_SHORT FCUNominalScanMirrorSensor1Temp;
	UNSIGNED_SHORT FCURedundantScanMirrorSensor1Temp;
	UNSIGNED_SHORT FCUNominalScanMirrorSensor2Temp;
	UNSIGNED_SHORT FCURedundantScanMirrorSensor2Temp;
      } PACKED scanMirrorTemperature;
      struct /* M1M2M3Temperature */ {
	UNSIGNED_SHORT FCUNominalM1MirrorSensor1Temp;
	UNSIGNED_SHORT FCURedundantM1MirrorSensor1Temp;
	UNSIGNED_SHORT FCUNominalM1MirrorSensor2Temp;
	UNSIGNED_SHORT FCURedundantM1MirrorSensor2Temp;
	UNSIGNED_BYTE FCUNominalM23AssemblySensor1Temp;
	UNSIGNED_BYTE FCURedundantM23AssemblySensor1Temp;
	UNSIGNED_BYTE FCUNominalM23AssemblySensor2Temp;
	UNSIGNED_BYTE FCURedundantM23AssemblySensor2Temp;
      } PACKED M1M2M3Temperature;
      struct /* BaffleTemperature */ {
	UNSIGNED_SHORT FCUNominalM1BaffleTemp;
	UNSIGNED_SHORT FCURedundantM1BaffleTemp;
      } PACKED baffleTemperature;
      struct /* BlackBodyTemperature */ {
	UNSIGNED_SHORT FCUNominalBlackBodySensorTemp;
	UNSIGNED_SHORT FCURedundantBlackBodySensorTemp;
      } PACKED blackBodyTemperature;
      struct /* FCUMode */ {
	BITSTRING_SIZE(16) FCUNominalSMMStatus;
	BITSTRING_SIZE(16) FCURedundantSMMStatus;
      } PACKED FCUMode;
      struct /* ExtractedBBData */ {
	UNSIGNED numberOfPixelsUsed;
	REAL meanCount;
	REAL RMS;
	UNSIGNED_SHORT maxCount;
	UNSIGNED_SHORT minCount;
	REAL_DOUBLE BB_Processing_Slope;
	REAL_DOUBLE BB_Processing_Offset;
      } PACKED extractedBBData[12];
    } PACKED BBRelatedData;
  } PACKED blackBodyDataUsed; /* CHECKME: error in Document MSG Level 1.5 Image Data Format Description (closing curly brackets missing in the document) */

  IMPF_CAL_Data MPEFCalFeedback;
  REAL radTransform[42][64]; /* CHECKME: check order of dimensions (Fortran and C don't share the same ordering scheme) */
  struct /* RadProcMTFAdaptation */ {
    REAL VIS_IRMTFCorrectionE_W[33][16]; /* CHECKME: check order of dimensions (Fortran and C don't share the same ordering scheme) */
    REAL VIS_IRMTFCorrectionN_S[33][16]; /* CHECKME: check order of dimensions (Fortran and C don't share the same ordering scheme) */
    REAL HRVMTFCorrectionE_W[42-34+1][16]; /* CHECKME: check order of dimensions (Fortran and C don't share the same ordering scheme) */
    REAL HRVMTFCorrectionN_S[42-34+1][16]; /* CHECKME: check order of dimensions (Fortran and C don't share the same ordering scheme) */
    REAL straylightCorrection[12][8][8]; /* CHECKME: check order of dimensions (Fortran and C don't share the same ordering scheme) */
  } PACKED radProcMTFAdaptation;
} PACKED RadiometricProcessing;

typedef struct _GeometricProcessing {
  struct /* OptAxisDistances */ {
    REAL E_WFocalPlane[42];
    REAL N_SFocalPlane[42];
  } PACKED optAxisDistances;
  struct /* EarthModel */ {
    ENUMERATED_BYTE typeOfEarthModel;
    REAL_DOUBLE equatorialRadius;
    REAL_DOUBLE northPolarRadius;
    REAL_DOUBLE southPolarRadius;
  } PACKED earthModel;
  REAL atmosphericModel[12][360]; /* CHECKME: check order of dimensions (Fortran and C don't share the same ordering scheme) */
  ENUMERATED_BYTE resamplingFunctions[12];
} PACKED GeometricProcessing;

/* FIXME: IMPFConfiguration not yet implemented
struct _IMPFConfiguration {
  
} IMPFConfiguration;
*/

typedef struct _MSG_Prologue {
  SatelliteStatus satelliteStatus;
  ImageAcquisition imageAcquisition;
  CelestialEvents celestialEvents;
  ImageDescription imageDescription;
  RadiometricProcessing radiometricProcessing;
  GeometricProcessing geometricProcessing;
} PACKED MSG_Prologue;

typedef struct _MSG_Epilogue {
  char *dont_use_me; /* FIXME: for the time being, this structure is not implemented. */
} PACKED MSG_Epilogue;

#endif
