/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Preferences functions
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xrit2pic.h"
#include "xrit_prefs.h"
#include "avhrr.h"
#include "xrit2pic_funcs.h"

#define DEFSAT_SHOW "HMSG1"
#define DEFCHAN_DEP "IR_108"

/* Default mappings */
FMAP fmap_vis_hmsg=
{
  {0,0,0},
  {1.,1.,1.},
  FALSE,
  3,
  {
    { "VIS006", 0., 0., 1. },
    { "VIS008", 0., 1., 0. },
    { "IR_016", 1., 0., 0. },
  }
};

FMAP fmap_vis_lmsg=
{
  {0,0,0},
  {1.,1.,1.},
  FALSE,
  2,
  {
    { "VIS006", 0., 1., 1. },
    { "IR_016", 1., 0., 0. },
  }
};

FMAP fmap_vis_noaa=
{
  {0,0,0},
  {1.,1.,1.},
  FALSE,
  3,
  {
    { A_CH1, 1., 0., 0. },
    { A_CH2, 0., 1., 0. },
    { A_CH4, 0., 0., 1. },
  }
};

FMAP fmap_vis_metop=
{
  {0,0,0},
  {1.,1.,1.},
  FALSE,
  3,
  {
    { A_CH1, 1., 0., 0. },
    { A_CH2, 0., 1., 0. },
    { A_CH3, 0., 0., 1. },
  }
};

/* Defaults fixed zoom area */
GEOAREADEFS gadef[NRGEODEFS]=
{
  { "Europe"  ,{ 0.,0.,0.,  6.0, 56.0 }, { 0.,0.,0., 19.7, 21.0 }},
  { "Atlantic",{ 0.,0.,0.,-18.0, 42.0 }, { 0.,0.,0., 19.7, 20.0 }},
  { ""        ,{ 0.,0.,0.,  0.0,  0.0 }, { 0.,0.,0.,  0.0,  0.0 }},
  { ""        ,{ 0.,0.,0.,  0.0,  0.0 }, { 0.,0.,0.,  0.0,  0.0 }}
};

float get_gadef(int i,int j)
{
  if (j==0) return gadef[i].center.lon;
  if (j==1) return gadef[i].center.lat;
  if (j==2) return gadef[i].delta.lon;
  if (j==3) return gadef[i].delta.lat;
  return 0.;
}

int search_file_1dir(char *rootfile,char *file,char *dir)
{
  char *fn;
  if (!dir) return 0;
  if (!*rootfile) return 0;
  if (!file) fn=malloc(strlen(dir)+strlen(rootfile)+10);
  else       fn=file;
  if (!fn) return 0;

  sprintf(fn,"%s%c%s",dir,DIR_SEPARATOR,rootfile);

  if ((exist_file(fn)))
  {
    if ((!file)&&(fn)) free(fn);
    return 1;
  }
  if ((!file)&&(fn)) free(fn);
  return 0;
}

#define EXTRA_DIR "xrit_extra"
/******************************************
 * Search root file; return full path in file.
 *  return: 1 found in  cur_dir
 *        : 2 found in  home_dir
 *        : 3 found in  prog_dir
 *        : 4 found in  prog_dir/extra_dir
 ******************************************/
int search_file(char *rootfile,char *file,
                 char *cur_dir,char *home_dir,char *prog_dir)
{
  char *extra_dir;
  if (search_file_1dir(rootfile,file,cur_dir)) return 1;
  if (search_file_1dir(rootfile,file,home_dir)) return 2;
  if (search_file_1dir(rootfile,file,prog_dir)) return 3;

  extra_dir=malloc(strlen(prog_dir)+strlen(EXTRA_DIR)+10);
  sprintf(extra_dir,"%s%c%s",prog_dir,DIR_SEPARATOR,EXTRA_DIR);
  if (search_file_1dir(rootfile,file,extra_dir))
  {
    free(extra_dir);
    return 4;
  }
  free(extra_dir);
  
  if (file) *file=0;
  return 0;
}
 
void get_dirs(char *progname,char **cur_dir,char **prog_dir,char **prog_xdir,char **home_dir)
{
  if (*cur_dir)   free(*cur_dir);   *cur_dir=NULL;
  if (*prog_dir)  free(*prog_dir);  *prog_dir=NULL;
  if (*prog_xdir) free(*prog_xdir); *prog_xdir=NULL;
  if (*home_dir)  free(*home_dir);  *home_dir=NULL;

  *cur_dir=g_get_current_dir();
  *prog_dir=get_path(progname);
  if (!*prog_dir)
    *prog_dir=".";
  *prog_xdir=malloc(strlen(*prog_dir)+strlen(EXTRA_DIR)+10);
  sprintf(*prog_xdir,"%s%c%s",*prog_dir,DIR_SEPARATOR,EXTRA_DIR);

  *home_dir=(char *)g_get_home_dir();

  #if __GTK_WIN32__ == 1
  {
    if (!(*home_dir))
    {
      if (!strncmp(*cur_dir+1,":\\",2))
      {
        *home_dir=malloc(10);
        strncpy(*home_dir,*cur_dir,2);
        *((*home_dir)+2)=0;
      }
    }
  }
  #endif
}
/*
#define WIN_SRCDIR "c:\\tellique\\received\\"
#define LIN_SRCDIR "/dosc/tellique/received/"
*/

#define WIN_TLQDIR "\\Program Files\\T-Systems\\BusinessTV-IP\\"
#define LIN_TLQPDIR "$HOME/bin/"
#define LIN_TLQSDIR "$HOME/tq/"

