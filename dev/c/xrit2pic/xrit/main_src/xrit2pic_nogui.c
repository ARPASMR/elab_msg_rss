/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * non-gui part
 ********************************************************************/
#include "xrit2pic.h"
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef __NOGUI__
  #include "xrit2pic_nogtk.h"
#endif

extern GLOBINFO globinfo;
extern GROUP *globgrp;

typedef struct to_select
{
  char *gsat,gtype,*gchan_name,*gsegm_range,*gdate,*gtime;
  gboolean pro,epi;
  gboolean compose;
  gboolean use_hrv_lum;
  gboolean gen_rah;
  gboolean anagl;
} TO_SELECT;


static void process_grp(GROUP *group,GENMODES *genmode,
                 void do_it(),int sel_grp(),int sel_chan(),int sel_segm())
{
  GROUP *grp;
  CHANNEL *chan;
  SEGMENT *segm;
  for (grp=group; grp; grp=grp->next)
  {
    if ((sel_grp) && (!sel_grp(grp))) continue;
    for (chan=grp->chan; chan; chan=chan->next)
    {
      if ((sel_chan) && (!sel_chan(chan))) continue;

      if ((sel_segm) && (sel_segm(NULL)))
      {
        for (segm=chan->segm; segm; segm=segm->next)
        {
          if ((sel_segm) && (!sel_segm(segm))) continue;
          if (do_it) do_it(grp,chan,segm);
        }
      }
      else
      {
        chan->nselected=TRUE;
        if (chan->pro) chan->pro->nselected=genmode->gen_pro; 
        if (chan->epi) chan->epi->nselected=genmode->gen_epi; 
        if (grp->pro_epi)
        {
          if (grp->pro_epi->pro) grp->pro_epi->pro->nselected=genmode->gen_pro;
          if (grp->pro_epi->epi) grp->pro_epi->epi->nselected=genmode->gen_epi;
        }
      }
    }
  }
}


void print_log(FILE *fp,char *frmt,...)
{
  char strtime[20];
  struct tm *tm;
  time_t t=time(NULL);
  va_list arg;
  if (!fp) fp=stdout;
  
  va_start(arg,frmt);
  *strtime=0;
  tm=gmtime(&t);

  if (tm)
    strftime(strtime,20,"%y-%m-%d %H:%M",tm);

  fprintf(fp,"[%s] ",strtime);

  vfprintf(fp,frmt,arg);

  va_end(arg);

}


static TO_SELECT ts;

static void extract_segmrange(char *range,int *min,int *max)
{
  char *p;
  if (!range) return;
  *min=atoi(range); *max=*min;
  p=strchr(range,'-');
  if (p) { p++; *max=atoi(p); }
}


static int sel_grp(GROUP *grp)
{
  char idate[20],itime[20];

  strftime(idate,10,"%y-%m-%d",&grp->grp_tm);
  strftime(itime,10,"%H:%M",&grp->grp_tm);

  if ((ts.gtype) && (ts.gtype!=grp->h_or_l)) return 0;
  if ((ts.gsat)  && (strcmpwild(grp->sat_src,ts.gsat))) return 0;
  if ((ts.gdate) && (strcmpwild(idate,ts.gdate))) return 0;
  if ((ts.gtime) && (strcmpwild(itime,ts.gtime))) return 0;
  if (ts.gen_rah) grp->nselected=TRUE;
  extract_segmrange(ts.gsegm_range,&grp->segm_first,&grp->segm_last);

  if (ts.compose)
  {
    grp->nselected=TRUE;
    grp->compose=TRUE;
    globinfo.hrv_lum=ts.use_hrv_lum;
  }
/* Group is selected. */

  return 1;
}

static int sel_chan(CHANNEL *chan)
{
  if (ts.gchan_name)
  {
    if (!chan->chan_name) return 0;
    if ((strcmpwild(chan->chan_name,ts.gchan_name))) return 0;
  }
/* Channel is selected. */
  if ((ts.pro) && (chan->pro)) chan->pro->nselected=TRUE;
  if ((ts.epi) && (chan->epi)) chan->epi->nselected=TRUE;
  return 1;
}

static int sel_segm(SEGMENT *segm)
{
  int min=1,max;
  if (!segm) return (int)ts.gsegm_range;
  extract_segmrange(ts.gsegm_range,&min,&max);

  if ((!ts.gsegm_range) || ((segm->xh.segment >=min) && (segm->xh.segment <=max)))
    segm->nselected=TRUE;
  return 0;
}


