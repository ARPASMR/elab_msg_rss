#define RGBMAP
/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Pop-up windows for channel and color selection
 ********************************************************************/
#include "xrit2pic.h"
#include "avhrr.h"
#include <string.h>


extern GLOBINFO globinfo;
extern GROUP *globgrp;


#include "msgmappings.h"

static int search_chan(FMAP fm,char *chan)
{
  int j;
  for (j=0; j<fm.n; j++)
  {
    if (!strcmp(fm.fm[j].chan,chan)) return j;
  }
  return -1;
}

//   range T1 ... T2:
//     kelvins/val=Kpv=(T2-T1)/256
//     offset=-(Kpv*T1)
//     fact=Kpv (1.=100%)
#define MAXVAL 255
FMAP msgmap2fmap(MSGCMAP mm)
{
  FMAP fm;
  RGBMAPS rgb;
  float vpk;
  int i,j;
  memset(&fm,0,sizeof(fm));
  fm.use_temp=mm.use_temp;
  fm.n=0;

  for (i=0; i<3; i++)
  {
    if (i==0) rgb=mm.r; else if (i==1) rgb=mm.g; else  rgb=mm.b;
    fm.gamma[i]=rgb.gamma;

    vpk=(float)MAXVAL/(rgb.valmax-rgb.valmin);
    fm.offset[i]=-1.*vpk*rgb.valmin;
    if (*rgb.chanp)
    {
      if ((j=search_chan(fm,rgb.chanp)) < 0) j=fm.n;
      strcpy(fm.fm[j].chan,rgb.chanp);
      if (vpk)
      {
        switch(i)
        {
          case 0: if (!fm.fm[j].r) fm.fm[j].r=vpk; break;
          case 1: if (!fm.fm[j].g) fm.fm[j].g=vpk; break;
          case 2: if (!fm.fm[j].b) fm.fm[j].b=vpk; break;
        }
      }
      if (j==fm.n) fm.n++;
    }
    if (*rgb.chann)
    {
      vpk*=-1.;
      if ((j=search_chan(fm,rgb.chann)) < 0) j=fm.n;
      strcpy(fm.fm[j].chan,rgb.chann);
      if (vpk)
      {
        switch(i)
        {
          case 0: if (!fm.fm[j].r) fm.fm[j].r=vpk; break;
          case 1: if (!fm.fm[j].g) fm.fm[j].g=vpk; break;
          case 2: if (!fm.fm[j].b) fm.fm[j].b=vpk; break;
        }
      }
      if (j==fm.n) fm.n++;
    }
  }
  return fm;
}

MSGCMAP fmap2msgmap(FMAP fm)
{
  int i,j;
  MSGCMAP mm;
  RGBMAPS *rgb;
  memset(&mm,0,sizeof(mm));

  for (i=0; i<3; i++)
  {
    if (i==0) rgb=&mm.r; else if (i==1) rgb=&mm.g; else  rgb=&mm.b;
    rgb->gamma=fm.gamma[i];
    if (fm.offset[i])  rgb->valmin=-1*fm.offset[i];

    for (j=0; j<fm.n; j++)
    {
      float fmrgb;
      if (i==0) fmrgb=fm.fm[j].r; else if (i==1) fmrgb=fm.fm[j].g; else fmrgb=fm.fm[j].b;

      if (fmrgb)
      {
        rgb->valmax=fmrgb*255;
        strcpy(rgb->chanp,fm.fm[j].chan);
     }
    }
  }
  return mm;
}

/*
 static char listchan[MAXCHAN][LENLISTCHAN];
 static int nrlistchan;
*/

static CHANMAP *Load_Fixed(FMAP *fmap)
{
  CHANMAP *cm=NULL,*cc;
  int i;

  for (i=0; i<fmap->n; i++)
  {
    cc=Create_Chanmap(&cm,fmap->fm[i].chan,fmap->fm[i].r,fmap->fm[i].g,fmap->fm[i].b);
    if (cc) cc->temperature=fmap->use_temp;
  }
  return cm;
}
#ifndef __NOGUI__



#define OFFS_RED_ON "!R_Offset"
#define OFFS_RED_VAL "!Offset R"
#define OFFS_GREEN_ON "!G_Offset"
#define OFFS_GREEN_VAL "!Offset G"
#define OFFS_BLUE_ON "!B_Offset"
#define OFFS_BLUE_VAL "!Offset B"

#define GAMM_RED_ON "!R_Gamma"
#define GAMM_RED_VAL "!Gamma R"
#define GAMM_GREEN_ON "!G_Gamma"
#define GAMM_GREEN_VAL "!Gamma G"
#define GAMM_BLUE_ON "!B_Gamma"
#define GAMM_BLUE_VAL "!Gamma B"