//#define WIN_SRCDIR "c:\\Program Files\\T-Systems\\BusinessTV-IP\\received\\"
#define WIN_SRCDIR "\\ecast\\received\\"
#define LIN_SRCDIR "$HOME/from_msg/received/"

#define WIN_DSTDIR "\\ecast\\exported\\"
#define LIN_DSTDIR "$HOME/"

#define WIN_LUTDIR "\\lut\\"
#define LIN_LUTDIR "$HOME/lut/"
#define LUT_FNAME "LUT361_MM-HRIT-cc.bmp"

#define NOAAKEPFILE "weather.txt"
#define EARTHMAPFILE "earthmap.gif"
#define CITYGPXFILE "cities.gpx"

void str_rshift(char *s,int n)
{
  int i=strlen(s);
  for (; i>=0; i--) s[i+n]=s[i];
}


void Load_Defaults(PREFER *prefer)
{
  int i;
  #if __GTK_WIN32__ == 1
    strcpy(prefer->prog_jpg,WIN_VIEWER_JPG);
    strcpy(prefer->prog_jpg12,WIN_VIEWER_JPG);
    strcpy(prefer->ud_jpg,WIN_SNOPT_JPG);
    strcpy(prefer->prog_pgm,WIN_VIEWER_PGM);
    strcpy(prefer->ud_pgm,WIN_SNOPT_PGM);
    strcpy(prefer->prog_playmovie,WIN_PLAY_MOVIE);
    strcpy(prefer->prog_genmovie,WIN_GEN_MOVIE);
    strcpy(prefer->run_cmd,"");
    strcpy(prefer->prog_bufrextr,WIN_BUFR_EXTR);
    strcpy(prefer->bufrtblloc,WIN_BUFRTBL_LOC);
//    strcpy(prefer->opt_speed,WIN_OPTSPEED);
    strcpy(prefer->tlq_pdir,WIN_TLQDIR);
    strcpy(prefer->tlq_sdir,WIN_TLQDIR);
    strcpy(prefer->src_dir,WIN_SRCDIR);
    strcpy(prefer->dest_dir,WIN_DSTDIR);
    strcpy(prefer->lut_dir,WIN_LUTDIR);
  #else
    strcpy(prefer->prog_jpg,LIN_VIEWER_JPG);
    strcpy(prefer->prog_jpg12,LIN_VIEWER_JPG);
    strcpy(prefer->ud_jpg,LIN_SNOPT_JPG);
    strcpy(prefer->prog_pgm,LIN_VIEWER_PGM);
    strcpy(prefer->ud_pgm,LIN_SNOPT_PGM);
    strcpy(prefer->prog_playmovie,LIN_PLAY_MOVIE);
    strcpy(prefer->prog_genmovie,LIN_GEN_MOVIE);
    strcpy(prefer->run_cmd,"");
    strcpy(prefer->prog_bufrextr,LIN_BUFR_EXTR);
    strcpy(prefer->bufrtblloc,LIN_BUFRTBL_LOC);
//    strcpy(prefer->opt_speed,LIN_OPTSPEED);
    strcpy(prefer->tlq_pdir,LIN_TLQPDIR);
    strcpy(prefer->tlq_sdir,LIN_TLQSDIR);
    if (!strncmp(prefer->tlq_sdir,"$HOME",strlen("$HOME")))
    {
      str_rshift(prefer->tlq_sdir,strlen(prefer->home_dir)-strlen("$HOME"));
      strncpy(prefer->tlq_sdir,prefer->home_dir,strlen(prefer->home_dir));
    }
    if (!strncmp(prefer->tlq_pdir,"$HOME",strlen("$HOME")))
    {
      str_rshift(prefer->tlq_pdir,strlen(prefer->home_dir)-strlen("$HOME"));
      strncpy(prefer->tlq_pdir,prefer->home_dir,strlen(prefer->home_dir));
    }

    strcpy(prefer->src_dir,LIN_SRCDIR);
    if (!strncmp(prefer->src_dir,"$HOME",strlen("$HOME")))
    {
      str_rshift(prefer->src_dir,strlen(prefer->home_dir)-strlen("$HOME"));
      strncpy(prefer->src_dir,prefer->home_dir,strlen(prefer->home_dir));
    }

    strcpy(prefer->dest_dir,LIN_DSTDIR);
    if (!strncmp(prefer->dest_dir,"$HOME",strlen("$HOME")))
    {
      str_rshift(prefer->dest_dir,strlen(prefer->home_dir)-strlen("$HOME"));
      strncpy(prefer->dest_dir,prefer->home_dir,strlen(prefer->home_dir));
    }

    strcpy(prefer->lut_dir,LIN_LUTDIR);
    if (!strncmp(prefer->lut_dir,"$HOME",strlen("$HOME")))
    {
      str_rshift(prefer->lut_dir,strlen(prefer->home_dir)-strlen("$HOME"));
      strncpy(prefer->lut_dir,prefer->home_dir,strlen(prefer->home_dir));
    }
  #endif
    strcpy(prefer->lut_file,LUT_FNAME);

  strcpy(prefer->src_archdir,"archive");
  prefer->speed=1;

  strcpy(prefer->done_dir,"done");
  prefer->scandir_correct=TRUE;
  prefer->low_mem=FALSE;
  prefer->size_draw_limit=TRUE;
  prefer->full_globe=FALSE;
  prefer->avhrr_mode=FALSE;
  prefer->deldaymin[0]=3;
  prefer->deldaymin[1]=0;
  prefer->deldaymax=30;
  prefer->roundday=FALSE;
  prefer->invert_ir=FALSE;

  prefer->upd_cycle=5;
  strcpy(prefer->upd_chan,"");
  strcpy(prefer->upd_sat,DEFSAT_SHOW);
  prefer->wndwidth=400;
  prefer->wndheight=500;
  prefer->font_size=0;
  prefer->pr_wndwidth=500;
  prefer->pr_wndheight=500;
  prefer->lv_wndwidth=500;
  prefer->lv_wndheight=500;
  prefer->film_size=2048;
  prefer->polar_size=7200;
  prefer->max_nr_rfiles=20000;

  prefer->overlay=load_overlaysfiles(prefer);
  prefer->prf_ovltype='P';
  prefer->ovl_clr    =0xfff;
  prefer->lonlat_clr =0x00f;
  prefer->scat_clr   =0x940;
  prefer->scat_selclr=0xf0f;
  prefer->add_marks=TRUE;
  prefer->size_cmark=50;
  strcpy(prefer->depth_chan,DEFCHAN_DEP);
  prefer->dchan_iv=TRUE;
  strcpy(prefer->noradfile,NOAAKEPFILE);
  strcpy(prefer->placesfile,CITYGPXFILE);
  strcpy(prefer->earthmapfile1,EARTHMAPFILE);
  strcpy(prefer->earthmapfile2,EARTHMAPFILE);
  prefer->record_at_start=FALSE;
  prefer->delete_old_at_start=FALSE;


  for (i=0; i<NRGEODEFS; i++)
  {
    strcpy(prefer->geoarea[i].part_name,gadef[i].part_name);
    prefer->geoarea[i].center.lon=gadef[i].center.lon;
    prefer->geoarea[i].center.lat=gadef[i].center.lat;
    prefer->geoarea[i].delta.lon =gadef[i].delta.lon;
    prefer->geoarea[i].delta.lat =gadef[i].delta.lat;
    prefer->geoarea[i].pol_dir   =0;
  }

  prefer->use_fn_frmt=FALSE;
  strcpy(prefer->fn_frmt,"%s_%c_%t");
  
  prefer->fmap_vis_hmsg=fmap_vis_hmsg;
  prefer->fmap_vis_lmsg=fmap_vis_lmsg;
  prefer->fmap_vis_noaa=fmap_vis_noaa;
  prefer->fmap_vis_metop=fmap_vis_metop;


  prefer->lmark.lon= 5.1812;
  prefer->lmark.lat=52.2359;


}

