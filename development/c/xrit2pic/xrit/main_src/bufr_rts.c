/*******************************************************************
 * Copyright (C) 2008 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "xrit2pic.h"
extern GLOBINFO globinfo;
extern PREFER prefer;

#define LAB_RELPWND "RELATED_PREVWND"

/*****************************************************
 * Common bufr analyze funcs.
 *****************************************************/
// Get bufr section
static int read_section0(FILE *fp)
{
  unsigned char b[10];
  int len,nmbr;
//  printf("----- Section 0 -----\n");
  if (!(fread(b,4,1,fp))) return -1;
  if (strncmp((char *)b,"BUFR",4)) return -2;
  fread(b,4,1,fp);
  len=(b[0]<<16)+(b[1]<<8)+b[2];
  nmbr=b[3];
  return len;
}

// Analyze bufr
char *bufrinfo(GROUP *grp_sel)
{
  static char str[500];
  FILE *fp;
  int n=0,c;
  int len;
  int ehdr;
  if (!(fp=fopen(grp_sel->chan->segm->pfn,"rb"))) return "??";

  if ((len=read_section0(fp))>0)
  {
    ehdr=0;
  }
  else
  {
    ehdr=0x29;
  }
  fseek(fp,ehdr,SEEK_SET);

  while (((len=read_section0(fp))>0))
  {
    n++;
    fseek(fp,-8,SEEK_CUR);
    for (; len>0; len--)
    {
      fgetc(fp);
    }
    if (ehdr)
    {
      fseek(fp,4,SEEK_CUR);
      fseek(fp,ehdr,SEEK_CUR);
    }
    else
    {
      while ((c=fgetc(fp))==0);
      fseek(fp,-1,SEEK_CUR);
    }
  }
  fclose(fp);
  sprintf(str,"%s\nContent: %d BUFR's",grp_sel->chan->segm->fn,n);
  return str;
}

int extract_bufr(FILE *fp,char *fno_base,char *fno_ext)
{
  FILE *fpo;
  char fno[1000];
  int n=0;
  int len,c;
  int ehdr;
  if ((len=read_section0(fp))>0)
  {
    ehdr=0;
  }
  else
  {
    ehdr=0x29;
  }
  fseek(fp,ehdr,SEEK_SET);

  while (((len=read_section0(fp))>0))
  {
    fseek(fp,-8,SEEK_CUR);
    sprintf(fno,"%s_%d%s",fno_base,++n,fno_ext);
    fpo=fopen(fno,"wb");
    for (; len>0; len--)
    {
      c=fgetc(fp);
      fputc(c,fpo);
    }
    fclose(fpo);
    if (ehdr)
    {
      fseek(fp,4,SEEK_CUR);
      fseek(fp,ehdr,SEEK_CUR);
    }
    else
    {
      while ((c=fgetc(fp))==0);
      fseek(fp,-1,SEEK_CUR);
    }
  }
  return n;
}


// Extract info from bufr
// Dir. name MUST end with '/', ALSO for Windows!!! (Buf2Asc)
int translate_bufr(char *ifn,char *ofn)
{
  char cmd[300],env[300];
  #if __GTK_WIN32__ == 1
  char exe[300];
  #endif
  memset(env,0,300);
  #if __GTK_WIN32__ == 1
    if (RCHAR(prefer.bufrtblloc) == '\\') RCHAR(prefer.bufrtblloc)=0;
    if (RCHAR(prefer.bufrtblloc) != '/') strcat(prefer.bufrtblloc,"/");
    if (strchr(prefer.prog_bufrextr,DIR_SEPARATOR))
    {
      sprintf(env,"BUFR_TABLES=%s",prefer.bufrtblloc);
      sprintf(exe,"%s  \"%s\" \"%s\"",prefer.prog_bufrextr,ifn,ofn);
    }
    else
    {
      sprintf(env,"BUFR_TABLES=%s",prefer.bufrtblloc);
      sprintf(exe,"%s\\%s  \"%s\" \"%s\"",prefer.prog_dir,prefer.prog_bufrextr,ifn,ofn);
    }
    if (globinfo.test_bufr)
    {
      char *p;
      sprintf(cmd,"echo \"set %s & %s \"& pause",env,exe);
      system(cmd);
      sprintf(cmd,"set %s & \"%s  & pause",env,exe);
      if (p=strstr(cmd,prefer.prog_bufrextr))
      {
        *(p+strlen(prefer.prog_bufrextr))='"';
      }
      system(cmd);
      return -2;
    }
    else
    {
      if (execcmd(exe,env,NULL,TRUE)) return -1;
    }
  #else
    if (RCHAR(prefer.bufrtblloc) != '/') strcat(prefer.bufrtblloc,"/");
    sprintf(cmd,"export BUFR_TABLES=%s; %s \"%s\" \"%s\" > bufr.log",
       prefer.bufrtblloc,prefer.prog_bufrextr,ifn,ofn);
    if (system(cmd)) return -1;
    if (globinfo.test_bufr) puts(cmd);
  #endif
  return 0;
}

