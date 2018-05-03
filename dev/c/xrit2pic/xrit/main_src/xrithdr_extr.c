#define HRPTLIN_NW
/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Basic functions to extract info from XRIT files
 ********************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "vcdu.h"
#include "xrit2pic.h"
#include "avhrr.h"
/*************************************
 * Extraction XRIT header info 
 * 
 *************************************/
extern char *channellist[];

/*************************************
 * translate channel name into number 
 * 
 *************************************/
/* Note: Only info in file-name known here! */
static void channame2nr(XRIT_HDR *xrit_hdr)
{
  if      (xrit_hdr->special=='p')           xrit_hdr->chan_nr=0;
  else if (xrit_hdr->special=='P')           xrit_hdr->chan_nr=0;
  else if (xrit_hdr->special=='e')           xrit_hdr->chan_nr=0;
  else if (xrit_hdr->special=='E')           xrit_hdr->chan_nr=0;
  else if (!strcmp(xrit_hdr->chan,"VIS006")) xrit_hdr->chan_nr=1;
  else if (!strcmp(xrit_hdr->chan,"VIS008")) xrit_hdr->chan_nr=2;
  else if (!strcmp(xrit_hdr->chan,"IR_016")) xrit_hdr->chan_nr=3;
  else if (!strcmp(xrit_hdr->chan,"IR_039")) xrit_hdr->chan_nr=4;
  else if (!strcmp(xrit_hdr->chan,"WV_062")) xrit_hdr->chan_nr=5;
  else if (!strcmp(xrit_hdr->chan,"WV_073")) xrit_hdr->chan_nr=6;
  else if (!strcmp(xrit_hdr->chan,"IR_087")) xrit_hdr->chan_nr=7;
  else if (!strcmp(xrit_hdr->chan,"IR_097")) xrit_hdr->chan_nr=8;
  else if (!strcmp(xrit_hdr->chan,"IR_108")) xrit_hdr->chan_nr=9;
  else if (!strcmp(xrit_hdr->chan,"IR_120")) xrit_hdr->chan_nr=10;
  else if (!strcmp(xrit_hdr->chan,"IR_134")) xrit_hdr->chan_nr=11;
  else if (!strcmp(xrit_hdr->chan,"HRV"))    xrit_hdr->chan_nr=12;
/* MTP */
  else if (!strcmp(xrit_hdr->chan,"VIS007")) xrit_hdr->chan_nr=1;
  else if (!strcmp(xrit_hdr->chan,"IR_064")) xrit_hdr->chan_nr=2;
  else if (!strcmp(xrit_hdr->chan,"WV_064")) xrit_hdr->chan_nr=2;
  else if (!strcmp(xrit_hdr->chan,"IR_115")) xrit_hdr->chan_nr=3;
/* GOES */
  else if (!strcmp(xrit_hdr->chan,"IR_066")) xrit_hdr->chan_nr=2;
  else if (!strcmp(xrit_hdr->chan,"IR_068")) xrit_hdr->chan_nr=2;
  else if (!strcmp(xrit_hdr->chan,"IR_107")) xrit_hdr->chan_nr=3;
  else if (!strcmp(xrit_hdr->chan,"VIS"))    xrit_hdr->chan_nr=1;
  else if (!strcmp(xrit_hdr->chan,"IR1"))    xrit_hdr->chan_nr=2;
  else if (!strcmp(xrit_hdr->chan,"IR2"))    xrit_hdr->chan_nr=3;
  else if (!strcmp(xrit_hdr->chan,"IR3"))    xrit_hdr->chan_nr=4;
  else if (!strcmp(xrit_hdr->chan,"IR4"))    xrit_hdr->chan_nr=5;
  else if (isdigit(*(xrit_hdr->chan+3)))     xrit_hdr->chan_nr=atoi(xrit_hdr->chan+3); /* not defined here */
  else                                       xrit_hdr->chan_nr=-1; /* not defined here */
}

static void nr2channame(XRIT_HDR *xrit_hdr)
{
  if ((xrit_hdr->chan_nr>=1) && (xrit_hdr->chan_nr <=12))
  {
    strcpy(xrit_hdr->chan,channellist[xrit_hdr->chan_nr-1]);
  } 
}

/* Remove trailing underscores */
static void remove_tr_usc(char *s)
{
  char *p;
  p=s+strlen(s)-1;
  while ((p>=s) && (*p=='_'))
  {
    *p=0;
    p--;
  } 
}
#define NO_SATPOS 9999
/*************************************
  Change channel names to make it more clear and to match with overlay files
  Format: e.g. 00_7_000E: 00_7 = spectr, 000E=pos
  Save sat-pos from name, in case it is needed for subsat pos. lateron
 *************************************/
static void change_channelnames(XRIT_HDR *xh)
{
  char ch[10];
  if (isdigit(xh->chan[0]))
  {
    xh->satpos=NO_SATPOS; // code: no satellite position
    /* Extract position */
    xh->satpos=atoi(xh->chan+5);
    if (strchr(xh->chan+5,'W'))
      xh->satpos*=-1;

    /* Remove position */
    *(xh->chan+5)=0;
    if (xh->chan[2]=='_')
    {
      sprintf(ch,"%c%c%c",xh->chan[0],xh->chan[1],xh->chan[3]);
    }
    else
    {
      strncpy(ch,xh->chan,4);
      xh->chan[4]=0;
    }
    if (atoi(ch) < 9)
      sprintf(xh->chan,"VIS%s",ch);
    else if ((atoi(ch) > 61) && (atoi(ch) < 74))
      sprintf(xh->chan,"WV_%s",ch);
    else
      sprintf(xh->chan,"IR_%s",ch);
    
  }
  xh->wavel=atoi(xh->chan+3);
}

/*************************************
 * Extract annotation info MSG
 * Examples:
 * 
 * L-000-MSG1__-GOES7_______-IR_107___-00004____-200202020202-CE
 * L-000-MSG1__-MSG1________-IR_016___-00001____-200202020202-CE
 * H-000-MSG1__-MSG1________-_________-EPI______-200305040944-__
 * H-000-MSG1__-MSG1________-_________-PRO______-200305040914-__
 *************************************/
