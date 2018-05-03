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

extern GLOBINFO globinfo;
/***********************************************
 * Color selection
 ***********************************************/
static char listchan[MAXCHAN][20];

#define LENLISTCHAN 20
int in_list(char listchan[MAXCHAN][LENLISTCHAN],int ,char *);


/***********************************************
 * Channel selection
 ***********************************************/
static GtkWidget *windowi;
static char igen;
static GROUP *igrp;
extern PREFER prefer;
extern GLOBINFO globinfo;
extern GROUP *globgrp;
static int globnrlistchan;
float size_files_Chan();

#define LAB_FILM "Movie"
#define LAB_TMSTR "Time id"
#define LAB_SEGMRANGE "Segments"
#define LAB_SEGMSKINC "Skip incomplete"
#define LAB_DOIT "Do it"
#define LAB_ABORT "Abort"
#define LAB_DISMISS "Dismiss"
#define LAB_ALLMSG "Select all/Deselect all"
#define LAB_ALL "Selected only/Everything"
#define CHANLIST "Selected channels"
#define WPACK "!packlijst"
#define LAB_HELP "Help"
#define LAB_FSIZE "Size selected files"
#define CLRMEANING "Meaning of colours"
GtkWidget *mklegend()
{
  GtkWidget *w[10];
  w[1]=Create_Led("Not processed yet",0xf00);
  w[2]=Create_Led("Busy",0xff0);
  w[3]=Create_Led("Done; OK",0x0f0);
  w[4]=Create_Led("Done but missing segment(s)",0x0a0);
  w[5]=Create_Led("Aborted (missing segment(s))",0x00f);
  w[6]=Create_Led("File exist; export skipped (movie: reused)",0x0bb);
  w[0]=Pack(CLRMEANING,'v',w[1],1,w[2],1,w[3],1,w[4],1,w[5],1,w[6],1,NULL);
  return w[0];
}

