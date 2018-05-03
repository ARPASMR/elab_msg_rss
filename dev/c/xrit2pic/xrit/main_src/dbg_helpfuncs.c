/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Debug functions XRIT2PIC
 ********************************************************************/
#include <stdio.h>
#define DEBUG
#ifdef DEBUG
#include <stdlib.h>

static int nralloc(int a)
{
  static int n;
  n+=a;
  return n;
}

char *dbg_malloc(int n)
{
  char *p=malloc(n);
  printf("mal : %lx  %d   nr: %d\n",(long)p,n,nralloc(1));
  return p;
}

char *dbg_calloc(int n,int m)
{
  char *p=calloc(n,m);
  printf("cal : %lx  %d  nr: %d\n",(long)p,n*m,nralloc(1));
  return p;
}

void dbg_free(char *p)
{
  printf("free: %lx          nr: %d\n",(long)p,nralloc(-1));
  free(p);
}

#include "xrit2pic.h"
void dump_list(LIST_FILES *lf)
{
  printf("------- List files-------\n");
  for (; lf; lf=lf->next)
    if (lf->fn) printf("    lf: %s\n",lf->fn);
}

void dump_segm(SEGMENT *segm)
{
  printf("    segm: %d\n",segm->xh.segment);
}

void dump_segms(SEGMENT *segm)
{
  for (; segm; segm=segm->next)
    printf("    segm: %d\n",segm->xh.segment);
}

void dump_chan(CHANNEL *chan,int lower)
{
  SEGMENT *segm;
  printf("  chan: name=%s  %s  %s\n",
    (chan->chan_name? chan->chan_name : "name=NULL"),
    (chan->pro? "has PRO" : ""),
    (chan->epi? "has EPI" : ""));

  if (lower)
  {
    printf("    segm: ");
    for (segm=chan->segm; segm; segm=segm->next)
    {
      printf("%d ",segm->xh.segment);
    }
    printf("\n");
  }
}

void dump_chans(CHANNEL *chan)
{
  for (; chan; chan=chan->next)
  {
    dump_chan(chan,0);
  }
}

void dump_group(GROUP *grp,int lower)
{
  CHANNEL *chan;
  printf("grp: time=%s  %s\n",
    (grp->grp_time? grp->grp_time : "name=NULL!"),
    (grp->pro_epi? "has PRO/EPI" : ""));
  if (lower)
    for (chan=grp->chan; chan; chan=chan->next)
      dump_chan(chan,1);
}

void dump_groups(GROUP *grp,int lower)
{
  for (; grp; grp=grp->next)
  {
    dump_group(grp,lower);
  }
}

void dump_dbase(GROUP *grp)
{
  for (; grp; grp=grp->next)
  {
    dump_group(grp,1);
  }
}

void print_hdrinfo(char *fni,XRIT_HDR *h,FILE *fpo,gboolean one_line)
{
  if (one_line)
  {
    fprintf(fpo,"%s %s  (%d x %d) %s %d  %s",fni,
      IFRMT2TEXT(h->image_iformat),h->nc,h->nl,h->chan,h->segment,asctime(&h->time));
    return;
  }

  fprintf(fpo,"file_type=%d   hdr_len=%ld  data_len=%ld\n",
                            h->file_type,
                            h->hdr_len,
                            h->data_len);
  if (h->file_type==0)
  {
    fprintf(fpo,"\n");
    fprintf(fpo,"This a image in format %s.\n",IFRMT2TEXT(h->image_iformat));
    fprintf(fpo,"Size %d x %d x %d   compr.: %x\n",h->nc,h->nl,h->nb,h->cf);
  }

  fprintf(fpo,"\n");
  fprintf(fpo,"anno=%s\n",h->anno);
  fprintf(fpo,"Extracted from anno:\n");
  fprintf(fpo,"  Type   : %cRIT\n",h->hl);
  fprintf(fpo,"  Version: %s\n",h->vers);
  fprintf(fpo,"  Sat    : %s\n",h->sat);
  fprintf(fpo,"  Source : %s\n",h->src);
  fprintf(fpo,"  Channel: %s\n",h->chan);
  fprintf(fpo,"  Segment: %d\n",h->segment);
  fprintf(fpo,"  Time   : %s",asctime(&h->time));
  fprintf(fpo,"  Flags  : %c %c\n",h->compr,h->encry);
}

void testsegm(GROUP *grp)
{
  CHANNEL *chan;
  SEGMENT *segm;
puts("Start test");
  for (; grp; grp=grp->next)
  {
    for (chan=grp->chan; chan; chan=chan->next)
    {
      for (segm=chan->segm; segm; segm=segm->next)
      {
        if (chan!=segm->chan)
          printf("!! %x  %x  >%s<  %d\n",segm->chan,chan,chan->chan_name,segm->xh.segment);
      }
    }
  }
puts("Einde test");
}

void rep_selected(GROUP *grp)
{
  CHANNEL *chan;
  SEGMENT *segm;
  for (; grp; grp=grp->next)
  {
    for (chan=grp->pro_epi; chan; chan=chan->next)
    {
      if ((chan->pro) && (chan->pro->selected)) dump_segm(chan->pro);
      if ((chan->epi) && (chan->epi->selected)) dump_segm(chan->epi);
      if (chan->selected) dump_chan(chan,FALSE);
    }
    
    for (chan=grp->chan; chan; chan=chan->next)
    {
      if ((chan->pro) && (chan->pro->selected)) dump_segm(chan->pro);
      if ((chan->epi) && (chan->epi->selected)) dump_segm(chan->epi);
      if (chan->selected) dump_chan(chan,FALSE);
      else for (segm=chan->segm; segm; segm=segm->next)
      {
        if (segm->selected) dump_segm(chan);
      }
    }
  }
}
#else
void dump_dbase() { puts("UITGESCHAKELD!"); }
#endif
