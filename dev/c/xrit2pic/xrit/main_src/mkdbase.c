/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/

#include "xrit2pic.h"
#include "avhrr.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef __NOGUI__
  #include "xrit2pic_nogtk.h"
#endif
extern PREFER prefer;
extern GLOBINFO globinfo;
extern XRIT_DBASE dbase;

/**********************************
 * Copy channel info from xrit-dbase to channel
 **********************************/
static void add_chaninfo(CHANNEL *chan,XRIT_HDR *xh)
{
  if (!chan->chan_name) strcpyd(&chan->chan_name,xh->chan);

  if (!chan->data_src) strcpyd(&chan->data_src,xh->src);
  chan->wavel=xh->wavel;
  chan->chan_nr=xh->chan_nr;
  chan->satpos=xh->satpos;
  chan->seq_start=xh->seq_start;
  chan->seq_end=xh->seq_end;
  chan->nc=xh->nc;
  chan->nl=xh->nl;
  chan->nb=xh->nb;
  chan->ifrmt=xh->image_iformat;
  chan->scan_dir=xh->scan_dir;
  chan->lines_per_sec=xh->lines_per_sec;

// NOG TE BEKIJKEN: ncext ergens anders bepalen (vlak voor generatie)
  if ((chan->chan_name) && (!strcmp(chan->chan_name,"HRV")))
  {
//    chan->x_shift=HRV_XSHIFT;
//    chan->shift_ypos=HRV_SHIFT_YPOS;
    chan->ncext=11136; // chan->nc*2;
  }
  else
  {
//    chan->x_shift=0;
//    chan->shift_ypos=0;
    chan->ncext=chan->nc;
  }
}

void copy_segminfo(SEGMENT *segm1)
{
  SEGMENT *segm;
  for (segm=segm1->next; segm; segm=segm->next)
  {
    segm->xh.seq_start=segm1->xh.seq_start;
    segm->xh.seq_end=segm1->xh.seq_end;
  }
}

/*
3.2.6, page 111:

Note 2: The relation between the binary pixel value (the pixel count) and the 
physical radiance units (expressed in:
    -2  -1    -1 -1
 mWm  sr   (cm  )
 
 
 ) is fully defined for each spectral band by the relation:

                 Physical Units = Cal_Offset + (Cal_Slope . Pixel Count)

More details can be found in the Level1_5ImageCalibration Record.

*/

/*
Related doc: L_15_Data_Desc.pdf: MSG Level 1.5 Image Data Format Description
Types: See pdf_ten_99055_spe_msg_gsds.pdf:
 INTEGER: 4 bytes
 REAL: 4 bytes
 REAL DOUBLE: 8 bytes
*/
void get_pro(FILE *fp,SEGMENT *pro)
{
  long lng;
  char byte;
  long pro_start;
  int i;

  union
  {
    double dbl;
    gint64 int64;
  } dbl_ch;
  union
  {
    float f;
    long l;
  } flt_ch;
  pro_start=ftell(fp);
  if (pro->chan->group->h_or_l=='H')
  {

/* ProjectionDescription starts at 5e74c (see page 177), length: 5 bytes */
    fseek(fp,pro_start,SEEK_SET);         // offset=0x5a=90
    fseek(fp,0x5e74c,SEEK_CUR);           // Start projection descr.
    fread(&byte,1,1,fp);                  // TypeOfProjection 1=earth centerd in grid
    fread(&flt_ch.l,1,4,fp);              // LongitudeOfSSP
    flt_ch.l=GINT32_FROM_BE(flt_ch.l);    // nominal sub-satellite point
    pro->ProjectionDescription.LongitudeOfSSP=flt_ch.f; // subsat position (after correction)

/* ReferenceGridVIS_IR starts at 5e751 (see page 177), length: 17 bytes */
    fseek(fp,pro_start,SEEK_SET);         // offset=0x5a=90
    fseek(fp,0x5e751,SEEK_CUR);           // Start ReferenceGridVIS_IR
    fread(&lng,1,4,fp);                   // NumberOfLines
//    pro->NumberOfLines=GINT32_FROM_BE(lng);  
    fread(&lng,1,4,fp);                   // NumberOfColumns
    fread(&flt_ch.l,1,4,fp);              // LineDirGridStep
    fread(&flt_ch.l,1,4,fp);              // ColumnDirGridStep
    fread(&byte,1,1,fp);                  // GridOrigin: 0=NW, 1=SW, 2=SE, 3=NE

/* ReferenceGridHRV  starts at 5e762 (see page 177), length: 17 bytes */
    fseek(fp,pro_start,SEEK_SET);         // offset=0x5a=90
    fseek(fp,0x5e762,SEEK_CUR);           // Start ReferenceGridHRV

/* PlannedCoverageVIS_IR starts at 5e773 (see page 177), length: 16 bytes */
    fseek(fp,pro_start,SEEK_SET);         // offset=0x5a=90
    fseek(fp,0x5e773,SEEK_CUR);           // Start PlannedCoverageVIS_IR

/* PlannedCoverageHRV starts at 5e783 (see page 178), length: 64 bytes */
    fseek(fp,pro_start,SEEK_SET);         // offset=0x5a=90

    fseek(fp,0x5e783,SEEK_CUR);           // Start PlannedCoverageHRV
    if (feof(fp)) return;

    fread(&lng,1,4,fp);                   // LowerSouthLinePlanned
    fread(&lng,1,4,fp);                   // LowerNorthLinePlanned
    pro->lower_nord_row=GINT32_FROM_BE(lng); // huh??? -2;
    fread(&lng,1,4,fp);                   // LowerEastColumnPlanned
    pro->lower_east_col=GINT32_FROM_BE(lng);  
    fread(&lng,1,4,fp);                   // LowerWestColumnPlanned
    fread(&lng,1,4,fp);                   // UpperSouthLinePlanned
    fread(&lng,1,4,fp);                   // UpperNorthLinePlanned
    fread(&lng,1,4,fp);                   // UpperEastColumnPlanned
    pro->upper_east_col=GINT32_FROM_BE(lng);
    fread(&lng,1,4,fp);                   // UpperWestColumnPlanned

/* Level1_5ImageCalibration starts at 5e853= 0x5e7f9+0x5a*/
    fseek(fp,pro_start,SEEK_SET);
    fseek(fp,0x5e7f9,SEEK_CUR);           // start Level1_5ImageCalibration
    for (i=0; i<12; i++)
    {
      fread(&dbl_ch.int64,8,1,fp);                   // Cal_Slope
      dbl_ch.int64=GUINT64_FROM_BE(dbl_ch.int64);
      pro->chan->Cal_Slope[i]=dbl_ch.dbl;
      fread(&dbl_ch.int64,8,1,fp);                   // Cal_Offset
      dbl_ch.int64=GUINT64_FROM_BE(dbl_ch.int64);
      pro->chan->Cal_Offset[i]=dbl_ch.dbl;
    }
  }

/* For LRIT: slope *4 to compensate 8 bpp ipv 10bpp */
  if (pro->chan->group->h_or_l=='L')
  {
    fseek(fp,pro_start,SEEK_SET);
    fseek(fp,0x5e7f9,SEEK_CUR);           // start Level1_5ImageCalibration
    for (i=0; i<12; i++)
    {
      fread(&dbl_ch.int64,8,1,fp);                   // Cal_Slope
      dbl_ch.int64=GUINT64_FROM_BE(dbl_ch.int64);
      pro->chan->Cal_Slope[i]=dbl_ch.dbl*4.;

      fread(&dbl_ch.int64,8,1,fp);                   // Cal_Offset
      dbl_ch.int64=GUINT64_FROM_BE(dbl_ch.int64);
      pro->chan->Cal_Offset[i]=dbl_ch.dbl;
    }
  }  
}

