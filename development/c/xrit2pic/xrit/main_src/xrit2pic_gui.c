/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Main gui part of xrit2pic
 ********************************************************************/
#include <stdlib.h>
#include "xrit2pic.h"
#include "gtk/gtk.h"
#include "gdk/gdkkeysyms.h"

/* Buttons etc. main */
#define MENU_FILE  "/File   "
#define MENU_RELD  "File   /Reload    "
#define MENU_FINF  "File   /Raw data  "
#define MENU_FILENAME "File   /Filenames selected"
#define MENU_FILEINFO "File   /Fileinfo selected"
#define MENU_FILEARCH "File   /Archive selected"
#define MENU_DELOLD   "File   /Delete old"
#define MENU_DELF  "File   /Delete selected"
#define MENU_DELTF "File   /Delete temp. movie files"
#define MENU_OVERL "File   /Show overlay files"
#define MENU_LUTS  "File   /Show LUT files"
#define MENU_SEPAR "separator/"
#define MENU_QUIT "File   /Quit"

#define MENU_EDIT  "/Edit "
#define MENU_PREFS  "Edit/Preferences"
#define MENU_SGUI   "Edit/Save gui state"


#define LAB_EUROPE   "!Europe"
#define LAB_FIXNAME   "Europe"

#define LAB_RECEIVE "New files"

#define PACK_MENU "!Packmenu"
#define LAB_FIXAREA "Area"

extern XRIT_DBASE dbase;
extern PREFER prefer;
extern char releasenr[100];
extern GROUP *globgrp;
extern GLOBINFO globinfo;

static char gseldir[1000];  // static
static GtkCTree *globdlist;
void create_xritinfo(GROUP *igrp,GtkWidget *widget);


#define NTODO "!To do"
#define NDONE "!Done"
#define LAB_MOVESEL "Move selected"
void do_status(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  GROUP *grp;
  int ntop=0,ndone=0;
  gboolean move_selected=Get_Button(widget,LAB_MOVESEL);
  for (grp=globgrp; grp; grp=grp->next)
  {
    CHANNEL *chan;
    for (chan=grp->pro_epi; chan; chan=chan->next)
    {
      move_segmfiles(chan->pro,name,move_selected);
      move_segmfiles(chan->epi,name,move_selected);
    }
    for (chan=grp->chan; chan; chan=chan->next)
    {
      move_segmfiles(chan->pro,name,move_selected);
      move_segmfiles(chan->epi,name,move_selected);
      move_segmfiles(chan->segm,name,move_selected);
    }
  }
  count_files(globgrp,&ntop,&ndone);
  Set_Entry(widget,NTODO,"%d",ntop);
  Set_Entry(widget,NDONE,"%d",ndone);
}

