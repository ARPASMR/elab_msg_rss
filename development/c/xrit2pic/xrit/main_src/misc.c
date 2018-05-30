/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * misc functions
 ********************************************************************/
#include "xrit2pic.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <math.h>

int igamma(int p,int pmax,float g)
{
  float pg=pow((float)p/(float)pmax,1./g);
  return (int)(pg*pmax);
}

/*************************************
 * Wild char comparator
 *************************************/
int strcmpwild(char *s,char *v)
{
  while ((*v) && (*s))
  {
    if (*v=='*')
    {
      v++;
      if (!*v) return 0;                   /* "abc"=="abc*" */
      while ((*s) && (strcmpwild(s,v))) s++;
    }
    if ((*v!='?') && (*v!=*s)) return 1;   /* "abc"=="adb" */
    v++; s++;
  }
  if (*s) return 2;                        /* "abc"=="ab" */
  while (*v=='*') v++;
  if (*v) return 3;                        /* "ab"=="abc" */
  return 0;                                /* "abc"=="a*c" enz. */
} 
     
int strcasecmpwild(char *s,char *v)
{
  while ((*v) && (*s))
  {
    if (*v=='*')
    {
      v++;
      if (!*v) return 0;                   /* "abc"=="abc*" */
      while ((*s) && (strcasecmpwild(s,v))) s++;
    }
    if ((*v!='?') && (tolower(*v)!=tolower(*s))) return 1;   /* "abc"=="adb" */
    v++; s++;
  }
  if (*s) return 2;                        /* "abc"=="ab" */
  while (*v=='*') v++;
  if (*v) return 3;                        /* "ab"=="abc" */
  return 0;                                /* "abc"=="a*c" enz. */
}      

int str_ends_with(char *s,char *e)
{
  return !strcmp(s+strlen(s)-strlen(e),e);
}

#if __GTK_WIN32__ == 1
// NIET helemaal goed, maar voor hier goed genoeg!
char *strcasestr(const char *haystack, const char *needle)
{
  char *p,*q=needle;
  for (p=haystack; *p; p++)
  {
    if (tolower(*p)==tolower(*q)) q++; else q=needle;
    if (*q==0) return p;
  }
  return NULL;
}
#endif
   

gboolean is_ir_chan(CHANNEL *chan)
{
  if (!chan) return FALSE;
  if (!strncasecmp(chan->chan_name,"ir_016",6)) return FALSE;
  if (!strncasecmp(chan->chan_name,"ir",2)) return TRUE;
  if (!strncasecmp(chan->chan_name,"wv",2)) return TRUE;
  return FALSE;
}

gboolean is_ir_group(GROUP *grp)      // one single ir in group
{
  if (!grp) return FALSE;
  if (!grp->chan) return FALSE;
  if (grp->chan->next) return FALSE;  // composite -> ignore 'is_ir'
  return is_ir_chan(grp->chan);
}

int chan_complete(CHANNEL *chan,int chnk_first,int chnk_last)
{
  int i;
  for (i=chnk_first; i<=chnk_last; i++)
    if (!Get_Segm(chan,i)) return 0;              /* Not all needed segments available */
  return 1;
}

void range2int(char *range,int *s1,int *s2)
{
  char *p;
  if (!range) return;
  *s1=atoi(range);
  if ((p=strchr(range,'-')))
    *s2=atoi(p+1);
  else
    *s2=*s1;
}

/* Check if complete.
   Return:  1 if OK
           -1 if prologue HRV missing but otherwise complete
            0 if incomplete
*/
int channel_complete(CHANNEL *chan)
{
  if (chan->nr_segm==chan->seq_end-chan->seq_start+1)
  {
    if ((chan->chan_name) && (!strcmp(chan->chan_name,"HRV")) && (chan->group) && (!chan->group->pro_epi))
      return -1;
    else
      return 1;
  }
  return 0;
}

// If area not full globe: for the moment ensure ALL segments are present!
#define DETECT_LIM_SEGM 0
/* Check if all needed segments are available. Use gmode->chanmap */
/* !!! Toevoegen: Hires: controleer of HRV compleet!!! */