static void do_log(GENMODES *gmode,GROUP *grp,char *tfno,int err)
{
  char *action;
  FILE *fplog=gmode->fplog;
  if (gmode->test)
    action="Test: would extract";
  else
    action="Translated";

  switch(err)
  {
    case 0:
      {
        if (grp->chan)
        {
          if (globinfo.dim3)
          {
            if (grp->compose)
              print_log(fplog,"%s colour Anaglyph shift=%d, range %d...%d to %s\n",action,gmode->agl.shift_3d,gmode->agl.lmin,gmode->agl.lmax,tfno);
            else
              print_log(fplog,"%s Anaglyph shift=%d, range %d...%d to %s\n",action,gmode->agl.shift_3d,gmode->agl.lmin,gmode->agl.lmax,tfno);
          }
          else if (grp->compose)
            print_log(fplog,"%s colour pic to %s\n",action,tfno);
          else if (grp->gen_rah)
          {
            if (grp->has_kepler)
              print_log(fplog,"%s rah format to %s (Kepler data included)\n",action,tfno);
            else
              print_log(fplog,"%s rah format to %s\n",action,tfno);
          }
/*
veranderen: grp->combineer of zo?
          else if (grp->)
            print_log(fplog,"%s to %s\n",action,tfno);
*/
          else
            print_log(fplog,"%s %s to %s\n",action,grp->chan->chan_name,tfno);
        }
        else if (grp->pro_epi)
          print_log(fplog,"%s %s to %s\n",action,grp->pro_epi->chan_name,tfno);
      }
    break;
    case Exist_fn:
      print_log(fplog,"Warning: %s (%s)\n",
                           report_genpicerr(err,NULL),tfno);
    break;
    default:
      if ((tfno) && (*tfno))
        print_log(fplog,"Error %s while translating %s to %s.\n",
                         report_genpicerr(err,NULL),grp->chan->chan_name,tfno);
      else if (grp->compose)
        print_log(fplog,"Error %s in composed pic of %s\n",
                         report_genpicerr(err,NULL),grp->grp_time);
      else
        print_log(fplog,"Error %s in channel %s of %s\n",
                         report_genpicerr(err,NULL),grp->chan->chan_name,grp->grp_time);
    break;
  }
}

int nogui(GENMODES *genmodes,PREFER *prefer,gboolean test,gboolean log)
{
  FILE *fplog=stdout;
  char *logfile=NULL;
  memset(&ts,0,sizeof(ts));
  globinfo.overwrite_mode=genmodes->overwrite_mode;  /* 2=don't overwrite, 1=overwrite */
  if (genmodes->overlaytype)
  {
    globinfo.add_overlay=TRUE;
//    globinfo.overlaytype=(genmodes->overlaytype=='C'? 0 : genmodes->overlaytype); weggeh. 27-07-2006, waarom zat dit erin?
    globinfo.overlaytype=genmodes->overlaytype;
  }
#define LOGFILE "xrit2pic.log"
  if (log)
  {
    strcpyd(&logfile,globinfo.dest_dir);
    strcatd(&logfile,LOGFILE);
    fplog=fopen(logfile,"a");
    if (!fplog)
    {
      fprintf(stderr,"Error: Can't open logfile %s.\n",logfile);
      fplog=stdout;
    }
  }

  genmodes->log_func=do_log;
  genmodes->fplog=fplog;
  genmodes->test=test;

/* first load_list: read from donedir, first clear globgrp
   second load_list: read from maindir, don't clear
*/
  globgrp=Load_List_all(globgrp,NULL,NULL,0);

  if (!(globgrp))
  {
    fprintf(fplog,"ERROR: Can't read XRIT source directory '%s'\n",globinfo.src_dir);
    return 0;
  }
  fprintf(fplog,"====================================================\n");
  fprintf(fplog,"%d items (files) found.\n",globinfo.nrfiles);
  if (fplog!=stdout)
    fprintf(fplog,"Opened log file %s.\n",(logfile? logfile : LOGFILE));
  fprintf(fplog,"----------------------------------------------------\n");
  ts.gchan_name=genmodes->chan_name;
  ts.gsat=genmodes->sat;
  ts.gtype=genmodes->type;
  ts.gdate=genmodes->date;
  ts.gtime=genmodes->time;
  ts.gsegm_range=genmodes->segm_range;
  ts.pro=genmodes->gen_pro;
  ts.epi=genmodes->gen_epi;
  ts.compose=genmodes->spm.compose;
  ts.anagl=globinfo.dim3;
  if (genmodes->cformat=='r') ts.gen_rah=TRUE;
  ts.use_hrv_lum=genmodes->use_hrv_lum;
  globinfo.spm.compose=genmodes->spm.compose;

  if (genmodes->spm.compose_type!=map_cust)
  {
    if (genmodes->type=='A')              // AVHRR
      set_mapping_from_fixed(map_noaavis);
    else if (genmodes->type=='M')         // METOP
      set_mapping_from_fixed(map_metopvis);
    else if (genmodes->type=='L')         // LRIT
      set_mapping_from_fixed(map_vis_lmsg);
    else                                  // HRIT
      set_mapping_from_fixed(genmodes->spm.compose_type);
  }

  /* needed for avhrr rah: generate 1 file (see get_selected, chan_combined) */
  genmodes->otype='f';
  process_grp(globgrp,genmodes,NULL,sel_grp,sel_chan,sel_segm);
  gen_item(NULL,globgrp,genmodes,prefer);
  fprintf(fplog,"====================================================\n");

  if (fplog!=stdout) fclose(fplog);
  if (logfile) free(logfile);
  return 1;
}