void set_mapping(GtkWidget *wdgt,int *offset,float *gamma,CHANMAP *chanmap)
{
  char tmp[50];
  CHANMAP *cm;
  if (!wdgt) return;
  Set_All_Buttons(Find_Parent_Window(wdgt),0);

  if (offset)
  {
    if (offset[0])
    {
      Set_Button(wdgt,OFFS_RED_ON,TRUE);
      Set_Adjust(wdgt,OFFS_RED_VAL,"%d",offset[0]);
    }
    else
    {
      Set_Button(wdgt,OFFS_RED_ON,FALSE);
      Set_Adjust(wdgt,OFFS_RED_VAL,"%d",0);
    }
    
    if (offset[1])
    {
      Set_Button(wdgt,OFFS_GREEN_ON,TRUE);
      Set_Adjust(wdgt,OFFS_GREEN_VAL,"%d",offset[1]);
    }
    else
    {
      Set_Button(wdgt,OFFS_GREEN_ON,FALSE);
      Set_Adjust(wdgt,OFFS_GREEN_VAL,"%d",0);
    }
    if (offset[2])
    {
      Set_Button(wdgt,OFFS_BLUE_ON,TRUE);
      Set_Adjust(wdgt,OFFS_BLUE_VAL,"%d",offset[2]);
    }
    else
    {
      Set_Button(wdgt,OFFS_BLUE_ON,FALSE);
      Set_Adjust(wdgt,OFFS_BLUE_VAL,"%d",0);
    }
  }

  if (gamma)
  {
    if (gamma[0]!=1.0)
    {
      Set_Button(wdgt,GAMM_RED_ON,TRUE);
      Set_Adjust(wdgt,GAMM_RED_VAL,"%f",gamma[0]);
    }
    else
    {
      Set_Button(wdgt,GAMM_RED_ON,FALSE);
      Set_Adjust(wdgt,GAMM_RED_VAL,"%f",1.);
    }
    
    if (gamma[1]!=1.0)
    {
      Set_Button(wdgt,GAMM_GREEN_ON,TRUE);
      Set_Adjust(wdgt,GAMM_GREEN_VAL,"%f",gamma[1]);
    }
    else
    {
      Set_Button(wdgt,GAMM_GREEN_ON,FALSE);
      Set_Adjust(wdgt,GAMM_GREEN_VAL,"%f",1.);
    }
    if (gamma[2]!=1.0)
    {
      Set_Button(wdgt,GAMM_BLUE_ON,TRUE);
      Set_Adjust(wdgt,GAMM_BLUE_VAL,"%f",gamma[2]);
    }
    else
    {
      Set_Button(wdgt,GAMM_BLUE_ON,FALSE);
      Set_Adjust(wdgt,GAMM_BLUE_VAL,"%f",1.);
    }
  }

  for (cm=chanmap; cm; cm=cm->next)
  {
    sprintf(tmp,"!I_%s",cm->chan_name);
    Set_Button(wdgt,tmp,cm->invert);

    if (cm->r)
    {
      sprintf(tmp,"!R_%s",cm->chan_name);
      Set_Button(wdgt,tmp,TRUE);
      sprintf(tmp,"!%s R",cm->chan_name);
      Set_Adjust(wdgt,tmp,"%d",(int)(cm->r*100));
    }
    if (cm->g)
    {
      sprintf(tmp,"!G_%s",cm->chan_name);
      Set_Button(wdgt,tmp,TRUE);
      sprintf(tmp,"!%s G",cm->chan_name);
      Set_Adjust(wdgt,tmp,"%d",(int)(cm->g*100));
    }
    if (cm->b)
    {
      sprintf(tmp,"!B_%s",cm->chan_name);
      Set_Button(wdgt,tmp,TRUE);
      sprintf(tmp,"!%s B",cm->chan_name);
      Set_Adjust(wdgt,tmp,"%d",(int)(cm->b*100));
    }
  }
}



void fmap2clrwnd(GtkWidget *wnd,FMAP *m)
{
  CHANMAP *cm;
  int *offset=NULL;
  float *gamma=NULL;

  cm=Load_Fixed(m);
  offset=m->offset;
  gamma=m->gamma;
  wnd=Find_Window(wnd,LAB_COLOR);
  set_mapping(wnd,offset,gamma,cm);
}

#define HMAP 400
#define VMAP 100
#ifdef RGBMAP

#define IS 12
//      l*(w-(2*IS))
// x = ------------- + IS
//        lmax
#define L2X(l,lmax,w) (lmax? (l*(w-2*IS)/lmax + IS) : 0)
#define X2L(x,lmax,w) (w? ((x-IS)*lmax/(w-2*IS)) : 0)