/*****************************************************
 * Scattero analyze funcs.
 *****************************************************/
// Get one scatt line
int get_scatline(char *l,SCATPOINT *sp)
{
  char *p;
  if (*l=='#') return 0;
  if ((p=strtok(l," ")))     sp->lat=atof(p);    else return 0;
  if ((p=strtok(NULL," ")))  sp->lon=atof(p);    else return 0;
  if ((p=strtok(NULL," ")));  // date            else return 0;
  if ((p=strtok(NULL," ")));  // time            else return 0;
  if ((p=strtok(NULL," ")))   sp->speed=atof(p); else return 0;
  if ((p=strtok(NULL," ")))   sp->sdir=atof(p);   else return 0;
  sp->errmessage=NULL;
  return 1;
}

#ifdef NOT_USED_YET
 // Determine # scattero data lines
static int bufr_nr_scattlines(char *ifn,char *ofn)
{
  SCATPOINT spi;
  char l[200];
  int n=0;
  FILE *fp;
  if (translate_bufr(ifn,ofn)) return -3;
  if (!((fp=fopen(ofn,"r")))) return -1;
  while (fgets(l,190,fp))
  {
    if (get_scatline(l,&spi)) n++;
  }
  fclose(fp);
  return n;
}
#endif

// For database scattero
SCATPOINT *Create_Scat(SCATPOINT **si)
{
  SCATPOINT *sn,*s=NULL;
  if (si) s=*si;
  sn=calloc(1,sizeof(*sn));

  if (s)
  {
    while (s->next) s=s->next;
    s->next=sn;
    sn->prev=s;
  }
  else if (si)
  {
    *si=sn;
  }
  return sn;
}

void Remove_Scats(SCATPOINT *sp)
{
  SCATPOINT *spnext;
  for (; sp; sp=spnext)
  {
    if (sp->errmessage) free(sp->errmessage);
    spnext=sp->next;
    free(sp);
  }
}

ASCATPOINT *Create_AScat(ASCATPOINT **si,SCATPOINT *sp)
{
  ASCATPOINT *sn,*s=NULL;
  if (si) s=*si;
  sn=calloc(1,sizeof(*sn));

  if (s)
  {
    while (s->next) s=s->next;
    s->next=sn;
    sn->prev=s;
  }
  else if (si)
  {
    *si=sn;
  }
  sn->sp=sp;
  return sn;
}

void Remove_AScats(ASCATPOINT *sp)
{
  ASCATPOINT *spnext;
  for (; sp; sp=spnext)
  {
    spnext=sp->next;
    free(sp);
  }
}