static int extract_anno_msg(XRIT_HDR *xh)
{
  char *p;
  char tmp[20];
  char anno[1000];
  strcpy(anno,xh->anno);
  if (!strchr(anno,'-')) return 0;
  if (!(p=strtok(anno,"-"))) return 0;      /* L or H */
  xh->hl=*p; 

  if (!(p=strtok(NULL,"-"))) return 0;      /* version */
  strncpy(xh->vers,p,4);

  if (!(p=strtok(NULL,"-"))) return 0;      /* satellite name; MSG or MTP */
  strncpy(xh->sat,p,7);                     /* copy 1 more to get closing 0 */
  remove_tr_usc(xh->sat);

  if (!(p=strtok(NULL,"-"))) return 0;      /* ID#1: data source (12 chars) */
  strncpy(xh->src,p,13);    
  remove_tr_usc(xh->src);

  if (!strncmp(xh->src,"MSG",3))
    strcpy(xh->satsrc,xh->src);
  else if (!strncmp(xh->src,"SERVICE",7))
    strcpy(xh->satsrc,"Srvc");
  else if (!strncmp(xh->src,"MPEF",7))
    strcpy(xh->satsrc,xh->src);
  else if (!strncmp(xh->src,"MET",3))
    strcpy(xh->satsrc,xh->src);
  else if (!strncmp(xh->src,"GOES",4))
    strcpy(xh->satsrc,xh->src);
  else if (!strncmp(xh->src,"MTSAT",5))
    strcpy(xh->satsrc,xh->src);
  else
    strcpy(xh->satsrc,"Frgn");

  if (!(p=strtok(NULL,"-"))) return 0;      /* ID#2: channel (9 chars) */
  strncpy(xh->chan,p,10);
  remove_tr_usc(xh->chan);

  if ((p=strtok(NULL,"-")))                 /* ID#3: nr. or PRO/EPI (9 chars) */
  {
    xh->special=0;
    if (!strncmp(p,"PRO_",4))
    {
      if (!*xh->chan)
        xh->special='P';                    /* group PRO */
      else
        xh->special='p';                    /* channel PRO */
    }
    else if (!strncmp(p,"EPI_",4))
    {
      if (!*xh->chan)
        xh->special='E';                    /* group EPI */
      else
        xh->special='e';                    /* channel EPI */
    }
    else if (!strcmp(xh->satsrc,"Srvc"))    /* service: segmentnr=servicenr */
    {  /* ignore */
      xh->segment=0;
    }
    else
    {
      xh->segment=atoi(p); 
    }
  }
/*
  if (!*xh->chan)
  {
    if (xh->special=='p') strcpy(xh->chan,"PRO");
    if (xh->special=='e') strcpy(xh->chan,"EPI");
  }
*/
  if ((p=strtok(NULL,"-")))                  /* prod. ID#4: time (12 chars)  */
  {
    strcpy(xh->itime,p);
    memset(&xh->time,0,sizeof(xh->time));
    strncpy(tmp,p,4); tmp[4]=0; p+=4;
    xh->time.tm_year=atoi(tmp)-1900;

    strncpy(tmp,p,2); tmp[2]=0; p+=2;
    xh->time.tm_mon=atoi(tmp)-1;

    strncpy(tmp,p,2); tmp[2]=0; p+=2;
    xh->time.tm_mday=atoi(tmp);

    strncpy(tmp,p,2); tmp[2]=0; p+=2;
    xh->time.tm_hour=atoi(tmp);

    strncpy(tmp,p,2); tmp[2]=0; p+=2;
    xh->time.tm_min=atoi(tmp);

/*
Don't use this; time gets confused because of daylight saving!
    mktime(&xh->time);
*/
    mday_mon2yday(&xh->time);                /* alternative for mktime */

  }
  xh->tlen=0;
  if ((p=strtok(NULL,"-")))                  /* flags */
  {
    xh->compr=xh->encry='_';
    if (strchr(p,'C')) xh->compr='C';
    if (strchr(p,'E')) xh->encry='E';
  }

/* Determine sort-order number: [yyyymmddhhmm][t][c] MUST be always equal length! */
  channame2nr(xh);
  if (xh->chan_nr>=0)
    sprintf(xh->sortn,"%s%c%x%02x",xh->itime,xh->hl,xh->chan_nr,xh->segment);
  else
    sprintf(xh->sortn,"%s%c%s%02x",xh->itime,xh->hl,xh->chan,xh->segment);
  sprintf(xh->id,"%-10s %c   ",xh->chan,xh->hl);
  strftime(xh->id+14,20,"%d-%m-%y %H:%M  ",&xh->time);             /* time */
  change_channelnames(xh);
  return 1;
}


/*************************************
 * Extract annotation info AVHRR METOP
 * Examples:
 * 
 * AVHR_xxx_1B_M02_20060528092503Z_20060528092803Z_N_O_20060528093309Z.bz2
 *    xxx=
 *    1B
 *    MO2
 *    200605280925   starttime
 *    03Z
 *    200605280928   stoptime
 *    03Z
 *    N_O
 *    200605280933
 *    09Z
 *************************************/
static int extract_anno_mtp(XRIT_HDR *xh)   /*METOP*/
{
  int n;
  xh->hl='M';                               /* metop */
  xh->segment=1; 
  xh->image_iformat='Z';
  xh->file_type=0;                          /* means: pic */
  xh->scan_dir='n';                         /* maybe 's'? */

  n=12;
  
  strcpy(xh->satsrc,"METOP"); 
  if (!strncmp(get_strpart(xh->anno,n+0,n+3),"M02",3))
  {
    if (!strncmp(xh->anno,"AVHR_HRP",strlen("AVHR_HRP")))
      strcpy(xh->satsrc,"metop-A");
    else
      strcpy(xh->satsrc,"METOP-A");
  }
  else if (!strncmp(get_strpart(xh->anno,n+0,n+3),"M01",3))
    strcpy(xh->satsrc,"METOP-1");
  else  if (!strncmp(get_strpart(xh->anno,n+0,n+3),"N1",2))
  {
    sprintf(xh->satsrc,"NOAA%s",get_strpart(xh->anno,n+1,n+2));
    xh->hl='G';
  }
  else
    strcpy(xh->satsrc,"METOP-?"); 
  
  n=16;
  xh->time.tm_year=atoi(get_strpart(xh->anno,n+0,n+3))-1900;
  xh->time.tm_mon=atoi(get_strpart(xh->anno,n+4,n+5))-1;
  xh->time.tm_mday=atoi(get_strpart(xh->anno,n+6,n+7));
  xh->time.tm_hour=atoi(get_strpart(xh->anno,n+8,n+9));
  xh->time.tm_min=atoi(get_strpart(xh->anno,n+10,n+11));
  xh->time.tm_sec=0;
  mday_mon2yday(&xh->time);                /* alternative for mktime */

  n=32;
  xh->etime.tm_year=atoi(get_strpart(xh->anno,n+0,n+3))-1900;
  xh->etime.tm_mon=atoi(get_strpart(xh->anno,n+4,n+5))-1;
  xh->etime.tm_mday=atoi(get_strpart(xh->anno,n+6,n+7));
  xh->etime.tm_hour=atoi(get_strpart(xh->anno,n+8,n+9));
  xh->etime.tm_min=atoi(get_strpart(xh->anno,n+10,n+11));
  xh->etime.tm_sec=0;
  mday_mon2yday(&xh->etime);                /* alternative for mktime */

  xh->tlen=difftime_tm(&xh->time,&xh->etime);
  strcpy(xh->itime,get_strpart(xh->anno,16,27));

  strcpy(xh->sat,"MSG");
  strcpy(xh->src,xh->satsrc);    
  strtoupper(xh->src);
  
  xh->compr=xh->encry='_';
  if (!strcmp(xh->anno+strlen(xh->anno)-4,".bz2"))
    xh->compr='C';                                /* compressed */

/* Determine sort-order number: [yyyymmddhhmm][t][c] MUST be always equal length! */
  channame2nr(xh);
  strcpy(xh->chan,A_CH1); xh->chan_nr=1;
  if (xh->chan_nr>=0)
    sprintf(xh->sortn,"%s%c%02x",xh->itime,xh->hl,xh->segment);
  else
    sprintf(xh->sortn,"%s%c%02x",xh->itime,xh->hl,xh->segment);
  sprintf(xh->id,"%-10s %c   ",xh->chan,xh->hl);
  strftime(xh->id+14,20,"%d-%m-%y %H:%M  ",&xh->time);             /* time */
  return 1;
}


static int extract_anno_mavh(XRIT_HDR *xh)  /*NOAA*/
{
  extract_anno_mtp(xh);

  xh->image_iformat='y';
//  xh->hl='m';                               /* metop avhrr */
//  xh->file_type=0;                          /* means: pic */
//  xh->scan_dir='n';                         /* maybe 's'? */
  return 1;
}

/*************************************
 * Extract annotation info AVHRR NOAA
 * Examples:
 * 
 * avhrr_20060320_233600_noaa17.hrp.bz2     1 min segment 23:36
 * avhrr_20060320_233700_noaa17.hrp.bz2     1 min segment 23:37
 * avhrr_20060320_233800_noaa17.hrp.bz2     1 min segment 23:38
 *************************************/