void read_pro(SEGMENT *pro)
{
  FILE *fp;
  if (!pro) return;
  if (!pro->chan) return;
  if (!pro->chan->group) return;
//  if (pro->chan->group->h_or_l!='H') return;  // for LRIT: different pro?
  if (!(fp=open_xritimage(pro->pfn,NULL,EXRIT,NULL))) return;
  get_pro(fp,pro);
  fclose(fp);
}

#ifdef XXX
void xxx(GROUP *grp)
{
  CHANNEL *chan;
  for (; grp; grp=grp->next)
    for (chan=grp->chan; chan; chan=chan->next)
       if (chan->Cal_Offset[0]>5)  printf(">>xxx>>! %f\n",chan->Cal_Offset[0]);
}
void yyy(GROUP *grp)
{
  CHANNEL *chan;
    for (chan=grp->chan; chan; chan=chan->next)
       if (chan->Cal_Offset[0]>5)  printf(">>yyy>>! %f\n",chan->Cal_Offset[0]);
}
#endif

/*
// debug
void zzz(CHANNEL *chan,char *s,char *p)
{
static int n;
if (!chan) return;
  if (chan->Cal_Offset[0]>5)
  {  printf("%s  %s >>zzz>>! %f    %x  %s\n",p,s,chan->Cal_Offset[0],chan,chan->chan_name);
n++;
if (n>2) exit(0);
}
}
*/
void unzip_metop(GROUP *grp,int progress_func())
{
  char *p;
  GROUP *g;
  SEGMENT *s;
  static int prgrsmax;
  int prgrscnt=0;
  if (!progress_func) prgrsmax=0;
  if (progress_func) progress_func('s',"Unzipping...",0,prgrsmax);
  for (g=grp; g; g=g->next)
  {
    if (!g->chan) continue;
    if (!g->chan->segm) continue;prgrsmax++;
    if (g->chan->segm->xh.xrit_frmt!=METOP) continue;
    
    for (s=grp->chan->segm; s; s=s->next)
    {
      if (!strstr(s->pfn,".bz2")) continue;
      if (!progress_func) prgrsmax++; 
      if (progress_func)
      {
        if (!(progress_func('w',NULL,++prgrscnt,prgrsmax)))
        {
          break;
        }
        bunzip_to_tempfile(s->pfn,NULL);
        if ((p=strstr(s->pfn,".bz2"))) *p=0;
        if ((p=strstr(s->fn,".bz2"))) *p=0; 
      }
    }
  }
  if (progress_func) progress_func('e',NULL,0,prgrsmax);
}

/*******************************************************
 * Scan all images and create linked list 
 *******************************************************/ 
