/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Common functions for Preview and File generation
 ********************************************************************/
#include "xrit2pic.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

extern PREFER prefer;
extern GLOBINFO globinfo;

/***********************************************
 ***********************************************
 ** Common help-functions for presenting xrit info
 ***********************************************
 ***********************************************/



/*
int height_chan(CHANNEL *chan)
{
  SEGMENT *segm;
  int first_chunck=0,last_chunck=0;
  for (segm=chan->segm; segm; segm=segm->next)
  {
    if (!first_chunck)
      first_chunck=segm->xh.segment;
    else
      first_chunck=MIN(first_chunck,segm->xh.segment);

    if (!last_chunck) 
      last_chunck=segm->xh.segment;
    else
      last_chunck=MAX(pc.last_chunck,segm->xh.segment);
  }

  return chan->nl*(last_chunck-first_chunck+1);
}
*/

void polar_map_add_lonlat(segm,pc)
{
}

/***********************************************
 * Translate picture (wvt or jpeg) into 
 *   segm->chnk.
 *   str!=NULL -> copy in str; free segm->chnk
 * polar: segm of all 5 channels loaded.
 ***********************************************/
int pic2str(SEGMENT *segm,guint16 **str,PIC_CHARS *pc,gboolean no_errmes)
{
  guint16 W,H,D;
  int err=0;
  static int nr_err;
  gboolean all_chan=TRUE;
  if (str) all_chan=FALSE;   // if pic to str instead of segm-chunk: load just requested cna.
  if (!segm) return Mis_segm;
  if (segm->corrupt) return Mis_segm;    /* corrupt cleared with new generation */
  if (!segm->chnk) // asumes all channels of this segm not loaded yet!
  {
    switch (segm->chan->ifrmt)
    {
      case 'j': err=jpg2str(segm,&segm->chnk,&W,&H,&D);   break;
      case 'p': err=pgm2str(segm,&segm->chnk,&W,&H,&D);   break;
      case 'w': err=wvt2str(segm,&segm->chnk,&W,&H,&D);   break;
      case 'z': err=noaa2str(segm,all_chan,&W,&H,pc);     break;
      case 'Z': err=eps2str(segm,all_chan,&W,&H,pc);      break;
      case 'y': err=eps2str(segm,all_chan,&W,&H,pc);      break;
      default : err=dump_data(segm,&segm->chnk,&W,&H,&D); break; /* unknown format */
    }
    if (err)
    {
#ifndef __NOGUI__
      if ((!nr_err) && (!no_errmes))
      {
        Create_Message("Error",
          "Translating picture failed (pic2str)!\nPicture format: %c\nError: %s\n(Note: message generated one time.)",
              segm->chan->ifrmt,report_genpicerr(err,NULL));
      }
#endif
      nr_err++;
      return err;
    }

    segm->width=W;  // already done for polar sats in noaa2str and metop2str...
    segm->height=H; // already done for polar sats in noaa2str and metop2str...

    if ((segm->width==0) || (segm->height==0)) err=1;
    segm->loaded_ovl=0;

    if (!segm->ovlchnk)
    {
      segm->ovlchnk=calloc(segm->width*segm->height,sizeof(*segm->ovlchnk));
    }
    if ((pc) && (pc->orbittype==POLAR))
    {
      if (pc->gmap==normal)
      {
        polar_add_lonlat(segm,pc);
      }
//      else
//        polar_add_lonlat(segm,pc);
    }
    else
    {
      geo_add_lonlat(&prefer,segm,pc);
    }
  }

/* Add overlay.*/
// Always do this (except for polar sats), so overlay may be enabled lateron in preview menu.
  if ((!pc) || (pc->orbittype!=POLAR))
  {
    if ((segm) && (segm->loaded_ovl!=globinfo.overlaytype))
    {
      OVERLAYTYPE actual_overlay;
      actual_overlay=add_overlay(&prefer,globinfo.overlaytype,segm,pc);
      if (actual_overlay>0) globinfo.overlaytype=actual_overlay;
    }

  }
  if (!err) /* err=0 -> no error */
  {
    if (str)
    {
      int size;
      size=segm->width*segm->height;
      *str=malloc(size*sizeof(guint16));
      memcpy(*str,segm->chnk,size*sizeof(guint16));
      free(segm->chnk); segm->chnk=NULL;
    }
  }
  return err;
}


/***********************************************
 * Determine size of (composed) pic
 *   nc=with 1 channel.
 *   ncext=extended width (different from nc for HRV)
 *   return: TRUE if HRV channel used
 ***********************************************/
gboolean calc_size(GROUP *grp,int *xmax,int *ymax,int *ncmaxext)
{
  CHANNEL *chan;
  gboolean has_hrv=FALSE;
  if (xmax) *xmax=0;
  if (ymax) *ymax=0;
  if (ncmaxext) *ncmaxext=0;
  for (chan=grp->chan; chan; chan=chan->next)
  {
    int tot_chnks=chan->seq_end-chan->seq_start+1;
    /* Skip if not used */
    if ((!chan->r) && (!chan->g) && (!chan->b) && (!chan->lum)) continue;
    
    if (ncmaxext) *ncmaxext=MAX(*ncmaxext,chan->ncext);  // full earth width (different if HRV)
    if (xmax)
    {
      *xmax=MAX(*xmax,chan->max_x_shift+chan->nc);
      if (prefer.full_globe)
        *xmax=MAX(*xmax,chan->width_totalglobe);
    }

    if (ymax)
    {
      if (chan->height_totalglobe)                  // for MDM HRV
        *ymax=MAX(*ymax,chan->height_totalglobe);
      else
        *ymax=MAX(*ymax,chan->nl*tot_chnks);
    }
    if ((chan->chan_name) && (!strcmp(chan->chan_name,"HRV"))) has_hrv=TRUE;
  }
  return has_hrv;
}

