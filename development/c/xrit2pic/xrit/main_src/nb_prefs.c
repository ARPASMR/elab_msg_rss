/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#define MPARTS
#include "xrit2pic.h"
#include "xrit_prefs.h"
#include "gtk/gtk.h"
#include "gdk/gdkkeysyms.h"

#define LAB_SAVE "Save it."
#define LAB_GSAVE "Save gui state."
#define LAB_LDDEF "Load defaults"
#define PACK_MENU "!Packmenu"
#define LAB_PREFFILE  "!Preferences file"
#define LAB_GPREFFILE "!Gui state file  "
#define LAB_EXTMGEN "Extern"

#define LAB_NAMEOVL "Land/Coast"
#define LAB_OVLRED "!OVLR"
#define LAB_OVLGRN "!OVLG"
#define LAB_OVLBLE "!OVLB"
#define LAB_NAMEOVLCLR "!landclr"
#define LAB_PREFOVL1 "Picture"
#define LAB_PREFOVL2 "Vector"
#define LAB_PREFMARK "Show Mark"
#define LAB_PREFCITIES "Show cities"
#define LAB_SIZECMARK "Size mark"

#define LAB_NAMELOLA "lon/lat"
#define LAB_LOLARED "!LOLAR"
#define LAB_LOLAGRN "!LOLAG"
#define LAB_LOLABLE "!LOLAB"
#define LAB_NAMELOLACLR "!lonlatclr"

#define LAB_NAMEBFR "Scattero"
#define LAB_BFRRED "!BFRR"
#define LAB_BFRGRN "!BFRG"
#define LAB_BFRBLE "!BFRB"
#define LAB_NAMEBFRCLR "!Scatteroclr"

#define LAB_NAMESBFR "Scattero selected"
#define LAB_SBFRRED "!SBFR"
#define LAB_SBFGRN "!SBFG"
#define LAB_SBFBLE "!SBFB"
#define LAB_NAMESBFRCLR "!Scatteroselclr"

#define LAB_AVHRRMODE "AVHRR mode"

#define LAB_FFND_LOC "Found in:"
#define LAB_FSRCH "Search..."

#define LAB_NRPLACES "^Nr. found"


#define LAB_NORF_GET "Download:"


#define LAB_DELDAYMIN "!deldaymin"
#define LAB_DELHOURMIN "!delhourmin"
#define LAB_DELDAYMAX "but NOT older than"
#define LAB_DROUND "Round to days"
#define LAB_HOURMIN "!hour_min"
#define LAB_RECATSTART "Tree updating"
#define LAB_DELOLDATSTART "Delete old"

#define LAB_FOV "Overwrite:"
#define LAB_FASK "Ask"
#define LAB_FOVER "Yes"
#define LAB_FNOVER "No  "

#define LAB_FULLGLOBE "Full globe"
#define LAB_INV_IR "Invert IR"

#define LAB_PARTW " width"
#define LAB_OFFW "offset_x"
#define LAB_PARTH "height"
#define LAB_OFFH "offset_y"
#define LAB_PARTDEF "Default fixed boundaries"
#define LAB_PARTN "Name"

#define LAB_CLON "center lon"
#define LAB_DLON "delta lon"
#define LAB_CLAT "center lat"
#define LAB_DLAT "delta lat"
#define LAB_PLDIR "Polar dir."

#define LAB_PNAMX "!pnam"
#define LAB_CLONX "!clon"
#define LAB_CLATX "!clat"
#define LAB_DLONX "!dlon"
#define LAB_DLATX "!dlat"
#define LAB_PLDRX "!poldir"

#define LAB_PNAM1 "!pnam1"
#define LAB_CLON1 "!clon1"
#define LAB_CLAT1 "!clat1"
#define LAB_DLON1 "!dlon1"
#define LAB_DLAT1 "!dlat1"
#define LAB_PLDR1 "!poldir1"

#define LAB_PNAM2 "!pnam2"
#define LAB_CLON2 "!clon2"
#define LAB_CLAT2 "!clat2"
#define LAB_DLON2 "!dlon2"
#define LAB_DLAT2 "!dlat2"
#define LAB_PLDR2 "!poldir2"


#define LAB_PNAM3 "!pnam3"
#define LAB_CLON3 "!clon3"
#define LAB_CLAT3 "!clat3"
#define LAB_DLON3 "!dlon3"
#define LAB_DLAT3 "!dlat3"
#define LAB_PLDR3 "!poldir3"


#define LAB_PNAM4 "!pnam4"
#define LAB_CLON4 "!clon4"
#define LAB_CLAT4 "!clat4"
#define LAB_DLON4 "!dlon4"
#define LAB_DLAT4 "!dlat4"
#define LAB_PLDR4 "!poldir4"

#define LAB_PREFMAPINFO "!pref_mapinfo"
#define SAVE_HMSGMAP    "MSG HRIT"
#define DEF_HMSGMAP     "MSG HRIT "
#define SAVE_LMSGMAP    "MSG LRIT"
#define DEF_LMSGMAP     "MSG LRIT "
#define SAVE_NOAA       "AVHRR NOAA"
#define DEF_NOAA        "AVHRR NOAA "
#define SAVE_METOP      "EPS METOP/NOAA"
#define DEF_METOP       "EPS METOP/NOAA "

#define LAB_MARKLON "!Lon"
#define LAB_MARKLAT "!Lat"
#define LAB_FORMAT "Use ddmm.mm"
#define LAB_EFORMAT "!formatx"
#define TXT_GFRMT "(d)d.dddd"
#define TXT_MFRMT "(d)dmm.mm"

extern PREFER prefer;
extern GLOBINFO globinfo;
extern GROUP *globgrp;

extern FMAP fmap_vis_hmsg;
extern FMAP fmap_vis_lmsg;
extern FMAP fmap_vis_noaa;
extern FMAP fmap_vis_metop;

static char *gseldir;
static char gdestdir[1000];

static char glutdir[1000];

#define LIMIT_NFILES 100
#define LAB_PREFWIN "Preferences"

/*******************************************************
 * Catch selected destination directory from file manager
 *******************************************************/
static void file_ok_destsel(GtkWidget *widget, GtkFileSelection *fs)
{
  GtkWidget *file_window=Find_Parent_Window(widget);
  GtkWidget *pref_window=Find_Window(file_window,LAB_PREFWIN);
  if (file_selection_get_dirname())
  {
    Set_Entry(pref_window,LAB_DDIR,gdestdir);
    strcpy(prefer.dest_dir,gdestdir);
    finish_path(prefer.dest_dir);
  }
  Close_Fileselectf(file_window);
}

/*******************************************************
 * Catch selected source directory from file manager
 *******************************************************/
static void file_ok_srcsel(GtkWidget *widget, GtkFileSelection *fs)
{
  GtkWidget *file_window=Find_Parent_Window(widget);
  GtkWidget *pref_window=Find_Window(file_window,LAB_PREFWIN);
  if (file_selection_get_dirname())
  {
    Set_Entry(pref_window,LAB_SDIR,gseldir);
  }
  Close_Fileselectf(file_window);
}

/*******************************************************
 * Catch selected lut directory from file manager
 *******************************************************/
static void file_ok_lutsel(GtkWidget *widget, GtkFileSelection *fs)
{
  GtkWidget *file_window=Find_Parent_Window(widget);
  GtkWidget *pref_window=Find_Window(file_window,LAB_PREFWIN);
  if (file_selection_get_dirname())
  {
    Set_Entry(pref_window,LAB_LDIR,glutdir);
  }
  Close_Fileselectf(file_window);
}

static void clrwnd2fmap(GtkWidget *wnd,FMAP *m)
{
  int i;
  CHANMAP *cm;
  int n=0;
  get_mapping(wnd,LAB_COLOR);
  for (i=0; i<3; i++) m->offset[i]=globinfo.offset[i];
  for (i=0; i<3; i++) m->gamma[i]=globinfo.gamma[i];

  for (cm=globinfo.chanmap; cm; cm=cm->next)
  {
    if ((cm->r) || (cm->g) || (cm->b))
    {
      if (n>=8) break;
      strcpy(m->fm[n].chan,cm->chan_name);
      m->fm[n].r=cm->r;
      m->fm[n].g=cm->g;
      m->fm[n].b=cm->b;
      n++;
      m->n=n;
    }
  }
}

#define LAB_EMAP1 "Browse 1"
#define LAB_EMAP2 "Browse 2"
#define LAB_LUTF "LUT Filename:"
/*******************************************************
 * Catch selected directory from file manager
 *******************************************************/
static void file_ok_sel1(GtkWidget *widget, GtkFileSelection *fs)
{
  char *fn;
  GtkWidget *file_window=Find_Parent_Window(widget);
  GtkWidget *pref_window=Find_Window(file_window,LAB_PREFWIN);
  if ((fn=file_selection_get_filename()))
  {
    strcpy(prefer.earthmapfile1,fn);
    Set_Entry(pref_window,LAB_EARTHMAP1,prefer.earthmapfile1);
  }
  Close_Fileselectf(file_window);
}
static void file_ok_sel2(GtkWidget *widget, GtkFileSelection *fs)
{
  char *fn;
  GtkWidget *file_window=Find_Parent_Window(widget);
  GtkWidget *pref_window=Find_Window(file_window,LAB_PREFWIN);
  if ((fn=file_selection_get_filename()))
  {
    strcpy(prefer.earthmapfile2,fn);
    Set_Entry(pref_window,LAB_EARTHMAP2,prefer.earthmapfile2);
  }
  Close_Fileselectf(file_window);
}