GROUP *mk_dbase(LIST_FILES *lf,
                LOCATION_SEGM where,
                int progress_func())
{
  LIST_FILES *lft,*lft2;
  GROUP *grp=NULL,*grp1;
  CHANNEL *chan1;
  gboolean abort=FALSE;
  int pic_nr=0;
  int prgrscnt=0,prgrsmax=0;

  if (!lf) return NULL;
#ifndef __NOGUI__
  while (g_main_iteration(FALSE));
#endif

  for (lft=lf; lft; lft=lft->next) prgrsmax++;           /* Determine # files */
  if (progress_func) progress_func('s',"Sorting...",0,prgrsmax);

/* Create dbase with GROUP, CHANNEL and SEGMENT */


  for (lft=lf->next; lft; lft=lft->next)
  {
    GROUP *cur_grp=NULL;
    CHANNEL *cur_chan=NULL;
    SEGMENT *segm;
    if (!lft->fn) { puts("???"); continue;}
    if (lft->is_dir) continue;
    if (lft->unknown)
    {
      LIST_FILES *unk=NULL;
      unk=Create_List(dbase.unknown,NULL);
      strcpyd(&unk->fn,lft->fn);    
      strcpyd(&unk->pfn,lft->pfn);    
      if (!dbase.unknown) dbase.unknown=unk;
      continue;
    }

    for (lft2=lft->prev; lft2; lft2=lft2->prev) /* search back to find corresponding pieces */
    {
      if ((lft2->unknown) || (lft2->is_dir)) continue;
      /* MSG: pic_id=yyy ddddddddd hh mm; NOAA: yyy ddddddddd iiii */

      if ((lft2->pxh.pic_id==lft->pxh.pic_id) &&
          (!strcmp(lft2->pxh.satsrc,lft->pxh.satsrc)) &&
          (lft2->pxh.hl==lft->pxh.hl))      /* Same time and same xrit-type: */
      {
        cur_grp=lft2->grp;

        /* Stop if also same channel name found.
           This is the case if:
           . there was a previous segment belonging to same channel
           . there was a previous segment of pro/epi belonging to same channel (GOES etc.)
           A group pro/epi, not belonging to a certain channel (MSG) is not recognized here.
        */
        if ((*lft->pxh.chan) && (!strcmp(lft2->pxh.chan,lft->pxh.chan)))
        {
          cur_chan=lft2->chan;
// Waarom plots cl_offset van 'foreign' kapot (=12455731200.000002)?
//printf("%s\n",lft2->pxh.chan);
//zzz(lft2->chan,lft2->fn,"");
          break;
        }
      }

    }
    segm=NULL;
    if (!cur_grp)                                /* first occurrence this group */
    {
      cur_grp=Create_Grp(&grp);
      cur_grp->h_or_l=lft->pxh.hl;
      cur_grp->datatype=lft->pxh.xrit_frmt;
      switch(cur_grp->h_or_l)
      {
        case 'A': case 'M': case 'G': cur_grp->orbittype=POLAR;     break;
        case 'h': case 'H':           cur_grp->orbittype=GEO;       break;
        case 'l': case 'L':           cur_grp->orbittype=GEO;       break;
        case 'J':                     cur_grp->orbittype=GEO;       break;
        default:                      cur_grp->orbittype=UNK_ORBIT; break;
      }
      strcpy(cur_grp->sat_src,lft->pxh.satsrc);
      strcpy(cur_grp->grp_time,lft->pxh.itime);
      cur_grp->grp_tm=lft->pxh.time;
      cur_chan=NULL;                           /* New group, so new channels */
    }

    if (cur_grp)                                /* corresponding group found */
    {
      if (cur_chan)                             /* corresponding channel found */
      {
        switch (lft->pxh.special)
        {
          case 'p':                             /* 'P' not expected */
            segm=Create_Segm(&cur_chan->pro);   /* pro of specific chan */
            segm->is_pro=TRUE;
            segm->chan=cur_chan;
          break;
          case 'e':                             /* 'E' not expected */
            segm=Create_Segm(&cur_chan->epi);   /* epi of specific chan */
            segm->is_epi=TRUE;
            segm->chan=cur_chan;
          break;
          default:
            cur_chan->nr_segm++;
            segm=Create_Segm(&cur_chan->segm);  /* segm of specific chan */
            segm->chan=cur_chan;
          break;
        }
      }
      else                                             /* first occurrence this channel */
      {
        switch (lft->pxh.special)
        {
          case 'P': case 'E':                           /* group pro/epi */
            if (!(cur_chan=cur_grp->pro_epi))
            {
              cur_chan=Create_Chan(&cur_grp->pro_epi);  /* create special entry */
              add_chaninfo(cur_chan,&lft->pxh);
              cur_chan->group=cur_grp;
            }
          break;
          default:                                      /* channel pro/epi or pic segment */
            cur_chan=Create_Chan(&cur_grp->chan);       /* create new channel entry */
            cur_chan->group=cur_grp;
          break;
        }
        switch (lft->pxh.special)
        {
          case 'p': case 'P':
            segm=Create_Segm(&cur_chan->pro);           /* group or channel PRO */
            segm->is_pro=TRUE;
            segm->chan=cur_chan;
          break;
          case 'e': case 'E':
            segm=Create_Segm(&cur_chan->epi);           /* group or channel EPI */
            segm->is_epi=TRUE;
            segm->chan=cur_chan;
          break;
          default:
//            add_chaninfo(cur_chan,&lft->pxh); wordt later gedaan, hier nog niet alle info besch.
            cur_chan->chan_nr=lft->pxh.chan_nr;  // maar dit is al nodig voor sorteren!
            cur_chan->nr_segm++;
            segm=Create_Segm(&cur_chan->segm);          /* add first segment to new channel */
            segm->chan=cur_chan;
          break;
        }
      }
    }
    if (segm)
    {
      segm->xh=lft->pxh;            // pxh doesn't contain dynamic allocated parts!
      segm->time=lft->time;         // creation time
      strcpyd(&segm->pfn,lft->pfn);
      strcpyd(&segm->fn,lft->fn);
      segm->where=where;
      if (segm->is_pro) read_pro(segm);
    }
    lft->grp=cur_grp;
    lft->chan=cur_chan;

    if (progress_func)
    {
      if (!(progress_func('w',NULL,++prgrscnt,prgrsmax)))
      {
        abort=TRUE;
        break;
      }
    }
  }

/* Sort dbase */
  if (!abort) grp=Sort_Grp(grp);
  if (progress_func) progress_func('e',NULL,0,prgrsmax);

/* Unzip METOP */
  unzip_metop(grp,NULL);
  unzip_metop(grp,progress_func);

/* Complete xrit header-info by reading header of first file of each channel */
  prgrsmax=0;
  for (grp1=grp; grp1; grp1=grp1->next) prgrsmax++;
  prgrscnt=0;
  if (progress_func) progress_func('s',"Updating...",0,prgrsmax);

  for (grp1=grp; grp1; grp1=grp1->next)
  {
    int nr_chans=0;
    if (progress_func)
    {
      if (!(progress_func('w',NULL,++prgrscnt,prgrsmax)))
      {
        abort=TRUE;
        break;
      }
    }

/* make unique id. for grouping. ! Should be the same as in mkdbase:
      MSG: pic_id=yyy ddddddddd hh mm; NOAA: yyy ddddddddd iiii 
----------------------
      if ((lft2->pxh.pic_id==lft->pxh.pic_id) &&
          (!strcmp(lft2->pxh.satsrc,lft->pxh.satsrc)) &&
          (lft2->pxh.hl==lft->pxh.hl)) 
----------------------
grp1->grp_time differs from lft->pxh.pic_id???
Rest same.
???if (grp1->chan->xh) printf("%lx  %x\n",grp1->chan->xh.pic_id,grp1->grp_time);
*/
    sprintf(grp1->id,"%c%s%s",grp1->h_or_l,grp1->grp_time,grp1->sat_src);
    for (chan1=grp1->chan; chan1; chan1=chan1->next)
    {
      FILE *fpi;
      /* read xrit-header to complete info. Only do this for first segm. */
      if (!chan1->segm) continue;
      if (!(fpi=open_xritimage(chan1->segm->pfn,&chan1->segm->xh,chan1->segm->xh.xrit_frmt,NULL)))
      {
        continue;
      }
      fclose(fpi);
      add_chaninfo(chan1,&chan1->segm->xh);
      if ((grp1->pro_epi) && (grp1->pro_epi->pro))
      {
        chan1->Cal_Slope[0]=grp1->pro_epi->pro->chan->Cal_Slope[chan1->chan_nr-1];
        chan1->Cal_Offset[0]=grp1->pro_epi->pro->chan->Cal_Offset[chan1->chan_nr-1];
        if (chan1->satpos==0)
          chan1->satpos=(int)(grp1->pro_epi->pro->ProjectionDescription.LongitudeOfSSP);
      }

      copy_segminfo(chan1->segm);
      chan1->pic_nr=(++pic_nr);
      if ((grp1->pro_epi) && (!strcmp(chan1->chan_name,"HRV")))
      {
//        chan1->x_shift=grp1->pro_epi->x_shift;
//        chan1->shift_ypos=grp1->pro_epi->shift_ypos;
      }
      if (nr_chans<MAXCHANSTR-1)
      {
        if (chan1->chan_name)
        {
          if (isalpha(chan1->chan_name[0]))
          {
            grp1->chanstr[nr_chans]=chan1->chan_name[0];
          }
          else
          {            
            if (chan1->wavel<10)
              grp1->chanstr[nr_chans]='V';
            else if ((chan1->wavel>60) && (chan1->wavel<75))
              grp1->chanstr[nr_chans]='W';
            else
              grp1->chanstr[nr_chans]='I';
          }
        }
        else
          grp1->chanstr[nr_chans]='?';
        nr_chans++;
        grp1->chanstr[nr_chans]=0;
      }
/* Note: max string length=30 (see MAXLEN_ID) */
      sprintf(chan1->id,"%s    %c%s",chan1->chan_name,grp1->h_or_l,grp1->grp_time);
    }
  }

/* AVHRR (NOAA, METOP):
     determine ID
     add 4 more channels
*/
  for (grp1=grp; grp1; grp1=grp1->next)
  {
    if ((grp1->h_or_l) && (strchr("GAM",grp1->h_or_l)))          /* AVHRR or GAC */
    {
      int i;
      strcpy(grp1->chanstr,"12345");
      grp1->chan->chan_nr=1;
      for (i=2; i<=5; i++)
      {
        CHANNEL *new_chan=NULL,*ch;
        SEGMENT *segm;
        new_chan=Copy_Chan(grp1->chan);
        new_chan->group=grp1;
       /* Add new_chan at end of chan-list */
        for (ch=grp1->chan; ch; ch=ch->next) if (!ch->next) break;
        ch->next=new_chan; new_chan->prev=ch;

        /* Define id and channel name */
        new_chan->id[2]=i+'0';        // correct id needed for translate wnd
        if (new_chan->chan_name)
        {
          switch(i)
          {
            case 1: sprintf(new_chan->chan_name,A_CH1); break;
            case 2: sprintf(new_chan->chan_name,A_CH2); break;
            case 3: sprintf(new_chan->chan_name,A_CH3); break;
            case 4: sprintf(new_chan->chan_name,A_CH4); break;
            case 5: sprintf(new_chan->chan_name,A_CH5); break;
          }
        }
        new_chan->chan_nr=i;
        for (segm=new_chan->segm; segm; segm=segm->next)
        { /* These segms use file connected to segm of first channel! */
          segm->where=no_file;
        }
      }
    }
  }

  if (progress_func) progress_func('e',NULL,0,prgrsmax);

/* make donedir if not exist, and if not in donedir itself */
  if ((where == in_received) && (!globinfo.donedir_exist))
    globinfo.donedir_tomake=TRUE;
  else
    globinfo.donedir_tomake=FALSE;

  return grp;
}