SEGMENT *y2segm(CHANNEL *chan,int y1,int ymax,int *yi,int *y_chnk)
{
  int chnk,tot_chnks;
  int y;
  tot_chnks=chan->seq_end-chan->seq_start+1; 
  if (ymax)
  {
    if (chan->height_totalglobe) 
    {
      y=y1*chan->height_totalglobe/ymax;    // For MDM HRV; ==> y=y1
    }
    else
    {
      if (y1>40000)
        y=(int)((float)y1*((float)chan->nl*(float)tot_chnks)/(float)ymax);         // mapped with hrv: y=y1/3 for 1...11
      else
        y=y1*(chan->nl*tot_chnks)/ymax;         // mapped with hrv: y=y1/3 for 1...11
    }
  }
  else
  {
    y=y1;
  }
  chnk=y/chan->nl;                        // chunk in which req. y is
  if (y_chnk)
  {
    (*y_chnk)=y-(chnk*chan->nl);               // y within this chunk
    if ((*y_chnk)<0) return NULL;
  }
  if (yi) *yi=y;
  return Get_Segm(chan,chnk+chan->seq_start);             // segment holding y
}

// Detect if for polar sats this segment is in selected area
int select_polar(SEGMENT *segm)
{
  gboolean nbound=(segm->orbit.subsat_start.lat > segm->orbit.subsat_end.lat);
  if ((globinfo.pdir_show=='a') && (nbound)) return 0;
  if ((globinfo.pdir_show=='d') && (!nbound)) return 0;

  if (globinfo.ltlat!=globinfo.gtlat)
  {
    if (segm->orbit.subsat_start.lat<globinfo.ltlat)  return 0;
    if (segm->orbit.subsat_start.lat>globinfo.gtlat)  return 0;
  }
  return 1;
}

SEGMENT *get_linefromsegm(CHANNEL *chan,int y1,gboolean has_hrv,
                          int ymax,int *spos,int *dshft,PIC_CHARS *pc)
{
  SEGMENT *segm;
  int y,y_chnk;
  segm=y2segm(chan,y1,ymax,&y,&y_chnk);
  if (!segm) return NULL;
  if ((pc->orbittype!=POLAR) || (select_polar(segm)))
  {
    if (pic2str(segm,NULL,pc,FALSE)) return NULL; // decompress chunck if needed
  }
  if (y_chnk>=segm->height) y_chnk=segm->height-1;
  *spos=y_chnk*chan->nc;                   // start-pos in chunck

  if (y+(chan->seq_start-1)*chan->nl>chan->shift_ypos)
    *dshft=chan->upper_x_shift;
  else
    *dshft=chan->lower_x_shift;
  return segm;
}

static guint16 *get_1str(CHANNEL *chan,guint16 *lum,int y1,
            gboolean has_hrv,int pixvalmax,int xmax,int ymax,int ncmaxext,PIC_CHARS *pc)
{
  SEGMENT *segm;
  int spos,dshft,x;
  if (!(segm=get_linefromsegm(chan,y1,has_hrv,ymax,&spos,&dshft,pc))) return 0;
  if (!lum) lum=calloc(xmax,sizeof(*lum));

  for (x=0; x<ncmaxext; x++)                 // copy line
  {
    guint16 pix;
    int xx;
    xx=x*chan->ncext/ncmaxext;            // offset in chunck for this x
    if (xx>chan->nc) break;
    if (dshft+x>=xmax) continue;
    pix=segm->chnk[spos+xx];       // get pix, remove overlay
    if (chan->invert) if (pix)
    {
      if (pix>pixvalmax) pix=0; 
      else pix=pixvalmax-pix;
    }
    lum[dshft+x]=pix;
  }
  return lum;
}



void Add_Overlaybits(GROUP *grp,guint16 *rgbstr,int y1)
{
  CHANNEL *chan;
  int width,height,ncmaxext;
  gboolean has_hrv;
  if (!rgbstr) return;
  has_hrv=calc_size(grp,&width,&height,&ncmaxext);

  if (has_hrv)                                  // Take HRV channel if used
  {
    for (chan=grp->chan; chan; chan=chan->next)
      if (!strcmp(chan->chan_name,"HRV"))
        break;
  }
  else                                          // else take first channel
  {
    for (chan=grp->chan; chan; chan=chan->next)
      if ((*chan->chan_name) && ((chan->r)||(chan->g)||(chan->b))) break;
  }

  if (chan)
  {
    int tot_chnks;
    int y_chnk,chnk,y,spos,dshft,x,xx;
    SEGMENT *segm;
    tot_chnks=chan->seq_end-chan->seq_start+1; 
    y=y1;
    chnk=y/chan->nl;                           // chunk in which req. y is
    segm=Get_Segm(chan,chnk+chan->seq_start);  // segment holding y
    y_chnk=y-(chnk*chan->nl);                  // y within this chunk
    if ((segm) && (segm->chnk) && (segm->ovlchnk))
    {
      y_chnk=MIN(y_chnk,segm->height-1);
      y_chnk=MAX(y_chnk,0);                    // should never have effect
      spos=y_chnk*chan->nc;                    // start-pos in chunck

// Add HRV shifts
      if (y+(chan->seq_start-1)*chan->nl>chan->shift_ypos)
        dshft=chan->upper_x_shift;
      else
        dshft=chan->lower_x_shift;

      for (x=0; x<ncmaxext; x++)
      {
        xx=x*chan->ncext/ncmaxext;             // offset in chunck for this x
        if (xx<chan->nc)
        {
          rgbstr[dshft+x] &=~((CMASK|CLMASK));       // clear overlay bit
          rgbstr[dshft+x] |= segm->ovlchnk[spos+xx]; // restore overlay
        }
      }
    }
  }
}
int *temp2lut(float pix);