//            l*(h-2*IS)
// y = h-IS - ---------- 
//             lmax
//#define L2Y(l,lmax,h) (lmax? (h-IS-((l*(h-2*IS))/lmax)) : h)
#define Y2L(y,lmax,h) ((h-IS-y)*lmax/(h-2*IS))

int L2Y(int l,int lmax,int h)
{
  int y;
  if (lmax==0)
  {
    y=h;
  }
  else
  {
    if (l<0) l=0;
    if (l>lmax) l=lmax;
    y=(h-IS-((l*(h-2*IS))/lmax));
  }
  return y;
}

static void vbar(RGBI *rgbi,int i,int lminrgb,int lmaxrgb,int lummax,int toplumrgb,gboolean is_color)
{
  GdkColor clr;
  int xl,xh,y,yp;
  char str[20];
  clr.red=0x00; clr.green=0xff; clr.blue=0xff;

  yp=L2Y(0,toplumrgb,VMAP);
  y=L2Y(toplumrgb,toplumrgb,VMAP);
  if (!is_color) { yp*=1; i=0; }

  xh=L2X(lmaxrgb,lummax,HMAP);
  xl=L2X(lminrgb,lummax,HMAP);

  if (xl!=xh)
  {
    draw_rgbline(rgbi,&clr,xh,yp+i*VMAP,xh,y+i*VMAP);
    sprintf(str,"%d",lmaxrgb);
    draw_rgbstring(rgbi,&clr,xh+2,yp+i*VMAP+2,str);

    draw_rgbline(rgbi,&clr,xl,yp+i*VMAP,xl,y+i*VMAP);
    sprintf(str,"%d",lminrgb);
    draw_rgbstring(rgbi,&clr,xl+2,yp+i*VMAP+2,str);
  }
}

#define WIDTH_CANVAS HMAP+15
#define HEIGHT_CANVAS VMAP*4+10
#define HEIGHT_CANVAS1 VMAP+10
static void draw_clrgraph(RGBI *rgbi,RGBPICINFO *rgbpi,gboolean is_color)
{
  GdkColor clr;
  int x,y,xp,yp,l,i;
  unsigned long toplum=0,tl,lumst;
  unsigned long toplumrgb[3]={0,0,0};
  int lummax;
  int lumscale=(rgbpi->lummax+1)/NLUMSTAT;
  if (lumscale==0) lumscale=1;
  lummax=rgbpi->lummax/lumscale;
  if (!rgbi) return;
  for (l=15; l<NLUMSTAT -1; l++)  // was: i<lummax, gaat mis als e kleur >256 is
  {
    if (l==51) continue;
    toplum=MAX(toplum,rgbpi->lumstat[l]);
    toplumrgb[0]=MAX(toplumrgb[0],rgbpi->lumstatrgb[0][l]);
    toplumrgb[1]=MAX(toplumrgb[1],rgbpi->lumstatrgb[1][l]);
    toplumrgb[2]=MAX(toplumrgb[2],rgbpi->lumstatrgb[2][l]);
  }
  if (!toplum) return;

  // background
  clr.red=clr.green=clr.blue=0x16;
  if (is_color)
    for (i=0; i<HEIGHT_CANVAS; i++) 
      draw_rgbline(rgbi,&clr,0,i,WIDTH_CANVAS,i);
  else
    for (i=0; i<HEIGHT_CANVAS1; i++) 
      draw_rgbline(rgbi,&clr,0,i,WIDTH_CANVAS,i);

  // kaders
  if (is_color)
  {
    for (i=0; i<4; i++)
    {
      switch(i)
      {
        case 0: clr.red=0xff; clr.green=0x00; clr.blue=0x00; break;
        case 1: clr.red=0x00; clr.green=0xff; clr.blue=0x00; break;
        case 2: clr.red=0x00; clr.green=0x00; clr.blue=0xff; break;
        case 3: clr.red=0xff; clr.green=0xff; clr.blue=0xff; break;
      }
      draw_rgbrect(rgbi,&clr,1,i*VMAP+1,HMAP,VMAP-2);
    }
  }
  else
  {
    clr.red=0xff; clr.green=0xff; clr.blue=0xff; 
    draw_rgbrect(rgbi,&clr,1,0+1,HMAP,VMAP-2);
  }

  if (!is_color)
  {
    clr.red=clr.green=clr.blue=0xff;
    for (l=0; l<=lummax; l++)
    {
      x=L2X(l,lummax,HMAP);
      y=L2Y(rgbpi->lumstat[l],toplum,VMAP);
      if (y<0) y=IS;
//      y*=4;
      if (l) draw_rgbline(rgbi,&clr,xp,yp,x,y);
      yp=y;
      xp=x;
    }
    vbar(rgbi,3,rgbpi->lmin,rgbpi->lmax,rgbpi->lummax,toplum,is_color);
  }


  if (is_color)
  {
    int lmin,lmax;
    for (i=0; i<4; i++)
    {
      clr.red=clr.green=clr.blue=0;
      switch(i)
      {
        case 0: clr.red=0xff; clr.green=0x00; clr.blue=0x00; break;
        case 1: clr.red=0x00; clr.green=0xff; clr.blue=0x00; break;
        case 2: clr.red=0x00; clr.green=0x00; clr.blue=0xff; break;
        case 3: clr.red=0xff; clr.green=0xff; clr.blue=0xff; break;
      }
      for (l=0; l<=lummax; l++)
      {
        x=L2X(l,lummax,HMAP);
        if (i<3)
        {
          lmin=rgbpi->lminrgb[i];
          lmax=rgbpi->lmaxrgb[i];
          lumst=rgbpi->lumstatrgb[i][l];
          tl=toplumrgb[i];
        }
        else
        {
          lmin=rgbpi->lmin;
          lmax=rgbpi->lmax;
          lumst=rgbpi->lumstat[l];
          tl=toplum;
        }

        y=L2Y(lumst,tl,VMAP);
        if (y<0) y=IS;
        if (l) draw_rgbline(rgbi,&clr,xp,yp+i*VMAP,x,y+i*VMAP);
        yp=y;
        xp=x;
      }
      vbar(rgbi,i,lmin,lmax,rgbpi->lummax,tl,is_color);
    }
  }
}