void list_overlay(GtkWidget *widget);
void list_luts(GtkWidget *widget);
float show_info_sel(GROUP *);
void menu_func(GtkWidget *widget,gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  char *name=(char *)data;

  if (!strcmp(name,MENU_FINF))
  {
    create_piclist(wnd,globgrp,'m');
/*
    float n;
    n=show_info_sel(globgrp);
    Create_Message("File info","Total filesize=%.0f\n(%.0f %c)",
       n,(n>1000000000? n/1000000000: n>1000000?n/1000000: n>1000?n/1000: n),
         (n>1000000000? 'G'         : n>1000000? 'M':      n>1000?'k'   :' '));
*/
  }
  if (!strcmp(name,MENU_FILENAME))
  {
    char *fnstr;
    fnstr=fn_selected(globgrp);
    if (*fnstr)
      Create_Message("Related files",fnstr);
    
  }

  if (!strcmp(name,MENU_FILEINFO))
  {
    create_xritinfo(globgrp,widget);  
  }

  if (!strcmp(name,MENU_FILEARCH))
  {
    char *str;
    Set_Entry(wnd,LAB_INFO,"");
    if ((str=Create_Entrychoice("Move","Move selected to archive")))
    {
      int err_arch;
      gtk_clist_freeze((GtkCList *)dbase.main_tree);
      err_arch=archive_selected(&globgrp,dbase.main_tree,str);

      gtk_clist_thaw((GtkCList *)dbase.main_tree);
      globgrp=Load_List_all(globgrp,dbase.main_tree,widget,0);
      Load_CTree_Dirlist(globdlist,NULL,globinfo.src_archdir,3);
      if (err_arch)
        Set_Entry(wnd,LAB_INFO,"Move to archive failed.");
      else
        Set_Entry(wnd,LAB_INFO,"Move to archive ready.");
    }
    else
    {
      Set_Entry(wnd,LAB_INFO,"Move to archive canceled.");
    }
  }

  if (!strcmp(name,MENU_DELOLD))
  {
    int n;
    char age[40];
    if (prefer.roundday)
      sprintf(age,"%d days",prefer.deldaymin[0]);
    else
      sprintf(age,"%d days, %d hours",prefer.deldaymin[0],prefer.deldaymin[1]);

    n=delete_old(&globgrp,NULL,prefer.deldaymin,prefer.deldaymax,prefer.roundday);
    if (n)
    {
      n=Create_Choice("Warning",2,"Yes","No!","Delete %d groups (age: %s, < %d days old). Proceed?",
                                     n,age,prefer.deldaymax);
      if (n==1)
      {
        gtk_clist_freeze((GtkCList *)dbase.main_tree);
        n=delete_old(&globgrp,dbase.main_tree,prefer.deldaymin,prefer.deldaymax,prefer.roundday);
        gtk_clist_thaw((GtkCList *)dbase.main_tree);

        Set_Entry(wnd,LAB_INFO,"Deleted %d files.",n);
      }
    }
    else
    {
      if ((prefer.deldaymin[0]) || (prefer.deldaymin[1]))
      {
        if (prefer.deldaymax)
          Create_Message("info","Nothing to delete  (age: %s...%d days).",age,prefer.deldaymax);
        else
          Create_Message("info","Nothing to delete  (age: > %s).",age);
      }
      else
      {
        Create_Message("info","Disabled (age=%d days, %d hours)",prefer.deldaymin[0],prefer.deldaymin[1]);
      }
    }
  }

  if (!strcmp(name,MENU_DELF))
  {
    int n;
    n=delete_selected(&globgrp,NULL);
    if (n)
    {
      n=Create_Choice("Warning",2,"Yes","No!","Delete %d files. Proceed?",n);
      if (n==1)
      {
        gtk_clist_freeze((GtkCList *)dbase.main_tree);
        Set_Entry(wnd,LAB_INFO,"Deleting....");
        n=delete_selected(&globgrp,dbase.main_tree);
        gtk_clist_thaw((GtkCList *)dbase.main_tree);
        Set_Entry(wnd,LAB_INFO,"Deleted %d files.",n);
      }
      else if (n==2)
      {
        Set_Entry(wnd,LAB_INFO,"Nothing deleted.");
      }
      else 
      {
        Set_Entry(wnd,LAB_INFO,"(Aborted)");
      }
    }
  }

  if (!strcmp(name,MENU_DELTF))
  {
    int n;
    n=remove_dircontent(globinfo.dest_tmpdir,"*.jpg",FALSE);
    if (n)
    {
      n=Create_Choice("Info",2,"Yes","No","%d items will be removed \nfrom %s. \nDo it?",n,globinfo.dest_tmpdir);
      if (n==1)
      {
        n=remove_dircontent(globinfo.dest_tmpdir,"*.jpg",TRUE);
        Set_Entry(widget,LAB_RECINFO,"Removed %d items.",n);
        Set_Entry(widget,LAB_INFO,"Removed %d items.",n);
      }
    }
    else
    {
      Set_Entry(widget,LAB_RECINFO,"Nothing to remove in %s.",globinfo.dest_tmpdir);
      Set_Entry(widget,LAB_INFO,"Nothing to remove in %s.",globinfo.dest_tmpdir);
    }
  }

  if (!strcmp(name,MENU_OVERL))
  {
    list_overlay(widget);
  }

  if (!strcmp(name,MENU_LUTS))
  {
    list_luts(widget);
  }

  if (!strcmp(name,MENU_RELD))
  {
    Set_Button(widget,LAB_CRLOAD,FALSE);  // Stop cont. updating.
    Set_Button(widget,LAB_EXPCOL,FALSE);

/* set to 'All'; otherwise tree not properly shown (to be fixed) */
    globinfo.vis_mode='a'; 
    if ((Get_Optionsmenu(widget,LAB_VIS))!=0) /* to prevent unnecess. redraw */
      Set_Optionsmenu(widget,LAB_VIS,0); 

    globgrp=Load_List_all(globgrp,dbase.main_tree,widget,0);
    show_tree(dbase.main_tree,globgrp,TRUE,TRUE);  /* redraw tree */
    Load_CTree_Dirlist(globdlist,NULL,globinfo.src_archdir,3);
  }
  
  if (!strcmp(name,MENU_3DIM))
  {
    globinfo.dim3=Get_Button(wnd,MENU_3DIM);
    Sense_Button(wnd,LAB_SHFT3D,globinfo.dim3);
  }

  if (!strcmp(name,MENU_TEMP))
  {
    globinfo.spm.map_temp=Get_Button(wnd,MENU_TEMP);
    if (globinfo.spm.map_temp) Set_Button(wnd,MENU_LUT,FALSE);
  }
  if (!strcmp(name,MENU_TEMPG))
  {
    globinfo.spm.map_temp_G_mA=Get_Button(wnd,MENU_TEMPG);
    if (globinfo.spm.map_temp_G_mA) Set_Button(wnd,MENU_LUT,FALSE);
  }

  if (!strcmp(name,MENU_LUT))
  {
    globinfo.lut.use=Get_Button(wnd,MENU_LUT);
    if (globinfo.lut.use)
    {
      Set_Button(wnd,MENU_TEMP,FALSE);
      Set_Button(wnd,MENU_TEMPG,FALSE);
    }
  }

  if (!strcmp(name,MENU_SLUT))
  {
    select_lut(widget);
  }


  if (!strcmp(name,MENU_FIRE))
  {
    globinfo.spm.fire_detect=Get_Button(wnd,MENU_FIRE);
  }

  if (!strcmp(name,MENU_EVIEW))
  {
    globinfo.view_exported=Get_Button(wnd,MENU_EVIEW);
  }
  if (!strcmp(name,MENU_LONLAT))
  {
    globinfo.add_lonlat=Get_Button(wnd,MENU_LONLAT);
  }

  if (!strcmp(name,MENU_AVLIN))
  {
    globinfo.avhrr_lin=Get_Button(wnd,MENU_AVLIN);
  }
  if (!strcmp(name,MENU_GEOGR_NR))
  {
    globinfo.gmap=normal;
  }
  if (!strcmp(name,MENU_GEOGR_PC))
  {
    globinfo.gmap=plate_carree;
  }
  if (!strcmp(name,MENU_GEOGR_MT))
  {
    globinfo.gmap=mercator;
  }
  if (!strcmp(name,MENU_GEOGR_PN))
  {
    globinfo.gmap=polar_n;
  }
  if (!strcmp(name,MENU_GEOGR_PS))
  {
    globinfo.gmap=polar_s;
  }

  globinfo.area_nr=Get_Optionsmenu(widget,LAB_FIXAREA);
}