/* Search 'node' in list */
int mark_selected(GROUP *group,GtkCTreeNode *node,gboolean selected)
{
  GROUP *grp;
  CHANNEL *chan;
  SEGMENT *segm;
  int iformat=0;
  if (!node) return 0;

  for (grp=group; grp; grp=grp->next)
  {
    if (grp->node==node) grp->nselected=selected;

    for (chan=grp->chan; chan; chan=chan->next)
    {
      if (chan->node==node) chan->nselected=selected;

      for (segm=chan->segm; segm; segm=segm->next)
      {
        if (segm->node==node) segm->nselected=selected;

/* Check if one of selected pics is JPEG, to enable PGM/JPEG generation choice */
        if ((grp->nselected) || (chan->nselected) || (segm->nselected))
        {
          if (segm->xh.image_iformat)
          {
            if (iformat!='j') iformat=segm->xh.image_iformat; 
          }
        }
      }

      for (segm=chan->pro; segm; segm=segm->next)
      {
        if (segm->node==node) segm->nselected=selected;
      }

      for (segm=chan->epi; segm; segm=segm->next)
      {
        if (segm->node==node) segm->nselected=selected;
      }
    }

    for (chan=grp->pro_epi; chan; chan=chan->next)
    {
      if (chan->node==node) chan->nselected=selected;

      for (segm=chan->pro; segm; segm=segm->next)
      {
        if (segm->node==node) segm->nselected=selected;
      }
      for (segm=chan->epi; segm; segm=segm->next)
      {
        if (segm->node==node) segm->nselected=selected;
      }
    }
  }
  return iformat;
}

void set_selected(GROUP *group,GENMODES *gmode)
{
  GROUP *grp;
  CHANNEL *chan;
  SEGMENT *segm;
  for (grp=group; grp; grp=grp->next)
  {
    grp->selected=grp->nselected;
    for (chan=grp->pro_epi; chan; chan=chan->next)
    {
      chan->selected=chan->nselected;
      for (segm=chan->pro; segm; segm=segm->next)
      {
        segm->selected=segm->nselected;
      }
      for (segm=chan->epi; segm; segm=segm->next)
      {
        segm->selected=segm->nselected;
      }
    }

    for (chan=grp->chan; chan; chan=chan->next)
    {
      chan->selected=chan->nselected;
      for (segm=chan->segm; segm; segm=segm->next)
      {
        segm->selected=segm->nselected;
      }
      for (segm=chan->pro; segm; segm=segm->next)
      {
        segm->selected=segm->nselected;
      }
      for (segm=chan->epi; segm; segm=segm->next)
      {
        segm->selected=segm->nselected;
      }
    }

    for (chan=grp->chan; chan; chan=chan->next)
    {
      if (((grp->selected) || (chan->selected)) && (gmode))
      {
        if (grp->pro_epi)
        {
          if (grp->pro_epi->pro) grp->pro_epi->pro->selected|=gmode->gen_pro;
          if (grp->pro_epi->epi) grp->pro_epi->epi->selected|=gmode->gen_epi;
        }

        if (chan->pro) chan->pro->selected|=gmode->gen_pro;
        if (chan->epi) chan->epi->selected|=gmode->gen_epi;
      }
    }
  }
}

GROUP *get_first_selected_grp(GROUP *group)
{
  GROUP *grp;
  CHANNEL *chan;
  SEGMENT *segm;
  for (grp=group; grp; grp=grp->next)
  {
    if (grp->nselected) return grp;
    for (chan=grp->chan; chan; chan=chan->next)
    {
      if (chan->nselected) return grp;
      for (segm=chan->segm; segm; segm=segm->next)
      {
        if (segm->nselected) return grp;
      }
    }
  }
  return NULL;
}