// rgbpi from preview window!
static void Update_Graph(GtkWidget *widget,RGBPICINFO *rgbpi,gboolean is_color)
{
  RGBI *rgbi=Get_RGBI(widget);
  if (!widget) return;
  draw_clrgraph(rgbi,rgbpi,is_color);
  Refresh_Rect(widget,0,0,rgbi->width,rgbi->height);    /* Make new row visible */
}

#define CANVAS_KEY "key_canvas"
#define DRI_KEY "key_dri"
#define WND_KEY "key_prevwnd"
#define COLOR_KEY "key_color"

// rgbpi from preview window!
void refresh_graph(GtkWidget *window,RGBPICINFO *rgbpi,PIC_CHARS *pc)
{
  char wndname[200];
  GtkWidget *wndgraph,*canvas,*di,*drawable;
  sprintf(wndname,"Graph %s",pc->picname);
  wndgraph=Find_Window(window,wndname);
  if (wndgraph)
  {
    di=gtk_object_get_data((gpointer)wndgraph,DRI_KEY);
    canvas=gtk_object_get_data((gpointer)wndgraph,CANVAS_KEY);
    drawable=gtk_object_get_data((gpointer)canvas,CHILD_ID);
    Renew_RGBBuf(drawable);
    Update_Graph(drawable,rgbpi,(gboolean)gtk_object_get_data((gpointer)wndgraph,COLOR_KEY));
  }
}

#include "preview.h"
static gint graphfunc(GtkWidget *widget,GdkEventConfigure *event)
{
  GtkWidget *window=Find_Parent_Window(widget);

  DRAW_INFO *di=gtk_object_get_data((gpointer)window,DRI_KEY);
  RGBPICINFO *rgbpi=NULL;
  if (di) rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)di->drawable,RGBPI_DL);
  Renew_RGBBuf(widget);
// rgbpi from preview window!
  Update_Graph(widget,rgbpi,(gboolean)gtk_object_get_data((gpointer)window,COLOR_KEY));
  return TRUE;
}

static void close_graph(GtkWidget *widget)
{
  GtkWidget *window=Find_Parent_Window(widget);
  GtkWidget *canvas=gtk_object_get_data((gpointer)window,CANVAS_KEY);
  GtkWidget *drawing_area;
  DRAW_INFO *di=gtk_object_get_data((gpointer)window,DRI_KEY);
  if (di) di->graphwnd=NULL;

  if (!(drawing_area=gtk_object_get_data(GTK_OBJECT(canvas),CHILD_ID)))
    drawing_area=canvas;

  Close_RGBPic(drawing_area);
}

#define LAB_RGBSET "Lum->RGB"
#define LAB_REDRAW "Redraw"
#define LAB_ALUM "Alum"
#define LAB_ALUMRGB "Hue"