static GtkWidget *mklijst(GROUP *grp,gboolean start,GtkWidget *widget,char gen)
{
  GtkWidget *wpack;
  GtkWidget *wlist;
  GtkWidget *wled;
  GROUP *grp_sel;
  float fsize=0;
  gboolean sav_compose=FALSE;
  GENMODES gmode;
  globinfo.nr_selpics=0;
  globinfo.tot_size=0;
  memset(&gmode,0,sizeof(gmode));
  gmode.otype=gen;
  
  if ((wlist=Find_Widget(widget,CHANLIST))) gtk_widget_destroy(wlist->parent);
  wlist = Pack(CHANLIST,'v',NULL);
  if (gen=='m')
  {
    sav_compose=globinfo.spm.compose;
    globinfo.spm.compose=TRUE;
  }

  get_selected_item_rng(grp,NULL,TRUE);
  while ((grp_sel=get_selected_item_rng(NULL,&gmode,TRUE)))
  {
    if (grp_sel->chan)
    {
      pic_info(grp_sel,widget,NULL);
      /* Get # segments of first pic, */

/* Select individual chanels:
     - NOT if composed (-> combine channels to false colour pic)
     - NOT if group(s) of AVHRR are selected (-> rah format generated, containing all channels) 
*/
      if ((grp_sel->compose) || ((grp_sel->nselected) && (gen=='f') && (strchr("GAM",grp_sel->h_or_l))))
      {
        if (gen=='m')
        {
          CHANNEL *chan;
          int nr_chansel=0;
          for (chan=grp_sel->chan; chan; chan=chan->next)
          {
            if (start)
            {
              if (chan->chan_name)
              {
                if ((globnrlistchan<MAXCHAN) && (!in_list(listchan, globnrlistchan,chan->chan_name)))
                  strcpy(listchan[globnrlistchan++],chan->chan_name);
              }
            }
            else if (Get_Button(widget,chan->chan_name))
            {
              if ((!strchr("GAM",grp_sel->h_or_l)) || (!nr_chansel))
              {
                nr_chansel++;
                fsize+=size_files_Chan(chan);
              }
              wled=Create_Led(chan->id,0xf00); /* normal */
              globinfo.nr_selpics++;
//              globinfo.tot_size+=(grp_sel->pc.width*grp_sel->pc.height/1024);
              Add_Widget(wlist,wled,CHANLIST,NULL);
            }
          }
        }
        else
        {
          wled=Create_Led(grp_sel->id,0xf00);
          globinfo.nr_selpics++;
          globinfo.tot_size+=(grp_sel->pc.width*grp_sel->pc.height*3/1024);
          Add_Widget(wlist,wled,CHANLIST,NULL);
        }
      }
      else
      {
        if (start)
        {
          if ((grp_sel->chan) && (grp_sel->chan->chan_name))
          {
            if ((globnrlistchan<MAXCHAN) && (!in_list(listchan, globnrlistchan,grp_sel->chan->chan_name)))
              strcpy(listchan[globnrlistchan++],grp_sel->chan->chan_name);
          }
        }
        else if (Get_Button(widget,grp_sel->chan->chan_name))
        {
          fsize+=size_files_Chan(grp_sel->chan);
          wled=Create_Led(grp_sel->chan->id,0xf00); /* normal */
          globinfo.nr_selpics++;
          globinfo.tot_size+=(grp_sel->pc.width*grp_sel->pc.height/1024);
          Add_Widget(wlist,wled,CHANLIST,NULL);
        }
      }
    }
    Remove_Grps(grp_sel);
  }

  if ((!widget) || (!(wpack=Find_Widget(widget,WPACK))))
    wpack=SPack(WPACK,"vs",wlist,"1",NULL);
  else
    Add_Widget(wpack,wlist,WPACK,NULL);

  if (gen=='m')
  {
    globinfo.spm.compose=sav_compose;
    Set_Entry(widget,LAB_FSIZE,"%.3f %s",
         (fsize>1000000000? fsize/1000000000:
          fsize>1000000   ?fsize/1000000:
          fsize>1000      ?fsize/1000:
          fsize),
         (fsize>1000000000? "Gb":
          fsize>1000000?    "Mb":
          fsize>1000?       "kb":
                            "bytes"));

  }
  gtk_widget_show(wlist);
  gtk_widget_show_all(wpack);

  return wpack;
}
float show_info_sel();
static gboolean trans_busy;
#if __GTK_WIN32__ == 1
#define SCRIPTFILE "msg_raw_do.bat"
void gen_script(GtkWidget *widget,GROUP *grp)
{
  FILE *fp=stdout;
  char fn[1000];
  GtkWidget *list_wnd=Find_Window(widget,LAB_PROGRESS_TRANS);
  GROUP *grp_sel;
  CHANNEL *chan;
  gboolean sav_compose=FALSE;
  sav_compose=globinfo.spm.compose;
  globinfo.spm.compose=TRUE;
  
  sprintf(fn,"%s%s",globinfo.dest_dir,SCRIPTFILE);
  fp=fopen(fn,"w");
  fprintf(fp,"@echo off\n");
  fprintf(fp,"set files=\n");

  get_selected_item_rng(grp,NULL,TRUE);
  while ((grp_sel=get_selected_item_rng(NULL,NULL,TRUE)))
  {
    if (grp_sel->pro_epi)
    {
      SEGMENT *segm;
      for (segm=grp_sel->pro_epi->pro; segm; segm=segm->next)
        fprintf(fp,"set files=%%files%% \"%s\"\n",segm->pfn);
    }
    for (chan=grp_sel->chan; chan; chan=chan->next)
    {
      /* Get # segments of first pic, */

      if (Get_Button(list_wnd,chan->chan_name))
      {
        SEGMENT *segm;
        for (segm=chan->segm; segm; segm=segm->next)
          fprintf(fp,"set files=%%files%% \"%s\"\n",segm->pfn);
      }
    }
    Remove_Grps(grp_sel);
  }
  fprintf(fp,"\n");
  fprintf(fp,"for %%%%i in ( %%files%% ) do ");
  fprintf(fp,"  dir %%%%i\n");

  fclose(fp);
  globinfo.spm.compose=sav_compose;
  Set_Entry(widget,LAB_INFO,"Created script %s.",SCRIPTFILE);
}
#else
#define SCRIPTFILE "msg_raw_do"
void gen_script(GtkWidget *widget,GROUP *grp)
{
  FILE *fp=stdout;
  char fn[1000];
  GtkWidget *list_wnd=Find_Window(widget,LAB_PROGRESS_TRANS);
  GROUP *grp_sel;
  CHANNEL *chan;
  gboolean sav_compose=FALSE;
  sav_compose=globinfo.spm.compose;
  globinfo.spm.compose=TRUE;

  sprintf(fn,"%s%s",globinfo.dest_dir,SCRIPTFILE);
  fp=fopen(fn,"w");
  fprintf(fp,"files=\"\\\n");

  get_selected_item_rng(grp,NULL,TRUE);
  while ((grp_sel=get_selected_item_rng(NULL,NULL,TRUE)))
  {
    if (grp_sel->pro_epi)
    {
      SEGMENT *segm;
      for (segm=grp_sel->pro_epi->pro; segm; segm=segm->next)
        fprintf(fp,"  %s \\\n",segm->pfn);
    }
    for (chan=grp_sel->chan; chan; chan=chan->next)
    {
      /* Get # segments of first pic, */

      if (Get_Button(list_wnd,chan->chan_name))
      {
        SEGMENT *segm;
        for (segm=chan->segm; segm; segm=segm->next)
          fprintf(fp,"  %s \\\n",segm->pfn);
      }
    }
    Remove_Grps(grp_sel);
  }
  fprintf(fp,"  \"\n");
  fprintf(fp,"for i in $files; do\n");
  fprintf(fp,"  ls -l $i\n");
  fprintf(fp,"done\n");

  fclose(fp);
  globinfo.spm.compose=sav_compose;
  Set_Entry(widget,LAB_INFO,"Created script %s.",SCRIPTFILE);
}
#endif

