/*******************************************************************
 * Copyright (C) 2011 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include <stdio.h>
#include <malloc.h>
extern GLOBINFO globinfo;

static unsigned char BMPSIGN1[4]={ 0x42,0x4d,0x36,0x06};
static unsigned char BMPSIGN2[4]={ 0x42,0x4d,0xae,0x08};
static int read_bmphdr(FILE *fp,int *Width,int *Height,int *Depth)
{
  char s[16];
  fread(s,4,1,fp);
  if ((memcmp(s,BMPSIGN1,4)) && (memcmp(s,BMPSIGN2,4))) return 1;
  fread(s,14,1,fp);
  fread(s,2,1,fp);
  *Width=s[1]*0x100+s[0];
  fread(s,2,1,fp);
  fread(s,2,1,fp);
  *Height=s[1]*0x100+s[0];
  fread(s,16,1,fp);
  fread(s,14,1,fp);
  return 0;
}

static int read_line(FILE *fp,int w,int *rgb)
{
  unsigned char r,g,b;
  int x;
  for (x=0; x<w; x++)
  {
    if (!(fread(&b,1,1,fp))) return 1;
    if (!(fread(&g,1,1,fp))) return 1;
    if (!(fread(&r,1,1,fp))) return 1;
    rgb[x]=(((int)r)<<16)+(((int)g<<8))+b;
  }
  return 0;
}

static void free_lut(LUT *lut)
{
  int i;
  for (i=0; i<NRLUTPF; i++)
  {
    if (lut->rgb[i])
    {
      free(lut->rgb[i]);
      lut->rgb[i]=NULL;
    }
  }
  lut->w=0;
  lut->nrluts=0;
}

static LUT load_lut(LUT lut)
{
  int w,h,d,i;
  FILE *fp;
  char glutname[300],*p;
  free_lut(&lut);

  snprintf(glutname,300,"%s%s",lut.dir,lut.fname);
  strcpy(lut.name,lut.fname);
  if ((p=strchr(lut.name,'.'))) *p=0;
  if (!(fp=fopen(glutname,"rb"))) return lut;
  if ((read_bmphdr(fp,&w,&h,&d))) { fclose(fp); return lut; }
  h=MAX(h,NRLUTPF);

  for (i=0; i<h; i++)
  {
    if ((lut.rgb[i]=calloc(w,sizeof(*lut.rgb))))
    {
      if ((i) && (w%2)) fseek(fp,1,SEEK_CUR);  // + 1 byte?
      if (read_line(fp,w,lut.rgb[i]))
      {
        free(lut.rgb[i]);
        lut.rgb[i]=NULL;
        break;
      }
    }
  }
  lut.nrluts=i;
  lut.w=w;
  lut.sellut=MIN(lut.sellut,lut.nrluts-1);
  fclose(fp);
  return lut;
}

void get_lut()
{
  globinfo.lut=load_lut(globinfo.lut);
}

int *temp2lut(float pix)
{
  static int opix[3];
  int rgb;
  int sellut=globinfo.lut.sellut;
  if (globinfo.lut.w==100)  // range -60...40, res. 1 degree
  {
    pix=(pix>=60? pix-60 : 0);
  }
  else                      // range -120...60, res. 0.5 degree
  {
    pix=(pix>=(273-120)? pix-(273-120) : 0);
    pix*=2.;
  }
  pix=MIN(pix,globinfo.lut.w-1);
  if (globinfo.lut.rgb[sellut])
  {
    rgb=globinfo.lut.rgb[sellut][(int)pix];
    opix[0]=(rgb>>16)&0xff;
    opix[1]=(rgb>>8)&0xff;
    opix[2]=(rgb)&0xff;
  }
  else
  {
    opix[0]=0xff; // not available
    opix[1]=0;
    opix[2]=0;
  }
  return opix;
}

void extract_lutfiles(LUT **lut,LIST_FILES *list_files)
{
  LIST_FILES *lf;
  LUT *l;
  char *filter="*.bmp";
  char *p;
  for (lf=list_files; lf; lf=lf->next)
  {
    if (!lf->fn) continue;
    if (lf->is_dir) continue;
    if ((filter) && (strcmpwild(lf->fn,filter))) continue;
    if (!(l=Create_Lut(lut))) continue;
    strncpy(l->fname,lf->fn,190);
    strncpy(l->name,lf->fn,190);
    if ((p=strchr(l->name,'.'))) *p=0;
  }
}

// create linked list with overlay file info
LUT *load_lutfiles(char *lut_dir)
{
  LUT *lut=NULL;
  LIST_FILES *list_files=NULL;
  list_files=create_filelist(lut_dir);    /* Generate a list of all files/dirs */