/*********************************************************
 * Get 1 rgb line 'y1' to draw/write. 
 * Compose using channels in grp.
 * Composed line in rgbstr.
 * Allocated here if rgbstr[]=NULL, otherwise:
 *   expected that it is already allocated with correct size.
 * Original lines per channel are in segm->chnk,
 * allocated here if not done yet.
 * So after this proc allocated;
 *  . rgbstr with 1 line
 *  . segm->chnk(s) containing this line
 * return: segment of first used channel of this line
 *********************************************************/
SEGMENT *get_composed_line(GROUP *grp,int y1,guint16 *rgbstr[4],ANAGL *anagl,
                     gboolean fire_detect,gboolean use_lut)
{
  CHANNEL *chan;
  SEGMENT *segm,*segm_ret=NULL;
  int ncmaxext=0;
  int xmax=0;
  int ymax=0;
//  int y_chnk;
  int spos,dshft=0;
  int x,xx; // ,chnk;
//  int y;
  gboolean has_hrv=FALSE;
  gint16 *srgbstr[3];
  int pixvalmax=(0x100<<grp->pc.bitshift)-1;
  double *frgbstr[3];
  gboolean tempmap=grp->conv_temp;
  has_hrv=calc_size(grp,&xmax,&ymax,&ncmaxext);
  if (y1<0) return NULL;    // mag nooit voorkomen!
  if (!xmax) return NULL;
  if (!grp->chan) return NULL;
/* If no compose: Remove temp flag
   (May be set by fog etc. choice; if 1 (VIS) channel is selected 
       it should be ignored.
*/
  if (tempmap)
  {
    frgbstr[0]=calloc(xmax,sizeof(**frgbstr));
    frgbstr[1]=calloc(xmax,sizeof(**frgbstr));
    frgbstr[2]=calloc(xmax,sizeof(**frgbstr));
  }

  /* If rgbstr[] !=0 then assumed to be allocated ext. with correct size . */
  if (!rgbstr[0]) rgbstr[0]=calloc(xmax,sizeof(*rgbstr));
  if (!rgbstr[1]) rgbstr[1]=calloc(xmax,sizeof(*rgbstr));
  if (!rgbstr[2]) rgbstr[2]=calloc(xmax,sizeof(*rgbstr));
  if (!rgbstr[3]) rgbstr[3]=calloc(xmax,sizeof(*rgbstr));

  srgbstr[0]=(gint16 *)rgbstr[0];
  srgbstr[1]=(gint16 *)rgbstr[1];
  srgbstr[2]=(gint16 *)rgbstr[2];

/***** Compose line of pictures using several channels *****/
/*** Step 1: Compose 1 line in srgbstr (signed!) ***/
  for (chan=grp->chan; chan; chan=chan->next)
  {
    gboolean is_irchan=FALSE;
    if ((!chan->r) && (!chan->g) && (!chan->b)) continue;

    if (!grp->compose)
    {
      chan->g=chan->r;
      chan->b=chan->r;
    }
    if (!(segm=get_linefromsegm(chan,y1,has_hrv,ymax,&spos,&dshft,&grp->pc))) continue;
    if (!segm->chnk) continue;
    if (!segm_ret) segm_ret=segm;              // store segment of first used channel
    is_irchan=is_ir_chan(chan);
    for (x=0; x<ncmaxext; x++)                 // copy line
    {
      guint16 pix;
      xx=x*chan->ncext/ncmaxext;               // offset in chunck for this x
      if (chan->scan_dir=='n')
        if (chan->ncext!=ncmaxext) xx=xx+chan->ncext-(chan->ncext*xmax)/ncmaxext; //    3712-2560;

      if (xx<0) break;
      if (xx>chan->nc) break;
      if (dshft+x>=xmax) continue;
      pix=segm->chnk[spos+xx];               // get pix
      if (!pix)
      {
        rgbstr[3][x]|=SMASK;                 // Mark space
        continue;
      }

      if (tempmap) // use mappings in msgmapping.h or LUT; temp. flag on
      {
        if ((grp->pro_epi) && (is_irchan))  // (chan->chan_nr>=4) && (chan->chan_nr<=11))
        {
          float fpix;
          fpix=pix2temp(grp->pro_epi->pro,chan,pix);
          frgbstr[0][x]+=fpix*chan->r;      // compose pic
          frgbstr[1][x]+=fpix*chan->g;
          frgbstr[2][x]+=fpix*chan->b;

        }
        else if (((chan->ifrmt=='j') || (chan->ifrmt=='p')) && (is_irchan))
        {
          float fpix;
          if (pix==255)
          {
            rgbstr[3][x]|=SMASK; // Mark space
            continue;
          }

          fpix=val2temp(pix);
          frgbstr[0][x]+=fpix*chan->r;      // compose pic
          frgbstr[1][x]+=fpix*chan->g;
          frgbstr[2][x]+=fpix*chan->b;
        }
        else
        {
          frgbstr[0][x]+=pix*chan->r;      // compose pic
          frgbstr[1][x]+=pix*chan->g;      // compose pic
          frgbstr[2][x]+=pix*chan->b;      // compose pic
        }
      }
      else if ((use_lut) && (is_irchan))
      {
        int *pixrgb;
        float t;
        t=pix2temp((grp->pro_epi? grp->pro_epi->pro:NULL),chan,pix);
        pixrgb=temp2lut(t);
        srgbstr[0][dshft+x]=pixrgb[0];// *chan->r;      // compose pic
        srgbstr[1][dshft+x]=pixrgb[1];// *chan->g;
        srgbstr[2][dshft+x]=pixrgb[2];// *chan->b;
      }
      else   // inversion specified per channel. 
      {      // Normally: see in convert_line:  'if ((gmode->invert) ... '
        if (chan->invert) if (pix)         // don't invert "space" pixels
        {
          if (pix>pixvalmax) pix=0; 
          else pix=pixvalmax-pix;
        }
        srgbstr[0][dshft+x]+=pix*chan->r;      // compose pic
        srgbstr[1][dshft+x]+=pix*chan->g;
        srgbstr[2][dshft+x]+=pix*chan->b;
      }
    }
  }

  if (tempmap)
  {
    for (x=0; x<ncmaxext; x++)                 // copy line
    {
      if (dshft+x>=xmax) continue;
      srgbstr[0][dshft+x]=frgbstr[0][dshft+x];      // compose pic
      srgbstr[1][dshft+x]=frgbstr[1][dshft+x];
      srgbstr[2][dshft+x]=frgbstr[2][dshft+x];
    }

    if (frgbstr[0]) free(frgbstr[0]); frgbstr[0]=NULL;
    if (frgbstr[1]) free(frgbstr[1]); frgbstr[1]=NULL;
    if (frgbstr[2]) free(frgbstr[2]); frgbstr[2]=NULL;
  }

/*** Step 2: Add offset and handle negative values (-> unsigned) ***/
  for (x=0; x<ncmaxext; x++)             // Translate from signed to unsigned
  {
    int n;
    if (rgbstr[3][dshft+x] & SMASK) continue;
    for (n=0; n<3; n++)
    {
//if (srgbstr[n][dshft+x])  // waarom? Gaat "mis" bij aftrekken! Dus WEG!
      srgbstr[n][dshft+x]+=(grp->offset[n]); // *1023/100;
      if (globinfo.mirror_negval)
      {                       // make value positive
        if (srgbstr[n][dshft+x]<0) srgbstr[n][dshft+x]*=-1;
      }
      else
      {                       // Add offset, and clip on 0
        if (srgbstr[n][dshft+x]<0) srgbstr[n][dshft+x]=0; 
      }
      if ((grp->gamma[n]) && (grp->gamma[n]!=1.))
        srgbstr[n][dshft+x]=igamma(srgbstr[n][dshft+x],pixvalmax,grp->gamma[n]);
    }
  }
/* Now unsigned values are in rgbstr (points to same mem as srgbstr) */

/*** Step 3: Modulate HRV with colour-info of low-res channels (if needed) ***/
  for (chan=grp->chan; chan; chan=chan->next)
  {
    guint16 rgb[3],rgbmax;
    if (!chan->lum) continue;

    if (!(segm=get_linefromsegm(chan,y1,has_hrv,ymax,&spos,&dshft,&grp->pc))) continue;
    for (x=0; x<ncmaxext; x++)                // copy line
    {
      guint16 pix;
      xx=x*chan->ncext/ncmaxext;              // offset in chunck for this x
      if (xx>chan->nc) break;
      pix=segm->chnk[spos+xx];               // get pix
      rgb[0]=rgbstr[0][dshft+x];
      rgb[1]=rgbstr[1][dshft+x];
      rgb[2]=rgbstr[2][dshft+x];
      rgbmax=(rgb[0]+rgb[1]+rgb[2])/3;
      if (rgbmax)
      {
        rgbstr[0][dshft+x]=pix*rgb[0]/rgbmax;
        rgbstr[1][dshft+x]=pix*rgb[1]/rgbmax;
        rgbstr[2][dshft+x]=pix*rgb[2]/rgbmax;
      }
      else
      {
        rgbstr[0][dshft+x]=rgbstr[1][dshft+x]=rgbstr[2][dshft+x]=0;
      }
    }

    break;
  }

/*** Step 4: Fire detection ***/
  if (fire_detect)
  {
    fire_filter_line(grp,y1,ncmaxext,xmax,ymax,rgbstr);
  }
  

/*** Step 5: Restore overlay bit ***/
/* Find suitable channel containing overlay. */
//  Add_Overlaybits(grp,rgbstr[3],y1);

  if (anagl)
  {
    guint16 *lum=NULL;
    if (anagl->depthchan)
    {
      chan=Get_Chan(grp->chan,prefer.depth_chan);
      if (chan)
      {
        chan->invert=prefer.dchan_iv;
        lum=get_1str(chan,lum,y1, has_hrv, pixvalmax,xmax, ymax,ncmaxext,&grp->pc);
      }
      else
      {
      // no anaglyph depth channel found
      }
    }
    conv_to_3d(rgbstr,lum,grp->pc.width,pixvalmax,grp->pc.scan_dir,*anagl);
    if (lum) free(lum); lum=NULL;
  }
//for (x=0; x<ncmaxext; x++) printf("2rgbstr: %d  %x  %x  %x\n",x,rgbstr[0][x],rgbstr[1][x],rgbstr[2][x]);
  return segm_ret;
}