void clearsel_remnodetree(GROUP *grp)
{
  CHANNEL *chan;
  SEGMENT *segm;
  for (; grp; grp=grp->next)
  {
    grp->nselected=FALSE;
    grp->node=NULL;

    for (chan=grp->chan; chan; chan=chan->next)
    {
      chan->nselected=FALSE;
      chan->node=NULL;
      for (segm=chan->segm; segm; segm=segm->next)
      {
        segm->nselected=FALSE;
        segm->node=NULL;
      }
      for (segm=chan->pro; segm; segm=segm->next)
      {
        segm->nselected=FALSE;
        segm->node=NULL;
      }
      for (segm=chan->epi; segm; segm=segm->next)
      {
        segm->nselected=FALSE;
        segm->node=NULL;
      }
    }
    for (chan=grp->pro_epi; chan; chan=chan->next)
    {
      chan->nselected=FALSE;
      chan->node=NULL;
      for (segm=chan->pro; segm; segm=segm->next)
      {
        segm->nselected=FALSE;
        segm->node=NULL;
      }
      for (segm=chan->epi; segm; segm=segm->next)
      {
        segm->nselected=FALSE;
        segm->node=NULL;
      }
      for (segm=chan->segm; segm; segm=segm->next)
      {
        segm->nselected=FALSE;
        segm->node=NULL;
      }
    }
  }
}

void update_chanstr(GROUP *grp,CHANNEL *chan,GtkCTree *ctree)
{
  int nr_chans=strlen(grp->chanstr);
  if ((nr_chans < MAXCHANSTR-1) && (chan->chan_name))
  {
    grp->chanstr[nr_chans]=chan->chan_name[0];
    grp->chanstr[nr_chans+1]=0;
  }

#ifndef __NOGUI__
  if ((ctree) && (grp->node))
    gtk_ctree_node_set_text(ctree,grp->node,5,grp->chanstr);
#endif
}

/******************************************************************
 * Update channel 1: add segments of chan2 to chan1
 *****************************************************************/
void update_chan(GROUP *grp1,CHANNEL *chan1,CHANNEL *chan2,GtkCTree *ctree)
{
  SEGMENT *segm1,*segm2,*segm2next;
  for (segm2=chan2->segm; segm2; segm2=segm2next)     /* scan through segm. in new chan */
  {
    segm2next=segm2->next;
    /* Check if there are double segments */
    for (segm1=chan1->segm; segm1; segm1=segm1->next) /* scan through segm. in cur. chan */
    {
//      if (!segm1->xh) { unexp("segm1->xh=0: %s",chan1->chan_name); continue;  } /* shouldn't occur... */
//      if (!segm2->xh) { unexp("segm2->xh=0: %s",chan2->chan_name); continue;  } /* shouldn't occur... */
      if (segm1->xh.segment==segm2->xh.segment)  // segment already exist
      {                                          // segment will be removed
        if (!segm2->prev)                        // first segm of chan2?
          chan2->segm=segm2->next;               // Change link chan->segm
        Remove_Segm(segm2);                      // Now remove segm2
        segm2=NULL;
        chan2->nr_segm--;                        // correct nr new segments
        break;
      }
    }

    /* Update string of existing channels if current segment is number 1 */
    if ((segm2) && (segm2->xh.segment==1))
    {
      update_chanstr(grp1,chan2,ctree);
    }
  }


/* remaining segments are new; add to segments in channel in main group */
  if (chan2->segm)                              /* new segments to add */
  {
    Add_Segm(&chan1->segm,chan2->segm);         /* connect remaining segms to end of segms of chan1. */
    for (segm1=chan2->segm; segm1; segm1=segm1->next)
    {
      segm1->chan=chan1;                        /* Change link from chan2 to chan1 */
    }
    chan1->nr_segm+=chan2->nr_segm;
    /* For AVHRR: Update seq_end */

    if (strchr("GAM",grp1->h_or_l)) chan1->seq_end+=chan2->nr_segm; /*METOP*/

/*
Kan niet sorteren, of alle segm. eerst verwijderen!
    chan1->segm=Sort_Segm(chan1->segm);
*/

    /* New segments still exist from chan2; use it to update gui-tree */
    for (segm1=chan2->segm; segm1; segm1=segm1->next)
    {
      Add_Segm_totree(ctree,grp1,chan1,segm1);
    }

    chan2->segm=NULL;           /* Now chan2 can be freed without loss of segm! */
    /* chan2 is removed lateron; has to deal with old group of this channel! */

#ifndef __NOGUI__
    if ((ctree) && (chan1->node))
    {
      GdkColor clr_chok={0,0,0x9999,0};  /* color channel item complete */
      GdkColor clr_chft={0,0xffff,0,0};  /* color channel item not complete */
      GdkColor clr_chan;
      char strrange[50];
      sprintf(strrange,"%d of %d ",chan1->nr_segm,chan1->seq_end-chan1->seq_start+1);
      gtk_ctree_node_set_text(ctree,chan1->node,6,strrange);
      gtk_ctree_node_set_text(ctree,chan1->node,7,strrange);
      if (chan1->nr_segm==chan1->seq_end-chan1->seq_start+1)
        clr_chan=clr_chok;
      else
        clr_chan=clr_chft;
      gtk_ctree_node_set_foreground(ctree,chan1->node,&clr_chan);
    }
#endif
  }
}