int picture_complete(CHANNEL *chans,PIC_CHARS *pc,GENMODES *gmode)
{
  CHANNEL *chan;
  CHANMAP *chm;
  int i;
  int segm_first,segm_last;
  int has_something=0;

  if (gmode->chanmap)
  {
    if (!gmode->chanmap) return 0;
    for (chm=gmode->chanmap; chm; chm=chm->next)
    {
      // Check if chan available.
      if (!(chan=Get_Chan(chans,chm->chan_name))) return 0;  // chan not available
      segm_first=chan->seq_start;
      segm_last=chan->seq_end;

      #if DETECT_LIM_SEGM != 0
      if (gmode->area_nr)
         segm_first=segm_last-segm_last/4+1;
      #endif

         
      // Check if all needed segments of this channel are available.
      for (i=segm_first; i<=segm_last; i++)
        if (!Get_Segm(chan,i)) return 0;           /* Not all needed segments available */
    }
    return 1;
  }

/* Rest te vervangen door voorgaande */
  if (pc->is_color)
  {
    for (chan=chans; chan; chan=chan->next)
    {
      if ((chan->r) || (chan->g) || (chan->b))
      {
        has_something=1;
        segm_first=chan->seq_start;
        segm_last=chan->seq_end;
        #if DETECT_LIM_SEGM != 0
        if ((gmode) && (gmode->area_nr))
           segm_first=segm_last-segm_last/4+1;
        #endif
        // Check if all needed segments of this channel are available.
        for (i=segm_first; i<=segm_last; i++)
          if (!Get_Segm(chan,i)) return 0;           /* Not all needed segments available */
      }
    }
  }
  else
  {
    chan=chans;
    has_something=1;
    segm_first=chan->seq_start;
    segm_last=chan->seq_end;
    #if DETECT_LIM_SEGM != 0
    if ((gmode) && (gmode->area_nr))
       segm_first=segm_last-segm_last/4+1;
    #endif

    for (i=segm_first; i<=segm_last; i++)
      if (!Get_Segm(chan,i)) return 0;           /* Not all needed segments available */

  }
  return has_something;
}


/* Open temp-file. For Linux: fd returns file destructor. */
int gen_tempfile(char *fnt)
{
  int fd=0;
  #if __GTK_WIN32__ == 1
    char *ifnt;
    if (!(ifnt=tmpnam(NULL)))
    {
      ifnt="xrit2pic_temp.tmp";
    } 
  #else
    char ifnt[50];
    strcpy(ifnt,"/tmp/tmp_xrit2pic_XXXXXX");
    if ((fd=mkstemp(ifnt)) == -1) return -1;
  #endif
  if (fnt) strcpy(fnt,ifnt);
  return fd;
}

FILE *open_tempfile(int *fd,char *fnt)
{
  FILE *fp;
  *fd=gen_tempfile(fnt);
  if (*fd==-1) return NULL;
  if (fnt) fp=fopen(fnt,"wb");
  return fp;
}

/* Close temp-file. For Linux: also close file destructor (fd). */
void close_tempfile(FILE *fp,int fd)
{
  if (fp) fclose(fp);
  #if __GTK_WIN32__ != 1
    if (fd>=0) close(fd);
  #endif
}

int exist_prog(char *fn,char *opt)
{
  char *cmd;
  int err;
  cmd=malloc(strlen(fn)+strlen(opt)+10);
  if (!cmd) return 0;
  sprintf(cmd,"%s %s",fn,opt);
  err=system(cmd);
  free(cmd);
  if (err) return 0;
  return 1;
}

/* Detect if file exist */
int exist_file(char *fn)
{
  FILE *fp;
  if (!(*fn)) return 0;
  if (!(fp=fopen(fn,"r"))) return 0;
  fclose(fp);
  return 1;
}

int exist_file_in_dir(char *dir,char *fn)
{
  char *tmp=NULL;
  int res;
  strcpyd(&tmp,dir);
  finish_path(tmp);
  strcatd(&tmp,fn);
  res=exist_file(tmp);
  free(tmp);
  return res;
}

int sprintfd(char **s,char *f,...)
{
  int len;
  char *s2;
  va_list ap;
  va_start(ap,f);
  len=vsnprintf(NULL,0,f,ap);
  va_end(ap);
  if (len<0) return 0;
  if (!((s2=malloc(len+1)))) return 0;
  va_start(ap,f);
  len=vsprintf(s2,f,ap);
  va_end(ap);
  if (*s) free(*s);
  *s=s2;
  return len;
}

void tfreenull(char **p)
{
  if (*p) free(*p); *p=NULL;
}

int memcpyd(void **sp,void *s,int n)
{
  if (!(*sp=(char *)malloc(n))) return 1;
  memcpy(*sp,s,n);
  return 0;
}