static void but_func(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;

  GtkWidget *window=Find_Parent_Window(widget);
  GtkWidget *canvas=gtk_object_get_data((gpointer)window,CANVAS_KEY);
  GtkWidget *drawable=gtk_object_get_data((gpointer)canvas,CHILD_ID);
  GtkWidget *prev_window=gtk_object_get_data((gpointer)window,WND_KEY);

  DRAW_INFO *di=gtk_object_get_data((gpointer)window,DRI_KEY);
  RGBPICINFO *rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)di->drawable,RGBPI_DL);

  if (!strcmp(name,LAB_RGBSET))
  {
    int i;
    if ((!rgbpi->lmaxrgb[0]) && (!rgbpi->lmaxrgb[1]) && (!rgbpi->lmaxrgb[2]))
    {
      for (i=0; i<3; i++)
      {
        rgbpi->lminrgb[i]=rgbpi->lmin;
        rgbpi->lmaxrgb[i]=rgbpi->lmax;
      }
      rgbpi->lmin=0;
      rgbpi->lmax=rgbpi->lummax/2;
      graphfunc(drawable,NULL);
    }
  }
  if (!strcmp(name,LAB_REDRAW))
  {
    draw_pic(prev_window);
  }
  if (!strcmp(name,LAB_ALUM))
  {
    Set_Button(prev_window,PMENU_ALUM,TRUE);
  }
  if (!strcmp(name,LAB_ALUMRGB))
  {
    Set_Button(prev_window,PMENU_ALUMRGB,TRUE);
  }
}

static void key_func(GtkWidget *widget, GdkEventKey *event)
{
  GtkWidget *window=Find_Parent_Window(widget);
  GtkWidget *prev_window=gtk_object_get_data((gpointer)window,WND_KEY);

  switch ((guchar)event->keyval)
  {
    case 'r':
      draw_pic(prev_window);
    break;
  }
}

static void mouse_func(GtkWidget *widget, GdkEventMotion *event)
{
  GtkWidget *window=Find_Parent_Window(widget);
  DRAW_INFO *di;
  RGBPICINFO *rgbpi;
  GtkWidget *prev_window;
  gboolean is_color;
  GdkModifierType state;
  int x,y;
  if (!(di=gtk_object_get_data((gpointer)window,DRI_KEY))) return;
  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)di->drawable,RGBPI_DL);
  prev_window=gtk_object_get_data((gpointer)window,WND_KEY);
  is_color=(gboolean)gtk_object_get_data((gpointer)window,COLOR_KEY);

  if (event->is_hint)
  {
    gdk_window_get_pointer(event->window,&x,&y,&state);
  }
  else
  {
    x=event->x;
    y=event->y;
    state=event->state;
  }

  if (state&GDK_BUTTON1_MASK)
  {
    int gnr,l;
    gnr=(y-2)/VMAP;
    gnr=MIN(MAX(gnr,0),3);
    x=MAX(x,11);
    x=MIN(x,HMAP-11);

    l=X2L(x,rgbpi->lummax,HMAP);
    if ((gnr<3) && (is_color))
    {
      if (rgbpi->lmaxrgb[gnr]-l < l-rgbpi->lminrgb[gnr])
        rgbpi->lmaxrgb[gnr]=l;
      else
        rgbpi->lminrgb[gnr]=l;
    }
    else
    {
      if (rgbpi->lmax-l < l-rgbpi->lmin)
        rgbpi->lmax=l;
      else
        rgbpi->lmin=l;
    }

    Set_Adjust(prev_window,Lab_LMIN,"%d",rgbpi->lmin);
    Set_Adjust(prev_window,Lab_LMAX,"%d",rgbpi->lmax);
    graphfunc(widget,NULL);
  }
}

void rgbmap(GtkWidget *iwnd,DRAW_INFO *di,char *name,gboolean is_color)
{
  GtkWidget *wnd,*canvas=NULL;
  char wndname[200];
  GtkWidget *w[5];
  sprintf(wndname,"Graph %s",name);
  wnd=Create_Window(iwnd,0,0,wndname,close_graph);
  if (!wnd) return;
  di->graphwnd=wnd;

//  canvas=Create_Canvas(wnd,0,0,graphfunc,key_func,mouse_func,mouse_func);
  if (is_color)
    canvas=Create_Canvas(wnd,WIDTH_CANVAS,HEIGHT_CANVAS,graphfunc,key_func,mouse_func,mouse_func);
  else
    canvas=Create_Canvas(wnd,WIDTH_CANVAS,HEIGHT_CANVAS1,graphfunc,key_func,mouse_func,mouse_func);

  gtk_object_set_data(GTK_OBJECT(wnd),CANVAS_KEY,(gpointer)canvas);
  gtk_object_set_data(GTK_OBJECT(wnd),DRI_KEY,(gpointer)di);
  gtk_object_set_data(GTK_OBJECT(wnd),WND_KEY,(gpointer)iwnd);
  gtk_object_set_data(GTK_OBJECT(wnd),COLOR_KEY,(gpointer)is_color);

  w[1]=Create_Button(LAB_RGBSET,but_func);
  w[2]=Create_Button(LAB_REDRAW,but_func);
  w[3]=Create_Button(LAB_ALUM,but_func);
  w[4]=Create_Button(LAB_ALUMRGB,but_func);
  
  w[0]=SPack(NULL,"h",w[1],"ef1",w[2],"ef1",w[3],"ef1",w[4],"ef1",NULL);

  w[0]=Pack("",'v',canvas,1,w[0],1,NULL);
//  w[0]=Pack("",'v',w[0],1,NULL);

  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show_all(wnd);
  return;
}
#endif