// 

// convmode: 'g'=graden float:                gg.gggg
//           'm'=graden int + minuten float : ggmm.mm
static float conv_coord(float l,int convmode)
{
  int ig;
  float m;
  if (convmode=='m')  // g->m
  {
    ig=(int)l;             // graden
    m=(l-ig)*60.;          // min.
    l=ig*100. + m; // kunstmatig gg.mm 
  }

  if (convmode=='g')
  {
    ig=(int)l/100;
    m=((float)(l-ig*100)*10./6.);
    l=ig+((float)m/100.);
  }
  return  l;

}


/*******************************************************
 * Handle preferences buttons etc.
 *******************************************************/
void do_prefbut(GtkWidget *iwidget,gpointer data)
{
  GtkWidget *wnd;
  GtkWidget *widget=NULL;
  char *name=(char *)data;
  int n,i;
  char *opt;
  static int format;

  if (GTK_IS_WIDGET(iwidget))
    widget=gtk_widget_get_toplevel(iwidget);

  wnd=widget;

  opt=soption_menu_get_history(Find_Widget(iwidget,LAB_FOV));
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
  {
    if (!strcmp(name,LAB_FASK)) globinfo.overwrite_mode=0;
    if (!strcmp(name,LAB_FOVER)) globinfo.overwrite_mode=1;
    if (!strcmp(name,LAB_FNOVER)) globinfo.overwrite_mode=2;
  }

  if (!strcmp(name,LAB_FULLGLOBE))
  {
    prefer.full_globe=Get_Button(widget,LAB_FULLGLOBE);
  }
  if (!strcmp(name,LAB_INV_IR))
  {
    prefer.invert_ir=Get_Button(widget,LAB_INV_IR);
    globinfo.spm.inv_ir=prefer.invert_ir;   // maak direct actief
  }

  
  if (!strcmp(name,LAB_LDDEF))
  {
    Load_Defaults(&prefer); 
    pref_to_gui(wnd,NULL,&prefer);
  }
  if (!strcmp(name,LAB_WNDWIDTH))
  {
    prefer.wndwidth=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_WNDHEIGHT))
  {
    prefer.wndheight=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_PREVWNDWIDTH))
  {
    prefer.pr_wndwidth=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_PREVWNDHEIGHT))
  {
    prefer.pr_wndheight=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_LIVEWNDWIDTH))
  {
    prefer.lv_wndwidth=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_LIVEWNDHEIGHT))
  {
    prefer.lv_wndheight=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_FONTSIZE))
  {
    prefer.font_size=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_FILMSIZE))
  {
    prefer.film_size=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_POLARSIZE))
  {
    prefer.polar_size=GTK_ADJUSTMENT(iwidget)->value;
  }

  if (!strcmp(name,LAB_UPDCYCLE))
  {
    prefer.upd_cycle=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_SHOWCHANPREF))
  {
    strcpy(prefer.upd_chan,Get_Entry(gtk_widget_get_toplevel(widget),LAB_SHOWCHANPREF));
  }

  if (!strcmp(name,LAB_SCDIRCOR))
  {
    prefer.scandir_correct=Get_Button(widget,LAB_SCDIRCOR);
  }
  if (!strcmp(name,LAB_HOLDCHNK))
  {
    prefer.low_mem=Get_Button(widget,LAB_HOLDCHNK);
  }
  if (!strcmp(name,LAB_FIXDRAW))
  {
    prefer.size_draw_limit=Get_Button(widget,LAB_FIXDRAW);
  }
  
  if (!strcmp(name,LAB_PJPG))
  {
    strcpy(prefer.prog_jpg,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PJPG));
  }
  if (!strcmp(name,LAB_UJPG))
  {
    strcpy(prefer.ud_jpg,Get_Entry(gtk_widget_get_toplevel(widget),LAB_UJPG));
  }
  if (!strcmp(name,LAB_PPGM))
  {
    strcpy(prefer.prog_pgm,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PPGM));
  }
  if (!strcmp(name,LAB_UPGM))
  {
    strcpy(prefer.ud_pgm,Get_Entry(gtk_widget_get_toplevel(widget),LAB_UPGM));
  }
  if (!strcmp(name,LAB_PAVI))
  {
    strcpy(prefer.prog_playmovie,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PAVI));
  }
  
  if (!strcmp(name,LAB_PMOVIE))
  {
    strcpy(prefer.prog_genmovie,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PMOVIE));
  }
