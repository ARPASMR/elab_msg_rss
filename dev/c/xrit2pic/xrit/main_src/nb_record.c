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
#define LAB_UPDATE "Update list 1x"
#define LAB_NOTIFY "MSG Notify"
#define LAB_CDELOLDNEXT "Delete-run every"

#define PART_REMOVECHAN "!Delete all channels EXCEPT"
#define LAB_REMOVECHAN "Delete items"
#define LAB_MOVDONE "Move"
#define LAB_SHOWNEW "Show"
#define LAB_SHOWCLR "Colour"
#define LAB_MAPCLR "Colour Map"

#define LAB_LGEN "On "
#define LAB_RMALL "Remove raw data"
#define LAB_GENMOV "Movie"
#define LAB_TIMEID "Add Time id"
#define LAB_RUNCMD "Run command"
#define LAB_ESSENTIAL "0/6/12/18"
#define LAB_REGEL "Manage files"

#define LAB_RECEIVE "Record"

#define LAB_LISTRECEIVED "List Received"


#define AFTERREC_SET "ARECSET"
#define AFTERREC_DEL "ARECDEL"
#define SHOWLIVE "SHOWLIVE"
#define LAB_UNKNOWN "Keep unknown"
#define LAB_CDDELRUNNEXT "Delete info:"

extern XRIT_DBASE dbase;
extern GLOBINFO globinfo;
extern PREFER prefer;
extern GROUP *globgrp;
extern char *channellist[];
extern char *satlist[];

/* Load channelmapping requested for generation. */
void load_chanmap(GENMODES *gmode,char *chan_name)
{
  if (gmode->chanmap) Remove_Chanmap(&gmode->chanmap);
  if (gmode->spm.compose)
  {
    gmode->chanmap=Copy_Chanmap(globinfo.chanmap);
  }
  else
  {
    Create_Chanmap(&gmode->chanmap,chan_name,1.,1.,1.);
  }
}

static gboolean loadlistfunc(GtkWidget *widget,gboolean move_avhrr,gboolean show_new)
{
  static int n;
  static int v;

  char vv[]="|/-\\";
  Set_Entry(widget,LAB_NUPDATE,"%-4d  ...",n);

/* set to 'All'; otherwise tree not properly shown (to be fixed) */
  globinfo.vis_mode='a'; 
  if ((Get_Optionsmenu(widget,LAB_VIS))!=0) /* to prevent unnecess. redraw */
    Set_Optionsmenu(widget,LAB_VIS,0);

/* In LoadList: pass NULL on widget arg. to disable progress funcs etc. */
  globgrp=Load_List(globgrp,dbase.main_tree,widget,globinfo.src_dir,FALSE,in_received,TRUE,0);
  n+=Process_Segm(widget,NULL,globgrp,globinfo.move_to_done,move_avhrr,show_new);

  if (globinfo.gen_new)
  {
    GENMODES gmode;
    memset(&gmode,0,sizeof(gmode));
    gmode.gen_film=globinfo.gen_movienew;
    gmode.spm.compose=globinfo.spm_live.compose;
    load_chanmap(&gmode,globinfo.upd_chan);  // no colour: gmode->chanmap rgb=(1,1,1)
    gmode.area_nr=globinfo.area_nr;
    if (gmode.gen_film)
      gmode.cformat='j';
    else
      gmode.cformat=get_cformat(widget);
    gmode.overwrite_mode=REPLACE;
    gmode.filmframe_ovrwrmode=REPLACE;
    gmode.timeid=Get_Button(widget,LAB_TIMEID);
    gmode.run_script=Get_Button(widget,LAB_RUNCMD);
    gmode.essential_only=Get_Button(widget,LAB_ESSENTIAL);
    
    gmode.wnd_clrmap=LAB_COLORFIX;
    auto_translate(&gmode,globgrp,&prefer,widget);                         /* Translate pics */
  }
  
  Set_Entry(widget,LAB_NUPDATE,"%-4d  %c",n,vv[v]);
  v++; if (v>=4) v=0;

  return TRUE;
}

static char *pri_curtime(int hoffset)
{
  time_t time2;
  static char s[20];
  struct tm tm;
  time(&time2);
  tm=*gmtime(&time2);
  tm.tm_hour+=hoffset;
  mktime_ntz(&tm);
  strftime(s,15,"%b %d %H:%M",&tm);
  return s;
}

