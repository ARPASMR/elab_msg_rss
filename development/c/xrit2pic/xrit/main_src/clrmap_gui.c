/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
extern GLOBINFO globinfo;
extern char *channellist[];

#define LENLISTCHAN 20

#define CHANMAPLIST "Channel mapping"
#define CHANMAPVALS "Channel values"
#define WPACK "!packlijst"
#define LAB_MIR "Mirror neg. values"


#ifndef __NOGUI__
static void func_but(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_MIR))
  {
    globinfo.mirror_negval=Get_Button(widget,LAB_MIR);
  }
}


/* Create part to change contr. of each channel */
static GtkWidget *mkclrmaplijst(GtkWidget *widget,char listchan[MAXCHAN][LENLISTCHAN],int nrlistchan)
{
  GtkWidget *wpack,*wtbl,*wlab,*wmir;
  GtkWidget *wlist;
  GtkWidget *wspin=NULL;
  int i,j;
  static GUISTATE *gs;
  int n=0;
  if (gs) Remove_Guistate(gs); gs=NULL;
  wlist=Find_Widget(widget,CHANMAPVALS);
  if (wlist)
  {
    gs=store_guistate(NULL,wlist,NULL);
    gtk_widget_destroy(wlist->parent);
  }
  
  wtbl=gtk_table_new(0,0,FALSE);
  wmir=Create_Check(LAB_MIR,func_but,FALSE);
  wlist = Pack(CHANMAPVALS,'v',wmir,1,wtbl,1,NULL);
  Set_Widgetcolor(wlist,'b',0,0,0xff);

  for (j=0; j<3; j++)
  {
    wlab=Create_Label(j==1? "Green":j==2? "Blue" : "Red");
    gtk_table_attach(GTK_TABLE(wtbl),wlab,1+j,2+j,n,n+1,GTK_FILL,GTK_FILL,1,1);
  }
  n++;
  for (i=0; i<nrlistchan; i++)
  {
    char tmp[50];
    wlab=0;
    for (j=0; j<3; j++)
    {
      sprintf(tmp,"!%c_%s",(j==1? 'G' : j==2? 'B' :'R'),listchan[i]);
      if (Get_Button(widget,tmp))
      {
        sprintf(tmp,"!%s %c",listchan[i],(j==1? 'G' : j==2? 'B' :'R'));
        if (i==0)    // Offset: start with 1%
        {
          wspin=Create_Spin(tmp,NULL,"%3d%3d%3d",1,-1000000,1000000);
        }
        else if (i==1)    // Gamma
        {
          wspin=Create_Spin(tmp,NULL,"%3.1f%3.1f%3.1f",1.,.3,3.);
          }
        else         // channels: start with 100%
          wspin=Create_Spin(tmp,NULL,"%3d%3d%3d",100,-1000000,1000000);

        if (!wlab)
        {
          wlab=Create_Label(listchan[i]);
          gtk_table_attach(GTK_TABLE(wtbl),wlab,0,1,n,n+1,GTK_FILL,GTK_FILL,1,1);
        }
        gtk_table_attach(GTK_TABLE(wtbl),wspin,1+j,2+j,n,n+1,GTK_FILL,GTK_FILL,1,1);
      }
    }
    n++;
  }
  
  if ((!widget) || (!(wpack=Find_Widget(widget,WPACK))))
    wpack=SPack(WPACK,"v",wlist,"1",NULL);   // "vs" --> scrolled
  else
    Add_Widget(wpack,wlist,WPACK,NULL);

  gtk_widget_show(wlist);
  gtk_widget_show_all(wpack);
  if (wlist) restore_guistate(wlist,gs);

  return wpack;
}

static char listchan1[MAXCHAN][LENLISTCHAN];
static int nrlistchan1;
static void clrmapfunc(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_CLOUDS))
  {
     Set_Button(widget,"!R_IR_039",TRUE);
     Set_Button(widget,"!G_IR_039",TRUE);
     Set_Button(widget,"!B_IR_039",TRUE);
  }
  else
    mkclrmaplijst(widget,listchan1,nrlistchan1);
}

