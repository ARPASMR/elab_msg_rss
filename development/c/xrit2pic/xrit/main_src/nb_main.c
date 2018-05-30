/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include "gtk/gtk.h"


#define LAB_MESSG "Messages"
#define LAB_VISH1 "Hrit1 pic"
#define LAB_VISH2 "Hrit2 pic"
#define LAB_VISHA "Hrit all"
#define LAB_VISL1 "Lrit1 pic"
#define LAB_VISL2 "Lrit2 pic"
#define LAB_VISLA "Lrit all"
#define LAB_VISF  "Other"
#define LAB_VISMET5 "Met5"
#define LAB_VISMET6 "Met6"
#define LAB_VISMET7 "Met7"
#define LAB_VISM  "Mpef"
#define LAB_VISNOAA  "NOAA"
#define LAB_VISMETP  "METOP"
#define LAB_DWDSAT  "DWDSAT"
#define LAB_BUFR  "BUFR"
#define LAB_ERS   "ERS"
#define LAB_ERSP  "ERS+Geo"

#define LAB_VISA  "All  "              // Beware of LAB_BOTH!

#define LAB_CLRMAP "Mapping"
#define LAB_EXPCLR "Enable"
#define LAB_HRVLUM "Hires"

#define LAB_CLRSPECMAP "RGBComp"

#define LAB_FIXMAP "!Fixed"
#define LAB_NRGB    "nrgb"
#define LAB_AIRMASS "airm"
#define LAB_DUST    "dust"
#define LAB_NFOG    "fog Night"
#define LAB_DFOG    "fog Day"
#define LAB_UCLD    "mu_cloud"
#define LAB_UDUST   "mu_dust"
#define LAB_UASH    "mu_ash"
#define LAB_UNIGHT  "mu_night"
#define LAB_UDAY    "mu_day"
#define LAB_CNVSTRM "conv_storm"
#define LAB_SNOWFOG "snow fog"

#define LAB_PREVIEW "Preview"
#define LAB_MAKE "Export "
#define LAB_OVERL "Overlay"
#define LAB_CHLAY "!overlay"
#define LAB_COAST "Coast"
#define LAB_COUNTR "Cntry"
#define LAB_LATLON "Latlon"
#define LAB_BOTH "All   "              // extra space because of LAB_VISA!
#define LAB_SELALL "Sel. all"

#define LAB_LISTCHAN "Images"  /* tree ID */

/****** MORE part ******/
#define LAB_ADV "More"

#define LAB_BITSHFT "!Bitshift"
#define LAB_7_0 "[7:0]"
#define LAB_8_1 "[8:1]"
#define LAB_9_2 "[9:2]"
#define LAB_BITLED "!Lumi"
#define LAB_INVERT "Invert"


#define LAB_WORLDMAP "World\nmap"

/****** end MORE part ******/

#define LAB_FILEGEN "File generation"

#define LIST_NRCOL 10
#define NR_TREECOLS 10

#include "preview.h"

extern GLOBINFO globinfo;
extern GROUP *globgrp;
extern PREFER prefer;

static GtkCTree *globtree;

/*********************************************************************/
/* Delayed function activation. 
   To prevent useless function calls in case of signal-bursts.
*/
static gboolean colormap_busy;
gboolean create_colormap_add(gpointer data)
{
  GtkWidget *twnd=data;
  color_map(twnd,globgrp);
  colormap_busy=FALSE;
  return FALSE;
}

void create_colormap_delayed(GtkWidget *twnd)
{
  if (!colormap_busy)
  {
    gtk_timeout_add(100,create_colormap_add,twnd);
    colormap_busy=TRUE;
  }
}

static gboolean colormapspec_busy;
gboolean create_colormapspec_add(gpointer data)
{
  GtkWidget *twnd=data;
  color_map_spec(twnd,globgrp);
  colormapspec_busy=FALSE;
  return FALSE;
}

void create_colormapspec_delayed(GtkWidget *twnd)
{
  if (!colormapspec_busy)
  {
    gtk_timeout_add(100,create_colormapspec_add,twnd);
    colormapspec_busy=TRUE;
  }
}

static gboolean piclist_busy;
static gboolean create_piclist_add(gpointer data)
{
  GtkWidget *twnd=data;
  create_piclist(twnd,globgrp,0);
  piclist_busy=FALSE;
  return FALSE;
}

