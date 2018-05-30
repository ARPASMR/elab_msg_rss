#define HRPTLIN_NW
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "xrit2pic.h"
#include "avhrr.h"

extern GLOBINFO globinfo;
extern PREFER prefer;

/*************************************************************************
 *************************************************************************
 ** Bunzip functions.
 *************************************************************************
 *************************************************************************/

/*************************************
 * Bunzip2 file 'pfn' into 'fn'
 *    fn==NULL: decompress with filename (without .bz2 extension)
 *************************************/
#define NO_STDOUT_FOR_MEMPROFN
static int bunzip2_file(char *pfn,char *fn)
{
  char cmd[300];
  int err=0;
  if (strstr(pfn,".bz2"))
  {
    if (!fn)
    {
      sprintf(cmd,"%s -q -d  %s",globinfo.prog_bzip2,pfn);
      if (system(cmd)) err=Bzip2_Err;
    }
    else
    {
#ifdef NO_STDOUT_FOR_MEMPROF
      char *p;
      sprintf(cmd,"%s -q -d -k %s",globinfo.prog_bzip2,pfn);
      err=system(cmd);
      strcpy(cmd,pfn);
      if (p=strstr(cmd,".bz2")) *p=0;
      rename(cmd,fn);
#else
      sprintf(cmd,"%s -q -d -c \"%s\" > %s",globinfo.prog_bzip2,pfn,fn);
      #if __GTK_WIN32__ == 1
        #define BZIPCMDFILE "\\xrbzcmd.bat"
        #define BZIPCMDFILE2 "xrbzcmd.bat"
        {
          FILE *fp;
          char *cmdzip=NULL;
          cmdzip=BZIPCMDFILE;
          if (!(fp=fopen(cmdzip,"w")))
          {
            cmdzip=BZIPCMDFILE2;
            if (!(fp=fopen(cmdzip,"w")))
            {
              return Open_Wrt;
            }
          }
          fprintf(fp,"@echo off\n");
          fprintf(fp,cmd);
          fclose(fp);
          if (execcmd(cmdzip,NULL,NULL,TRUE)) err=Bzip2_Err;
  //        if (system(cmdzip)) err=Bzip2_Err;
          remove(cmdzip);
        }
      #else
        if (system(cmd)) err=Bzip2_Err;
      #endif
    if (err) remove(fn);
#endif
      }
  }
  return err;
}

/*************************************************************************
 *************************************************************************
 ** External functions.
 *************************************************************************
 *************************************************************************/

/*************************************
 * Bunzip2 file 'ifn' into a temp-file (returned in ofn)
 * ofn should have space for ~ 50 chars.
 * return: 0 if OK
 *************************************/
int bunzip_to_tempfile(char *ifn,char *ofn)
{
  int err=0;
  int fd=0;
  #if __GTK_WIN32__ == 1
    char *fnt;
  #else
    static char fnt[50];
  #endif

  if (ofn)
  {
  /* Create and open temp. file. */
    #if __GTK_WIN32__ == 1
      if ((!(fnt=tmpnam(NULL))) || (!(create_file(fnt))))
      {
        fnt="xrit2pic_temp.tmp";
        strcpy(ofn,fnt);
        if (!(create_file(fnt))) return Tmp_fn;
      } 
    #else
      strcpy(fnt,"/tmp/tmp_xrit2pic_XXXXXX");
      if ((fd=mkstemp(fnt)) == -1) return Tmp_fn;
    #endif
    /* Bunzip to temp-file */
    err=bunzip2_file(ifn,fnt);
    #if __GTK_WIN32__ != 1
      close(fd);
    #endif

    strcpy(ofn,fnt);
  }
  else
  {
    err=bunzip2_file(ifn,NULL);        // unzip
//    if ((p=strstr(ifn,".bz2"))) *p=0;  // remove extension from input filename
  }
  return err;
}

struct tm get_time(FILE *fpi,int year)
{
  struct tm tm;
  long t,tp,tms;
  int d,dp;
  int y;
  guint16 l[20];
  memset(&tm,0,sizeof(tm));
  t=tp=0;
  d=dp=-1;
  for (y=0; y<100; y++)
  {
    fseek(fpi,(y*11090 )*2 ,SEEK_SET);
    fread(l,2,12,fpi);
    d=GUINT16_FROM_BE(l[8])>>1;
    t=((GUINT16_FROM_BE(l[9])&0x7f)<<20) + (GUINT16_FROM_BE(l[10])<<10) + (GUINT16_FROM_BE(l[11]));
    if (((t-tp==166) || (t-tp==167)) && (d==dp)) break;
    tp=t;
    dp=d;
  }
  t-=y*167;
  tm.tm_year=year-1900;
  tm.tm_yday=d;
  tm.tm_hour=t/1000/3600;
  tm.tm_min=(t-tm.tm_hour*3600000)/1000/60;
  tm.tm_sec=(t-tm.tm_hour*3600000 - tm.tm_min*60000)/1000;
  tms=(t-tm.tm_hour*3600000-tm.tm_min*60000-tm.tm_sec*1000);
  yday2mday_mon(&tm);
  return tm;
}