static int extract_anno_avh(XRIT_HDR *xh)  /*NOAA*/
{
  char *p;
  int n;
  if (strcmp(get_strpart(xh->anno,28,31),".hrp")) return 0;

  xh->hl='A';                               /* avhrr */
  xh->segment=1; 
  xh->image_iformat='z';
  xh->file_type=0;                          /* means: pic */
  xh->scan_dir='n';                         /* maybe 's'? */
  
  n=6;
  xh->time.tm_year=atoi(get_strpart(xh->anno,n+0,n+3))-1900;
  xh->time.tm_mon=atoi(get_strpart(xh->anno,n+4,n+5))-1;
  xh->time.tm_mday=atoi(get_strpart(xh->anno,n+6,n+7));
  n++;
  xh->time.tm_hour=atoi(get_strpart(xh->anno,n+8,n+9));
  xh->time.tm_min=atoi(get_strpart(xh->anno,n+10,n+11));
  xh->time.tm_sec=0;
  mday_mon2yday(&xh->time);                /* alternative for mktime */
  xh->tlen=60;

  strcpy(xh->itime,get_strpart(xh->anno,6,13));
  strcat(xh->itime,get_strpart(xh->anno,15,18));

  strcpy(xh->satsrc,get_strpart(xh->anno,22,27)); 
  strcpy(xh->sat,"MSG");
  strcpy(xh->src,xh->satsrc);    
  strtoupper(xh->src);

  if ((p=strstr(xh->src,"NOAA"))) *(xh->src+4)=0;
  
  if (!strcmp(get_strpart(xh->anno,32,35),".bz2"))
    xh->compr='C';

  xh->compr=xh->encry='_';
  if (!strcmp(xh->anno+strlen(xh->anno)-4,".bz2"))
    xh->compr='C';                                /* compressed */

/* Determine sort-order number: [yyyymmddhhmm][t][c] MUST be always equal length! */
  channame2nr(xh);
  strcpy(xh->chan,A_CH1); xh->chan_nr=1;
  if (xh->chan_nr>=0)
    sprintf(xh->sortn,"%s%c%02x",xh->itime,xh->hl,xh->segment);
  else
    sprintf(xh->sortn,"%s%c%02x",xh->itime,xh->hl,xh->segment);

  snprintf(xh->id,MAXLEN_ID,"%-10s %c   ",xh->chan,xh->hl);
  strftime(xh->id+14,MAXLEN_ID-14,"%d-%m-%y %H:%M  ",&xh->time);             /* time */
  return 1;
}


static int extract_anno_bufr(XRIT_HDR *xh)  /*BUFR*/
{
  char *p,*q;
  strcpy(xh->sat,"MSG");
  strcpy(xh->src,"BUFR");
  strcpy(xh->satsrc,"BUFR");
  xh->hl='B';
  if (!(p=strchr(xh->anno,'_'))) return 0;
  strcpy(xh->chan,get_strpart(xh->anno,0,p-xh->anno-1));
  p++;
  xh->time.tm_year=atoi(get_strpart(p,2,3))+100;
  xh->time.tm_mon=atoi(get_strpart(p,4,5))-1;
  xh->time.tm_mday=atoi(get_strpart(p,6,7));

  xh->time.tm_hour=atoi(get_strpart(p,9,10));
  xh->time.tm_min=atoi(get_strpart(p,11,12));
  xh->time.tm_sec=0;
  mday_mon2yday(&xh->time);                /* alternative for mktime */
  strcpy(xh->itime,get_strpart(p,0,7));
  strcat(xh->itime,get_strpart(p,9,12));
  if (!(p=strchr(p+1,'_'))) return 0;
  if (!(p=strchr(p+1,'_'))) return 0;
  if (!(q=strchr(p+1,'_'))) return 0;
  strcpy(xh->satsrc,get_strpart(p,1,q-p-1));

  sprintf(xh->id,"%-10s %c   ",xh->chan,xh->hl);
  strftime(xh->id+14,20,"%d-%m-%y %H:%M  ",&xh->time);             /* time */

  sprintf(xh->sortn,"%s%c%02x",xh->itime,xh->hl,0);
  return 1;
}

static int extract_anno_dwdsat(XRIT_HDR *xh)  /*DWDSAT*/
{
  char *p;
  int n=0;

  strcpy(xh->sat,"MSG");
  strcpy(xh->src,"DWDSAT");
  strcpy(xh->satsrc,"DWDSAT");
  xh->hl=xh->anno[0];
  if (!strncmp(xh->anno,"grb",3)) xh->hl='R';
  if (!strcmp(xh->anno+strlen(xh->anno)-4,"tiff"))
  {
    xh->image_iformat='t';
    xh->segment=0; 
    xh->file_type=0;                          /* means: pic */
    xh->nb=8;            /* # bitplanes */
  }
  else if (strstr(xh->anno,".pl"))
  {
    xh->image_iformat='z';                    /* means: bz2 */
    xh->file_type=0;      
  }
  else
  {
    xh->file_type=2;                          /* means: txt */
  }
  
  if (strstr(xh->anno,".pl"))
  {
    strcpy(xh->chan,get_strpart(xh->anno,0,2));
    strcat(xh->chan,"_");
    strcat(xh->chan,get_strpart(xh->anno,15,17));
    xh->time.tm_year=atoi(get_strpart(xh->anno,4,5))+100;
    xh->time.tm_mon=atoi(get_strpart(xh->anno,6,7))-1;
    xh->time.tm_mday=atoi(get_strpart(xh->anno,8,9));

    xh->time.tm_hour=atoi(get_strpart(xh->anno,10,11));
    xh->time.tm_min=atoi(get_strpart(xh->anno,12,13));
    xh->time.tm_sec=0;
    mday_mon2yday(&xh->time);                /* alternative for mktime */
    strcpy(xh->itime,"20");
    strcat(xh->itime,get_strpart(xh->anno,4,13));
    sprintf(xh->id,"%-10s %c   ",xh->chan,xh->hl);
    strftime(xh->id+14,20,"%d-%m-%y %H:%M  ",&xh->time);             /* time */
  }
  else
  {
    strcpy(xh->chan,get_strpart(xh->anno,6,15));
    if ((p=strchr(xh->anno,'-')) && (p=strchr(p+1,'-')))
    {
      p++;
      xh->time.tm_year=atoi(get_strpart(p,n+0,n+1))+100;
      xh->time.tm_mon=atoi(get_strpart(p,n+2,n+3))-1;
      xh->time.tm_mday=atoi(get_strpart(p,n+4,n+5));

      xh->time.tm_hour=atoi(get_strpart(p,n+6,n+7));
      xh->time.tm_min=atoi(get_strpart(p,n+8,n+9));
      xh->time.tm_sec=0;
      mday_mon2yday(&xh->time);                /* alternative for mktime */
      strcpy(xh->itime,"20");
      strcat(xh->itime,get_strpart(p,0,9));
      sprintf(xh->id,"%-10s %c   ",xh->chan,xh->hl);
      strftime(xh->id+14,20,"%d-%m-%y %H:%M  ",&xh->time);             /* time */
    }
  }
  sprintf(xh->sortn,"%s%c%02x",xh->itime,xh->hl,0);
  return 1;
}


struct tm time2tm(char *p)
{
  struct tm tm;
  memset(&tm,0,sizeof(tm));
  tm.tm_year=atoi(get_strpart(p,0,3))-1900;
  tm.tm_yday=atoi(get_strpart(p,4,6))-1;
  yday2mday_mon(&tm);
  tm.tm_hour=atoi(get_strpart(p,7,8));
  tm.tm_min=atoi(get_strpart(p,9,10));
  return tm;
}