/*********************************************************
 * Get 1 composed line 'y1' to draw/write. 
 * If grp->keep_in_mem==FALSE: same as get_composed_line
 * If grp->keep_in_mem==TRUE: result of get_composed_line is saved, and with next call reused.
 * NOTE: rgbl MUST be 4x NULL!
 *********************************************************/
SEGMENT *store_composed_line(GROUP *grp,
                             int y,              // y in drawable to get
                             guint16 *rgbl[4],
                             ANAGL *anagl,
                             gboolean fire_detect,gboolean use_lut)
{
  SEGMENT *segm=NULL;
  PIC_CHARS *pc=&grp->pc;
  if (!globinfo.dim3) anagl=NULL;
  if ((y<0) || (y>=pc->height)) return NULL;  // out-of-range
  if (grp->keep_in_mem)
  {
    int width,height;
    calc_size(grp,&width,&height,NULL);  // Calc. width/height composed pic
    if (!grp->rgbpicstr[0]) grp->rgbpicstr[0]=calloc(width*height,sizeof(**grp->rgbpicstr));
    if (!grp->rgbpicstr[1]) grp->rgbpicstr[1]=calloc(width*height,sizeof(**grp->rgbpicstr));
    if (!grp->rgbpicstr[2]) grp->rgbpicstr[2]=calloc(width*height,sizeof(**grp->rgbpicstr));
    if (!grp->line_in_mem)  grp->line_in_mem=calloc(height,sizeof(grp->line_in_mem));

    if (!grp->rgbpicstr[3]) grp->rgbpicstr[3]=calloc(width*height,sizeof(**grp->rgbpicstr));
    if (!grp->ovl_in_mem)   grp->ovl_in_mem=calloc(height,sizeof(grp->ovl_in_mem));

    rgbl[0]=grp->rgbpicstr[0]+y*width;
    rgbl[1]=grp->rgbpicstr[1]+y*width;
    rgbl[2]=grp->rgbpicstr[2]+y*width;
    rgbl[3]=grp->rgbpicstr[3]+y*width;
    if (!grp->line_in_mem[y])                                  // not in mem yet
    {
      segm=get_composed_line(grp,y,rgbl,anagl,fire_detect,use_lut);   // so get it now
    }
    grp->line_in_mem[y]=TRUE;

    if (!grp->ovl_in_mem[y])                                  // not in mem yet
    {
      Add_Overlaybits(grp,rgbl[3],y);
    }
    grp->ovl_in_mem[y]=TRUE;

  }
  else         // Don't store: free mem if it was allocated; use get_composed_line
  {
    if (grp->rgbpicstr[0]) free(grp->rgbpicstr[0]); grp->rgbpicstr[0]=NULL;
    if (grp->rgbpicstr[1]) free(grp->rgbpicstr[1]); grp->rgbpicstr[1]=NULL;
    if (grp->rgbpicstr[2]) free(grp->rgbpicstr[2]); grp->rgbpicstr[2]=NULL;
    if (grp->rgbpicstr[3]) free(grp->rgbpicstr[3]); grp->rgbpicstr[3]=NULL;
    if (grp->line_in_mem)  free(grp->line_in_mem);  grp->line_in_mem=NULL;
    if (grp->ovl_in_mem)   free(grp->ovl_in_mem);   grp->ovl_in_mem=NULL;
    segm=get_composed_line(grp,y,rgbl,anagl,fire_detect,use_lut);
    Add_Overlaybits(grp,rgbl[3],y);
  }

  return segm;
}

