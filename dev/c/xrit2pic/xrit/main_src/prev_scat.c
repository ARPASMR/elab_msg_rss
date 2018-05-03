/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Preview functions
 ********************************************************************/
#include <math.h>
#include "xrit2pic.h"
extern GLOBINFO globinfo;
extern PREFER prefer;
extern GROUP *globgrp;

static int drawfunc(GtkWidget *widget)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  GdkColor clr;
  ASCATPOINT *asp;
  SCATPOINT *sp;
  int x,y;

  guchar *aarde=gtk_object_get_data((gpointer)wnd,"PICDATA");
  GtkWidget *drawable=gtk_object_get_data((gpointer)wnd,"AARDEMAP");
  RGBI *rgbi=Get_RGBI(widget);
  RGBPICINFO *rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)widget,RGBPI_DL);
  if (drawable==NULL) return 0;

  rgbi=Get_RGBI(widget);
  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)widget,RGBPI_DL);
  Renew_RGBBuf(drawable);

  if ((aarde) && (rgbi) && (rgbpi))
    draw_backgrnd(aarde,rgbi,rgbpi);

  for (asp=globinfo.ascatp; asp; asp=asp->next)
  {
    for (sp=asp->sp; sp; sp=sp->next)
    {
      float lon2,lat2;
      int x2,y2;
      clr.red  =((prefer.scat_selclr>>8)&0xf)<<4;
      clr.green=((prefer.scat_selclr>>4)&0xf)<<4;
      clr.blue =((prefer.scat_selclr>>0)&0xf)<<4;
      pcm_lonlat2xy(rgbi,rgbpi,sp->lon,sp->lat,&x,&y);
      lon2=sp->lon-sp->speed/30.*sin(D2R(sp->sdir));  // sdir=90 -> to west
      lat2=sp->lat-sp->speed/30.*cos(D2R(sp->sdir));  // sdir= 0 -> to south
      pcm_lonlat2xy(rgbi,rgbpi,lon2,lat2,&x2,&y2);

      draw_scatarrow(NULL,rgbi,NULL,&clr,x,y,x2,y2);
    }
  }

  Refresh_Rect(drawable,0,0,rgbi->width,rgbi->height);
  return 0;
}

void gen_prev_scatmap(GtkWidget *wnd,GENMODES *modes,GROUP *grp)
{
  GROUP *grp1;
  get_selected_item_rng(grp,modes,TRUE);     /* Init selection */
  while ((grp1=get_selected_item_rng(NULL,modes,TRUE)))
  {
    globinfo.scat_plot_all=TRUE;
    if (grp1->h_or_l=='B')
    {
      SCATPOINT *sp;
      scat_extract(grp1->chan->segm->pfn,&sp);
      Create_AScat(&globinfo.ascatp,sp);
    }
  }
  open_earthmap(NULL,"Scat",prefer.earthmapfile2,wnd,NULL,drawfunc,NULL,NULL);
}

/*
 * Plot scat in sat-pic.
 * Expected: Selected one sat-pic, and one or more ERS-2.
 */
void gen_prev_scat(GtkWidget *wnd,GENMODES *modes,GROUP *grp)
{
  GROUP *grppic,*grpbufr;

  // Get selected pic-item
  get_selected_item_rng(grp,modes,TRUE);     /* Init selection */
  while ((grppic=get_selected_item_rng(NULL,modes,TRUE)))
  {
    if (grppic->h_or_l!='B') break;
  }
  if (!grppic) return;

  // Collect selected ERS-2 data
  get_selected_item_rng(grp,modes,TRUE);     /* Init selection */
  while ((grpbufr=get_selected_item_rng(NULL,modes,TRUE)))
  {
    globinfo.scat_plot_all=TRUE;
    if (grpbufr->h_or_l=='B')
    {
      SCATPOINT *sp;
      scat_extract(grpbufr->chan->segm->pfn,&sp);
      Create_AScat(&globinfo.ascatp,sp);
    }
  }

  // Plot pic + scattero info
  gen_item(wnd,grppic,modes,&prefer);
}

#ifdef OUD
int gen_prev_scat(GtkWidget *wnd,GENMODES *modes,GROUP *gsel1,GROUP *gsel2)
{
  if ((gsel1->h_or_l=='B') && (gsel2->h_or_l!='B'))
  {
    GROUP *grp=gsel1;
    gsel1=gsel2;
    gsel2=grp;
  }

  if ((gsel1->h_or_l!='B') && (gsel2->h_or_l=='B'))
  {
    scat_extract(gsel2->chan->segm->pfn,&globinfo.scatp);
    gen_item(wnd,gsel1,modes,&prefer);
  
    return 1;
  }
  return 0;
}
#endif
