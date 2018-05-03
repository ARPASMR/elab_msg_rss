/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Header XRIT2PIC
 ********************************************************************/
#define TEST_MEMn
#ifdef TEST_MEM
  #ifndef malloc
    #define malloc tmalloc
  #endif
  #ifndef realloc
    #define realloc trealloc
  #endif
  #ifndef calloc
    #define calloc tcalloc
  #endif
  #ifndef free
    #define free tfree
  #endif
#endif

#define DEBUG_STR(s) if (gdebug) puts(s);
extern int gdebug;

#ifdef __NOGUI__
  #include "xrit2pic_nogtk.h"
  #include <stdio.h>
#else
  #include "sgtk.h"
#endif
#include "vcdu.h"
#include "vcdu_funcdef.h"
#define MEMALLOCDBGn
#ifdef MEMALLOCDBG
#define malloc(a) dbg_malloc(a)
#define calloc(a,b) dbg_calloc(a,b)
#define free(a) dbg_free(a)
#endif

/* Files preferences */
#define PREFFILE "xrit2pic.ini"
#define GPREFFILE "xrit2pic.gini"

/* Determine if/when warnings are given */
#define PARANOIDE 1

#define IFRMT2TEXT(f) (f=='j'? "JPEG" : \
                       f=='w'? "WVT"  : \
                               "Unknown")
#define RCHAR(l) l[strlen(l)-1]
#define rewind_s(str) while ((str) && (str->prev)) str=str->prev
#define wind_s(str) while ((str) && (str->next)) str=str->next
#define R2D(a) ((a)*180./3.14159)
#define SIGN(a) ((a)>0? 1 : (a)<0? -1 : 0)

#define NINT(i) ((i) < 0? (int)((i)-0.5) : (int)((i)+0.5))

#if __GTK_WIN32__ == 1
  #define BZIPPROG "bzip2.exe"
  #define WGETPROG "wget.exe"
#else
  #define BZIPPROG "bzip2"
  #define WGETPROG "wget"
#endif

#define LAB_HOLDCHNK "Minimize"

/* Defaults external programs: viewer and mencoder */
#define LIN_VIEWER_PGM "wsat -w 500 -h 500 -i"
#define LIN_SNOPT_PGM "-sn"
#define LIN_VIEWER_JPG "xv"
#define LIN_SNOPT_JPG "-rotate 180"
#define LIN_PLAY_MOVIE "mplayer"
#define LIN_GEN_MOVIE "mencoder mf://%s%s -o %s%s -mf type=jpeg -ovc lavc -lavcopts vcodec=msmpeg4v2 -mf fps=1 > /dev/null"
#define LIN_BUFR_EXTR "Bufr2Asc"
#define LIN_BUFRTBL_LOC "$HOME/bin/bufr_tables/"
//#define LIN_OPTSPEED "-mf fps=1"

#define WIN_VIEWER_PGM "i_view32"
#define WIN_SNOPT_PGM "/rotate_r /rotate_r"
#define WIN_VIEWER_JPG "i_view32"
#define WIN_SNOPT_JPG "/rotate_r /rotate_r"
#define WIN_PLAY_MOVIE "mplayer"
#define WIN_GEN_MOVIE "mencoder mf://%s%s -o %s%s -mf type=jpeg -ovc lavc -lavcopts vcodec=msmpeg4v2 -mf fps=1"
#define WIN_BUFR_EXTR "Bufr2Asc"
#define WIN_BUFRTBL_LOC "c:\\bufr_tables\\"  // NO spaces allowed!
//#define WIN_OPTSPEED "-mf fps=1"


#define REMOVEWINDOW "Channels to keep"
#define LAB_NUPDATE "^nr. Done"
#define LAB_VIS   "!Show in list"

#define nrchanlist 12
#define nrsatlist 17

#define MAXCHAN 25
#define MAX_3DSHIFT 50
#define DEF_3DSHIFT 20

#define MAIN_TAB "!main_tab"
#define RECORD_TAB "!record_tab"

#define MENU_VIEW "/View"
#define MENU_LONLAT "View/lon_lat"
#define MENU_EVIEW "View/Exported"
#define MENU_3DIM "View/Anaglyph"
#define MENU_TEMP "View/IR_Temp_CLR"
#define MENU_TEMPG "View/IR_Temp_BW"
#define MENU_FIRE "View/Fires"
#define MENU_LUT "View/Use Lut"
#define MENU_SLUT "View/Select Lut"