static void create_piclist_delayed(GtkWidget *twnd)
{
  if (!piclist_busy)
  {
    gtk_timeout_add(100,create_piclist_add,twnd);
    piclist_busy=TRUE;
  }
}


static CHANNEL *get_selected_chan(GROUP *grp)
{
  GROUP *sgrp;
  CHANNEL *schan=NULL;
  for (sgrp=grp; sgrp; sgrp=sgrp->next)
  {
    if (sgrp->nselected)
    {
      schan=sgrp->chan;
      break;
    }
    for (schan=sgrp->chan; schan; schan=schan->next)
    {
      if (schan->nselected)
      {
        break;
      }
    }
    if (schan) break;
  }
  return schan;
}


void pol_savesegmselect(GtkWidget *widget,CHANNEL *schan)
{
  globinfo.segmrange[0]=(int)Get_Adjust(widget,LAB_SEGMFRST);
  if (schan)
  {
    if (globinfo.segmrange[0] > schan->seq_end)
      Set_Adjust(widget,LAB_SEGMFRST,"%d",schan->seq_end);
  }
  if (Get_Adjust(widget,LAB_SEGMNR))
  {
    if (globinfo.segmrange[0]==0) globinfo.segmrange[0]=1;
    globinfo.segmrange[1]=globinfo.segmrange[0]+(int)Get_Adjust(widget,LAB_SEGMNR)-1;
  }
  else
    globinfo.segmrange[1]=0;
}

static char type_selected()
{
  GROUP *grp;
  for (grp=globgrp; grp; grp=grp->next)
  { 
    if (grp->nselected) return grp->h_or_l;
  }
  return 0;
}

/*******************************************************
 * Handle main buttons etc.
 *******************************************************/