/*
  if (!strcmp(name,LAB_SMOVIE))
  {
    strcpy(prefer.opt_speed,Get_Entry(gtk_widget_get_toplevel(widget),LAB_SMOVIE));
  }
*/
  if (!strcmp(name,LAB_SMOVIE))
  {
    prefer.speed=GTK_ADJUSTMENT(iwidget)->value;
  }

  if (!strcmp(name,LAB_RUNCMD))
  {
    strcpy(prefer.run_cmd,Get_Entry(gtk_widget_get_toplevel(widget),LAB_RUNCMD));
  }

  if (!strcmp(name,LAB_ANADEPCH))
  {
    strcpy(prefer.depth_chan,Get_Entry(gtk_widget_get_toplevel(widget),LAB_ANADEPCH));
//    strcpy(globinfo.depth_chan,prefer.depth_chan);
  }
  if (!strcmp(name,LAB_ANADEPIV))
  {
    prefer.dchan_iv=Get_Button(widget,LAB_ANADEPIV);
//    globinfo.dchan_iv=prefer.dchan_iv;
  }

  if (!strcmp(name,LAB_OVLRED))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.ovl_clr&=0x0ff;
    prefer.ovl_clr|=((n&0xf)<<8);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMEOVLCLR),'b',(prefer.ovl_clr>>4)&0xf0,(prefer.ovl_clr>>0)&0xf0,(prefer.ovl_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_OVLGRN))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.ovl_clr&=0xf0f;
    prefer.ovl_clr|=((n&0xf)<<4);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMEOVLCLR),'b',(prefer.ovl_clr>>4)&0xf0,(prefer.ovl_clr>>0)&0xf0,(prefer.ovl_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_OVLBLE))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.ovl_clr&=0xff0;
    prefer.ovl_clr|=((n&0xf)<<0);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMEOVLCLR),'b',(prefer.ovl_clr>>4)&0xf0,(prefer.ovl_clr>>0)&0xf0,(prefer.ovl_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_LOLARED))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.lonlat_clr&=0x0ff;
    prefer.lonlat_clr|=((n&0xf)<<8);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMELOLACLR),'b',(prefer.lonlat_clr>>4)&0xf0,(prefer.lonlat_clr>>0)&0xf0,(prefer.lonlat_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_LOLAGRN))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.lonlat_clr&=0xf0f;
    prefer.lonlat_clr|=((n&0xf)<<4);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMELOLACLR),'b',(prefer.lonlat_clr>>4)&0xf0,(prefer.lonlat_clr>>0)&0xf0,(prefer.lonlat_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_LOLABLE))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.lonlat_clr&=0xff0;
    prefer.lonlat_clr|=((n&0xf)<<0);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMELOLACLR),'b',(prefer.lonlat_clr>>4)&0xf0,(prefer.lonlat_clr>>0)&0xf0,(prefer.lonlat_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_BFRRED))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.scat_clr&=0x0ff;
    prefer.scat_clr|=((n&0xf)<<8);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMEBFRCLR),'b',(prefer.scat_clr>>4)&0xf0,(prefer.scat_clr>>0)&0xf0,(prefer.scat_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_BFRGRN))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.scat_clr&=0xf0f;
    prefer.scat_clr|=((n&0xf)<<4);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMEBFRCLR),'b',(prefer.scat_clr>>4)&0xf0,(prefer.scat_clr>>0)&0xf0,(prefer.scat_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_BFRBLE))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.scat_clr&=0xff0;
    prefer.scat_clr|=((n&0xf)<<0);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMEBFRCLR),'b',(prefer.scat_clr>>4)&0xf0,(prefer.scat_clr>>0)&0xf0,(prefer.scat_clr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_SBFRRED))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.scat_selclr&=0x0ff;
    prefer.scat_selclr|=((n&0xf)<<8);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMESBFRCLR),'b',(prefer.scat_selclr>>4)&0xf0,(prefer.scat_selclr>>0)&0xf0,(prefer.scat_selclr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_SBFGRN))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.scat_selclr&=0xf0f;
    prefer.scat_selclr|=((n&0xf)<<4);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMESBFRCLR),'b',(prefer.scat_selclr>>4)&0xf0,(prefer.scat_selclr>>0)&0xf0,(prefer.scat_selclr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_SBFBLE))
  {
    n=GTK_ADJUSTMENT(iwidget)->value;
    prefer.scat_selclr&=0xff0;
    prefer.scat_selclr|=((n&0xf)<<0);
    Set_Widgetcolor(Find_Widget(iwidget,LAB_NAMESBFRCLR),'b',(prefer.scat_selclr>>4)&0xf0,(prefer.scat_selclr>>0)&0xf0,(prefer.scat_selclr<<4)&0xf0);
  }

  if (!strcmp(name,LAB_PREFOVL1))
  {
    prefer.prf_ovltype='P';
    globinfo.marktype_vec=FALSE;
  }
  if (!strcmp(name,LAB_PREFOVL2))
  {
    prefer.prf_ovltype='V';
    globinfo.marktype_vec=TRUE;
  }
  if (!strcmp(name,LAB_PREFMARK))
  {
    prefer.add_marks=Get_Button(widget,LAB_PREFMARK);
  }
  if (!strcmp(name,LAB_SIZECMARK))
  {
    prefer.size_cmark=GTK_ADJUSTMENT(iwidget)->value;
  }

  if (!strcmp(name,LAB_EXTMGEN))
  {
    prefer.extern_moviegen=Get_Button(widget,LAB_EXTMGEN);
    Show_Button(iwidget,LAB_PMOVIE,prefer.extern_moviegen);
  }

  if (!strcmp(name,LAB_AVHRRMODE))
  {
    prefer.avhrr_mode=Get_Button(widget,LAB_AVHRRMODE);
  }
  
  if (!strcmp(name,LAB_DELDAYMIN))
  {
    prefer.deldaymin[0]=GTK_ADJUSTMENT(iwidget)->value;
    prefer.deldaymin[0]=MAX(prefer.deldaymin[0],0);
  }
  if (!strcmp(name,LAB_DELHOURMIN))
  {
    prefer.deldaymin[1]=GTK_ADJUSTMENT(iwidget)->value;
    prefer.deldaymin[1]=MIN(MAX(prefer.deldaymin[1],0),23);
  }

  if (!strcmp(name,LAB_DELDAYMAX))
  {
    prefer.deldaymax=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_DROUND))
  {
    prefer.roundday=Get_Button(widget,LAB_DROUND);
    if (prefer.roundday)
      Show_Button(iwidget,LAB_HOURMIN,FALSE);
    else
      Show_Button(iwidget,LAB_HOURMIN,TRUE); 
  }

  if (!strcmp(name,LAB_RECATSTART))
  {
    prefer.record_at_start=Get_Button(widget,LAB_RECATSTART);
  }
  if (!strcmp(name,LAB_DELOLDATSTART))
  {
    prefer.delete_old_at_start=Get_Button(widget,LAB_DELOLDATSTART);
  }

  if (!strcmp(name,LAB_NORF_GET))
  {
    if (!(replace_kepler(NULL,NORADFILE,NORADFILE)))
    {
      Create_Message("OK","Successfully downloaded %s\nfrom %s.",NORADFILE,HTTP_KEPLER);
    }
    else
    {
      Create_Message("Error","Problem downloading %s\nfrom %s.",NORADFILE,HTTP_KEPLER);
    }
  }

  if (!strcmp(name,LAB_EARTHMAP1))
  {
    strcpy(prefer.earthmapfile1,Get_Entry(gtk_widget_get_toplevel(widget),LAB_EARTHMAP1));
  }

  if (!strcmp(name,LAB_EMAP1))
  {
    GtkWidget *wnd=gtk_widget_get_toplevel(iwidget);
    Create_Fileselectf(wnd,"Select earthmap Kepler",NULL,file_ok_sel1,prefer.prog_dir,
                                        NULL,NULL,0,NULL) ;
  }

  if (!strcmp(name,LAB_EARTHMAP2))
  {
    strcpy(prefer.earthmapfile2,Get_Entry(gtk_widget_get_toplevel(widget),LAB_EARTHMAP2));
  }
  if (!strcmp(name,LAB_EMAP2))
  {
    GtkWidget *wnd=gtk_widget_get_toplevel(iwidget);
    Create_Fileselectf(wnd,"Select earthmap Scattero",NULL,file_ok_sel2,prefer.prog_dir,
                                        NULL,NULL,0,NULL) ;
  }

  if (!strcmp(name,LAB_CITYLOC))
  {
    strncpy(prefer.placesfile,Get_Entry(gtk_widget_get_toplevel(widget),LAB_CITYLOC),40);
  }
  if (!strcmp(name,LAB_LUTF))
  {
    strncpy(prefer.lut_file,Get_Entry(gtk_widget_get_toplevel(widget),LAB_LUTF),40);
    strcpy(globinfo.lut.fname,prefer.lut_file);
  }

  if (!strcmp(name,LAB_USEFNFRMT))
  {
    prefer.use_fn_frmt=Get_Button(widget,LAB_USEFNFRMT);
  }
  if (!strcmp(name,LAB_FNFRMT))
  {
    strcpy(prefer.fn_frmt,Get_Entry(gtk_widget_get_toplevel(widget),LAB_FNFRMT));
  }

  if (!strcmp(name,LAB_BRDD))
  {
    Create_Fileselectf(wnd,"Select dest dir",NULL,file_ok_destsel,gdestdir,
                                                             NULL,NULL,0,NULL) ;
  }
  if (!strcmp(name,LAB_DDIR))
  {
    strcpy(prefer.dest_dir,Get_Entry(gtk_widget_get_toplevel(widget),LAB_DDIR));
    finish_path(prefer.dest_dir);
  }

  if (!strcmp(name,LAB_SPLITDESTDIR))
  {
    prefer.split_destdir=Get_Button(widget,LAB_SPLITDESTDIR);
  }

  if (!strcmp(name,LAB_BRSD))
  {
    Create_Fileselectf(wnd,"Select source dir",NULL,file_ok_srcsel,gseldir,
                                                  NULL,NULL,LIMIT_NFILES,NULL) ;
  }
  if (!strcmp(name,LAB_SDIR))
  {
    strcpy(prefer.src_dir,Get_Entry(gtk_widget_get_toplevel(widget),LAB_SDIR));
    finish_path(prefer.src_dir);
  }

  if (!strcmp(name,LAB_ARCHDIR))
  {
    strcpy(prefer.src_archdir,Get_Entry(gtk_widget_get_toplevel(widget),LAB_ARCHDIR));
    finish_path(prefer.src_archdir);
  }

  if (!strcmp(name,LAB_BUFRTBLLOC))
  {
    strcpy(prefer.bufrtblloc,Get_Entry(gtk_widget_get_toplevel(widget),LAB_BUFRTBLLOC));
  }

  if (!strcmp(name,LAB_BUFRPROG))
  {
    strcpy(prefer.prog_bufrextr,Get_Entry(gtk_widget_get_toplevel(widget),LAB_BUFRPROG));
  }
#define LAB_BUFRPROGTEST "Test bufr extractor"
  if (!strcmp(name,LAB_BUFRPROGTEST))
  {
    globinfo.test_bufr=Get_Button(widget,LAB_BUFRPROGTEST);
  }

  if (!strcmp(name,LAB_BRLD))
  {
    Create_Fileselectf(wnd,"Select LUT dir",NULL,file_ok_lutsel,glutdir,
                                                  NULL,NULL,LIMIT_NFILES,NULL) ;
  }
  if (!strcmp(name,LAB_LDIR))
  {
    strcpy(prefer.lut_dir,Get_Entry(gtk_widget_get_toplevel(widget),LAB_LDIR));
    finish_path(prefer.lut_dir);
    strcpy(globinfo.lut.dir,prefer.lut_dir);
  }

  if (!strcmp(name,LAB_SAVE))
  {
    Save_Pref(globinfo.dirsel.list,&prefer,PREFFILE);
    Create_Message("Info","Prefs and dir. list saved in %s.",PREFFILE);
  }

  if (!strcmp(name,LAB_GSAVE))
  {
    save_guistate_all(widget);
    Create_Message("Info","Gui state saved in %s.",GPREFFILE);
    search_file(GPREFFILE,prefer.gui_inifile,
                prefer.cur_dir,prefer.home_dir,prefer.prog_dir);
    Set_Entry(widget,LAB_GPREFFILE,prefer.gui_inifile);
  }

  if (!strcmp(name,LAB_PARTW))
  {
    prefer.geoarea[0].pp_norm.pct_width=GTK_ADJUSTMENT(iwidget)->value;
    prefer.geoarea[0].pp_hrv.pct_width=prefer.geoarea[0].pp_norm.pct_width*33/22;
  }
  if (!strcmp(name,LAB_OFFW))
  {
    prefer.geoarea[0].pp_norm.pct_woffset=GTK_ADJUSTMENT(iwidget)->value;
    prefer.geoarea[0].pp_hrv.pct_woffset=prefer.geoarea[0].pp_norm.pct_woffset*51/36;
  }
  if (!strcmp(name,LAB_PARTH))
  {
    prefer.geoarea[0].pp_norm.pct_height=GTK_ADJUSTMENT(iwidget)->value;
    prefer.geoarea[0].pp_hrv.pct_height=prefer.geoarea[0].pp_norm.pct_height;
  }
  if (!strcmp(name,LAB_OFFH))
  {
    prefer.geoarea[0].pp_norm.pct_hoffset=GTK_ADJUSTMENT(iwidget)->value;
    prefer.geoarea[0].pp_hrv.pct_hoffset=prefer.geoarea[0].pp_norm.pct_hoffset;
  }