#define MENU_GMAP  "/Projection "
#define MENU_AVLIN  "Projection/AVHRR lin"
#define MENU_GEOGR  "/Projection/proj"
#define MENU_GEOGR_NR  "Projection/proj/normal"
#define MENU_GEOGR_PC  "Projection/proj/Plate Carree"
#define MENU_GEOGR_MT  "Projection/proj/Mercator"
#define MENU_GEOGR_PN  "Projection/proj/Polar North"
#define MENU_GEOGR_PS  "Projection/proj/Polar South"

#define LAB_SEGMFRST "Start:"
#define LAB_SEGMNR "Nbr.:"

#define OOR -1000

typedef struct point
{
  float x,y,z;
  float lon,lat;
} POINT;

typedef struct places
{
  struct places *prev,*next;
  POINT p;
  char name[40];
} PLACES;

typedef struct satchannel_list
{
  gboolean use_list;
  gboolean chan_sel[nrchanlist];
  gboolean sat_sel[nrsatlist];
  gboolean unk_sel;
} SATCHAN_LIST;

typedef enum { UNK_ORBIT=0,GEO=1,POLAR=2} SAT_ORBITTYPE;

typedef enum { ASK=0,REPLACE=1,SKIP=2} OVERWRMODE;

typedef enum { in_received=0,in_done=1,in_archive,no_file } LOCATION_SEGM;

typedef enum { map_vis_hmsg=1,map_vis_lmsg=2,
               map_airm=3,map_dust=4,map_nfog=5,map_dfog=6,
               map_ucld=7,map_udust=8,map_uash=9,map_unight=10,
               map_uday=11,map_cnv_strm=12,map_snowfog=13,
               map_noaavis=100,map_metopvis=101,
               map_cust=200,map_cust1=201
             } COMPOSE_TYPE;

typedef struct dirsel
{
  GtkCList *list;
  int selrow;
} DIRSEL;

typedef struct rgbmaps
{
  float gamma;
  char chanp[10];  // MSG channel; pos. contr.
  char chann[10];  // MSG channel; neg. contr
  int  valmin;     // lower level of this combination
  int  valmax;     // higher level of this combination
} RGBMAPS;

typedef struct msgcmap
{
  char name[20];
  char name_sh[20];
  gboolean use_temp;
  struct rgbmaps r,g,b;
} MSGCMAP;

typedef enum 
{
  normal=0,plate_carree=1, mercator=2, 
  polar_n=3,polar_s=4,
  polar_cn=5,polar_cs=6
} GEOMAP;

typedef enum 
{
  coast_p='C',country_p='c', lonlat_p='l',both_p='b',
  coast_v='v',
  ovl_err=-1
} OVERLAYTYPE;

typedef struct ascatpoint
{
  struct ascatpoint *prev,*next;
  struct scatpoint *sp;
} ASCATPOINT;

typedef struct scatpoint
{
  struct scatpoint *prev,*next;
  float lon,lat;
  float speed,sdir;
  char *errmessage;
} SCATPOINT;

typedef struct showpixmode
{
  gboolean compose;
  gboolean fire_detect;
  gboolean map_temp;        // map IR channels onto temperature (colour)
  gboolean map_temp_G_mA;   // map IR channels onto temperature (Goes mode-A)
  COMPOSE_TYPE compose_type;
  gboolean invert;
  gboolean inv_ir;
} SHOWPIXMODE;

typedef enum
{
  S_messages ='S',
  S_hritp1   ='h',
  S_hritp2   ='g',
  S_hrita    ='H',
  S_lritp1   ='l',
  S_lritp2   ='k',
  S_lrita    ='L',
  S_other    ='f',
  S_met5     ='5',
  S_met6     ='6',
  S_met7     ='7',
  S_mpef     ='m',
  S_noaa     ='A',
  S_metop    ='M',
  S_dwdsat   ='D',
  S_bufr     ='B',
  S_ers      ='E',
  S_ers_geo  ='s',
  S_all      ='a',
  S_geo_all  = 0xf00,
  S_pol_all  = 0xf01
} SHOWTREE;

#define NRLUTPF 2
typedef struct lut
{
  struct lut *prev,*next;
  char dir[200];
  char name[200];
  char fname[200];
  int w;                  // lut-len
  int nrluts;             // # luts in this file
  int sellut;             // selected lut
  int *rgb[NRLUTPF];
  gboolean use;
} LUT;