/* grp1 and grp2 have same id, so combine into 1 group. */
static gboolean  update_group(GROUP *grp1,GROUP *grp2,GtkCTree *ctree)
{
  CHANNEL *chan1,*chan2,*chan2next;
  gboolean last=FALSE;
/* Add segments part of channel which is already in main group */
  for (chan2=grp2->chan; chan2; chan2=chan2next)       /* scan through new channels (to add) */
  {
    chan2next=chan2->next;
    if (!chan2->chan_name) continue;
    for (chan1=grp1->chan; chan1; chan1=chan1->next)   /* scan through existing channels */
    {
      if (!chan1->chan_name) continue;
      if (!strcmp(chan1->chan_name,chan2->chan_name))  /* channel already exist */
      {
        update_chan(grp1,chan1,chan2,ctree);           /* add new items in chan2 to chan1 */
        if (!chan2->prev)                              /* first in group? */
          grp2->chan=chan2->next;                      /* Change link from grp to chan */
        Remove_Chan(chan2);                            /* Now remove channel */
        break;
      }
    }
  }

/* remaining channels are new; add to channels in main group */
  if (grp2->chan)                          /* new channel to add */
  {
    Add_Chan(&grp1->chan,grp2->chan);         /* connect remaining segms to end of segms of chan1. */

/* update channel overview with new channel */
    for (chan2=grp2->chan; chan2; chan2=chan2->next)
    {
      chan2->group=grp1;
      if (!chan2->segm) continue;
      if (chan2->segm->xh.segment==1)  /* Add channel code (column Format) */
      {
        update_chanstr(grp1,chan2,ctree);
      }
      Add_Chan_totree(ctree,grp1,chan2);  /* update tree: add new channels */
    }

    grp2->chan=NULL;           /* Now grp2 can be freed without loss of chan! */
  }

  if (grp2->pro_epi)
  {
    if (!grp1->pro_epi)
    {
      grp1->pro_epi=grp2->pro_epi;
      grp2->pro_epi=NULL;
      grp1->pro_epi->group=grp1;
      Add_Chan_totree(ctree,grp1,grp1->pro_epi);  /* update tree: add new channels */
      if ((grp1->pro_epi) && (grp1->pro_epi->epi)) last=TRUE;
    }
    else
    {
      if (grp2->pro_epi->pro)
      {
        if (!grp1->pro_epi->pro)                   /* only add if not present yet */
        {
          grp1->pro_epi->pro=grp2->pro_epi->pro;
          grp2->pro_epi->pro=NULL;
          grp1->pro_epi->pro->chan=grp1->pro_epi;
          Add_Segm_totree(ctree,grp1,grp1->pro_epi,grp1->pro_epi->pro);
        }
      }
      if (grp2->pro_epi->epi)
      {
        last=TRUE;
        if (!grp1->pro_epi->epi)                   /* only add if not present yet */
        {
          grp1->pro_epi->epi=grp2->pro_epi->epi;
          grp2->pro_epi->epi=NULL;
          grp1->pro_epi->epi->chan=grp1->pro_epi;
          Add_Segm_totree(ctree,grp1,grp1->pro_epi,grp1->pro_epi->epi);
        }
      }
    }
  }
  return last;
}

int check_msg(GROUP *grp)
{
  CHANNEL *c;
  int err=0;
  if (!grp) return -1;
  for (c=grp->chan; c; c=c->next)
    if (!channel_complete(c)) err++;
  return err;
}

void msgnotify(GROUP *grp,int err)
{
#if __GTK_WIN32__ == 1
  const char nwproc=0;
#else
  const char nwproc='&';
#endif
  char cmd[100],tstr[20];
  strftime(tstr,20,"%Y %m %d %H %M",&grp->grp_tm);
  sprintf(cmd,"%s %s %s %c","msgnotify",(err? "MSGDATALOSS" : "MSGSCANEND"),tstr,nwproc);
  system(cmd);
}

/******************************************************
 * Update GROUP->CHAN->SEGM database with new items in filelist 'lf'
 * and add to ctree
 ******************************************************/
GROUP *update_dbase(GtkCTree *ctree,GROUP *curgrp,LIST_FILES *lf,
                    LOCATION_SEGM where,
                    int progress_func())
{
  GROUP *nwgrp,*grp1,*grp2,*grp2next,*lastgrp=NULL;
  gboolean last=FALSE;

  nwgrp=mk_dbase(lf,where,progress_func);     /* make new group-set of current files */
#ifndef __NOGUI__
  if (ctree) gtk_clist_freeze((GtkCList *)ctree); /* Freeze list for fast creation */
#endif
  
/* check each nwgrp if it contains items not yet in current group */
  for (grp1=curgrp; grp1; grp1=grp1->next)    /* scan through existing group */
  {
    for (grp2=nwgrp; grp2; grp2=grp2next)     /* scan through new group (items to add) */
    {
      grp2next=grp2->next;

      if (!strcmp(grp1->id,grp2->id))          /* channel belongs to already existing group */ 
      {
        if (update_group(grp1,grp2,ctree))         /* add new items in grp2 to grp1 */
        {
        last=TRUE;
          lastgrp=grp1;
        }
/* grp2 done; remove it */
        if (!grp2->prev)
          nwgrp=grp2->next;
        Remove_Grp(grp2);
        break;
      }
    }
  }

/* remaining groups are new; add to main group; */
  if (nwgrp)
  {
    for (grp1=curgrp; grp1 && grp1->next; grp1=grp1->next);
    if (grp1)
    {
      grp1->next=nwgrp;
      nwgrp->prev=grp1;
    }
  }

  if ((globinfo.notify) && (lastgrp))
  {
    msgnotify(lastgrp,check_msg(lastgrp));
  }

#ifndef __NOGUI__
  if (ctree) gtk_clist_thaw((GtkCList *)ctree);
#endif
  return nwgrp;
}

SEGMENT *Find_Segmfromfile(GROUP *grp,char *fn)
{
  CHANNEL *chan;
  SEGMENT *segm;
  if (!fn) return NULL;
  for (; grp; grp=grp->next)
  {
    for (chan=grp->chan; chan; chan=chan->next)
    {
      for (segm=chan->segm; segm; segm=segm->next)
      {
        if (!strcmp(fn,segm->fn)) return segm;
      }
    }
  }
  return NULL;
}

static LIST_FILES *remove_this(LIST_FILES *list_files,LIST_FILES *lf)
{
  if (!(remove(lf->pfn)))
  {
    if (dbase.log) print_log(NULL,"Removed unknown: %s\n",(lf->fn? lf->fn : lf->pfn));
  }
  if (!lf->prev) list_files=lf->next;
  Destroy_Listitem(lf);
  return list_files;
}

LIST_FILES *remove_file(LIST_FILES *list_files,gboolean rm_unk,gboolean rm_nonpic)
{
  LIST_FILES *lf,*lfnext;
  for (lf=list_files; lf; lf=lfnext)
  {
    lfnext=lf->next;
    /* Handle unknown files (.bz2, .gz, etc.) */
    if ((rm_unk) && (lf->unknown))
    {   
      list_files=remove_this(list_files,lf);
    }
    else if ((rm_nonpic) && ((lf->pxh.xrit_frmt==DWDSAT) || (lf->pxh.xrit_frmt==BUFR)))
    {   
      list_files=remove_this(list_files,lf);
    }
  }
  return list_files;
}



