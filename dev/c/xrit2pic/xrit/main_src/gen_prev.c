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
#include "xrit2pic.h"
#include <stdlib.h>
#include "gdk/gdkkeysyms.h"


extern GLOBINFO globinfo;
extern PREFER prefer;
extern GROUP *globgrp;
int draw_pic(GtkWidget *widget);

/***********************************************
 ***********************************************
 ** Preview functions
 ***********************************************
 ***********************************************/
/***********************************************
 * Preview admin messages
 ***********************************************/
void open_textwnd(char *fn,char *name,int width,int height,void mesfunc())
{
  FILE *fpi;
  GtkWidget *text;
  if (!(fpi=open_xritimage(fn,NULL,EXRIT,NULL))) if (!(fpi=fopen(fn,"r"))) return;
  text=Create_Info(NULL,name,width,height,NULL,NULL);
#ifndef __GTK_20__
  gtk_text_freeze(GTK_TEXT(text));
#endif
  mesfunc(fpi,text);
  fclose(fpi);
#ifndef __GTK_20__
  gtk_text_thaw(GTK_TEXT(text));
#endif
}

void preview_admin(GROUP *grp_sel,void mesfunc())
{
  open_textwnd(grp_sel->chan->segm->pfn,"Admin",700,500, mesfunc);
  Remove_Grps(grp_sel);
}

/***********************************************
 * Next functions are for drawing preview-picture
 ***********************************************/
#include "preview.h"

gboolean is_drawing;
static GENMODES genmode;

#define LAB_DUMMY "DUM"
#define LAB_DUMMY2 "!DUM2"
#define LAB_TCLR "!Temp C"
#define LAB_TCLR1 "Temp."
#define LAB_GAMMA "Gamma"
void errmess(char *frmt,...)
{
  va_list arg;
  va_start(arg,frmt);
  vfprintf(stdout,frmt,arg);
  va_end(arg);
}

#define LAB_RELPWND "RELATED_PREVWND"
/***********************************************
 * Exit preview; do clean-up
 ***********************************************/
void clean_func(GtkWidget *widget)
{
  GtkWidget *window=Find_Parent_Window(widget);
  GROUP *grp=(GROUP *)gtk_object_get_data((gpointer)window,GRP_DATA);
  DRAW_INFO *di=gtk_object_get_data((gpointer)window,"Draw_Info");
  GtkWidget *relwindow=gtk_object_get_data((gpointer)window,LAB_RELPWND);

  if (relwindow)
  {
    Close_Window(relwindow);
  }

  if (di)
  {
    di->stop_drawing=TRUE;
/*    if (di->drawing) return;  */   /* Don't close if drawing! */
  }

  if (grp)
  {
    gtk_object_remove_data((gpointer)window,LIST_DATA);
    Remove_Grps(grp);
  }

  if (di)
  {
    if (di->graphwnd) Close_Window(di->graphwnd);
    Close_RGBPic(di->drawable);
    Close_RGBPic(di->drawable_temp);
    gtk_object_remove_data((gpointer)window,"Draw_Info");
    free(di);
  }
  is_drawing=FALSE;
}

void Set_Button_All_wnd(GtkWidget *wnd,char *name,gboolean val)
{
  wnd=First_window(wnd);
  for (; wnd; wnd=Next_window(wnd))
  {
    if (Find_Widget(wnd,name))
    {
      Set_Button(wnd,name,val);
    }
  }
}

void lumrange(guint32 *lumstat,int lummax,int lumscale,int *ilmin,int *ilmax)
{
  int lmin,lmax;
  long toplum=0;
  lummax/=lumscale;
  for (lmin=53; lmin<lummax-1; lmin++) toplum=MAX(toplum,lumstat[lmin]);
  toplum/=10;
  toplum=MAX(toplum,1); 
  for (lmin=2; lmin<lummax-1; lmin++) if (lumstat[lmin]>=toplum) break;
  for (lmax=lummax; lmax>0; lmax--)   if (lumstat[lmax]>=toplum) break;
  if (lmin>=lmax) { lmin=0; lmax=lummax; }
  lmax*=lumscale;

  *ilmin=lmin;
  *ilmax=lmax;
}

#define ZOOM_DWND "Zoom details"
#define LAB_ZOOMBUT "Zoom and Offset"
#define LAB_ZOOMX2 "Zx "
#define LAB_ZOOMY2 "Zy "
#define LAB_OFFSX2 "Ox "
#define LAB_OFFSY2 "Oy "
#define PAR_WND "parent_window"
void adapt_zoomvals(GtkWidget *widget,RGBPICINFO *rgbpi)
{
  GtkWidget *wnd1=Find_Parent_Window(widget);
  GtkWidget *wnd=Find_Window(wnd1,ZOOM_DWND);

  Set_Adjust(wnd,LAB_ZOOMX2,"%f",rgbpi->zx);
  Set_Adjust(wnd,LAB_ZOOMY2,"%f",rgbpi->zy);
  Set_Adjust(wnd,LAB_OFFSX2,"%d",rgbpi->ox);
  Set_Adjust(wnd,LAB_OFFSY2,"%d",rgbpi->oy);
}
static void zoom2func(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  DRAW_INFO *di;
  RGBPICINFO *rgbpi;
  GtkWidget *wnd1=Find_Parent_Window(widget);
  GtkWidget *wnd=First_window(wnd1);
  wnd=(GtkWidget *)gtk_object_get_data((gpointer)wnd1,PAR_WND);
  if (!wnd) return;
  if (!(di=gtk_object_get_data((gpointer)wnd,"Draw_Info"))) return;
  if (!(rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)di->drawable,RGBPI_DL))) return;

  if (!strcmp(name,LAB_ZOOMX2))
  {
    rgbpi->zx=Get_Adjust(wnd1,LAB_ZOOMX2);
  }
  if (!strcmp(name,LAB_ZOOMY2))
  {
    rgbpi->zy=Get_Adjust(wnd1,LAB_ZOOMY2);
  }
  if (!strcmp(name,LAB_OFFSX2))
  {
    // In RGB_Pic_keyfunc: zie opm.: 20-04-2010
    rgbpi->ox=Get_Adjust(wnd1,LAB_OFFSX2);
  }
  if (!strcmp(name,LAB_OFFSY2))
  {
    // In RGB_Pic_keyfunc: zie opm.: 20-04-2010
    rgbpi->oy=Get_Adjust(wnd1,LAB_OFFSY2);
  }
}

void open_zoomdetails(GtkWidget *window,RGBPICINFO *rgbpi)
{
  GtkWidget *wnd,*wb;
  wnd=Create_Window(window,0,0,ZOOM_DWND,NULL);
  if (!wnd) return;
  gtk_object_set_data((gpointer)wnd,PAR_WND,(gpointer)window);
  wb=Create_ButtonArray(LAB_ZOOMBUT,zoom2func,2,
               SPIN,LAB_ZOOMX2,"%.2f%.2f%.2f",rgbpi->zx,0.01,100.,
               SPIN,LAB_OFFSX2,"%d%d%d",rgbpi->ox,0,20000,
               SPIN,LAB_ZOOMY2,"%.2f%.2f%.2f",rgbpi->zy,0.01,100.,
               SPIN,LAB_OFFSY2,"%d%d%d",rgbpi->oy,0,20000,
               0);
  gtk_container_add(GTK_CONTAINER(wnd),wb);
  gtk_widget_show_all(wnd);
}

