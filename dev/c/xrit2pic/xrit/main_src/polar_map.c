/*******************************************************************
 * Copyright (C) 2006 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Polar mapping funcs
 ********************************************************************/

#include <stdlib.h>
#include <math.h>
#include "xrit2pic.h"
#include "gtk/gtk.h"
#include "gif.h"
#include "avhrr.h"
#define WNDW 600

#define LONOFFSET 180
#define LATOFFSET 90

/*
#define LON2Xt(l) ((int)((l+LONOFFSET-rgbpi->ox)*rgbpi->pwidth/rgbi->width*rgbpi->zx))
#define LAT2Yt(l) ((int)((LATOFFSET-l-rgbpi->oy)*rgbpi->pheight/rgbi->height*rgbpi->zy))
*/
/*
#define LON2X1(l) ((int)((l+LONOFFSET)*rgbpi->pwidth/360*rgbpi->zx)-rgbpi->ox*rgbpi->zx)
#define LAT2Y1(l) ((int)((LATOFFSET-l)*rgbpi->pheight/180*rgbpi->zy)-rgbpi->oy*rgbpi->zy)
*/


#define LAB_AGE "Age"
#define LAB_TIMEFRST "Time:"
#define LAB_SEGMFRST "Start:"
#define LAB_SEGMNR "Nbr.:"
#define LAB_PREVIEW "Preview"
#define LAB_MAKE "Export "
#define LAB_ALLCOV "Cov."
#define LAB_POS "Pos"
#define LAB_ASC "North bound"
#define LAB_DES "South bound"
#define LAB_BOTH "Both"
#define LAB_PDIREC "^Direction"
#define LAB_LTLAT "Lower lat"
#define LAB_GTLAT "Upper lat"
#define LAB_NPOL "North pole"
#define LAB_SPOL "South pole"
#define LAB_FPOL "Full globe"

extern GLOBINFO globinfo;
extern PREFER prefer;

// Funcs to translate (lon,lat) to (x,y) in case of plate carree mapping 
#define LON2X(l) ((int)((l+LONOFFSET)*(float)rgbi->width /360.*rgbpi->zx - (float)rgbpi->ox*rgbpi->zx*rgbi->width/rgbpi->pwidth))
#define LAT2Y(l) ((int)((LATOFFSET-l)*(float)rgbi->height/180.*rgbpi->zy - (float)rgbpi->oy*rgbpi->zy*rgbi->height/rgbpi->pheight))
#define X2LON(x) (((x+(rgbpi->ox*rgbpi->zx*rgbi->width/rgbpi->pwidth))*360/rgbi->width/rgbpi->zx)-LONOFFSET)
#define Y2LAT(y) (LATOFFSET-((y+(float)rgbpi->oy*rgbpi->zy*rgbi->height/rgbpi->pheight)*180/rgbi->height/rgbpi->zy))
void pcm_lonlat2xy(RGBI *rgbi,RGBPICINFO *rgbpi,float lon,float lat,int *x,int *y)
{
  *x=LON2X(lon);
  *y=LAT2Y(lat);
}




// Zie nb_main: eps_read_pos!!!
static void xyrect(GROUP *grp,RGBI *rgbi,RGBPICINFO *rgbpi,GdkColor clr,int x1,int ys1,int x2,int ys2)
{
  int xa[2],ya[2],y;
  float lon1,lon2,lat;

  y=ys1*grp->chan->nl;
  fxy2lonlat(x1,y,&lon1,&lat,&grp->pc);
  xa[0]=LON2X(lon1);
  ya[0]=LAT2Y(lat);
  y=ys2*grp->chan->nl;
  fxy2lonlat(x2,y,&lon2,&lat,&grp->pc);
  xa[1]=LON2X(lon2);
  ya[1]=LAT2Y(lat);
  if ((ABS(lon1-lon2))<90)
  {
    draw_rgbline(rgbi,&clr,xa[0],ya[0],xa[1],ya[1]);
  }
}