/*************************************
 * Extract annotation info GOES
 * Examples:
 *
 * gos11chnWV03rgnFDseg001res08dat233053600109.lrit
 * gos12chnWV03rgnUSseg005res04dat128153709981_030.lrit
 *                                ---daynr
 *                                   ----UTC
 *                                       -----??
 * TEXTdat128155459046_063.lrit
 *************************************/
static int extract_anno_gos(XRIT_HDR *xh)
{
  char *p,*p1;
  char anno[1000];
  strcpy(anno,xh->anno);
  p=anno;

  strncpy(xh->sat,p,5);
  if ((p=strstr(anno,"chn"))) strncpy(xh->chan,p+3,4);
  if ((p=strstr(anno,"rgn"))) strncpy(xh->src,p+3,2);
  if ((p=strstr(anno,"seg"))) xh->segment=atoi(p+3); 
  if ((p=strstr(anno,"res")));
  if ((p=strstr(anno,"dat")))
  {
    if ((p1=strchr(anno,'.')))
    {
       *p1=0;
       xh->hl=*(p1+1); 
    }

    memset(&xh->itime,0,sizeof(xh->itime));
    strncpy(xh->itime,p+3,7);
    memset(&xh->time,0,sizeof(xh->time));
    xh->time=time2tm(p-1);
    xh->time.tm_year=2000-1900;     // No time in annotation/filename!
  }
  if (!strncasecmp(xh->sat,"gos",3))
    sprintf(xh->satsrc,"GOES%s",xh->sat+3);
  else
    strcpy(xh->satsrc,xh->sat);

  if ((p=strstr(xh->anno,".")))
    xh->hl=*(p+1); 
  if (!xh->lfac) xh->scan_dir='n';  // if lfac==0: asume 'n'

  return 0; // force catch time-info from inside file!
}


/*************************************
 * Extract annotation info JMA
 * Examples:
 * 
 * IMG_DK01IR1_200804080330_001 
 *************************************/
static int extract_anno_jma(XRIT_HDR *xh)
{
  char *p;
  char anno[1000];
  char tmp[20];
  strcpy(anno,xh->anno);
  xh->hl='h'; 
  if (!(p=strtok(anno,"_"))) return 0;
  if (!(p=strtok(NULL,"_"))) return 0;
  strncpy(xh->satsrc,p,4);
  strcpy(xh->sat,xh->satsrc); 
  strcpy(xh->src,xh->sat); 
  if (strlen(p)>4) strcpy(xh->chan,p+4);

  if ((p=strtok(NULL,"_")))                  /* prod. ID#4: time (12 chars)  */
  {
    strcpy(xh->itime,p);
    memset(&xh->time,0,sizeof(xh->time));
    strncpy(tmp,p,4); tmp[4]=0; p+=4;
    xh->time.tm_year=atoi(tmp)-1900;

    strncpy(tmp,p,2); tmp[2]=0; p+=2;
    xh->time.tm_mon=atoi(tmp)-1;

    strncpy(tmp,p,2); tmp[2]=0; p+=2;
    xh->time.tm_mday=atoi(tmp);

    strncpy(tmp,p,2); tmp[2]=0; p+=2;
    xh->time.tm_hour=atoi(tmp);

    strncpy(tmp,p,2); tmp[2]=0; p+=2;
    xh->time.tm_min=atoi(tmp);
/*
Don't use this; time gets confused because of daylight saving!
    mktime(&xh->time);
*/
    mday_mon2yday(&xh->time);                /* alternative for mktime */

  }
  if ((p=strtok(NULL,"_")))                  /* prod. ID#4: time (12 chars)  */
    xh->segment=atoi(p);
  return 1;
}

/*
200603291130-msg-ch01.jpg
*/
/*************************************
 * Extract annotation info Taylor format
 * Examples:
 *
 * 200603291130-msg-ch01.jpg        same for ch02---ch11
 * 200603291130-msg-ch12w.jpg       corrected HRV
 *************************************/
static int extract_anno_tay(XRIT_HDR *xh)
{
  char *p;
  char tmp[20];
  char anno[1000];
  strcpy(anno,xh->anno);
  xh->hl='H';                               /* info not in filename */
  xh->segment=1; 
  if (strstr(xh->anno,".pgm"))
    xh->image_iformat='p';
  else
    xh->image_iformat='j';
  xh->file_type=0;
  xh->scan_dir='n';
  if (!strchr(anno,'-')) return 0;

  // Extract time part: 200603291130-msg-ch12w.jpg 
  //                    ------------ 
  if (!(p=strtok(anno,"-"))) return 0;      /* date and time */

  strcpy(xh->itime,p);
  memset(&xh->time,0,sizeof(xh->time));
  strncpy(tmp,p,4); tmp[4]=0; p+=4;
  xh->time.tm_year=atoi(tmp)-1900;

  strncpy(tmp,p,2); tmp[2]=0; p+=2;
  xh->time.tm_mon=atoi(tmp)-1;

  strncpy(tmp,p,2); tmp[2]=0; p+=2;
  xh->time.tm_mday=atoi(tmp);

  strncpy(tmp,p,2); tmp[2]=0; p+=2;
  xh->time.tm_hour=atoi(tmp);

  strncpy(tmp,p,2); tmp[2]=0; p+=2;
  xh->time.tm_min=atoi(tmp);

  mday_mon2yday(&xh->time);   /* don't use mktime because of daylight saving! */


  // Extract sat: 200603291130-msg-ch12w.jpg 
  //                           --- 
  if (!(p=strtok(NULL,"-"))) return 0;      /* satellite name; MSG */
  strncpy(xh->sat,p,4);                     /* copy 1 more to get closing 0 */
  if (!strcmp(xh->sat,"msg")) strcpy(xh->sat,"MSG"); 
  strcpy(xh->src,xh->sat);    

  if (!strncmp(xh->src,"MSG",3))
    strcpy(xh->satsrc,xh->src);
  else if (!strncmp(xh->src,"SERVICE",7))
    strcpy(xh->satsrc,"Srvc");
  else if (!strncmp(xh->src,"MPEF",7))
    strcpy(xh->satsrc,xh->src);
  else if (!strncmp(xh->src,"MET",3))
    strcpy(xh->satsrc,xh->src);
  else if (!strncmp(xh->src,"GOES",4))
    strcpy(xh->satsrc,xh->src);
  else if (!strncmp(xh->src,"MTSAT",5))
    strcpy(xh->satsrc,xh->src);
  else
    strcpy(xh->satsrc,"Frgn");

  // Extract chan: 200603291130-msg-ch12w.jpg 
  //                                ----- 
  if (!(p=strtok(NULL,"-."))) return 0;      /* channel (ch##[w]) */
  xh->chan_nr=atoi(p+2);
  
  // For MDM suffixes: se 'prepare.c'
  if (xh->chan_nr==12) strncpy(xh->mdm_pos_code,p+4,3);

/* Determine sort-order number: [yyyymmddhhmm][t][c] MUST be always equal length! */
  nr2channame(xh);
  if (xh->chan_nr>=0)
    sprintf(xh->sortn,"%s%c%x%02x",xh->itime,xh->hl,xh->chan_nr,xh->segment);
  else
    sprintf(xh->sortn,"%s%c%s%02x",xh->itime,xh->hl,xh->chan,xh->segment);
  sprintf(xh->id,"%-10s %c   ",xh->chan,xh->hl);
  strftime(xh->id+14,20,"%d-%m-%y %H:%M  ",&xh->time);             /* time */
  return 1;
}

