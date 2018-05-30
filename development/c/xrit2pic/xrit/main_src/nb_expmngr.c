#define LAB_ELIST "explist"
#define LAB_EDEL "Delete"
#define LAB_EFILT "Filter"
#define LAB_EXPUPD "Update"
#define LAB_EXPF "Exported"
#define LAB_MVTMP "Temp files 1"
#define LAB_MVTMPSAV "Temp files 2"
#define LAB_EPAD "!EPAD"
#define LAB_EVIEW "View"
#include <stdlib.h>
#include "xrit2pic.h"
#include "gtk/gtk.h"
extern PREFER prefer;
extern GLOBINFO globinfo;
static GtkCList *list;

static GList *get_nextsel(GList *gl)
{
  GList *gl1,*glrt=NULL;
  static int n;
  int n1=9999;
  if (gl==NULL)
  {
    n=-1;
    return NULL;
  }
  for (gl1=gl; gl1; gl1=gl1->next)
  {
    if ((int)gl1->data > n)
    {
      if ((int)gl1->data < n1) { n1=(int)gl1->data; glrt=gl1; }
    }
  }
  n=n1;
  return glrt;
}

static char dest_dir[300];  // see xrit2pic.h

static void expfunc(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  GList *gl;
  char *filter=Get_Entry(widget,LAB_EFILT);
  GtkCList *clist=(GtkCList *)Find_Widget(widget,LAB_ELIST);
  if (!strcmp(name,LAB_EDEL))
  {
    if (clist->selection)
    {
      int nf=0,chce;
      get_nextsel(NULL);
      while ((gl=get_nextsel(clist->selection))) nf++;
      chce=Create_Choice("Warning",2,"Yes","No!","Delete %d files. Proceed?\n",nf);
      if (chce==1)
      {
        int err=0;
        get_nextsel(NULL);
        while ((gl=get_nextsel(clist->selection)))
        {
          char absf[500];
          char *file;
          gtk_clist_get_text(clist,(int)gl->data,0,&file);
          if ((!file) || (!(*file))) continue;
          sprintf(absf,"%s%s",Get_Entry(widget,LAB_EPAD),file);
          if (remove(absf)) err++;
        }
        Load_Dirfilelist(list,Get_Entry(widget,LAB_EPAD),'f',NULL);
        if (err) Create_Message("Error","Failed to remove %d files.",err);
      }
    }
  }
  
  if (!strcmp(name,LAB_EVIEW))
  {
    if (clist->selection)
    {
      get_nextsel(NULL);
      gl=get_nextsel(clist->selection);
      {
        char cmd[500];
        char *file;
#if __GTK_WIN32__ == 1
        const char nwproc=0;
#else
        const char nwproc='&';
#endif
        *cmd=0;
        gtk_clist_get_text(clist,(int)gl->data,0,&file);
        if ((file) && ((*file)))
        {
          char tmp[500];
          sprintf(tmp,"%s%s",Get_Entry(widget,LAB_EPAD),file);
          switch(ftype(tmp))
          {
            case 'd':
              if (!strcmp(file,".."))
              {
                char *p;
                if ((p=strrchr(dest_dir,DIR_SEPARATOR))) *p=0;
                if ((p=strrchr(dest_dir,DIR_SEPARATOR))) *(p+1)=0;
              }
              else
              {
                sprintf(dest_dir,"%s%s%c",dest_dir,file,DIR_SEPARATOR);
              }
              Set_Entry(widget,LAB_EPAD,"%s",dest_dir);
              Load_Dirfilelist(list,dest_dir,0,filter);
              *cmd=0;
            break;
            case 'a':
              if (strstr(prefer.prog_playmovie,"%s"))
              {
                sprintf(cmd,prefer.prog_playmovie,tmp);
              }
              else
              {
               sprintf(cmd,"%s %s%s %c",prefer.prog_playmovie,Get_Entry(widget,LAB_EPAD),file,nwproc);
              }
            break;
            case 'j': case '?':
              sprintf(cmd,"%s %s%s%c",prefer.prog_jpg,Get_Entry(widget,LAB_EPAD),file,nwproc);
            break;
            case 'p':
              sprintf(cmd,"%s %s%s%c",prefer.prog_pgm,Get_Entry(widget,LAB_EPAD),file,nwproc);
            break;
            default:
              Create_Message("Error","Can't open %s.\n",tmp);
            break;
          }
          if (*cmd) system(cmd);
        }
      }
    }
  }

  if (!strcmp(name,LAB_EXPUPD))
  {
    Load_Dirfilelist(list,dest_dir,0,filter);
  }
  if (!strcmp(name,LAB_EXPF))
  {
    if (Get_Button(widget,LAB_EXPF))
    {
      strcpy(dest_dir,globinfo.dest_dir);
      Set_Entry(widget,LAB_EPAD,"%s",dest_dir);
      Load_Dirfilelist(list,dest_dir,0,filter);
    }
  }
  if (!strcmp(name,LAB_MVTMP))
  {
    if (Get_Button(widget,LAB_MVTMP))
    {
      Set_Entry(widget,LAB_EPAD,"%s",globinfo.dest_tmpdir);
      Load_Dirfilelist(list,globinfo.dest_tmpdir,'f',NULL);
    }
  } 
  if (!strcmp(name,LAB_MVTMPSAV))
  {
    if (Get_Button(widget,LAB_MVTMPSAV))
    {
      Set_Entry(widget,LAB_EPAD,"%s",globinfo.dest_tmpsavdir);
      Load_Dirfilelist(list,globinfo.dest_tmpsavdir,'f',NULL);
    }
  } 
}