void do_hue(RGBPICINFO *rgbpi)
{
  guint32 lumrgb[3],lumx;
  int i,j,l;
  for (i=0; i<3; i++)
  {
    lumrgb[i]=0;
    for (l=0; l<rgbpi->lummax; l++)
    {
      lumrgb[i]=lumrgb[i]+l*rgbpi->lumstatrgb[i][l];
    }
  }
  lumx=99999;
  for (i=0; i<3; i++)
  {
    lumrgb[i]=lumrgb[i]/1000;
    if (lumrgb[i]) lumx=MIN(lumx,lumrgb[i]);
  }
  
  for (j=0; j<3; j++)
  {
    if (lumrgb[j]==lumx) break;
  }
  if (j>2) return;
  for (i=0; i<3; i++)
  {
    if (i!=j) rgbpi->lmaxrgb[i]=rgbpi->lmaxrgb[i]*lumrgb[i]/lumrgb[j];
  } 
}

int screen_dump(RGBI *rgbi,char *fno,char type)
{
  GdkColor clr;
  FILE *fp;
  int x,y;
  char *line=NULL;
  if (!((fp=fopen(fno,"wb")))) return -1;
  if (type=='p')
  {
    write_pgmhdr(fp,rgbi->width,rgbi->height,8,1,NULL);
  }
  if (type=='j')
  {
    line=calloc(rgbi->width*rgbi->height,3);
    write_jpghdr(fp,rgbi->width,rgbi->height,8,75);
  }



  for (y=0; y<rgbi->height; y++)
  {
    for (x=0; x<rgbi->width; x++)
    {
      get_rgbpoint(rgbi,&clr,x,y);
      if (type=='p')
      {
        fwrite(&clr.red,1,1,fp);
        fwrite(&clr.green,1,1,fp);
        fwrite(&clr.blue,1,1,fp);
      }
      if (type=='j')
      {
        line[0+3*x]=clr.red;
        line[1+3*x]=clr.green;
        line[2+3*x]=clr.blue;
      }
    }
    if (type=='j') if (write_jpgline((guchar *)line)) break;
  }
  if (line) free(line);
  if (type=='j') close_wr_jpeg();
  fclose(fp);
  return 0;
}

#define LAB_ZOOMX "ZX"
#define LAB_ZOOMY "ZY"

#define PMENU_SXY "View/Show (x,y) coord."
#define ZOOMVALD "Zoom details"
/*******************************************************
 * 
 *******************************************************/
void saveprev_func(GtkWidget *widget,gpointer data)
{
  GtkWidget *window=Find_Parent_Window(widget);
  GtkWidget *main_window=Find_Window(widget,LAB_MAIN);
  RGBPICINFO *rgbpi;
  gboolean hold_chnk=Get_Button(main_window,LAB_HOLDCHNK);
  DRAW_INFO *di=gtk_object_get_data((gpointer)window,"Draw_Info");
  char tfno[1000];
  int err;
  int w,h;
  static int stop;

  GROUP *grp_sel=gtk_object_get_data((gpointer)window,GRP_DATA);

  if (!di) return;    // first time di maybe not defined.

  w=di->drawable->allocation.width;
  h=di->drawable->allocation.height;
  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)di->drawable,RGBPI_DL);

  rgbpi->redraw=FALSE;    // new request: reset redraw

  if ((!stop) && (!strcmp(data,LAB_FCLR_PREV)))
  {
    stop=1;
    if (!Get_Button(widget,LAB_FCLR_PREV))
    {
      Set_Button_All_wnd(window,LAB_FCLR_PREV,TRUE);
      Set_Button(widget,LAB_FCLR_PREV,FALSE);
      set_mapping_from_group(window,grp_sel);
    }
    stop=0;
  }

  if ((!strcmp(data,PMENU_SAVEPREV_JPG)) ||
      (!strcmp(data,PMENU_SAVEPREV_PGM)) ||
      (!strcmp(data,PMENU_SAVEPREV_PGM8)))
  {
    genmode.overwrite_mode=globinfo.overwrite_mode;
    genmode.filmframe_ovrwrmode=globinfo.filmframe_ovrwrmode;
    genmode.zx=MAX(rgbpi->zx,1.);
    genmode.ox=rgbpi->ox;
    genmode.zy=MAX(rgbpi->zy,1.);
    genmode.oy=rgbpi->oy;

    genmode.adapt_lum=TRUE;
    genmode.lmin=rgbpi->lmin;
    genmode.lmax=rgbpi->lmax;
    genmode.lminrgb[0]=rgbpi->lminrgb[0];
    genmode.lminrgb[1]=rgbpi->lminrgb[1];
    genmode.lminrgb[2]=rgbpi->lminrgb[2];
    genmode.lmaxrgb[0]=rgbpi->lmaxrgb[0];
    genmode.lmaxrgb[1]=rgbpi->lmaxrgb[1];
    genmode.lmaxrgb[2]=rgbpi->lmaxrgb[2];
    genmode.spm.invert=Get_Button(window,PMENU_INVERT);
    genmode.spm.inv_ir=globinfo.spm.inv_ir;
//    genmode.flip=Get_Button(window,PMENU_FLIP);
    if (!strcmp(data,PMENU_SAVEPREV_JPG))  genmode.cformat='j';
    if (!strcmp(data,PMENU_SAVEPREV_PGM))  genmode.cformat='p';
    if (!strcmp(data,PMENU_SAVEPREV_PGM8)) genmode.cformat='P';

    genmode.agl.shift_3d=di->anagl.shift_3d;
    genmode.agl.lmin=di->anagl.lmin;
    genmode.agl.lmax=di->anagl.lmax;
    genmode.agl.depthchan=di->anagl.depthchan;
    genmode.agl.ol_3d=di->anagl.ol_3d;
    genmode.ol_lum=di->ol_lum;
    genmode.add_lonlat=di->add_lonlat;
    genmode.add_overlay=di->add_overlay;
    genmode.gmap=di->gmap;

    // Next is workaround, to force colour mode if temp-mapping requested.
    // NOTE: Switching back to normal doesn't reset this;
    // result is generation ppm instead of pgm 
    // (not a big problem, but not nice)
    if (grp_sel->pc.map_temp) grp_sel->pc.is_color=TRUE;
    grp_sel->pc.bitshift=globinfo.bitshift;
    
    err=gen_picfile(window,grp_sel,&prefer,tfno,!hold_chnk,TRUE,&genmode);
    if (err)
    {
      if (err==Exist_fn)
        Create_Message("Warning",report_genpicerr(err,tfno));
      else
        Create_Message("Error",report_genpicerr(err,tfno));
    }
    else
    {
      char *fn=strrchr(tfno,DIR_SEPARATOR);
      if (fn) fn++; else fn=tfno;
      Set_Entry(main_window,LAB_INFO,"Created %s",fn);
    }
  }

  if (!strcmp(data,PMENU_SCREENDUMP))
  {
    RGBI *rgbi=Get_RGBI(di->drawable);
    make_filename(globinfo.dest_dir,&grp_sel->pc,".ppm",tfno,&genmode,prefer.split_destdir);    /* generate in dest dir */
    if (!screen_dump(rgbi,tfno,'p'))
    {
      char *fn=strrchr(tfno,DIR_SEPARATOR);
      if (fn) fn++; else fn=tfno;
      Set_Entry(main_window,LAB_INFO,"Created screendump %s",fn);
    }
  }

  if (!strcmp(data,LAB_DUMMY2))
  {
    draw_pic(widget);
  }