static void read_map(char *key,char *arg,char *name,FMAP *m)
{
  char tkey[20];
  int i;
  sprintf(tkey,"map_%s_offset",name);
  if (!strcmp(key,tkey))
  {
    sscanf(arg,"(%d,%d,%d)",&m->offset[0],&m->offset[1],&m->offset[2]);
  }

  sprintf(tkey,"map_%s_gamma",name);
  if (!strcmp(key,tkey))
  {
    sscanf(arg,"(%f,%f,%f)",&m->gamma[0],&m->gamma[1],&m->gamma[2]);
  }

  for (i=0; i<8; i++)
  {
    sprintf(tkey,"map_%s_fm[%d]",name,i);
    if (!strcmp(key,tkey))
    {
      char *p=strchr(arg,'(');
      if (p)
      {
        *p=0;
        strcpy(m->fm[i].chan,arg);
        *p='(';
        sscanf(p,"(%f,%f,%f)",&m->fm[i].r,&m->fm[i].g,&m->fm[i].b);
        m->n=i+1;
      }
    }
  }
}


#define PART_NAME "part_name"
#define PART_LON_CENTER "part_lon_center"
#define PART_LAT_CENTER "part_lat_center"
#define PART_LON_DELTA "part_lon_delta"
#define PART_LAT_DELTA "part_lat_delta"
#define PART_WIDTH "part_width"
#define PART_HEIGHT "part_height"
#define PART_WIDTH_OFF "part_woffs"
#define PART_HEIGHT_OFF "part_hoffs"
#define PART_POLDIR "part_polar_dir"
int Read_Pref(DIRSEL *dirsel,PREFER *prefer)
{
  FILE *fp;
  char l[1000],*w1,*w2,*w3,*p;
  int i;
  Load_Defaults(prefer);

  if (!prefer->used_preffile) return 0;
  if ((fp=fopen(prefer->used_preffile,"r")))
  {
    while (fgets(l,1000,fp))
    {
      if ((w1=strchr(l,'#'))) *w1=0;

      if (!(w1=strtok(l," \n	="))) continue;
      if (!(w2=strtok(NULL,"\n"))) continue;
      if (*w2=='"') w2++; if ((p=strchr(w2,'"'))) *p=0; 
      if (!strcmp(w1,"window_width"))
      {
        prefer->wndwidth=atoi(w2);
      }
      if (!strcmp(w1,"window_height"))
      {
        prefer->wndheight=atoi(w2);
      }
      if (!strcmp(w1,"prev_window_width"))
      {
        prefer->pr_wndwidth=atoi(w2);
      }
      if (!strcmp(w1,"prev_window_height"))
      {
        prefer->pr_wndheight=atoi(w2);
      }
      if (!strcmp(w1,"live_window_width"))
      {
        prefer->lv_wndwidth=atoi(w2);
      }
      if (!strcmp(w1,"live_window_height"))
      {
        prefer->lv_wndheight=atoi(w2);
      }
      if (!strcmp(w1,"font_size"))
      {
        prefer->font_size=atoi(w2);
      }
      if (!strcmp(w1,"film_size"))
      {
        prefer->film_size=atoi(w2);
      }

      if (!strcmp(w1,"polar_max_width"))
      {
        prefer->polar_size=atoi(w2);
      }

      if (!strcmp(w1,"overlay_type"))
      {
        prefer->prf_ovltype=*w2;
      }

      if (!strcmp(w1,"add_landmarks"))
      {
        prefer->add_marks=(*w2=='y');
      }
      if (!strcmp(w1,"size_landmark"))
      {
        prefer->size_cmark=atoi(w2);
      }
      if (!strcmp(w1,"location_cities"))
      {
        strncpy(prefer->placesfile,w2,40);
      }

      if (!strcmp(w1,"overlay_clr"))
      {
        prefer->ovl_clr=strtol(w2,NULL,16);
      }
      if (!strcmp(w1,"lonlat_clr"))
      {
        prefer->lonlat_clr=strtol(w2,NULL,16);
      }
      if (!strcmp(w1,"bufrovl_clr"))
      {
        prefer->scat_clr=strtol(w2,NULL,16);
      }
      if (!strcmp(w1,"bufrovl_sel_clr"))
      {
        prefer->scat_selclr=strtol(w2,NULL,16);
      }
      if (!strcmp(w1,"invert_ir"))
      {
        if (*w2=='n')
          prefer->invert_ir=FALSE;
        else
          prefer->invert_ir=TRUE;
      }


      read_map(w1,w2,"vishmsg",&prefer->fmap_vis_hmsg);
      read_map(w1,w2,"vislmsg",&prefer->fmap_vis_lmsg);
      read_map(w1,w2,"visnoaa",&prefer->fmap_vis_noaa);
      read_map(w1,w2,"vismetop",&prefer->fmap_vis_metop);
      if (!strcmp(w1,"scandir_correct"))
      {
        if (*w2=='n')
          prefer->scandir_correct=FALSE;
        else
          prefer->scandir_correct=TRUE;
      }

      if (!strcmp(w1,"low_mem"))
      {
        if (*w2=='y')
          prefer->low_mem=TRUE;
        else
          prefer->low_mem=FALSE;
      }
      
      if (!strcmp(w1,"keep_pic_in_mem"))
      {
        if (*w2=='y')
          prefer->low_mem=TRUE;
        else
          prefer->low_mem=FALSE;
      }

      if (!strcmp(w1,"avhrr_mode"))
      {
        if (*w2=='y')
          prefer->avhrr_mode=TRUE;
        else
          prefer->avhrr_mode=FALSE;
      }

      if (!strcmp(w1,"full_globe"))
      {
        if (*w2=='y')
          prefer->full_globe=TRUE;
        else
          prefer->full_globe=FALSE;
      }

      if (!strcmp(w1,"del_olderthan"))
      {
        prefer->deldaymin[0]=atoi(w2);
        if ((w3=strchr(w2,':')))
          prefer->deldaymin[1]=atoi(w3+1);
        else
          prefer->deldaymin[1]=0;

        if ((!prefer->deldaymin[0]) && (!prefer->deldaymin[1]))
          prefer->deldaymin[0]=1;

        prefer->deldaymin[1]=MIN(MAX(prefer->deldaymin[1],0),23);
        prefer->deldaymin[0]=MAX(prefer->deldaymin[0],0);
      }
      
      if (!strcmp(w1,"del_newerthan"))
      {
        prefer->deldaymax=atoi(w2);
      }
      if (!strcmp(w1,"del_roundday"))
      {
        if (*w2=='y')
          prefer->roundday=TRUE;
        else
          prefer->roundday=FALSE;
      }
      if (!strcmp(w1,"record_at_start"))
      {
        if (*w2=='y')
          prefer->record_at_start=TRUE;
        else
          prefer->record_at_start=FALSE;
      }
      if (!strcmp(w1,"delete_old_at_start"))
      {
        if (*w2=='y')
          prefer->delete_old_at_start=TRUE;
        else
          prefer->delete_old_at_start=FALSE;
      }

      if (!strcmp(w1,"norad_file"))
      {
        strcpy(prefer->noradfile,w2);
      }
      if (!strcmp(w1,"earthmap_file1"))
      {
        strcpy(prefer->earthmapfile1,w2);
      }
      if (!strcmp(w1,"earthmap_file2"))
      {
        strcpy(prefer->earthmapfile2,w2);
      }
      if (!strcmp(w1,"markpoint"))
      {
        if ((w3=strchr(w2,',')))
        {
          prefer->lmark.lat=atof(w2);
          prefer->lmark.lon=atof(w3+1);
        }
      }

      if (!strcmp(w1,"update_cycle"))
      {
        prefer->upd_cycle=atoi(w2);
      }
      if (!strcmp(w1,"update_channel"))
      {
        strcpy(prefer->upd_chan,w2);
      }

      if (!strcmp(w1,"alert_nr_recfiles"))
      {
        prefer->max_nr_rfiles=atoi(w2);
      }

      if (!strcmp(w1,"use_filename_format"))
      {
        if (*w2=='y')
          prefer->use_fn_frmt=TRUE;
        else
          prefer->use_fn_frmt=FALSE;
      }
      if (!strcmp(w1,"filename_format"))
      {
        strcpy(prefer->fn_frmt,w2);
      }

      if (!strcmp(w1,"viewer_jpeg"))
      {
        strcpy(prefer->prog_jpg,w2);
      }
      else if (!strcmp(w1,"viewer_pgm"))
      {
        strcpy(prefer->prog_pgm,w2);
      }
      else if (!strcmp(w1,"upside_down_jpg"))
      {
        strcpy(prefer->ud_jpg,w2);
      }
      else if (!strcmp(w1,"upside_down_pgm"))
      {
        strcpy(prefer->ud_pgm,w2);
      }
      else if (!strcmp(w1,"viewer_avi"))
      {
        strcpy(prefer->prog_playmovie,w2);
      }

      else if (!strcmp(w1,"movie_extern"))
      {
        if (*w2=='y')
          prefer->extern_moviegen=TRUE;
        else
          prefer->extern_moviegen=FALSE;
      }
      else if (!strcmp(w1,"movie_encoder"))
      {
        strcpy(prefer->prog_genmovie,w2);
      }
      else if (!strcmp(w1,"execute_command"))
      {
        strcpy(prefer->run_cmd,w2);
      }
      else if (!strcmp(w1,"movie_encspeed"))
      {
        prefer->speed=MAX(1,atoi(w2));
      }
      else if (!strcmp(w1,"bufr_extract"))
      {
        strcpy(prefer->prog_bufrextr,w2);
      }
      else if (!strcmp(w1,"bufrtbl_loc"))
      {
        strcpy(prefer->bufrtblloc,w2);
      }
      else if (!strcmp(w1,"lut_loc"))
      {
        strcpy(prefer->lut_dir,w2);
      }
      else if (!strcmp(w1,"lut_file"))
      {
        strcpy(prefer->lut_file,w2);
      }
      else if (!strcmp(w1,"anaglyph_depthchan"))
      {
        strcpy(prefer->depth_chan,w2);
      }
      else if (!strcmp(w1,"anaglyph_depthchan_invert"))
      {
        if (*w2=='y')
          prefer->dchan_iv=TRUE;
        else
          prefer->dchan_iv=FALSE;
      }
      else if (!strcmp(w1,PART_WIDTH))
      {
        prefer->geoarea[0].pp_norm.pct_width=MAX(0,MIN(100,atoi(w2)));
        prefer->geoarea[0].pp_hrv.pct_width=prefer->geoarea[0].pp_norm.pct_width*33/22;
      }
      else if (!strcmp(w1,PART_WIDTH_OFF))
      {
        prefer->geoarea[0].pp_norm.pct_woffset=MAX(0,MIN(100,atoi(w2)));
        prefer->geoarea[0].pp_hrv.pct_woffset=prefer->geoarea[0].pp_norm.pct_woffset*51/36;
      }
      else if (!strcmp(w1,PART_HEIGHT))
      {
        prefer->geoarea[0].pp_norm.pct_height=MAX(0,MIN(100,atoi(w2)));
        prefer->geoarea[0].pp_hrv.pct_height=prefer->geoarea[0].pp_norm.pct_height;
      }
      else if (!strcmp(w1,PART_HEIGHT_OFF))
      {
        prefer->geoarea[0].pp_norm.pct_hoffset=MAX(0,MIN(100,atoi(w2)));
        prefer->geoarea[0].pp_hrv.pct_hoffset=prefer->geoarea[0].pp_norm.pct_hoffset;
      }

      else if (!strcmp(w1,PART_NAME))
      {
        strcpy(prefer->geoarea[0].part_name,w2);
      }
      else if (!strcmp(w1,PART_LON_CENTER))
      {
        prefer->geoarea[0].center.lon=atof(w2);
      }
      else if (!strcmp(w1,PART_LAT_CENTER))
      {
        prefer->geoarea[0].center.lat=atof(w2);
      }
      else if (!strcmp(w1,PART_LON_DELTA))
      {
        prefer->geoarea[0].delta.lon=atof(w2);
      }
      else if (!strcmp(w1,PART_LAT_DELTA))
      {
        prefer->geoarea[0].delta.lat=atof(w2);
      }
      else if (!strcmp(w1,PART_POLDIR))
      {
        prefer->geoarea[0].pol_dir=atoi(w2);
      }

      else if (!strncmp(w1,PART_NAME,strlen(PART_NAME)))
      {
        if ((i=atoi(w1+strlen(PART_NAME))))
          if ((i>=1) && (i<=4))
            strcpy(prefer->geoarea[i-1].part_name,w2);
      }
      else if (!strncmp(w1,PART_LON_CENTER,strlen(PART_LON_CENTER)))
      {
        if ((i=atoi(w1+strlen(PART_LON_CENTER))))
          if ((i>=1) && (i<=4))
            prefer->geoarea[i-1].center.lon=atof(w2);
      }
      else if (!strncmp(w1,PART_LAT_CENTER,strlen(PART_LAT_CENTER)))
      {
        if ((i=atoi(w1+strlen(PART_LAT_CENTER))))
          if ((i>=1) && (i<=4))
            prefer->geoarea[i-1].center.lat=atof(w2);
      }
      else if (!strncmp(w1,PART_LON_DELTA,strlen(PART_LON_DELTA)))
      {
        if ((i=atoi(w1+strlen(PART_LON_DELTA))))
          if ((i>=1) && (i<=4))
            prefer->geoarea[i-1].delta.lon=atof(w2);
      }
      else if (!strncmp(w1,PART_LAT_DELTA,strlen(PART_LAT_DELTA)))
      {
        if ((i=atoi(w1+strlen(PART_LAT_DELTA))))
          if ((i>=1) && (i<=4))
            prefer->geoarea[i-1].delta.lat=atof(w2);
      }
      else if (!strncmp(w1,PART_POLDIR,strlen(PART_POLDIR)))
      {
        if ((i=atoi(w1+strlen(PART_POLDIR))))
          if ((i>=1) && (i<=4))
            prefer->geoarea[i-1].pol_dir=atoi(w2);
      }

      else if (!strcmp(w1,"split_destdir"))
      {
        if (*w2=='y')
          prefer->split_destdir=TRUE;
        else
          prefer->split_destdir=FALSE;
      }
      else if (!strcmp(w1,"a"))
      {
        strcpy(prefer->src_archdir,w2);
        finish_path(prefer->src_archdir);
      }
      else if (!strcmp(w1,"d"))
      {
        strcpy(prefer->dest_dir,w2);
        finish_path(prefer->dest_dir);
      }
      else if (!strcmp(w1,"s"))
      {
        strcpy(prefer->src_dir,w2);
        finish_path(prefer->src_dir);
      }
      else if (!strcmp(w1,"t"))
      {
        strcpy(prefer->tlq_sdir,w2);
        finish_path(prefer->tlq_sdir);
      }
      else if (!strcmp(w1,"T"))
      {
        strcpy(prefer->tlq_pdir,w2);
        finish_path(prefer->tlq_pdir);
      }
    }
    fclose(fp);
  }
  load_places(prefer);

  return 1;
}