typedef struct globinfo
{
  OVERWRMODE overwrite_mode;
  OVERWRMODE filmframe_ovrwrmode;
  SHOWTREE vis_mode;
  gboolean abort;
  gboolean hrv_lum;
  gboolean mirror_negval;

  struct showpixmode spm;
  struct showpixmode spm_live;

  int nr_pics;
  int nr_selpics;
  unsigned long tot_size;
  float mbsize;
  gboolean move_to_done;
  gboolean donedir_tomake;
  gboolean donedir_exist;
  gboolean show_new;
  gboolean gen_new;
  gboolean rm_raw;
  gboolean gen_movienew;
  gboolean pic_only;
  int bitshift;
  gboolean avhrr_lin;
  gboolean view_exported;
  int area_nr;              // 0=full
  gboolean dim3;
  GEOMAP gmap;
  gboolean essential_only;

  int shift_3d;
  int nrfiles;

// Limit polar pics
  char pdir_show;     // 'a', 'd' or 'b'=0
  int ltlat,gtlat;

  gboolean add_overlay;
  OVERLAYTYPE overlaytype;

  gboolean marktype_vec;   // draw marks as vector in drawable
  gboolean add_marks;
  gboolean add_lonlat;

  struct satchannel_list keep_chanlist;
  int segmrange[2];
  
/* list received dirs */
  struct dirsel dirsel;

  struct chanmap *chanmap;
  int offset[3];
  float gamma[3];
  struct msgcmap clrmapspec;
  
  char upd_sat[30];
  char upd_chan[30];
//  char depth_chan[30];             // anaglyph depth chan
//  gboolean dchan_iv;

  gboolean notify;

  char src_dir[300];
  char src_archdir[300];
  char src_donedir[300];
  char dest_dir[300];
  char dest_tmpdir[300];
  char dest_tmpsavdir[300];
  char prog_bzip2[300];
  char run_cmd[300];

  struct segments *bufr;
  SCATPOINT *scatp;
  ASCATPOINT *ascatp;
  gboolean scat_plot_all;
  gboolean test_bufr;
  
  LUT lut;
} GLOBINFO;


typedef struct chanmap
{
  struct chanmap *prev,*next;
  char chan_name[20];
  float r,g,b;
  gboolean temperature;
  gboolean invert;
} CHANMAP;

typedef struct overlay
{
  struct overlay *prev,*next;
  char fname[80];
  char *dir;
  char satname[20];                  /* met,goes,mtsat */
  int sattype;                       /* 8,7,6,5 (met), 10,11,12 (goes) */
  OVERLAYTYPE overlaytype;           /* l(atlong), c(ountry), 0(just coast) */
  char channel[10];                  /* channel: hrv, ir, vis or "" */
  int satpos;                        /* position satellite (east) */
  gboolean rapidscan;
} OVERLAY;

typedef struct picpart
{
  int fix_width;
  int fix_woffset;
  int fix_height;
  int fix_hoffset;
  int pct_width;
  int pct_woffset;
  int pct_height;
  int pct_hoffset;
} PICPART;

typedef struct geoarea
{
  char part_name[50];
  struct picpart pp_norm;
  struct picpart pp_hrv;
  struct point center;
  struct point delta;
  int pol_dir;
} GEOAREA;

typedef struct f1map
{
  char chan[10];
  float r, g, b;
} F1MAP;

typedef struct fmap
{
  int offset[3];
  float gamma[3];
  gboolean use_temp;
  int n;
#if __GTK_WIN32__ == 1
   F1MAP fm[8];
#else
   F1MAP fm[8];
#endif
} FMAP;

#define NRGEODEFS 4
typedef struct geoareadefs
{
  char part_name[50];  
  struct point center;
  struct point delta;
} GEOAREADEFS;