/*************************************
 * Translate avhrr into  guint16 string.
 * Get all 5 channels and
 * connect each of them with a segments in one of 5 chan's.
 *************************************/
int noaa2str(SEGMENT *segm,
             gboolean get_allchan,
             guint16 *owidth,guint16 *oheight,
             PIC_CHARS *pc)
{
//  static int *xtbl;
  CHANNEL *chan;
  FILE *fpi;
  guint16 *nstr[AVHRR_NRCHANS];
  guint16 pix[AVHRR_NRCHANS];
  int x,y,i,x1,xp;
  int err=0;
  int width,height;
  int iheight;   // height for this segment
  char fnt[200];
  if (segm->chan->nc==0) return -1;
  if (segm->chan->nl==0) return -1;
  
  memset(nstr,0,sizeof(*nstr)*AVHRR_NRCHANS);

  /* Expected size in this segment (see init_noaachars) */  
  width=segm->chan->nc;  // segm->xh.nc;
  height=segm->chan->nl; // segm->xh.nl;
  *owidth=width;                         /* Output pic width */
  *oheight=height;                         /* Output pic width */
  
  /* Bunzip2 into a temp file */
  if (str_ends_with(segm->pfn,".bz2"))
  {
    if ((err=bunzip_to_tempfile(segm->pfn,fnt)))
    {
      segm->corrupt=TRUE;             /* cleared with new generation */
      return err;
    }
    if (!(fpi=fopen(fnt,"rb")))
    {
      remove(fnt);                    /* Remove temp file */
      return Open_Rd;                 /* can't open file */
    }
  }
  else
  {
    *fnt=0;
    if (!(fpi=fopen(segm->pfn,"rb")))
      return Open_Rd;                 /* can't open file */
  }  
  /* Determine actual # lines (may be lower for first/last segment)
     HRPT contains 11090 words per line; each word is 2 bytes.
  */
  fseek(fpi,0,SEEK_END);
  iheight=ftell(fpi)/(11090*2);       // actual # lines of this segment

/* first and last segm may have lower amount of lines.
   Make missing lines'black'.
   Startpoint > 0 only for first segment -> ystart
*/
  if (segm->xh.segment==1)
  {
    segm->y_offset=height - iheight;
  }
  else
  {
    segm->y_offset=0;
  }


/*
  Allocate mem for channels to process.
  This is either 1 channel (chan->chan_nr=1...5) or all 5 channels.
*/
  for (chan=segm->chan; chan->prev; chan=chan->prev);
  for (; chan; chan=chan->next)
  {
    nstr[chan->chan_nr-1]=calloc((width*height),2); // cleared array
  }
#ifdef TODO  
//Get actual time from HRPT-data.
  get_time(fpi,year);
#endif

// Laatste segment geeft geen zwart op einde!
/* Draw for actual height. Total height maybe bigger; difference is black. */
  for (y=0; y<iheight; y++)
  {
    fseek(fpi,(y*11090 + 750)*2 ,SEEK_SET);
    xp=-1;
    for (x1=0; x1<width; x1++)
    {
      /* Read a pixel (all 5 channels) */
      x=x1;
      if (!(fread(pix,2,5,fpi))) { x1=-1; break; }

      /* Store pix in string. Only for channels for which nstr is allocated. */
      for (i=0; i<AVHRR_NRCHANS; i++)
      {
        if (nstr[i])
          *(nstr[i]+(x1+(segm->y_offset+y)*width)*1)=GUINT16_FROM_BE(pix[i]);
      }
    }
    if (x1<0) break;       /* EOF; stop */
  }

  fclose(fpi);
  if (*fnt) remove(fnt);                /* Remove temp file */

/* Connect nstr to correct segm for all channels */
  {
    int nrsegm=segm->xh.segment;
    SEGMENT *segml;
    chan=segm->chan;
    while (chan->prev) chan=chan->prev;
    for (; chan; chan=chan->next)
    {
      for (segml=chan->segm; segml; segml=segml->next)
      {
        if (segml->xh.segment==nrsegm)
        {
          if (get_allchan)
          {
            segml->chnk=nstr[chan->chan_nr-1];
            segml->width=width;
            segml->height=height;   // take max. segment height, also if partial filled!
          }
          else
          {
            if (segml==segm)
            {
              segml->chnk=nstr[chan->chan_nr-1];
              segml->width=width;
              segml->height=height;
            }
            else
            {
              free(nstr[chan->chan_nr-1]);
            }
          }
          break;
        }
      }
    }
  }
  return 0;
}