gboolean is_absdir(char *d)
{
  #if __GTK_WIN32__ == 1
    if (*(d+1)==':') return TRUE;
  #else
    if (*d==DIR_SEPARATOR) return TRUE;
  #endif
  return FALSE;
}

#define TMPDIR "movietmp"
#define TMPSAVDIR "sav"
void related_dirs(GLOBINFO *gi,PREFER *prefer)
{
  sprintf(gi->src_donedir,"%s%s",gi->src_dir,prefer->done_dir);
  finish_path(gi->src_donedir);
  if (is_absdir(prefer->src_archdir))
  {
    sprintf(gi->src_archdir,"%s",prefer->src_archdir);
  }
  else
  {
    sprintf(gi->src_archdir,"%s%s",gi->src_dir,prefer->src_archdir);
  }
  finish_path(gi->src_archdir);
  sprintf(gi->dest_tmpdir,"%s%s%c",gi->dest_dir,TMPDIR,DIR_SEPARATOR);
  sprintf(gi->dest_tmpsavdir,"%s%s%c",gi->dest_tmpdir,TMPSAVDIR,DIR_SEPARATOR);
}

/* Next part contains funcs not needed for non-gui mode. */
#ifndef __NOGUI__

#include "gtk/gtk.h"
static void add_dir_to_list(DIRSEL *dirsel,char *w2)
{
  char *tmp[1];
  tmp[0]=w2;

  if ((dirsel) && (dirsel->list))
  {
    if (!find_clistitem(dirsel->list,tmp[0]))
      gtk_clist_append(GTK_CLIST(dirsel->list), tmp);

  }
}