#ifdef MPARTS
  for (i=0; i<4; i++)
  {
    char lab[20];
    sprintf(lab,"%s%d",LAB_CLONX,i+1);
    if (!strcmp(name,lab))
    {
      prefer.geoarea[i].center.lon=GTK_ADJUSTMENT(iwidget)->value;
    }
    sprintf(lab,"%s%d",LAB_CLATX,i+1);
    if (!strcmp(name,lab))
    {
      prefer.geoarea[i].center.lat=GTK_ADJUSTMENT(iwidget)->value;
    }
    sprintf(lab,"%s%d",LAB_DLONX,i+1);
    if (!strcmp(name,lab))
    {
      prefer.geoarea[i].delta.lon=GTK_ADJUSTMENT(iwidget)->value;
    }
    sprintf(lab,"%s%d",LAB_DLATX,i+1);
    if (!strcmp(name,lab))
    {
      prefer.geoarea[i].delta.lat=GTK_ADJUSTMENT(iwidget)->value;
    }
    sprintf(lab,"%s%d",LAB_PLDRX,i+1);
    if (!strcmp(name,lab))
    {
      prefer.geoarea[i].pol_dir=GTK_ADJUSTMENT(iwidget)->value;
    }

    sprintf(lab,"%s%d",LAB_PNAMX,i+1);
    if (!strcmp(name,lab))
    {
      strcpy(prefer.geoarea[i].part_name,Get_Entry(gtk_widget_get_toplevel(widget),lab));
    }
  }

  if (!strcmp(name,LAB_PARTDEF))
  {
    for (i=0; i<4; i++)
    {
      char lab[20];
      sprintf(lab,"%s%d",LAB_CLONX,i+1);
      Set_Adjust(wnd,lab,"%f",get_gadef(i,0));
      sprintf(lab,"%s%d",LAB_CLATX,i+1);
      Set_Adjust(wnd,lab,"%f",get_gadef(i,1));
      sprintf(lab,"%s%d",LAB_DLONX,i+1);
      Set_Adjust(wnd,lab,"%f",get_gadef(i,2));
      sprintf(lab,"%s%d",LAB_DLATX,i+1);
      Set_Adjust(wnd,lab,"%f",get_gadef(i,3));
    }
  }

#else
  if (!strcmp(name,LAB_CLON))
  {
    prefer.geoarea[0].center.lon=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_DLON))
  {
    prefer.geoarea[0].delta.lon=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_CLAT))
  {
    prefer.geoarea[0].center.lat=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_DLAT))
  {
    prefer.geoarea[0].delta.lat=GTK_ADJUSTMENT(iwidget)->value;
  }
  if (!strcmp(name,LAB_PARTDEF))
  {
    Set_Adjust(wnd,LAB_PARTW,"%d",22);  // abs. 817
    Set_Adjust(wnd,LAB_OFFW,"%d",36);   // abs. 1336
    Set_Adjust(wnd,LAB_PARTH,"%d",17);  // abs. 631
    Set_Adjust(wnd,LAB_OFFH,"%d",82);   // abs. 3043

    Set_Adjust(wnd,LAB_CLON,"%f",6.);
    Set_Adjust(wnd,LAB_DLON,"%f",19.7);
    Set_Adjust(wnd,LAB_CLAT,"%f",56.);
    Set_Adjust(wnd,LAB_DLAT,"%f",21.);

  }
  if (!strcmp(name,LAB_PARTN))
  {
    strcpy(prefer.geoarea[0].part_name,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PARTN));
  }
#endif

  if (!strcmp(name,SAVE_HMSGMAP))
  {
    GtkWidget *mwnd;
    if ((mwnd=Find_Window(wnd,LAB_COLOR)))
    {
      clrwnd2fmap(mwnd,&prefer.fmap_vis_hmsg);
      Set_Entry(wnd,LAB_PREFMAPINFO,"OK");
    }
    else
    {
      Set_Entry(wnd,LAB_PREFMAPINFO,"! Open mapping!");
    }
  }
  if (!strcmp(name,DEF_HMSGMAP))
  {
    prefer.fmap_vis_hmsg=fmap_vis_hmsg;
    fmap2clrwnd(wnd,&prefer.fmap_vis_hmsg);
  }

  if (!strcmp(name,SAVE_LMSGMAP))
  {
    GtkWidget *mwnd;
    if ((mwnd=Find_Window(wnd,LAB_COLOR)))
    {
      clrwnd2fmap(mwnd,&prefer.fmap_vis_lmsg);
      Set_Entry(wnd,LAB_PREFMAPINFO,"OK");
    }
    else
    {
      Set_Entry(wnd,LAB_PREFMAPINFO,"! Open mapping!");
    }
  }
  if (!strcmp(name,DEF_LMSGMAP))
  {
    prefer.fmap_vis_lmsg=fmap_vis_lmsg;
    fmap2clrwnd(wnd,&prefer.fmap_vis_lmsg);
  }

  if (!strcmp(name,SAVE_NOAA))
  {
    GtkWidget *mwnd;
    if ((mwnd=Find_Window(wnd,LAB_COLOR)))
    {
      clrwnd2fmap(mwnd,&prefer.fmap_vis_noaa);
      Set_Entry(wnd,LAB_PREFMAPINFO,"OK");
    }
    else
    {
      Set_Entry(wnd,LAB_PREFMAPINFO,"! Open mapping!");
    }
  }
  if (!strcmp(name,DEF_NOAA))
  {
    prefer.fmap_vis_noaa=fmap_vis_noaa;
    fmap2clrwnd(wnd,&prefer.fmap_vis_noaa);
  }

  if (!strcmp(name,SAVE_METOP))
  {
    GtkWidget *mwnd;
    if ((mwnd=Find_Window(wnd,LAB_COLOR)))
    {
      clrwnd2fmap(mwnd,&prefer.fmap_vis_metop);
      Set_Entry(wnd,LAB_PREFMAPINFO,"OK");
    }
    else
    {
      Set_Entry(wnd,LAB_PREFMAPINFO,"! Open mapping!");
    }
  }
  if (!strcmp(name,DEF_METOP))
  {
    prefer.fmap_vis_metop=fmap_vis_metop;
    fmap2clrwnd(wnd,&prefer.fmap_vis_metop);
  }

  if (!strcmp(name,LAB_FORMAT))
  {
    if (Get_Button(widget,LAB_FORMAT))
    {
      format=1;
      Set_Entry(widget,LAB_EFORMAT,TXT_MFRMT);
      Set_Adjust(wnd,LAB_MARKLON,"%f",conv_coord(prefer.lmark.lon,'m'));
      Set_Adjust(wnd,LAB_MARKLAT,"%f",conv_coord(prefer.lmark.lat,'m'));
    }
    else
    {
      format=0;
      Set_Entry(widget,LAB_EFORMAT,TXT_GFRMT);
      Set_Adjust(wnd,LAB_MARKLON,"%f",prefer.lmark.lon);
      Set_Adjust(wnd,LAB_MARKLAT,"%f",prefer.lmark.lat);
    }

  }

  if (!strcmp(name,LAB_MARKLAT))
  {
    prefer.lmark.lat=GTK_ADJUSTMENT(iwidget)->value;
    if (format)
      prefer.lmark.lat=conv_coord(prefer.lmark.lat,'g');
  }

  if (!strcmp(name,LAB_MARKLON))
  {
    prefer.lmark.lon=GTK_ADJUSTMENT(iwidget)->value;
    if (format) 
      prefer.lmark.lon=conv_coord(prefer.lmark.lon,'g');
  }
}

void save_guistate_all(GtkWidget *widget)
{
  GUISTATE *gsp=read_guistate(GPREFFILE);
  GtkWidget *wnd2=Find_Window(Find_Parent_Window(widget),LAB_MAIN);
  save_guistate(NULL,Find_Widget(wnd2,PACK_MENU),GPREFFILE,FALSE);
  save_guistate(wnd2,Find_Widget(wnd2,MENU_EVIEW)->parent,GPREFFILE,TRUE);
  save_guistate(wnd2,Find_Widget(wnd2,MENU_AVLIN)->parent,GPREFFILE,TRUE);
  save_guistate(wnd2,Find_Widget(wnd2,MENU_GEOGR_NR)->parent,GPREFFILE,TRUE);
//  save_guistate(NULL,Find_Widget(wnd2,MENU_3DIM),GPREFFILE,TRUE);
//  save_guistate(NULL,Find_Widget(wnd2,MENU_TEMP)->parent,GPREFFILE,TRUE);
  save_guistate(NULL,Find_Widget(wnd2,MAIN_TAB),GPREFFILE,TRUE);
  save_guistate(NULL,Find_Widget(wnd2,RECORD_TAB),GPREFFILE,TRUE);
  save_guistate(NULL,Find_Window(wnd2,LAB_PROGRESS_TRANS),GPREFFILE,TRUE);
  add_guistate(gsp,GPREFFILE);
  Remove_Guistate(gsp);
}