#define LAB_ZOOMVALS "Zoom"
  if (!strcmp(data,PMENU_ZOOMV))
  {
    if (Get_Button(widget,PMENU_ZOOMV))
    {
      gtk_widget_show(Find_Widget(widget,LAB_ZOOMVALS)->parent);
      Set_Adjust(widget,LAB_ZOOMX,"%f",rgbpi->zx);
      Set_Adjust(widget,LAB_ZOOMY,"%f",rgbpi->zy);
    }
    else
      gtk_widget_hide(Find_Widget(widget,LAB_ZOOMVALS)->parent);
  }

  if (!strcmp(data,PMENU_ZOOMF))
  {
    rgbpi->zx=1;
    rgbpi->zy=1;
    rgbpi->ox=0;
    rgbpi->oy=0;
    if (rgbpi->do_shape)
      makeit_square(w,h,rgbpi);
    draw_pic(widget);
  }

#define fix_zoomfactor 2
  if (!strcmp(data,PMENU_ZOOMI))
  {
    rgbpi->zx=rgbpi->zx*fix_zoomfactor;
    rgbpi->ox=rgbpi->ox+
                 (int)(rgbpi->pwidth/rgbpi->zx*(fix_zoomfactor-1)/2);
    rgbpi->ox=MAX(rgbpi->ox,0);
    rgbpi->ox=MIN(rgbpi->ox,rgbpi->pwidth);


    rgbpi->zy=rgbpi->zy*fix_zoomfactor;
    rgbpi->oy=rgbpi->oy+
                 (int)(rgbpi->pheight/rgbpi->zy*(fix_zoomfactor-1)/2);
    rgbpi->oy=MAX(rgbpi->oy,0);
    rgbpi->oy=MIN(rgbpi->oy,rgbpi->pheight);
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_ZOOMD))
  {
    open_zoomdetails(window,rgbpi);
  }

  if (!strcmp(data,PMENU_ZOOMO))
  {
    rgbpi->ox=rgbpi->ox-
                 (int)(rgbpi->pwidth/rgbpi->zx*(fix_zoomfactor-1)/2);
    rgbpi->zx=rgbpi->zx/fix_zoomfactor;

    rgbpi->oy=rgbpi->oy-
                 (int)(rgbpi->pheight/rgbpi->zy*(fix_zoomfactor-1)/2);
    rgbpi->zy=rgbpi->zy/fix_zoomfactor;
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_REDRAW))
  {
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_INVERT))
  {
    if (di) di->invert=Get_Button(window,PMENU_INVERT);
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_OVERLAY))
  {
    if (di)
    {
      int showlumbut;
      di->add_overlay=Get_Button(window,PMENU_OVERLAY);
      showlumbut=di->add_overlay | di->add_lonlat;
      Show_Button(Find_Parent_Window(widget),LAB_OLADD_PREV,showlumbut);
    }
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_MARK))
  {
    if (di)
    {
      di->add_marks=Get_Button(window,PMENU_MARK);
    }
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_CITY))
  {
    if (di)
    {
      di->add_cities=Get_Button(window,PMENU_CITY);
    }
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_LONLAT))
  {
    if (di)
    {
      int showlumbut;
      di->add_lonlat=Get_Button(window,PMENU_LONLAT);
      showlumbut=di->add_overlay | di->add_lonlat;
      Show_Button(Find_Parent_Window(widget),LAB_OLADD_PREV,showlumbut);
    }
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_FIRE))
  {
    if (di) di->fire_detect=Get_Button(window,PMENU_FIRE);
    draw_pic(widget);
  }

  if ((!strcmp(data,PMENU_TEMP)) || (!strcmp(data,PMENU_TEMPG)))
  {
    if (di)
    {
      di->map_temp=Get_Button(window,PMENU_TEMP);
      di->map_temp_G_mA=Get_Button(window,PMENU_TEMPG);
    }
    if ((di) && (di->map_temp) && (!di->map_temp_G_mA))
    {
      gtk_widget_show(Find_Widget(widget,LAB_TCLR));
      gtk_widget_hide(Find_Widget(widget,LAB_GAMMA)->parent);
    }
    else
    {
      gtk_widget_hide(Find_Widget(widget,LAB_TCLR));
      gtk_widget_show(Find_Widget(widget,LAB_GAMMA)->parent);
    }
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_LUT))
  {
    if (di)
    {
      di->use_lut=Get_Button(window,PMENU_LUT);
      if (di->use_lut) get_lut();
    }
    Sense_Button(widget,PMENU_TEMP,!di->use_lut);
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_BUFR))
  {
     handle_bufr(window,globgrp);
  }

  if (!strcmp(data,PMENU_AVLIN))
  {
    if (di) di->avhrr_lin=Get_Button(window,PMENU_AVLIN);
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_GMAPN))
  {
    if (di) if (Get_Button(window,PMENU_GMAPN)) di->gmap=normal;    // verander als meerdere maps
    draw_pic(widget);
  }
  if (!strcmp(data,PMENU_GMAPC))
  {
    if (di) if (Get_Button(window,PMENU_GMAPC)) di->gmap=plate_carree;    // verander als meerdere maps
    draw_pic(widget);
  }
  if (!strcmp(data,PMENU_GMAPM))
  {
    if (di) if (Get_Button(window,PMENU_GMAPM)) di->gmap=mercator;    // verander als meerdere maps
    draw_pic(widget);
  }
  if (!strcmp(data,PMENU_GMAPPN))
  {
    if (di) if (Get_Button(window,PMENU_GMAPPN)) di->gmap=polar_n;    // verander als meerdere maps
    draw_pic(widget);
  }
  if (!strcmp(data,PMENU_GMAPPS))
  {
    if (di) if (Get_Button(window,PMENU_GMAPPS)) di->gmap=polar_s;    // verander als meerdere maps
    draw_pic(widget);
  }

  if (!strcmp(data,PMENU_FLIP))
  {
    grp_sel->pc.scan_dir=(grp_sel->pc.scan_dir=='n'? 's' : 'n');
    draw_pic(widget);
  }
  if (!strcmp(data,PMENU_ALUMRGB))
  {
    int lumscale=(rgbpi->lummax+1)/NLUMSTAT;
    if (lumscale==0) lumscale=1;
    lumrange(rgbpi->lumstatrgb[0],rgbpi->lummax,lumscale,&rgbpi->lminrgb[0],&rgbpi->lmaxrgb[0]);
    lumrange(rgbpi->lumstatrgb[1],rgbpi->lummax,lumscale,&rgbpi->lminrgb[1],&rgbpi->lmaxrgb[1]);
    lumrange(rgbpi->lumstatrgb[2],rgbpi->lummax,lumscale,&rgbpi->lminrgb[2],&rgbpi->lmaxrgb[2]);

    do_hue(rgbpi);
    rgbpi->lmin=0;
    rgbpi->lmax=rgbpi->lummax/2;
    Set_Adjust(widget,Lab_LMIN,"%d",rgbpi->lmin);
    Set_Adjust(widget,Lab_LMAX,"%d",rgbpi->lmax);
    draw_pic(widget);
    rgbpi->redraw=TRUE;
  }
  if (!strcmp(data,PMENU_ALUM))
  {
    int lumscale=(rgbpi->lummax+1)/NLUMSTAT;
    if (lumscale==0) lumscale=1;

    lumrange(rgbpi->lumstat,rgbpi->lummax,lumscale,&rgbpi->lmin,&rgbpi->lmax);
    rgbpi->lmaxrgb[0]=rgbpi->lmaxrgb[1]=rgbpi->lmaxrgb[2]=0;
    rgbpi->lminrgb[0]=rgbpi->lminrgb[1]=rgbpi->lminrgb[2]=0;
    Set_Adjust(widget,Lab_LMIN,"%d",rgbpi->lmin);
    Set_Adjust(widget,Lab_LMAX,"%d",rgbpi->lmax);

    draw_pic(widget);
    /* Stop redrawing if it was aborted by this lum-setting.
       Otherwise redrawing is done with old lum-settings.
       See 'rgbpi->redraw=FALSE' at start of this func!!!
    */
    rgbpi->redraw=TRUE;
  }
     
  if (!strcmp(data,PMENU_LUMGRAPH))
    rgbmap(window,di,grp_sel->pc.picname,grp_sel->pc.is_color);

  if (!strcmp(data,PMENU_PIXSIZE))
  {
    rgbpi->do_shape=Get_Button(widget,PMENU_PIXSIZE);
  }
  
  if (!strcmp(data,PMENU_FAST))
  {
    if (Get_Button(widget,PMENU_FAST)) grp_sel->keep_in_mem=1; else grp_sel->keep_in_mem=0;
  }

  if (!strcmp(data,PMENU_SXY))
  {
    if (Get_Button(widget,PMENU_SXY))
      gtk_widget_show_all(Find_Widget(window,LAB_COORDXY)->parent);
    else
      gtk_widget_hide(Find_Widget(window,LAB_COORDXY)->parent);
  }

  if (!strcmp(data,Lab_LMIN))
  {
    rgbpi->lmin=Get_Adjust(widget,Lab_LMIN);
  }
  if (!strcmp(data,Lab_LMAX))
  {
    rgbpi->lmax=Get_Adjust(widget,Lab_LMAX);
  }
  if (!strcmp(data,LAB_GAMMA))
  {
    di->gamma=Get_Adjust(widget,LAB_GAMMA);
  }

  if (!strcmp(data,LAB_SHFT3D_PREV))
  {
    di->anagl.shift_3d=Get_Adjust(widget,LAB_SHFT3D_PREV);
  }
  if (!strcmp(data,LAB_LMIN3D_PREV))
  {
    di->anagl.lmin=Get_Adjust(widget,LAB_LMIN3D_PREV);
    if (di->anagl.lmin>=di->anagl.lmax) Set_Adjust(widget,LAB_LMAX3D_PREV,"%d",di->anagl.lmin+1);
  }
  if (!strcmp(data,LAB_LMAX3D_PREV))
  {
    di->anagl.lmax=Get_Adjust(widget,LAB_LMAX3D_PREV);
    if (di->anagl.lmax<=di->anagl.lmin) Set_Adjust(widget,LAB_LMIN3D_PREV,"%d",di->anagl.lmax-1);
  }
  if (!strcmp(data,LAB_INIT3D_PREV))
  {
    di->anagl.init=Get_Button(widget,LAB_INIT3D_PREV);
  }
  if (!strcmp(data,LAB_DCHAN_PREV))
  {
    di->anagl.depthchan=Get_Button(widget,LAB_DCHAN_PREV);
  }
  if (!strcmp(data,LAB_OL3D_PREV))
  {
    di->anagl.ol_3d=Get_Button(widget,LAB_OL3D_PREV);
  }
  if (!strcmp(data,LAB_OLADD_PREV))
  {
    di->ol_lum=Get_Adjust(widget,LAB_OLADD_PREV);
  }
  
  if (!strcmp(data,LAB_MOREPREV))
  {
    Show_Button(Find_Parent_Window(widget),LAB_MOREBUTPREV,Get_Button(widget,LAB_MOREPREV));
  }
  
  if (!strcmp(data,LAB_Redraw))
  {
    draw_pic(widget);
  }