#endif
extern PREFER prefer;
void set_mapping_from_fixed(COMPOSE_TYPE n)
{
  CHANMAP *cm;
  MSGCMAP cms;
  MSGCMAP msgmap_cust=globinfo.clrmapspec;
  int *offset=NULL;
  float *gamma=NULL;
  FMAP fm;
  memset(&cms,0,sizeof(cms));

  globinfo.spm.compose_type=n;
  switch(n)
  {
    case map_vis_hmsg: fm=prefer.fmap_vis_hmsg; cms=fmap2msgmap(fm);  break;
    case map_vis_lmsg: fm=prefer.fmap_vis_lmsg; cms=fmap2msgmap(fm);  globinfo.spm.compose_type=map_vis_hmsg; break;
    case map_noaavis : fm=prefer.fmap_vis_noaa; cms=fmap2msgmap(fm);  globinfo.spm.compose_type=map_vis_hmsg; break;
    case map_metopvis: fm=prefer.fmap_vis_metop;cms=fmap2msgmap(fm);  globinfo.spm.compose_type=map_vis_hmsg; break;
    case map_airm    : cms=msgmap_airmass;      fm=msgmap2fmap(cms);  break;
    case map_dust    : cms=msgmap_dust;         fm=msgmap2fmap(cms);  break;
    case map_nfog    : cms=msgmap_nfog;         fm=msgmap2fmap(cms);  break;
    case map_dfog    : cms=msgmap_dfog;         fm=msgmap2fmap(cms);  break;

    case map_ucld    : cms=msgmap_cloud_micro;  fm=msgmap2fmap(cms);  break;
    case map_udust   : cms=msgmap_dust_micro;   fm=msgmap2fmap(cms);  break;
    case map_uash    : cms=msgmap_ash_micro;    fm=msgmap2fmap(cms);  break;
    case map_unight  : cms=msgmap_night_micro;  fm=msgmap2fmap(cms);  break;
    case map_uday    : cms=msgmap_day_micro;    fm=msgmap2fmap(cms);  break;
    case map_cnv_strm: cms=msgmap_conv_storms;  fm=msgmap2fmap(cms);  break;
    case map_snowfog : cms=msgmap_snowfog;      fm=msgmap2fmap(cms);  break;
    case map_cust1   : cms=msgmap_cust;         fm=msgmap2fmap(cms);  break;

    default          : fm=prefer.fmap_vis_hmsg;                       globinfo.spm.compose_type=map_vis_hmsg; break;
  }
  cm=Load_Fixed(&fm);  offset=fm.offset;  gamma=fm.gamma;
  if (globinfo.chanmap) Remove_Chanmap(&globinfo.chanmap);
  globinfo.chanmap=cm;
  globinfo.offset[0]=globinfo.offset[1]=globinfo.offset[2]=0;
  if (offset)
  {
    globinfo.offset[0]=offset[0];
    globinfo.offset[1]=offset[1];
    globinfo.offset[2]=offset[2];
  }

  if (gamma)
  {
    globinfo.gamma[0]=gamma[0];
    globinfo.gamma[1]=gamma[1];
    globinfo.gamma[2]=gamma[2];
  }

  globinfo.clrmapspec=cms;
}

void set_mapping_from_group(GtkWidget *wdgt,GROUP *grp)
{
  CHANMAP *cm=NULL;
  CHANNEL *chan;
  for (chan=grp->chan; chan; chan=chan->next)
  {
    Create_Chanmap(&cm,chan->chan_name,chan->r,chan->g,chan->b);
  }
  if (globinfo.chanmap) Remove_Chanmap(&globinfo.chanmap);

  globinfo.offset[0]=grp->offset[0];
  globinfo.offset[1]=grp->offset[1];
  globinfo.offset[2]=grp->offset[2];

  globinfo.gamma[0]=grp->gamma[0];
  globinfo.gamma[1]=grp->gamma[1];
  globinfo.gamma[2]=grp->gamma[2];

  globinfo.chanmap=cm;
#ifndef __NOGUI__
  if (Find_Window(wdgt,LAB_COLOR))
    color_map(wdgt,grp);
#endif
}

