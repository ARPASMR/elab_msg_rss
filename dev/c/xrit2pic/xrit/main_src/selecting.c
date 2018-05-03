/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include <string.h>
#include <math.h>
extern GLOBINFO globinfo;
extern PREFER prefer;
#define combine_chan(grp) (((globinfo.spm.compose) || (gen_rah) || (globinfo.spm.fire_detect)) && \
                           (grp->datatype!=DWDSAT) && (grp->datatype!=BUFR)) //(!strchr("DB",grp->h_or_l)))
/*******************************************************************
 * Selecting functions.
 ********************************************************************/
/* check if channel is selected ; return TRUE if so */
gboolean chan_selected(GtkWidget *window,char *chan)
{
#ifndef __NOGUI__
  GtkWidget *wndlist=NULL;
  wndlist=Find_Window(window,LAB_PROGRESS_TRANS);

  if ((wndlist) && (*chan) && (Find_Widget(wndlist,chan)))
  {
    if (Get_Button(wndlist,chan))
      return TRUE;               /* button exist and it is selected -> return 'selected' */
    else
      return FALSE;              /* button exist and it is NOT selected -> return 'not selected' */
  }
#endif

  return TRUE;                   /* button doesn't exist -> return 'selected' */
}

/*
 * tlen>0 for Polar sats (with undefined nr. of segments).
 * For non-polar: set max. # segments to 'infinite'.
 * For polar sats (tlen>0):
 *   segm_last==0 -> clip # segments to max. 102 minutes
 *   segm_last >0 -> take this (so > 102 minutes possible)
 */
int adapt_lastsegmnr(int tlen,int segm_frst,int segm_last)
{
  if (segm_last<=0)
  {
    tlen/=60;
    if ((tlen) && (segm_frst==0))
      segm_last=segm_frst+102/tlen;           // process max. 102 minutes
    else
      segm_last=999;                          // process "all"
  }
  segm_frst=MAX(segm_frst,1);
  segm_last=MAX(segm_last,segm_frst);         // process at least 1 segment

  return segm_last;
}

#define DISTMAX 999999999.
static int pos2segm(GROUP *grp,GEOAREA geoarea,int *sa,int *sb)
{
  SEGMENT *segm;
  float lon1,lat1,lon2,lat2;
  float slon1,slat1,slon2,slat2,slata;
  int s1,s2;
  float dist1=DISTMAX;
  float dist=DISTMAX;
  int ret=0;
  lon1=geoarea.center.lon-geoarea.delta.lon;
  lon2=geoarea.center.lon+geoarea.delta.lon;
  lat1=geoarea.center.lat-geoarea.delta.lat;
  lat2=geoarea.center.lat+geoarea.delta.lat;
  s1=s2=0;
  *sa=*sb=0;
  load_kepler(grp);
  for (segm=grp->chan->segm; segm; segm=segm->next)
  {
////printf("AxA: %f\n",segm->orbit.subsat_start.lon);
//    if (segm->xh.segment>=grp->segm_first)  // apart startpunt nemen!
    {
      slon1=segm->orbit.subsat_start.lon;
      slon2=segm->orbit.subsat_end.lon;
      slat1=segm->orbit.subsat_start.lat;
      slat2=segm->orbit.subsat_end.lat;
      if ((((slon1>lon1) && (slon1<lon2)) &&
           ((slat1>lat1) && (slat1<lat2))) ||
          (((slon2>lon1) && (slon2<lon2)) &&
           ((slat2>lat1) && (slat2<lat2))))
      {
        if (!s1)
        {
          s1=segm->xh.segment;
          slata=slat1;
        }
        s2=0;
        dist=MIN(dist,pow(slon1-geoarea.center.lon,2.)+pow(slat1-geoarea.center.lat,2.));
      }
      else if ((s1) && (!s2))
      {
        s2=segm->xh.segment-1;
        if (((geoarea.pol_dir<0) && (slat1<slata)) || ((geoarea.pol_dir>0) && (slat1>slata)))
        {
          s1=s2=0;
        }
        else if (dist<dist1)
        {
          dist1=dist;
          *sa=s1;
          *sb=s2;
          s1=s2=0;
          ret=1;
        }
        dist=DISTMAX;
      }
      else s1=s2=0;
    }
  }
  return ret;
}

/*
 * Get next image to process. 
 * igrp!=NULL -> init
 * igrp==NULL -> get next image
 */