static void pijlrgb(RGBI *rgbi,GdkColor *clr,int x,int y,int r)
{
  int dx1,dy1,dx2,dy2;

  dx1=10*cos(D2R(r-10));
  dy1=10*sin(D2R(r-10));
  dx2=10*cos(D2R(r+10));
  dy2=10*sin(D2R(r+10));
  draw_rgbline(rgbi,clr,x+dx1,y+dy1,x,y);
  draw_rgbline(rgbi,clr,x+dx2,y+dy2,x,y);
}

static void mkpijl(GROUP *grp,RGBPICINFO *rgbpi,RGBI *rgbi,GdkColor *clr,int x,int ns)
{
  float lon1,lon2,lat1,lat2;
  fxy2lonlat(x,ns*grp->chan->nl+0  ,&lon1,&lat1,&grp->pc);
  fxy2lonlat(x,ns*grp->chan->nl+100,&lon2,&lat2,&grp->pc);
  pijlrgb(rgbi,clr,LON2X(lon1),LAT2Y(lat1),(int)R2D(atan2(D2R(lat2-lat1),D2R(lon1-lon2))));
}

int select_polar(SEGMENT *segm);

static void draw_sat(GROUP *grp,RGBPICINFO *rgbpi,RGBI *rgbi,gboolean show_all_cov)
{
  GdkColor clr,clr_sel,clr_nsel,clr_smis,clr_mis;
  int x,y,xmax,ymax,n,ns,px,py;
  int sel_first=globinfo.segmrange[0];
  int sel_last=globinfo.segmrange[1];
  int nrsegm=grp->chan->seq_end-grp->chan->seq_start+1;
  POINT pos,ppos;
  clr.red=clr.green=clr.blue=0;
  xmax=grp->chan->nc;
  ymax=grp->chan->nl*(grp->chan->seq_end-grp->chan->seq_start+1);
  sel_last=adapt_lastsegmnr(grp->chan->segm->xh.tlen, sel_first, sel_last);
  if (sel_first) sel_first--;
  clr_sel.red=0xff;  clr_sel.green=0x00;  clr_sel.blue=0x00;
  clr_nsel.red=0x7f; clr_nsel.green=0x7f; clr_nsel.blue=0x7f;
  clr_smis.red=0x00; clr_smis.green=0x00; clr_smis.blue=0xff;
  clr_mis.red=0x00;  clr_mis.green=0x00;  clr_mis.blue=0x00;
  if (!grp->pc.orbit.valid)
  {
    draw_rgbstring(rgbi,&clr_sel,10,10,"No Kepler for");
    draw_rgbstring(rgbi,&clr_sel,10,20,grp->sat_src);
    return;
  }

  px=py=-1;
  for (ns=0; ns<nrsegm; ns++)
  {
    gboolean selected=((ns>=sel_first) && (ns<sel_last));
    gboolean exist=(Get_Segm(grp->chan,ns+1)? TRUE : FALSE);
    SEGMENT  *csegm=Get_Segm(grp->chan,ns+1);
    if (exist)
    {
      if (!(select_polar(csegm))) continue;
    }
    else if ((globinfo.pdir_show=='a') || (globinfo.pdir_show=='d'))
    {
      continue;
    }
  
    if (selected)
    {
      if (exist) clr=clr_sel; else clr=clr_smis;
    }
    else
    {
      if (exist) clr=clr_nsel; else clr=clr_mis;
    }

    if (selected)                                       // first width line
    {
      xyrect(grp,rgbi,rgbpi,clr,0,ns,xmax/2,ns);        // x1
      xyrect(grp,rgbi,rgbpi,clr,xmax/2,ns,xmax,ns);     // x2
    }
    if ((selected) || (show_all_cov))                   // rest of rect
    {
      xyrect(grp,rgbi,rgbpi,clr,xmax,ns,xmax,ns+1);     // y
      xyrect(grp,rgbi,rgbpi,clr,xmax/2,ns+1,xmax,ns+1); // x2
      xyrect(grp,rgbi,rgbpi,clr,0,ns+1,xmax/2,ns+1);    // x1
      xyrect(grp,rgbi,rgbpi,clr,0,ns+1,0,ns);           // y
    }
    mkpijl(grp,rgbpi,rgbi,&clr,xmax/2,ns+1);
    for (n=0; n<grp->chan->nl; n+=(grp->chan->nl/10))
    {
      y=ns*grp->chan->nl+n;
      if (!(fxy2lonlat(xmax/2,y,&pos.lon,&pos.lat,&grp->pc))) break;
      x=LON2X(pos.lon);
      y=LAT2Y(pos.lat);

      if ((n) || (ns))
      {
        if (((ABS(pos.lon-ppos.lon)) < 90))
        {
          if ((px>=0) && (py>=0)) draw_rgbline(rgbi,&clr,px,py,x,y);
        }
      }
      ppos=pos;
      px=x;
      py=y;
    }

  }
}