GtkWidget *nb_prefmisc(char *gseldiri)
{
  GtkWidget *w[8];
  gseldir=gseldiri;
  w[1]=Create_ButtonArray("Saved preferences",NULL,2,
                  LABEL,"Preferences file",
                  ENTRY,LAB_PREFFILE ,"%-30s",(*prefer.used_preffile? prefer.used_preffile : "Not found."),
                  LABEL,"Gui state file",
                  ENTRY,LAB_GPREFFILE,"%-30s",(*prefer.gui_inifile? prefer.gui_inifile : "Not found."),
                  0);
  w[2]=NULL;
//  w[1]=Create_Entry(LAB_PREFFILE,NULL,"%-30s",
//       (*prefer.used_preffile? prefer.used_preffile : "Not found."));
//  w[2]=Create_Entry(LAB_GPREFFILE,NULL,"%-30s",
//       (*prefer.gui_inifile? prefer.gui_inifile : "Not found."));

  w[3]=Create_Text("xx",FALSE,NULL);
  Add_Text(w[3],100,"%s\n","Search dirs in next order:");
  Add_Text(w[3],100,"-> %s  (working dir)\n",prefer.cur_dir);
  Add_Text(w[3],100,"-> %s  (home dir)\n",prefer.home_dir);
  Add_Text(w[3],100,"-> %s  (program dir)\n",prefer.prog_dir);

  w[1]=Pack("",'v',w[1],1,w[2],1,w[3],1,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0xff);


  w[2]=Create_Button(LAB_LDDEF,do_prefbut);

  w[3]=Create_Spin(LAB_WNDWIDTH,do_prefbut,"%3d%3d%3d%3d",50,prefer.wndwidth,100,1000);
  w[4]=Create_Spin(LAB_WNDHEIGHT,do_prefbut,"%3d%3d%3d%3d",50,prefer.wndheight,100,1000);
  w[5]=Create_Spin(LAB_FONTSIZE,do_prefbut,"%3d%3d%3d%3d",20,prefer.font_size,0,180);
  w[3]=Pack("",'h',w[3],3,w[4],3,NULL);
  w[3]=Pack("",'v',w[3],3,w[5],3,NULL);
  Set_Widgetcolor(w[3],'b',0,0,0xff);

  w[4]=Create_Check(LAB_HOLDCHNK,do_prefbut,FALSE);
  w[4]=Pack("Memory usage",'h',w[4],1,NULL);
  Set_Widgetcolor(w[4],'b',0,0,0xff);

  w[0]=Pack("",'v',w[1],5,w[2],5,w[3],5,w[4],5,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0xff);

  return w[0];
}

GtkWidget *nb_prefrec()
{
  GtkWidget *w[13];
  w[1]=Create_Spin(LAB_UPDCYCLE,do_prefbut,"%2d%2d%2d",prefer.upd_cycle,1,1000);
  w[2]=Create_Entry(LAB_SHOWCHANPREF,do_prefbut,"%10s","");
  w[0]=Pack("Update during record",'v',w[1],5,w[2],5,NULL);
  Set_Widgetcolor(w[0],'b',0,0,0xff);

  w[3]=Create_Check(LAB_AVHRRMODE,do_prefbut,prefer.avhrr_mode);
  w[3]=Pack("",'h',w[3],1,NULL);
  Set_Widgetcolor(w[3],'b',0,0,0xff);

  w[4]=Create_Label("Delete files");

  w[5]=Create_Spin(LAB_DELDAYMIN,do_prefbut,"%2d%2d%2d",prefer.deldaymin[0],0,365);
  w[6]=Create_Label("days");
  w[5]=Pack("",'h',w[5],1,w[6],5,NULL);
  Set_Widgetcolor(w[5],'b',0,0,0x0);

  w[6]=Create_Label("+");
  w[7]=Create_Spin(LAB_DELHOURMIN,do_prefbut,"%2d%2d%2d",prefer.deldaymin[1],0,23);
  w[8]=Create_Label("hours");
  w[7]=Pack("",'h',w[7],1,w[8],1,NULL);
  Set_Widgetcolor(w[7],'b',0,0,0x0);

  w[7]=Pack(LAB_HOURMIN,'h',w[6],5,w[7],1,NULL);

  w[9]=Create_Label("and older (0 = disabled)");

  w[4]=Pack(NULL,'h',w[4],4,w[5],1,w[7],1,w[9],1,NULL);

  w[10]=Create_Spin(LAB_DELDAYMAX,do_prefbut,"%2d%2d%2d",prefer.deldaymax,0,365);
  w[11]=Create_Label("days (0 = ignore 'older than')");
  w[10]=Pack(NULL,'h',w[10],1,w[11],1,NULL);

  w[12]=Create_Check(LAB_DROUND,do_prefbut,prefer.roundday);
  w[4]=Pack("Delete old files: (menu 'File->Delete old' and 'Delete old')",'v',w[4],1,w[10],1,w[12],1,NULL);

/*
  w[4]=Create_ButtonArray("Delete old files: (for menu 'File->Delete old')",do_prefbut,3,
                            SPIN,LAB_DELDAYMIN,"%2d%2d%2d",prefer.deldaymin,0,365,
                            LABEL,"days and older",
                            LABEL,"(0=don't delete)",
                            SPIN,LAB_DELDAYMAX,"%2d%2d%2d",prefer.deldaymax,0,365,
                            LABEL,"days",
                            LABEL,"(0=ignore 'older than')",
                            CHECK,LAB_DROUND,
                            0);
  Set_Button(w[4],LAB_DROUND,prefer.roundday);
*/
  Set_Widgetcolor(w[4],'b',0,0,0xff);
  w[11]=Create_Check(LAB_RECATSTART,do_prefbut,prefer.record_at_start);
  w[12]=Create_Check(LAB_DELOLDATSTART,do_prefbut,prefer.delete_old_at_start);
  w[11]=Pack("Activate Record at program start:",'v',w[11],1,w[12],1,NULL);
  Set_Widgetcolor(w[11],'b',0,0,0xff);

  w[0]=Pack("",'v',w[0],5,w[3],5,w[4],5,w[11],5,NULL);

  if (prefer.roundday)
    Show_Button(w[0],LAB_HOURMIN,FALSE);
  else
    Show_Button(w[0],LAB_HOURMIN,TRUE); 

  return w[0];
}

static GtkWidget *nb_prefview_ext()
{
  GtkWidget *w[10];
  w[1]=Create_ButtonArray(NULL,do_prefbut,2,
                            ENTRY,LAB_PJPG,"%-12s","",
                            ENTRY,LAB_UJPG,"%-5s","",
                            ENTRY,LAB_PPGM,"%-12s","",
                            ENTRY,LAB_UPGM,"%-5s","",
//                            ENTRY,LAB_PAVI,"%-18s","",
                           0);
  w[2]=Create_Entry(LAB_PAVI,do_prefbut,"%-25s","");

  w[1]=Pack("Viewer programs",'v',w[1],10,w[2],10,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0xff);
/*
  w[3]=Create_ButtonArray("Window size",do_prefbut,2,
                          SPIN,LAB_PREVWNDWIDTH,"%3d%3d%3d%3d",10,prefer.pr_wndwidth,100,1000,
                          SPIN,LAB_LIVEWNDWIDTH,"%3d%3d%3d%3d",10,prefer.lv_wndwidth,100,1000,
                          SPIN,LAB_PREVWNDHEIGHT,"%3d%3d%3d%3d",10,prefer.pr_wndheight,100,1000,
                          SPIN,LAB_LIVEWNDHEIGHT,"%3d%3d%3d%3d",10,prefer.lv_wndheight,100,1000,
                          0);
  Set_Widgetcolor(w[3],'b',0,0,0xff);
*/

  w[3]=Create_ButtonArray("Preview Window",do_prefbut,2,
                          LABEL, "Width ",
                          SPIN,LAB_PREVWNDWIDTH,"%3d%3d%3d%3d",16,prefer.pr_wndwidth,128,1024,
                          LABEL, "Height ",
                          SPIN,LAB_PREVWNDHEIGHT,"%3d%3d%3d%3d",16,prefer.pr_wndheight,128,1024,
                          0);
  Set_Widgetcolor(w[3],'b',0,0,0x0);

  w[4]=Create_ButtonArray("Live Window",do_prefbut,2,
                          LABEL, "Width ",
                          SPIN,LAB_LIVEWNDWIDTH,"%3d%3d%3d%3d",16,prefer.lv_wndwidth,128,1024,
                          LABEL, "Height ",
                          SPIN,LAB_LIVEWNDHEIGHT,"%3d%3d%3d%3d",16,prefer.lv_wndheight,128,1024,
                          0);
  Set_Widgetcolor(w[4],'b',0,0,0x0);

  w[3]=Pack("Window size",'h',w[3],10,w[4],10,NULL);
  Set_Widgetcolor(w[3],'b',0,0,0xff);


  w[1]=Pack("Viewers",'v',w[1],10,w[3],10,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0);

  w[5]=Create_Entry(LAB_BUFRPROG,do_prefbut,"%-20s","");
  w[6]=Create_Check(LAB_BUFRPROGTEST,do_prefbut,FALSE);
  w[5]=Pack("BUFR files",'h',w[5],1,w[6],1,NULL);
  Set_Widgetcolor(w[5],'b',0,0,0xff);

  w[7]=Create_Entry(LAB_RUNCMD,do_prefbut,"%-40s","");
  w[8]=Pack("Execute after each generated file (see record tab main window)",'h',w[7],5,NULL);
  Set_Widgetcolor(w[8],'b',0,0,0xff);

  w[9]=Pack("Post processings",'v',w[5],5,w[8],1,NULL);
  Set_Widgetcolor(w[9],'b',0,0,0);


  w[0]=Pack("",'v',w[1],10,w[9],10,NULL);
  return w[0];
}

