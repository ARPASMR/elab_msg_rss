/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include "xrit2pic.h"
#include "gtk/gtk.h"
#include "gdk/gdkkeysyms.h"


extern XRIT_DBASE dbase;
extern GROUP *globgrp;
extern PREFER prefer;
extern GLOBINFO globinfo;
static GtkCTree *globtree;

#define TEXTWND "!Summary"
#define LAB_SUM "Summary"
#define LAB_MOVE_ARCH "Move to archive"
#define LAB_MOVE_REC "Move from archive"
#define LAB_MOVE_DONE "Move to 'done'"
#define LAB_MOVE_DREC "Move from 'done'"

#define LAB_MVINFO "^Last action  "
#define LAB_LDSEL "Load selected"
#define LAB_DELSEL "Delete selected"
#define LAB_DLIST "File manager"

#define LAB_UPDTREE "Update   "
void forall_segm(SEGMENT *segm,int segmfunc())
{
  CHANNEL *chan=NULL;
  GROUP *grp=NULL;
  if (segm) chan=segm->chan;
  if (chan) grp=chan->group;
  if (!segmfunc) return;
  for (; segm; segm=segm->next) segmfunc(grp,chan,segm);
}

void forall_chan(CHANNEL *chan,void chanfunc(),int segmfunc())
{
  GROUP *grp=NULL;
  if (chan) grp=chan->group;
  if ((!chanfunc) && (!segmfunc)) return;
  for (; chan; chan=chan->next)
  {
    if (chanfunc) chanfunc(grp,chan);
    forall_segm(chan->segm,segmfunc);
    forall_segm(chan->pro,segmfunc);
    forall_segm(chan->epi,segmfunc);
  }
}

static GtkWidget *globwidget;


void forall_selgrp(GROUP *grp,void grpfunc(),void chanfunc(),int segmfunc(),
     gboolean just_selected)
{
  int n=0;
  for (; grp; grp=grp->next)
  {
    if ((just_selected) && (!grp->nselected)) continue;
    n++;
    if (!(n%10)) Set_Entry(globwidget,LAB_MVINFO,"%d...",n);
    while (g_main_iteration(FALSE));
    if (grpfunc) grpfunc(grp);
    forall_chan(grp->chan,chanfunc,segmfunc);
    forall_chan(grp->pro_epi,chanfunc,segmfunc);
  }
}

void forall_grp(GROUP *grp,void grpfunc(),void chanfunc(),int segmfunc())
{
  forall_selgrp(grp, grpfunc, chanfunc, segmfunc,FALSE);
}

/************************************************************/
void list_dates(GROUP *grp)
{
  static char tmstr[20],ptmstr[20];
  if (!grp)
  {
    *tmstr=*ptmstr=0;
     return;
  }
  {
    strftime(tmstr,20,"%Y%m%d",&grp->grp_tm);
    if (strcmp(tmstr,ptmstr))
      Add_Text(Find_Widget(globwidget,TEXTWND),100,"  %s \n",tmstr);
    strcpy(ptmstr,tmstr);
  }
}