void Read_PrefDirlist(DIRSEL *dirsel,PREFER *prefer)
{
  FILE *fp;
  char l[1000],*w1,*w2;
  int row=0;
  if (!prefer->used_preffile) return;
  if ((fp=fopen(prefer->used_preffile,"r")))
  {
    while (fgets(l,1000,fp))
    {
      if ((w1=strchr(l,'#'))) *w1=0;

      if (!(w1=strtok(l," \n	="))) continue;
      if (!(w2=strtok(NULL,"\n"))) continue;

// Handle XRIT source Directory list
      if (strlen(w1)==1)
      {
        if (strchr("0s",*w1))            // line starts with s or 0
        {
          char *p=w2+strlen(w2)-1;
          if (*p != DIR_SEPARATOR)
            sprintf(p,"%s%c",p,DIR_SEPARATOR);

          add_dir_to_list(dirsel,w2);    // Add source dir to list

          if (!strcmp(w1,"s"))           // This line contains active dir
          {
            if (dirsel) dirsel->selrow=row;
          }

          row++;
        }
      }
    }
  }
}

static void Save_PrefDirlist(GtkCList *clist,PREFER *prefer,FILE *fp)
{
  int row=0;
  gchar *text;
  if (!clist) return;

  for (row=0; ; row++)
  {
    if(!gtk_clist_get_text(GTK_CLIST(clist), row, 0, &text))
      break;

    if (strcmp(text,prefer->src_dir))
      fprintf(fp,"0 %s\n",text);
  }
}