GtkWidget *nb_prefgen()
{
  GtkWidget *w[13];

//-----------------------------------------------------------
  w[1]=Create_Check(LAB_SCDIRCOR,do_prefbut,TRUE);
  w[1]=Pack(" ",'h',w[1],1,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0xff);

  w[2]=Create_Optionmenu(LAB_FOV,do_prefbut,0,LAB_FASK,LAB_FOVER,LAB_FNOVER,0);
  w[3]=Create_Check(LAB_PROEPI,do_prefbut,FALSE);
  w[2]=Pack(" ",'h',w[2],10,w[3],10,NULL);
  Set_Widgetcolor(w[2],'b',0,0,0xff);
  w[1]=Pack(NULL,'h',w[1],1,w[2],1,NULL);

//-----------------------------------------------------------

  w[2]=Create_ButtonArray("Overlay colours",do_prefbut,5,
               LABEL,"--",
               LABEL,"Red",
               LABEL,"Green",
               LABEL,"Blue",
               LABEL,"--",

               LABEL,LAB_NAMEOVL,
               SPIN,LAB_OVLRED,"%2d%2d%2d",(prefer.ovl_clr>>8)&0xf,0,0xf,
               SPIN,LAB_OVLGRN,"%2d%2d%2d",(prefer.ovl_clr>>4)&0xf,0,0xf,
               SPIN,LAB_OVLBLE,"%2d%2d%2d",(prefer.ovl_clr>>0)&0xf,0,0xf,
               BUTTON,LAB_NAMEOVLCLR,

               LABEL,LAB_NAMELOLA,
               SPIN,LAB_LOLARED,"%2d%2d%2d",(prefer.lonlat_clr>>8)&0xf,0,0xf,
               SPIN,LAB_LOLAGRN,"%2d%2d%2d",(prefer.lonlat_clr>>4)&0xf,0,0xf,
               SPIN,LAB_LOLABLE,"%2d%2d%2d",(prefer.lonlat_clr>>0)&0xf,0,0xf,
               BUTTON,LAB_NAMELOLACLR,

               LABEL,LAB_NAMEBFR,
               SPIN,LAB_BFRRED,"%2d%2d%2d",(prefer.scat_clr>>8)&0xf,0,0xf,
               SPIN,LAB_BFRGRN,"%2d%2d%2d",(prefer.scat_clr>>4)&0xf,0,0xf,
               SPIN,LAB_BFRBLE,"%2d%2d%2d",(prefer.scat_clr>>0)&0xf,0,0xf,
               BUTTON,LAB_NAMEBFRCLR,

               LABEL,LAB_NAMESBFR,
               SPIN,LAB_SBFRRED,"%2d%2d%2d",(prefer.scat_selclr>>8)&0xf,0,0xf,
               SPIN,LAB_SBFGRN,"%2d%2d%2d",(prefer.scat_selclr>>4)&0xf,0,0xf,
               SPIN,LAB_SBFBLE,"%2d%2d%2d",(prefer.scat_selclr>>0)&0xf,0,0xf,
               BUTTON,LAB_NAMESBFRCLR,
               0);

  Set_Widgetcolor(w[2],'b',0,0,0xff);
  w[3]=Create_Radio(LAB_PREFOVL1,TRUE,do_prefbut);
  w[4]=Create_Radio(LAB_PREFOVL2,FALSE,do_prefbut);
  w[5]=Create_Check(LAB_PREFMARK,do_prefbut,prefer.add_marks);
  w[6]=Create_Spin(LAB_SIZECMARK,do_prefbut,"%2d%2d%2d%02d",5,prefer.size_cmark,0,200);
//  w[7]=Create_Check(LAB_PREFCITIES,do_prefbut,prefer.add_cit);

  w[3]=Pack("Overlay type",'v',w[3],1,w[4],1,w[5],1,w[6],1,NULL);
  Set_Widgetcolor(w[3],'b',0,0,0xff);
  if (prefer.prf_ovltype=='V') Set_Button(w[3],LAB_PREFOVL2,TRUE);

  w[2]=Pack("Overlay",'h',w[2],1,w[3],1,NULL);
  Set_Widgetcolor(w[2],'b',0,0,0xff);

  Set_Widgetcolor(Find_Widget(w[2],LAB_NAMEOVLCLR),'b',(prefer.ovl_clr>>4)&0xf0,(prefer.ovl_clr>>0)&0xf0,(prefer.ovl_clr<<4)&0xf0);
  Set_Widgetcolor(Find_Widget(w[2],LAB_NAMELOLACLR),'b',(prefer.lonlat_clr>>4)&0xf0,(prefer.lonlat_clr>>0)&0xf0,(prefer.lonlat_clr<<4)&0xf0);
  Set_Widgetcolor(Find_Widget(w[2],LAB_NAMEBFRCLR),'b',(prefer.scat_clr>>4)&0xf0,(prefer.scat_clr>>0)&0xf0,(prefer.scat_clr<<4)&0xf0);
  Set_Widgetcolor(Find_Widget(w[2],LAB_NAMESBFRCLR),'b',(prefer.scat_selclr>>4)&0xf0,(prefer.scat_selclr>>0)&0xf0,(prefer.scat_selclr<<4)&0xf0);

//-----------------------------------------------------------
  w[3]=Create_Check(LAB_FULLGLOBE,do_prefbut,prefer.full_globe);
  w[3]=Pack("HRV",'h',w[3],1,NULL);
  Set_Widgetcolor(w[3],'b',0,0,0xff);

  w[4]=Create_Check(LAB_INV_IR,do_prefbut,prefer.invert_ir);
  w[4]=Pack("Infra red",'h',w[4],1,NULL);
  Set_Widgetcolor(w[4],'b',0,0,0xff);

  w[3]=Pack("",'h',w[3],10,w[4],10,NULL);

//-----------------------------------------------------------
  w[4]=Create_Spin(LAB_SMOVIE,do_prefbut,"%2d%2d%2d",prefer.speed,0,100);
  w[5]=Create_Label("frames / sec");
  w[4]=Pack("",'h',w[4],1,w[5],1,NULL);
  Set_Widgetcolor(w[4],'b',0,0,0xff);

  w[5]=Create_Spin(LAB_FILMSIZE,do_prefbut,"%3d%3d%3d%3d",64,prefer.film_size,64,8192);
  w[5]=Pack("",'h',w[5],1,NULL);
  Set_Widgetcolor(w[5],'b',0,0,0xff);
  w[6]=Pack(" ",'h',w[4],5,w[5],5,NULL);
  Set_Widgetcolor(w[6],'b',0x88,0x88,0x88);


  w[7]=Create_ButtonArray("External movie generator",do_prefbut,2,
                            CHECK,LAB_EXTMGEN,
                            ENTRY,LAB_PMOVIE,"%-30s","",
                            0);
  Set_Widgetcolor(w[7],'b',0x88,0x88,0x88);

  w[8]=Pack("Movies",'v',w[6],5,w[7],5,NULL);
  Set_Widgetcolor(w[8],'b',0,0,0xff);

//-----------------------------------------------------------
/*
  w[9]=Create_Entry(LAB_RUNCMD,do_prefbut,"%-40s","");
  w[9]=Pack("Execute after each generated file (see record tab main window)",'h',w[9],5,NULL);
  Set_Widgetcolor(w[9],'b',0,0,0xff);
*/

//-----------------------------------------------------------
  w[10]=Create_ButtonArray("Anaglyph depth map",do_prefbut,2,
                             ENTRY,LAB_ANADEPCH,"%-8s","",
                             CHECK,LAB_ANADEPIV,
                            0);
  Set_Widgetcolor(w[10],'b',0,0,0xff);

//-----------------------------------------------------------
  w[11]=Create_Spin(LAB_POLARSIZE,do_prefbut,"%3d%3d%3d%3d",900,prefer.polar_size,900,18000);
  w[11]=Pack(" ",'h',w[11],5,NULL);
  Set_Widgetcolor(w[11],'b',0,0,0xff);

//-----------------------------------------------------------

  w[0]=Pack(NULL,'v',w[1],1,w[2],1,w[3],1,w[8],1,w[10],1,w[11],1,NULL);

  Set_Button(w[3],LAB_EXTMGEN,prefer.extern_moviegen);

  gtk_widget_show_all(w[0]);
  Show_Button(w[3],LAB_PMOVIE,prefer.extern_moviegen);
  return w[0];
}