void pref_wnd(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;

  if (!strcmp(name,MENU_SGUI))
  {
    save_guistate_all(widget);
    Create_Message("Info","Gui state saved.");
  }
  
  if (!strcmp(name,MENU_PREFS))
    prefwindow(widget,gseldir);
}

/*********************************************************************/


/*******************************************************
 * Create the gui
 *******************************************************/
void create_gui(int argc,char **argv)
{
  GtkWidget *menu,*menubut;
  GtkWidget *wnd,*wz[10],*wa,*wb,*wc,*wd,*we,*wf;
  GtkCTree *ctree;
  GtkCList *dlist;
  #ifdef __GTK_20__
  gtk_disable_setlocale();  // prevent changing . into , !
  #endif
  gtk_init(&argc, &argv);
  gdk_rgb_init();

  globinfo.move_to_done=TRUE;
  globinfo.show_new=TRUE;
  globinfo.dirsel.selrow=-1;
  strcpy(gseldir,prefer.src_dir);

  set_fontsize(prefer.font_size);
  wnd=Create_Window(NULL,prefer.wndwidth,prefer.wndheight,LAB_MAIN,gtk_main_quit);

/* Menu */

  menu=Create_Menu(wnd,MENU_FILE           ,NULL        ,BUTTON,
                         MENU_RELD         ,menu_func   ,BUTTON,
                         MENU_FINF         ,menu_func   ,BUTTON,
                         MENU_FILENAME     ,menu_func   ,BUTTON,
                         MENU_FILEINFO     ,menu_func   ,BUTTON,
                         MENU_FILEARCH     ,menu_func   ,BUTTON,
                         MENU_SEPAR        ,NULL        ,BUTTON,
                         MENU_DELOLD       ,menu_func   ,BUTTON,
                         MENU_DELF         ,menu_func   ,BUTTON,
                         MENU_DELTF        ,menu_func   ,BUTTON,
                         MENU_SEPAR        ,NULL        ,BUTTON,
                         MENU_OVERL        ,menu_func   ,BUTTON,
                         MENU_LUTS        ,menu_func   ,BUTTON,

                         MENU_SEPAR        ,NULL        ,BUTTON,
                         MENU_QUIT         ,Close_Window,BUTTON,
                       MENU_EDIT           ,NULL        ,BUTTON,
                         MENU_PREFS        ,pref_wnd    ,BUTTON,
                         MENU_SGUI         ,pref_wnd    ,BUTTON,
                       MENU_GMAP           ,NULL        ,BUTTON,
                         MENU_AVLIN        ,menu_func   ,CHECK,
                         MENU_GEOGR        ,menu_func   ,BUTTON,
                         MENU_GEOGR_NR     ,menu_func   ,RADIO,
                         MENU_GEOGR_PC     ,menu_func   ,RADIO,
                         MENU_GEOGR_MT     ,menu_func   ,RADIO,
                         MENU_GEOGR_PN     ,menu_func   ,RADIO,
                         MENU_GEOGR_PS     ,menu_func   ,RADIO,
                       MENU_VIEW           ,NULL        ,BUTTON,
                         MENU_LONLAT       ,menu_func   ,CHECK,
                         MENU_EVIEW        ,menu_func   ,CHECK,
                         MENU_3DIM         ,menu_func   ,CHECK,
                         MENU_FIRE         ,menu_func   ,CHECK,
                         MENU_TEMP         ,menu_func   ,CHECK,
                         MENU_TEMPG        ,menu_func   ,CHECK,
                         MENU_LUT          ,menu_func   ,CHECK,
                         MENU_SLUT         ,menu_func   ,BUTTON,
                       NULL);

menubut=NULL;
#ifdef XXX
  menubut=Create_ButtonArray("",menu_func,3,
                     CHECK,LAB_EUROPE,
                     LABEL,prefer.geoarea[0].part_name,
//                     LABEL,LAB_FIXAREA,
                     0);
  Set_Widgetcolor(menubut,'b',0,0,0);
#endif
  menubut=Create_Optionmenu(LAB_FIXAREA,menu_func,0,0);
  {
    int i,j;
    char *tmp[NRGEODEFS+1];
    j=0;
    tmp[j++]="Full";
    for (i=0; i<4; i++)
    {
      if (*prefer.geoarea[i].part_name)
      {
        tmp[j++]=prefer.geoarea[i].part_name;
      }
    }
    Add_Options(menubut,LAB_FIXAREA,menu_func,0,tmp,j,0);
  }
  menubut=Pack("",'h',menubut,1,NULL);
  Set_Widgetcolor(menubut,'b',0,0,0);

  menu=Pack(PACK_MENU,'h',menu,1,menubut,1,NULL);
  wa=nb_main(&ctree);
  dbase.main_tree=ctree;

/* ---------------- Record TAB ---------------- */
  wb=nb_record();

/* ---------------- File manager TAB ---------------- */
  wd=nb_filemngr(&dlist,ctree);
  globdlist=(GtkCTree *)dlist;

/* ---------------- DIR Select TAB ---------------- */
  wc=nb_dirsel(ctree,dlist,gseldir);   // MUST be after nb_filemngr!

  we=nb_expmngr();

/* ---------------- Preferences TAB ---------------- */
//  we=nb_prefs(clist,gseldir);

/* ---------------- Program info TAB ---------------- */
  wf=nb_proginfo(releasenr);
  wz[1]=Create_Notebook(NULL,GTK_POS_TOP,
                     "Main "             ,wa,
                     "Record"            ,wb,
                     "Received "         ,wc,
                     "Archive"           ,wd,
                     "Exported"          ,we,
                     "Program info"      ,wf,
                     NULL);

  wz[0]=SPack(NULL,"v",menu,"1",wz[1],"ef1",NULL);
  gtk_container_add(GTK_CONTAINER(wnd),wz[0]);
  gtk_widget_show_all(wnd);

//  Read_Pref(&globinfo.dirsel,&prefer); // afgeleide dirs al bepaald in xrit2pic.c
  Read_PrefDirlist(&globinfo.dirsel,&prefer); // afgeleide dirs al bepaald in xrit2pic.c
  strcpy(globinfo.upd_chan,prefer.upd_chan);
  strcpy(globinfo.upd_sat,prefer.upd_sat);
  pref_to_gui(wnd,&globinfo.dirsel,&prefer);

  define_and_load_srcdir(wnd,wc);

  gtk_widget_hide((Find_Widget(wnd,LAB_MORE))->parent);

  gtk_window_set_default_size((GtkWindow *)wnd,1,1); /* to force that space of hidden arts is removed */
  search_file(GPREFFILE,prefer.gui_inifile,
              prefer.cur_dir,prefer.home_dir,prefer.prog_dir);

  Set_Entry(wnd,LAB_SHOWCHAN,globinfo.upd_chan);  /*  */
//  Set_Entry(wnd,LAB_SHOWSAT,globinfo.upd_sat);  /*  */

  Set_Button(wnd,MENU_AVLIN,globinfo.avhrr_lin);  /* Set default linearize for AVHRR */

  if (*prefer.gui_inifile) printf("Reading gui init-file %s..\n",prefer.gui_inifile);
  restore_guistate_fromfile(wa,prefer.gui_inifile);

/* Overwrite restore */
// For toggles: First set it Off, then to the state needed!
  Set_Button(wnd,LAB_CRLOAD,FALSE); 
  Set_Button(wnd,LAB_CRLOAD,prefer.record_at_start);

  Set_Button(wnd,LAB_CDELOLD,FALSE);
  Set_Button(wnd,LAB_CDELOLD,prefer.delete_old_at_start);

  Set_Entry(wnd,LAB_INFO,"");     /* don't restore! */
  Set_Entry(wnd,LAB_RECINFO,"");  /* don't restore! */
  Set_Entry(wnd,LAB_NUPDATE,"");  /* don't restore! */
  switch(globinfo.gmap)
  {
    case normal      : Set_Button(wnd,MENU_GEOGR_NR,TRUE); break;
    case plate_carree: Set_Button(wnd,MENU_GEOGR_PC,TRUE); break;
    case mercator    : Set_Button(wnd,MENU_GEOGR_MT,TRUE); break;
    case polar_n     : Set_Button(wnd,MENU_GEOGR_PN,TRUE); break;
    case polar_s     : Set_Button(wnd,MENU_GEOGR_PS,TRUE); break;
    case polar_cn     :  break; // te doen
    case polar_cs     :  break; // te doen
  }
  Set_Button(wnd,MENU_LUT,globinfo.lut.use);
  gtk_main();
}