static void do_mainbut(GtkWidget *widget,gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  GtkWidget *popup_window=Find_Window(wnd,LAB_PROGRESS_TRANS);
  GtkWidget *wnd_emap=Find_Window(wnd,LAB_EARTHWND);
  static int sav_bitshift;

  char *name=(char *)data;
  char *opt;
  GENMODES genmode;
  memset(&genmode,0,sizeof(genmode));

  get_genmode_from_gui(widget,&genmode);
  genmode.area_nr=globinfo.area_nr;

  genmode.avhrr_lin=globinfo.avhrr_lin;
  genmode.avhrr_lin_dblx=FALSE;
  genmode.gmap=globinfo.gmap;
  

  opt=soption_menu_get_history(Find_Widget(widget,LAB_FFT));
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
  {
    if (!strcmp(name,LAB_PGM))
    {
      Set_Optionsmenu(popup_window,LAB_FFTP,1);  // set button popup if present
    }
    else if (!strcmp(name,LAB_PGM8))
    {
      Set_Optionsmenu(popup_window,LAB_FFTP,2);
    }
    else if (!strcmp(name,LAB_JPG))
    {
      Set_Optionsmenu(popup_window,LAB_FFTP,3);
    }
    else if (!strcmp(name,LAB_CJPG))
    {
      Set_Optionsmenu(popup_window,LAB_FFTP,4);
    }
    else if (!strcmp(name,LAB_FFTDEF))
    {
      Set_Optionsmenu(popup_window,LAB_FFTP,0);
    }
  }
  
#ifdef XXX
  opt=soption_menu_get_history(Find_Widget(widget,LAB_FOV));
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
  {
    if (!strcmp(name,LAB_FASK)) globinfo.overwrite_mode=0;
    if (!strcmp(name,LAB_FOVER)) globinfo.overwrite_mode=1;
    if (!strcmp(name,LAB_FNOVER)) globinfo.overwrite_mode=2;
  }
#endif
  
  opt=soption_menu_get_history(Find_Widget(widget,LAB_VIS));
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
  {
    int showflag=0;
    if (!strcmp(name,LAB_MESSG))  { globinfo.vis_mode='S'; showflag=1; }
    if (!strcmp(name,LAB_VISH1))  { globinfo.vis_mode='h'; showflag=1; }
    if (!strcmp(name,LAB_VISH2))  { globinfo.vis_mode='g'; showflag=1; }
    if (!strcmp(name,LAB_VISHA))  { globinfo.vis_mode='H'; showflag=1; }
    if (!strcmp(name,LAB_VISL1))  { globinfo.vis_mode='l'; showflag=1; }
    if (!strcmp(name,LAB_VISL2))  { globinfo.vis_mode='k'; showflag=1; }
    if (!strcmp(name,LAB_VISLA))  { globinfo.vis_mode='L'; showflag=1; }
    if (!strcmp(name,LAB_VISF))   { globinfo.vis_mode='f'; showflag=1; }
    if (!strcmp(name,LAB_VISMET5)){ globinfo.vis_mode='5'; showflag=1; }
    if (!strcmp(name,LAB_VISMET6)){ globinfo.vis_mode='6'; showflag=1; }
    if (!strcmp(name,LAB_VISMET7)){ globinfo.vis_mode='7'; showflag=1; }
    if (!strcmp(name,LAB_VISM))   { globinfo.vis_mode='m'; showflag=1; }
    if (!strcmp(name,LAB_VISNOAA)){ globinfo.vis_mode='A'; showflag=1; }
    if (!strcmp(name,LAB_VISMETP)){ globinfo.vis_mode='M'; showflag=1; }
    if (!strcmp(name,LAB_DWDSAT)) { globinfo.vis_mode='D'; showflag=1; }
    if (!strcmp(name,LAB_BUFR))   { globinfo.vis_mode='B'; showflag=1; }
    if (!strcmp(name,LAB_ERS))    { globinfo.vis_mode='E'; showflag=1; }
    if (!strcmp(name,LAB_ERSP))   { globinfo.vis_mode='s'; showflag=1; }
    
    if (!strcmp(name,LAB_VISA))   { globinfo.vis_mode='a'; showflag=1; }
    if (showflag)
    {
      int expcol=Get_Button(wnd,LAB_EXPCOL);
      Set_Button(wnd,LAB_EXPCOL,FALSE);  /* collapse tree */
      show_tree(globtree,globgrp,TRUE,TRUE);  /* redraw tree */
      Set_Button(wnd,LAB_EXPCOL,expcol); /* restore expand/collapse state */
    }
  }

  if (!strcmp(name,LAB_PREVIEW))
  {
    int nrp;
    genmode.otype='v';
    if ((nrp=nr_pics(wnd,globgrp,&genmode)) > 1)
    {
      if ((nrp=nr_pics_nonbufr(wnd,globgrp,&genmode)) > 1)
      {
        create_piclist(wnd,globgrp,'v');   // >1 pic
      }
      else if (nrp==1)                     // 1 pic, >=1 bufr
      {
        gen_prev_scat(wnd,&genmode,globgrp);
      }
      else  // nrp==0                      // 0 pic, >=1 bufr
      {
        gen_prev_scatmap(wnd,&genmode,globgrp);
      }

#ifdef XXX
      if (nrp==2)
      {
        GROUP *gsel1,*gsel2;
        get_selected_item_rng(globgrp,&genmode,TRUE);     /* Init selection */
        gsel1=get_selected_item_rng(NULL,&genmode,TRUE);
        gsel2=get_selected_item_rng(NULL,&genmode,TRUE);
        if (!gen_prev_scat(wnd,&genmode,gsel1,gsel2))
          create_piclist(wnd,globgrp,'v');   // not scat+non-scat, so do via list
      }
      else
      {
        create_piclist(wnd,globgrp,'v');
      }
#endif
    }
    else
    {
      gen_item(wnd,globgrp,&genmode,&prefer);
    }
  }

  if (!strcmp(name,LAB_MAKE))
  {
    genmode.agl.shift_3d=Get_Adjust(wnd,LAB_SHFT3D);
    genmode.agl.lmax=(1<<(globinfo.bitshift+8))-1;
    genmode.agl.lmin=0;
    genmode.ol_lum=0xff;
    genmode.otype='f';
    if ((nr_pics(wnd,globgrp,&genmode)) > 1)
      create_piclist(wnd,globgrp,'f');
    else
      gen_item(wnd,globgrp,&genmode,&prefer);
  }

  if (!strcmp(name,LAB_OVERL))
  {
    globinfo.add_overlay=Get_Button(widget,name);
    if (globinfo.add_overlay)
      Sense_Button(widget,LAB_CHLAY,TRUE);
    else
      Sense_Button(widget,LAB_CHLAY,FALSE);

  }
  globinfo.add_marks=globinfo.add_overlay && prefer.add_marks;


  if ((!strcmp(name,LAB_SEGMFRST)) || (!strcmp(name,LAB_SEGMNR)) ||
      (!strcmp(name,LAB_WORLDMAP)))
  {
    GROUP *sgrp=NULL;
    CHANNEL *schan=NULL;

    schan=get_selected_chan(globgrp);
    if (schan) sgrp=schan->group;
    if ((sgrp) && (sgrp->orbittype==POLAR))
    {
      if (!strcmp(name,LAB_WORLDMAP))
      {
        open_earth(wnd,TRUE);
      }
      if ((!strcmp(name,LAB_SEGMFRST)) || (!strcmp(name,LAB_SEGMNR)))
      {
        char str[3][100];
        pol_savesegmselect(widget,schan);
        if (sgrp)
        {
          if ((!genmode.area_nr) || (sgrp->orbittype==POLAR))
          {
            sgrp->segm_first=globinfo.segmrange[0];
            sgrp->segm_last=globinfo.segmrange[1];
          }
          else
          {
            sgrp->segm_first=0;
            sgrp->segm_last=0;
          }
          if ((noaa_coverage(sgrp,str[0],str[1],str[2],globinfo.segmrange[0],globinfo.segmrange[1]))>=0)
          {
            gtk_ctree_node_set_text(globtree,sgrp->node,6,str[0]);
            gtk_ctree_node_set_text(globtree,sgrp->node,8,str[2]);
          }

        }

        /* Copy to selects in worldmap window */
        Set_Adjust(wnd_emap,LAB_SEGMFRST,"%d",(int)Get_Adjust(wnd,LAB_SEGMFRST));
        Set_Adjust(wnd_emap,LAB_SEGMNR,"%d",(int)Get_Adjust(wnd,LAB_SEGMNR));
      }
    }
  }

  if (!strcmp(name,LAB_CLRMAP))
  {
    color_map(wnd,globgrp);
  }

  if (!strcmp(name,LAB_CLRSPECMAP))
  {
    color_map_spec(wnd,globgrp);
  }

  if ((!strcmp(name,LAB_NRGB)) || (!strcmp(name,LAB_AIRMASS)) ||
      (!strcmp(name,LAB_DUST)) || (!strcmp(name,LAB_NFOG)) ||
      (!strcmp(name,LAB_DFOG)) || (!strcmp(name,LAB_UCLD)) ||
      (!strcmp(name,LAB_UDUST)) || (!strcmp(name,LAB_UASH)) ||
      (!strcmp(name,LAB_UNIGHT)) || (!strcmp(name,LAB_UDAY)) ||
      (!strcmp(name,LAB_CNVSTRM)) || (!strcmp(name,LAB_SNOWFOG)))
  {
    if ((Find_Window(wnd,LAB_COLOR)))
    {
      create_colormap_delayed(wnd);
    }
    if ((Find_Window(wnd,LAB_COLORSPEC)))
    {
      create_colormapspec_delayed(wnd);
    }
  }

  if (!strcmp(name,LAB_HRVLUM))
  {
    globinfo.hrv_lum=Get_Button(wnd,LAB_HRVLUM);
  }
  
  if (!strcmp(name,LAB_EXPCLR))
  {
    globinfo.spm.compose=Get_Button(wnd,LAB_EXPCLR);
    if (globinfo.spm.compose)
    {
      Sense_Button(widget,LAB_CLRMAP,TRUE);
      Sense_Button(widget,LAB_CLRSPECMAP,TRUE);
      Sense_Button(widget,LAB_HRVLUM,TRUE);
      globinfo.hrv_lum=Get_Button(widget,LAB_HRVLUM);
    }
    else
    {
      Sense_Button(widget,LAB_CLRMAP,FALSE);
      Sense_Button(widget,LAB_CLRSPECMAP,FALSE);
      Sense_Button(widget,LAB_HRVLUM,FALSE);
      globinfo.hrv_lum=FALSE;
    }

    if ((nr_pics(wnd,globgrp,&genmode)) > 1)
    {
      if (Find_Window(wnd,LAB_PROGRESS_TRANS))
        create_piclist(wnd,globgrp,0);
    }
  }

  if (!strcmp(name,LAB_EXPCOL))
  {
    Update_Togglelabel(widget);
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
      GROUP *grp1;

      for (grp1=globgrp; grp1; grp1=grp1->next)
        if (grp1->node)
          gtk_ctree_expand(globtree,grp1->node);
    }
    else
    {
      gtk_ctree_collapse_recursive(globtree,NULL);
    }
  }
  if (!strcmp(name,LAB_SELALL))
  {
    GROUP *grp1;
    for (grp1=globgrp; grp1; grp1=grp1->next)
      if (grp1->node)
        gtk_ctree_select(globtree,grp1->node);

  }
  
  opt=soption_menu_get_history(Find_Widget(widget,LAB_BITSHFT));
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
  {
    if (!strcmp(name,LAB_7_0))
    {
      Set_Led(widget,LAB_BITLED,0xfff);
      globinfo.bitshift=0;
      Set_Adjust(Find_aw_Widget(wnd,Lab_LMIN),Lab_LMIN,"%d",0);
      Set_Adjust(Find_aw_Widget(wnd,Lab_LMAX),Lab_LMAX,"%d",255);
    }
    if (!strcmp(name,LAB_8_1))
    {
      Set_Led(widget,LAB_BITLED,0x666);
      globinfo.bitshift=1;
      Set_Adjust(Find_aw_Widget(wnd,Lab_LMIN),Lab_LMIN,"%d",0);
      Set_Adjust(Find_aw_Widget(wnd,Lab_LMAX),Lab_LMAX,"%d",511);
    }
    if (!strcmp(name,LAB_9_2))
    {
      Set_Led(widget,LAB_BITLED,0x000);
      globinfo.bitshift=2;
      Set_Adjust(Find_aw_Widget(wnd,Lab_LMIN),Lab_LMIN,"%d",0);
      Set_Adjust(Find_aw_Widget(wnd,Lab_LMAX),Lab_LMAX,"%d",1023);
    }
  }

  if (!strcmp(name,LAB_INVERT))
  {
    globinfo.spm.invert=Get_Button(wnd,LAB_INVERT);
  }
  if (!strcmp(name,LAB_SHFT3D))
  {
    globinfo.shift_3d=Get_Adjust(wnd,LAB_SHFT3D);
    genmode.agl.shift_3d=Get_Adjust(wnd,LAB_SHFT3D);
  }

  if (!strcmp(name,LAB_ADV))
  {
    if (Get_Button(widget,LAB_ADV))
    {
      gtk_widget_show((Find_Widget(widget,LAB_MORE))->parent);
    }
    else
    {
      gtk_widget_hide((Find_Widget(widget,LAB_MORE))->parent);
    }
  }

  opt=soption_menu_get_history(Find_Widget(widget,LAB_CHLAY));