#define ARCH_MULTIDIR
/************************************************************/
static int nrmv,nrnmv;
int mv_files_to(GROUP *grp,CHANNEL *chan,SEGMENT *segm,char *archdir)
{
  char *curdir=NULL,*p;
  char *nwfn=NULL;
  char yearstr[10];
  char monstr[10];
  char daystr[10];
  
  int n;
  if (!grp) return 0;
  if (segm->where==in_archive) return 0;
  if (strcpyd(&curdir,segm->pfn)) return -1;
  if ((p=strrchr(curdir,DIR_SEPARATOR))) *(p+1)=0;
  if (archdir)
  {
  }
  else
  {
    n=strftime(yearstr,8,"%Y",&grp->grp_tm);
    if (n!=4) { free(curdir); return -2; }
    n=strftime(monstr,8,"%m",&grp->grp_tm);
    if (n!=2) { free(curdir); return -2; }
    n=strftime(daystr,8,"%d",&grp->grp_tm);
    if (n!=2) { free(curdir); return -2; }
  }
#if __GTK_WIN32__ == 1
  if ((!archdir) || (!archdir[0]) || (archdir[1]!=':'))
#else
  if ((!archdir) || (*archdir!='/'))
#endif
  {
    if (strcpyd(&nwfn,globinfo.src_archdir)) return -1; // path to archive
    finish_path(nwfn);
    make_dir_hier(nwfn);
  }
  else if (archdir)
  {
    n=Create_Choice("Warning",2,"yes","No","Selected moved to non-archive dir \n%s\nOK?",archdir);
    if (n!=1) return -2;
  }
  if (archdir)
  {
    if (strcatd(&nwfn,archdir)) return -1;           // add ext. dir to path
    finish_path(nwfn);
    make_dir_hier(nwfn);
  }
  else
  {
    if (strcatd(&nwfn,yearstr)) return -1;           // add year as dir to path
    finish_path(nwfn);
    make_dir(nwfn);
    if (strcatd(&nwfn,monstr)) return -1;           // add month as dir to path
    finish_path(nwfn);
    make_dir(nwfn);
    if (strcatd(&nwfn,daystr)) return -1;           // add day as dir to path
    finish_path(nwfn);
    make_dir(nwfn);
  }
  strcatd(&nwfn,segm->fn);                         // add filename
  if (!(rename(segm->pfn,nwfn)))                   // move file
  {
    nrmv++;
    free(segm->pfn);    /* remove old name */
    segm->pfn=nwfn;     /* Add new name */
  }
  else
  {
    nrnmv++;
    free(nwfn);
  }
  if (curdir) free(curdir);
  
  return 0;
}

int mv_files(GROUP *grp,CHANNEL *chan,SEGMENT *segm)
{
  return mv_files_to(grp,chan,segm,NULL);
}

int count_dircontent(char *dir)
{
  int n=0;
  struct dirent *dirEntry;
  DIR *directory;
  directory = opendir(dir);
  if (!directory) return 0;
  while ((dirEntry = readdir(directory)))
  {
    n++;
  }
  closedir(directory);
  return n-2; // remove . and .. from count
}


void mv_unk(LIST_FILES *lf,char *unkdir)
{
  LIST_FILES *lf1;
  for (lf1=lf; lf1; lf1=lf1->next)
  {
    char *nwfn;
//    printf("%s\n",lf1->pfn);

    strcpyd(&nwfn,globinfo.src_archdir);
    finish_path(nwfn);
    make_dir(nwfn);
    strcatd(&nwfn,unkdir);
    finish_path(nwfn);
    make_dir(nwfn);

    strcatd(&nwfn,lf1->fn);
    if (!(rename(lf1->pfn,nwfn)))
    {
      nrmv++;
      free(lf1->pfn);    /* remove old name */
      lf1->pfn=nwfn;     /* Add new name */
    }
    else
    {
      nrnmv++;
      free(nwfn);
    }
  }
}

int mv_todone(GROUP *grp,CHANNEL *chan,SEGMENT *segm)
{
  static int n,n1;
  if (!segm) { n1=n; n=0; return n1; }
  if (segm->where==in_received) { Move_File(segm,TRUE); n++;  }
  return n;
}

int mv_fromdone(GROUP *grp,CHANNEL *chan,SEGMENT *segm)
{
  static int n,n1;
  if (!segm) { n1=n; n=0; return n1; }
  if (segm->where==in_done) { Move_File(segm,FALSE); n++;  }
  return n;
}

LIST_FILES *create_dirlist(char *path,int *nrf)
{
  LIST_FILES *lf=NULL;
  struct dirent *dirEntry;
  DIR *directory;
  char *abspath;
  if (nrf) *nrf=0;
  // Windows: May not end with DIR_SEPARATOR!
  if (path[strlen(path)-1]==DIR_SEPARATOR) path[strlen(path)-1]=0;
  if (!((directory = opendir(path)))) return 0;
  abspath=malloc(strlen(path)+200);
  while ((dirEntry = readdir(directory)))
  {
    sprintf(abspath,"%s%c%s",path,DIR_SEPARATOR,dirEntry->d_name);
//    if (dirEntry->d_type==4)       /* if is a directory (portable??? NO!!!) */
    if (is_a_dir(abspath))
    {
      if (!strcmp(dirEntry->d_name,".")) continue;
      if (!strcmp(dirEntry->d_name,"..")) continue;
      lf=Create_List(lf,dirEntry->d_name); /* filename */
      lf->is_dir=TRUE;
    }
    else
    {
      (*nrf)++;
    }
  }
  closedir(directory);
  free(abspath);
  rewind_s(lf);
  return lf;
}