typedef struct pref
{
  char used_preffile[300];
  char gui_inifile[300];
  char *cur_dir;
  char *home_dir;
  char *prog_dir;
  char *prog_xdir;
  char *ini_file;

  int font_size;
  int film_size;
  int polar_size;
  int max_nr_rfiles;    // amx. nr. of received fiels (above: do 'something')
  char prog_jpg[100];
  char prog_jpg12[100];
  char ud_jpg[100];
  char prog_pgm[100];
  char ud_pgm[100];
  char prog_playmovie[300];
  char prog_genmovie[300];
  char prog_bufrextr[300];
  char bufrtblloc[300];
//  char opt_speed[20];
  int speed;
  gboolean extern_moviegen;
  char src_archdir[300];
  char tlq_cmd[300];
  char tlq_pdir[300];      // tellique program location
  char tlq_sdir[300];      // tellique start location
  char src_dir[300];
  char dest_dir[300];
  gboolean split_destdir;
  char done_dir[20];
  char lut_dir[300];
  char lut_file[30];
  struct overlay *overlay;
  struct lut *lut;
  char prf_ovltype;        // prefered overlaytype: 'p'(icture) or 'v'(ector)
  int ovl_clr;           // colour overlay (1 nibble per clr;'rgb')
  int lonlat_clr;        // colour lon/lat lines
  int scat_clr;          // colour scat arrows
  int scat_selclr;       // colour scat arrows selected
  gboolean add_marks;
  int size_cmark;    // size cross mark

  POINT lmark;
  PLACES *places;
  gboolean show_marks;
  
  FMAP fmap_vis_hmsg;
  FMAP fmap_vis_lmsg;
  FMAP fmap_vis_noaa;
  FMAP fmap_vis_metop;
  MSGCMAP msgmap_cust;

  gboolean scandir_correct;
  gboolean low_mem;
  gboolean size_draw_limit;
  gboolean avhrr_mode;
  gboolean full_globe;
  
  int deldaymin[2];     // [0]: das, [1]: hours
  int deldaymax;
  gboolean roundday;

  GEOAREA geoarea[NRGEODEFS];

  gboolean use_fn_frmt;
  char fn_frmt[100];
  char noradfile[30];
  char placesfile[30];
  char earthmapfile1[300];
  char earthmapfile2[300];
  char run_cmd[300];
  char upd_sat[30];
  char upd_chan[30];
  int upd_cycle;
  int wndwidth;
  int wndheight;
  int pr_wndwidth;
  int pr_wndheight;
  int lv_wndwidth;
  int lv_wndheight;
  char depth_chan[30];             // anaglyph depth chan
  gboolean dchan_iv;
  gboolean invert_ir;
  gboolean essential_only;

  gboolean record_at_start;
  gboolean delete_old_at_start;

} PREFER;

typedef enum { OK=0 ,Unk_Frmt,Tmp_fn,JPG_Rd,JPG_Wr,JPG_Conc,
               Open_Rd,Open_Wr,Open_Wrt,Out_fn,WVT_Err,Aborted,Exist_fn,Mis_segm,Bzip2_Err,
               Avi_picsize=-1,Avi_nrframes=-2,Avi_write=-3   // These MUST be negative!
             } ERRCODE;

typedef struct plottext
{
  int x,y;
  int size;
  char str[100];
} PLOTTEXT;

typedef struct anagl
{
  int shift_3d;
  int lmin;
  int lmax;
  gboolean init;
  gboolean depthchan;
  gboolean ol_3d;
} ANAGL;

typedef struct genmodes
{
  char type;               /* L or H */
  char *sat;
  char *chan_name;
  char *segm_range;
  char *date;
  char *time;
  char otype;       /* 'v(iew)'  or 'f(ile)' */  
  char oformat;     /* 'j(peg)': direct, 'J(peg)': indir, 'p(gm)' */ 
  char cformat;     /* choosen format: 'd(efault)','j(peg)','p(gm)' */ 
  struct chanmap *chanmap;
  gboolean do_one_pic;
  gboolean gen_film;
  gboolean run_script;
  gboolean essential_only;
  gboolean timeid;
  struct showpixmode spm;
  gboolean tempmap;

  char *wnd_clrmap;
  gboolean use_hrv_lum;
  OVERWRMODE overwrite_mode;
  OVERWRMODE filmframe_ovrwrmode;
  gboolean gen_pro;
  gboolean gen_epi;
  gboolean gen_rah;
  gboolean skip_incomplete;
  gboolean test;
  struct plottext pt;
  char image_oformat;
  void (*log_func)();
  FILE *fplog;
//  int xoff,yoff,xfac,yfac;  // for mapped polar
  int dx,dy;
  float zx,zy;
  int ox,oy;
  int odepth;
  int frame_minwidth;   // min. width used frames
  gboolean adapt_lum;
  int lmin,lmax;
  int lminrgb[3],lmaxrgb[3];
  float gamma;
  OVERLAYTYPE overlaytype;           /* l(atlong), c(ountry), 0(just coast) */
  int ol_lum;
  int area_nr;              // 0=full
  gboolean avhrr_lin;
  gboolean avhrr_lin_dblx;
  GEOMAP gmap;
  struct anagl agl;
  GEOAREA geoarea;
  gboolean add_overlay;
  gboolean add_lonlat;
  struct plottext pt2;

//  struct picpart pp_norm;
//  struct picpart pp_hrv;
} GENMODES;