static gboolean dellistoldcont(GtkWidget *widget)
{
  int n;
  int delcycle=(prefer.deldaymin[0]? 24 : 1); // each day or each hour
  gtk_clist_freeze((GtkCList *)dbase.main_tree);
  n=delete_old(&globgrp,dbase.main_tree,prefer.deldaymin,prefer.deldaymax,prefer.roundday);
  gtk_clist_thaw((GtkCList *)dbase.main_tree);
  
  Set_Entry(widget,LAB_CDDELRUNNEXT,"next at %s.",pri_curtime(delcycle));
  Set_Entry(widget,LAB_RECINFO,"Deleted %d items.",n);
  return TRUE;
}

static gboolean loadlistcont(GtkWidget *widget)
{
  return loadlistfunc(widget,FALSE,globinfo.show_new);
}

void do_recordbut(GtkWidget *widget,gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  char *name=(char *)data;
  if (!strcmp(name,LAB_UPDATE))
  {
    loadlistcont(widget);
  }

  if (!strcmp(name,LAB_NOTIFY))
  {
    globinfo.notify=Get_Button(widget,LAB_NOTIFY);
  }

  if (!strcmp(name,LAB_CDELOLD))
  {
    static int to=0;

    if ((Update_Togglelabel(widget)) && (!to))
    {           
      int delcycle=(prefer.deldaymin[0]? 24 : 1); // each day or each hour
      Set_Widgetcolor(widget,'b',0xff,0,0);
      if (delcycle==1)
        Set_Entry(widget,LAB_CDELOLDNEXT,"hour");
      else
        Set_Entry(widget,LAB_CDELOLDNEXT,"%d hours",delcycle);

      Set_Entry(widget,LAB_CDDELRUNNEXT,"next at %s.",pri_curtime(delcycle));

      delcycle*=3600;
//delcycle/=4; // 1 uur: 4x per uur checken
      to=gtk_timeout_add(delcycle*1000,(GtkFunction)dellistoldcont,widget);
    }
    else
    {
      Set_Widgetcolor(widget,'b',0xe0,0xe0,0xe0);
      if (to) gtk_timeout_remove(to); to=0;
    }
  }

  if (!strcmp(name,LAB_CRLOAD))
  {
    static int to=0;
    if ((Update_Togglelabel(widget)) && (!to))
    {
      Set_Entry(widget,LAB_RECINFO,"");
      for (dbase.grp_last=globgrp; 
           (dbase.grp_last) && (dbase.grp_last->next); 
           dbase.grp_last=dbase.grp_last->next);
           
      Set_Widgetcolor(widget,'b',0xff,0,0);   // button 'red'
      Set_Widgetcolor(Find_Widget(widget,AFTERREC_SET),'b',0,0,0xff); // butbox blue
//      Set_Widgetcolor(Find_Widget(widget,AFTERREC_DEL),'b',0,0,0xff); // butbox blue
      Set_Widgetcolor(Find_Widget(widget,SHOWLIVE),'b',0,0,0xff);     // butbox blue

      if (prefer.upd_cycle)
      {
        loadlistfunc(widget,TRUE,FALSE);  // do 1 run to move all data to 'done'
        to=gtk_timeout_add(prefer.upd_cycle*1000,(GtkFunction)loadlistcont,widget);
      }
    }
    else
    {
      Set_Widgetcolor(widget,'b',0xe0,0xe0,0xe0);
      Set_Widgetcolor(Find_Widget(widget,AFTERREC_SET),'b',0,0,0x00); // butbox black
//      Set_Widgetcolor(Find_Widget(widget,AFTERREC_DEL),'b',0,0,0x00); // butbox black
      Set_Widgetcolor(Find_Widget(widget,SHOWLIVE),'b',0,0,0x00);     // butbox black
      if (to) gtk_timeout_remove(to); to=0;
    }
  }

  if (!strcmp(name,LAB_REMOVECHAN))
  {
    GtkWidget *choosepart=Find_Widget(wnd,PART_REMOVECHAN);
    
    globinfo.keep_chanlist.use_list=Get_Button(widget,LAB_REMOVECHAN);
    globinfo.rm_raw=globinfo.keep_chanlist.use_list;
    /* set buttons of this window inactive if 'Remove' is deselected */
    gtk_widget_set_sensitive(choosepart,Get_Button(wnd,LAB_REMOVECHAN));

    if (globinfo.keep_chanlist.use_list)
      Set_Widgetcolor(Find_Widget(widget,AFTERREC_DEL),'b',0,0,0xff); // butbox blue
    else
      Set_Widgetcolor(Find_Widget(widget,AFTERREC_DEL),'b',0,0,0); // butbox blue
  }
  
  if (!strcmp(name,LAB_SHOWCHAN))
  {
    strcpy(globinfo.upd_chan,Get_Entry(widget,LAB_SHOWCHAN));
  }

#ifdef __GTK_20__
  {
    char *opt;
    if ((opt=soption_menu_get_history(Find_Widget(widget,LAB_SHOWSAT))))
    {
      strcpy(globinfo.upd_sat,opt);
      strcat(globinfo.upd_sat,"*");  // for e.g. MSG1_RSS
    }
  }
#else
  for (i=0; i<nrsatlist; i++)
  {
    if (!strcmp(name,satlist[i]))
    {
      strcpy(globinfo.upd_sat,name);
      strcat(globinfo.upd_sat,"*");  // for e.g. MSG1_RSS
    }
  }
#endif


  if (!strcmp(name,LAB_MAPCLR))
  {
    color_map_fixed(wnd);
  }
  
  
  if (!strcmp(name,LAB_MOVDONE))
  {
    if (Get_Button(widget,LAB_MOVDONE))
      globinfo.move_to_done=TRUE;
    else
      globinfo.move_to_done=FALSE;
  }
  if (!strcmp(name,LAB_SHOWNEW))
  {
    if (Get_Button(widget,LAB_SHOWNEW))
      globinfo.show_new=TRUE;
    else
      globinfo.show_new=FALSE;
  }
  if (!strcmp(name,LAB_SHOWCLR))
  {
    if (Get_Button(widget,LAB_SHOWCLR))
      globinfo.spm_live.compose=TRUE;
    else
      globinfo.spm_live.compose=FALSE;
  }  
  
  if (!strcmp(name,LAB_SHOWNEW))
    Sense_Button(widget,LAB_SHOWCLR,globinfo.show_new);
  
  if ((!strcmp(name,LAB_SHOWNEW)) || (!strcmp(name,LAB_SHOWCLR)))
  {
    Sense_Button(widget,LAB_SHOWCHAN,globinfo.show_new & !globinfo.spm_live.compose);
    Sense_Button(widget,LAB_SHOWSAT,globinfo.show_new);
    Sense_Button(widget,LAB_MAPCLR,globinfo.show_new & globinfo.spm_live.compose);
  }
  
  if (!strcmp(name,LAB_LGEN))
  {
    globinfo.gen_new=Get_Button(widget,LAB_LGEN);
//    Sense_Button(widget,LAB_GENMOV,globinfo.gen_new);
//    Sense_Button(widget,LAB_RUNCMD,globinfo.gen_new);

    /* Clear all 'new' flags if gen_new mode switched off */
    if (!globinfo.gen_new)
    {
      GROUP *grp;
      for (grp=globgrp; grp; grp=grp->next)
        grp->new=FALSE;
    }
  }


  if (!strcmp(name,LAB_GENMOV))
  {
    globinfo.gen_movienew=Get_Button(widget,LAB_GENMOV);
  }
}