#ifdef __GTK_20__
  if (opt)
  {
    if (!strcmp(opt,LAB_COAST))  globinfo.overlaytype=coast_p;
    if (!strcmp(opt,LAB_COUNTR)) globinfo.overlaytype=country_p;
    if (!strcmp(opt,LAB_LATLON)) globinfo.overlaytype=lonlat_p;
    if (!strcmp(opt,LAB_BOTH))   globinfo.overlaytype=both_p;  

  }
#else
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
  {
    if (!strcmp(name,LAB_COAST))  globinfo.overlaytype=coast_p;
    if (!strcmp(name,LAB_COUNTR)) globinfo.overlaytype=country_p;
    if (!strcmp(name,LAB_LATLON)) globinfo.overlaytype=lonlat_p;
    if (!strcmp(name,LAB_BOTH))   globinfo.overlaytype=both_p;
  }
#endif

  {
    GtkWidget *wndp=Find_Parent_Window(widget);
    if (GTK_IS_WINDOW(wndp))
      gtk_window_set_default_size((GtkWindow *)wndp,0,0);
  }

  opt=soption_menu_get_history(Find_Widget(widget,LAB_FIXMAP));
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
  {
//    GtkWidget *wcm=Find_Window(wnd,LAB_COLOR);
//    GtkWidget *wcms=Find_Window(wnd,LAB_COLORSPEC);
    if (!strcmp(name,LAB_NRGB))
    {

      if (type_selected()=='A')
        set_mapping_from_fixed(map_noaavis);
      else if (type_selected()=='M')
        set_mapping_from_fixed(map_metopvis);
      else if (type_selected()=='G')
        set_mapping_from_fixed(map_metopvis);
      else if (type_selected()=='L')
        set_mapping_from_fixed(map_vis_lmsg);
      else
        set_mapping_from_fixed(map_vis_hmsg);   // overwrites command line settings!

      if (sav_bitshift)
        Set_Optionsmenu(wnd,LAB_BITSHFT,sav_bitshift-1);
      sav_bitshift=0;
    }

    if (!strcmp(name,LAB_AIRMASS))
    {
      set_mapping_from_fixed(map_airm);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_DUST))
    {
      set_mapping_from_fixed(map_dust);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_NFOG))
    {
      set_mapping_from_fixed(map_nfog);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_DFOG))
    {
      set_mapping_from_fixed(map_dfog);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }

    if (!strcmp(name,LAB_UCLD))
    {
      set_mapping_from_fixed(map_ucld);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_UDUST))
    {
      set_mapping_from_fixed(map_udust);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_UASH))
    {
      set_mapping_from_fixed(map_uash);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_UNIGHT))
    {
      set_mapping_from_fixed(map_unight);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_UDAY))
    {
      set_mapping_from_fixed(map_uday);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_CNVSTRM))
    {
      set_mapping_from_fixed(map_cnv_strm);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
    if (!strcmp(name,LAB_SNOWFOG))
    {
      set_mapping_from_fixed(map_snowfog);
      if (!sav_bitshift) sav_bitshift=Get_Optionsmenu(wnd,LAB_BITSHFT)+1;
      Set_Optionsmenu(wnd,LAB_BITSHFT,0);
    }
  }
}