typedef struct luminfo
{
  gboolean sep_rgblum;
  guint32 *lumstat;
  int lumscale;
  int lminrgb[3];
  int lmaxrgb[3];
  int lmin;
  int lmax;
} LUMINFO;

#define HTTP_KEPLER "http://celestrak.com/NORAD/elements"
#define NORADFILE "weather.txt"

typedef struct kepler
{
  gint32 epoch_year;
  float  epoch_day;
  float  decay_rate;
  float  inclination;
  float  raan;
  float  eccentricity;
  float  perigee;
  float  anomaly;
  float  motion;
  gint32 epoch_rev;
} KEPLER;

typedef struct orbit
{
  gboolean valid;                  /* flag: kepler data valid? */

/* Start time record (used) */
  struct tm start_tm;              /* record start time  */
  int       start_ms;              /* record start time msec */

/* Start time record (satellite) */
  struct tm start_tm_sat;          /* record start time  */
  int       start_ms_sat;          /* record start time msec */

/* Start time record (computer) */
  struct tm start_tm_comp;         /* record start time  */
  int    loop_nr;                  /* # loops from ref. at start pic */

/* Reference time Kepler data */
  gint32    ref_time;              /* total reference time  */
  struct tm ref_tm;                /* reference time year ... sec */
  int       ref_ms;                /* reference time msec */

  float height;                    /* sat height [meters] */ 
  float loop_time;                 /* time 1 loop [seconds] */
  float k2_n;
  float k2_w;
  float dh_w;

  int age_days;                    /* age Kepler data in days */
  gboolean use_sattime;
  int offset_gmt;
  int offset_sec;
  float offset_lon;
} ORBIT;

typedef struct geochar
{
  gint32 coff,cfac,loff,lfac;
  int upper_x_shift;     /* shift after shift_ypos (HRV upper) */
  int lower_x_shift;     /* shift before shift_ypos (HRV lower) */
  int shift_ypos;        /* pos after which x shifts (HRV) */
  float sub_lon;
} GEOCHAR;

typedef struct
{
// info pic with combined channels
  int widthmax;
  int heightmax;
  
  int first_chunck;
  int last_chunck;
  int nrchuncks;
  int y_chunck;
  int width;
  int org_width;            // org. width (for HRV: width total globe)
  int x_w,x_e,y_n,y_s;      // actual pos. earth globe
  int height;
  int depth;
  int lines_per_sec;        // for polar sats
  char image_iformat;
  char fno[1000];
  char picname[1000];
  char chan[50];
  char sat_src[20];         // see satsrc in vcdu.h
  int scan_dir;

  KEPLER kepler;
  ORBIT orbit;
  gboolean avhrr_lin;
  GEOMAP gmap;
  float *ypos_tbl[2];
  int xoff,yoff,xfac,yfac;  // for mapped polar
  float pixel_shape;
  SAT_ORBITTYPE orbittype;
  gboolean incomplete;      // picture incomplete (missing segments)
  gboolean partly;          // part of picture to generate
  gboolean is_color;
  gboolean map_temp;        // map IR channels onto temperature
  gboolean map_temp_G_mA;   // map IR channels onto temperature GOES mode-A (dog-leg)
  gboolean use_lut;
  gboolean invert;
  gboolean avhrr_lin_dblx;
  int bitshift;
  struct geochar gchar;
  int o_width,o_height;
  int o_xoffset,o_yoffset;
} PIC_CHARS;

typedef struct orbit_segm
{
  POINT subsat_start;
  POINT subsat_end;

  int orbit_start;
  int epoch_year;
  float epoch_day;
  float eccentricity;
  float inclination;
  float perigee;
  float raan;
  float anomaly;
  int total_mdr;
  int duration_of_product;
} ORBIT_SEGM;