#define LAB_MSGALL "  All  "
static void choose_chanfunc(GtkWidget *widget,gboolean data)
{
  char *name=(char *)data;
  int i;
  if (!strcmp(name,LAB_MSGALL))
  {
    gboolean ss=FALSE;
    for (i=0; i<nrchanlist; i++)
      if (!Get_Button(widget,channellist[i]))
        ss=TRUE;
    for (i=0; i<nrchanlist; i++)
      Set_Button(widget,channellist[i],ss);
  }
  
  for (i=0; i<nrchanlist; i++)
  {
    if (!strcmp(name,channellist[i]))
      globinfo.keep_chanlist.chan_sel[i]=Get_Button(widget,name);
  }

  for (i=0; i<nrsatlist; i++)
  {
    if (!strcmp(name,satlist[i]))
      globinfo.keep_chanlist.sat_sel[i]=Get_Button(widget,name);
  }
  if (!strcmp(name,LAB_UNKNOWN))
    globinfo.keep_chanlist.unk_sel=Get_Button(widget,name);
}

static GtkWidget *choose_channel()
{
  GtkWidget *w[10];
  int h,v,i;

/* Meteosat new generation satellites */
  v=0; h=0;
  w[0]=gtk_table_new(3,1,FALSE);
  for (i=0; i<nrsatlist; i++)
  {
    if (is_nomsg(satlist[i])) continue;
    
    w[1]=Create_Check(satlist[i],choose_chanfunc,globinfo.keep_chanlist.sat_sel[i]);
    gtk_table_attach(GTK_TABLE(w[0]),w[1],h,h+1,v,v+1,
                         GTK_FILL,GTK_FILL,1,1);
    h++;
    if (h>=6) { h=0; v++; }
  }

/* Meteosat old satellites */
  v=0; h=0;
  w[2]=gtk_table_new(3,1,FALSE);
  for (i=0; i<nrsatlist; i++)
  {
    if (!is_mfg(satlist[i])) continue;
    
    w[3]=Create_Check(satlist[i],choose_chanfunc,globinfo.keep_chanlist.sat_sel[i]);
    gtk_table_attach(GTK_TABLE(w[2]),w[3],h,h+1,v,v+1,
                         GTK_FILL,GTK_FILL,1,1);
    h++;
    if (h>=6) { h=0; v++; }
  }

/* Other satellites/sources */
  v=0; h=0;
  w[4]=gtk_table_new(3,1,FALSE);
  for (i=0; i<nrsatlist; i++)
  {
    if (!is_nomsg(satlist[i])) continue;
    if (is_mfg(satlist[i])) continue;

    w[5]=Create_Check(satlist[i],choose_chanfunc,globinfo.keep_chanlist.sat_sel[i]);
    gtk_table_attach(GTK_TABLE(w[4]),w[5],h,h+1,v,v+1,
                         GTK_FILL,GTK_FILL,1,1);
    h++;
    if (h>=4) { h=0; v++; }
  }
    
/* MSG channels */
  v=0; h=0;
  w[6]=gtk_table_new(3,1,FALSE);
  for (i=0; i<nrchanlist; i++)
  {
    w[7]=Create_Check(channellist[i],choose_chanfunc,globinfo.keep_chanlist.chan_sel[i]);
    gtk_table_attach(GTK_TABLE(w[6]),w[7],h,h+1,v,v+1,
                         GTK_FILL,GTK_FILL,1,1);
    h++;
    if (h>=4) { h=0; v++; }
  }
  h=4; v=0;
  w[7]=Create_Button(LAB_MSGALL,choose_chanfunc);
  gtk_table_attach(GTK_TABLE(w[6]),w[7],h,h+1,v,v+1,
                         GTK_FILL,GTK_FILL,1,1);
  h++;
  if (h>=4) { h=0; v++; }

/************************************************************************/  
  w[0]=Pack("Satellites MSG ",'h',w[0],1,NULL);           /* MSG list */
  w[6]=Pack("Channels",'h',w[6],1,NULL);                 /* channel list */
  w[0]=Pack("Meteosat New Generation",'v',w[0],1,w[6],1,NULL);
  Set_Widgetcolor(w[0],'b',0,0,0);

  w[2]=Pack("Satellites MFG ",'h',w[2],1,NULL);           /* MET list */
  Set_Widgetcolor(w[2],'b',0,0,0);

  w[4]=Pack("Other sources ",'h',w[4],1,NULL);    /* other list */
  Set_Widgetcolor(w[4],'b',0,0,0);
  w[9]=Create_Label("Delete all data just received EXCEPT ");

  w[9]=Pack(PART_REMOVECHAN,'h',w[9],1,NULL);
  w[0]=Pack(" ",'v',w[9],3,w[0],1,w[2],1,w[4],1,NULL);
  Set_Widgetcolor(w[0],'b',0x00,0,0x00);
  gtk_object_set_data(GTK_OBJECT(w[0]),WDGT_ID,(gpointer)AFTERREC_DEL);
  w[0]=Pack(NULL,'v',w[0],1,NULL);   // Needed for gtk_widget_set_sensitive...
  gtk_widget_show_all(w[0]);

  Sense_Button(w[0],PART_REMOVECHAN,FALSE);
  return w[0];
}