static void wri_map(FILE *fp,char *name,FMAP m)
{
  int i;
  fprintf(fp,"map_%s_offset=(%d,%d,%d)\n",name,m.offset[0],m.offset[1],m.offset[2]);
  fprintf(fp,"map_%s_gamma=(%.2f,%.2f,%.2f)\n",name,m.gamma[0],m.gamma[1],m.gamma[2]);
  for (i=0; i<m.n; i++)
    fprintf(fp,"map_%s_fm[%d]=%s(%.2f,%.2f,%.2f)\n",name,i,m.fm[i].chan,m.fm[i].r,m.fm[i].g,m.fm[i].b);
  
}

int Save_Pref(GtkCList *clist,PREFER *prefer,char *preffile)
{
  FILE *fp=NULL;
  int i;
  if (!(fp=fopen(preffile,"w")))
  {
    Create_Message("Error","Can't open pref-file %s.",preffile);
    return 0;
  }

  fprintf(fp,"#Preferences xrit2pic\n");

  fprintf(fp,"window_width=%d\n",prefer->wndwidth);
  fprintf(fp,"window_height=%d\n",prefer->wndheight);
  fprintf(fp,"font_size=%d\n",prefer->font_size);
  fprintf(fp,"prev_window_width=%d\n",prefer->pr_wndwidth);
  fprintf(fp,"prev_window_height=%d\n",prefer->pr_wndheight);
  fprintf(fp,"live_window_width=%d\n",prefer->lv_wndwidth);
  fprintf(fp,"live_window_height=%d\n",prefer->lv_wndheight);
  
  fprintf(fp,"film_size=%d\n",prefer->film_size);
  fprintf(fp,"polar_max_width=%d\n",prefer->polar_size);

  fprintf(fp,"overlay_type=%s\n",(prefer->prf_ovltype=='V'? "V" : "P"));
  fprintf(fp,"add_landmarks=%c\n",(prefer->add_marks? 'y' : 'n'));
  fprintf(fp,"size_landmark=%d\n",prefer->size_cmark);

  fprintf(fp,"location_cities=%s\n",prefer->placesfile);
  fprintf(fp,"overlay_clr=%03x\n",prefer->ovl_clr);
  fprintf(fp,"lonlat_clr=%03x\n",prefer->lonlat_clr);
  fprintf(fp,"bufrovl_clr=%03x\n",prefer->scat_clr);
  fprintf(fp,"bufrovl_sel_clr=%03x\n",prefer->scat_selclr);
  fprintf(fp,"invert_ir=%c\n",(prefer->invert_ir? 'y' : 'n'));

  fprintf(fp,"\n");
  wri_map(fp,"vishmsg",prefer->fmap_vis_hmsg);
  fprintf(fp,"\n");
  wri_map(fp,"vislmsg",prefer->fmap_vis_lmsg);
  fprintf(fp,"\n");
  wri_map(fp,"visnoaa",prefer->fmap_vis_noaa);
  fprintf(fp,"\n");
  wri_map(fp,"vismetop",prefer->fmap_vis_metop);
  fprintf(fp,"\n");

  fprintf(fp,"scandir_correct=%c\n",(prefer->scandir_correct? 'y' : 'n'));
  fprintf(fp,"keep_pic_in_mem=%c\n",(prefer->low_mem? 'y' : 'n'));
  fprintf(fp,"avhrr_mode=%c\n",(prefer->avhrr_mode? 'y' : 'n'));
  fprintf(fp,"full_globe=%c\n",(prefer->full_globe? 'y' : 'n'));

  fprintf(fp,"del_olderthan=%d:%d\n",prefer->deldaymin[0],prefer->deldaymin[1]);
  fprintf(fp,"del_newerthan=%d\n",prefer->deldaymax);
  fprintf(fp,"del_roundday=%c\n",(prefer->roundday? 'y' : 'n'));

  fprintf(fp,"record_at_start=%c\n",(prefer->record_at_start? 'y' : 'n'));
  fprintf(fp,"delete_old_at_start=%c\n",(prefer->delete_old_at_start? 'y' : 'n'));

  fprintf(fp,"norad_file=%s\n",prefer->noradfile);
  fprintf(fp,"earthmap_file1=%s\n",prefer->earthmapfile1);
  fprintf(fp,"earthmap_file2=%s\n",prefer->earthmapfile2);

  fprintf(fp,"markpoint=%f,%f\n",prefer->lmark.lat,prefer->lmark.lon);

  fprintf(fp,"fixed_drawable=%c\n",(prefer->size_draw_limit? 'y' : 'n'));

  fprintf(fp,"update_cycle=%d\n",prefer->upd_cycle);
  fprintf(fp,"update_channel=%s\n",prefer->upd_chan);

  fprintf(fp,"viewer_jpeg=%s\n",prefer->prog_jpg);
  fprintf(fp,"viewer_pgm=%s\n",prefer->prog_pgm);
  fprintf(fp,"upside_down_jpg=%s\n",prefer->ud_jpg);
  fprintf(fp,"upside_down_pgm=%s\n",prefer->ud_pgm);

  fprintf(fp,"viewer_avi=%s\n",prefer->prog_playmovie);
  fprintf(fp,"movie_extern=%c\n",(prefer->extern_moviegen? 'y' : 'n'));
  fprintf(fp,"movie_encoder=%s\n",prefer->prog_genmovie);
  fprintf(fp,"execute_command=%s\n",prefer->run_cmd);

  fprintf(fp,"movie_encspeed=%d\n",prefer->speed);
  fprintf(fp,"bufr_extract=%s\n",prefer->prog_bufrextr);
  fprintf(fp,"bufrtbl_loc=%s\n",prefer->bufrtblloc);
  fprintf(fp,"lut_loc=%s\n",prefer->lut_dir);
  fprintf(fp,"lut_file=%s\n",prefer->lut_file);
  fprintf(fp,"anaglyph_depthchan=%s\n",prefer->depth_chan);
  fprintf(fp,"anaglyph_depthchan_invert=%c\n",(prefer->dchan_iv? 'y' : 'n'));


  fprintf(fp,"%s=%d\n",PART_WIDTH,prefer->geoarea[0].pp_norm.pct_width);
  fprintf(fp,"%s=%d\n",PART_WIDTH_OFF,prefer->geoarea[0].pp_norm.pct_woffset);
  fprintf(fp,"%s=%d\n",PART_HEIGHT,prefer->geoarea[0].pp_norm.pct_height);
  fprintf(fp,"%s=%d\n",PART_HEIGHT_OFF,prefer->geoarea[0].pp_norm.pct_hoffset);
  fprintf(fp,"\n");
  fprintf(fp,"%s=%s\n",PART_NAME,prefer->geoarea[0].part_name);
  fprintf(fp,"%s=%.2f\n",PART_LON_CENTER,prefer->geoarea[0].center.lon);
  fprintf(fp,"%s=%.2f\n",PART_LAT_CENTER,prefer->geoarea[0].center.lat);
  fprintf(fp,"%s=%.2f\n",PART_LON_DELTA,prefer->geoarea[0].delta.lon);
  fprintf(fp,"%s=%.2f\n",PART_LAT_DELTA,prefer->geoarea[0].delta.lat);
  fprintf(fp,"%s=%d\n",PART_POLDIR,prefer->geoarea[0].pol_dir);

  for (i=0; i<NRGEODEFS; i++)
  {
    fprintf(fp,"%s%d=\"%s\"\n",PART_NAME,i+1,prefer->geoarea[i].part_name);
    fprintf(fp,"%s%d=%.2f\n",PART_LON_CENTER,i+1,prefer->geoarea[i].center.lon);
    fprintf(fp,"%s%d=%.2f\n",PART_LAT_CENTER,i+1,prefer->geoarea[i].center.lat);
    fprintf(fp,"%s%d=%.2f\n",PART_LON_DELTA,i+1,prefer->geoarea[i].delta.lon);
    fprintf(fp,"%s%d=%.2f\n",PART_LAT_DELTA,i+1,prefer->geoarea[i].delta.lat);
    fprintf(fp,"%s%d=%d\n",PART_POLDIR,i+1,prefer->geoarea[i].pol_dir);
  }
  fprintf(fp,"\n");

  fprintf(fp,"alert_nr_recfiles=%d\n",prefer->max_nr_rfiles);
  fprintf(fp,"use_filename_format=%c\n",(prefer->use_fn_frmt? 'y' : 'n'));
  fprintf(fp,"filename_format=%s\n",prefer->fn_frmt);
  fprintf(fp,"split_destdir=%c\n",(prefer->split_destdir? 'y' : 'n'));

  fprintf(fp,"#XRIT archive Directory\n");
  fprintf(fp,"a %s\n",prefer->src_archdir);

  fprintf(fp,"#XRIT destination Directory\n");
  fprintf(fp,"d %s\n",prefer->dest_dir);

  fprintf(fp,"#XRIT source Directory list\n");
  fprintf(fp,"s %s\n",prefer->src_dir);

  if (clist) Save_PrefDirlist(clist,prefer,fp);

  fprintf(fp,"#XRIT Tellique Directory\n");
  fprintf(fp,"t %s\n",prefer->tlq_sdir);
  fprintf(fp,"T %s\n",prefer->tlq_pdir);

  fclose(fp);
  return 1;
}