int Load_CTree_Dirlist(GtkCTree *ctree,GtkCTreeNode *node,char *path,int depth)
{
  GtkCList *dlist=(GtkCList *)ctree;
  LIST_FILES *list_dirs=NULL,*ld;
  int nrf=0,n1;
  char *tmp[3];
  GtkCTreeNode *node1;
  if (depth<0) return 0;
  if (!node) gtk_clist_clear(GTK_CLIST(ctree));
  if (!(list_dirs=create_dirlist(path,&nrf))) return nrf;
  gtk_clist_freeze((GtkCList *)ctree);
  for (ld=list_dirs; ld; ld=ld->next)
  {
    GdkColor clr_dir={0,0,0,0xffff};  /* color dir */
    char nn[9];
    char *npath;
    finish_path(path);
    npath=malloc(strlen(path)+strlen(ld->fn)+10);
    sprintf(npath,"%s%s",path,ld->fn);
    tmp[0]=ld->fn;
    tmp[1]="";
    tmp[2]=npath;
    node1=Add_CLeaf(ctree,node,tmp);   /* next channel */
    gtk_ctree_node_set_foreground(ctree,node1,&clr_dir);
    n1=Load_CTree_Dirlist(ctree,node1,npath,depth-1);
    sprintf(nn,"%d",n1);
    gtk_ctree_node_set_text(ctree,node1,1,nn);
    if (node) gtk_ctree_node_set_selectable(ctree,node,FALSE);
/*
    if (depth<=1)
      gtk_ctree_node_set_selectable(ctree,node1,TRUE);
    else
      gtk_ctree_node_set_selectable(ctree,node1,FALSE);
*/
    nrf+=n1;
    free(npath);
  }
  
  Destroy_List(list_dirs,TRUE);
  gtk_clist_set_sort_column(dlist,0);
  gtk_clist_sort(dlist);
  gtk_clist_thaw((GtkCList *)ctree);
  return nrf;
}