/* Satellite select function */
static void Sat_selmade(GtkWidget *widget,gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  GtkWidget *twnd=gtk_widget_get_toplevel(widget);
  int frmt=0;
  GROUP *grp;
  GtkCTreeNode *node;
  node=(GtkCTreeNode *)data;
  frmt=mark_selected(globgrp,node,TRUE);
//  globinfo.bufr=connect_bufr(globgrp,"ers2","scatt");
//  if (globinfo.bufr) puts(globinfo.bufr->pfn);

  Set_Button_All_wnd(wnd,LAB_FCLR_PREV,TRUE);

  if ((Find_Window(wnd,LAB_PROGRESS_TRANS)))
  {
    create_piclist_delayed(twnd);
  }
  if ((Find_Window(wnd,LAB_COLOR)))
  {
    create_colormap_delayed(twnd);
  }
  if ((Find_Window(wnd,LAB_COLORSPEC)))
  {
    create_colormapspec_delayed(twnd);
  }

  /* If normal rgb is selected then touch select button 
     to force reload mapping. Needed to switch from MSG <--> NOAA type channels.
   */
  if ((!Get_Optionsmenu(twnd,LAB_FIXMAP))) // NRGB selected
    do_mainbut(twnd,LAB_NRGB);

  if ((grp=get_first_selected_grp(globgrp)))
  {
    int segm_first=grp->segm_first;  // copy to locals to prevent problems during adjust
    int segm_last=grp->segm_last;    // (Set_Adjust causes function call to segm. spins!)
    if (segm_first)
    {
      Set_Adjust(twnd,LAB_SEGMNR,"%d",segm_last-segm_first+1);
    }
    else
    {
      Set_Adjust(twnd,LAB_SEGMNR,"%d",0);
    }
    Set_Adjust(twnd,LAB_SEGMFRST,"%d",segm_first+1);  // to force signal to do_but (see polar_map) to redraw
    Set_Adjust(twnd,LAB_SEGMFRST,"%d",segm_first);

  }

  for (grp=globgrp; grp; grp=grp->next)
  {
    int nsp;
    char str[2][40];
    if ((grp->node==node) && (grp->nselected))
    {
      if (!strncmp(grp->sat_src,"ers",3))
      {
        if ((nsp=scatt_coverage(grp,str[0]))>=0)
        {
          sprintf(str[1],"%d points",nsp);
          gtk_ctree_node_set_text(globtree,grp->node,6,str[0]);
          gtk_ctree_node_set_text(globtree,grp->node,7,str[1]);
        }
        else
        {
          Set_Entry(widget,LAB_INFO,"Translator '%s' or bufr tables not found!?",prefer.prog_bufrextr);
          break;
        }
      }
    }
  }
}