#ifdef XXX
static GROUP *check_size(GROUP *grp,RGBPICINFO *rgbpi,GENMODES *mode,GtkWidget *window,int *x,int *y)
{
  GROUP *grp_sel;
  get_selected_item(grp,mode);                    /* Init selection */
  while ((grp_sel=get_selected_item(NULL,mode)))  /* Get next selected image */
  {
    if ((grp_sel->chan) && (chan_selected(window,grp_sel->chan->chan_name)))
    {
      pic_info(grp_sel,NULL,NULL);
      calc_size(grp_sel,x,y,NULL);
      if ((rgbpi->pwidth!=*x) || (rgbpi->pheight!=*y))
        return grp_sel;
    }
  }
  return NULL;
}
#endif

#include "preview.h"

void piclistfunc(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  char *opt;
  GtkWidget *window=Find_Window(Find_Parent_Window(widget),LAB_PROGRESS_TRANS);
  GtkWidget *main_window=Find_Window(window,LAB_MAIN);
  GtkWidget *pwnd=Find_Window(main_window,PREV_WNDNAME);

  GENMODES genmode;
  int i;
  memset(&genmode,0,sizeof(genmode));
  get_genmode_from_gui(main_window,&genmode);
  genmode.area_nr=globinfo.area_nr;
  genmode.otype=igen;

/* These genmodes are from popup-window. */
  genmode.gen_film=Get_Button(window,LAB_FILM);
  genmode.timeid=Get_Button(window,LAB_TMSTR);

  genmode.skip_incomplete=Get_Button(window,LAB_SEGMSKINC);

  opt=soption_menu_get_history(Find_Widget(widget,LAB_FFTP));

  if (!strcmp(name,LAB_HELP))
  {
    if (Get_Button(window,LAB_HELP))
      gtk_widget_show((Find_Widget(window,CLRMEANING))->parent);
    else
      gtk_widget_hide((Find_Widget(window,CLRMEANING))->parent);
  }
  else if (!strcmp(name,LAB_DISMISS))
  {
    Close_Window(window);
  }
  else if (!strcmp(name,LAB_DOIT))
  {
    genmode.agl.shift_3d=globinfo.shift_3d;
    genmode.agl.lmax=(1<<(globinfo.bitshift+8))-1;
    genmode.agl.lmin=0;
    genmode.ol_lum=0xff;
    
    trans_busy=TRUE;            /* block change of list during translation */
//!! weg    genmode.segm_range=Get_Entry(window,LAB_SEGMRANGE);
    if (igen=='m')
    {
      gen_script(windowi,igrp);
    }
    else
    {
      if ((pwnd) && (igen=='f'))
      {
        DRAW_INFO *di=gtk_object_get_data((gpointer)pwnd,"Draw_Info");
        RGBPICINFO *rgbpi=NULL;
        if (di)
          rgbpi=gtk_object_get_data(GTK_OBJECT(di->drawable),RGBPI_DL);
        if (rgbpi)
        {
          genmode.zx=MAX(rgbpi->zx,1.);
          genmode.ox=rgbpi->ox;
          genmode.zy=MAX(rgbpi->zy,1.);
          genmode.oy=rgbpi->oy;
          genmode.adapt_lum=TRUE;
          genmode.lmin=rgbpi->lmin;
          genmode.lmax=rgbpi->lmax;
          genmode.agl.shift_3d=Get_Adjust(pwnd,LAB_SHFT3D_PREV);
          genmode.agl.lmin=Get_Adjust(pwnd,LAB_LMIN3D_PREV);
          genmode.agl.lmax=Get_Adjust(pwnd,LAB_LMAX3D_PREV);
          genmode.agl.depthchan=Get_Button(pwnd,LAB_DCHAN_PREV);
          genmode.agl.ol_3d=Get_Button(pwnd,LAB_OL3D_PREV);
          genmode.ol_lum=Get_Adjust(pwnd,LAB_OLADD_PREV);
#ifdef XXX
werkt niet goed: pic_info toegevoegd om upper_x_shift enz. te gbruiken,
maar hier is nog geen kanaal geselecteerd:
    if ((!chan->r) && (!chan->g) && (!chan->b) && (!chan->lum)) continue; // not visible

pic_info(igrp,NULL,&genmode);

          if (check_size(igrp,rgbpi,&genmode,window,&x,&y))
          {
            int n=Create_Choice("WARNING",2,"Yes","No, abort",
"Size of at least one picture to generate (%dx%d)\n"\
"  not equal to size of picture in preview window (%dx%d)!\n"\
"  This will give unexpected results.\n"\
"  Proceed anyway?",x,y,rgbpi->pwidth,rgbpi->pheight);
             if (n==2) return;
          }
#endif
        }
      }
      gen_item(windowi,igrp,&genmode,&prefer);
    }
    genmode.segm_range=NULL;
    trans_busy=FALSE;
  }
  else if (!strcmp(name,LAB_ALLMSG))
  {
    gboolean set;
    Update_Togglelabel(widget);
    set=Get_Button(widget,LAB_ALLMSG);
    for (i=0; i<globnrlistchan; i++)
      Set_Button(widget,listchan[i],set);
    mklijst(globgrp,FALSE,widget,igen);
  }
  else if (!strcmp(name,LAB_ALL))
  {
    if (Update_Togglelabel(widget))
    {
      Sense_Button(widget,LAB_ALLMSG,FALSE);
    }
    else
    {
      Sense_Button(widget,LAB_ALLMSG,TRUE);
    }
  }
  else if (!strcmp(name,LAB_FILM))
  {
  }
  else if (!strcmp(name,PREV_WNDNAME))
  {
    GENMODES genmode2=genmode;
    genmode2.otype='v';
    genmode2.do_one_pic=TRUE;
    gen_item(windowi,igrp,&genmode2,&prefer);
  }
  else if ((opt) && (!strcmp(name,opt)))
  {
    if (!strcmp(name,LAB_FFTDEF))
    {
      Set_Optionsmenu(main_window,LAB_FFT,0);
    }
    else if (!strcmp(name,LAB_PGM))
    {
      Set_Optionsmenu(main_window,LAB_FFT,1);
    }
    else if (!strcmp(name,LAB_PGM8))
    {
      Set_Optionsmenu(main_window,LAB_FFT,2);
    }
    else if (!strcmp(name,LAB_JPG))
    {
      Set_Optionsmenu(main_window,LAB_FFT,3);
    }
    else if (!strcmp(name,LAB_CJPG))
    {
      Set_Optionsmenu(main_window,LAB_FFT,4);
    }
  }
  else
  {
    mklijst(globgrp,FALSE,widget,igen);
  }
    
  if ((!GTK_IS_ADJUSTMENT(widget)) && (strcmp(name,LAB_SEGMRANGE)))
  {
    if (globnrlistchan)
    {
      Sense_Button(widget,LAB_DOIT,FALSE);
      for (i=0; i<globnrlistchan; i++)
      {
        if (Get_Button(widget,listchan[i]))
          Sense_Button(widget,LAB_DOIT,TRUE);
      }
    }
  }

}