void do_reorder(GtkWidget *widget,gpointer gpointer)
{
  GtkCList *clist=(GtkCList *)Find_Widget(widget,LAB_DLIST);
  GtkCTree *ctree=(GtkCTree *)Find_Widget(widget,LAB_DLIST);
  GtkNotebook *notebook=NULL;
  char *name=(char *)gpointer;
  globwidget=widget;
  
  {
    GtkWidget *w;
    for (w=widget; w; w=w->parent)
      if (GTK_IS_NOTEBOOK(w))
        notebook=(GtkNotebook *)w;
  }

  if (!strcmp(name,LAB_UPDTREE))
  {
    Load_CTree_Dirlist(ctree,NULL,globinfo.src_archdir,3);
  }
  
  if (!strcmp(name,LAB_SUM))
  {
    list_dates(NULL);
    Add_Text(Find_Widget(widget,TEXTWND),100," Dates:\n");
    forall_grp(globgrp,list_dates,NULL,NULL);
  }

// Move from received/done to archive
  if (!strcmp(name,LAB_MOVE_ARCH))
  {
    nrmv=nrnmv=0;
    Set_Entry(widget,LAB_MVINFO,"Moving to archive...");
    while (g_main_iteration(FALSE));
    mv_unk(dbase.unknown,"unknown");
    forall_grp(globgrp,NULL,NULL,mv_files);
    if ((nrmv) || (nrnmv))
      Set_Entry(widget,LAB_MVINFO,"Moved to arch %d; failed to move %d.",nrmv,nrnmv);
    else
      Set_Entry(widget,LAB_MVINFO,"Nothing to move.");

//    globgrp=Load_List(globgrp,globtree,widget,globinfo.src_dir,TRUE,in_received,FALSE,0);
    globgrp=Load_List_all(globgrp,dbase.main_tree,widget,0);
//    Load_Dirfilelist(clist,globinfo.src_archdir,'d',NULL);
    Load_CTree_Dirlist(ctree,NULL,globinfo.src_archdir,3);
  }
  
// Move from archive to received  
  if (!strcmp(name,LAB_MOVE_REC))
  {
    int n=0;
    if ((clist->selection))
    {
      GList *gl;
      Set_Entry(widget,LAB_MVINFO,"Moving to received...");
      while (g_main_iteration(FALSE));
      if (globtree) gtk_clist_clear(GTK_CLIST(globtree));
      if (globgrp)
      {
        Remove_Grps(globgrp);
        globgrp=NULL;
        Destroy_List(dbase.unknown,TRUE);              /* Remove unk list */
        dbase.unknown=NULL;
      }

      for (gl=clist->selection; gl; gl=gl->next)
      {
        char *path,*f_in_path=NULL;
        gtk_ctree_node_get_text(ctree,gl->data,2,&path);
        strcpyd(&f_in_path,path);
        finish_path(f_in_path);
        move_dircontent(f_in_path,globinfo.src_dir);
        rmdir(f_in_path);
        free(f_in_path); f_in_path=NULL;
//        globgrp=Load_List(globgrp,globtree,widget,globinfo.src_dir,FALSE,in_received,FALSE,0);
        n++;
      }
      globgrp=Load_List_all(globgrp,dbase.main_tree,widget,0);
      Load_CTree_Dirlist(ctree,NULL,globinfo.src_archdir,3);
      Set_Entry(widget,LAB_MVINFO,"Moved content of %d dirs to 'received'.",n);
    }
    else
    {
      Set_Entry(widget,LAB_MVINFO,"Nothing selected.");
    }
  }  


  if (!strcmp(name,LAB_MOVE_DONE))
  {
    int n=0;
    mv_todone(NULL,NULL,NULL);
    forall_grp(globgrp,NULL,NULL,mv_todone);
    n=mv_todone(NULL,NULL,NULL);
    Set_Entry(widget,LAB_MVINFO,"Moved from 'received' to 'done': %d.",n);
  }
  if (!strcmp(name,LAB_MOVE_DREC))
  {
    int n=0;
    mv_fromdone(NULL,NULL,NULL);
    forall_grp(globgrp,NULL,NULL,mv_fromdone);
    n=mv_fromdone(NULL,NULL,NULL);
    Set_Entry(widget,LAB_MVINFO,"Moved from 'done' to 'received': %d.",n);
  }

// Load selected archive item
  if (!strcmp(name,LAB_LDSEL))
  {
    GList *gl;
    if (clist->selection)
    {
      Set_Entry(widget,LAB_MVINFO,"");
      if (globtree) gtk_clist_clear(GTK_CLIST(globtree));
      if (globgrp)
      {
        Remove_Grps(globgrp);
        globgrp=NULL;
        Destroy_List(dbase.unknown,TRUE);              /* Remove unk list */
        dbase.unknown=NULL;
      }
      for (gl=clist->selection; gl; gl=gl->next)
      {
        char *path;
        gtk_ctree_node_get_text(ctree,gl->data,2,&path);
        globgrp=Load_List(globgrp,globtree,widget,path,FALSE,in_archive,FALSE,0);
      }
      globgrp=Sort_Grp(globgrp);                          /* sort received/done */
      show_tree(globtree,globgrp,TRUE,TRUE);
      if (notebook) gtk_notebook_set_page(notebook,0);

    }
    else
    {
      Set_Entry(widget,LAB_MVINFO,"Nothing selected.");
    }
  }
  
  if (!strcmp(name,LAB_DELSEL))
  {
    char *dpath=NULL;
    GList *gl;
    int n;
    Set_Entry(widget,LAB_MVINFO,"");

    if (clist->selection)
    {
      for (gl=clist->selection; gl; gl=gl->next)
      {
        char *path;
        if (!gtk_ctree_get_node_info(ctree,gl->data,&path,NULL,NULL,NULL,NULL,NULL,NULL,NULL)) continue;
        strcatd(&dpath,path);
        if (!gtk_ctree_node_get_text(ctree,gl->data,1,&path)) continue;
        strcatd(&dpath," (");
        strcatd(&dpath,path);
        strcatd(&dpath," files)");

        strcatd(&dpath,"\n");
      }
      n=Create_Choice("Warning",2,"yes","No","Are you sure \nto delete content of \n--------------------\n%s",dpath);
      free(dpath);
      if (n==1)
      {
        n=0;
        for (gl=clist->selection; gl; gl=gl->next)
        {
          char *path;
          if (!gtk_ctree_get_node_info(ctree,gl->data,&path,NULL,NULL,NULL,NULL,NULL,NULL,NULL)) continue;
          Set_Entry(widget,LAB_MVINFO,"Deleting %s...",path);
          if (!gtk_ctree_node_get_text(ctree,gl->data,2,&path)) continue;
          n+=remove_dircontent(path,NULL,TRUE);
          rmdir(path);
          Set_Entry(widget,LAB_MVINFO,"Removed %d files....",n);
        }
       globgrp=Load_List_all(globgrp,dbase.main_tree,widget,0);
//       globgrp=Load_List(globgrp,globtree,widget,globinfo.src_dir,TRUE,in_received,FALSE,0);
        Load_CTree_Dirlist(ctree,NULL,globinfo.src_archdir,3);
        Set_Entry(widget,LAB_MVINFO,"Removed %d files.",n);
      }
    }
    else
    {
      Set_Entry(widget,LAB_MVINFO,"Nothing selected.");
    }
  }
}