/* report picture translation error */
char *report_genpicerr(int err,char *fn)
{
  static char errstr[1000];
  if (!fn) fn="";
  *errstr=0;
  switch(err)
  {
    case Exist_fn: sprintf(errstr,"File exist, not overwritten:\n%s",fn); break;
    case Aborted : strcpy(errstr,"Translation aborted."); break;
    case Mis_segm: strcpy(errstr,"Missing segment(s)."); break;
    case Unk_Frmt: strcpy(errstr,"Unknown picture format."); break;
    case JPG_Rd  : strcpy(errstr,"JPEG read problem."); break;
    case JPG_Wr  : sprintf(errstr,"Can't open file for write:\n%s",fn); break;
    case Tmp_fn  : strcpy(errstr,"Can't open temp file."); break;
    case Open_Wrt: strcpy(errstr,"Can't open temp file write."); break;
    case Open_Rd : strcpy(errstr,"Can't open file for read. Wrong format?"); break;
    case Open_Wr : sprintf(errstr,"Can't open file for write:\n%s",fn); break;
    case WVT_Err : strcpy(errstr,"Segments missing."); break;
    case Bzip2_Err: strcpy(errstr,"Bzip2 failed."); break;
    case Out_fn  : strcpy(errstr,"Outputfile not defined."); break;
    case JPG_Conc: strcpy(errstr,"Can't combine without decompressing first.\nChoose Jpeg or PGM output."); break;
    case 0x7f00  : strcpy(errstr,"Can't open program for translation."); break;
    default      : sprintf(errstr,"Error code %d\n",err);
  }
/* No message if aborted */
/*
  if (err!=Aborted)
    Create_Message("Error",errstr);
*/
  return errstr;
}

char *report_genpicerr_1line(int err)
{
  char *errstr,*p;
  errstr=report_genpicerr(err,NULL);
  if ((p=strchr(errstr,'\n'))) *p=0;
  return errstr;
}

char *report_genpicerr_short(int err,char *fn)
{
  static char errstr[1000];
  if (!fn) fn="";
  *errstr=0;
  if (strrchr(fn,DIR_SEPARATOR)) fn=strrchr(fn,DIR_SEPARATOR)+1;
  switch(err)
  {
    case Exist_fn: sprintf(errstr,"Not overwritten: %s",fn); break;
    case Aborted : strcpy(errstr,"Translation aborted."); break;
    case Mis_segm: strcpy(errstr,"Missing segment(s)."); break;
    default      : sprintf(errstr,"Error code %d\n",err);
  }
  return errstr;
}