#ifdef XXX
  if (!strcmp(data,LAB_ZI))
  {
    rgbpi->zx*=2;
    rgbpi->zy*=2;
    draw_pic(widget);
  }
  if (!strcmp(data,LAB_ZO))
  {
    rgbpi->zx/=2;
    rgbpi->zy/=2;
    draw_pic(widget);
  }

  if (!strcmp(data,LAB_ZF))
  {
    rgbpi->zx=1;
    rgbpi->zy=1;
    draw_pic(widget);
  }
#endif
}

void helpfunc(GtkWidget *wnd)
{
  GtkWidget *w;
  w=Create_Info(wnd,"Info",230,240,NULL,NULL);
  if (!w) return;
  Add_Text(w,0,"Key            Function\n");
  Add_Text(w,0,"-------------------------------------\n");
  Add_Text(w,0,"i                zoom in\n");
  Add_Text(w,0,"I                zoom in around cursor\n");
  Add_Text(w,0,"o               zoom out\n");
  Add_Text(w,0,"r                redraw\n");
  Add_Text(w,0,"f                full\n");
  Add_Text(w,0,"F               full x and y (distorted)\n");
  Add_Text(w,0,"<arrow>      pan\n");
  Add_Text(w,0,"<esc>        abort drawing\n");
  Add_Text(w,0,"\n");
  Add_Text(w,0,"mouseb. R  freeze position\n");
  
//  Create_Message("Info","Don't click!\nJust move cursor on button for help.");
}

int conv_pixel(GROUP *grp,int px,int py)
{
  int chnk,y_chnk,spos;
  SEGMENT *segm;
  CHANNEL *chan=grp->chan;
  if (px<0) return 0;
  if (py<0) return 0;
  if (px>chan->nc) return 0;
  chnk=py/chan->nl;                        // chunk in which req. y is
  y_chnk=py-(chnk*chan->nl);               // y within this chunk
  if (y_chnk<0) return 0;
  spos=y_chnk*chan->nc;                   // start-pos in chunck
  segm=Get_Segm(chan,chnk+chan->segm->xh.segment);             // segment holding y
  if ((segm) && (segm->chnk))
  {
    return segm->chnk[spos+px];
  }
  return 0;
}

#ifdef XXX
void test(int px,int py,float lon,float lat,PIC_CHARS *pc)
{
  int x,y;
  lonlat2fxy(lon,lat,&x,&y,pc);
  printf("%f,%f  %d,%d  %d,%d  %d,%d\n",lon,lat,px,py,x,y,px-x,py-y);
}
#endif

#define LAB_SCATINFO "Scatt"

#include "avhrr.h"
static void show_lonlat(GtkWidget *w,GROUP *grp,int px,int py,PIC_CHARS *pc)
{

  float lon,lat;
  int xoff=pc->xoff;
  int xfac=pc->xfac;
  int yoff=pc->yoff;
  int yfac=pc->yfac;
  if (!xfac)
  {
    xfac=pc->width/180;
    xoff=90;
  }
  if (!yfac)
  {
    yfac=pc->height/180;
    yoff=90;
  }
  dxy2lonlat_area(px,py,xoff,yoff,xfac,yfac,&lon,&lat,pc);
//test(px,py,lon,lat,pc);
  Set_Entry(w,LAB_COORDXY,"[%-4d,%-4d] = %d",px,py,conv_pixel(grp,px,py));
  if (pc->orbit.valid)
    Set_Entry(w,LAB_COORDLL,"%c %.2f, %c %.2f",
                  (lat>0? 'N' : 'S'),lat,(lon>0? 'E' : 'W'),lon);
  else
    Set_Entry(w,LAB_COORDLL,"[%-4d,%-4d]",px,py);

  {
    POINT pp;
    pp.lon=D2R(lon);
    pp.lat=D2R(lat);
    SCATPOINT *sp=nearest_scat(&pp);
    if (sp)
    {
      Set_Entry(w,LAB_SCATINFO,"V=%.2f m/s  D=%d",sp->speed,(int)sp->sdir);
    }
  }
}