#define BUFRLOG "bufr.log"  // read-in bufr content
// Extract scattero info from ifn into spx.
// If error: load error emessage into sp.
// return: <0 if error
//         >=0 if OK (val=# of scat-lines read)
static int scat_read(char *fn,SCATPOINT **spx)
{
  SCATPOINT *sp=NULL,spi,*sp1;
  FILE *fp;
  int nrscat=0;
  char l[200];
  char *errmessage=NULL;
  // read-in bufr log info
  if ((fp=fopen(BUFRLOG,"r")))
  {
    while (fgets(l,190,fp))
    {
      if (strstr(l,"ERROR"))
      {
        errmessage=l;
        break;
      }
    }
    fclose(fp);
  }
  
  if (errmessage)
  {
    if ((Create_Scat(&sp)))
    {
      strcpyd(&sp->errmessage,errmessage);
      sp->errmessage[strlen(sp->errmessage)-1]=0; // /n eraf
    }
    *spx=sp;
    return -2;
  }

  // read-in bufr content
  if (!((fp=fopen(fn,"r")))) return -3;
  while (fgets(l,190,fp))
  {
    if (get_scatline(l,&spi))
    {
      if ((sp1=Create_Scat(&sp)))
      {
        SCATPOINT *prev,*next;
        prev=sp1->prev;
        next=sp1->next;
        *sp1=spi;
        sp1->prev=prev;
        sp1->next=next;
        nrscat++;
      }
    }
  }
  fclose(fp);

  *spx=sp;
  return nrscat;
}

/*
Extract bufr info from file ifn.
Result in spx.
Return: >=0: # of items
        -1: error
*/
int scat_extract(char *ifn,SCATPOINT **spx)
{
  int fd;
  char ofn[50];
  int ret;

  // extract bufr to temp-file
  fd=gen_tempfile(ofn);
  close_tempfile(NULL,fd);
  if (translate_bufr(ifn,ofn)) return -1; // bufr2asc failed at all
  ret=scat_read(ofn,spx);                 // ret: # of vals
  remove(ofn);
  return ret;
}

/*
Extract bufr info from file ifn.
Result in ofn
*/
char *scat_extract_to(char *ifn,char *ofn,int *nrp)
{
  static char errmess[80];
  SCATPOINT *sp=NULL;
  *errmess=0;
  errmess[79]=0;
  *nrp=0;
  if ((translate_bufr(ifn,ofn)))
  {
    sprintf(errmess,"Translator %s not found!?",prefer.prog_bufrextr);
    return errmess;
  }
  if (((*nrp)=scat_read(ofn,&sp))<0)
  {
    if ((sp) && (*sp->errmessage))
      strncpy(errmess,sp->errmessage,79);
    else
      strcpy(errmess,"Unknown error.");
  }

  Remove_Scats(sp);
  return errmess;
}



#include <math.h>
SCATPOINT *nearest_scat(POINT *point)
{
  SCATPOINT *sp,*sp1=NULL;
  
  float lon=R2D(point->lon);
  float lat=R2D(point->lat);
  float dist1,dist;
  for (sp=globinfo.scatp; sp; sp=sp->next)
  {
    if (sp==globinfo.scatp)
    {
      dist1=pow(sp->lon-lon,2.)+pow(sp->lat-lat,2.);
      sp1=sp;
    }
    else
    {
      dist=((pow(sp->lon-lon,2.)+pow(sp->lat-lat,2.)));
      if (dist<dist1) { sp1=sp; dist1=dist; }
    } 
  }
  return sp1;
}


int scatt_coverage(GROUP *grp,char *start)
{
  int n;
  SCATPOINT *sp=NULL;
  if (!grp) return -1;
  if (strncmp(grp->sat_src,"ers",3)) return -1;
  if (!grp->chan) return -1;
  if (!grp->chan->segm) return -1;
  if (!grp->chan->segm->pfn) return -1;
  n=scat_extract(grp->chan->segm->pfn,&sp);
  *start=0;
  if (sp)
  {
    sprintf(start,"[%.0f,%.0f]",sp->lon,sp->lat);
    Remove_Scats(sp);
  }
  return n;
}

// rest is only gui-stuff
#ifndef __NOGUI__

#define LAB_EARTHWND2 "ERS-2 winds"
#define LAB_SCATWND "SCATTEROMETER"