static void define_subsat_from_anno(XRIT_HDR *xh)
{
  xh->sub_lon=xh->satpos; // GOES; 'satpos' is derived from channelname
  if (xh->sub_lon==NO_SATPOS)
  {  // Next values: Actual sat-position, before geom. correction?
     // 1-5-2008: replaced by number in annotation (xh.satpos)
    if (!strcasecmp(xh->satsrc,"MET7"))    xh->sub_lon= 57.5; 
    if (!strcasecmp(xh->satsrc,"gos12"))   xh->sub_lon=-75.1; 
    if (!strcasecmp(xh->satsrc,"GOES12"))  xh->sub_lon=-75.1; 
    if (!strcasecmp(xh->satsrc,"gos11"))   xh->sub_lon=-135.; 
    if (!strcasecmp(xh->satsrc,"GOES11"))  xh->sub_lon=-135.; 
    if (!strcasecmp(xh->satsrc,"GOES10"))  xh->sub_lon=-135.; 
    if (!strcasecmp(xh->satsrc,"GOES9"))   xh->sub_lon=-204.2; 
    if (!strcasecmp(xh->satsrc,"MTSATR"))  xh->sub_lon= 145.;
    if (!strcasecmp(xh->satsrc,"DK01"))    xh->sub_lon= 145.;
    if (!strcasecmp(xh->satsrc,"DK02"))    xh->sub_lon= 145.;
    if (!strcasecmp(xh->satsrc,"DK03"))    xh->sub_lon= 145.;
  }
  if (!strcasecmp(xh->satsrc,"MSG1_RSS"))  xh->sub_lon= 9.5;  // te veranderen!
}

/* Macros for detecting type */
#define is_meteo(s) ((!strncmp(s,"MSG",3))? 1 : (!strncmp(s,"MTP",3))? 1 : 0)
#define is_goes(s)  ((!strncmp(s,"gos",3))? 1 : 0)
#define is_jmalrit(s) ((!strncmp(s,"IMG_DK",6))? 1 : 0)
#define is_jpeg(s)  ((!strncasecmp(s+strlen(s)-3,"jpg",3))? 1 : 0)
#define is_pgm(s)   ((!strncasecmp(s+strlen(s)-3,"pgm",3))? 1 : 0)
#define is_avhrr(s) (((!strncmp(s,"avhrr",5)) && (strstr(s,"hrp")))? 1 : 0)
#define is_avhrrmet(s) ((!strncmp(s,"AVHR_HRP",8)) ? 1 : 0)
#define is_eps(s) ((!strncmp(s,"AVHR_",5))? 1 : 0)
#define is_bufr(s)  ((!strncmp(s+strlen(s)-4,"bufr",4))? 1 : 0)

#define is_dwdsat(s) (((!strncmp(s,"fx301",5)) || (!strncmp(s,"fx401",5)) || \
                       (!strncmp(s,"ps401",5)) || (!strncmp(s,"cgm01",5)) || \
                       ((!strncmp(s,"gts",3)) && (strncmp(s,"gts_",3)))   || \
                       (!strncmp(s,"rad01",5)) || \
                       (!strncmp(s,"sat01",5)) || (!strncmp(s,"sap01",5)) || \
                       (!strncmp(s,"wst11",5)) || (!strncmp(s,"wst12",5)) || \
                       (!strncmp(s,"buf01",5)) || (!strncmp(s,"grb01",5)) || \
                       (strstr(s,".pl3.bz2"))  || (strstr(s,".pl4.bz2"))  || \
                       (strstr(s,".pl5.bz2"))) \
                                        ? 1 : 0)

/* DWDSAT files:
   plotted obs.          : fx301, fx401
   analyses              : fx301, fx401
   forecast              : fx301, fx401, ps401
   diagnoses             : fx301, fx401
   meteograms            : fx301, fx401
   spec. prod            : cgm01, fx401
   sat images            : sat01, sap01, rad01
   prod observ           : fx401
   EGRR                  : fx401
   prod shipping         : fx401
   single bulletins ASCII: gts10, wst11, wst12
   single bulletins BUFR : buf01
   bulletin container    : gts02,gts06,gts05,gts11,gts08
   GRIB container        : grb01
   NIN JO DATA           : grb01, o_gmosw
*/

/*************************************
 * Extract annotation info 
 * Extracted:
 *   type (HRIT/LRIT/AVHRR etc.)
 *   time (needed for sorting etc.)
 * Used first on each file name (fast access), 
 *   then on annotation record
 *************************************/
static int extract_anno(XRIT_HDR *xh)
{
  int ret=0;
  /* Do preselection using disseminatingS/C */
  if (is_meteo(xh->anno+6))           /* disseminated by Meteosat */
    xh->xrit_frmt=EXRIT;
  else if (is_goes(xh->anno))         /* disseminated by GOES */
    xh->xrit_frmt=NXRIT;
  else if (is_jmalrit(xh->anno))      /* disseminated by GOES */
    xh->xrit_frmt=JMALRIT;
  else if (is_jpeg(xh->anno))         /* jpeg (Taylor-format) */
    xh->xrit_frmt=STDFRMT;
  else if (is_pgm(xh->anno))          /* pgm (Taylor-format) */
    xh->xrit_frmt=STDFRMT;
  else if (is_avhrr(xh->anno))        /* NOAA AVHRR */
    xh->xrit_frmt=NHRPT;
  else if (is_avhrrmet(xh->anno))     /* METOP AVHRR */
    xh->xrit_frmt=MHRPT;
  else if (is_eps(xh->anno))          /* METOP AVHRR */
    xh->xrit_frmt=METOP;
  else if (is_dwdsat(xh->anno))       /* DWDSAT */
    xh->xrit_frmt=DWDSAT;
  else if (is_bufr(xh->anno))         /* BUFR */
    xh->xrit_frmt=BUFR;
  else
    xh->xrit_frmt=UNKDTYPE;           /* unknown */

  switch(xh->xrit_frmt)
  {
    case EXRIT:                       /* Meteosat format */
      ret=extract_anno_msg(xh);       /*   format L-000-MSG1__-MSG1___... */
    break;
    case NXRIT:                       /* Goes format */
      ret=extract_anno_gos(xh);       /*   format gos12chnVS01rgnSHseg001... */
    break;
    case JMALRIT:                     /* Meteosat format */
      ret=extract_anno_jma(xh);       /*   format IMG_DK01IR1_200804080330_001 */
    break;
    case STDFRMT:                     /* Taylor (std Pic format) */
      ret=extract_anno_tay(xh);       /*   format 200603291130-msg-ch01.jpg */
    break;
    case NHRPT:                       /* AVHRR */
      ret=extract_anno_avh(xh);       /*   format avhrr_20060320_233800_noaa17.hrp.bz2 */
    break;
    case MHRPT:                       /* AVHRR */
      ret=extract_anno_mavh(xh);      /*   format AVHR_HRP_1B_M02_20060528092503Z_20060528092803Z_N_O_20060528093309Z.hrp.bz2 */
    break;
    case METOP:                       /* METOP */
      ret=extract_anno_mtp(xh);       /*   format AVHR_xxx_1B_M02_20060528092503Z_20060528092803Z_N_O_20060528093309Z.bz2avhrr_20060320_233800_noaa17.hrp.bz2 */
    break;
    case BUFR:                        /* BUFR */
      ret=extract_anno_bufr(xh);      /*   format e.g. hirs_20070225_1058_noaa17_24282_lan.l1c_bufr */
    break;
    case DWDSAT:                      /* DWDSAT */
      ret=extract_anno_dwdsat(xh);    /*    */
    break;
    default:                          /* unknown: assume MSG */
      ret=extract_anno_msg(xh);       /*   format L-000-MSG1__-MSG1___... */
    break;
  }
  // If sub_lon not defined here (from type2 heaader) do it now
  if (xh->sub_lon==NO_SATPOS)
  {
    define_subsat_from_anno(xh);
  }

  return ret;
}