static void show_pixval(GtkWidget *w,GROUP *grp,int px,int py,gboolean map_temp)
{
  float t;
  int cnr=grp->chan->chan_nr;
  int pix;

  dxy2fxy(px+grp->pc.o_xoffset,py+grp->pc.o_yoffset,&px,&py,&grp->pc);

  if (px<0) return;
  if (px>=grp->pc.width) return;

  pix=conv_pixel(grp,px,py);
  if ((grp->pro_epi) && (grp->pro_epi->pro) && 
       (grp->pro_epi->pro->chan->Cal_Slope[cnr-1]) &&
       (is_ir_group(grp)))
      
  {  // single IR channel
    t=pix2temp(grp->pro_epi->pro,grp->chan,pix);
    Set_Entry(w,LAB_PIXVAL,"%.1f C",t-273);
  }
  else if (grp->chan->cal.caltbl[1])
  {
    t=pix2temp(NULL,grp->chan,pix);
    Set_Entry(w,LAB_PIXVAL,"%.1f C",t-273);
  }
  else if (map_temp)
  {
    t=val2temp(pix);
    Set_Entry(w,LAB_PIXVAL,"%.1f C",t-273);
  }
  else
  {
    Set_Entry(w,LAB_PIXVAL,"%d",pix);
  }    
}

/* Handle extra keys. (standard already handled: i, o etc.) 
  (x,y): position mouse in drawable
  (px,py): position mouse in picture
*/
static void keyfunc(GtkWidget *widget,int data,int state,int x,int y,int px,int py)
{
  int opx;   // px always starting at east 90 degrees (needed for HRV)
  GtkWidget *window=Find_Parent_Window(widget);
  RGBPICINFO *rgbpi;
  GROUP *grp=gtk_object_get_data((gpointer)window,GRP_DATA);
  PIC_CHARS *pc=&grp->pc;
  DRAW_INFO *di=gtk_object_get_data((gpointer)window,"Draw_Info");
  if ((data==GDK_Left) ||(data==GDK_Right) ||(data==GDK_Up) ||(data==GDK_Down))
    gtk_widget_grab_focus(Find_Widget(window,LAB_DUMMY));      // to prevent change spin-values by arrows

  Show_Button(Find_Parent_Window(widget),LAB_BUT3D,globinfo.dim3);
  Show_Button(Find_Parent_Window(widget),LAB_OLADD_PREV,di->add_overlay | di->add_lonlat);

  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)widget,RGBPI_DL);
  if ((globinfo.scat_plot_all) || (globinfo.scatp))
    gtk_widget_show(Find_Widget(window,LAB_SCATINFO)->parent);

/* Adapt pixel-coord. to flipping */
  if ((grp->pc.scan_dir!='n'))
  {
    px=rgbpi->pwidth-px;
    py=rgbpi->pheight-py;
  }

/* Adapt pixel-coord. to shift (for HRV: depending on py) */
  opx=px-pc->gchar.lower_x_shift;
opx=px;
//printf("%d  %d  %d\n",pc->gchar.lower_x_shift,pc->gchar.upper_x_shift,px);
//opx=px-MIN(pc->gchar.lower_x_shift,pc->gchar.upper_x_shift);
  if (py>pc->gchar.shift_ypos)
    px-=pc->gchar.upper_x_shift;
  else
    px-=pc->gchar.lower_x_shift;

  if (state&GDK_BUTTON3_MASK)
  {
    GtkWidget *w1=Find_Widget(widget,LAB_COORDLL);
    GtkWidget *w2=Find_Widget(widget,LAB_PIXVAL);
    di->freeze_pos=!di->freeze_pos;
    if (di->freeze_pos)
    {
      Set_Widgetcolor(w1,'f',0xff,0x00,0x00);
      Set_Widgetcolor(w2,'f',0xff,0x00,0x00);
    }
    else
    {
      Set_Widgetcolor(w1,'f',0x00,0x00,0x00);
      Set_Widgetcolor(w2,'f',0x00,0x00,0x00);
    }
  }

  switch (data)
  {
    case 'a':
      Set_Adjust(widget,Lab_LMIN,"%d",rgbpi->lmin);
      Set_Adjust(widget,Lab_LMAX,"%d",rgbpi->lmax);
    break;
    case 'r': case 'R':
      if (grp->rgbpicstr[0]) free(grp->rgbpicstr[0]); grp->rgbpicstr[0]=NULL;
      if (grp->rgbpicstr[1]) free(grp->rgbpicstr[1]); grp->rgbpicstr[1]=NULL;
      if (grp->rgbpicstr[2]) free(grp->rgbpicstr[2]); grp->rgbpicstr[2]=NULL;
      if (grp->line_in_mem) free(grp->line_in_mem);   grp->line_in_mem=NULL;
    break;
    case 'i': case 'o': case 'f': case 'F':
    /* only set zx/zy spin if visible. 
       NOTE: Set_Adjust may cause rounding of zx/zy because of limited 
       range set in Create_Spin.
    */
      if (Get_Button(widget,PMENU_ZOOMV))
      { 
        Set_Adjust(widget,LAB_ZOOMX,"%f",rgbpi->zx);
        Set_Adjust(widget,LAB_ZOOMY,"%f",rgbpi->zy);
      }
      adapt_zoomvals(widget,rgbpi);
    break;
    default:
      adapt_zoomvals(widget,rgbpi);
    break;
  }

  if (!di->freeze_pos)
  {
    show_lonlat(widget,grp,opx,py,&grp->pc);
    show_pixval(widget,grp,opx,py,di->map_temp);
  }
}

#define TBAR_W 10                    // width tempbar
#define TBAR_W_TOT TBAR_W+40         // width drawable for tempbar
#define TBAR_H_TOT 300               // height drawable for tempbar 
#define TBAR_H 280                   // height tempbar
#define TBAR_S (TBAR_H_TOT-TBAR_H)/2 // startpos bar
#define TMIN -40                     // min temp
#define TMAX +60                     // max temp

#define FG 255
#define BG 0
void temp_legend(RGBI *rgbi)
{
  GdkColor clr;
  int x,y;
  int tp=-100;

/* Bar */
  for (y=0; y<TBAR_H; y++)
  {
    int t=((TBAR_H-1-y)*(TMAX-TMIN))/(TBAR_H-1)+TMIN;
    guint16 pix[3]={0,0,0};
    temp_clrmap((float)(t+273.),pix);

    for (x=0; x<TBAR_W; x++)
    {
      clr.red  =MIN(pix[0],0xff);
      clr.green=MIN(pix[1],0xff);
      clr.blue =MIN(pix[2],0xff);
      draw_rgbpoint(rgbi,&clr,x,y+TBAR_S);
    }

    // Bottom part: markers per 10 degrees
    for (x=0; x<10; x++)
    {
      if ((!(t%10)) && (tp!=t))
      {
        clr.red  =0x0;
        clr.green=0x0;
        clr.blue =0x0;
        if (t==0) clr.red  =0xff;
        draw_rgbpoint(rgbi,&clr,x+TBAR_W,y+TBAR_S);
      }
    }
    tp=t;
  }

/* numbers */
  tp=-100;
  clr.red  =0x0;
  clr.green=0x0;
  clr.blue =0x0;
  for (y=0; y<TBAR_H; y++)
  {
    int t=((TBAR_H-1-y)*(TMAX-TMIN))/(TBAR_H-1)+TMIN;
    char tmp[10];
    sprintf(tmp,"%d",(int)t);
    if ((!(t%10)) && (tp!=t))
    {
      draw_rgbstring(rgbi,&clr,TBAR_W+4,y+2,tmp);
    }
    tp=t;
  }
}