#ifndef __NOGUI__
  update_flist1(list_files);
#endif
  extract_lutfiles(&lut,list_files);
  Destroy_List(list_files,TRUE);  // destroy list; it's now in clist

  return lut;

}

#ifndef __NOGUI__
extern PREFER prefer;
void list_luts(GtkWidget *widget)
{
  GtkWidget *wnd,*wl;
  char *tmp[5];
  int row=0;
  LUT *lut;
  wnd=Create_Window(widget,550,400,prefer.lut_dir,NULL);
  wl=Create_Clist("Clist",NULL,NULL,NULL,2,
    "LUT",20,"File",20,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),wl->parent);

  gtk_widget_show(wnd);
  if (prefer.lut) Remove_Lut(prefer.lut);
  prefer.lut=load_lutfiles(prefer.lut_dir);
  for (lut=prefer.lut; lut; lut=lut->next)
  {
    tmp[0]=lut->name;
    tmp[1]=lut->fname;
    gtk_clist_append(GTK_CLIST(wl), tmp);
    gtk_clist_set_selectable(GTK_CLIST(wl),row++,FALSE);
  }
}

#define LAB_SELLUT "^Selected"
#define LAB_SELLUTN "^L2"
static GtkWidget *bar;



static void draw_bar(GtkWidget         *widget,
                     GdkEventConfigure *event)
{
  RGBI *rgbi;
  GdkColor clr;
  int x,y,nl;
  Renew_RGBBuf(widget);
  rgbi=Get_RGBI(widget);
  if (!rgbi) return;
  if (rgbi->width<=1) return;
  for (y=0; y<rgbi->height; y++)
  {
    nl=(y*globinfo.lut.nrluts)/rgbi->height;
    for (x=0; x<rgbi->width; x++)
    {
      int xl=x*globinfo.lut.w/rgbi->width;
      clr.red  =clr.green=clr.blue =0;
      if (globinfo.lut.rgb[nl])
      {
        int rgb=globinfo.lut.rgb[nl][xl];
        clr.red  =(rgb>>16)&0xff;
        clr.green=(rgb>>8)&0xff;
        clr.blue =rgb&0xff;
      }
      draw_rgbpoint(rgbi,&clr,x,y);
    }
  }
  Refresh_Rect(widget,0,0,rgbi->width,rgbi->height);
}

static void selected(GtkWidget      *clist,
              gint            row,
              gint            column,
	      GdkEventButton *event,
              gpointer        data)

{
  gchar *slut;
  gtk_clist_get_text(GTK_CLIST(clist), row, 1, &slut);
  if (slut)
  {
    strcpy(globinfo.lut.fname,slut);
    get_lut();
    Set_Entry(clist->parent,LAB_SELLUT,globinfo.lut.name);
    {
      GtkWidget *dr=Find_Widget1(bar,"GTK_DRAWING_AREA");
      draw_bar(dr,NULL);
    }
  }
}


void lfunc(GtkWidget *w,gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_SELLUTN))
  {
    globinfo.lut.sellut=(Get_Button(w,LAB_SELLUTN)? 1 : 0);
    get_lut();
    {
      GtkWidget *dr=Find_Widget1(bar,"GTK_DRAWING_AREA");
      draw_bar(dr,NULL);
    }
  }
}

void select_lut(GtkWidget *widget)
{
  GtkWidget *wnd,*w[3];
  GtkCList *clist;
  char *tmp[5];
  LUT *lut;
  wnd=Create_Window(widget,200,400,prefer.lut_dir,NULL);
  bar=Create_Drawable(0,40,draw_bar,NULL,NULL);
  clist=(GtkCList *)Create_Clist("Clist",selected,NULL,NULL,2,
    "LUT",20,"file",20,NULL);
  w[1]=Create_Entry(LAB_SELLUT,NULL,"%-20s",globinfo.lut.name);
  w[2]=Create_Check(LAB_SELLUTN,lfunc,(globinfo.lut.sellut? TRUE : FALSE));
  w[1]=Pack(NULL,'h',w[1],1,w[2],1,NULL);
  w[0]=Pack(NULL,'v',bar,1,clist,1,w[1],1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),w[0]);

  gtk_widget_show(wnd);
  gtk_clist_set_column_visibility(clist,1,FALSE);
  if (prefer.lut) Remove_Lut(prefer.lut);
  prefer.lut=load_lutfiles(prefer.lut_dir);
  for (lut=prefer.lut; lut; lut=lut->next)
  {
    tmp[0]=lut->name;
    tmp[1]=lut->fname;
    gtk_clist_append(GTK_CLIST(clist), tmp);
  }
  get_lut();
  draw_bar(bar,NULL);

}
#endif