/* Remove files if not in keep-list */
LIST_FILES *filter_list(LIST_FILES *list_files,GROUP *grp,GtkWidget *wdgt)
{
  LIST_FILES *lf,*lfnext;
  Set_Entry(wdgt,LAB_RECINFO,"");
  for (lf=list_files; lf; lf=lfnext)
  {
    char tmp[10];
    lfnext=lf->next;
    if (lf->unknown) continue;    /* This is done by 'remove_unk" */

/* Distinquish between MSG1 and 2. 
   If MSG-2 operational: Don't dist. between 1 and 2!
*/
    if (!strncmp(lf->pxh.src,"MSG1",4))
      sprintf(tmp,"%c%s",lf->pxh.hl,"MSG1");
    else if (!strncmp(lf->pxh.src,"MSG2",4))
      sprintf(tmp,"%c%s",lf->pxh.hl,"MSG2");
    else
      strcpy(tmp,lf->pxh.src);

    if (!in_satchanlist(tmp,lf->pxh.chan,lf->pxh.special,globinfo.keep_chanlist))
    {
/* Check if this file already in grp, don't remove if so! */
      if ((lf->fn) && (!lf->is_dir) && (!Find_Segmfromfile(grp,lf->fn)))
      {
        if (!(remove(lf->pfn)))
        {
          if (dbase.log) print_log(NULL,"Removed xrit: %s\n",(lf->fn? lf->fn : lf->pfn));
          if (lf->pxh.hl)
            Set_Entry(wdgt,LAB_RECINFO,"Removed %c %s-%d",
                      lf->pxh.hl,lf->pxh.chan,lf->pxh.segment);
          else
            Set_Entry(wdgt,LAB_RECINFO,"Removed %s",lf->fn);
        }
        if (!lf->prev) list_files=lf->next;
        Destroy_Listitem(lf);
      }
    }
  }

  return list_files;
}

/* remove lf from list */
void extract_lf(LIST_FILES *lf)
{
  if (lf->next) lf->next->prev=lf->prev;
  if (lf->prev) lf->prev->next=lf->next;
  lf->prev=NULL;
  lf->next=NULL;
}

LIST_FILES *add_lf_order(LIST_FILES *lf,LIST_FILES *lf2)
{
  LIST_FILES *lf1;
  if (!lf) return lf2;
  for (lf1=lf; lf1; lf1=lf1->next)
  {
    if (!strcmp(lf1->pxh.satsrc,lf2->pxh.satsrc))
    {
      struct tm tm;
      long sec;
      tm=lf1->pxh.time;
      sec=mktime(&tm);
      tm=lf2->pxh.time;
      sec=mktime(&tm)-sec;  // time_lf2 - time_lf1
      if (sec<0) break;     // time lf2 > time lf1
    }
  }

/* Add lf2 before lf1 */
  if (lf1)
  {
    if (lf1->prev) lf1->prev->next=lf2; else lf=lf2;
    lf2->prev=lf1->prev;
    lf2->next=lf1;
    lf1->prev=lf2;
  }
  else
  {
    for (lf1=lf; lf1->next; lf1=lf1->next);
    lf1->next=lf2;
    lf2->prev=lf1;
  }
  return lf;
}

static LIST_FILES *sort_avhrr_lf(LIST_FILES *lf,char type)
{
  LIST_FILES *lf1,*lf2,*lf2next,*lftmp;
  char *satname=NULL;
  lftmp=NULL;
  lf1=lf;
  while (lf1)
  {
    if (lf1->pxh.xrit_frmt!=type) { lf1=lf1->next; continue; } // no AVHRR
    satname=lf1->pxh.satsrc;
    for (lf2=lf1; lf2; lf2=lf2next)
    {
      lf2next=lf2->next;
      if (!strcmp(satname,lf2->pxh.satsrc))
      {
        if (!lf2->prev) lf=lf2->next;
        extract_lf(lf2);
        lftmp=add_lf_order(lftmp,lf2);
        lf1=lf;
      }
    }
  }
  if (!lf)
  {
    lf=lftmp;
  }
  else if (lftmp)
  {
    for (lf1=lf; ((lf1) && (lf1->next)); lf1=lf1->next);
    lf1->next=lftmp;
    lftmp->prev=lf1;
  }
  return lf;
}

#define DMIN_NEWORBIT 4   // more: treat as new orbit
/* !! Probleem als gedeelte in received en gedeelte in done! */

// voor avhrr-metop: min_segm =1 -> uit data of fnaam bepalen!
static void update_avhrr(LIST_FILES *lfi,char type)  /*NOAA*/
{
  LIST_FILES *lf,*lfa=NULL,*lfp=NULL;
  int min_segm=(type==METOP? 3 : 1);    // bepaal 

  int neighbours;
  for (lf=lfi; lf; lf=lf->next)
  {
    if (lf->pxh.xrit_frmt!=type) continue;  // no AVHRR
    if (!lfp)
    {
      lfa=lf;
      lf->pxh.seq_start=1;
      lf->pxh.seq_end=1;
      lf->pxh.segment=1;
      lfp=lf;
      continue;
    }

    neighbours=0;
    if (!strcmp(lfp->pxh.satsrc,lf->pxh.satsrc))
    {
      int dt=mktime_ntz(&lf->pxh.time)-mktime_ntz(&lfp->pxh.time);
      int dsegm=dt/(min_segm*60);
      if ((dsegm>0) && (dsegm<=DMIN_NEWORBIT)) neighbours=dsegm;
    }
    if (neighbours)
    {
      lf->pxh.segment=lfp->pxh.segment+neighbours;
      lfa->pxh.seq_start=1;
      lfa->pxh.seq_end=lf->pxh.segment;
      lfa->pxh.seq_no=lf->pxh.segment;
    }
    else                   // start new path
    {
      lfa=lf;              // 1e segment of group
      lf->pxh.segment=1;
      lf->pxh.seq_start=1; // toegevoegd 20-11-2011; nw metop-segment!
      lf->pxh.seq_end=1;
    }
    lfp=lf;
  }

  lfa=NULL;
  for (lf=lfi; lf; lf=lf->next)
  {
    if (lf->pxh.xrit_frmt!=type) continue;  // no AVHRR

    if (lf->pxh.segment==1)
    {
      lfa=lf;
    }
    else if (lfa)
    {

// Volgende geeft alle segm zelfde start-tijd. Waarom?
// Weggehaald ivm NOAA positieberekeningen! (2-12-06)
//      lf->pxh.time=lfa->pxh.time;
      lf->pxh.pic_id=lfa->pxh.pic_id; // make id's of segments 1 group the same
    }
  }  
}