static int gen_pic_item(GtkWidget *window,
                 GROUP *grp_sel,          /* group containing xrit with pic to generate */
                 GENMODES *modes,
                 PREFER *prefer,
                 char *tfno,              /* output file generated */
                 gboolean *grp_ready)     /* output if grp_sel is ready */
{
#ifndef __NOGUI__
  GtkWidget *list_wnd=Find_Window(window,LAB_PROGRESS_TRANS);
#endif
  int err=0;
  gboolean show_progress=((window) && (globinfo.nr_pics<=1)? TRUE : FALSE);
  /* Do now if just 1 item to be done or if this item is selected in list-window */
  /* Use channel name to determine selected item in list-window. */

  if ((globinfo.nr_pics == 1) || (chan_selected(window,grp_sel->chan->chan_name)))
  {
#ifndef __NOGUI__
    Set_Led(list_wnd,grp_sel->chan->id,0xff0); /* yellow (if chan selected) */
    Set_Led(list_wnd,grp_sel->id,0xff0);       /* yellow (if group selected) */
#endif

    if (modes->otype=='v')
    {
#ifndef __NOGUI__
      err=preview_pic(window,grp_sel,modes);         /* Do it! */
      if ((grp_sel->orbittype==POLAR) && (modes->area_nr))
      {
        if ((grp_sel->segm_first==0) && (grp_sel->segm_last==0))
        { // niet gevonden, zet op def.
          Set_Adjust(window,LAB_SEGMFRST,"%d",0);  
          Set_Adjust(window,LAB_SEGMNR,"%d",0);
        }
        else
        {
          Set_Adjust(window,LAB_SEGMFRST,"%d",grp_sel->segm_first+1);  
          Set_Adjust(window,LAB_SEGMNR,"%d",grp_sel->segm_last-grp_sel->segm_first+1);
        }
      }
#endif
      if (err) err=Aborted;
      *grp_ready=FALSE;                        /* Remove_Grps done by preview! */
    }
    else /* if (modes->otype=='f') */
    {
      if ((grp_sel->chan->pro) && (grp_sel->chan->pro->selected))
      {
        err= gen_proepi1(grp_sel->chan->pro,&grp_sel->pc,prefer,tfno,modes);
        if (modes->log_func) modes->log_func(modes,grp_sel,tfno,err);
      }
      if ((grp_sel->chan->epi) && (grp_sel->chan->epi->selected))
      {
        err= gen_proepi1(grp_sel->chan->epi,&grp_sel->pc,prefer,tfno,modes);
        if (modes->log_func) modes->log_func(modes,grp_sel,tfno,err);
      }
      pic_info(grp_sel,window,modes);
      modes->overwrite_mode=globinfo.overwrite_mode;
      modes->filmframe_ovrwrmode=globinfo.filmframe_ovrwrmode;
      modes->spm.invert=globinfo.spm.invert;
      modes->spm.inv_ir=globinfo.spm.inv_ir;
      modes->add_overlay=globinfo.add_overlay;
      modes->add_lonlat=globinfo.add_lonlat;
      err=gen_picfile(window,grp_sel,prefer,tfno,FALSE,show_progress,modes);
      if (modes->log_func) modes->log_func(modes,grp_sel,tfno,err);
    }
    {
      int fcode;
      switch(err)
      {
        case Aborted : fcode=0x00f; break;  /* blue */
        case Exist_fn: fcode=0x0bb; break;  /* cyan */
        case Mis_segm: fcode=0x0a0; break;  /* dark green */
        case 0       : fcode=0x0f0; break;  /* green */
        default      : fcode=0xf00; break;  /* red */
      }
#ifndef __NOGUI__
      Set_Led(list_wnd,grp_sel->chan->id,fcode); /* (if chan selected) */
      Set_Led(list_wnd,grp_sel->id,fcode);       /* (if group selected) */
#endif
    }
  }
  return err;
}


GtkWidget *progress_gen;

#define STARTING "Starting..."
/*******************************************************
 * Main generate function: generate message(s) and/or picture(s) 
 *******************************************************/