static void draw_geosat(float sub_lon,GROUP *grp,RGBI *rgbi,RGBPICINFO *rgbpi)
{
  GdkColor clr;
  int x,y;

  clr.red=0xff;  clr.green=0x00;  clr.blue=0x00;

  x=LON2X(sub_lon);
  y=LAT2Y(0);
  draw_rgbrect(rgbi,&clr,x-5,y-5,10,10);
  draw_rgbrect(rgbi,&clr,x-4,y-4,8,8);
}

extern GROUP *globgrp;

static int drawfunc(GtkWidget *drawing_area)
{
  GtkWidget *wnd=Find_Parent_Window(drawing_area);
  guchar *aarde=gtk_object_get_data((gpointer)wnd,"PICDATA");
  int show_all_cov;
  RGBPICINFO *rgbpi;
  RGBI *rgbi;
  GROUP *sgrp=get_first_selected_grp(globgrp);
  if (sgrp) sgrp->pc.orbittype=sgrp->orbittype;
  if (!drawing_area) return 0;
  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)drawing_area,RGBPI_DL);
  rgbi=Get_RGBI(drawing_area);
  show_all_cov=Get_Button(drawing_area,LAB_ALLCOV);
  globinfo.pdir_show=(Get_Optionsmenu(drawing_area,LAB_PDIREC)==2? 'd' :
                      Get_Optionsmenu(drawing_area,LAB_PDIREC)==1? 'a' :
                      'b');

  Renew_RGBBuf(drawing_area);
  if (!rgbpi) return 0;
  if (!rgbpi->zx) rgbpi->zx=1;
  if (!rgbpi->zy) rgbpi->zy=1;
  if ((aarde) && (rgbi) && (rgbpi))
    draw_backgrnd(aarde,rgbi,rgbpi);

  globinfo.ltlat=(int)Get_Adjust(drawing_area,LAB_LTLAT);
  globinfo.gtlat=(int)Get_Adjust(drawing_area,LAB_GTLAT);
  // Draw sat
  if (sgrp)
  {
    if (strchr("GAM",sgrp->h_or_l))
    {
      draw_sat(sgrp,rgbpi,rgbi,show_all_cov);
      Set_Entry(wnd,LAB_AGE,"%d",sgrp->pc.orbit.age_days);
    }
    else if ((sgrp->chan) && (sgrp->chan->segm))
    {
      draw_geosat(sgrp->chan->segm->xh.sub_lon,sgrp,rgbi,rgbpi);
      Set_Entry(wnd,LAB_AGE," ");
    }
  }

  // draw selected area
  if (globinfo.area_nr)
  {
    GdkColor clr;
    int x1,y1,x2,y2;
    float clon=prefer.geoarea[globinfo.area_nr-1].center.lon;
    float clat=prefer.geoarea[globinfo.area_nr-1].center.lat;
    float dlon=prefer.geoarea[globinfo.area_nr-1].delta.lon;
    float dlat=prefer.geoarea[globinfo.area_nr-1].delta.lat;

    clr.red=0xff;  clr.green=0x00;  clr.blue=0xff;
    x1=LON2X(clon-dlon);
    y1=LAT2Y(clat-dlat);
    x2=LON2X(clon+dlon);
    y2=LAT2Y(clat+dlat);
    draw_rgbrect(rgbi,&clr,x1,y1,x2-x1,y2-y1);
    
  }
  
  // Draw other lon/lat, if needed
  
  Refresh_Rect(drawing_area,0,0,rgbi->width,rgbi->height);    /* Make new row visible */
  return 0;
}