static void clrblok(RGBI *rgbi,int r,int g,int b,int oy)
{
  GdkColor clr;
  int x,y;
  clr.red=r;
  clr.green=g;
  clr.blue=b;
  for (y=0; y<20; y++)
  {
    for (x=0; x<20; x++)
    {
      draw_rgbpoint(rgbi,&clr,x+5,y+oy);
    }
  }
}

static void airm_legend(RGBI *rgbi)
{
  GdkColor clr;

  clr.red=clr.green=clr.blue =FG;
  draw_rgbstring(rgbi,&clr,2,10,"Jet");
  draw_rgbstring(rgbi,&clr,2,20,"streak");
  clrblok(rgbi,131,8,12,30);

  clr.red=clr.green=clr.blue =FG;
  draw_rgbstring(rgbi,&clr,2,80,"warm");
  draw_rgbstring(rgbi,&clr,2,90,"airmass");
  clrblok(rgbi,27,114,32,100);

  clr.red=clr.green=clr.blue =FG;
  draw_rgbstring(rgbi,&clr,2,150,"cold");
  draw_rgbstring(rgbi,&clr,2,160,"airmass");
  clrblok(rgbi,58,8,126,170);
}

static void dust_legend(RGBI *rgbi)
{
  GdkColor clr;
  clr.red=clr.green=clr.blue =FG;
  draw_rgbstring(rgbi,&clr,2,10,"Dust");

  clrblok(rgbi,255,96,202,20);
}

static void nfog_legend(RGBI *rgbi)
{
  GdkColor clr;
  clr.red=clr.green=clr.blue=FG;
  draw_rgbstring(rgbi,&clr,2,10,"Fog");
  clrblok(rgbi,204,252,176,20);

  clr.red=clr.green=clr.blue =FG;
  draw_rgbstring(rgbi,&clr,2,70,"thick");
  draw_rgbstring(rgbi,&clr,2,80,"clouds");
  clrblok(rgbi,170,0,0,90);

  clr.red=170; clr.green=218; clr.blue =0;
  draw_rgbpoint(rgbi,&clr,14,10+90);
  draw_rgbpoint(rgbi,&clr,15,10+90);
  draw_rgbpoint(rgbi,&clr,15,11+90);
  draw_rgbpoint(rgbi,&clr,15,12+90);

  clr.red=clr.green=clr.blue =FG;
  draw_rgbstring(rgbi,&clr,2,140,"thin");
  draw_rgbstring(rgbi,&clr,2,150,"clouds");
  clrblok(rgbi,0,0,69,160);
}

static void dfog_legend(RGBI *rgbi)
{
}

static COMPOSE_TYPE compose_type;
static void draw_tempmap(GtkWidget         *widget,
                         GdkEventConfigure *event)
{
  RGBI *rgbi;
  GdkColor clr;
  int x,y;
  int bg=BG,fg=FG;
  if (compose_type==0)
  {
    bg=255;
    fg=0;
  }
  
  Renew_RGBBuf(widget);
  rgbi=Get_RGBI(widget);
  if (!rgbi) return;
  if (rgbi->width<=1) return;

/* Background */
  for (y=0; y<rgbi->height; y++)
  {
    for (x=0; x<rgbi->width; x++)
    {
      if (((y==0) || (y==rgbi->height-1)) ||
          ((x==0) || (x==rgbi->width-1)))
        clr.red  =clr.green=clr.blue =fg;
      else
        clr.red  =clr.green=clr.blue =bg;
      draw_rgbpoint(rgbi,&clr,x,y);
    }
  }

  switch (compose_type)
  {
    case map_airm:
      airm_legend(rgbi);
    break;
    case map_dust:
      dust_legend(rgbi);
    break;
    case map_nfog:
      nfog_legend(rgbi);
    case map_dfog:
      dfog_legend(rgbi);
    break;
    default:
      temp_legend(rgbi);
    break;
  }
  return;
}

static gboolean temp_avail(GROUP *grp)
{
  if (!(is_ir_group(grp))) return FALSE;
  return TRUE;
//  return ((grp->pro_epi) && (grp->pro_epi->pro));
}

static void zoomfunc(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  GtkWidget *window=Find_Parent_Window(widget);
  DRAW_INFO *di=gtk_object_get_data((gpointer)window,"Draw_Info");
  RGBPICINFO *rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)di->drawable,RGBPI_DL);
  if (!strcmp(name,LAB_ZOOMX))
    rgbpi->zx=GTK_ADJUSTMENT(widget)->value;
  if (!strcmp(name,LAB_ZOOMY))
    rgbpi->zy=GTK_ADJUSTMENT(widget)->value;
}
#define LAB_RSPACE "!PREV_RSPACE"
/*******************************************************
 * Create window and generate preview-picture
 *******************************************************/