static GtkCTree *ctree;
static GtkCTreeNode *sel_node;
static void func(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_EXPCOL))
  {
    Update_Togglelabel(widget);
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
      gtk_ctree_expand_recursive(ctree,sel_node);
    else
      gtk_ctree_collapse_recursive(ctree,sel_node);
  }
}

static void sel_made(GtkWidget *widget,gpointer data)
{
  sel_node=(GtkCTreeNode *)data;
}

#define LAB_EXPCOL "Expand/Collapse"
#define LENSTR 200
void create_xritinfo(GROUP *igrp,GtkWidget *widget)
{
  GtkWidget *wnd,*wb[3];
  GtkCTreeNode *gn,*cn,*sn,*hn,*hn2;
  char str[LENSTR];
  char *tmphdr[4];
  char *tmp[4];
  int i;
  GROUP *grp;
  CHANNEL *chan;
  SEGMENT *segm;
  sel_node=NULL;

  tmphdr[0]="";
  wnd=Create_Window(Find_Parent_Window(widget),500,300,"Xrit header info",NULL);
  if (!wnd) return;

  ctree=Create_CTree("X",sel_made,NULL,NULL,NULL,NULL,1,tmphdr);
  wb[1]=Create_Toggle(LAB_EXPCOL,func,FALSE);
  wb[0]=Pack(NULL,'v',GTK_WIDGET(ctree)->parent,1,wb[1],1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),wb[0]);
  gtk_widget_show(wnd);
  gtk_clist_freeze((GtkCList *)ctree);
  gn=cn=sn=hn=hn2=NULL;
  for (grp=igrp; grp; grp=grp->next)
  {
    if (grp->nselected)
    {
      tmp[0]="Group";
      gn=Add_CLeaf(ctree,NULL,tmp);
      if ((grp->chan) && (grp->chan->segm))
      {
        snprintf(str,LENSTR,"Sub-sat position: lon=%.2f",grp->chan->segm->xh.sub_lon);
        tmp[0]=str;
        cn=Add_CLeaf(ctree,gn,tmp);
      }
    }
    for (chan=grp->chan; chan; chan=chan->next)
    {
      if ((grp->nselected) || (chan->nselected))
      {
//if (chan->Cal_Offset[0]>10.) printf("!! %f\n",chan->Cal_Offset[0]);
        snprintf(str,LENSTR,"Channel %s",chan->chan_name);
        tmp[0]=str;
        cn=Add_CLeaf(ctree,gn,tmp);
        snprintf(str,LENSTR,"Cal_Offset %f",chan->Cal_Offset[0]);
        tmp[0]=str;
        Add_CLeaf(ctree,cn,tmp);
        snprintf(str,LENSTR,"Cal_Slope %f",chan->Cal_Slope[0]);
        tmp[0]=str;
        Add_CLeaf(ctree,cn,tmp);
      }
      for (segm=chan->segm; segm; segm=segm->next)
      {
        FILE *fp;
        if ((!grp->nselected) && (!chan->nselected) && (!segm->nselected)) continue;
        fp=open_xritimage(segm->pfn,&segm->xh,segm->xh.xrit_frmt,
          (segm==chan->segm? segm->chan : NULL));
        snprintf(str,LENSTR,"Segment %d",segm->xh.segment);
        tmp[0]=str;
        sn=Add_CLeaf(ctree,cn,tmp);
        sprintf(str,"  File=%s",segm->fn);
        tmp[0]=str;
        Add_CLeaf(ctree,sn,tmp);
        if (fp)
        {
          switch(segm->xh.xrit_frmt)
          {
            case EXRIT: case JMALRIT: case NXRIT:
              sprintf(str,"  coff=%d, cfac=%d, loff=%d, lfac=%d",segm->xh.coff,segm->xh.cfac,segm->xh.loff,segm->xh.lfac);
              tmp[0]=str;
              Add_CLeaf(ctree,sn,tmp);
              sprintf(str,"  Headers");
              tmp[0]=str;
              hn=Add_CLeaf(ctree,sn,tmp);
              {
                unsigned char l[100];
                int type,len,hdr_len;
                rewind(fp);
                fread(l,16,1,fp);
                type=l[0];
                len=(l[1]<<8)+l[2];
                if (type==0)
                {
                  hdr_len=(l[4]<<24)+(l[5]<<16)+(l[6]<<8)+l[7];
                  rewind(fp);
                  do
                  {
                    char *stype="";
                    fread(l,3,1,fp);
                    type=l[0];
                    len=(l[1]<<8)+l[2];
                    switch(type)
                    {
                      case   0: stype=" >>> Primary Header";             break;
                      case   1: stype=" >>> Image Structure";            break;
                      case   2: stype=" >>> Image Navigation";           break;
                      case   3: stype=" >>> Image Data Function";        break;
                      case   4: stype=" >>> Annotation";                 break;
                      case   5: stype=" >>> Time Stamp";                 break;
                      case   6: stype=" >>> Ancillary Text";             break;
                      case   7: stype=" >>> Key Header";                 break;
                      case 128: stype=" >>> Segment Identification";     break;
                      case 129: stype=" >>> Image Segment Line Quality"; break;
                      case 131:
                        switch(segm->xh.xrit_frmt)
                        {
                          case NXRIT  : stype=" >>> Rice Compression record"; break;
                          case JMALRIT: stype=" >>> Image Observation Time";   break;
                          default     : stype=" >>> Unknown type";            break;
                        }
                      break;
                      case 132:
                        switch(segm->xh.xrit_frmt)
                        {
                          case JMALRIT: stype=" >>> Image Quality Information"; break;
                          default     : stype=" >>> Unknown type";             break;
                        }
                      break;
                      default : stype=" >>> Unknown type"; break;
                    }
                    sprintf(str,"type=%4d  len=%5d  %s         [fp=%4lx]",type,len,stype,ftell(fp)-3);
                    tmp[0]=str;
                    hn2=Add_CLeaf(ctree,hn,tmp);

                    if ((type==3) && (chan->cal.caltbl[0]))
                    {
                      int last=chan->cal.nrcalpoints-1;
                      if ((last>1) && (chan->cal.caltbl[1][last]==chan->cal.caltbl[1][last-1])) last--;
                      sprintf(str,"Calibration: %d points;",last+1);
                      sprintf(str,"%s %s; ",str,(chan->cal.caltbl_type=='k'? "Kelvin" : "percent"));
                      if (chan->cal.caltbl[0])
                      { 
                        sprintf(str,"%s [%.0f,%f] ... [%.0f,%f]",str,
                          chan->cal.caltbl[0][0],chan->cal.caltbl[1][0],
                          chan->cal.caltbl[0][last],chan->cal.caltbl[1][last]);
                      }
                      tmp[0]=str;
                      Add_CLeaf(ctree,hn2,tmp);
                    }

                    if ((type==131) && (segm->xh.xrit_frmt==JMALRIT))
                    {
                      char *p1,*p2;
                      char *s=NULL;
                      if ((len<1000) && ((s=calloc(len,1))))
                      {
                        fread(s,len-3,1,fp);
                        fseek(fp,-1*(len-3),SEEK_CUR);
                        while ((p1=strchr(s,'\r'))) *p1='\n';
                        p1=s;
                        while ((p2=strchr(p1,'\n')))
                        {
                          *p2=0;
                          tmp[0]=p1;
                          Add_CLeaf(ctree,hn2,tmp);
                          p1=p2+1;
                        }
                        free(s);
                      }
                    }

                    strcpy(str,"hexdump: ");
                    fseek(fp,-3,SEEK_CUR);
                    for (i=0; i<MIN(len,16); i++)
                    {
                      sprintf(str,"%s %02x",str,fgetc(fp));
                    }
                    if (len>16) sprintf(str,"%s .... (%d more)",str,len-16);
                    tmp[0]=str;
                    Add_CLeaf(ctree,hn2,tmp);
                    fseek(fp,len-MIN(len,16),SEEK_CUR);
//                    fseek(fp,len-3,SEEK_CUR);
                    hdr_len-=len;
                  } while (hdr_len>0);
                }
              }
            break;
            case METOP:
              if (segm->orbit.duration_of_product)
              {
                sprintf(str,"duration: %d",segm->orbit.duration_of_product);
                Add_CLeaf(ctree,sn,tmp);

                strcpy(str,"Position");
                tmp[0]=str;
                hn=Add_CLeaf(ctree,sn,tmp);
                sprintf(str,"Start: (%.2f,%.2f)  end: (%.2f,%.2f)",
                  segm->orbit.subsat_start.lat,segm->orbit.subsat_start.lon,
                  segm->orbit.subsat_end.lat,segm->orbit.subsat_end.lon);
                tmp[0]=str;
                Add_CLeaf(ctree,hn,tmp);

                sprintf(str,"orbit_start: %d",segm->orbit.orbit_start);
                Add_CLeaf(ctree,hn,tmp);
                sprintf(str,"epoch_year: %d",segm->orbit.epoch_year);
                Add_CLeaf(ctree,hn,tmp);
                sprintf(str,"epoch_day: %.2f",segm->orbit.epoch_day);
                Add_CLeaf(ctree,hn,tmp);
                sprintf(str,"eccentricity: %.2f",segm->orbit.eccentricity);
                Add_CLeaf(ctree,hn,tmp);
                sprintf(str,"inclination: %.2f",segm->orbit.inclination);
                Add_CLeaf(ctree,hn,tmp);
                sprintf(str,"perigee: %.2f",segm->orbit.perigee);
                Add_CLeaf(ctree,hn,tmp);
                sprintf(str,"raan: %.2f",segm->orbit.raan);
                Add_CLeaf(ctree,hn,tmp);
                sprintf(str,"anomaly: %.2f",segm->orbit.anomaly);
                Add_CLeaf(ctree,hn,tmp);
              }
            break;
            default:
            break;
          } // switch
          fclose(fp);
        }
        tfreenull(&segm->xh.img_obs_time);
      }
    }
    if ((grp->chan) && (grp->chan->segm))
    {
      if ((grp->nselected) || (grp->chan->nselected) || (grp->chan->segm->nselected))
      {
        switch(grp->chan->segm->xh.xrit_frmt)
        {
          case MHRPT: case NHRPT:
            snprintf(str,LENSTR,"Kepler");
            tmp[0]=str;
            hn=Add_CLeaf(ctree,gn,tmp);

            snprintf(str,LENSTR,"epoch_year %d",grp->pc.kepler.epoch_year);
            tmp[0]=str;
            Add_CLeaf(ctree,hn,tmp);
            sprintf(str,"epoch_day: %.2f",grp->pc.kepler.epoch_day);
            Add_CLeaf(ctree,hn,tmp);
            sprintf(str,"decay: %.8f",grp->pc.kepler.decay_rate);
            Add_CLeaf(ctree,hn,tmp);
            sprintf(str,"inclination: %.2f",R2D(grp->pc.kepler.inclination));
            Add_CLeaf(ctree,hn,tmp);
            sprintf(str,"raan: %.2f",R2D(grp->pc.kepler.raan));
            Add_CLeaf(ctree,hn,tmp);
            sprintf(str,"eccentricity: %.2f",R2D(grp->pc.kepler.eccentricity));
            Add_CLeaf(ctree,hn,tmp);
            sprintf(str,"perigee: %.2f",R2D(grp->pc.kepler.perigee));
            Add_CLeaf(ctree,hn,tmp);
            sprintf(str,"anomaly: %.2f",R2D(grp->pc.kepler.anomaly));
            Add_CLeaf(ctree,hn,tmp);
          break;
          default:
          break;
        }
      }
    }
  }
  gtk_clist_thaw((GtkCList *)ctree);
}