static GtkWidget *create_mapbuttonarray(char listchan[MAXCHAN][LENLISTCHAN],int nrlistchan,void func())
{
  GtkWidget *w[5];
  int i,j,nrow;
  
  w[1]=gtk_table_new(0,0,FALSE);
  for (j=0; j<5; j++)               /* name / red / green / blue / invert */
  {
/* header */
    w[2]=Create_Label("%s",(j==0? "" : j==1? "Red    " : j==2? "Green  " : j==3? "Blue   ":"Invert"));
    gtk_table_attach(GTK_TABLE(w[1]),w[2],j,j+1,0,1,GTK_FILL,GTK_FILL,1,1);


/* all selected channels */
    nrow=0;
    for (i=0; i<nrlistchan; i++)   /* add 1 row per channel */
    {
      char tmp[50]; *tmp=0;
      if (!*listchan[i]) continue;
      if (j==0)
      {
        w[2]=Create_Label(listchan[i]);
      }
      else if ((j==4) && (i<=1))
      {
        w[2]=Create_Label("");
      }
      else
      {
        sprintf(tmp,"!%c_%s",(j==1? 'R' : j==2? 'G' :j==3? 'B' : 'I'),listchan[i]);
        w[2]=Create_Check(tmp,func,FALSE);
      }
      gtk_table_attach(GTK_TABLE(w[1]),w[2],j,j+1,nrow+2,nrow+3,GTK_FILL,GTK_FILL,1,1);
      nrow++;
    }
  }
  w[1]=Pack(CHANMAPLIST,'v',w[1],1,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0xff);
  return w[1];
}

int in_list(char listchan[MAXCHAN][20],int nrlistchan,char *chan)
{
  int i;
  for (i=0; i<nrlistchan; i++)
  {
    if (!strcmp(listchan[i],chan)) return 1;
  }
  return 0;
}

static int create_list_available_channels(GROUP *grp,char listchan[MAXCHAN][LENLISTCHAN],gboolean add_extras)
{
  GROUP *grp_sel;
  CHANNEL *chan;
  int nrlistchan=0;
  if (add_extras)
  {
    sprintf(listchan[nrlistchan++],"%s","Offset");
    sprintf(listchan[nrlistchan++],"%s","Gamma");
  }
  else
  {
    sprintf(listchan[nrlistchan++],"%s","");
  }
  
  get_selected_item(grp,NULL);
  while ((grp_sel=get_selected_item(NULL,NULL)))
  {
    for (chan=grp_sel->chan; chan; chan=chan->next)
    {
      if (!chan->chan_name) continue;
      if ((nrlistchan<MAXCHAN) && (!in_list(listchan, nrlistchan,chan->chan_name)))
        sprintf(listchan[nrlistchan++],"%s",chan->chan_name);
    }
    Remove_Grps(grp_sel);
  }
  return nrlistchan;
}

 /* Create pop-up for color selection (elaborated) */
void color_map(GtkWidget *window,GROUP *grp)
{
  GtkWidget *wnd,*w[10];
  static GtkWidget *wcnt;
  if (!(wnd=Find_Window(window,LAB_COLOR)))       // window doesn't exits =>
  {                                               //   create it
    wnd=Create_Window(window,0,0,LAB_COLOR,NULL);
    gtk_widget_show(wnd);
    place_window(wnd,0,0,right_top);
    gtk_window_set_policy((GtkWindow *)wnd,FALSE,FALSE,TRUE);
  }
  else if (wcnt)                                 // window exists =>
  {                                              //   remove content of window
    gtk_widget_destroy(wcnt);
  }

  /* Create (new) buttons */
  nrlistchan1=create_list_available_channels(grp,listchan1,TRUE);
  w[1]=create_mapbuttonarray(listchan1,nrlistchan1,clrmapfunc);
  w[2]=mkclrmaplijst(window,listchan1,nrlistchan1);
//  w[3]=Create_Toggle(LAB_CLOUDS,clrmapfunc,FALSE);
  wcnt=SPack(NULL,"v",w[1],"1",w[2],"ef1",NULL);
  gtk_container_add(GTK_CONTAINER(wnd),wcnt);
  gtk_widget_show_all(wnd);

/* Set mapping defined in globinfo */
  set_mapping(wcnt,globinfo.offset,globinfo.gamma,globinfo.chanmap);

  return;
}

/*************************************************************************/
static int create_list_fixed_channels(char listchan[MAXCHAN][LENLISTCHAN])
{
  int i=0;
  strcpy(listchan[i++],"");
  for (i=0; i<nrchanlist; i++) strcpy(listchan[i+1],channellist[i]);
  return nrchanlist;
}


 /* Create pop-up for color selection (simple) */