/*******************************************************
 * Load xrit-files at location '*path' into tree.
 * clear=true  -> Clear prev. tree and start from scratch.
 * clear=false -> Read new items and add to existing tree.
 * where=in_done -> Assumed to read from done dir.
 * If maxfiles>0 then abort if more than that amount of files exist.
 *******************************************************/
GROUP *Load_List(GROUP *grp,GtkCTree *ctree,GtkWidget *widget,
                 char *path,gboolean clear,
                 LOCATION_SEGM where,
                 gboolean cont_update,int *maxfiles)
{
  GtkWidget *wnd=NULL;
  LIST_FILES *list_files=NULL;
  GROUP *nwgrp;
  gboolean show_progress=((!cont_update) && (widget));
#ifndef __NOGUI__
  wnd=Find_Parent_Window(widget);
#endif
  if (clear)
  {
#ifndef __NOGUI__
    if (ctree) gtk_clist_clear(GTK_CLIST(ctree)); /* Clear list to start from scratch */
#endif
    Remove_Grps(grp);
    grp=NULL;
    Destroy_List(dbase.unknown,TRUE);              /* Remove unk list */
    dbase.unknown=NULL;
  }
  if (!cont_update)                    /* no GDK_WATCH if in update mode */
  {
/* problem with XP:
    'DestroyCursor failed: de bewerking is voltooid' 
    (new load via xrit dir, not via File->Reload!?)
*/
//    Create_Cursor(wnd,GDK_WATCH);      /* !! GDK_WATCH has a small mem lek!! */
  }
  list_files=create_filelist(path);    /* Generate a list of all files/dirs */
  if ((maxfiles) && (*maxfiles>0))
  {
    if (list_files->nr_files>(*maxfiles))
    {
      *maxfiles=list_files->nr_files;
      Destroy_List(list_files,TRUE);                /* Remove list, but NOT pointers inside! */
      return grp;
    }
    *maxfiles=list_files->nr_files;
  }

  if (show_progress)
  {
    Set_Entry(widget,LAB_INFO,"Make filelist...");
#ifndef __NOGUI__
    while (g_main_iteration(FALSE));
#endif
//    progress_func('S',"Make filelist...",0,100);
  }
  
  update_filelist(list_files,cont_update,NULL);   
  if (show_progress)
  {
    Set_Entry(widget,LAB_INFO,"Make filelist......");
#ifndef __NOGUI__
    while (g_main_iteration(FALSE));
#endif
//    progress_func('w',NULL,33,100);
  }
    
  if (!list_files)
  {
    if (wnd)
    {
      if (where == in_received)
      {
        Create_Message("Error","Can't open directory\n  %s.",path);
      }
//      Create_Cursor(wnd,GDK_TOP_LEFT_ARROW);
    }
    if (show_progress)
    {
      Set_Entry(widget,LAB_INFO,"");
#ifndef __NOGUI__
      while (g_main_iteration(FALSE));
#endif
//      progress_func('e',NULL,100,100);
    }
    return 0;
  }

  if (cont_update)
  {                               /* Remove files if requested */
    if (globinfo.pic_only)
    {
      list_files=remove_file(list_files,TRUE,TRUE);
    }
    if (globinfo.keep_chanlist.use_list)
    {
      list_files=remove_file(list_files,!globinfo.keep_chanlist.unk_sel,FALSE);
      if (!globinfo.gen_new)
        list_files=filter_list(list_files,grp,widget);
    }
    widget=NULL;                  /* Suppress progress windows */   
  }
  if (show_progress)
  {
    Set_Entry(widget,LAB_INFO,"Make filelist.........");
#ifndef __NOGUI__
    while (g_main_iteration(FALSE));
#endif
//    progress_func('w',NULL,66,100);
  }
  list_files=sort_avhrr_lf(list_files,NHRPT);
  update_avhrr(list_files,NHRPT);

  list_files=sort_avhrr_lf(list_files,MHRPT);
  update_avhrr(list_files,MHRPT);

  list_files=sort_avhrr_lf(list_files,METOP);
  update_avhrr(list_files,METOP);

  if (show_progress)
  {
    Set_Entry(widget,LAB_INFO,"");
#ifndef __NOGUI__
    while (g_main_iteration(FALSE));
#endif
//    progress_func('e',NULL,100,100);
  }
  
//  Create_Cursor(wnd,GDK_TOP_LEFT_ARROW);
  if (grp)
  {
    if (widget)
      nwgrp=update_dbase(ctree,grp,list_files,where,(void *)progress_func); /* Update dbase */
    else
      nwgrp=update_dbase(ctree,grp,list_files,where,NULL); /* Update dbase */

#ifndef __NOGUI__
      if (cont_update)    show_tree(ctree,nwgrp,FALSE,!cont_update);                /* Add new groups to tree */
#endif
  }
  else                                           /* Start building dbase and tree from scratch */
  {
    if (widget)
      nwgrp=mk_dbase(list_files,where,(void *)progress_func);
    else
      nwgrp=mk_dbase(list_files,where,NULL);
#ifndef __NOGUI__
      if (cont_update) show_tree(ctree,nwgrp,TRUE,!cont_update);
#endif
    grp=nwgrp;
  }

  Destroy_List(list_files,TRUE);                /* Remove list, but NOT pointers inside! */

  if (!cont_update)                             /* not in update mode: */
    grp=Sort_Grp(grp);                          /* sort received/done */

  return grp;
} 

GROUP *Load_List_all(GROUP *grp,GtkCTree *ctree,GtkWidget *widget,int *maxfiles)
{
  GROUP *grp1;
  int maxfiles1=0;
  int maxfiles2=0;
  if (maxfiles)
  {
    maxfiles1=*maxfiles;
    maxfiles2=*maxfiles;
  }
  grp1=Load_List(grp ,ctree,widget,globinfo.src_donedir,TRUE ,in_done    ,FALSE,&maxfiles1);
  if (maxfiles) maxfiles2=MAX(*maxfiles-maxfiles1,1);
  grp =Load_List(grp1,ctree,widget,globinfo.src_dir    ,FALSE,in_received,FALSE,&maxfiles2);
  if (!grp) grp=grp1;
  if (maxfiles)
  {
    if (maxfiles1+maxfiles2>(*maxfiles))
    {
      *maxfiles=maxfiles1+maxfiles2;
      if (grp)
      {
        Remove_Grp(grp); grp=NULL;
      }
    }
  }
#ifndef __NOGUI__
  show_tree(ctree,grp,TRUE,TRUE);  /* redraw tree */
#endif
  return grp;
}
