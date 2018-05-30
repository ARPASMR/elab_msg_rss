/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include <stdlib.h>
#include <string.h>

/*******************************************************************
 * database functions
 ********************************************************************/

/**********************************
 * Segments related funcs
 **********************************/
SEGMENT *Get_Segm(CHANNEL *chan,int n)
{
  SEGMENT *s;
  for (s=chan->segm; s; s=s->next)
  {
    if (s->xh.segment==n) return s;
  }
  return NULL;
}

void Add_Segm(SEGMENT **si,SEGMENT *sn)
{
  SEGMENT *s;
  if (!si) return;

  s=*si;
  if (s)
  {
    while (s->next) s=s->next;
    s->next=sn;
    sn->prev=s;
  }
  else
  {
    *si=sn;
  }
}

SEGMENT *Create_Segm(SEGMENT **si)
{
  SEGMENT *sn;
  sn=calloc(1,sizeof(*sn));
  Add_Segm(si,sn);
  return sn;
}

void Remove_Segm(SEGMENT *segm)
{
  if (!segm) return;
  if (segm->prev)
  {
    segm->prev->next=segm->next;
  }
  else if ((segm->chan) && (segm->chan->segm==segm))
  {
    segm->chan->segm=segm->next;
  }
  else if ((segm->chan) && (segm->chan->pro==segm))
  {
    segm->chan->pro=segm->next;
  }
  else if ((segm->chan) && (segm->chan->epi==segm))
  {
    segm->chan->epi=segm->next;
  }

  if (segm->next) segm->next->prev=segm->prev;
  if (segm->chnk) free(segm->chnk);
  if (segm->ovlchnk) free(segm->ovlchnk);

  if (segm->pfn) free(segm->pfn);
  if (segm->fn) free(segm->fn);
  tfreenull(&segm->xh.img_obs_time);
  free(segm);
}

void Remove_Segms(SEGMENT *segm)
{
  SEGMENT *segmnext;

  if (!segm) return;
  for (; segm; segm=segmnext)
  {
    segmnext=segm->next;
    Remove_Segm(segm);
  }
}

int Delete_Segm(SEGMENT *segm,GtkCTree *ctree)
{
  int removed=0;
  if ((segm->pfn) && (segm->where!=no_file))
  {
    if (ctree)
    {    
      if (!(remove(segm->pfn))) removed=1;
    }
    else
      removed=1;
  }
  if (ctree)   // do actual remove if ctree defined (tree item and segm)
  {
#ifndef __NOGUI__ 
    if (segm->node) gtk_ctree_remove_node(ctree,segm->node);
#endif
    Remove_Segm(segm);
  }
  return removed;
}


int Delete_Segms(SEGMENT *segm,GtkCTree *ctree)
{
  int nrdel=0;
  SEGMENT *segmnext;
  for (; segm; segm=segmnext)
  {
    segmnext=segm->next;
    nrdel+=Delete_Segm(segm,ctree);
  }
  return nrdel;
}

SEGMENT *Copy_Segm(SEGMENT *isegm)
{
  SEGMENT *segm;
  if (!isegm) return NULL;
  segm=Create_Segm(NULL);
  *segm=*isegm;
  segm->chnk=NULL;           // pointer: Set NULL or reallocate!
  segm->ovlchnk=NULL;        // pointer: Set NULL or reallocate!
  segm->chan=NULL;           /* expected segm will be placed under different chan! */
/* Overwrite pointers with new allocated mem and copy content. Needed in case of free'ing segm */
  strcpyd(&segm->pfn,isegm->pfn);
  strcpyd(&segm->fn,isegm->fn);
  memcpy(&segm->xh,&isegm->xh,sizeof(isegm->xh));
  segm->xh.img_obs_time=NULL;
  if (isegm->xh.img_obs_time) strcpyd(&segm->xh.img_obs_time,isegm->xh.img_obs_time);
  segm->prev=segm->next=NULL;
  return segm;
}

void Swap_Segm(SEGMENT *s1,SEGMENT *s2)
{
  SEGMENT *s0,*s3;
  s0=s1->prev;
  s3=s2->next;

/* s0 <-> s1 <-> s2 <-> s3 ==> s0 <-> s2 <-> s1 <-> s3 */
  if (s0) s0->next=s2;    /* s0 --> s1 ==> s0 --> s2 */
  s1->next=s3;            /* s1 --> s2 ==> s1 --> s3 */
  s2->next=s1;            /* s2 --> s3 ==> s2 --> s1 */

  if (s3) s3->prev=s1;    /* s2 <-- s3 ==> s1 <-- s3 */
  s2->prev=s0;            /* s1 <-- s2 ==> s0 <-- s2 */
  s1->prev=s2;            /* s0 <-- s1 ==> s2 <-- s1 */
}