void color_map_fixed(GtkWidget *window)
{
  GtkWidget *wnd,*w[10];
  char listchanfix[MAXCHAN][LENLISTCHAN];
  int nrlistchanfix;
 
  if ((wnd=Find_Window(window,LAB_COLORFIX)))
  {
    return;
  }
  wnd=Create_Window(window,1,250,LAB_COLORFIX,NULL);

  nrlistchanfix=create_list_fixed_channels(listchanfix);
  
  w[1]=create_mapbuttonarray(listchanfix,nrlistchanfix,NULL);
  w[0]=SPack(NULL,"v",w[1],"1",NULL);

/* Set default mapping */
  {
    char tmp[50];
    CHANMAP *cm;
    for (cm=globinfo.chanmap; cm; cm=cm->next)
    {
      sprintf(tmp,"!I_%s",cm->chan_name);
      Set_Button(w[0],tmp,cm->invert);

      if (cm->r)
      {
        sprintf(tmp,"!R_%s",cm->chan_name);
        Set_Button(w[0],tmp,TRUE);
      }
      if (cm->g)
      {
        sprintf(tmp,"!G_%s",cm->chan_name);
        Set_Button(w[0],tmp,TRUE);
      }
      if (cm->b)
      {
        sprintf(tmp,"!B_%s",cm->chan_name);
        Set_Button(w[0],tmp,TRUE);
      }
    }
  }

  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show_all(wnd);
  place_window(wnd,0,0,right_top);
  return;
}


#define LAB_CHANP "chanp"
#define LAB_CHANN "chann"
#define LAB_HBND "hbnd"
#define LAB_LBND "lbnd"
#define LAB_GAMMA "gamma"

void do_maprngbut(GtkWidget *widget,gpointer data)
{
//  smap2map(widget);    // gaat niet?? Wist zichzelf?
}

void set_knopsetrgb(MSGCMAP *mm)
{
}

#define BFRMT "!%c_%sxx"
GtkWidget *create_knopsetrgb(char listchan[MAXCHAN][LENLISTCHAN],int nrlistchan)
{
  int i,j;
  GtkWidget *w[8];

  w[0]=gtk_table_new(0,0,FALSE);
  j=0;
  w[1]=Create_Label("Chan P");
  w[2]=Create_Label("");
  w[3]=Create_Label("Chan N");
  w[4]=Create_Label("Temp L");
  w[5]=Create_Label("Temp H");
  w[6]=Create_Label("Gamma");
  for (i=1; i<=6; i++)
    gtk_table_attach(GTK_TABLE(w[0]),w[i],i+1,i+2,1+j,2+j,GTK_FILL,GTK_FILL,1,1);

  for (j=0; j<3; j++)
  {
    char hdr=(j==1? 'G' : j==2? 'B' : 'R');
    char lbl[20];
    w[1]=Create_Label(j==1? "Green":j==2? "Blue" : "Red");

    sprintf(lbl,BFRMT,hdr,LAB_CHANP);
    w[2]=Create_Optionmenu(lbl,do_maprngbut,0,0);
    Add_Options(w[2],lbl,do_maprngbut,0,(char **)listchan,nrlistchan,LENLISTCHAN);

    w[3]=Create_Label(" - ");

    sprintf(lbl,BFRMT,hdr,LAB_CHANN);
    w[4]=Create_Optionmenu(lbl,do_maprngbut,0,0);
    Add_Options(w[4],lbl,do_maprngbut,0,(char **)listchan,nrlistchan,LENLISTCHAN);

    sprintf(lbl,BFRMT,hdr,LAB_LBND);
    w[5]=Create_Spin(lbl,do_maprngbut,"%3d%3d%3d",0,-400,400);
    sprintf(lbl,BFRMT,hdr,LAB_HBND);
    w[6]=Create_Spin(lbl,do_maprngbut,"%3d%3d%3d",0,-400,400);
    sprintf(lbl,BFRMT,hdr,LAB_GAMMA);
    w[7]=Create_Spin(lbl,do_maprngbut,"%3f%3f%3f",1.,-5.,5.);

    for (i=1; i<=7; i++)
      gtk_table_attach(GTK_TABLE(w[0]),w[i],i,i+1,2+j,3+j,GTK_FILL,GTK_FILL,3,3);
  }

  return w[0];
}