/* Allocate and copy. 2 extra bytes allocated: for closing 0 and maybe 1 extra char to add. */
int strcpyd(char **sp,char *s)
{
  if (!(*sp=(char *)malloc(strlen(s)+2))) return 1;
  strcpy(*sp,s);
  return 0;
}

/* Allocate and concat. 2 extra bytes allocated: for closing 0 and maybe 1 extra char to add. */
int strcatd(char **sp,char *s)
{
  if (!*sp)
  {
    strcpyd(sp,s);
    return 0;
  }
  
  if (!(*sp=(char *)realloc(*sp,strlen(*sp)+strlen(s)+2))) return 1;
  strcat(*sp,s);
  return 0;
}

void strcatc(char *s,char c)
{
  *(s+strlen(s)+1)=0;
  *(s+strlen(s))=c;
}

/*************************************
 * Extract number defined between positions 's' and 'e' from  l
 * and return as float
 *************************************/
#define TMPLEN 300
char *get_strpart(char *l,int s,int e)
{
  static char tmp[TMPLEN];
  int nrb=e-s+1;
  if (nrb>TMPLEN-1) nrb=TMPLEN-1;
  tmp[nrb]=0;
  if (strlen(l+s) < nrb)
  {
    strcpy(tmp,"");
  }
  else
  {
    strncpy(tmp,l+s,nrb);
    tmp[e-s+1]=0;
  }
  return tmp;
}

void strtoupper(char *a)
{
  for (; *a; a++) *a=toupper(*a);
}


void finish_path(char *path)
{
  if (!path) return;
  if (RCHAR(path) != DIR_SEPARATOR)
    strcatc(path,DIR_SEPARATOR);
}

/*************************************
 * Correct tm-struct and return #seconds since 1970.
 * ntz stands for "no time-zone";
 * similar to mktime, except that daylight saving is ignored.
 *************************************/
gint32 mktime_ntz(struct tm *tm)
{
  gint32 secs;
  gint32 rdays;
  int i;
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};

  while (tm->tm_sec >= 60) { tm->tm_min++;  tm->tm_sec-=60; }
  while (tm->tm_sec <   0) { tm->tm_min--;  tm->tm_sec+=60; }
  while (tm->tm_min >= 60) { tm->tm_hour++; tm->tm_min-=60; }
  while (tm->tm_min <   0) { tm->tm_hour--; tm->tm_min+=60; }
  while (tm->tm_hour>= 24) { tm->tm_mday++; tm->tm_hour-=24; }
  while (tm->tm_hour<   0) { tm->tm_mday--; tm->tm_hour+=24; }
  while (tm->tm_mon >= 12) { tm->tm_year++; tm->tm_mon-=12; }
  if (tm->tm_year/4==tm->tm_year/4.) days[1]=29;

  while (tm->tm_mday>days[tm->tm_mon])
  {
    tm->tm_mday-=days[tm->tm_mon]; tm->tm_mon++;
    while (tm->tm_mon>= 12)  { tm->tm_year++; tm->tm_mon-=12; }
    if (tm->tm_year/4==tm->tm_year/4.) days[1]=29; else days[1]=28;
  }
  secs=tm->tm_sec+60*(tm->tm_min+60*(tm->tm_hour+24*(tm->tm_mday-1)));
  for (i=0; i<tm->tm_mon; i++) secs+=(days[i]*24*3600);
  rdays=(tm->tm_year-70)*365;
  rdays+=(tm->tm_year-70+1)/4;
  secs+=(rdays*24*3600);
  tm->tm_wday=rdays%7;
  tm->tm_yday=0;
  for (i=0; i<tm->tm_mon; i++) tm->tm_yday+=days[i];
  tm->tm_yday+=(tm->tm_mday-1);

  return secs;
}

int tm2secs(struct tm *tm)
{
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  int secs,rdays;
  int i;
  secs=tm->tm_sec+60*(tm->tm_min+60*(tm->tm_hour+24*(tm->tm_mday-1)));
  for (i=0; i<tm->tm_mon; i++) secs+=(days[i]*24*3600);
  rdays=(tm->tm_year-70)*365;
  rdays+=(tm->tm_year-70+1)/4;
  secs+=(rdays*24*3600);
  return secs;
}

int difftime_tm(struct tm *tm1,struct tm *tm2)
{
  return tm2secs(tm2)-tm2secs(tm1);
}

/*************************************
 * translate month and day in tm struct into days-in-year  
 *************************************/