GROUP *get_selected_image(GROUP *igrp,GENMODES *gmode)
{
  static GROUP *grp;
  static CHANNEL *chan;
  static SEGMENT *segm;
  static int cstate,sstate;

  GROUP *grpsel=NULL;
  CHANNEL *chansel=NULL;
  SEGMENT *segmsel;
  int segm_frst=0;
  int segm_last=0;
/* Initialize selector */
  if (igrp)
  {
    grp=igrp;
    chan=NULL;
    segm=NULL;
    cstate=sstate=0;

    set_selected(grp,gmode);

    return NULL;
  }

/* Get next image */
  for (; grp; grp=grp->next)
  {
    gboolean gen_rah=((gmode) && (gmode->otype=='f') && (grp->h_or_l) && (strchr("GAM",grp->h_or_l)));
    gboolean chan_combined=combine_chan(grp);

/* Return if this group is selected and if multi-channel compose is active */
    if ((grp->selected) && (chan_combined))
    {
      grpsel=Copy_Grp_Rng(grp,segm_frst,segm_last);
      grpsel->compose=globinfo.spm.compose;
      grpsel->gen_rah=gen_rah;
      grp=grp->next;
      return grpsel;
    }
    
/* Search next selected channel */
    if (!chan) chan=grp->chan; 
    while (chan)
    {

/* Get this channel if selected */
      if ((chan->selected) || ((grp->selected) && (!chan_combined)))
      {
        grpsel=Create_Grp(NULL);         /* create new group to copy selected chan to */
        *grpsel=*grp;
        grpsel->prev=grpsel->next=NULL;
        grpsel->chan=NULL;
        grpsel->pro_epi=NULL;
        grpsel->org=grp;
        grpsel->pc.ypos_tbl[0]=grpsel->pc.ypos_tbl[1]=NULL;
        Add_Chan(&grpsel->chan,Copy_Chan_Rng(chan,segm_frst,segm_last));
        Add_Chan(&grpsel->pro_epi,Copy_Chan(grp->pro_epi));  

/* Prepair for next entry */
        chan=chan->next;
        if (!chan) grp=grp->next; 

        return grpsel;
      }

      grpsel=NULL;
      chansel=NULL;

/* Search next selected segment */
      if (!segm) segm=chan->segm;
      while (segm)
      {

/* Get this segment if selected and add to new group/channel */
        if (segm->selected)
        {

/* Create new group to add selected segment */
          if (!grpsel)
          {
            grpsel=Create_Grp(NULL);            /* create new group ... */
            *grpsel=*grp;
            grpsel->prev=grpsel->next=NULL;
            grpsel->chan=NULL;
            grpsel->pro_epi=NULL;
            grpsel->pc.ypos_tbl[0]=grpsel->pc.ypos_tbl[1]=NULL;
          }
/* Create new channel to add selected segment */
          if (!chansel)
          {
            chansel=Create_Chan(&grpsel->chan); /* add channel to copy selected segm to */

            *chansel=*chan;
            strcpyd(&chansel->chan_name,chansel->chan_name);
            strcpyd(&chansel->data_src,chansel->data_src);
            chansel->prev=chansel->next=NULL;
            chansel->segm=NULL;
            chansel->pro=NULL;
            chansel->epi=NULL;
          }
          segmsel=Copy_Segm(segm);
          segmsel->chan=chansel;

          Add_Segm(&chansel->segm,segmsel);
          Add_Segm(&chansel->pro,Copy_Segm(chan->pro));
          Add_Segm(&chansel->epi,Copy_Segm(chan->epi));
        }


/* Return if segment group is generated and current segment isn't selected */
        if ((grpsel) && ((!segm->selected) || (!segm->next)))
        {
          segm=segm->next;                          /* prepair for next entry */

/* Correct segment range (only if limited amount of segments are choosen) */
          {
            SEGMENT *s;
            CHANNEL *ch;
            for (ch=grpsel->chan; ch; ch=ch->next)
            {
              for (s=ch->segm; ((s) && (s->next)); s=s->next); 
              ch->seq_start=ch->segm->xh.segment;
              ch->seq_end=s->xh.segment;
            }

            if ((grpsel->chan) && (grpsel->chan->segm))
              grpsel->grp_tm=grpsel->chan->segm->xh.time;
          }

          /* prepair for next entry */
          if (!segm)  chan=chan->next; 
          if (!chan)  grp=grp->next;

          return grpsel;
        }
/* prepair for next entry */
        segm=segm->next;

      } /* for (; segm; segm=segm->next) */

      chan=chan->next;

    } /* for (; chan; chan=chan->next) */
  } /* for (; grp; grp=grp->next) */
  return NULL;

}