#define LAB_TRANSGEN_FL "Files"
#define LAB_TRANSGEN_PV "Previews"
#define LAB_TRANSGEN_MR "Raw file mngr"

/* Create pop-up for channel selection */
GtkWidget *create_piclist(GtkWidget *window,GROUP *grp,char gen)
{
  GtkWidget *wnd,*w[12];
  int h=0,v=0,i;
  static GUISTATE *gs;
  static char sav_wndmode;
  char *action;
  if (trans_busy) return NULL;          /* don't recreate if busy */

  if (gen) sav_wndmode=gen; else gen=sav_wndmode;
  windowi=window;
  igrp=grp;
  igen=gen;
  globnrlistchan=0;

  if ((wnd=Find_Window(window,LAB_PROGRESS_TRANS)))
  {
    if (gs) Remove_Guistate(gs);
    gs=store_guistate(NULL,wnd,NULL);
    Close_Window(wnd);
  }
  wnd=Create_Window(window,10,300,LAB_PROGRESS_TRANS,NULL);

  w[1]=mklijst(grp,TRUE,window,igen);

  if (gen=='m')
  {
    w[4]=Create_Entry(LAB_FSIZE,NULL,"%7s","");
    w[2]=SPack(NULL,"h",w[4],"1",NULL);
    w[5]=NULL;
    action="Generate script...";
  }
  else if (gen=='f')
  {
    int state_frmt=Get_Optionsmenu(window,LAB_FFT);
    w[4]=Create_Toggle(LAB_HELP,piclistfunc,FALSE);
   
    w[2]=Create_Optionmenu(LAB_FFTP,piclistfunc,state_frmt,
                                LAB_FFTDEF,LAB_PGM,LAB_PGM8,LAB_JPG,LAB_CJPG,0);
    w[3]=Create_Check(LAB_TMSTR,piclistfunc,FALSE);
    w[2]=SPack(NULL,"h",w[2],"2",w[3],"2",w[4],"E2",NULL);

    w[5]=Create_Check(LAB_FILM,piclistfunc,FALSE);
   /* See overwrite()! */
    w[6]=Create_Optionmenu(LAB_FLMTMP,piclistfunc,2,LAB_TMPRM,LAB_TMPRPL,LAB_TMPREUSE,0);
    w[7]=Create_Button(PREV_WNDNAME,piclistfunc);
    w[5]=Pack("",'h',w[5],1,w[6],1,w[7],1,NULL);
    Set_Widgetcolor(w[5],'b',0,0,0xff);
    action="Generate file...";
  }
  else
  {
    w[4]=Create_Toggle(LAB_HELP,piclistfunc,FALSE);
    w[2]=SPack(NULL,"h",w[4],"E1",NULL);
    w[5]=NULL;
    action="Preview...";
  }
      
    
  w[2]=Pack(NULL,'v',w[2],1,w[5],1,NULL);

  w[3]=Create_Button(LAB_DOIT,piclistfunc);
  w[5]=Create_Button(LAB_DISMISS,piclistfunc);
  w[3]=SPack(action,"h",w[3],"ef1",w[5],"ef1",NULL);
  Set_Widgetcolor(w[3],'b',0,0xff,0);
  w[6]=gtk_table_new(4,1,FALSE);
  
  for (i=0; i<globnrlistchan; i++)
  {
    w[7]=Create_Check(listchan[i],piclistfunc,FALSE);
    gtk_table_attach(GTK_TABLE(w[6]),w[7],h,h+1,v,v+1,
                         GTK_FILL,GTK_FILL,1,1);
    h++;
    if (h>=4) { h=0; v++; }
  }
  gtk_widget_show(w[6]);

  w[9]=w[10]=NULL;
  if (gen!='m')
  {
//WEG!   w[9]=Create_Entry(LAB_SEGMRANGE,piclistfunc,"%3s","");
    w[10]=Create_Check(LAB_SEGMSKINC,piclistfunc,TRUE);
  }
  if (globnrlistchan)
  {
    w[8]=Create_Toggle(LAB_ALLMSG,piclistfunc,FALSE);
// weg!    w[8]=SPack(NULL,"h",w[8],"ef1",w[9],"4",w[10],"1",NULL);
    w[8]=SPack(NULL,"h",w[8],"ef1",w[10],"1",NULL);
  }
  else
  {
// weg!     w[8]=SPack(NULL,"h",w[9],"1",w[10],"1",NULL);
    w[8]=SPack(NULL,"h",w[10],"1",NULL);
  }

  if (w[1])
  {
    if (w[8])
      w[0]=SPack(NULL,"v",w[2],"1",w[6],"1",w[8],"1",w[1],"ef1",w[3],"1",NULL);
    else
      w[0]=SPack(NULL,"v",w[2],"1",w[6],"1",w[1],"ef1",w[3],"1",NULL);
  }
  else
  {
    w[0]=SPack(NULL,"v",w[2],"1",w[3],"1",NULL);
  }
  
  w[0]=SPack(NULL,"v",w[0],"ef1",mklegend(),"1",NULL);

  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show_all(wnd);
  place_window(wnd,0,0,right_top);
  if (globnrlistchan) Sense_Button(wnd,LAB_DOIT,FALSE);
  if (gs)
  {
    restore_guistate(wnd,gs);    /* ! no restore of chan. buttons! */
  }
  else
  {
    restore_guistate_fromfile(wnd,prefer.gui_inifile);
  }

  gtk_window_set_title((GtkWindow *)wnd,
      (gen=='f'? LAB_TRANSGEN_FL : gen=='m'? LAB_TRANSGEN_MR: LAB_TRANSGEN_PV));
  
  gtk_widget_hide((Find_Widget(wnd,CLRMEANING))->parent);

  return wnd;
}

