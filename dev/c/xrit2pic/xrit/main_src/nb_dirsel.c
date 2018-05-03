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

static char *gseldir;
static GtkCTree *globtree;
static GtkCList *globdlist;

extern PREFER prefer;
extern GLOBINFO globinfo;
extern GROUP *globgrp;

void do_prefbut(GtkWidget *iwidget,gpointer data);

/* Buttons dir select part */
#define LAB_LOC "Browse dir."
#define LAB_ADDL "Add to list"
#define LAB_REML "Remove from list"
#define LAB_VIS   "!Show in list"
#define LAB_SAVE "Save it."

void do_prefbut1(GtkWidget *iwidget,gpointer data)
{
  char *name=(char *)data;
  GtkWidget *wnd=gtk_widget_get_toplevel(iwidget);
  if (!strcmp(name,LAB_SAVE))
  {
    char *tmp;
    tmp=Get_Entry(wnd,LAB_ADIR);
    strcpy(prefer.src_dir,tmp);
    finish_path(prefer.src_dir);
    do_prefbut(iwidget,data);
    strcpy(gseldir,prefer.src_dir);
  }
}


#define LIMIT_NFILES 100
/*******************************************************
 * Catch selected directory from file manager
 *******************************************************/
static void file_ok_sel(GtkWidget *widget, GtkFileSelection *fs)
{
  GtkWidget *file_window=Find_Parent_Window(widget);
  GtkWidget *main_window=Find_Window(file_window,LAB_MAIN);
  if (file_selection_get_dirname())
  {
    Set_Entry(main_window,LAB_ADIR,gseldir);
 /*   Set_Entry(main_window,LAB_SDIR,gseldir);*/
  }
  Close_Fileselectf(file_window);
}

/*******************************************************
 * Load received dir
 * and archive
 *******************************************************/
void define_and_load_srcdir(GtkWidget *wnd,GtkWidget *widget)
{
  GtkNotebook *notebook=(widget? (GtkNotebook *)widget->parent : NULL);;
  Set_Entry(wnd,LAB_SDIR,prefer.src_dir);
/* set to 'All'; otherwise tree not properly shown (to be fixed) */
  globinfo.vis_mode='a'; 
  if ((Get_Optionsmenu(widget,LAB_VIS))!=0) /* to prevent unnecess. redraw */
    Set_Optionsmenu(widget,LAB_VIS,0); 

/* Load received */
  globgrp=Load_List_all(globgrp,globtree,widget,0);

/* Load archive list */
  if (globdlist)
  {
    gtk_clist_freeze(globdlist);

    if ((GTK_IS_NOTEBOOK(notebook)) && ((gtk_notebook_get_current_page(notebook))==3))
      Load_CTree_Dirlist((GtkCTree *)globdlist,NULL,globinfo.src_archdir,3);
    else
      Load_CTree_Dirlist((GtkCTree *)globdlist,NULL,globinfo.src_archdir,1);
    gtk_clist_thaw(globdlist);
    Set_Entry(wnd,LAB_ARCHSRCDI,globinfo.src_archdir);
  }
}


/*******************************************************
 * Handle select buttons etc.
 *******************************************************/
void do_selbut(GtkWidget *widget,gpointer data)
{
  GtkWidget *wnd=gtk_widget_get_toplevel(widget);
  GtkNotebook *notebook=(GtkNotebook *)widget->parent->parent->parent;
  char *name=(char *)data;

  if (!strcmp(name,LAB_LOC))
  {
    Create_Fileselectf(wnd,"Select dir",NULL,file_ok_sel,gseldir,
                                        NULL,NULL,LIMIT_NFILES,NULL) ;
  }

/* Add to list */
  if (!strcmp(name,LAB_ADDL))
  {
    char *tmp[1];
    tmp[0]=Get_Entry(wnd,LAB_ADIR);
    gtk_clist_append(GTK_CLIST(globinfo.dirsel.list), tmp);
  }

/* Remove from list */
  if (!strcmp(name,LAB_REML))
  {
    if (globinfo.dirsel.selrow>=0) gtk_clist_remove(GTK_CLIST(globinfo.dirsel.list), globinfo.dirsel.selrow);
  }

/* Load xrit-dir */
  if (!strcmp(name,LAB_LOAD))
  {
    char *tmp;
    tmp=Get_Entry(wnd,LAB_ADIR);
    Set_Button(widget,LAB_EXPCOL,FALSE);
    gtk_notebook_set_page(notebook,0);
    strcpy(prefer.src_dir,tmp);
    finish_path(prefer.src_dir);
    strcpy(globinfo.src_dir,prefer.src_dir);
    related_dirs(&globinfo,&prefer);
    
    define_and_load_srcdir(wnd,widget);
  }
}

/* Selection of XRIT received directory */
void Dir_selection_lost( GtkWidget      *clist,
                         gint            row,
                         gint            column,
		         GdkEventButton *event,
                         gpointer        data)
{
  globinfo.dirsel.selrow=-1;
}

void Dir_selection_made( GtkWidget      *clist,
                         gint            row,
                         gint            column,
		         GdkEventButton *event,
                         gpointer        data)
{
  GtkWidget *main_window=Find_Parent_Window(clist);
  gchar *text;
  char sel_dir[300];
  globinfo.dirsel.selrow=row;
  gtk_clist_get_text(GTK_CLIST(globinfo.dirsel.list), row, 0, &text);

  strcpy(sel_dir,text);
  finish_path(sel_dir);
  Set_Entry(main_window,LAB_ADIR,sel_dir);
}

GtkWidget *nb_dirsel(GtkCTree *ctree,GtkCList *dlist,char *gseldiri)
{
  GtkWidget *wb[10];
  GtkCList *clist;
  gseldir=gseldiri;
  globtree=ctree;
  globdlist=dlist;

  clist=(GtkCList *)Create_Clist(NULL,Dir_selection_made,Dir_selection_lost,NULL,1,
                                                         "XRIT dir. list",20,0);
  wb[1]=Create_Button(LAB_LOC,do_selbut);
  wb[2]=Create_Button(LAB_ADDL,do_selbut);
  wb[3]=Create_Button(LAB_REML,do_selbut);
  wb[4]=Create_Button(LAB_SAVE,do_prefbut1);

  wb[5]=Pack(NULL,'h',wb[1],5,wb[2],5,wb[3],5,wb[4],5,NULL);
  wb[6]=Create_Entry(LAB_ADIR,do_selbut,"%20s","");
  wb[7]=Create_Button(LAB_LOAD,do_selbut);

  wb[0]=Pack("XRIT directories",'v',wb[5],5,wb[6],5,clist,5,wb[7],5,NULL);
  Set_Widgetcolor(wb[0],'b',0,0,0xff);
  globinfo.dirsel.list=clist;
  return wb[0];
}