CHANNEL *selected_in_group(GROUP *grp)
{
  CHANNEL *chan;
  for (chan=grp->chan; chan; chan=chan->next)
    if (chan->selected) break;
  return chan;
}

/*
 * Get next item to process. 
 * igrp!=NULL -> init
 * igrp==NULL -> get next image
 */
GROUP *get_selected_item(GROUP *igrp,GENMODES *gmode)
{
  return get_selected_item_rng(igrp,gmode,FALSE);
}

/*
 * segm_selected==FALSE: all segments for GEO, max. 10 minutes for polar.
 * segm_selected==TRUE : use grp->segm_first/grp->segm_last
 */
GROUP *get_selected_item_rng(GROUP *igrp,GENMODES *gmode,gboolean segm_selected)
{
  static GROUP *grp;
  static CHANNEL *chan;
  static SEGMENT *segm;
  static int cstate,sstate;
  int segm_frst,segm_last;
  
  GROUP *grpsel=NULL;
  CHANNEL *chansel=NULL,*schan;
  SEGMENT *segmsel;

/* Initialize selector */
  if (igrp)
  {
    grp=igrp;
    chan=NULL;
    segm=NULL;
    cstate=sstate=0;
    set_selected(grp,gmode);
    return NULL;
  }

// Only allow segment selection if GEO and no 'europe' selected, or if polar.

/* Get next item */
  for (; grp; grp=grp->next)
  {
    gboolean gen_rah=((gmode) && (gmode->otype=='f') && (grp->h_or_l) && (strchr("GAM",grp->h_or_l)));
    gboolean chan_combined=combine_chan(grp);
#define XXX
#ifdef XXX

    if (grp->orbittype==POLAR)
    {
      if (segm_selected)
      {                            // segmentrange, no europe or no geostat.
        if ((gmode) && (gmode->area_nr))
        {
          if ((gmode->area_nr>0) && (gmode->area_nr<=NRGEODEFS))
            gmode->geoarea=prefer.geoarea[gmode->area_nr-1]; 
          if (pos2segm(grp,gmode->geoarea,&segm_frst,&segm_last))
          {
            grp->segm_first=segm_frst-1;
            grp->segm_last=segm_last-1;
          }
          else continue;

        }
        else
        {
          segm_frst=grp->segm_first;
          segm_last=grp->segm_last;
        }
        if ((grp) && (grp->chan) && (grp->chan->segm))
          segm_last=adapt_lastsegmnr(grp->chan->segm->xh.tlen, segm_frst, segm_last);
        if ((segm_last) && (!segm_frst)) segm_frst=1;
      }
      else
      {
        segm_frst=1;
        segm_last=0;  // causes max. of 102 minutes, see adapt_lastsegmnr
        if ((grp) && (grp->chan) && (grp->chan->segm))
          segm_last=adapt_lastsegmnr(grp->chan->segm->xh.tlen, segm_frst, segm_last);
      }
    }
    else   // GEO
    {
      if ((segm_selected) && (!globinfo.area_nr))
      {                            // segmentrange, no europe or no geostat.
        segm_frst=grp->segm_first;
        segm_last=grp->segm_last;
        if ((segm_last) && (!segm_frst)) segm_frst=1;
        if ((segm_frst) && (!segm_last)) segm_last=grp->chan->segm->xh.seq_end;
      }
      else if ((grp->chan) && (grp->chan->segm) && (grp->chan->segm->xh.file_type==2))
      {                           // text (service, admin messages)
        segm_frst=0;
        segm_last=0;  // select everything
      }
      else
      {
        segm_frst=0;
        segm_last=0;  // select everything
      }
    }
#else
    
  allow_segmselection=((!globinfo.europe) || (grp->orbittype==POLAR));
    if ((segm_selected) && (allow_segmselection))
    {                            // segmentrange, no europe or no geostat.
      segm_frst=grp->segm_first;
      segm_last=grp->segm_last;
//      if (grp->orbittype==POLAR)
//      if ((grp) && (grp->chan) && (grp->chan->segm))
//        segm_last=adapt_lastsegmnr(grp->chan->segm->xh.tlen, segm_frst, segm_last);
      if ((segm_last) && (!segm_frst)) segm_frst=1;
    }
    else if ((grp->chan) && (grp->chan->segm) && (grp->chan->segm->xh.file_type==2))
    {                           // text (service, admin messages)
      segm_frst=0;
      segm_last=0;  // select everything
    }
    else if (grp->orbittype==POLAR)
    {
      segm_frst=1;
      segm_last=0;  // causes max. of 102 minutes, see adapt_lastsegmnr
      segm_last=adapt_lastsegmnr(grp->chan->segm->xh.tlen, segm_frst, segm_last);
    }
    else
    {
      segm_frst=0;
      segm_last=0;  // select everything
    }
#endif

/* Return if this group is selected and if multi-channel compose is active */
    if ((grp->selected) && (chan_combined))
    {
//      segm_last=adapt_lastsegmnr(grp->chan->segm->xh.tlen, segm_frst, segm_last);
      grpsel=Copy_Grp_Rng(grp,segm_frst,segm_last);
      grpsel->compose=globinfo.spm.compose;
      grpsel->gen_rah=gen_rah;
      grp=grp->next;

      if ((grpsel->chan) && (grpsel->chan->segm))
        grpsel->grp_tm=grpsel->chan->segm->xh.time;
      return grpsel;
    }

/* Return if a channel in this group is selected and if fire_detect is active */
    if ((selected_in_group(grp)) && (globinfo.spm.fire_detect))
    {
//      segm_last=adapt_lastsegmnr(grp->chan->segm->xh.tlen, segm_frst, segm_last);
      grpsel=Copy_Grp_Rng(grp,segm_frst,segm_last);
      if ((grpsel->chan) && (grpsel->chan->segm))
        grpsel->grp_tm=grpsel->chan->segm->xh.time;
      grp=grp->next;
      return grpsel;
    }
    
/* Search next selected channel */
    if (!chan) { chan=grp->pro_epi; cstate='P'; }
    if (!chan) { chan=grp->chan;    cstate='C'; }

    while (chan)
    {
/* Get this channel if selected */
      if ((chan->selected) || ((grp->selected) && (!chan_combined)))
      {
        grpsel=Create_Grp(NULL);         /* create new group to copy selected chan to */
        *grpsel=*grp;
        grpsel->prev=grpsel->next=NULL;
        grpsel->chan=NULL;
        grpsel->pro_epi=NULL;
        grpsel->org=grp;
        grpsel->pc.ypos_tbl[0]=grpsel->pc.ypos_tbl[1]=NULL;

        switch(cstate)
        {
          case 'P':
            Add_Chan(&grpsel->pro_epi,Copy_Chan(chan));
          break;
          default :
//            segm_last=adapt_lastsegmnr(grp->chan->segm->xh.tlen, segm_frst, segm_last);
            Add_Chan(&grpsel->chan,Copy_Chan_Rng(chan,segm_frst,segm_last));
/* Add depth channel in case it is needed */ 
            if (globinfo.dim3)
            {
              if ((*prefer.depth_chan) && (chan->chan_name) && 
                  (strcmp(prefer.depth_chan,chan->chan_name)) &&   // depth channel = selected channel
                  (schan=Get_Chan(grp->chan,prefer.depth_chan)))
              {
                 Add_Chan(&grpsel->chan,Copy_Chan_Rng(schan,segm_frst,segm_last));
              }
            }
            Add_Chan(&grpsel->pro_epi,Copy_Chan(grp->pro_epi));
            if (grpsel->pro_epi) grpsel->pro_epi->selected=FALSE;
          break;
        }

/* Prepair for next entry */
        chan=chan->next;
        if ((!chan) && (cstate=='P')) { chan=grp->chan; cstate='C'; }
        if (!chan)                    { grp =grp->next; cstate= 0 ; }
        if ((grpsel->chan) && (grpsel->chan->segm))
          grpsel->grp_tm=grpsel->chan->segm->xh.time;
        return grpsel;
      }

      grpsel=NULL;
      chansel=NULL;

/* Search next selected segment */
      if (!segm) { segm=chan->pro;  sstate='p'; }    /* start with prologue */
      if (!segm) { segm=chan->epi;  sstate='e'; }    /* or with epilogue */
      if (!segm) { segm=chan->segm; sstate='c'; }      /* or with actual picture */ 

      while (segm)
      {
/* Get this segment if selected and add to new group/channel */
        if (segm->selected)
        {
/* Create new group to add selected segment */
          if (!grpsel)
          {
            grpsel=Create_Grp(NULL);            /* create new group ... */
            *grpsel=*grp;
            grpsel->prev=grpsel->next=NULL;
            grpsel->chan=NULL;
            grpsel->pro_epi=NULL;
            grpsel->pc.ypos_tbl[0]=grpsel->pc.ypos_tbl[1]=NULL;
          }
/* Create new channel to add selected segment */
          if (!chansel)
          {
            if ((sstate=='p') || (sstate=='e'))
              chansel=Create_Chan(&grpsel->pro_epi); /* add channel to copy selected segm to */
            else
              chansel=Create_Chan(&grpsel->chan); /* add channel to copy selected segm to */

            *chansel=*chan;
            strcpyd(&chansel->chan_name,chansel->chan_name);
            strcpyd(&chansel->data_src,chansel->data_src);
            chansel->prev=chansel->next=NULL;
            chansel->segm=NULL;
            chansel->pro=NULL;
            chansel->epi=NULL;
          }
          segmsel=Copy_Segm(segm);
          segmsel->chan=chansel;

          switch(sstate)
          {
            case 'p': Add_Segm(&chansel->pro,segmsel);  break;
            case 'e': Add_Segm(&chansel->epi,segmsel);  break;
            default : Add_Segm(&chansel->segm,segmsel); break;
          }
//          if ((grpsel->chan)&&(grpsel->chan->segm))
//            grpsel->grp_tm=grpsel->chan->segm->xh.time;  // correct starttime
        }


/* Return if segment group is generated and current segment isn't selected, or no next segment */
        if ((grpsel) && ((!segm->selected) || (!segm->next)))
        {
          segm=segm->next;                          /* prepair for next entry */
          if ((!segm) && (sstate=='p'))             /* cont. with epilogue */
          {
            sstate='e';
            segm=chan->epi;
          }
          if ((!segm) && (sstate=='e'))             /* cont. with picture */
          { 
            sstate='c';
            segm=chan->segm;
          }
/* Correct segment range (only if limited amount of segments are choosen) */
          {
            SEGMENT *s;
            CHANNEL *ch;
            for (ch=grpsel->chan; ch; ch=ch->next)
            {
              for (s=ch->segm; ((s) && (s->next)); s=s->next); 
              ch->seq_start=ch->segm->xh.segment;
              ch->seq_end=s->xh.segment;
            }
            if ((grpsel->chan) && (grpsel->chan->segm)) grpsel->grp_tm=grpsel->chan->segm->xh.time;
          }


          /* prepair for next entry */
          if (!segm) { chan=chan->next; sstate=0; }
          if ((!chan) && (cstate=='P')) { chan=grp->chan; cstate='C'; }
          if (!chan) { grp=grp->next; cstate=0; }

          return grpsel;
        }
/* prepair for next entry */
        segm=segm->next;
        if ((!segm) && (sstate=='p')) { sstate='e'; segm=chan->epi; } /* cont. with epilogue */
        if ((!segm) && (sstate=='e')) { sstate='c'; segm=chan->segm; }/* cont. with picture */

      } /* for (; segm; segm=segm->next) */

      chan=chan->next;
      if ((!chan) && (cstate=='P')) { cstate='C'; chan=grp->chan; }

    } /* for (; chan; chan=chan->next) */
  } /* for (; grp; grp=grp->next) */
  return NULL;
}

/* Determine # pics to generate */
int nr_pics(GtkWidget *window,GROUP *grp,GENMODES *gmode)
{
  GROUP *grp1=NULL;
  int n=0,npe=0;
  get_selected_image(grp,gmode);
  while ((grp1=get_selected_image(NULL,gmode)))
  {
    if (grp1->pro_epi) npe++;
    n++;                     /* was: else n++; gaat mis als geen pro_epi in 1e grp */
    Remove_Grps(grp1);
  }
  if (!n) n=npe;
  globinfo.nr_pics=n;
  return n;
}

int nr_pics_nonbufr(GtkWidget *window,GROUP *grp,GENMODES *gmode)
{
  GROUP *grp1=NULL;
  int n=0;
  get_selected_image(grp,gmode);
  while ((grp1=get_selected_image(NULL,gmode)))
  {
    if (grp1->h_or_l!='B') n++;
    Remove_Grps(grp1);
  }
  return n;
}