typedef struct
{
  float LongitudeOfSSP;  /* sub-satellite point in degrees (after correction) */
} ProjectionDescription;

/* NOTE! If link to allocated mem added: See Copy_Segm; set NULL in copy! */ 
typedef struct segments
{
  struct segments *prev,*next;
  struct channels *chan;
  gboolean is_pro,is_epi;
  GtkCTreeNode *node;
  gboolean selected;
  gboolean nselected;
  gboolean shown;
  struct xrit_hdr xh;
  char *fn;              /* filename segment */
  char *pfn;             /* path + filename segment */
  time_t time;           /* time file */
  LOCATION_SEGM where;   /* location of file: received, done, archive */
  gboolean corrupt;      /* corrupt file */
  guint16 *chnk;
  char *ovlchnk;
  int width,height;
  int y_offset;          /* y where actual pic starts (partly filled segm) */
  OVERLAYTYPE loaded_ovl;
  int upper_east_col;    /* shift after shift_ypos (HRV upper) */
  int lower_east_col;    /* shift before shift_ypos (HRV lower) */
  int lower_nord_row;    /* pos after which x shifts (HRV) */
  ProjectionDescription ProjectionDescription;
  ORBIT_SEGM orbit;
} SEGMENT;

struct calibration
{
  int caltbl_bpp;
  int caltbl_type;
  float *caltbl[2];
  int nrcalpoints;
} CALIBRATION;

#define MAXLEN_ID 40
typedef struct channels
{
  struct channels *prev,*next;
  struct group *group;
  GtkCTreeNode *node;
  gboolean selected;
  gboolean nselected;
  gboolean hide;
  struct segments *segm;
  struct segments *pro;
  struct segments *epi;
  char *chan_name;
  char *data_src;
  char ifrmt;
  char id[MAXLEN_ID+1];
  char scan_dir;
  int chan_nr;
  int satpos;
  int wavel;
  int pic_nr;
  int nr_segm;
  int seq_start;
  int seq_end;
  int nc,nl,nb;
  int lines_per_sec;        // for polar sats
  double Cal_Slope[12];  /* calibration slope */
  double Cal_Offset[12]; /* calibration offset */

  struct calibration cal;

  int upper_x_shift;     /* shift after shift_ypos (HRV upper) */
  int lower_x_shift;     /* shift before shift_ypos (HRV lower) */
  int shift_ypos;        /* pos after which x shifts (HRV) */
  int max_x_shift;       /* max of upper_x_shift and lower_x_shift */
  int width_totalglobe;  /* if !=0: # lines if pic would be total globe */
  int height_totalglobe; /* if !=0: # lines if pic would be total globe (for MDM) */
  int ncext;             /* extended nc for full earth (HRV) */
  float r,g,b;           /* contr. to r/g/b */
  gboolean invert;       /* invert this channel */
  gboolean lum;          /* use for luminance in comb. with rgb of other chans */
} CHANNEL;

#define MAXCHANSTR 20
typedef struct group
{
  struct group *prev,*next;
  struct group *org;
  struct list_files *list_files;
  GtkCTreeNode *node;
  gboolean selected;
  gboolean nselected;
  gboolean hide;
  gboolean compose;
  gboolean gen_rah;
  gboolean has_kepler;
  gboolean done;
  gboolean new;             // group is new
  struct channels *chan;
  struct channels *pro_epi;
  char sat_src[20];         // see satsrc in vcdu.h
  struct tm grp_tm;
  char grp_time[20];        // see satsrc in vcdu.h
  char id[MAXLEN_ID+1];
  int pic_nr;
  char h_or_l;
  DATATYPE datatype;       // 
  SAT_ORBITTYPE orbittype;
  char scan_dir;
  char chanstr[MAXCHANSTR];
  PIC_CHARS pc;
  guint16 *rgbpicstr[4];   // rgb and mask
  gboolean *line_in_mem;
  gboolean *ovl_in_mem;
  gboolean keep_in_mem;
  gboolean conv_temp;
  int offset[3];
  float gamma[3];
  int segm_first,segm_last;
  int overload;         // >0: too(?) much files, # = amount of files
} GROUP;


