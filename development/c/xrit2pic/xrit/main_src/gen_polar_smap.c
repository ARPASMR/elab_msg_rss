#include "xrit2pic.h"
#include <malloc.h>
#include <string.h>
extern PREFER prefer;
extern GLOBINFO globinfo;
/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/* 
 * Generate polar pic using special mapping.
 * A new group is generated containing the projected pic.
  * Note: grp->orbittype==POLAR
 */
#define AVHRR_NRCHANS 5
GROUP *gen_polarearth(GROUP *grp,GENMODES *modes,GtkWidget *window)
{
  GROUP *grpo;
  CHANNEL *chano,*chan;
  SEGMENT *segmo[AVHRR_NRCHANS],*segm[AVHRR_NRCHANS];
  SEGMENT *segmp[AVHRR_NRCHANS];
  int i;
  int pxl,pyl;
  int xoff,yoff,xfac,yfac;
  int ewidth,eheight;
  for (i=0; i<AVHRR_NRCHANS; i++)
  {
    segm[i]=NULL; segmo[i]=NULL; segmp[i]=NULL;
  }
  pic_info(grp,NULL,NULL);
  load_kepler(grp);
  calc_polar_segmsubsats(grp,FALSE);
  grp->pc.org_width=grp->pc.width;
  grp->pc.gmap=globinfo.gmap;
  grpo=Create_Grp(NULL);
  grpo->compose=grp->compose;
  grpo->grp_tm=grp->grp_tm;
  grpo->h_or_l=grp->h_or_l;
  strcpy(grpo->sat_src,grp->sat_src);
  strcpy(grpo->id,grp->id);

  /* Determine size of picto generate. */
  // Resolution GAC: max. 0.08 degrees ==> for max. res: w=4500
  // Resolution MET: max. 0.02 degrees ==> for max. res: w=18000
  if (grp->pc.org_width<500)
  {
    ewidth=3600;
  }
  else
  {
    ewidth=18000;
  }
  ewidth=MIN(ewidth,prefer.polar_size);  // if smaller size from preferences: take this
  eheight=ewidth/2;

  // Determine range of lon/lat, first for non-polar projection.
  if ((globinfo.gmap!=polar_n) && (globinfo.gmap!=polar_cn) &&
      (globinfo.gmap!=polar_s) && (globinfo.gmap!=polar_cs))
  {
    SEGMENT *isegm;
    // Determine min/max values of lon/lat using sub-sat values of each segment
    int lonmin,lonmax,latmin,latmax;
    if ((isegm=grp->chan->segm)) 
    {
      lonmin=isegm->orbit.subsat_start.lon;
      lonmax=lonmin;
      latmin=isegm->orbit.subsat_start.lat;
      latmax=latmin;
    }
    for (isegm=grp->chan->segm; isegm; isegm=isegm->next)
    {
      lonmin=MIN(lonmin,isegm->orbit.subsat_start.lon);
      lonmin=MIN(lonmin,isegm->orbit.subsat_end.lon);
      lonmax=MAX(lonmax,isegm->orbit.subsat_start.lon);
      lonmax=MAX(lonmax,isegm->orbit.subsat_end.lon);
      latmin=MIN(latmin,isegm->orbit.subsat_start.lat);
      latmin=MIN(latmin,isegm->orbit.subsat_end.lat);
      latmax=MAX(latmax,isegm->orbit.subsat_start.lat);
      latmax=MAX(latmax,isegm->orbit.subsat_end.lat);
    }

    // Stredge range; if nearly to poles: stredge to poles
    lonmin-=20;
    lonmax+=20;
    latmin-=10;
    latmax+=10;
    if (latmin<=-70) latmin=-90;
    if (latmax>= 70) latmax= 90;
    lonmin=MAX(lonmin,-180);
    lonmax=MIN(lonmax,180);

    // Decrease # pixels to new pic size
    ewidth=ewidth*(lonmax-lonmin)/360;
    eheight=eheight*(latmax-latmin)/180;

    // Store the factors between width/heijt and total lon/lat range
    xfac=ewidth/(lonmax-lonmin);
    yfac=eheight/(latmax-latmin);
    if (xfac>yfac) yfac=xfac;
    if (xfac<yfac) xfac=yfac;
    xoff=-1*lonmin;
    yoff=-1*latmin;
  }
  else
  {
    // polar mapping ==> fixed size
    xfac=ewidth/180;
    yfac=eheight/180;
    xoff=90;
    yoff=90;
  }
  grpo->pc.xoff=xoff;
  grpo->pc.xfac=xfac;
  grpo->pc.yoff=yoff;
  grpo->pc.yfac=yfac;

  // Set calculated sizes into pic_chars
  grpo->pc.width=ewidth;
  grpo->pc.org_width=grpo->pc.width;
  grpo->pc.height=eheight;
  grpo->pc.orbittype=grp->pc.orbittype;
  grpo->orbittype=grp->orbittype;

  // Create a new group/channel/segment set for mapped pic
  // and allocate space for chunks
  for (chan=grp->chan,i=0; chan; chan=chan->next,i++)
  {
    chano=Create_Chan(&grpo->chan);
    chano->group=grpo;

    strcpyd(&chano->chan_name,chan->chan_name);
    strcpyd(&chano->data_src,chan->data_src);
    chano->ifrmt=chan->ifrmt;
    strcpy(chano->id,chan->id);
    chano->scan_dir='n';
    chano->chan_nr=chan->chan_nr;
    chano->satpos=chan->satpos;
    chano->pic_nr=chan->pic_nr;
    chano->node=chan->node;

    chano->seq_start=1;
    chano->seq_end=1;

    chano->nc=ewidth;
    chano->nl=eheight;
    chano->nb=chan->nb;
    chano->ncext=ewidth;
    chano->nr_segm=1;
    if (i<AVHRR_NRCHANS)
    {
      SEGMENT *sp,*sn;
      segmo[i]=Create_Segm(&chano->segm);
      sp=segmo[i]->prev; 
      sn=segmo[i]->next; 
      segmo[i]->prev=sp; 
      segmo[i]->next=sn; 

      segmo[i]->chan=chano;
      segmo[i]->xh.hl='M';
      segmo[i]->xh.segment=1;
      segmo[i]->xh.image_iformat=0; // no input file
      segmo[i]->xh.file_type=0; 
      segmo[i]->xh.scan_dir='n';  
      segmo[i]->xh.hdr_type=1;

      segmo[i]->xh.nb=chano->nb;
      segmo[i]->xh.nc=ewidth;
      segmo[i]->xh.nl=eheight;
      segmo[i]->xh.data_len=segmo[i]->xh.nc*segmo[i]->xh.nl*2;
      segmo[i]->width=segmo[i]->xh.nc;
      segmo[i]->height=segmo[i]->xh.nl;

      segmo[i]->chnk=calloc(segmo[i]->xh.data_len,1);
      if (!segmo[i]) return grp;
    }
  }

  // Allocate space for overlay info
//  if (!segmo[0]->ovlchnk) // dit is onzin!?
  {
    segmo[0]->ovlchnk=calloc(segmo[0]->width*segmo[0]->height,sizeof(*segmo[0]->ovlchnk));
  }

  pic_info(grpo,NULL,modes);  // This is just to determine window name before generating pic.
  if (window)
  {
    if (Find_Window(window,grpo->pc.picname))
    {
      Create_Message("Info","To re-generate close preview window '%s'.",grpo->pc.picname);
      Remove_Grp(grpo);
      Remove_Grp(grp);
      return NULL;
    }
  }


  // Generate the projected pic
  progress_func('s',"Translating to lon/lat...",0,grp->pc.height);
  for (pyl=0; pyl<grp->pc.height; pyl+=1)   // all y's in drawable
  {
    if (!(progress_func('w',NULL,pyl,grp->pc.height))) break;
#ifndef __NOGUI__
    while (g_main_iteration(FALSE));
#endif

    for (chan=grp->chan,i=0; chan; chan=chan->next,i++)
    {
      int dshft,spos;

      // Get segm containing this y
      if (i<AVHRR_NRCHANS)
      {
        segm[i]=get_linefromsegm(chan,pyl,FALSE,0,&spos,&dshft,&grp->pc);
      }

      // free chunk if new segment used
      if (((segmp[i]) && (segmp[i]!=segm[i])))
      {
        free_segmchnk(segmp[i]);
      }
      segmp[i]=segm[i];
    }

    if (!segm[0]) continue;
    if (!segm[0]->height) continue;
    for (pxl=0; pxl<grp->pc.org_width; pxl+=1)
    {
      POINT pos;
      int xo,yo,pyl2;
      int poso_chnk,pos_chnk;
      pyl2=pyl%segm[0]->height;
      xy2lonlat(pxl,pyl,&grp->pc,&pos);                     // lon/lat of current pix
      lonlat2dxy(R2D(pos.lon),R2D(pos.lat),&xo,&yo,&grpo->pc); // plot into new segm struct
      pos_chnk=pxl+pyl2*grp->pc.org_width;  // chunk pos. in original segment
      poso_chnk=xo+yo*grpo->pc.org_width;   // projected chunk pos. in new segment

      // copy overlay
      if ((segmo[0]->ovlchnk) && (segm[0]->ovlchnk))
      { 
        segmo[0]->ovlchnk[poso_chnk]=segm[0]->ovlchnk[pos_chnk];
      }

      // copy pixel
      for (i=0; i<AVHRR_NRCHANS; i++)
      {
        if ((segmo[i]) && (segm[i]) && (segm[i]->chnk))
        {
          segmo[i]->chnk[poso_chnk]=segm[i]->chnk[pos_chnk];
        }
      }
    }
  } // for (py=0; py<rgbi->height; py++)

  Remove_Grp(grp);
  {
    OVERLAY *ovl=get_overlay(prefer.overlay,'v',NULL,NULL);
    add_shore(ovl,segmo[0],&grpo->pc);
  }
  progress_func('e',NULL,0,16);


  return grpo;
}