static char *cur_time(GROUP *grp,int ns)
{
  static char stim[20];
  struct tm tm;
  int min_segm;
  if (!grp) return "??";
  tm=grp->grp_tm;
  if ((grp) && (grp->chan) && (grp->chan->segm) && (grp->chan->segm->next))
    min_segm=grp->chan->segm->next->xh.time.tm_min-grp->chan->segm->xh.time.tm_min;
  else
    min_segm=(grp->h_or_l=='M'? 3 : 1);
  if (min_segm<0) min_segm+=60;

  if (ns) ns--;
  ns*=min_segm;
  tm.tm_min+=ns;
  mktime_ntz(&tm);
  strftime(stim,19,"%H:%M",&tm);
  return stim;
}


static void do_but(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  GtkWidget *wnd_loc=Find_Parent_Window(widget);
  GtkWidget *wnd_main=Find_Window(wnd_loc,LAB_MAIN);
  GtkWidget *drawable=gtk_object_get_data(GTK_OBJECT(wnd_loc),"AARDEMAP");
  GROUP *sgrp=get_first_selected_grp(globgrp);
  if (!drawable) return;
  if (!sgrp) return;

  pol_savesegmselect(widget,sgrp->chan);    // also done in nb-main, this is for weview
  if (sgrp->orbittype==POLAR)
  {
    sgrp->segm_first=globinfo.segmrange[0];
    sgrp->segm_last=globinfo.segmrange[1];
  }

  if (!strcmp(name,LAB_SEGMFRST))
  {
    int ns=(int)Get_Adjust(wnd_loc,LAB_SEGMFRST);
    Set_Adjust(wnd_main,LAB_SEGMFRST,"%d",ns);
    Set_Entry(widget,LAB_TIMEFRST,"%s",cur_time(sgrp,ns));
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_SEGMNR))
  {
    Set_Adjust(wnd_main,LAB_SEGMNR,"%d",(int)Get_Adjust(wnd_loc,LAB_SEGMNR));
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_ALLCOV))
  {
    drawfunc(drawable);  
  }
  
  if (!strcmp(name,LAB_PREVIEW))
  {
    Set_Button(wnd_main,LAB_PREVIEW,TRUE);
  }
  if (!strcmp(name,LAB_MAKE))
  {
    Set_Button(wnd_main,LAB_MAKE,TRUE);
  }

  if (!strcmp(name,LAB_BOTH))
  {
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_ASC))
  {
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_DES))
  {
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_LTLAT))
  {
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_GTLAT))
  {
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_NPOL))
  {
    Set_Adjust(drawable,LAB_LTLAT,"%d",60);
    Set_Adjust(drawable,LAB_GTLAT,"%d",90);
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_SPOL))
  {
    Set_Adjust(drawable,LAB_GTLAT,"%d",-60);
    Set_Adjust(drawable,LAB_LTLAT,"%d",-90);
    drawfunc(drawable);  
  }
  if (!strcmp(name,LAB_FPOL))
  {
    Set_Adjust(drawable,LAB_GTLAT,"%d",90);
    Set_Adjust(drawable,LAB_LTLAT,"%d",-90);
    drawfunc(drawable);  
  }
}

static int get_selected_from_map(GROUP *grp,RGBPICINFO *rgbpi,RGBI *rgbi,int mx,int my)
{
  int nrsegm,ns,xmax,x,y;
  POINT pos;
  int dist=-1;
  int d,nsn=0;
  if (!grp) return 0;
  nrsegm=grp->chan->seq_end-grp->chan->seq_start+1;
  xmax=grp->chan->nc;
  for (ns=0; ns<nrsegm; ns++)
  {
    y=ns*grp->chan->nl+grp->chan->nl/2;

    if (!(fxy2lonlat(xmax/2,y,&pos.lon,&pos.lat,&grp->pc))) break;
    x=LON2X(pos.lon);
    y=LAT2Y(pos.lat);

    d=(((x-mx)*(x-mx)) + ((y-my)*(y-my)));
    if ((dist<0) || ( d < dist))
    {
      dist=d;
      nsn=ns+1;
    }
  }
  return nsn;
}


