/*******************************************************************
 * Copyright (C) 2008 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "xrit2pic.h"

#define WNDW 600
extern PREFER prefer;

#define LONLATLINES
#define LON2X(l) ((int)((l+LONOFFSET)*(float)rgbi->width /360.*rgbpi->zx - (float)rgbpi->ox*rgbpi->zx*rgbi->width/rgbpi->pwidth))
#define LAT2Y(l) ((int)((LATOFFSET-l)*(float)rgbi->height/180.*rgbpi->zy - (float)rgbpi->oy*rgbpi->zy*rgbi->height/rgbpi->pheight))
#define LONOFFSET 180
#define LATOFFSET 90

void draw_backgrnd(guchar *iaarde,RGBI *rgbi,RGBPICINFO *rgbpi)
{
  GdkColor clr;
  int x,y;
  int xx,yy;
  int lon,lat;
  // Draw background */
  for (y=0; y<rgbi->height; y++)
  {
    for (x=0; x<rgbi->width; x++)
    {
      xx=rgbpi->ox+x*rgbpi->pwidth/rgbpi->zx/rgbi->width;
      yy=rgbpi->oy+y*rgbpi->pheight/rgbpi->zy/rgbi->height;
      if ((xx>=rgbpi->pwidth) || (yy>=rgbpi->pheight))
      {
        clr.red=clr.green=clr.blue=0xaa;
      }
      else
      {
        clr.red=*(iaarde+0+3*(xx+yy*rgbpi->pwidth));
        clr.green=*(iaarde+1+3*(xx+yy*rgbpi->pwidth));
        clr.blue=*(iaarde+2+3*(xx+yy*rgbpi->pwidth));
      }

      draw_rgbpoint(rgbi,&clr,x,y);
    }
  }

#ifdef LONLATLINES
  // Draw lat lines */
  clr.red=clr.green=clr.blue=0; clr.blue=0xff;
  for (lat=-90; lat<=90; lat+=45)
  {
    y=LAT2Y(lat);
    if ((y>=0) && (y<rgbi->height))
    {
      for (x=0; x<rgbi->width; x++)
        draw_rgbpoint(rgbi,&clr,x,y);
    }
  }

  // Draw lon lines */
  for (lon=-180; lon<=180; lon+=45)
  {
    x=LON2X(lon);
    if ((x>=0) && (x<rgbi->width))
    {
      for (y=0; y<rgbi->height; y++)
        draw_rgbpoint(rgbi,&clr,x,y);
    }
  }
#endif
}
static void close_earth(GtkWidget *widget)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  GtkWidget *drawable=gtk_object_get_data((gpointer)wnd,"AARDEMAP");
  guchar *aarde=gtk_object_get_data((gpointer)wnd,"PICDATA");
  if (aarde) free(aarde);
  aarde=NULL;
  if (drawable)
  {
    Close_RGBPic(drawable);
  }
}

GtkWidget *open_earthmap(GtkWidget *ewnd,char *wname,char *fn_back,
                         GtkWidget *wndi,GtkWidget *buts,
                         int dfunc(),void kfunc(),gpointer data)
{
  GtkWidget *wnd,*canvas,*drawable,*w[2];
  static int width,height;
  char emapfile[300];
  guchar  *aarde=NULL;
  if (!ewnd)
  {
    if ((wnd=Find_Window(wndi,wname)))
    {
      gdk_window_show(wnd->window);
      return NULL;
    }
  }
  else wnd=ewnd;

#if __GTK_WIN32__ == 1
  if (!strncmp(fn_back+1,":\\",2))
#else
  if (*fn_back=='/')
#endif
  {
    strcpy(emapfile,fn_back);
  }
  else
  {
    if (!(search_file(fn_back,emapfile,
                prefer.cur_dir,prefer.home_dir,prefer.prog_dir)))
    {
      Create_Message("Error","Can't find %s.",fn_back);
      width=1000;
      height=500;
      aarde=calloc(width*height,3);
//      return NULL;
    }
  }

  if (strstr(emapfile,".jpg"))
  {
    if (!(jpg2str1(emapfile,&aarde,&width,&height))); // return NULL;
  }
  else
  {
    if (!(gif2str(emapfile,&aarde,&width,&height))); //  return NULL;
  }

  if (!ewnd)
  {
    wnd=Create_Window(wndi,WNDW,(height+90)*WNDW/width,wname,close_earth);
    if (!wnd)
    {
      if (aarde) free(aarde);
      return NULL;
    }
  }
  gtk_object_set_data((gpointer)wnd,"PICDATA",aarde);
  gtk_object_set_data((gpointer)wnd,"PLOTDATA",data);

  canvas=Create_Canvas_RGBPic_dr2(wnd,width,height,8,dfunc,kfunc);
  drawable=gtk_object_get_data(GTK_OBJECT(canvas),CHILD_ID);
  gtk_object_set_data(GTK_OBJECT(wnd),"AARDEMAP",drawable);
  w[0]=SPack(NULL,"v",canvas,"ef1",buts,"1",NULL);
  if (!ewnd)
  {
    gtk_container_add(GTK_CONTAINER(wnd),w[0]);
    gtk_widget_show_all(wnd);
  }
  Set_Enable_Update(drawable,TRUE);
  return w[0];
}