SEGMENT *Sort_Segm(SEGMENT *s)
{
  SEGMENT *s1,*s1next;
  int sorted=0;
  while (!sorted)
  {
    sorted=1;
    for (s1=s; s1; s1=s1next)
    {
      if (!(s1next=s1->next)) continue;    /* no 'next' so end of this sort-part */
      if (s1->xh.segment > s1next->xh.segment)
      {
        s1next=s1next->next;               /* current and next done */
        Swap_Segm(s1,s1->next);             /* swap current and next */
        if (s1==s) s=s1->prev; 
        sorted=0;                              /* not sorted */
      }
    }
  }

  return s;
}


/**********************************
 * Channel related funcs
 **********************************/
void Add_Chan(CHANNEL **si,CHANNEL *sn)
{
  CHANNEL *s=NULL;
  if (!si) return;

  s=*si;
  if (s)
  {
    while (s->next) s=s->next;
    s->next=sn;
    sn->prev=s;
  }
  else
  {
    *si=sn;
  }
}

CHANNEL *Create_Chan(CHANNEL **si)
{
  CHANNEL *sn;
  sn=calloc(1,sizeof(*sn));
  Add_Chan(si,sn);

  return sn;
}

CHANNEL *Get_Chan(CHANNEL *chan,char *name)
{
  if (!name) return NULL;
  for (; chan; chan=chan->next)
  {
    if (chan->chan_name)
      if (!strcmp(chan->chan_name,name)) return chan;
  }
  return NULL;
}

gboolean Chan_Contributes(GROUP *grp,char *name)
{
  CHANNEL *chan=Get_Chan(grp->chan,name);
  if (!chan) return FALSE;
  if ((chan->r) || (chan->g) || (chan->b) || (chan->lum)) return TRUE;
  return FALSE;
}

void Remove_Chan(CHANNEL *chan)
{
  if (!chan) return;
  if (chan->prev)
  {
    chan->prev->next=chan->next;
  }
  else if ((chan->group) && (chan->group->chan==chan))
  {
    chan->group->chan=chan->next;
  }
  else if ((chan->group) && (chan->group->pro_epi==chan))
  {
    chan->group->pro_epi=chan->next;
  }
  
  if (chan->next) chan->next->prev=chan->prev;

  Remove_Segms(chan->pro);
  Remove_Segms(chan->epi);
  Remove_Segms(chan->segm);
  if (chan->chan_name) free(chan->chan_name);
  if (chan->data_src) free(chan->data_src);
  if (chan->cal.caltbl[0]) free(chan->cal.caltbl[0]); 
  if (chan->cal.caltbl[1]) free(chan->cal.caltbl[1]); 
  free(chan);
}

void Remove_Chans(CHANNEL *chan)
{
  CHANNEL *channext;
  if (!chan) return;

  for (; chan; chan=channext)
  {
    channext=chan->next;
    Remove_Chan(chan);
  }
}

int Delete_Chan(CHANNEL *chan,GtkCTree *ctree)
{
  int nrdel=0;
  nrdel+=Delete_Segms(chan->pro,ctree);
  nrdel+=Delete_Segms(chan->epi,ctree);
  nrdel+=Delete_Segms(chan->segm,ctree);
  if (ctree)     // do actual delete if ctree defined (tree and chan)
  {
#ifndef __NOGUI__ 
    if (chan->node) gtk_ctree_remove_node(ctree,chan->node);
#endif
    Remove_Chan(chan);
  }
  return nrdel;
}

int Delete_Chans(CHANNEL *chan,GtkCTree *ctree)
{
  int nrdel=0;
  CHANNEL *channext;
  for (; chan; chan=channext)
  {
    channext=chan->next;
    nrdel+=Delete_Chan(chan,ctree);
  }
  return nrdel;
}