typedef struct list_files
{
  struct list_files *prev,*next;
  struct list_files *cparent;
  struct list_files *tparent;
  GtkCTreeNode *pnode;
  GtkCTreeNode *cnode;
  GtkCTreeNode *tnode;

  struct xrit_hdr pxh;
  struct group *grp;
  struct channels *chan;
  gboolean set_selected;
  gboolean chan_selected;
  gboolean segm_selected;
  gboolean compose;
  char *path;            /* path to file */
  char *fn;              /* file name */
  char *pfn;             /* path + file name */
  time_t time;           /* time file */
  int age;               /* age in days */
  int nr_files;          /* # files found (only in top-struct, so with prev==NULL) */
  int set_nr;
  int pic_nr;
  int segm_nr;
  int nr_childs;
  int nr_segments;
  int lps;         // lines per second (polar)
  char frmt[20];
  gboolean is_dir;
  gboolean unknown;
  float r,g,b;
} LIST_FILES;

typedef struct filelist
{
  struct filelist *prev,*next;
  char *fn;
  unsigned long size;
  unsigned long offset;
  gboolean dont_use;
} FILELIST;

typedef struct xrit_dbase
{
//  GROUP *group;
  LIST_FILES *unknown;
  GROUP *grp_last;             // last group in list, at start recording
  GtkCTree *main_tree;
  gboolean log;
//  struct globinfo globinfo;  // nog te doen
//  struct pref prefer;        // nog te doen
} XRIT_DBASE;


#define IS_POLAR(g) (g->orbittype==POLAR)
#define IS_GEO(g)   (g->orbittype==GEO)

#define CMASK  0x01         // country boundaries
#define CLMASK 0x02         // country boundaries (anaglyph copy)
#define LMASK  0x04         // lon/lat lines
#define MMASK  0x08         // marked points
#define FMASK  0x10         // mask filter 
#define SMASK  0x20         // mask space


#define HRV_XSHIFT (2110-15)
#define HRV_SHIFT_YPOS (8190+64)

/* Menu and button texts */
#define LAB_MAIN "Xrit2Pic"

#define LAB_FROMDONE "Move files from Done"
#define LAB_TODONE "Move files to Done"

#define LAB_PROGRESS_TRANS "Translate list"
#define LIST_DATA "List_Data"
#define GRP_DATA "Group_Data"
#define LAB_TRANSGEN "Generate:"


#define LAB_ADIR "^Directory to add"

#define LAB_LOAD "Load selected"
#define LAB_INFO "!Info "       /* space needed; conflict with other 'Info'? */
#define LAB_RECINFO "!Record Info "

#define LAB_EXPCOL "Expand/Collapse"

#define LAB_FFT     "^Format"  /* format main */
#define LAB_FFTP    "Format"   /* format popup */

#define LAB_FFTDEF  "Default"
#define LAB_PGM     "PGM"
#define LAB_PGM8    "PGM8"
#define LAB_JPG     "JPeg"
#define LAB_CJPG    "CJPeg"

#define LAB_ARCHSRCDI "!arch_srcdir"
#define LAB_SDIR "!^Source directory"

#define PREV_WNDNAME "Preview"
#define LAB_SHFT3D_PREV "Shift"
#define LAB_LMIN3D_PREV "Lbnd"
#define LAB_LMAX3D_PREV "Hbnd"
#define LAB_DCHAN_PREV "Dpthchan"
#define LAB_OL3D_PREV "Ovl_3D"
#define LAB_OLADD_PREV "Ovl_dim"

#define LAB_CRLOAD "Start updating/Updating  "
#define LAB_CDELOLD "Start Delete old/Deleting old"

#define LAB_MORE     "!more"
#define LAB_PROEPI   "^Pro/Epi"

#define LAB_COLOR    "Colour map"
#define LAB_COLORFIX "Colour 'Live'"
#define LAB_COLORSPEC "Colour map spec"

#define LAB_CLOUDS "Night clouds"
#define LAB_SHFT3D "^3D sh"


//#define LAB_REMF "Remove tempfiles"
#define LAB_FLMTMP "Tempfiles"
#define LAB_TMPRM "Remove"
#define LAB_TMPRPL "Replace"
#define LAB_TMPREUSE "Reuse"

#define LAB_SHOWSAT "Sat"
#define LAB_SHOWCHAN "Chan"

#define LAB_EARTHWND "Polar sat orbit"

#define PI 3.1415926
#define D2R(g) ((g)*PI/180.)

#define LAB_HBAR "hbar"
#define LAB_VBAR "vbar"

#include "xrit2pic_funcs.h"