int preview_pic(GtkWidget *window,GROUP *grp,GENMODES *modes)
{
  GtkWidget *wnd,*canvas,*w[10],*menu,*menubut,*wana[8],*wzoom;
  GtkWidget *lmin,*lmax,*ollum,*fix_clr,*tmpbar,*wgam,*rspace;
  GtkWidget *dummy[4];
  char *windowname=PREV_WNDNAME;
  PIC_CHARS *pc=&grp->pc;
  int lum_max=(1<<pc->depth)-1;
  int lum_maxbar=lum_max;
  int lum_incr=(lum_max+1)/128;
  int pixvalmax=(0x100<<pc->bitshift)-1;
  int temperature_mapping=((globinfo.chanmap) && (grp->compose) && (globinfo.chanmap->temperature));
  DRAW_INFO *di;
  RGBPICINFO *rgbpi;
if (lum_maxbar==255) lum_maxbar=511;
  if (pc->image_iformat=='t')
  {
    char *cmd;
    modes->image_oformat=pc->image_iformat;
    if ((cmd=launch_viewer(&prefer,pc,grp->chan->segm->pfn,modes)))
    {
      Set_Entry(window,LAB_INFO,"Command %s failed.",cmd); /* failed */
      Create_Message("Error","Command %s failed.",cmd);
    }
    return 0;
  }

  if (!modes->do_one_pic) windowname=pc->picname;
//  pic_info(grp,window,modes); // weg 10-08-2008: al gedaan in gen_common.c, zie "Get drawing figures (-> grp_sel->pc)"

  if (!grp->pc.height)
  {
    Create_Message("Error","No height of picture found. No channels selected?");
//    color_map(window,grp);  // no height -> probably no channel mapping
    return 0;
  }
  genmode=*modes;
  if (pc->partly) genmode.area_nr=0;
  di=calloc(1,sizeof(*di));
  di->is_preview=TRUE;
  di->drawing=di->stop_drawing=FALSE;
  di->invert=globinfo.spm.invert;
  di->add_overlay=globinfo.add_overlay;
  di->add_marks=globinfo.add_marks;
  di->add_cities=globinfo.add_marks;
  di->add_lonlat=globinfo.add_lonlat;
  di->fire_detect=globinfo.spm.fire_detect;
  di->avhrr_lin=modes->avhrr_lin;
  di->gamma=1.;

  if (temp_avail(grp))
  {
    di->map_temp=globinfo.spm.map_temp;
    di->map_temp_G_mA=globinfo.spm.map_temp_G_mA;
  }
  di->use_lut=globinfo.lut.use;
  di->gmap=modes->gmap;
  di->xoff=pc->xoff;
  di->xfac=pc->xfac;
  di->yoff=pc->yoff;
  di->yfac=pc->yfac;

  while (is_drawing) while (g_main_iteration(FALSE));

  wnd=Create_Window(window,prefer.pr_wndwidth,prefer.pr_wndheight,windowname,clean_func);
  if (!wnd) return 0; /* window already exist, and is now popped-up */

  menu=Create_Menu(wnd,PMENU_FILE           ,NULL          ,BUTTON,
                         PMENU_SAVEPREV_JPG ,saveprev_func ,BUTTON,
                         PMENU_SAVEPREV_PGM ,saveprev_func ,BUTTON,
                         PMENU_SAVEPREV_PGM8,saveprev_func ,BUTTON,
                         PMENU_SCREENDUMP,   saveprev_func ,BUTTON,
                         PMENU_SEP          ,NULL          ,BUTTON, 
                         PMENU_SEP          ,NULL          ,BUTTON, 
                         PMENU_Dismiss      ,Close_Window  ,BUTTON, 
                       PMENU_VIEW           ,NULL          ,BUTTON, 
                         PMENU_LUMGRAPH     ,saveprev_func ,BUTTON,  
                         PMENU_ZOOM         ,NULL          ,BUTTON, 
                           PMENU_ZOOMD      ,saveprev_func ,BUTTON, 
                           PMENU_ZOOMV      ,saveprev_func ,CHECK, 
                           PMENU_ZOOMF      ,saveprev_func ,BUTTON, 
                           PMENU_ZOOMI      ,saveprev_func ,BUTTON, 
                           PMENU_ZOOMO      ,saveprev_func ,BUTTON, 
                         PMENU_REDRAW       ,saveprev_func ,BUTTON, 
                         PMENU_FLIP         ,saveprev_func ,CHECK,  
                         PMENU_INVERT       ,saveprev_func ,CHECK,  
                         PMENU_AVLIN        ,saveprev_func ,CHECK,  
                         PMENU_GMAP         ,NULL          ,BUTTON,  
                           PMENU_GMAPN      ,saveprev_func ,RADIO,  
                           PMENU_GMAPC      ,saveprev_func ,RADIO,  
                           PMENU_GMAPM      ,saveprev_func ,RADIO,  
                           PMENU_GMAPPN     ,saveprev_func ,RADIO,  
                           PMENU_GMAPPS     ,saveprev_func ,RADIO,  
                         PMENU_OVERLAY      ,saveprev_func ,CHECK,  
                         PMENU_MARK         ,saveprev_func ,CHECK,  
                         PMENU_CITY         ,saveprev_func ,CHECK,  
                         PMENU_LONLAT       ,saveprev_func ,CHECK,  
                         PMENU_FIRE         ,saveprev_func ,CHECK,  
                         PMENU_TEMP         ,saveprev_func ,CHECK,  
                         PMENU_TEMPG        ,saveprev_func ,CHECK,  
                         PMENU_LUT          ,saveprev_func ,CHECK,  
                         PMENU_BUFR         ,saveprev_func ,BUTTON,  
                         PMENU_SEP          ,NULL          ,BUTTON, 
                         PMENU_PIXSIZE      ,saveprev_func ,CHECK,  
                         PMENU_FAST         ,saveprev_func ,CHECK,  
                         PMENU_SXY          ,saveprev_func ,CHECK,  
                         PMENU_SEP          ,NULL          ,BUTTON, 
                         PMENU_HELP         ,helpfunc      ,BUTTON, 
                       NULL);

  menubut=Create_Toolbar(saveprev_func, PMENU_ALUM,"Optimize luminance mapping",
                                        PMENU_ALUMRGB,"Optimize hue",
                                        NULL);
// Invisible copies of alum and rgb, to access via lumrgb window 
// (how to send signal to toolbar button?)
  dummy[2]=Create_Button(PMENU_ALUM,saveprev_func);
  dummy[3]=Create_Button(PMENU_ALUMRGB,saveprev_func);
  menubut=Pack(NULL,'h',menubut,1,dummy[2],1,dummy[3],1,NULL);

// wnd was hier gedef., verplaatst naar boven. Zie Create_Menu(wnd,.. !!!
//  wnd=Create_Window(window,prefer.pr_wndwidth,prefer.pr_wndheight,windowname,clean_func);
//  if (!wnd) return 0; /* window already exist, and is now popped-up */

  canvas=Create_Canvas_RGBPic_dr2(wnd,pc->o_width,pc->o_height,pc->depth,draw_pic,keyfunc);
  di->drawable=gtk_object_get_data(GTK_OBJECT(canvas),CHILD_ID);
  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)di->drawable,RGBPI_DL);
  rgbpi->lmaxrgb[0]=rgbpi->lmaxrgb[1]=rgbpi->lmaxrgb[2]=0;
  lmin=Create_Scale(Lab_LMIN,'h',saveprev_func,"%d%d%d%d",lum_incr,0,0,lum_maxbar);
  lmax=Create_Scale(Lab_LMAX,'h',saveprev_func,"%d%d%d%d",lum_incr,lum_max,0,(temperature_mapping? MAX(512,lum_maxbar) : lum_maxbar));

  ollum=Create_Spin(LAB_OLADD_PREV,saveprev_func,"%2d%2d%2d%2d",0x8,di->ol_lum,0,0xff);
  fix_clr=Create_Check(LAB_FCLR_PREV,saveprev_func,FALSE);
  di->anagl.shift_3d=DEF_3DSHIFT;
  di->anagl.lmin=0;
  di->anagl.lmax=pixvalmax;
  wana[1]=Create_Spin(LAB_SHFT3D_PREV,saveprev_func,"%2d%2d%2d",di->anagl.shift_3d,0,MAX_3DSHIFT);
  wana[2]=Create_Scale(LAB_LMIN3D_PREV,'h',saveprev_func,"%d%3d%3d%3d",10,di->anagl.lmin,0,1023);
  wana[3]=Create_Scale(LAB_LMAX3D_PREV,'h',saveprev_func,"%d%3d%3d%3d",10,di->anagl.lmax,0,1023);
  wana[4]=Create_Check(LAB_INIT3D_PREV,saveprev_func,FALSE);
  wana[5]=Create_Check(LAB_DCHAN_PREV,saveprev_func,FALSE);
  wana[6]=Create_Check(LAB_OL3D_PREV,saveprev_func,FALSE);

  wana[6]=Pack("",'h',wana[6],1,NULL);
  wana[0]=Pack(LAB_BUT3D,'h',wana[1],1,wana[2],1,wana[3],1,wana[4],1,wana[5],1,wana[6],1,NULL);
  Set_Widgetcolor(wana[0],'b',0,0,0xff);
  Set_Widgetcolor(wana[6],'b',0xff,0,0xff);

/* To prevent scale buttons changing while arrow buttons are used (to scroll through pic)
   the focus has to be removed from these scale buttons. Don't know how to do this,
   so used the ugly method by creating an invisible dummy entry button to focus on.
*/
  dummy[0]=Create_Entry(LAB_DUMMY,NULL,""); 
  dummy[1]=Create_Button(LAB_DUMMY2,saveprev_func); 
  w[3]=Create_Check(LAB_MOREPREV,saveprev_func,FALSE);

  w[4]=Create_Entry(LAB_COORDLL,NULL,"%10s","");
  w[5]=Create_Entry(LAB_PIXVAL,NULL,"%5s","");
  w[6]=Create_Entry(LAB_COORDXY,NULL,"%10s","");
  w[7]=Create_Entry(LAB_SCATINFO,NULL,"%10s","");


  wzoom=Create_ButtonArray(LAB_ZOOMVALS,zoomfunc,3,
               SPIN,LAB_ZOOMX,"%.2f%.2f%.2f",rgbpi->zx,0.01,100.,
               SPIN,LAB_ZOOMY,"%.2f%.2f%.2f",rgbpi->zy,0.01,100.,
               0);

  tmpbar=Create_Drawable(TBAR_W_TOT,TBAR_H_TOT,draw_tempmap,NULL,NULL);