void clean_bufrwund(GtkWidget *widget)
{
  int row;
  char *text;
  SCATPOINT *sp;
  GtkWidget *wnd=Find_Parent_Window(widget);
  GtkWidget *iwnd=gtk_object_get_data((gpointer)wnd,LAB_RELPWND);
  gtk_object_remove_data((gpointer)iwnd,LAB_RELPWND);

  widget=Find_Widget(wnd,"Clist");
  for (row=0; ; row++)
  {
    if (!gtk_clist_get_text(GTK_CLIST(widget), row, 0, &text)) break;
    sp=(SCATPOINT *)gtk_clist_get_row_data(GTK_CLIST(widget),row);
    Remove_Scats(sp);
    Remove_AScats(globinfo.ascatp); globinfo.ascatp=NULL;
  }

  Close_Window(Find_Window(Find_Parent_Window(widget),LAB_EARTHWND2));
  destroy_window(widget);
  globinfo.scatp=NULL;
}

static int drawfunc2(GtkWidget *widget);

// Select one scatt. data-set
static void selected(GtkWidget      *clist,
              gint            row,
              gint            column,
	      GdkEventButton *event,
              gpointer        data)

{
  gchar *stime,*sdate;
  GtkWidget *prev_wnd=gtk_object_get_data((gpointer)Find_Parent_Window(clist),LAB_RELPWND);
  GtkWidget *mapwnd=Find_Window(Find_Parent_Window(clist),LAB_EARTHWND2);
  GtkWidget *drawable=NULL;
  if (mapwnd) drawable=gtk_object_get_data((gpointer)mapwnd,"AARDEMAP");
  gtk_clist_get_text(GTK_CLIST(clist), row, 1, &stime);
  gtk_clist_get_text(GTK_CLIST(clist), row, 2, &sdate);

  globinfo.scatp=(SCATPOINT *)gtk_clist_get_row_data(GTK_CLIST(clist),row);
  draw_pic(prev_wnd);
  if (drawable) drawfunc2(drawable);
}


#define OPEN_MAPSCAT "Open map"
#define PLOT_SCATALL "Plot all"
static void bufrfunc(GtkWidget *widget,gpointer data)
{
  GtkWidget *prev_wnd=gtk_object_get_data((gpointer)Find_Parent_Window(widget),LAB_RELPWND);
  char *name=(char *)data;
  if (!strcmp(name,OPEN_MAPSCAT))
  {
    open_earthmap(NULL,LAB_EARTHWND2,prefer.earthmapfile2,Find_Parent_Window(widget),NULL,drawfunc2,NULL,NULL);
  }
  if (!strcmp(name,PLOT_SCATALL))
  {
    globinfo.scat_plot_all=Get_Button(widget,PLOT_SCATALL);
    draw_pic(prev_wnd);
  }
}