int Copy_Segm_Range(CHANNEL *ichan,CHANNEL *ochan,int s_strt,int s_end,gboolean all)
{
  SEGMENT *isegm,*segm;
  int nr=0;
  for (isegm=ichan->segm; isegm; isegm=isegm->next)
  {
    if (!all)   // skip segments outside range 
    {
//if (isegm->orbit.subsat_start.lat < 60) continue; // gaat niet goed: gaten in segm. reeks!
      if (isegm->xh.segment < s_strt) continue;
      if (isegm->xh.segment > s_end) continue;
    }
    segm=Copy_Segm(isegm);
    segm->chan=ochan;
    Add_Segm(&ochan->segm,segm);
    nr++;
  }
  return nr;
}


CHANNEL *Copy_Chan_Rng1(CHANNEL *ichan,int s_strt,int s_end,gboolean all)
{
  CHANNEL *chan;
  SEGMENT *isegm,*segm;
  if (!ichan) return NULL;
  chan=Create_Chan(NULL);
  *chan=*ichan;
  chan->prev=chan->next=NULL;
  chan->group=NULL;
  chan->segm=NULL;
  chan->pro=NULL;
  chan->epi=NULL;
  chan->chan_name=NULL;
  if (ichan->chan_name) strcpyd(&chan->chan_name,ichan->chan_name);
  chan->data_src=NULL;
  if (ichan->data_src) strcpyd(&chan->data_src,ichan->data_src);

  chan->cal.caltbl[0]=NULL;
  chan->cal.caltbl[1]=NULL;
  chan->cal.nrcalpoints=0;

// If channel=HRV: assume selected segments based on non-HRV, so correct.
  if ((ichan->chan_name) && (!strcmp(ichan->chan_name,"HRV")))
  {
    if (s_strt)
      s_strt=(s_strt-1)*3+1;
    if (s_end)
      s_end=s_end*3;
  }
  Copy_Segm_Range(ichan,chan,s_strt,s_end,all);
  if (all)
  {
    chan->seq_start=ichan->seq_start;
    chan->seq_end=ichan->seq_end;
  }
  else
  {
    chan->seq_start=MAX(ichan->seq_start,s_strt);
    chan->seq_end=MIN(ichan->seq_end,s_end);
  }

  for (isegm=ichan->pro; isegm; isegm=isegm->next)
  {
    segm=Copy_Segm(isegm);
    segm->chan=chan;
    Add_Segm(&chan->pro,segm);
  }

  for (isegm=ichan->epi; isegm; isegm=isegm->next)
  {
    segm=Copy_Segm(isegm);
    segm->chan=chan;
    Add_Segm(&chan->epi,segm);
  }
  return chan;
}

CHANNEL *Copy_Chan_Rng(CHANNEL *ichan,int s_strt,int s_end)
{
  if ((s_strt==0) && (s_end==0))
    return Copy_Chan_Rng1(ichan, s_strt, s_end,TRUE);
  else
    return Copy_Chan_Rng1(ichan, s_strt, s_end,FALSE);
}

CHANNEL *Copy_Chan(CHANNEL *ichan)
{
  return Copy_Chan_Rng1(ichan,0,0,TRUE);
}

void Swap_Chan(CHANNEL *s1,CHANNEL *s2)
{
  CHANNEL *s0,*s3;
  s0=s1->prev;
  s3=s2->next;

/* s0 <-> s1 <-> s2 <-> s3 ==> s0 <-> s2 <-> s1 <-> s3 */
  if (s0) s0->next=s2;    /* s0 --> s1 ==> s0 --> s2 */
  s1->next=s3;            /* s1 --> s2 ==> s1 --> s3 */
  s2->next=s1;            /* s2 --> s3 ==> s2 --> s1 */

  if (s3) s3->prev=s1;    /* s2 <-- s3 ==> s1 <-- s3 */
  s2->prev=s0;            /* s1 <-- s2 ==> s0 <-- s2 */
  s1->prev=s2;            /* s0 <-- s1 ==> s2 <-- s1 */
}

CHANNEL *Sort_Chan(CHANNEL *s)
{
  CHANNEL *s1,*s1next;
  int sorted=0;

  while (!sorted)
  {
    sorted=1;

    for (s1=s; s1; s1=s1next)
    {
      if (!(s1next=s1->next)) continue;    /* no 'next' so end of this sort-part */

      if (s1->chan_nr > s1->next->chan_nr)
      {
        s1next=s1next->next;               /* current and next done */

        Swap_Chan(s1,s1->next);             /* swap current and next */
        if (s1==s) s=s1->prev;

        sorted=0;                              /* not sorted */
      }
    }
  }

  for (s1=s; s1; s1=s1->next)
  {
    s1->segm=Sort_Segm(s1->segm);
  }
  return s;
}