void gen_item(GtkWidget *window,GROUP *grp,GENMODES *modes,PREFER *prefer)
{
  GtkWidget *list_wnd=Find_Window(window,LAB_PROGRESS_TRANS);
  GtkWidget *progress=NULL;
  GROUP *grp_sel;
  PIC_CHARS *pc;
  FILELIST *fl=NULL;      // filelist for movie generation
  
  gboolean show_progress;
  char *ext=0;
  gboolean remove_temp=FALSE;    // flag 'Remove temp file'
  char tfno[1000],mfno[1000];
  int nract=0;
  int n1=0;
  int err;
  int minwidth=0;             // for film: contains min. width of all used frames
  *tfno=0;
  progress_gen=NULL;
  globinfo.abort=FALSE;
  // option LAB_FLMTMP: remove=0, replace=1, reuse=2
  globinfo.filmframe_ovrwrmode=Get_Optionsmenu(list_wnd,LAB_FLMTMP);
  if (globinfo.lut.use) get_lut();

/* filmframe overwrite mode: Never ask (see gen_file.c) -->
   filmframe_ovrwrmode=1 for remove or replace (means: overwrite without asking)
                      =2 for reuse (means: don't overwrite; don't generate)
*/
  if (!globinfo.filmframe_ovrwrmode)   // means: Remove temp file after used
  {
    remove_temp=TRUE;                  // set flag: Remove
    globinfo.filmframe_ovrwrmode=1;
  }
  
  show_progress=((window) && (globinfo.nr_pics>1)? TRUE : FALSE);
  Set_Entry(window,LAB_INFO,STARTING);
#ifndef __NOGUI__
  while (g_main_iteration(FALSE));
#endif

/* If translate list window is open then set led on red */
  if (list_wnd)
  {
    for (grp_sel=grp; grp_sel; grp_sel=grp_sel->next)
    {
      Set_Led(list_wnd,grp_sel->id,0xf00);              /* red */
      if (grp_sel->chan) 
        Set_Led(list_wnd,grp_sel->chan->id,0xf00);        /* red */
    }
  }
  
  /* Prepair for movie generation:
      . Catch first picture used for movie)
      . generate pic info from this first pic
      . make temp-dir, if already exist:
          clean it (move files to sav dir)
      . make avi filename
  */
  /* After prepair:
      . move pic needed for frame from sav to tmp dir (if exist, and size=OK)
      . reuse: don't re-generate
      . replace: generate and overwrite
      . remove: same as replace, but remove all files in tmp-dir after moviegeneration
        (not-used files un sav dir remain)
  */
  if ((modes->gen_film) && (modes->otype!='v'))
  {
    GROUP *grp_first=NULL;
    ext=0;
/* Search first pic of film to determine file name, and min. size of all frames.
   (Needed if HRV used: variable width, always starting at far east.)
*/
    get_selected_item(grp,modes);                    /* Init selection */
    while ((grp_sel=get_selected_item(NULL,modes)))  /* Get next selected image */
    {
      if ((grp_sel->chan) && (chan_selected(window,grp_sel->chan->chan_name)))
      {
        // Determine size of 'full-size pic', not taken into account "Europe" boundaries!
        pic_info(grp_sel,window,modes);  // window!=NULL-> use mapping to define filename
        pc=&grp_sel->pc;
        if (!minwidth) minwidth=pc->o_width; else minwidth=MIN(minwidth,pc->o_width);
        if (!grp_first)
          grp_first=grp_sel;
        else
          Remove_Grps(grp_sel);         /* Item handled; remove. */
      }
      else
        Remove_Grps(grp_sel);         /* Item handled; remove. */
    }
    modes->frame_minwidth=minwidth;
    grp_sel=grp_first;
    if (grp_sel)                       /* first pic of film to make found */
    {
      pc=&grp_sel->pc;
      if (!modes->cformat)             /* no format explicitely defined -> Define it! */
      {
        if (grp_sel->chan->ifrmt=='j') modes->cformat='j'; /* default always jpeg -> */
        else                           modes->cformat='j'; /* -> gives avi */
      }
      pic_info(grp_sel,window,modes);  // window!=NULL-> use mapping to define filename

/* If a temp-dir already exist: 
     Either make it empty and start over generate all needed files for movie
     Or keep it, and use previously generated files
*/
      if (make_dir(globinfo.dest_tmpdir))             // Make tempdir
      {                                               // if dir already exist
        make_dir(globinfo.dest_tmpsavdir);
        move_dircontent(globinfo.dest_tmpdir,globinfo.dest_tmpsavdir); // move cont. to sav
      }

      if (modes->cformat=='j')
      {
        /* Prepair generation standard avi fomat */
        ext=".avi";                  /* standard avi format */
        
        /* Prepair empty temp. dir for creating avi */
      }
      else
      {
        /* Prepair generation concatenated pgm/ppm fomat */
        if (grp_sel->compose)
          ext=".fpm";                /* concat. ppm */
        else
          ext=".fgm";                /* concat. pgm */
      }
     /* determine filename */
      make_filename(globinfo.dest_dir,pc,ext,mfno,modes,prefer->split_destdir); /* file for movie gen. */
      if (!(overwrite(mfno,globinfo.overwrite_mode)))
      {
        progress_gen=NULL;
        Set_Entry(window,LAB_INFO,"(Aborted)");
        return;
      }
      Remove_Grps(grp_sel);         /* Item handled; remove. */
    }
  } /* if (modes->gen_film) */

  /* create progress window */
  if (show_progress)
  {
    progress=Create_Progress(NULL,"Translate....",TRUE);
    progress_gen=progress;
  }
  
/* Loop in which picture files wil be generated.
   For pictures: filename is determined per pic.
   For movies  : temp files are generated; result is generated after this loop
                 result filename is already defined
*/
  get_selected_item_rng(grp,modes,TRUE);                    /* Init selection */
  while ((grp_sel=get_selected_item_rng(NULL,modes,TRUE)))  /* Get next selected image */
  {
    CHANNEL *chan;
    gboolean has_picture=FALSE;
    gboolean gr_ready=TRUE;
    if (globinfo.abort) break;                /* Stop this loop if abort request */

    if (show_progress)
    {
      if (Update_Progress(progress,nract,globinfo.nr_selpics)) break;
    }
    if ((grp_sel->orbittype==POLAR) && (modes->gmap!=normal))
    {
      grp_sel=gen_polarearth(grp_sel,modes,window);
      if (!grp_sel)
      {
        continue;
      }
    }
    pic_info(grp_sel,window,modes);           /* Get drawing figures (-> grp_sel->pc) */
    pc=&grp_sel->pc;

/* Check if group contains picture(s) */
    for (chan=grp_sel->chan; chan; chan=chan->next)
    {
      if ((chan->segm) && (chan->segm->xh.file_type==0) && (chan->nb))
      {
        has_picture=TRUE;
        break;
      }
    }

/* Generate pro/epi, admin or picture */
    if ((grp_sel->pro_epi) && ((grp_sel->pro_epi->selected) || 
                               ((grp_sel->pro_epi->pro) && (grp_sel->pro_epi->pro->selected)) ||
                               ((grp_sel->pro_epi->epi) && (grp_sel->pro_epi->epi->selected))))
    {
      err=gen_proepi_item(window,grp_sel,modes,prefer,tfno,&gr_ready);
      if (err)
      {
        Set_Entry(window,LAB_INFO,report_genpicerr_short(err,tfno));
        if ((err!=Aborted) && (err!=Exist_fn) && (err!=Mis_segm))
        {
          if (window) Create_Message("Error",report_genpicerr(err,tfno));
        }
      }
      else
      {
        char *fn=strrchr(tfno,DIR_SEPARATOR);
        if (fn) fn++; else fn=tfno;
        Set_Entry(window,LAB_INFO,"Created %s",fn);
        nract++;
      }
    }

    if (grp_sel->chan)
    {
      if ((grp_sel->chan->segm) && (grp_sel->chan->segm->xh.file_type==2) &&
               (!has_picture))
      {
        err=gen_admin_item(window,grp_sel,modes,prefer,tfno,&gr_ready);
        if ((err) && (err!=Aborted) && (err!=Exist_fn) && (err!=Mis_segm))
        {
          if (window) Create_Message("Error",report_genpicerr(err,tfno));
        }
        else
        {
          char *p=strrchr(tfno,DIR_SEPARATOR);
          Set_Entry(window,LAB_INFO,"Created %s",(p? p+1 : tfno));
          nract++;
        }
      }
      else if (has_picture)
      {
        int err=0;
        n1++;
        if (modes->skip_incomplete)
        {
          if (chan_selected(window,grp_sel->chan->chan_name))
          {    /* if translate window is up this channel is selected */
            if (!picture_complete(grp_sel->chan,pc,modes))
            {
              Set_Entry(window,LAB_INFO,"Incomplete %s",grp_sel->id);
              Set_Led(list_wnd,grp_sel->id,0x00f);              /* blue */
              Set_Led(list_wnd,grp_sel->chan->id,0x00f);        /* blue */
              if ((grp_sel->chan) && (chan_selected(window,grp_sel->chan->chan_name)))
                nract++;
              if (modes->log_func) modes->log_func(modes,grp_sel,tfno,Mis_segm);
              Remove_Grps(grp_sel);         /* Item handled; remove. */
              continue;
            }
          }
        }
        if (modes->cformat=='r') modes->image_oformat='r'; // needed here for filenamegen
        err=gen_pic_item(window,grp_sel,modes,prefer,tfno,&gr_ready);
        if (err==Aborted) break;
        
        /* if just 1 pic to process: Abort if done.*/
        if ((modes->do_one_pic) && (chan_selected(window,grp_sel->chan->chan_name)))
          break;

        if ((modes->gen_film) && (grp_sel->done))
        {
          Create_Filelist(&fl,tfno);
        }
        
        if (chan_selected(window,grp_sel->chan->chan_name)) nract++;
        
        if (err)
        {
          Set_Entry(window,LAB_INFO,"Error!");
        }
        else if (modes->otype=='f')
        {
          char *fn=strrchr(tfno,DIR_SEPARATOR);
          if (fn) fn++; else fn=tfno;
//          Set_Entry(window,LAB_INFO,"Created %s",fn);
        }
        else
          Set_Entry(window,LAB_INFO,"Generated %s",grp_sel->id);

        if ((modes->otype=='f') && (!modes->gen_film))
        {
          if ((!err) || (err==Exist_fn))
          {  
//            if ((Get_Button(window,LAB_VIEW)))                /* Launch viewer requested */ 
            if (globinfo.view_exported)                /* Launch viewer requested */ 
            {
              char *cmd;

              if ((cmd=launch_viewer(prefer,pc,tfno,modes)))
              {
                Set_Entry(window,LAB_INFO,"Command %s failed.",cmd); /* failed */
                if (window) Create_Message("Error","Command %s failed.",cmd);
              }
            }
          }
          else                                                 /* extraction error */
          {
            if ((err!=Aborted) && (err!=Exist_fn) && (err!=Mis_segm))
            {
              if (window) Create_Message("Error",report_genpicerr(err,tfno));
            }
          }
        }
      }
      else if (!grp_sel->chan->segm)
      {
        if (window) Create_Message("Error","Data not available.");
      }
      else if (grp_sel->h_or_l=='B')
      {
        err=gen_bufritem(window,grp_sel,modes,prefer,tfno);
//        Set_Entry(window,LAB_INFO,"");
      }
      else
      {
        err=gen_binitem(window,grp_sel,modes,prefer,tfno);
      }
    }
    else
    {
      if ((grp_sel->chan) && (grp_sel->chan->segm))
      {
        Create_Message("Error","Unsupported: not marked as HRIT or LRIT (%d/%d).",
                                      grp_sel->chan->segm->xh.file_type,grp_sel->h_or_l);
      }
    }

    if (gr_ready)   // if not: grp_sel will be removed by preview_pic
    {
      Remove_Grps(grp_sel);         /* Item handled; remove. */
    }

/* If grp_sel is used (drawn) then it is destroyed in clean-func of preview_pic! */
  }
  if (show_progress)
    Close_Progress(progress);

  if  (modes->gen_film)
  {
    char *pmfno=strrchr(mfno,DIR_SEPARATOR);
    if (pmfno) pmfno++; else pmfno=mfno; 
    if (modes->cformat=='j')
    {
      int n;
      if ((n=gen_avi(mfno,prefer,fl))>0)
        Set_Entry(window,LAB_INFO,"Created %s %d frames",pmfno,n);
      else
      {
        switch(n)
        {
          case Avi_picsize:
            Set_Entry(window,LAB_INFO,"AVI gen failed: different frame sizes!");
          break;
          case Avi_nrframes:
            Set_Entry(window,LAB_INFO,"AVI gen failed: unexpected # frames. %s corrupt!",pmfno);
          break;
          case Avi_write:
            Set_Entry(window,LAB_INFO,"AVI gen failed: can't write to %s",pmfno);
          break;
          default:
            Set_Entry(window,LAB_INFO,"AVI generation failed: %s",pmfno);
          break;
        }
      }
    }
    if (modes->cformat=='p')
    {
      if (!gen_cpgm(mfno,prefer))
        Set_Entry(window,LAB_INFO,"Created %s",pmfno);
      else
        Set_Entry(window,LAB_INFO,"Failed: %s",pmfno);
    }

    if (remove_temp)                              /* remove temp-dir */
    {
      remove_dircontent(globinfo.dest_tmpdir,NULL,TRUE);     /* remove content dir temp */
    }
  }
  
  Remove_Filelist(fl);  // remove filelist if present
  progress_gen=NULL;
#ifndef __NOGUI__
  if ((ext=Get_Entry(window,LAB_INFO)) && (!strcmp(ext,STARTING)))
    Set_Entry(window,LAB_INFO,"Nothing done?");
#endif  
}