void pref_to_gui(GtkWidget *wnd,DIRSEL *dirsel,PREFER *prefer)
{
  if (!wnd) return;
  Set_Adjust(wnd,LAB_WNDWIDTH,"%d",prefer->wndwidth);
  Set_Adjust(wnd,LAB_WNDHEIGHT,"%d",prefer->wndheight);
  Set_Adjust(wnd,LAB_PREVWNDWIDTH,"%d",prefer->pr_wndwidth);
  Set_Adjust(wnd,LAB_PREVWNDHEIGHT,"%d",prefer->pr_wndheight);
  Set_Adjust(wnd,LAB_LIVEWNDWIDTH,"%d",prefer->lv_wndwidth);
  Set_Adjust(wnd,LAB_LIVEWNDHEIGHT,"%d",prefer->lv_wndheight);
  Set_Adjust(wnd,LAB_FONTSIZE,"%d",prefer->font_size);
  Set_Adjust(wnd,LAB_FILMSIZE,"%d",prefer->film_size);
  Set_Adjust(wnd,LAB_UPDCYCLE,"%d",prefer->upd_cycle);
  Set_Entry(wnd,LAB_SHOWCHANPREF,prefer->upd_chan);
  Set_Entry(wnd,LAB_PPGM,prefer->prog_pgm);
  Set_Entry(wnd,LAB_UPGM,prefer->ud_pgm);
  Set_Entry(wnd,LAB_PJPG,prefer->prog_jpg);
  Set_Entry(wnd,LAB_UJPG,prefer->ud_jpg);
  Set_Entry(wnd,LAB_PAVI,"%s",prefer->prog_playmovie); // prog_playmovie may contain %s!
  Set_Entry(wnd,LAB_PMOVIE,"%s",prefer->prog_genmovie);
  Set_Entry(wnd,LAB_RUNCMD,"%s",prefer->run_cmd);
//  Set_Entry(wnd,LAB_SMOVIE,prefer->opt_speed);
  Set_Adjust(wnd,LAB_SMOVIE,"%d",MAX(1,prefer->speed));
  Set_Button(wnd,LAB_SCDIRCOR,prefer->scandir_correct);
  Set_Button(wnd,LAB_HOLDCHNK,prefer->low_mem);
  Set_Button(wnd,LAB_FIXDRAW,prefer->size_draw_limit);
  Set_Entry(wnd,LAB_DDIR,prefer->dest_dir);
  Set_Button(wnd,LAB_SPLITDESTDIR,prefer->split_destdir);
  Set_Entry(wnd,LAB_SDIR,prefer->src_dir);
  Set_Entry(wnd,LAB_LDIR,prefer->lut_dir);
  Set_Entry(wnd,LAB_ARCHDIR,prefer->src_archdir);
  Set_Entry(wnd,LAB_BUFRTBLLOC,prefer->bufrtblloc);
  Set_Entry(wnd,LAB_BUFRPROG,prefer->prog_bufrextr);

  Set_Entry(wnd,LAB_NORF,prefer->noradfile);
  Set_Entry(wnd,LAB_CITYLOC,prefer->placesfile);
  Set_Entry(wnd,LAB_EARTHMAP1,prefer->earthmapfile1);
  Set_Entry(wnd,LAB_EARTHMAP2,prefer->earthmapfile2);
  Set_Entry(wnd,LAB_ANADEPCH,prefer->depth_chan);
  Set_Button(wnd,LAB_ANADEPIV,prefer->dchan_iv);
  Set_Button(wnd,LAB_USEFNFRMT,prefer->use_fn_frmt);
  Set_Entry(wnd,LAB_FNFRMT,"%s",prefer->fn_frmt);

  if ((dirsel) && (dirsel->selrow>=0))
  {
     gtk_clist_select_row(GTK_CLIST(dirsel->list),dirsel->selrow,0);
  }
}

#endif