/**********************************
 * Group related funcs
 **********************************/
GROUP *Create_Grp(GROUP **si)
{
  GROUP *sn,*s=NULL;
  if (si) s=*si;
  sn=calloc(1,sizeof(*sn));

  if (s)
  {
    while (s->next) s=s->next;
    s->next=sn;
    sn->prev=s;
  }
  else if (si)
  {
    *si=sn;
  }
  return sn;
}

/* Remove and destroy 1 group from chain.
   Return: first existing grp in chain 
            (only changes if first grp was removed)
*/
GROUP *Remove_Grp(GROUP *grp)
{
  GROUP *grp_first=NULL;
  if (!grp->prev)
  {
    grp_first=grp->next;
  }
  else
  {
    grp_first=grp->prev;
    while (grp_first->prev)
      grp_first=grp_first->prev;
  }
  
  if (grp->prev) grp->prev->next=grp->next;
  if (grp->next) grp->next->prev=grp->prev;

  Remove_Chans(grp->pro_epi);
  Remove_Chans(grp->chan);
  if (grp->rgbpicstr[0]) free(grp->rgbpicstr[0]);
  if (grp->rgbpicstr[1]) free(grp->rgbpicstr[1]);
  if (grp->rgbpicstr[2]) free(grp->rgbpicstr[2]);
  if (grp->rgbpicstr[3]) free(grp->rgbpicstr[3]);
  if (grp->line_in_mem)  free(grp->line_in_mem);
  if (grp->pc.ypos_tbl[0]) free(grp->pc.ypos_tbl[0]);
  grp->pc.ypos_tbl[0]=NULL;
  if (grp->pc.ypos_tbl[1]) free(grp->pc.ypos_tbl[1]);
  grp->pc.ypos_tbl[1]=NULL;
  free(grp);
  return grp_first;
}

/* Destroy all groups in chain */
/* Destroy all groups in chain */
void Remove_Grps(GROUP *grp)
{
  GROUP *grpnext;

  for (; grp; grp=grpnext)
  {
    grpnext=grp->next;
    Remove_Grp(grp);
  }
}

int Delete_Grp(GROUP *grp,GtkCTree *ctree)
{
  int nrdel=0;
  if (strchr("GAM",grp->h_or_l))
  {
    if ((grp->chan) && (grp->chan->segm))
      nrdel=Delete_Chan(grp->chan,ctree);
  }
  else
  {
    nrdel+=Delete_Chans(grp->chan,ctree);
    nrdel+=Delete_Chans(grp->pro_epi,ctree);
  }
  if (ctree)  // if ctree defined then do actual remove (in tree and group)
  {
#ifndef __NOGUI__ 
    if (grp->node) gtk_ctree_remove_node(ctree,grp->node);
#endif
    Remove_Grp(grp);
  }
  return nrdel;
}


/* Delete all groups starting with 'grp' */
int Delete_Grps(GROUP *grp,GtkCTree *ctree)
{
  int nrdel=0;
  GROUP *grpnext;
  for (; grp; grp=grpnext)
  {
    grpnext=grp->next;
    nrdel+=Delete_Grp(grp,ctree);
  }
  return nrdel;
}


GROUP *Copy_Grp_Rng1(GROUP *igrp,int s_strt,int s_end,gboolean all)
{
  GROUP *grp;
  CHANNEL *ichan,*chan;
  grp=Create_Grp(NULL);
  *grp=*igrp;
  grp->prev=grp->next=NULL;
  grp->org=igrp;

  grp->chan=NULL;
  for (ichan=igrp->chan; ichan; ichan=ichan->next)
  {
    chan=Copy_Chan_Rng1(ichan,s_strt,s_end,all);
    Add_Chan(&grp->chan,chan);
    chan->group=grp;
  }

  grp->pro_epi=NULL;
  for (ichan=igrp->pro_epi; ichan; ichan=ichan->next)
  {
    chan=Copy_Chan(ichan);
    Add_Chan(&grp->pro_epi,chan);
    chan->group=grp;
  }

  return grp;
}

GROUP *Copy_Grp_Rng(GROUP *igrp,int s_strt,int s_end)
{
  if ((s_strt==0) && (s_end==0))
    return Copy_Grp_Rng1(igrp, s_strt, s_end,TRUE);
  else
    return Copy_Grp_Rng1(igrp, s_strt, s_end,FALSE);
}