/*
  mapping from cm via fm to globinfo.chanmap, offset and gamma
*/
void set_mapping_from_cm(GtkWidget *wdgt,MSGCMAP cms)
{
  int *offset=NULL;
  float *gamma=NULL;
  FMAP fm;
  CHANMAP *cm;

  fm=msgmap2fmap(cms);
  cm=Load_Fixed(&fm);  offset=fm.offset;  gamma=fm.gamma;

  if (globinfo.chanmap) Remove_Chanmap(&globinfo.chanmap);
  globinfo.chanmap=cm;
  globinfo.offset[0]=globinfo.offset[1]=globinfo.offset[2]=0;
  if (offset)
  {
    globinfo.offset[0]=offset[0];
    globinfo.offset[1]=offset[1];
    globinfo.offset[2]=offset[2];
  }
  if (gamma)
  {
    globinfo.gamma[0]=gamma[0];
    globinfo.gamma[1]=gamma[1];
    globinfo.gamma[2]=gamma[2];
  }
  globinfo.clrmapspec=cms;

//    set_mapping(wdgt,offset,gamma,cm);    // set mapping

}


void get_mapping(GtkWidget *wdgt,char *wndname)
{
#ifndef __NOGUI__
  GROUP *grp_sel;
  CHANNEL *chan;
  GtkWidget *wnd;
  wnd=Find_Window(Find_Parent_Window(wdgt),wndname);
  if (wnd)                         /* Get colour mapping from gui */
  {
    char tmp[30];
    globinfo.offset[0]=globinfo.offset[1]=globinfo.offset[2]=0;
    globinfo.gamma[0]=globinfo.gamma[1]=globinfo.gamma[2]=1.;
    if (globinfo.chanmap) Remove_Chanmap(&globinfo.chanmap);

    if (Get_Button(wnd,OFFS_RED_ON))
      globinfo.offset[0]=Get_Adjust(wnd,OFFS_RED_VAL);
    if (Get_Button(wnd,OFFS_GREEN_ON))
      globinfo.offset[1]=Get_Adjust(wnd,OFFS_GREEN_VAL);
    if (Get_Button(wnd,OFFS_BLUE_ON))
      globinfo.offset[2]=Get_Adjust(wnd,OFFS_BLUE_VAL);
    if (Get_Button(wnd,GAMM_RED_ON))
      globinfo.gamma[0]=Get_Adjust(wnd,GAMM_RED_VAL);
    if (Get_Button(wnd,GAMM_GREEN_ON))
      globinfo.gamma[1]=Get_Adjust(wnd,GAMM_GREEN_VAL);
    if (Get_Button(wnd,GAMM_BLUE_ON))
      globinfo.gamma[2]=Get_Adjust(wnd,GAMM_BLUE_VAL);


    get_selected_item(globgrp,NULL);
    if ((grp_sel=get_selected_item(NULL,NULL)))
    {
      for (chan=grp_sel->chan; chan; chan=chan->next)
      {
        if (!chan->chan_name) continue;
        sprintf(tmp,"!I_%s",chan->chan_name);
        chan->invert=Get_Button(wnd,tmp);
        chan->r=chan->g=chan->b=0;
        sprintf(tmp,"!R_%s",chan->chan_name);
        if (Get_Button(wnd,tmp))
        {
          sprintf(tmp,"!%s R",chan->chan_name);
          if (Find_Widget(wnd,tmp))
            chan->r=(float)Get_Adjust(wnd,tmp)/100.;
          else
            chan->r=1.;
        }
        sprintf(tmp,"!G_%s",chan->chan_name);
        if (Get_Button(wnd,tmp))
        {
          sprintf(tmp,"!%s G",chan->chan_name);
          if (Find_Widget(wnd,tmp))
            chan->g=(float)Get_Adjust(wnd,tmp)/100.;
          else
            chan->g=1.;
        }
        sprintf(tmp,"!B_%s",chan->chan_name);
        if (Get_Button(wnd,tmp))
        {
          sprintf(tmp,"!%s B",chan->chan_name);
          if (Find_Widget(wnd,tmp))
            chan->b=(float)Get_Adjust(wnd,tmp)/100.;
          else
            chan->b=1.;
        }
        Create_Chanmap(&globinfo.chanmap,chan->chan_name,chan->r,chan->g,chan->b);

      }
      Remove_Grps(grp_sel);
    }
  }
#endif  
}