GtkWidget *nb_prefdirs()
{
  GtkWidget *w[9];
  w[1]=Create_Button(LAB_BRDD,do_prefbut);
  w[2]=Create_Entry(LAB_DDIR,do_prefbut,"%20s","");
  w[3]=Create_Check(LAB_SPLITDESTDIR,do_prefbut,FALSE);
  w[1]=Pack(NULL,'h',w[1],1,w[2],1,NULL);
  w[1]=Pack("Destination directory",'v',w[1],1,w[3],1,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0xff);

  w[3]=Create_Button(LAB_BRSD,do_prefbut);
  w[4]=Create_Entry(LAB_SDIR,do_prefbut,"%20s","");
  w[3]=Pack("Source directory",'h',w[3],1,w[4],1,NULL);
  Set_Widgetcolor(w[3],'b',0,0,0xff);

  w[5]=Create_Entry(LAB_ARCHDIR,do_prefbut,"%20s","");
  w[5]=Pack("Archive directory",'h',w[5],1,NULL);
  Set_Widgetcolor(w[5],'b',0,0,0xff);

  w[6]=Create_Entry(LAB_BUFRTBLLOC,do_prefbut,"%20s","");
  w[6]=Pack("BUFR Table location",'h',w[6],1,NULL);
  Set_Widgetcolor(w[6],'b',0,0,0xff);

  w[7]=Create_Button(LAB_BRLD,do_prefbut);
  w[8]=Create_Entry(LAB_LDIR,do_prefbut,"%20s","");
  w[7]=Pack("LUT directory",'h',w[7],1,w[8],1,NULL);
  Set_Widgetcolor(w[7],'b',0,0,0xff);

  w[0]=Pack("",'v',w[1],5,w[3],5,w[5],5,w[6],5,w[7],5,NULL);
  return w[0];
}

int Set_Local_Entry(GtkWidget    *wdgt,
              char         *id,
              char         *frmt,...);

void do_srch_set_common(GtkWidget *widget,gpointer data,char *butname,char *str)
{
  GtkWidget *wl=NULL;
  char *name=(char *)data;
  if (widget) wl=widget->parent;

  if (!strcmp(name,butname))
  {
    char *s=Get_Entry(gtk_widget_get_toplevel(widget),butname);
    if (s)
      strcpy(str,s);
  }
  if (!strcmp(name,LAB_FSRCH))
  {
    int res;
    res=search_file(str,NULL,prefer.cur_dir,prefer.home_dir,prefer.prog_dir);
    switch(res)
    {
      case 1:  Set_Local_Entry(wl,LAB_FFND_LOC,"%s",prefer.cur_dir); break;
      case 2:  Set_Local_Entry(wl,LAB_FFND_LOC,"%s",prefer.home_dir); break;
      case 3:  Set_Local_Entry(wl,LAB_FFND_LOC,"%s",prefer.prog_dir); break;
      default: Set_Local_Entry(wl,LAB_FFND_LOC," File not found."); break;
    }
  }
}

GtkWidget *search_set(char *butname,void func())
{
  GtkWidget *w[5];
  w[1]=Create_Entry(butname,func,"%20s","");
  w[2]=Create_Button(LAB_FSRCH,func);
  w[3]=Create_Entry(LAB_FFND_LOC,NULL,"%20s","");
  w[4]=Create_Text("Search locations:",FALSE,NULL);
  Add_Text(w[4],100,"Searched in: (first found is taken)\n");
  Add_Text(w[4],100,"[1] %s\n",prefer.cur_dir);
  Add_Text(w[4],100,"[2] %s\n",prefer.home_dir);
  Add_Text(w[4],100,"[3] %s",prefer.prog_dir);
  w[2]=Pack("",'h',w[2],1,w[3],1,NULL);
  w[0]=Pack(NULL,'v',w[1],1,w[2],1,w[4],1,NULL);
  return w[0];
}

void do_srch_set1(GtkWidget *widget,gpointer data)
{
  do_srch_set_common(widget,data,LAB_NORF,prefer.noradfile);
}

void do_srch_set2(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  do_srch_set_common(widget,data,LAB_CITYLOC,prefer.placesfile);

  if (!strcmp(name,LAB_FSRCH))
  {
    PLACES *pl; int n=0;
    load_places(&prefer);

    for (pl=prefer.places; pl; pl=pl->next) n++;
    Set_Entry(widget,LAB_NRPLACES,"%4d",n);
  }
}

GtkWidget *nb_preffiles(char *gseldiri)
{
  GtkWidget *w[10];
  gseldir=gseldiri;
  w[1]=search_set(LAB_NORF,do_srch_set1);
  w[4]=Create_Button(LAB_NORF_GET,do_prefbut);
  w[5]=Create_Label(" from %s: %s",HTTP_KEPLER,NORADFILE);
  w[4]=Pack(NULL,'h',w[4],1,w[5],1,NULL);  
  w[1]=Pack("Track",'v',w[1],1,w[4],1,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0xff);

  w[2]=Create_Entry(LAB_EARTHMAP1,do_prefbut,"%20s","");
  w[3]=Create_Button(LAB_EMAP1,do_prefbut);
  w[2]=Pack(NULL,'h',w[2],1,w[3],1,NULL);
  w[4]=Create_Entry(LAB_EARTHMAP2,do_prefbut,"%20s","");
  w[5]=Create_Button(LAB_EMAP2,do_prefbut);
  w[4]=Pack(NULL,'h',w[4],1,w[5],1,NULL);
  w[2]=Pack("Earth maps",'v',w[2],1,w[4],1,NULL);
  Set_Widgetcolor(w[2],'b',0,0,0xff);

  w[3]=Create_ButtonArray(NULL,do_prefbut,3,
                            LABEL,"Format",
                            ENTRY_NOFUNC,LAB_EFORMAT,"%-12s",TXT_GFRMT,
                            CHECK,LAB_FORMAT,

                            LABEL,"Lat",
                            SPIN,LAB_MARKLAT,"%8.4f%8.4f%8.4f",prefer.lmark.lat,-9000.,9000.,
                            LABEL,NULL,

                            LABEL,"Lon",
                            SPIN,LAB_MARKLON,"%8.4f%8.4f%8.4f",prefer.lmark.lon,-18000.,18000.,
                            LABEL,NULL,
                            0);
  w[4]=Create_Label("See tab 'Generation; 'Add mark to overlay'");
  w[4]=Pack(NULL,'h',w[4],1,NULL);
  w[3]=Pack("Mark (e.g. your location)",'v',w[3],1,w[4],1,NULL);
  Set_Widgetcolor(w[3],'b',0,0,0xff);

  w[4]=search_set(LAB_CITYLOC,do_srch_set2);
  {
    PLACES *pl; int n=0;
    for (pl=prefer.places; pl; pl=pl->next) n++;
    w[5]=Create_Entry(LAB_NRPLACES,NULL,"%4d",n);
  }
//  w[6]=Create_Led(NULL,(prefer.places? 0x0f0 : 0xf00));
  w[4]=Pack("City locations (gpx)",'h',w[4],1,w[5],1,NULL);
  Set_Widgetcolor(w[4],'b',0,0,0xff);
  w[3]=Pack(NULL,'v',w[3],5,w[4],5,NULL);
  Set_Widgetcolor(w[3],'b',0,0,0xff);

  w[4]=Create_Entry(LAB_LUTF,do_prefbut,"%-15s",prefer.lut_file);
  w[4]=Pack(NULL,'v',w[4],1,NULL);
  w[4]=Pack("LUT",'h',w[4],1,NULL);
  Set_Widgetcolor(w[4],'b',0,0,0xff);

  w[5]=Create_Check(LAB_USEFNFRMT,do_prefbut,FALSE);
  w[6]=Create_Entry(LAB_FNFRMT,do_prefbut,"%20s","");
  w[5]=Pack("",'h',w[5],1,w[6],1,NULL);
  w[7]=Create_Text("xx",FALSE,NULL);
  Add_Text(w[7],100,"%s\n","%k = type (H(rit),L(rit),A(vhrr),M(etop))");
  Add_Text(w[7],100,"%s\n","%s = satellite (e.g. MSG1)");
  Add_Text(w[7],100,"%s\n","%c = channel name (e.g. VIS006)");
  Add_Text(w[7],100,"%s\n","%n = channel nr");
  Add_Text(w[7],100,"%s\n","%C = mapping name");
  Add_Text(w[7],100,"%s\n","%r = segment range (if appl.)");
  Add_Text(w[7],100,"%s\n","%t = time+date (e.g. 063012_1215)");
  Add_Text(w[7],100,"%s\n","other = according to strftime: %t = %y%m%d_%H%M");
  w[5]=Pack("Outputfile name formatting",'v',w[5],1,w[7],1,NULL);
  Set_Widgetcolor(w[5],'b',0,0,0xff);

  w[0]=Create_Notebook(NULL,GTK_POS_TOP,
                       "Track",w[1],
                       "Earth maps",w[2],
                       "Markers",w[3],
                       "LUT",w[4],
                       "Output",w[5],
                        NULL);

//  w[0]=SPack(NULL,"v",w[1],"5",w[2],"5",w[3],"5",w[4],"5",w[5],"ef1",NULL);
  return w[0];
}