static int colwidth[]={30,10,10};
#define PIJLO "[^]"
#define PIJLN "[v]"
#define NR_COL 3
static gint sortfunc(GtkCList *clist,gconstpointer *ptr1, gconstpointer *ptr2)
{
  const GtkCListRow *row1 = (const GtkCListRow *) ptr1;
  const GtkCListRow *row2 = (const GtkCListRow *) ptr2;
  char *s1,*s2;

  s1 = GTK_CELL_TEXT(row1->cell[clist->sort_column])->text;
  s2 = GTK_CELL_TEXT(row2->cell[clist->sort_column])->text;
  if ((!s1) || (!s2)) return 0;
  return strcmp(s1,s2);
}

static void sort_col(GtkWidget      *widget,
                     gint            column,
                     gboolean        as_desn,
                     gpointer        data)
{
  GtkCList *w_cl=(GtkCList *)widget;
  char str[50],*p;
  int c;
  gtk_clist_set_sort_column(w_cl,column);
  gtk_clist_set_compare_func(w_cl,(GtkCListCompareFunc)sortfunc);
  if (as_desn)
    gtk_clist_set_sort_type(w_cl,GTK_SORT_DESCENDING);
  else
    gtk_clist_set_sort_type(w_cl,GTK_SORT_ASCENDING);
  gtk_clist_sort(w_cl);

  for (c=0; c<NR_COL; c++)
  {
    strcpy(str,gtk_clist_get_column_title(w_cl,c));
    while (strlen(str)<colwidth[c]) strcat(str," ");
    if ((p=strstr(str,PIJLO))) *p=0;
    if ((p=strstr(str,PIJLN))) *p=0;
    if (c==column)
    {
      if (as_desn)
        strcat(str,PIJLO);
      else
        strcat(str,PIJLN);
    }
    gtk_clist_set_column_title(w_cl,c,str);
  }
}

static void sort_col1(GtkWidget      *widget,
                      gint            icol,
                      gpointer        data)
{
  static gboolean lastcol_sorted;
  static gboolean as_desn=FALSE;
  int column;
  if (icol<0)
  {
    column=lastcol_sorted;
  }
  else
  {
    column=icol;
    as_desn=!as_desn;
  }
  sort_col(widget,column,as_desn,data);
  if (icol>=0)
  {
    lastcol_sorted=column;
  }
}


GtkWidget *nb_expmngr()
{
  char filter[100];
  GtkWidget *w[9];
  strcpy(dest_dir,globinfo.dest_dir);
  strcpy(filter,"*");
  list=(GtkCList *)Create_Clist(LAB_ELIST,NULL,NULL,sort_col1,NR_COL,
                                                  "Exported",40,
                                                  "Date",5,
                                                  "weg",10,
                                                  0);
  gtk_clist_set_selection_mode(list,GTK_SELECTION_EXTENDED);
  gtk_clist_set_column_visibility(list,1,TRUE);
  gtk_clist_set_column_visibility(list,2,FALSE);

  Load_Dirfilelist(list,globinfo.dest_dir,0,filter);
  w[1]=Create_Button(LAB_EDEL,expfunc);
  w[2]=Create_ButtonArray("Generated files",expfunc,3,
                                            RADIO,LAB_EXPF,
                                            RADIO,LAB_MVTMP,
                                            RADIO,LAB_MVTMPSAV,
                                            NULL);

  w[3]=Create_Button(LAB_EXPUPD,expfunc);
  w[4]=Create_Entry(LAB_EFILT,expfunc,"%8s",". ");
  Set_Entry(w[4],LAB_EFILT,filter);
  w[3]=Pack("",'h',w[3],5,w[4],1,NULL);

  w[5]=Create_Label("Path exported");
  w[6]=Create_Entry(LAB_EPAD,NULL,"%-20s",globinfo.dest_dir);
  w[5]=SPack(NULL,"h",w[5],"1",w[6],"ef1",NULL);
  w[7]=Create_Button(LAB_EVIEW,expfunc);
  w[8]=SPack(NULL,"h",w[7],"ef1",w[1],"10",NULL);
  w[0]=Pack("",'v',w[2],1,w[3],1,w[5],1,list,1,w[8],1,NULL);
  return w[0];
}