// Create list of scattero data
void handle_bufr(GtkWidget *iwnd,GROUP *igrp)
{
  GtkWidget *wnd,*w[9];
  SCATPOINT *sp=NULL,*sp1;
  GROUP *grp;
  char *tmp[7];
  int row=0;
  int n=0;
  int nr=0;
  char title[100];
/*
  strcpy(title,"SCAT_");
  strncat(title,get_wndtitle(iwnd),90);
*/
  strcpy(title,LAB_SCATWND);  // 1 window for scatterometer

  globinfo.scatp=NULL;
  wnd=Create_Window(iwnd,500,300,title,clean_bufrwund);
  if (!wnd) return;
  gtk_object_set_data((gpointer)wnd,LAB_RELPWND,iwnd);
  gtk_object_set_data((gpointer)iwnd,LAB_RELPWND,wnd);
  w[1]=Create_Clist("Clist",selected,NULL,NULL,7,"nr",2,"Source",8,"Time",8,"Date",8,"pos1",8,"pos2",8,"info",8,NULL);
//  gtk_clist_set_selection_mode(w[1],GTK_SELECTION_EXTENDED);
  w[2]=Create_Button(OPEN_MAPSCAT,bufrfunc);
  w[3]=Create_Check(PLOT_SCATALL,bufrfunc,globinfo.scat_plot_all);
  w[2]=SPack(NULL,"h",w[2],"1",w[3],"E1",NULL);
  w[0]=SPack(NULL,"v",w[1],"ef1",w[2],"1",NULL);

  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show_all(wnd);

  for (grp=igrp; grp; grp=grp->next)
  {
    char strtime[50],strdate[50];
    if ((grp->h_or_l=='B') && (!strncmp(grp->sat_src,"ers",3)))
    {
      char str[4][80];
      int ret=0;
      *str[0]=*str[1]=*str[2]=0;
      str[2][79]=0;
      if ((ret=scat_extract(grp->chan->segm->pfn,&sp))<=0)
      {
        if (ret<0)
        {
          Create_Message("Error","bufr extraction failed:\nprogram '%s' missing?",prefer.prog_bufrextr);
          break;
        }
      }
      n=0; for (sp1=sp; ((sp1)&&(sp1->next)); sp1=sp1->next) n++;
      if (!n)
      {
        Remove_Scats(sp);
        continue;
      }
      Create_AScat(&globinfo.ascatp,sp);
      if ((sp) && (sp1))
      {
        sprintf(str[0],"[%d,%d]",(int)sp->lon,(int)sp->lat);
        sprintf(str[1],"[%d,%d]",(int)sp1->lon,(int)sp1->lat);
      }

      if ((sp) && (sp->errmessage))
        strncpy(str[2],sp->errmessage,79);
      else
        sprintf(str[2],"# values=%d",n);
      sprintf(str[3],"%d",++nr);
      strftime(strtime,20,"%H:%M  ",&grp->grp_tm);             /* time */
      strftime(strdate,20,"%d-%m-%y  ",&grp->grp_tm);          /* date */
      tmp[0]=str[3];
      tmp[1]=grp->sat_src;
      tmp[2]=strtime;
      tmp[3]=strdate;
      tmp[4]=str[0];
      tmp[5]=str[1];
      tmp[6]=str[2];
      gtk_clist_append(GTK_CLIST(w[1]), tmp);
      gtk_clist_set_row_data(GTK_CLIST(w[1]),row++,(gpointer)sp);
    }
  }
}

#define CIJFERS
static int drawfunc2(GtkWidget *widget)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  GdkColor clr;
  GtkWidget *clist;
  int row,srow;
  SCATPOINT *sp;
  int x,y;
  char *text;
  guchar *aarde=gtk_object_get_data((gpointer)wnd,"PICDATA");
  GtkWidget *drawable=gtk_object_get_data((gpointer)wnd,"AARDEMAP");
  RGBI *rgbi;
  RGBPICINFO *rgbpi;
  if (drawable==NULL) return 0;

  clist=Find_Widget(Find_Window(widget,LAB_SCATWND),"Clist");
  rgbi=Get_RGBI(widget);
  rgbpi=Get_RGBPI(widget);
  Renew_RGBBuf(drawable);

  if ((aarde) && (rgbi) && (rgbpi))
    draw_backgrnd(aarde,rgbi,rgbpi);

  if (clist)
  {
    if (GTK_CLIST(clist)->selection)
      srow=(int)(GTK_CLIST(clist)->selection->data);
    else
      srow=0;

    for (row=0; ; row++)
    {
      if (!gtk_clist_get_text(GTK_CLIST(clist), row, 0, &text)) break;
      sp=(SCATPOINT *)gtk_clist_get_row_data(GTK_CLIST(clist),row);
      if (srow==row)
      {
        clr.red  =((prefer.scat_selclr>>8)&0xf)<<4;
        clr.green=((prefer.scat_selclr>>4)&0xf)<<4;
        clr.blue =((prefer.scat_selclr)&0xf)<<4;
      }
      else
      {
        clr.red  =((prefer.scat_clr>>8)&0xf)<<4;
        clr.green=((prefer.scat_clr>>4)&0xf)<<4;
        clr.blue =((prefer.scat_clr)&0xf)<<4;
      }

      for (; sp; sp=sp->next)
      {
        float lon2,lat2;
        int x2,y2;
        pcm_lonlat2xy(rgbi,rgbpi,sp->lon,sp->lat,&x,&y);
        lon2=sp->lon-sp->speed/30.*sin(D2R(sp->sdir));  // sdir=90 -> to west
        lat2=sp->lat-sp->speed/30.*cos(D2R(sp->sdir));  // sdir= 0 -> to south
        pcm_lonlat2xy(rgbi,rgbpi,lon2,lat2,&x2,&y2);

        draw_scatarrow(NULL,rgbi,NULL,&clr,x,y,x2,y2);
      }
#ifdef CIJFERS
      {
        char txt[10];
        sprintf(txt,"%d",row+1);
        clr.red=0x00;  clr.green=0xff;  clr.blue=0x00;
        draw_rgbstring(rgbi,&clr,x,y,txt);
      }
#endif
    }
  }

  Refresh_Rect(drawable,0,0,rgbi->width,rgbi->height);
  return 0;
}

