#ifdef __NOGUI__
  #include "xrit2pic_nogtk.h"
#else
  #include <gtk/gtk.h>
#endif

#include <time.h>
#include <stdio.h>


#define DEF_NOAA_XRES 2048
#define DEF_NOAA_ALPHAMAX 55.4
#define AVHRR_NRCHANS 5
#define AVHRR_PICWIDTH 2048
#define AVHRR_LINESPSEC 6

/* --------- RAH header defs ---------- */
/* Header version */
#define HRPT1SIGN  "HRPT"
#define LENSIGN_H 4
#define HRPTHDRVERSION "EXT_V004"

/* End of header sign*/
#define ENDFLG 8
#define ENDHDR "END_HDR_"

/* nr. bytes between 2 frames */
#define NR_INTER_BYTES 7

/* Max. length sat-name */
#define LENSATNAME_HDR1 16

#define NR_CHANNELS_MAX 11
#define LENSATNAME 40
#define LENFILENAME 200

/* Length reserved for color maps into header */
#define CMAPLEN10B 1024
#define MASK_HRPT1_SYNC_OK_PRE  0x0c

#define Rearth 6378160.
#ifndef PI
  #define PI 3.1415926
#endif
#define PIx2 (PI*2.)           /* not present in math.h */
#ifndef D2R
  #define D2R(g) ((g)*PI/180.)
#endif

#define G0    9.798                   /* gravity constant */
#define Rearth 6378160.                /* radius earth (meters) */       
#define Const 9.95                    /* ??? */
#define Earth_offsetangle 98.967440   /* correctie! ??? */
#define LEN_YEAR 365.2422
#define LEN_LEAPYEAR 366.2422

/* Channel names */
#define A_CH1 "VIS006"
#define A_CH2 "VIS008"
#define A_CH3 "IR_039"
#define A_CH4 "IR_108"
#define A_CH5 "IR_120"

typedef enum {scan_unk=0,N_S,S_N,W_E,E_W} SCANDIR;
/*************************************
 * Frame info
 *************************************/
typedef struct frameinfo
{
  int words_per_frame;     /* amount of words in a frame */
  int words_per_line;      /* total amount of saved data words per line */
  int bytes_per_line;      /* total amount of saved bytes per line */
  int header_len;          /* words before first picture-word */
  int leader_len;          /* words after last picture-word */
  int inter_chan;          /* interval between 2 words of a channel */
  int dist_chan;           /* interval between 2 channels */
} FRAMEINFO;



/* Time info with 2 byte integers */
struct tm_sh
{
  gint16 tm_sec,tm_min,tm_hour;
  gint16 tm_mday,tm_mon,tm_year,tm_wday,tm_yday,tm_isdst;
};

/* Kepler info as saved in HRPT header ! DONT CHANGE! */
typedef struct 
{
  gint32 noradnr;
  gint32 internat;
  gint32 epoch_year;
  float epoch_day;
  float decay_rate;
  float inclination;
  float raan;
  float eccentricity;
  float perigee;
  float anomaly;
  float motion;
  gint32 epoch_rev;
} KEPLER1;

/* Definition HRPT header.
   NOTE: char=1 byte, float=4 bytes
*/
typedef struct
{
  char sign[LENSIGN_H];
  char sat_name[LENSATNAME_HDR1];
  char kan_tot;
  char filetype;
  gint16 width,height;
  gint16 depth;
  char offset_gmt,offset_sec;
  struct tm_sh tm_comp;
  KEPLER1 kepler;

  char hdr_version[ENDFLG];
  gint16 hdrlen;
  gint16 x_offset,y_offset;
  char satdir,reserve2;

  struct tm_sh tm_satstrt;
  char tm_comp_sat_vlg;

  float offset_lon;
  gint16 dmin[8],dmax[8];
  gint16 ro,go,bo;
  float kr[8],kg[8],kb[8];
  char channel[9];
  gint16 widthmax;
  float alphamax;
  char r[5][CMAPLEN10B],g[5][CMAPLEN10B],b[5][CMAPLEN10B];
  char reserve[348];
  char endhdr[ENDFLG];
} HRPTHDR;

typedef struct picinfo
{
  FILE *fp;                        /* filepointer; NULL if no file opened */
  int width,height;                /* Size picture */
  char sat_name[LENSATNAME];       /* name satellite */
  gboolean channels_recorded[NR_CHANNELS_MAX];  /* Channels recorded */
  FRAMEINFO fri;                   /* Frame info */

  KEPLER kepler;                   /* Keplerian elements of sat orbit */
  ORBIT orbit;                     /* Orbit vars */
  float max_sens_angle;            /* satellite sensor angle */
  int ox;
  int width_original;              /* Size original pic */
  int lines_per_sec;               /* lines per second */

} PICINFO;

union fl_lng
{
  float f;
  guint32 l;
};