void mday_mon2yday(struct tm *tmref)
{
  int i;
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  if (tmref->tm_year/4==tmref->tm_year/4.) days[1]=29;

  tmref->tm_yday=tmref->tm_mday-1;
  if (tmref->tm_mon>=12) tmref->tm_mon=11;
  
  for (i=0; i<tmref->tm_mon; i++)
  {
    tmref->tm_yday+=days[i];
  }

}

/*************************************
 * translate days-in-year in tm struct into month and day
 *************************************/
void yday2mday_mon(struct tm *tmref)
{
  int i;
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  if (tmref->tm_year/4==tmref->tm_year/4.) days[1]=29;

  tmref->tm_mday=tmref->tm_yday+1;
  tmref->tm_mon=0; 
  for (i=0; i<12; i++)
  {
    if (tmref->tm_mday<=days[i]) break;
    tmref->tm_mon++;
    tmref->tm_mday-=days[i];
  }

}

void unexp(char *frmt,...)
{
  va_list arg;

  va_start(arg,frmt);
  fprintf(stderr,"Unexpected: ");
  vfprintf(stderr,frmt,arg);
  fprintf(stderr,"\n");
  va_end(arg);
}

char ftype(char *s)
{
  FILE *fp;
  unsigned char b[4];
  if (is_a_dir(s)) return 'd';
  if (!(fp=fopen(s,"rb"))) return 0;
  if (!(fread(b,2,1,fp))) {  fclose(fp); return 0; }
  fclose(fp);
  if ((b[0]==0x50) && ((b[1]==0x35) || (b[1]==0x36))) return 'p';
  if ((b[0]==0x52) && (b[1]==0x49)) return 'a';
  if ((b[0]==0xff) && (b[1]==0xd8)) return 'j';
  return '?';
}

/*******************************************************
 * Check if file exist and determine overwrite policy
 * mode=0 -> ask to overwrite if file exist
 * mode=1 -> don't ask, overwrite       (REPLACE)
 * mode=2 -> don't ask, don't overwrite (SKIP)
 *******************************************************/ 
int overwrite(char *fn,OVERWRMODE mode)
{
  FILE *fp;
  if (!(fp=fopen(fn,"rb"))) return 1;
  fclose(fp);
  if (mode==REPLACE) return 1;
  if (mode==SKIP) return 0;

#ifndef __NOGUI__
  {
    int n;
    n=Create_Choice("Warning",2,"Overwrite","Don't","File %s exist. Overwrite?",fn);
    if (n==1) return 1;
  }
#endif
  return 0;
}


/*******************************************************
 * Progress bar for gui 
 *******************************************************/ 
int progress_func(char where,char *text,int prgrscnt,int prgrsmax)
{
  static GtkWidget *progress_bar;

  switch(where)
  {
    case 's':
      progress_bar=Create_Progress(NULL,text,TRUE);
    break;
    case 'S':
      progress_bar=Create_Progress(NULL,text,FALSE);
    break;
    case 'w':
      if (Update_Progress(progress_bar,prgrscnt,prgrsmax))
      {
        if (text) Create_Message("Warning",text);
        return 0;
      }
    break;
    case 'e':
      Close_Progress(progress_bar);
    break;
  }
  return 1;
}

// remaining funcs only for gui
#ifndef __NOGUI__
char get_cformat(GtkWidget *widget)
{
  char cformat;
  cformat=Get_Optionsmenu(widget,LAB_FFT);
  if (cformat==1) cformat='p';
  else if (cformat==2) cformat='P';  /* forced to 8-bits PGM */
  else if (cformat==3) cformat='j';
  else if (cformat==4) cformat='J';
  else cformat=0;
  return cformat;
}

void get_genmode_from_gui(GtkWidget *widget,GENMODES *genmode)
{
  genmode->gen_pro=Get_Button(widget,LAB_PROEPI);
  genmode->gen_epi=Get_Button(widget,LAB_PROEPI);
//  genmode->europe=Get_Button(widget,LAB_EUROPE); zit nu buiten routine
  genmode->cformat=get_cformat(widget);
}

/* Determine wdir/file; remove . and .. */
static void update_flist(LIST_FILES *lf)
{
  LIST_FILES *lft,*lftnext;
  if (!lf) return;
  for (lft=lf->next; lft; lft=lftnext)
  {
    lftnext=lft->next;
    if (is_a_dir(lft->pfn))                      /* slow, but not that many files here. */
    {
      lft->is_dir=TRUE;
    }
    if ((!strcmp(lft->fn,".")) || (!strcmp(lft->fn,".")))  // leave .. 
    {
      Destroy_Listitem(lft);                      /* No valid XRIT, so remove */
    }
  }
}