GtkWidget *nb_record()
{
  GtkWidget *w[10],*wa,*wb,*wc,*wd,*we,*wf,*wg,*wh,*wz;

/* ----- update ----- */
  w[1]=Create_Button(LAB_UPDATE,do_recordbut);
  w[2]=Create_Toggle(LAB_CRLOAD,do_recordbut,FALSE);
  
  w[3]=Create_Entry(LAB_NUPDATE,NULL,"  ");
  w[4]=Create_Check(LAB_NOTIFY,do_recordbut,globinfo.notify);
  w[1]=Pack(NULL,'v',w[1],2,w[2],2,NULL);
  w[3]=Pack(NULL,'v',w[3],1,w[4],1,NULL);
  wa=Pack("update   ",'h',w[1],2,w[3],2,NULL);
  Set_Widgetcolor(wa,'b',0x00,0,0);

/* ----- Show live ----- */
  w[1]=Create_Check(LAB_SHOWNEW,do_recordbut,globinfo.show_new);
  w[2]=Create_Check(LAB_SHOWCLR,do_recordbut,globinfo.spm_live.compose);
  w[1]=Pack(NULL,'v',w[1],2,w[2],2,NULL);

/* Don't add NOAA/METOP for showing live (and 'SERVICE') (last 4 of list) */
  w[3]=Create_Optionmenu(LAB_SHOWSAT,do_recordbut,0,0);
  Add_Options(w[3],LAB_SHOWSAT,do_recordbut,0,satlist,nrsatlist-1-2,0);

  w[4]=Create_Entry(LAB_SHOWCHAN,do_recordbut,"%-6s",globinfo.upd_chan);
  w[5]=Create_Button(LAB_MAPCLR,do_recordbut);
  w[3]=Pack("",'v',w[3],2,w[4],2,w[5],2,NULL);

  wb=Pack("Show 'Live'",'h',w[1],0,w[3],0,NULL);
  Sense_Button(wb,LAB_MAPCLR,FALSE);

  Set_Widgetcolor(wb,'b',0x00,0,0);
  gtk_object_set_data(GTK_OBJECT(wb),WDGT_ID,(gpointer)SHOWLIVE);


/* update and show */
  wa=SPack(" ","h",wa,"3",wb,"3",NULL);
  Set_Widgetcolor(wa,'b',0x00,0,0);
/* delete old */
  w[1]=Create_Toggle(LAB_CDELOLD,do_recordbut,FALSE);  
  w[2]=Create_Entry(LAB_CDELOLDNEXT,NULL,"%d hours",(prefer.deldaymin[0]? 24 : 1));
  w[3]=Create_Entry(LAB_CDDELRUNNEXT,NULL,"%12s","");
  w[4]=Create_Label("See\nPreferences.");
//  w[1]=Pack(NULL,'h',w[1],1,NULL);
  wh=Pack(NULL,'v',w[1],1,w[2],1,w[3],1,NULL);
  wh=Pack("Delete",'h',wh,1,w[4],1,NULL);
  Set_Widgetcolor(wh,'b',0x00,0,0);
  wh=Pack(NULL,'h',wh,1,NULL);

/* ----- Info ----- */
  w[1]=Create_Label("Record info");
  w[2]=Create_Entry(LAB_RECINFO,NULL,"   ");
  we=SPack("","h",w[1],"1",w[2],"ef1",NULL);

/* ----- Generate live ----- */
  wc=Create_ButtonArray("Generate",do_recordbut,3,
                         CHECK,LAB_LGEN,
                         CHECK,LAB_RUNCMD,
                         CHECK,LAB_TIMEID,
                         CHECK,LAB_GENMOV,
                         CHECK,LAB_ESSENTIAL,
                         0);
/*

  w[4]=Create_Check(LAB_LGEN,do_recordbut,FALSE);  
  w[5]=Create_Check(LAB_GENMOV,do_recordbut,FALSE);
//  w[5]=Pack("",'h',w[5],2,NULL);
  w[4]=Pack(NULL,'v',w[4],1,w[5],1,NULL);

  w[6]=Create_Check(LAB_TIMEID,do_recordbut,FALSE);
  w[7]=Create_Check(LAB_RUNCMD,do_recordbut,FALSE);

  w[6]=Pack(NULL,'v',w[6],1,w[7],1,NULL);

  wc=Pack("Generate",'h',w[4],2,w[6],2,NULL);
*/
  Set_Widgetcolor(wc,'b',0x00,0,0);
//  Sense_Button(wc,LAB_GENMOV,FALSE);
//  Sense_Button(wc,LAB_RUNCMD,FALSE);

/* ----- Remove live ----- */
  w[1]=Create_Check(LAB_REMOVECHAN,do_recordbut,FALSE);
  w[2]=Create_Check(LAB_UNKNOWN,choose_chanfunc,globinfo.keep_chanlist.unk_sel);
  w[1]=Pack("Raw data:",'v',w[1],1,w[2],1,NULL);
  Set_Widgetcolor(w[1],'b',0x00,0,0);

  wd=Pack("After record",'h',w[1],2,wc,2,NULL);
  Set_Widgetcolor(wd,'b',0,0,0);
  gtk_object_set_data(GTK_OBJECT(wd),WDGT_ID,(gpointer)AFTERREC_SET);




  wf=SPack(LAB_RECEIVE,"v",wd,"5",wa,"5",wh,"5",we,"EF1",NULL);
  Set_Widgetcolor(wf,'b',0xff,0,0);
/* ---------- Tab "delete during receiving" ---------- */

  wg=choose_channel();

  wz=Create_Notebook(NULL,GTK_POS_TOP,
                     "Control"            ,wf,
                     "Items to delete" ,wg,
                     NULL);


  wz=Pack(RECORD_TAB,'v',wz,1,NULL);
  gtk_widget_show_all(wz);

  return wz;
}