GROUP *Copy_Grp(GROUP *igrp)
{
  return Copy_Grp_Rng1(igrp,0,0,TRUE);
}

void Swap_Grp(GROUP *s1,GROUP *s2)
{
  GROUP *s0,*s3;
  s0=s1->prev;
  s3=s2->next;

/* s0 <-> s1 <-> s2 <-> s3 ==> s0 <-> s2 <-> s1 <-> s3 */
  if (s0) s0->next=s2;    /* s0 --> s1 ==> s0 --> s2 */
  s1->next=s3;            /* s1 --> s2 ==> s1 --> s3 */
  s2->next=s1;            /* s2 --> s3 ==> s2 --> s1 */

  if (s3) s3->prev=s1;    /* s2 <-- s3 ==> s1 <-- s3 */
  s2->prev=s0;            /* s1 <-- s2 ==> s0 <-- s2 */
  s1->prev=s2;            /* s0 <-- s1 ==> s2 <-- s1 */
}

GROUP *Sort_Grp(GROUP *s)
{
  GROUP *s1,*s1next;
  int sorted=0;

  while (!sorted)
  {
    sorted=1;
    for (s1=s; s1; s1=s1next)
    {
      if (!(s1next=s1->next)) continue;    /* no 'next' so end of this sort-part */
      if ((s1->grp_time) && (s1->next->grp_time))
      {
        if ((strcmp(s1->grp_time,s1->next->grp_time)) > 0)
        {
          s1next=s1next->next;               /* current and next done */
          Swap_Grp(s1,s1->next);             /* swap current and next */
          if (s1==s) s=s1->prev; 
          sorted=0;                              /* not sorted */
        }
      }
    }
  }

  for (s1=s; s1; s1=s1->next)
  {
    s1->chan=Sort_Chan(s1->chan);
  }

  return s;
}

/* free all chuncks */
void free_segmchnk(SEGMENT *segm)
{
  if (segm->chnk)
  {
    free(segm->chnk);
    segm->chnk=NULL;
  }
  if (segm->ovlchnk)
  {
    free(segm->ovlchnk);
    segm->ovlchnk=NULL;
  }
}

void free_chnks(GROUP *grp)
{
  CHANNEL *chan;
  SEGMENT *segm;
  for (; grp; grp=grp->next)
  {
    for (chan=grp->chan; chan; chan=chan->next)
    {
      for (segm=chan->segm; segm; segm=segm->next)
      {
        free_segmchnk(segm);
      }
    }
  }
}

/**********************************
 * Fileilist related funcs
 **********************************/
FILELIST *Create_Filelist(FILELIST **si,char *fn)
{
  FILELIST *sn,*s=NULL;
  if (si) s=*si;
  sn=calloc(1,sizeof(*sn));

  if (s)
  {
    while (s->next) s=s->next;
    s->next=sn;
    sn->prev=s;
  }
  else if (si)
  {
    *si=sn;
  }
  if (fn) strcpyd(&sn->fn,fn);
  return sn;
}

void Remove_Filelist(FILELIST *s)
{
  FILELIST *snext;

  for (; s; s=snext)
  {
    snext=s->next;
    if (s->fn) free(s->fn);
    free(s);
  }
}

PLACES *Create_Places(PLACES **si)
{
  PLACES *sn,*s=NULL;
  if (si) s=*si;
  sn=calloc(1,sizeof(*sn));

  if (s)
  {
    while (s->next) s=s->next;
    s->next=sn;
    sn->prev=s;
  }
  else if (si)
  {
    *si=sn;
  }
  return sn;
}

void Remove_Places(PLACES *s)
{
  PLACES *snext;

  for (; s; s=snext)
  {
    snext=s->next;
    free(s);
  }
}

/**********************************
 * Group related funcs
 **********************************/
LUT *Create_Lut(LUT **si)
{
  LUT *sn,*s=NULL;
  if (si) s=*si;
  sn=calloc(1,sizeof(*sn));

  if (s)
  {
    while (s->next) s=s->next;
    s->next=sn;
    sn->prev=s;
  }
  else if (si)
  {
    *si=sn;
  }
  return sn;
}

void Remove_Lut(LUT *s)
{
  LUT *snext;

  for (; s; s=snext)
  {
    snext=s->next;
    free(s);
  }
}