void update_flist1(LIST_FILES *lf)
{
  update_flist(lf);
}


void Load_Dirfilelist(GtkCList *dlist,char *path,char dirfilecode,char *filter)
{
  LIST_FILES *list_files=NULL,*lf;
  list_files=create_filelist(path);    /* Generate a list of all files/dirs */
  update_flist(list_files);
  if (dlist)
  {
    char *tmp[3];
    gtk_clist_clear(dlist);
    for (lf=list_files; lf; lf=lf->next)
    {
      if (!lf->fn) continue;
      if ((filter) && (strcmpwild(lf->fn,filter))) continue;
      if ((dirfilecode!='d') && (!lf->is_dir))
      {
        char ts[100];
        strftime(ts,100,"%F %H:%M",localtime(&lf->time));
        tmp[0]=lf->fn;
        tmp[1]=ts;
        tmp[2]=lf->pfn;
        gtk_clist_append(GTK_CLIST(dlist), tmp);
      }
    }
    gtk_clist_set_sort_column(dlist,0);
    gtk_clist_sort(dlist);
    for (lf=list_files; lf; lf=lf->next)
    {
      if ((dirfilecode!='f') && (lf->is_dir))
      {
        GdkColor clr_dir={0,0,0,0xffff};  /* color dir */
        int row;
//        char nn[9]; int n=1;
        char ts[100];
        strftime(ts,100,"%F %H:%M",gmtime(&lf->time));
//        if (lf->is_dir) n=count_dircontent(lf->pfn);
//        sprintf(nn,"%d",n);
        tmp[0]=lf->fn;
        tmp[1]=ts; // nn;
        tmp[2]=lf->pfn;
        row=gtk_clist_append(GTK_CLIST(dlist), tmp);
        gtk_clist_set_foreground(dlist,row,&clr_dir);
      }
    }
    
  }
  Destroy_List(list_files,TRUE);  // destroy list; it's now in clist
  gtk_clist_set_sort_column(dlist,0);
  gtk_clist_sort(dlist);
}
#define LAB_THISDIR "^Archive dir"
#define LAB_STDDIR "Use Standard path"
#define LAB_DOARCH "Archive"
#define LAB_CANCARCH "Cancel"
static char *rrstr=NULL;
static char rstr[100];

static void close_entrychoice(GtkWidget *widget)
{
  destroy_window(widget);
  Close_Window(widget);
  gtk_main_quit();
}


static void efunc(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  GtkWidget *window=Find_Parent_Window(widget);
  if (!strcmp(name,LAB_THISDIR))
  {
  }

  if (!strcmp(name,LAB_STDDIR))
  {
    Sense_Button(widget,LAB_THISDIR,!Get_Button(widget,LAB_STDDIR));
  }

  if (!strcmp(name,LAB_DOARCH))
  {
    if (Get_Button(widget,LAB_STDDIR))
    {
      *rstr=0;
    }
    else
    {
      strncpy(rstr,Get_Entry(widget,LAB_THISDIR),99);
    }
    rrstr=rstr;
    Close_Window(window);
  }
  if (!strcmp(name,LAB_CANCARCH))
  {
    rrstr=NULL;
    close_entrychoice(window);
  }
}

/* Create window with text entry and ask for choice */
char *Create_Entrychoice(char *title,char *str)
{
  GtkWidget *wnd;
  GtkWidget *w[6];
  *rstr=0;
  wnd=Create_Window(NULL,0,0,title,close_entrychoice);

  w[1]=gtk_label_new(str);
  w[2]=Create_Entry(LAB_THISDIR,efunc,"%-10s","");
  w[3]=Create_Check(LAB_STDDIR,efunc,FALSE);
  w[4]=Create_Button(LAB_DOARCH,efunc);
  w[5]=Create_Button(LAB_CANCARCH,efunc);
  w[2]=Pack(NULL,'v',w[2],1,w[3],1,NULL);
  w[4]=SPack(NULL,"h",w[4],"1",w[5],"E1",NULL);

  w[0]=Pack(NULL,'v',w[1],1,w[2],1,w[4],1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show_all(wnd);
  gtk_main();
  return rrstr;
}
#endif