static void catch_primhdr(unsigned char *l, XRIT_HDR *xrit_hdr)
{
  xrit_hdr->file_type=l[3]; /* 0=image,1=GTS mess.,2=text,3=encr. mess.*/
/* Total header length of this file */
  xrit_hdr->hdr_len=(l[4]<<24)+(l[5]<<16)+(l[6]<<8)+l[7];


/* Total content length of this file */
  xrit_hdr->datalen_msb=(l[8]<<24)+(l[9]<<16)+(l[10]<<8)+l[11];
  xrit_hdr->datalen_lsb=(l[12]<<24)+(l[13]<<16)+(l[14]<<8)+l[15];
  xrit_hdr->data_len=(xrit_hdr->datalen_lsb >> 3) +
                     (xrit_hdr->datalen_msb << 5);
}


// Time of frame start = 2007/233/05:30:13;
void extract_lritgoestime(char *l,char *time)
{
  char *key="Time of frame start = ";
  if (!strncmp(l,key,strlen(key)))
  {
    l+=strlen(key);
    strcpy(time,get_strpart(l,0,3));
    strcat(time,get_strpart(l,5,7));
    strcat(time,get_strpart(l,9,10));
    strcat(time,get_strpart(l,12,13));
    strcat(time,get_strpart(l,15,16));
  }
}

/*************************************
 * extract pix2tekp mapping for foreign sats
 * See: LRIT_HRIT_specific_implementation.pdf
 *        chapter 4.3.2.4: Header Type #3 - Image Data Function
 *      and
 *        A1.1.3 Example of an FSD Image Data Function (Header #3)
 *
 * format (ASCII):
 *   $HALFTONE:=10<CR>>LF>
 *   _NAME:=albedo<CR>>LF>
 *   _UNIT:=percent<CR>>LF>
 *   0:=0.0<CR>>LF>
 *   1023:=100.0<CR>>LF>
 *
 * or
 *   $HALFTONE:=10<CR>>LF>
 *   _NAME:=calibrated infrared<CR>>LF>
 *   _UNIT:=degree Kelvin<CR>>LF>
 *   0:=170.0<CR>>LF>
 *   1023:=340.0<CR>>LF>
 *
 * or:
 *   $HALFTONE:=8<CR>>LF>
 *   _NAME:=calibrated infrared<CR>>LF>
 *   _UNIT:=degree Kelvin<CR>>LF>
 *   0:=320.000<CR>>LF>
 *   ....
 *   255:=213.838<CR>>LF>
 *
 * or:
 *   $HALFTONE:=16<CR>>LF>
 *   _NAME:=INFRARED
 *   _UNIT:=KELVIN
 *   0:=320.000<CR>>LF>
 *   ...
 *   837:=232.95<CR>>LF>
 *   844:=231.42<CR>>LF>
 *
 *************************************/
 /*
 13-5-2012: changed \n to \n\r; GOES versus also MTSAT
 */
 
static void catch_irmap(char *l,CHANNEL *chan)
{
  char *w;
  int pv;
  float pt;
  int i;
  if (!chan) return;
  if (chan->cal.caltbl[0]) return; // table already loaded
  if (!(w=strtok(l+3,":=\n")))  return;   // $HALFTONE
  if (strcmp(w,"$HALFTONE"))    return;
  if (!(w=strtok(NULL,"=\n\r")))   return;  // 10
  chan->cal.caltbl_bpp=atoi(w);

  if (!(w=strtok(NULL,":=\n\r"))) return;   // _NAME 
  if (strcmp(w,"_NAME"))        return;
  if (!(w=strtok(NULL,"=\n\r")))   return;  // "calibrated infrared"
//  w[strlen(w)-1]=0;// waarom laatste char weg???
//  if (strstr(w,"infrared")) cal='i';

  if (!(w=strtok(NULL,":=\n\r"))) return;   // _UNIT 
  if (strcmp(w,"_UNIT"))        return;
  if (!(w=strtok(NULL,"=\n\r")))   return;  // "degree Kelvin" or "percent"
//  w[strlen(w)-1]=0; // waarom laatste char weg???
  if (strcasestr(w,"Kelvin"))   chan->cal.caltbl_type='k';
  if (strcasestr(w,"percent"))  chan->cal.caltbl_type='p';
  if (strcasestr(w,"albedo"))  chan->cal.caltbl_type='a';

  chan->cal.caltbl[0]=calloc(256,sizeof(*chan->cal.caltbl[0]));
  chan->cal.caltbl[1]=calloc(256,sizeof(*chan->cal.caltbl[1]));

  if (!chan->cal.caltbl[0]) return;
  for (i=0; i<256; i++)
  {
    if (!(w=strtok(NULL,":=\n\r"))) break;
    pv=atoi(w);
    if (!(w=strtok(NULL,":=\n\r"))) break;
    pt=atof(w);
    chan->cal.caltbl[0][i]=(float)pv;
    chan->cal.caltbl[1][i]=pt;
  }
//  if ((i) && (chan->cal.caltbl[0][i-1]==65535)) i--;
  chan->cal.nrcalpoints=i;

#ifdef OUD
{
  if (!chan->caltbl) return;
  do
  {
    if (!(w=strtok(NULL,":=\n\r"))) break;
    pv=atoi(w);
    if (!(w=strtok(NULL,":=\n\r"))) break;
//    w[strlen(w)-1]=0;// waarom laatste char weg???
    pt=atof(w);
// maak 2-dim tabel!
    if (chan->cal.caltbl_bpp<=8)
    {
      if ((pv<256) && (pv>=0))
      {
        chan->caltbl[pv]=pt;
      }
      chan->cal.nrcalpoints=256;
    }
    else
    {
//printf("%d  %f\n",pv,pt);
      if (pv) pv=1;
      chan->caltbl[pv]=pt;
      chan->cal.nrcalpoints=2;
    }
  } while (1);
}
#endif
}

/*************************************
 * Extract xrit header
 * Return: rest header (if !=0: error!)
 *************************************/