GtkWidget *nb_prefmap()
{
  GtkWidget *w[8];
  w[1]=Create_ButtonArray("Save current mapping as:",do_prefbut,1,
                            BUTTON,SAVE_HMSGMAP,
                            BUTTON,SAVE_LMSGMAP,
                            BUTTON,SAVE_NOAA,
                            BUTTON,SAVE_METOP,
                            ENTRY,LAB_PREFMAPINFO,"%10s","",
                            0);
  Set_Widgetcolor(w[1],'b',0,0,0xff);
  w[2]=Create_ButtonArray("Set default mapping for",do_prefbut,1,
                            BUTTON,DEF_HMSGMAP,
                            BUTTON,DEF_LMSGMAP,
                            BUTTON,DEF_NOAA,
                            BUTTON,DEF_METOP,
                            0);
  Set_Widgetcolor(w[2],'b',0,0,0xff);
  w[0]=Pack("",'h',w[1],10,w[2],10,NULL);
  w[4]=Create_ButtonArray("Zoomed boundaries (%)",do_prefbut,2,
               SPIN,LAB_PARTW,"%2d%2d%2d",prefer.geoarea[0].pp_norm.pct_width,0,100,
               SPIN,LAB_OFFW,"%2d%2d%2d",prefer.geoarea[0].pp_norm.pct_woffset,0,100,
               SPIN,LAB_PARTH,"%2d%2d%2d",prefer.geoarea[0].pp_norm.pct_height,0,100,
               SPIN,LAB_OFFH,"%2d%2d%2d",prefer.geoarea[0].pp_norm.pct_hoffset,0,100,
                            0);
  Set_Widgetcolor(w[4],'b',0,0,0x0);
#ifdef MPARTS
  w[5]=Create_ButtonArray("Zoomed boundaries (degrees)",do_prefbut,6,
               LABEL,LAB_PARTN,LABEL,LAB_CLON,LABEL,LAB_CLAT,LABEL,LAB_DLON,LABEL,LAB_DLAT,LABEL,LAB_PLDIR,
               ENTRY,LAB_PNAM1,prefer.geoarea[0].part_name,
               SPIN,LAB_CLON1,"%3.1f%3.1f%3.1f",prefer.geoarea[0].center.lon,-90.,90.,
               SPIN,LAB_CLAT1,"%3.1f%3.1f%3.1f",prefer.geoarea[0].center.lat,-90.,90.,
               SPIN,LAB_DLON1,"%3.1f%3.1f%3.1f",prefer.geoarea[0].delta.lon,0.,90.,
               SPIN,LAB_DLAT1,"%3.1f%3.1f%3.1f",prefer.geoarea[0].delta.lat,0.,90.,
               SPIN,LAB_PLDR1,"%1d%1d%1d",prefer.geoarea[0].pol_dir,-1,1,
               ENTRY,LAB_PNAM2,prefer.geoarea[1].part_name,
               SPIN,LAB_CLON2,"%3.1f%3.1f%3.1f",prefer.geoarea[1].center.lon,-90.,90.,
               SPIN,LAB_CLAT2,"%3.1f%3.1f%3.1f",prefer.geoarea[1].center.lat,-90.,90.,
               SPIN,LAB_DLON2,"%3.1f%3.1f%3.1f",prefer.geoarea[1].delta.lon,0.,90.,
               SPIN,LAB_DLAT2,"%3.1f%3.1f%3.1f",prefer.geoarea[1].delta.lat,0.,90.,
               SPIN,LAB_PLDR2,"%1d%1d%1d",prefer.geoarea[1].pol_dir,-1,1,
               ENTRY,LAB_PNAM3,prefer.geoarea[2].part_name,
               SPIN,LAB_CLON3,"%3.1f%3.1f%3.1f",prefer.geoarea[2].center.lon,-90.,90.,
               SPIN,LAB_CLAT3,"%3.1f%3.1f%3.1f",prefer.geoarea[2].center.lat,-90.,90.,
               SPIN,LAB_DLON3,"%3.1f%3.1f%3.1f",prefer.geoarea[2].delta.lon,0.,90.,
               SPIN,LAB_DLAT3,"%3.1f%3.1f%3.1f",prefer.geoarea[2].delta.lat,0.,90.,
               SPIN,LAB_PLDR3,"%1d%1d%1d",prefer.geoarea[2].pol_dir,-1,1,
               ENTRY,LAB_PNAM4,prefer.geoarea[3].part_name,
               SPIN,LAB_CLON4,"%3.1f%3.1f%3.1f",prefer.geoarea[3].center.lon,-90.,90.,
               SPIN,LAB_CLAT4,"%3.1f%3.1f%3.1f",prefer.geoarea[3].center.lat,-90.,90.,
               SPIN,LAB_DLON4,"%3.1f%3.1f%3.1f",prefer.geoarea[3].delta.lon,0.,90.,
               SPIN,LAB_DLAT4,"%3.1f%3.1f%3.1f",prefer.geoarea[3].delta.lat,0.,90.,
               SPIN,LAB_PLDR4,"%1d%1d%1d",prefer.geoarea[3].pol_dir,-1,1,
                            0);
  w[7]=NULL;
#else
  w[5]=Create_ButtonArray("Zoomed boundaries (degrees)",do_prefbut,2,
               SPIN,LAB_CLON,"%3.1f%3.1f%3.1f",prefer.geoarea[0].center.lon,-90.,90.,
               SPIN,LAB_DLON,"%3.1f%3.1f%3.1f",prefer.geoarea[0].delta.lon,0.,90.,
               SPIN,LAB_CLAT,"%3.1f%3.1f%3.1f",prefer.geoarea[0].center.lat,-90.,90.,
               SPIN,LAB_DLAT,"%3.1f%3.1f%3.1f",prefer.geoarea[0].delta.lat,0.,90.,
                            0);
  w[7]=Create_Entry(LAB_PARTN,do_prefbut,"%-12s",prefer.geoarea[0].part_name);
#endif
  Set_Widgetcolor(w[5],'b',0,0,0x0);
  w[6]=Create_Button(LAB_PARTDEF,do_prefbut);
  w[6]=Pack(NULL,'h',w[6],5,w[7],5,NULL);
  w[7]=Pack("Fixed zoom area",'v',w[4],3,w[5],5,w[6],3,NULL);
  Set_Widgetcolor(w[7],'b',0,0,0xff);
  w[0]=Pack(NULL,'v',w[0],1,w[7],1,NULL);
  return w[0];
}

static void Set_Button_for(GtkWidget *widget, char *lab,char *in,gboolean set)
{
  GtkWidget *w=Find_Widget(widget,in);
  if ((w) && (w->parent))
    Set_Local_Button(w->parent->parent,lab,set);
}



GtkWidget *prefwindow(GtkWidget *widget,char *gseldir)
{
  GtkWidget *wnd,*w[20];
  wnd=Find_Parent_Window(widget);
  wnd=Create_Window(wnd,0,0,LAB_PREFWIN,NULL);
  if (!wnd) return NULL;

  w[1]=nb_prefmisc(gseldir);
  w[2]=nb_prefrec();
  w[3]=nb_prefview_ext();
  w[4]=nb_prefgen();
  w[5]=nb_prefdirs();
  w[6]=nb_preffiles(gseldir);
  w[7]=nb_prefmap();
  w[9]=Create_Notebook(NULL,GTK_POS_TOP,
                       "Misc",w[1],
                       "Record",w[2],
                       "Viewers",w[3],
                       "Generation",w[4],
                       "Directories",w[5],
                       "Files",w[6],
                       "Mappings",w[7],
                        NULL);
  w[9]=Pack("",'h',w[9],1,NULL);
  Set_Widgetcolor(w[9],'b',0xff,0,0);

  w[11]=Create_Button(LAB_SAVE,do_prefbut);
  w[12]=Create_Button(LAB_GSAVE,do_prefbut);
  w[10]=SPack("Save preferences and directory list: ","h",w[11],"1",w[12],"1E",NULL);
  Set_Widgetcolor(w[10],'b',0xff,0,0);

  w[0]=Pack("",'v',w[9],5,w[10],1,NULL);

  strcpy(gseldir,prefer.src_dir);
  strcpy(gdestdir,prefer.dest_dir);
  strcpy(glutdir,prefer.lut_dir);

  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show(wnd);
  pref_to_gui(wnd,&globinfo.dirsel,&prefer);
  Set_Button_for(wnd,LAB_FSRCH,LAB_NORF,TRUE);
  Set_Button_for(wnd,LAB_FSRCH,LAB_CITYLOC,TRUE);
//  Sense_Button(wnd,LAB_PREFMAPINFO,FALSE); // werkt niet?
  return wnd;
}