#define LAB_SMAPNAM "RGB Composite type"
void set_mapping_spec(GtkWidget *wdgt,MSGCMAP cms,char listchan[MAXCHAN][LENLISTCHAN],int nrlistchan)
{
  GtkWidget *wnd=Find_Parent_Window(wdgt);
  char lbl[20];
  int i,j;
  char hdr[4];
  strcpy(hdr,"RGB");
  for (j=0; j<3; j++)
  {
    RGBMAPS crgb=(j==1? cms.g: j==2? cms.b: cms.r);
    gboolean chdone[2]={FALSE,FALSE};
    for (i=0; i<nrlistchan; i++)
    {
      if (!strcmp(listchan[i],crgb.chanp))
      {
        sprintf(lbl,BFRMT,hdr[j],LAB_CHANP);
        Set_Optionsmenu(wnd,lbl,i);
        chdone[0]=TRUE;
      }
      if (!strcmp(listchan[i],crgb.chann))
      {
        sprintf(lbl,BFRMT,hdr[j],LAB_CHANN);
        Set_Optionsmenu(wnd,lbl,i);
        chdone[1]=TRUE;
      }
    }
    if ((*crgb.chanp) && (!chdone[0]))
    {
      sprintf(lbl,BFRMT,hdr[j],LAB_CHANP);
      Set_Widgetcolor(Find_Widget(wnd,lbl),'b',0xff,0,0);
    }
    if ((*crgb.chann) && (!chdone[1]))
    {
      sprintf(lbl,BFRMT,hdr[j],LAB_CHANN);
      Set_Widgetcolor(Find_Widget(wnd,lbl),'b',0xff,0,0);
    }

    sprintf(lbl,BFRMT,hdr[j],LAB_LBND);
    Set_Adjust(wnd,lbl,"%d",crgb.valmin);
    sprintf(lbl,BFRMT,hdr[j],LAB_HBND);
    Set_Adjust(wnd,lbl,"%d",crgb.valmax);
    sprintf(lbl,BFRMT,hdr[j],LAB_GAMMA);
    Set_Adjust(wnd,lbl,"%f",crgb.gamma);

  }
  Set_Entry(wnd,LAB_SMAPNAM,cms.name);
}

void smap2map(GtkWidget *widget)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  int j;
  MSGCMAP cm;
  RGBMAPS rgbm;
  memset(&cm,0,sizeof(cm));

  for (j=0; j<3; j++)
  {
    char hdr=(j==1? 'G' : j==2? 'B' : 'R');
    char lbl[20];
    char *s;
    sprintf(lbl,BFRMT,hdr,LAB_CHANP);
    if ((s=soption_menu_get_history(Find_Widget(widget,lbl)))) strcpy(rgbm.chanp,s);
    sprintf(lbl,BFRMT,hdr,LAB_CHANN);
    if ((s=soption_menu_get_history(Find_Widget(widget,lbl)))) strcpy(rgbm.chann,s);
    sprintf(lbl,BFRMT,hdr,LAB_LBND);
    rgbm.valmin=Get_Adjust(widget,lbl);
    sprintf(lbl,BFRMT,hdr,LAB_HBND);
    rgbm.valmax=Get_Adjust(widget,lbl);
    sprintf(lbl,BFRMT,hdr,LAB_GAMMA);
    rgbm.gamma=Get_Adjust(widget,lbl);
    if (j==0) cm.r=rgbm; else if (j==1) cm.g=rgbm; else cm.b=rgbm;
  }
  cm.use_temp=TRUE;                  // assume for this mapping all IR-channels must be temps 
  set_mapping_from_cm(widget,cm);
  if ((Find_Window(wnd,LAB_COLOR)))
  {
    create_colormap_delayed(wnd);
  }
  
}

#define LAB_SETCMS "Set"
void func_spec(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_SETCMS))
  {
    smap2map(widget);
  }
}

 /* Create pop-up for color selection (set ranges) */
void color_map_spec(GtkWidget *window,GROUP *grp)
{
  GtkWidget *wnd,*w[10];
  static GtkWidget *wcnt;
  static char listchan2[MAXCHAN][LENLISTCHAN];
  static int nrlistchan2;

  if (!(wnd=Find_Window(window,LAB_COLORSPEC)))       // window doesn't exits =>
  {                                               //   create it
    wnd=Create_Window(window,0,0,LAB_COLORSPEC,NULL);
    gtk_widget_show(wnd);
    place_window(wnd,0,0,left_bottom);
    gtk_window_set_policy((GtkWindow *)wnd,FALSE,FALSE,TRUE);
  }
  else if (wcnt)                                 // window exists =>
  {                                              //   remove content of window
    gtk_widget_destroy(wcnt);
  }
  /* Create (new) buttons */
  nrlistchan2=create_list_available_channels(grp,listchan2,FALSE);
  w[1]=create_knopsetrgb(listchan2,nrlistchan2);
  w[2]=Create_Button(LAB_SETCMS,func_spec);
  w[3]=Create_Entry(LAB_SMAPNAM,NULL,"%-20s","");
  wcnt=Pack(NULL,'v',w[3],1,w[1],1,w[2],1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),wcnt);
  gtk_widget_show_all(wnd);

/* Set mapping defined in globinfo */
  set_mapping_spec(wnd,globinfo.clrmapspec,listchan2,nrlistchan2);
  gtk_widget_show_all(wnd);
  return;
}


#endif