static int catch_xrit_hdr(unsigned char *l,int ln,XRIT_HDR *xrit_hdr,CHANNEL *chan)
{
  unsigned char *p;
  int hdr_len=0;
  int pos=0;
  memset(xrit_hdr,0,sizeof(*xrit_hdr));
  xrit_hdr->sub_lon=NO_SATPOS;
  do
  {
/* Get header type and length */
    xrit_hdr->hdr_type=l[0];
    xrit_hdr->hdr_rec_len=(l[1]<<8)+l[2];

/* Test: header length < length l */
    pos+=xrit_hdr->hdr_rec_len;
    if (pos>ln) break;
/* Extract headertype dependent info */
//printf("type=%d  chan=%x\n",xrit_hdr->hdr_type,chan);
    switch(xrit_hdr->hdr_type)
    {
/* ------------------ Primary header ------------------ */
      case 0:
        catch_primhdr(l,xrit_hdr);
      break;
/* ------------------ Image header ------------------ */
      case 1:
        xrit_hdr->nb=l[3];           /* # bitplanes */
        xrit_hdr->nc=(l[4]<<8)+l[5]; /* # columns (=width) */
        xrit_hdr->nl=(l[6]<<8)+l[7]; /* # lines */
        xrit_hdr->cf=l[8];           /* compr. flag: 0, 1=lossless, 2=lossy */
      break;
/* ------------------ Image navigation ------------------ */
      case 2:
/* Projection name */
        strncpy(xrit_hdr->proj_name,(char *)(l+3),32); xrit_hdr->proj_name[32]=0;
/* Remove leading spaces */
        for (p=(unsigned char *)(xrit_hdr->proj_name+31); 
             ((*p==' ') && ((char *)p > xrit_hdr->proj_name));
             p--);
        p++; *p=0;
        if (strstr(xrit_hdr->proj_name,"GEOS"))
        {
          char *pp;
          if ((pp=strchr(xrit_hdr->proj_name,'('))) xrit_hdr->sub_lon=atof(pp+1);
        }
        xrit_hdr->cfac=(l[35]<<24)+(l[36]<<16)+(l[37]<<8)+l[38];
        xrit_hdr->lfac=(l[39]<<24)+(l[40]<<16)+(l[41]<<8)+l[42];
        xrit_hdr->coff=(l[43]<<24)+(l[44]<<16)+(l[45]<<8)+l[46];
        xrit_hdr->loff=(l[47]<<24)+(l[48]<<16)+(l[49]<<8)+l[50];
        if (xrit_hdr->lfac > 0) xrit_hdr->scan_dir='n';
        if (xrit_hdr->lfac < 0) xrit_hdr->scan_dir='s';
      break;
/* ------------------ Image data functions ------------------ */
      case 3:
        if (chan)
        {
          catch_irmap((char *)l,chan);
        }
      break;
/* ------------------ Annotation ------------------ */
      case 4:
/* Extract annotation */
        strncpy(xrit_hdr->anno,(char *)(l+3),64);
        xrit_hdr->anno[61]=0;
        extract_anno(xrit_hdr);
      break;
/* ------------------ Time stamp ------------------ */
      case 5:
      {
        int i;
        for (i=0; i<7; i++) xrit_hdr->ccsds[i]=l[3+i];
      }
      break;
/* ------------------ ancillary text ------------------ */
      case 6:
        if (xrit_hdr->xrit_frmt==NXRIT)
        {
          extract_lritgoestime((char *)(l+3),xrit_hdr->itime);
          xrit_hdr->time=time2tm(xrit_hdr->itime);
        }
      break;
/* ------------------ key header ------------------ */
      case 7:
      break;
/* ------------------ segment identification ------------------ */
      case 128:
        switch(xrit_hdr->xrit_frmt)
        {
          case EXRIT: case UNKDTYPE:
            /* Eumetsat (if unk: for the moment asume Eumetsat) */
            xrit_hdr->gp_sc_id   =(l[3]<<8)+l[4];   /* unique for each sat. source? (MSG, GOES...) */
            xrit_hdr->spec_ch_id =l[5];
            xrit_hdr->seq_no     =(l[6]<<8)+l[7];   /* segment no. */
            xrit_hdr->seq_start  =(l[8]<<8)+l[9];   /* "planned" start segment */
            xrit_hdr->seq_end    =(l[10]<<8)+l[11]; /* "planned" end segment */
            xrit_hdr->dt_f_rep   =l[12];
          break;
          case JMALRIT:
            xrit_hdr->seq_no     =l[3];             /* segment no. */
            xrit_hdr->seq_start  =1;                /* "planned" start segment */
            xrit_hdr->seq_end    =l[4];             /* "planned" end segment */
            xrit_hdr->dt_f_rep   =0;
// (l[5]<<8)+l[6]=line number of image segment
          break;
          case NXRIT:
            /* NOAA */
            xrit_hdr->gp_sc_id   =(l[3]<<8)+l[4];   /* unique for each picture?? */
            xrit_hdr->seq_no     =(l[5]<<8)+l[6];   /* segment seq. no. */
            /* start column=(l[7]<<8)+l[8] */
            /* start row   =(l[9]<<8)+l[10] */
            xrit_hdr->seq_start  =1;                /* start segment */
            xrit_hdr->seq_end    =(l[11]<<8)+l[12]; /* max segment */
            /* max column=(l[13]<<8)+l[14] */
            /* max row   =(l[15]<<8)+l[16] */
          break;
          default:  // not expected
          break;
       }
      break;
/* ------------------ image segment line quality ------------------ */
      case 129:
      break;
/* ------------------ ??? In GOES LRIT ??? ------------------ */
      case 130:
      break;
/* ------------------ JMA:  Image Observation Time Header ------------------ */
      case 131:
        if (xrit_hdr->xrit_frmt==JMALRIT)
        {
          tfreenull(&xrit_hdr->img_obs_time);
          xrit_hdr->img_obs_time=calloc(xrit_hdr->hdr_rec_len,1);
          strncpy(xrit_hdr->img_obs_time,(char *)(l+3),xrit_hdr->hdr_rec_len-3);
          while ((p=(unsigned char*)strchr(xrit_hdr->img_obs_time,'\r'))) *p=' ';
        }
      break;
/* ------------------ ??? In GOES LRIT ??? ------------------ */
      case 132:
      break;

      default:
        printf("Unexpected hdrtype=%d\n",xrit_hdr->hdr_type);
      break;
    }
    if (xrit_hdr->hdr_type==0)
    {
      hdr_len=xrit_hdr->hdr_len;
    }
    l+=xrit_hdr->hdr_rec_len;

    hdr_len-=xrit_hdr->hdr_rec_len;

  } while (hdr_len>0);
  return hdr_len;
}


/*************************************************************************
 * Read from a extracted file (channel with certain order number)
 * the XRIT header.
 * Remaining file is e.g. JPEG or Wavelet.
 *************************************************************************/
static int read_xrithdr(FILE *fp,XRIT_HDR *xrit_hdr,CHANNEL *chan)
{
  unsigned char l1[20],*l;
  int hdr_len;
  int hdr_rest=0;
/* Read in primary header, just to determine length of all headers  */
  fread(l1,16,1,fp);
  fseek(fp,-16,SEEK_CUR);

/* Test header; expected primary */
  if (l1[0]!=0) return -1;

  if (((l1[1]<<8) + l1[2]) !=16) return -1;

/* Determine total header length */
  hdr_len=(l1[4]<<24)+(l1[5]<<16)+(l1[6]<<8)+l1[7];
  if ((hdr_len>10000) || (hdr_len<10)) return -1;

/* Allocate and read all headers */
  if (!(l=malloc(hdr_len))) return -1;

  fread(l,hdr_len,1,fp);

/* Extract header info */
  hdr_rest=catch_xrit_hdr(l,hdr_len,xrit_hdr,chan);
// Nu moet fp precies aan begin image staan.
// Lijkt niet altijd het geval te zijn??
/* Determine image type. Used first 2 bytes of actual image. */
  if (xrit_hdr->file_type==0)
  {
    unsigned char c[2];
    fread(c,2,1,fp);
    if ((c[0]==0xff) && (c[1]==0x01))      xrit_hdr->image_iformat='w';
    else if ((c[0]==0xff) && (c[1]==0xd8)) xrit_hdr->image_iformat='j';
    else                                   xrit_hdr->image_iformat='?';
    fseek(fp,-2,SEEK_CUR);
  }

  channame2nr(xrit_hdr);

  xrit_hdr->done=TRUE;

  free(l);
  return hdr_rest;
}