static int drawfunc1(GtkWidget *widget)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  GdkColor clr;
  SCATPOINT *sp;
  int x,y;
  guchar *aarde=gtk_object_get_data((gpointer)wnd,"PICDATA");
  GtkWidget *drawable=gtk_object_get_data((gpointer)wnd,"AARDEMAP");
  RGBI *rgbi;
  RGBPICINFO *rgbpi;
  if (drawable==NULL) return 0;

  rgbi=Get_RGBI(widget);
  rgbpi=Get_RGBPI(widget);
  Renew_RGBBuf(drawable);

  if ((aarde) && (rgbi) && (rgbpi))
    draw_backgrnd(aarde,rgbi,rgbpi);

  if ((sp=gtk_object_get_data((gpointer)wnd,"PLOTDATA")))
  {
    for (; sp; sp=sp->next)
    {
      float lon2,lat2;
      int x2,y2;
      clr.red  =((prefer.scat_selclr>>8)&0xf)<<4;
      clr.green=((prefer.scat_selclr>>4)&0xf)<<4;
      clr.blue =((prefer.scat_selclr)&0xf)<<4;
      pcm_lonlat2xy(rgbi,rgbpi,sp->lon,sp->lat,&x,&y);
      lon2=sp->lon-sp->speed/30.*sin(D2R(sp->sdir));  // sdir=90 -> to west
      lat2=sp->lat-sp->speed/30.*cos(D2R(sp->sdir));  // sdir= 0 -> to south
      pcm_lonlat2xy(rgbi,rgbpi,lon2,lat2,&x2,&y2);

      draw_scatarrow(NULL,rgbi,NULL,&clr,x,y,x2,y2);
    }
  }

  Refresh_Rect(drawable,0,0,rgbi->width,rgbi->height);
  return 0;
}

/*
dump_from_rgbbuf(RGBPICINFO *rgbpi)
{
  FILE *fp;
  int x,y;
  if (!((fp=fopen("xx.ppm","wb")))) return;
  write_pgmhdr(fp,rgbi->width,rgbi->height,8,1,NULL);
  for (y=0; y<rgbpi->height; y++)
    for (x=0; x<rgbpi->width; x++)
    {
      rgbpicinfo->str8 
    }
}
*/
static void keyfunc(GtkWidget *widget,gpointer data)
{
/*
  if (data=='w')
  {
    GtkWidget *wnd=Find_Parent_Window(widget);
    RGBI *rgbi=Get_RGBI(widget);
    RGBPICINFO *rgbpi=Get_RGBPI(widget);
printf("%x  %x %x\n",wnd,  rgbpi,gtk_object_get_data(GTK_OBJECT(wnd),"AARDEMAP"));
dump_from_rgbbuf(rgbpi);
        
  
  }
*/
}

int open_earthmap1(GtkWidget *window,GROUP *grp)
{
  SCATPOINT *sp;
  int n;
  n=scat_extract(grp->chan->segm->pfn,&sp);
  if (n>0)
    open_earthmap(NULL,grp->pc.picname,prefer.earthmapfile2,window,NULL,drawfunc1,keyfunc,sp);
  return n;
}
#endif