static void keyfunc(GtkWidget *widget,int data,int state,int x,int y,int px,int py)
{
  static gboolean freeze_pos;
  GtkWidget *wnd=Find_Parent_Window(widget);
  GtkWidget *drawing_area=widget;
  RGBPICINFO *rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)drawing_area,RGBPI_DL);
  RGBI *rgbi=Get_RGBI(drawing_area);
  GROUP *sgrp=get_first_selected_grp(globgrp);
  int ns;
  POINT pos;
  if (state&GDK_BUTTON1_MASK)
  {
    ns=get_selected_from_map(sgrp,rgbpi,rgbi,x,y);
    Set_Adjust(wnd,LAB_SEGMFRST,"%d",ns);
    Set_Entry(widget,LAB_TIMEFRST,"%s",cur_time(sgrp,ns));
  }
  if (state&GDK_BUTTON3_MASK)
  {
    GtkWidget *w=Find_Widget(widget,LAB_POS);
    freeze_pos=~freeze_pos;
    if (freeze_pos)
      Set_Widgetcolor(w,'f',0xff,0x00,0x00);
    else
      Set_Widgetcolor(w,'f',0x00,0x00,0x00);
  }
  if (!freeze_pos)
  {
    pos.lon=X2LON(x);
    pos.lat=Y2LAT(y);
    Set_Entry(widget,LAB_POS,"%5.1f,%6.1f",pos.lat,pos.lon);
  }
}

GtkWidget *mkbuts(GtkWidget *wndi,gboolean sep_wnd)
{
  GtkWidget *w[11];
  int segm_frst=Get_Adjust(wndi,LAB_SEGMFRST);
  int segm_nr=Get_Adjust(wndi,LAB_SEGMNR);
  GROUP *sgrp=get_first_selected_grp(globgrp);
  w[1]=Create_Spin(LAB_SEGMFRST,do_but,"%2d%2d%2d",segm_frst,0,999);
  w[2]=Create_Spin(LAB_SEGMNR,do_but,"%2d%2d%2d",segm_nr,0,999);
  w[3]=w[4]=NULL;
  if (sep_wnd)
  {
    w[3]=Create_Button(LAB_PREVIEW,do_but);
    w[4]=Create_Button(LAB_MAKE,do_but);
  }
  w[5]=Create_Check(LAB_ALLCOV,do_but,FALSE);
  w[6]=Create_Entry(LAB_POS,NULL,"%6s","");
  w[7]=Create_Entry(LAB_TIMEFRST,NULL,"%5s",cur_time(sgrp,0));
  w[8]=Create_Entry(LAB_AGE,NULL,"%6s","?");
  w[7]=Pack(NULL,'h',w[7],1,w[8],1,NULL);
  w[8]=Create_Optionmenu(LAB_PDIREC,do_but,0,LAB_BOTH,LAB_ASC,LAB_DES,0);

  w[9]=Create_ButtonArray("Limit",do_but,5,
                          SPIN,LAB_LTLAT,"%2d%2d%2d%d",5,-90,-90,90,
                          SPIN,LAB_GTLAT,"%2d%2d%2d%d",5,90,-90,90,
                          BUTTON,LAB_NPOL,
                          BUTTON,LAB_SPOL,
                          BUTTON,LAB_FPOL,
                          0);
  w[8]=Pack("Select to process",'h',w[8],1,w[9],1,NULL);

  w[1]=Pack("",'h',w[1],1,w[2],1,w[5],1,NULL);
  if (sep_wnd)
  {
    w[3]=Pack("",'h',w[3],3,w[4],3,NULL);
    w[1]=Pack(NULL,'h',w[1],5,w[3],5,w[6],1,w[7],1,NULL);
  }
  else
  {
    w[1]=Pack(NULL,'h',w[1],5,w[6],1,w[7],1,NULL);
  }
  w[1]=Pack(NULL,'v',w[1],1,w[8],1,NULL);
  return w[1];
}

GtkWidget *open_earth(GtkWidget *wndi,gboolean sep_wnd)
{
  GROUP *sgrp;
  sgrp=get_first_selected_grp(globgrp);
  calc_polar_segmsubsats(sgrp,FALSE);
  return open_earthmap((sep_wnd? NULL : wndi),LAB_EARTHWND,prefer.earthmapfile1,wndi,mkbuts(wndi,sep_wnd),drawfunc,keyfunc,NULL);
}