static void init_noaachars(XRIT_HDR *xrit_hdr)
{
  int nrsecs=60;
  xrit_hdr->nb=10;                 /* # bitplanes */
  xrit_hdr->nc=AVHRR_PICWIDTH*2;   /* # columns (=width) ! Always linearize!*/
#ifdef HRPTLIN_NW
  xrit_hdr->nc=AVHRR_PICWIDTH;
#endif
  xrit_hdr->nl=6*nrsecs;           /* # lines one file */
  xrit_hdr->cf=1;                  /* compr. flag: 0, 1=lossless, 2=lossy */
  xrit_hdr->hdr_rec_len=0;
  xrit_hdr->lines_per_sec=AVHRR_LINESPSEC;
  xrit_hdr->done=TRUE;
}

static void init_metopchars(FILE *fp,XRIT_HDR *xrit_hdr)
{
//  struct tm tm1=xrit_hdr->time;
//  struct tm tm2=xrit_hdr->etime;
//  int nrsecs=mktime(&tm2)-mktime(&tm1);
  int width,height,lps;
  get_eps_picsize(fp,&width,&height,&lps);
  if (!width) width=2048;
  if (!height) height=1080;
  xrit_hdr->nb=14;               /* # bitplanes */
  xrit_hdr->nc=width;            /* # columns (=width) Don't lin, or lin. with expand_l=1! */
  xrit_hdr->nl=height;           /* # lines one file */
  xrit_hdr->cf=1;                /* compr. flag: 0, 1=lossless, 2=lossy */
  xrit_hdr->lines_per_sec=lps;
  xrit_hdr->hdr_rec_len=0;
  xrit_hdr->done=TRUE;
}

static void init_metopavhrrchars(XRIT_HDR *xrit_hdr)
{

  xrit_hdr->nb=10;               /* # bitplanes */
  xrit_hdr->nc=2048;             /* # columns (=width) Don't lin, or lin. with expand_l=1! */
  xrit_hdr->nl=360;              /* # lines one file */
  xrit_hdr->cf=1;                /* compr. flag: 0, 1=lossless, 2=lossy */
  xrit_hdr->lines_per_sec=0;
  xrit_hdr->hdr_rec_len=0;
  xrit_hdr->done=TRUE;
}


/*************************************************************************
 * Extract/define some pic chars.
 *   JPEG or PGM, generated by (e.g.) MDM.
 *************************************************************************/
static int read_stdpichdr(FILE *fp,XRIT_HDR *xrit_hdr)
{
  int width,height;
  int err=0;
  switch (xrit_hdr->image_iformat)
  {
    case 'j':
      err=get_jpegsize_opened(fp,&width,&height);
    break;
    case 'p':
     err=get_pgmsize_opened(fp,&width,&height);
    break;
  }

  xrit_hdr->nb=8;           /* # bitplanes */
  xrit_hdr->nc=width;       /* # columns (=width) */
  xrit_hdr->nl=height;      /* # lines */
  xrit_hdr->cf=2;           /* compr. flag: 0, 1=lossless, 2=lossy */
//  xrit_hdr->gp_sc_id   =(l[3]<<8)+l[4];   /* unique for each sat. source? (MSG, GOES...) */
//  xrit_hdr->spec_ch_id =l[5];
  xrit_hdr->seq_no     =1;   /* segment no. */
  xrit_hdr->seq_start  =1;   /* "planned" start segment */
  xrit_hdr->seq_end    =1;   /* "planned" end segment */
//  xrit_hdr->dt_f_rep   =0;
  xrit_hdr->hdr_rec_len=0;
  xrit_hdr->done=TRUE;

  if (xrit_hdr->nc==3712)                 // Assume MSG channels 1...11
  {
    xrit_hdr->coff=1856;
    xrit_hdr->cfac=13642337;
  }
  else if (xrit_hdr->nc==11136)           // Assume MSG channels 12
  {
    xrit_hdr->coff=5566;
    xrit_hdr->cfac=40927014;
  }
  else if (xrit_hdr->nc > 5566)
  {
    xrit_hdr->coff=xrit_hdr->nc-5566;                  // first segment
    xrit_hdr->cfac=40927014;
  }

  if (xrit_hdr->nl==3712)                 // Assume MSG channels 1...11
  {
    xrit_hdr->loff=1856;                  // first segment
    xrit_hdr->lfac=13642337;
  }
  else if (xrit_hdr->nl==11136)           // Assume MSG channel 12
  {
    xrit_hdr->loff=5566;                  // first segment
    xrit_hdr->lfac=40927014;
  }
  else if (xrit_hdr->nl > 3712)
  {
    xrit_hdr->loff=5566;                  // first segment
    xrit_hdr->lfac=40927014;
  }
  
  else if (xrit_hdr->nl==1392)            // Assume MSG channels 1..11, RSS
  {
    xrit_hdr->loff=-464;                  // first segment
    xrit_hdr->lfac=13642337;
  }
  else if (xrit_hdr->nl==4176)            // Assume MSG channel 12, RSS
  {
    xrit_hdr->loff=-1394;                  // first segment
    xrit_hdr->lfac=40927014;
  }
  return err;
}

static void init_dwdsat(XRIT_HDR *xh)
{
  xh->seq_no     =1;   /* segment no. */
  xh->seq_start  =1;   /* "planned" start segment */
  xh->seq_end    =1;   /* "planned" end segment */
}

/********************** Extrenal funcs *****************************/
/*************************************
 * Translate file name into xrit-info 
 * Return: 1 if OK
 *         0 if failed
 * If sat has no Meteosat-formatted xrit headers then return 0
 * 
 *************************************/
int fn2xrit(char *fn,XRIT_HDR *xh)
{
  char *p;
  if (!fn) return 0;

  if ((p=strrchr(fn,DIR_SEPARATOR))) p++; else p=fn;
  strcpy(xh->anno,p);
  if (!(extract_anno(xh))) return 0;
  if (xh->xrit_frmt==UNKDTYPE) return 0;
  channame2nr(xh);
  return 1;
}

FILE *open_xritimage(char *fn,XRIT_HDR *xrit_hdr,DATATYPE type,CHANNEL *chan)
{
  FILE *fp;
  XRIT_HDR i_xrit_hdr;
  char *fn1;
  gboolean hdr_intern=FALSE;
  if ((fn1=strrchr(fn,DIR_SEPARATOR))) fn1++; else fn1=fn;
#ifdef BUNZIP2_FOR_EXACT_PARAMETER_EXTRACTION
  if (str_ends_with(fn,".bz2"))
  {
    char fnt[200];
    if ((bunzip_to_tempfile(fn,fnt))) return NULL;
    if (!(fp=fopen(fnt,"rb")))
    {
      remove(fnt);                    /* Remove temp file */
      return NULL;                 /* can't reopen wvt-info file */
    }
  }
  else
#endif
  {
    if (!(fp=fopen(fn,"rb"))) return NULL;
  }

  if (!xrit_hdr)
  {
    xrit_hdr=&i_xrit_hdr;
    hdr_intern=TRUE;
  }
  switch(type)
  {
    case STDFRMT:
      if (read_stdpichdr(fp,xrit_hdr))
      {
        fclose(fp);
        fp=NULL;
      }
    break;
    case NHRPT:
      init_noaachars(xrit_hdr);
    break;
    case MHRPT:
      init_metopavhrrchars(xrit_hdr);
    break;
    case METOP:
      init_metopchars(fp,xrit_hdr);
    break;
    case DWDSAT:
      init_dwdsat(xrit_hdr);
    break;
    case BUFR:
    break;
    default:
      if ((read_xrithdr(fp,xrit_hdr,chan)))
      { // error
        fclose(fp);
        fp=NULL;
      }
      // JMA: plain 16 bpp, but actually used 10 bpp(?)
      if ((type==JMALRIT) && (xrit_hdr->nb==16)) xrit_hdr->nb=10;
    break;
  }
  if (hdr_intern) tfreenull(&xrit_hdr->img_obs_time);
  return fp;
}