/* Get settings from channel mapping window into channel record */
void get_map(GtkWidget *wdgt,CHANNEL *chani,char *wndname,char *wndsmname)
{
#ifndef __NOGUI__
  GtkWidget *wnd;
#endif
  CHANNEL *chan;
  if (!chani) return;
  
#ifndef __NOGUI__
  wnd=Find_Window(Find_Parent_Window(wdgt),wndname);

  if (wnd)                         /* Get colour mapping from gui */
  {
    char tmp[30];
    if (chani->group)
    {
      chani->group->offset[0]=chani->group->offset[1]=chani->group->offset[2]=0;
      chani->group->gamma[0]=chani->group->gamma[1]=chani->group->gamma[2]=1.;
      if (Get_Button(wnd,OFFS_RED_ON))
        chani->group->offset[0]=Get_Adjust(wnd,OFFS_RED_VAL);
      if (Get_Button(wnd,OFFS_GREEN_ON))
        chani->group->offset[1]=Get_Adjust(wnd,OFFS_GREEN_VAL);
      if (Get_Button(wnd,OFFS_BLUE_ON))
        chani->group->offset[2]=Get_Adjust(wnd,OFFS_BLUE_VAL);
      if (Get_Button(wnd,GAMM_RED_ON))
        chani->group->gamma[0]=Get_Adjust(wnd,GAMM_RED_VAL);
      if (Get_Button(wnd,GAMM_GREEN_ON))
        chani->group->gamma[1]=Get_Adjust(wnd,GAMM_GREEN_VAL);
      if (Get_Button(wnd,GAMM_BLUE_ON))
        chani->group->gamma[2]=Get_Adjust(wnd,GAMM_BLUE_VAL);
    }

    for (chan=chani; chan; chan=chan->next)
    {
      sprintf(tmp,"!I_%s",chan->chan_name);
      chan->invert=Get_Button(wnd,tmp);

      chan->r=chan->g=chan->b=0;
      sprintf(tmp,"!R_%s",chan->chan_name);
      if (Get_Button(wnd,tmp))
      {
        sprintf(tmp,"!%s R",chan->chan_name);
        if (Find_Widget(wnd,tmp))
          chan->r=(float)Get_Adjust(wnd,tmp)/100.;
        else
          chan->r=1.;
      }
      sprintf(tmp,"!G_%s",chan->chan_name);
      if (Get_Button(wnd,tmp))
      {
        sprintf(tmp,"!%s G",chan->chan_name);
        if (Find_Widget(wnd,tmp))
          chan->g=(float)Get_Adjust(wnd,tmp)/100.;
        else
          chan->g=1.;
      }
      sprintf(tmp,"!B_%s",chan->chan_name);
      if (Get_Button(wnd,tmp))
      {
        sprintf(tmp,"!%s B",chan->chan_name);
        if (Find_Widget(wnd,tmp))
          chan->b=(float)Get_Adjust(wnd,tmp)/100.;
        else
          chan->b=1.;
      }
    }
  }
  else                                 /* load default values */
#endif
  {
    if (chani->group)
    {
      chani->group->offset[0]=globinfo.offset[0];
      chani->group->offset[1]=globinfo.offset[1];
      chani->group->offset[2]=globinfo.offset[2];
      chani->group->gamma[0]=globinfo.gamma[0];
      chani->group->gamma[1]=globinfo.gamma[1];
      chani->group->gamma[2]=globinfo.gamma[2];
      for (chan=chani; chan; chan=chan->next)
      {
        CHANMAP *cm;
        chan->r=chan->g=chan->b=0.;
        chan->invert=FALSE;
        if ((cm=Get_Chanmap(globinfo.chanmap,chan->chan_name)))
        {
          chan->invert=cm->invert;
          chan->r=cm->r;
          chan->g=cm->g;
          chan->b=cm->b;
        }
      }
    }
  }
  if ((globinfo.chanmap) && (chani->group))
  {
    chani->group->conv_temp=globinfo.chanmap->temperature;
  }
}

#include <stdlib.h> 
CHANMAP *Create_Chanmap(CHANMAP **si,char *chan_name,float r,float g,float b)
{
  CHANMAP *sn,*s=NULL;
  if (si) s=*si;

  if (!(sn=Get_Chanmap(*si,chan_name)))
  {
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
  }
  if (chan_name)
  {
    strncpy(sn->chan_name,chan_name,19);
    sn->r+=r;
    sn->g+=g;
    sn->b+=b;
  }
  return sn;
}

void Remove_Chanmap(CHANMAP **si)
{
  CHANMAP *s,*snext;
  if (!si) return;

  for (s=*si; s; s=snext)
  {
    snext=s->next;
    free(s);
  }
  *si=NULL;
}

CHANMAP *Get_Chanmap(CHANMAP *cm,char *chan_name)
{
  if (!chan_name) return NULL;
  for (; cm; cm=cm->next)
  {
    if (!strcmp(cm->chan_name,chan_name)) break;
  }
  return cm;
}

CHANMAP *Copy_Chanmap(CHANMAP *si)
{
  CHANMAP *s=NULL,*s1,*s1prev,*s1next;
  for (; si; si=si->next)
  {
    s1=Create_Chanmap(&s,NULL,0.,0.,0.);
    s1prev=s1->prev;
    s1next=s1->next;
    *s1=*si;
    s1->prev=s1prev;
    s1->next=s1next;
  }
  return s;
}



 