// tmpbar maybe inside GtkBox, so find actual drawable. Needed for closing.
  di->drawable_temp=Find_Widget1(tmpbar,"GTK_DRAWING_AREA");

  tmpbar=Pack(LAB_TCLR1,'v',tmpbar,1,NULL);
  Set_Widgetcolor(tmpbar,'b',0,0,0);
  tmpbar=Pack(LAB_TCLR,'v',tmpbar,1,NULL);
  canvas=SPack(NULL,"h",tmpbar,"1",canvas,"ef1",NULL);
//canvas=NULL;

  wgam=Create_Spin(LAB_GAMMA,saveprev_func,"%2.1f%2.1f%2.1f",di->gamma,0.3,3.0);

  
  w[9]=Create_Led(LAB_BUSY,0x000);
  w[1]=SPack(NULL,"h",menu,"1",menubut,"1",lmin,"ef1",lmax,"ef2",fix_clr,"1",wzoom,"1",w[3],"1",w[9],"E1",dummy[0],"1",dummy[1],"1",NULL);
  w[2]=Pack(LAB_MOREBUTPREV,'h',ollum,4,w[4],1,w[5],1,w[6],1,w[7],1,wgam,1,NULL); // tmpbar,1,NULL);
  w[0]=SPack(NULL,"v",w[1],"1",w[2],"1",wana[0],"1",canvas,"ef1",NULL);
  rspace=NULL; // SPack(LAB_RSPACE,"h",NULL);
  w[0]=SPack(NULL,"h",w[0],"ef1",rspace,"1",NULL);

//  w[0]=Pack(NULL,'h',tmpbar,1,w[0],1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show_all(wnd);
  gtk_widget_hide(wana[0]);
  gtk_widget_hide(ollum);
  gtk_widget_hide(w[2]);
  gtk_widget_hide(Find_Widget(wnd,LAB_TCLR));
  gtk_widget_hide(Find_Widget(wnd,LAB_COORDXY)->parent);
  gtk_widget_hide(Find_Widget(wnd,LAB_SCATINFO)->parent);

  gtk_widget_hide(Find_Widget(wnd,LAB_ZOOMVALS)->parent);

  gtk_widget_hide(dummy[0]);
  gtk_widget_hide(dummy[1]);
  gtk_widget_hide(dummy[2]);
  gtk_widget_hide(dummy[3]);
  if (!pc->is_color) gtk_widget_hide(fix_clr);

  if (!IS_POLAR(grp)) Show_Button(wnd,PMENU_AVLIN,FALSE);
  if (!IS_GEO(grp))
  {
    Sense_Button(wnd,PMENU_GMAP,FALSE);
    if (pc->gmap!=normal)
    {
      Show_Button(wnd,PMENU_AVLIN,FALSE);
    }
  }

  Set_Button(wnd,PMENU_PIXSIZE,TRUE);
  gtk_object_set_data((gpointer)wnd,GRP_DATA,(gpointer)grp);
  gtk_object_set_data((gpointer)wnd,"Draw_Info",(gpointer)di);

  is_drawing=TRUE;        /* NOTE: draw_pic MUST make is_drawing FALSE! */

/* next is only for error message in case overlay not found. */
  if ((di->add_overlay) && (pc->orbittype!=POLAR) && (prefer.prf_ovltype=='P'))
  {
    get_overlay(prefer.overlay,globinfo.overlaytype,grp->chan,(void *)Create_Message);
  }
  if (pc->depth>8) Set_Adjust(wnd,Lab_LMAX,"%d",1023>>(2-globinfo.bitshift));

  if ((temperature_mapping) || (di->map_temp))
  {
    Set_Adjust(wnd,Lab_LMAX,"%d",255);
  }
   
  Set_Adjust(wnd,LAB_OLADD_PREV,"%d",0xff);
  Set_Button(wnd,PMENU_TEMP,di->map_temp);
  Set_Button(wnd,PMENU_TEMPG,di->map_temp_G_mA);
  Set_Button(wnd,PMENU_LUT,di->use_lut);

  switch(di->gmap)
  {
    case normal      : Set_Button(wnd,PMENU_GMAPN,TRUE);  break;
    case plate_carree: Set_Button(wnd,PMENU_GMAPC,TRUE);  break;
    case mercator    : Set_Button(wnd,PMENU_GMAPM,TRUE);  break;
    case polar_n     : Set_Button(wnd,PMENU_GMAPPN,TRUE); break;
    case polar_s     : Set_Button(wnd,PMENU_GMAPPS,TRUE); break;
    default          : Set_Button(wnd,PMENU_GMAPN,TRUE) ; break;
  }

  Set_Button(wnd,PMENU_INVERT,di->invert);
  Set_Button(wnd,PMENU_OVERLAY,di->add_overlay); 
  Set_Button(wnd,PMENU_MARK,di->add_marks); 
  Set_Button(wnd,PMENU_CITY,di->add_cities); 
  Set_Button(wnd,PMENU_LONLAT,di->add_lonlat); 
  if (di->fire_detect)
    Set_Button(wnd,PMENU_FIRE,di->fire_detect); 
  else
    Show_Button(wnd,PMENU_FIRE,FALSE);
    
//    gtk_widget_show(Find_Widget(wnd,LAB_GAMMA)->parent);
  if ((di->map_temp) && (!di->map_temp_G_mA))
  {
    gtk_widget_show(Find_Widget(wnd,LAB_TCLR));
  }

  if (grp->compose)
  {
    compose_type=globinfo.spm.compose_type;
    if ((globinfo.spm.compose_type==map_airm) ||
        (globinfo.spm.compose_type==map_dust) ||
        (globinfo.spm.compose_type==map_nfog))
    {
      gtk_widget_show(Find_Widget(wnd,LAB_TCLR));
      gtk_widget_hide(Find_Widget(wnd,LAB_GAMMA)->parent);
    }
  }
  else
  {
    compose_type=0;
  }
  
  if (pc->orbittype==POLAR)
  {
    Set_Button(wnd,PMENU_AVLIN,di->avhrr_lin);
  }


  if ((!temp_avail(grp)) || (di->use_lut))
  {
    Sense_Button(wnd,PMENU_TEMP,FALSE);
    Sense_Button(wnd,PMENU_TEMPG,FALSE);
  }
  if (rgbpi->do_shape)
  {
    RGBI *rgbi=Get_RGBI(di->drawable);
    makeit_square(rgbi->width,rgbi->height,rgbpi);
  }

// Next commands manage that pic is drawn 1 time. 
// Drawing (updating) is still disabled here.
  while (g_main_iteration(FALSE));  // Flush running processes activated 
                                    // by some of Set_Button commands.
  Set_Enable_Update(di->drawable,TRUE);      // Enable drawing pic.
  Set_Button(wnd,LAB_DUMMY2);       // Activate to get first drawing.                                    // (invisible dummy button 'misused' to transmit a signal)

  Set_Scrollbars(wnd,rgbpi);

  while (g_main_iteration(FALSE));
  // if di->stop_drawing: window destroyed! Stop ALL actions!
  return di->stop_drawing;
}