/* Satellite unselect function */
static void Sat_sellost(GtkWidget *widget,gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  GtkWidget *twnd=gtk_widget_get_toplevel(widget);
  GtkCTreeNode *node;
  node=(GtkCTreeNode *)data;
  mark_selected(globgrp,node,FALSE);

  if ((Find_Window(Find_Parent_Window(widget),LAB_PROGRESS_TRANS)))
  {
    create_piclist_delayed(twnd);
  }
  if ((Find_Window(wnd,LAB_COLOR)))
  {
    create_colormap_delayed(twnd);
  }
  if ((Find_Window(wnd,LAB_COLORSPEC)))
  {
    create_colormapspec_delayed(twnd);
  }
}

GtkWidget *nb_main(GtkCTree **ctree)
{
  GtkWidget *wa[6],*wb[5],*wc[7],*wy[5],*wz[5];
  GtkWidget *wd[6],*we[5],*wf[7],*wg[2];

  char *tmphdr[NR_TREECOLS];
  int def_overlaynr=1;                // Default overlay type; 1=country
  globinfo.overlaytype=country_p;     // Default overlay type; country
  globinfo.marktype_vec=(prefer.prf_ovltype=='V');  // Default marks etc.
  
/* Column names notebook */
  tmphdr[0]="";
  tmphdr[1]="Source";
  tmphdr[2]="Channel";
  tmphdr[3]="Time";
  tmphdr[4]="Date ";
  tmphdr[5]="Chan./Format ";
  tmphdr[6]="Segm / orbit[pos]";
  tmphdr[7]="Orbit info";
  tmphdr[8]="end orbit[pos]";
  tmphdr[9]="Kepler age";

/* 'Main' part of notebook */
  *ctree=Create_CTree(LAB_LISTCHAN,Sat_selmade,Sat_sellost,NULL,NULL,NULL,LIST_NRCOL,tmphdr);
  globtree=*ctree;

/* Allow multiple selection */
  gtk_clist_set_selection_mode(GTK_CLIST(*ctree),GTK_SELECTION_EXTENDED);

/* ---------------- MAIN TAB ---------------- */
/* Extract */
  wa[1]=Create_Button(LAB_PREVIEW,do_mainbut);
  wa[2]=Create_Button(LAB_MAKE,do_mainbut);
  wa[3]=Create_Check(LAB_OVERL,do_mainbut,FALSE);
  wa[4]=Create_Optionmenu(LAB_CHLAY,do_mainbut,def_overlaynr,LAB_COAST,LAB_COUNTR,LAB_LATLON,LAB_BOTH,0);
  wa[4]=SPack("","v",wa[3],"ef1",wa[4],"ef1",NULL);
  wa[0]=SPack(NULL,"v",wa[1],"ef1",wa[2],"ef1",NULL);
  wa[0]=SPack("Extract","h",wa[0],"ef1",wa[4],"ef1",NULL);
  Set_Widgetcolor(wa[0],'b',0,0,0);
  Sense_Button(wa[0],LAB_CHLAY,FALSE);

/* List */
  wb[1]=Create_Toggle(LAB_EXPCOL,do_mainbut,FALSE);
  wb[2]=Create_Button(LAB_SELALL,do_mainbut);
  wb[1]=Pack(NULL,'h',wb[1],1,wb[2],1,NULL);
  wb[3]=Create_Optionmenu(LAB_VIS,do_mainbut,0,
             LAB_VISA,LAB_MESSG,LAB_VISH1,LAB_VISH2,LAB_VISHA,LAB_VISL1,LAB_VISL2,LAB_VISLA,
             LAB_VISMET5,LAB_VISMET6,LAB_VISMET7,
             LAB_VISF,LAB_VISM,LAB_VISNOAA,LAB_VISMETP,LAB_DWDSAT,
             LAB_BUFR,LAB_ERS,LAB_ERSP,0);
             
  wb[0]=SPack("List ","v",wb[1],"ef1",wb[3],"ef1",NULL);
  Set_Widgetcolor(wb[0],'b',0,0,0);

/* Color */
  wc[1]=Create_Check(LAB_EXPCLR,do_mainbut,FALSE);
  wc[2]=Create_Check(LAB_HRVLUM,do_mainbut,FALSE);
  wc[1]=Pack(NULL,'h',wc[1],1,wc[2],1,NULL);
  wc[3]=Create_Button(LAB_CLRMAP,do_mainbut);
  wc[4]=Create_Button(LAB_CLRSPECMAP,do_mainbut);
wc[3]=Pack(NULL,'v',wc[3],1,wc[4],1,NULL);
  wc[5]=Create_Optionmenu(LAB_FIXMAP,do_mainbut,0,
                           LAB_NRGB,LAB_AIRMASS,LAB_DUST,LAB_NFOG,LAB_DFOG,
                           LAB_UCLD,LAB_UDUST,LAB_UASH,LAB_UNIGHT,LAB_UDAY,
                           LAB_CNVSTRM,LAB_SNOWFOG,
                           0);

//wc[1]=Pack(NULL,'h',wc[1],1,wc[4],1,NULL);

  wc[3]=Pack(NULL,'h',wc[3],1,wc[5],1,NULL);
  wc[1]=Pack("Colours",'v',wc[1],1,wc[3],1,NULL);
  Set_Widgetcolor(wc[1],'b',0,0,0);
  Sense_Button(wc[1],LAB_CLRMAP,FALSE);
  Sense_Button(wc[1],LAB_CLRSPECMAP,FALSE);
  
  Sense_Button(wc[2],LAB_HRVLUM,FALSE);

  wc[0]=SPack(NULL,"h",wc[1],"ef1",NULL);
  Set_Widgetcolor(wc[0],'b',0,0,0);

/********* More part *********/
/* Bitshift, Invert */
  wd[3]=Create_Optionmenu(LAB_BITSHFT,do_mainbut,1,LAB_7_0,LAB_8_1,LAB_9_2,0);
  wd[4]=Create_Led(LAB_BITLED,0x666);
  wd[3]=Pack("Bitselect",'h',wd[3],1,wd[4],1,NULL);
  wd[5]=Create_Check(LAB_INVERT,do_mainbut,FALSE);
  wd[3]=Pack("",'v',wd[3],1,wd[5],1,NULL);
  Set_Widgetcolor(wd[3],'b',0,0,0);

/* Format generated file */
  we[1]=Create_Optionmenu(LAB_FFT,do_mainbut,0,LAB_FFTDEF,LAB_PGM,LAB_PGM8,LAB_JPG,LAB_CJPG,0);
  we[0]=Pack(LAB_FILEGEN,'h',we[1],1,NULL);
  Set_Widgetcolor(we[0],'b',0,0,0);

/* segment selection */
  wf[1]=Create_Button(LAB_WORLDMAP,do_mainbut);
  wf[2]=Create_Spin(LAB_SEGMFRST,do_mainbut,"%2d%2d%2d",0,0,999);
  wf[3]=Create_Spin(LAB_SEGMNR,do_mainbut,"%2d%2d%2d",0,0,999);
  wf[0]=Pack("",'v',wf[2],1,wf[3],1,NULL);
  wf[0]=Pack("Segment range",'h',wf[0],2,wf[1],2,NULL);
  Set_Widgetcolor(wf[0],'b',0,0,0);

  wg[1]=Create_Spin(LAB_SHFT3D,do_mainbut,"%2d%2d%2d",DEF_3DSHIFT,0,MAX_3DSHIFT);
  wg[1]=Pack("Anagl",'h',wg[1],1,NULL);
  Set_Widgetcolor(wg[1],'b',0,0,0);

/* more packing */
  wg[0]=SPack(LAB_MORE,"h",we[0],"1",wf[0],"1",wd[3],"1",wg[1],"1",NULL);


/* Info */
  wy[1]=Create_Label("Info");
  wy[2]=Create_Entry(LAB_INFO,NULL,"%25s","");
  wy[3]=Create_Check(LAB_ADV,do_mainbut,FALSE);
  wy[0]=SPack("","h",wy[1],"1",wy[2],"ef1",wy[3],"1",NULL);

/* Get button-part together.
wa=extract, wb=list,  wc=colors wd=bitsel/invert, we=format, wf=segmsel
*/
  wz[2]=SPack(NULL,"h",wa[0],"ef2",wb[0],"2",wc[0],"ef2",NULL); /* generation */
  wz[3]=SPack(NULL,"h",wg[0],"ef1",NULL);          /* list */

  wz[0]=Pack(MAIN_TAB,'v',*ctree,1,wz[2],4,wz[3],2,wy[0],2,NULL);

  gtk_widget_show_all(wz[0]);
  Sense_Button(wz[0],LAB_SHFT3D,globinfo.dim3);

  do_mainbut(wz[0],LAB_NRGB);

  return wz[0];
}