void Dsel_made(){}

void Dsel_lost(GtkWidget      *clist,
               gint            row,
               gint            column,
               GdkEventButton *event,
               gpointer        data)
{
}

GtkWidget *nb_filemngr(GtkCList **dlist,GtkCTree *ctree)
{
  GtkWidget *wb[10],*w[5];
   char *tmphdr[3];
   tmphdr[0]="Subdirectories";
   tmphdr[1]="# items";
   tmphdr[2]="weg";
   *dlist=(GtkCList *)Create_CTree(LAB_DLIST,Dsel_made,Dsel_lost,NULL,NULL,NULL,3,tmphdr);

  globtree=ctree;
/* Allow multiple selection */
  gtk_clist_set_selection_mode(*dlist,GTK_SELECTION_EXTENDED);
  gtk_clist_set_column_visibility(*dlist,2,FALSE);
  
  w[0]=Create_Entry(LAB_ARCHSRCDI,NULL,"%20s","");
  // Move between received and archive
  wb[1]=Create_Button(LAB_MOVE_REC,do_reorder);
  wb[2]=Create_Button(LAB_MOVE_ARCH,do_reorder);
  w[1]=SPack(NULL,"h",wb[1],"ef5",wb[2],"ef5",NULL);
  w[1]=Pack("Received <--> Archive",'v',w[1],5,NULL);
  Set_Widgetcolor(w[1],'b',0,0,0x0);

  // Move between received and done
  wb[3]=Create_Button(LAB_MOVE_DREC,do_reorder);
  wb[4]=Create_Button(LAB_MOVE_DONE,do_reorder);
  w[2]=SPack(NULL,"h",wb[3],"ef5",wb[4],"ef5",NULL);
  w[2]=Pack("Received <--> Done",'v',w[2],5,NULL);
  Set_Widgetcolor(w[2],'b',0,0,0x0);

  // Selection
  wb[5]=Create_Button(LAB_LDSEL,do_reorder);
  wb[6]=Create_Button(LAB_DELSEL,do_reorder);
  wb[6]=Pack(NULL,'h',wb[6],5,NULL);

  wb[7]=Create_Entry(LAB_MVINFO,NULL,"%15s","");

  w[4]=Pack("Archive",'v',w[0],1,w[1],10,wb[5],10,wb[6],10,NULL);
  Set_Widgetcolor(w[4],'b',0,0,0xff);
  w[4]=Pack(NULL,'v',w[4],10,w[2],10,wb[7],10,NULL);
    
//  w[2]=Pack(NULL,'h',wb[3],1,wb[5],1,NULL);
//  w[2]=Pack(NULL,'v',w[2],10,wb[4],10,wb[6],10,wb[7],10,NULL);
  w[2]=Create_Button(LAB_UPDTREE,do_reorder);
  w[2]=Pack(NULL,'v',w[2],1,*dlist,1,NULL);
  w[0]=SPack("","h",w[4],"1",w[2],"ef1",NULL);
  Set_Entry(w[0],LAB_ARCHSRCDI,globinfo.src_archdir);

  return w[0];
}
