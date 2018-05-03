/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * EPS funcs
 * Docs: 
 *   [1] pdf_ten_97231-eps-avhrr.pdf
 *          Ref.: EPS.MIS.SPE.97231, Issue: 6 Rev. 5 / WBS number: 270000 / 12/03/04
 *          AVHRR-L1-PFS-ANNEX.xls
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "xrit2pic.h"
#include "avhrr.h"
#include "eps.h"

/*******************************************
 * Read header.
 * Each header starts with 20 bytes record header containing:
 *   - type
 *   - size
 *******************************************/

/*******************************************
 * Read record header. (first 20 bytes)
 *******************************************/
int read_rechdr(FILE *fp,HDR *hdr)
{
  unsigned char buf[20];
  unsigned long size;
  int hdrclass;

  fread(&buf,1,20,fp);
  size=(buf[4]<<24)+(buf[5]<<16)+(buf[6]<<8)+buf[7];
  hdrclass=buf[0];

  hdr->size=size;
  return hdrclass;
}

#define SCHEID " =\n	"

int get_eps_item(FILE *fp,long pos,char *item,char **val)
{
  static char buf[51],*p;
  int n;
  *val=NULL;
  fseek(fp,pos,SEEK_SET);
  if (!(n=fread(buf,1,50,fp))) return n;
  if (!(p=strtok(buf,SCHEID))) return n;
  if ((item) && (strcmp(p,item))) return n;
  if (!(*val=strtok(NULL,SCHEID))) return n;
  return n;
}

// 0922161826000
static int mday[]={31,28,31,30,31,30,31,31,30,31,30,31};
static float str2day(int year,char *s)
{
  float day=0.;
  int mon,i;
  mon=atoi(get_strpart(s,0,1));
  for (i=1; i<mon; i++)
    day+=mday[i-1];
  if ((mon>2) && (!(year%4))) day++;
  day+=atoi(get_strpart(s,2,3));
  day+=atoi(get_strpart(s,4,5))/24.;
  day+=atoi(get_strpart(s,6,7))/(24.*60.);
  day+=atoi(get_strpart(s,8,9))/(24.*3600.);
  return day;
}

#define POS_ORBIT_START 0x561
#define POS_EPOCHTIME   0x5d9
#define POS_ECCENTR     0x638
#define POS_INCLIN      0x664
#define POS_PERIGEE     0x690
#define POS_RAAN        0x6bc
#define POS_MANOMAL     0x6e8

#define POS_LATSTART    0x950
#define POS_LONSTART    0x97c
#define POS_LATEND      0x9a8
#define POS_LONEND      0x9d4
#define POS_MDR         0xb8b
#define POS_DURATION    0xc4e
/*******************************************
 * Read class 0 header.
 *******************************************/
int rd_cl1hdr(FILE *fp,HDR *hdr)
{
  long n=0;
  char *p;

  n=POS_ORBIT_START-20;         // 20 bytes already readed
  n+=get_eps_item(fp,POS_ORBIT_START,NULL,&p);
  if (p) hdr->cls1.orbit_start=atoi(p);


  n=POS_EPOCHTIME-20; 
  n+=get_eps_item(fp,POS_EPOCHTIME,NULL,&p);
  if (p)
  {
    hdr->cls1.epoch_year=atoi(get_strpart(p,0,3))-1900;
    hdr->cls1.epoch_day=str2day(hdr->cls1.epoch_year,p+4);
  }

  n=POS_ECCENTR-20; 
  n+=get_eps_item(fp,POS_ECCENTR,NULL,&p);
  if (p) hdr->cls1.eccentricity=atoi(p)/1000000.;

  n=POS_INCLIN-20;
  n+=get_eps_item(fp,POS_INCLIN,NULL,&p);
  if (p) hdr->cls1.inclination=atoi(p)/1000.;

  n=POS_PERIGEE-20;
  n+=get_eps_item(fp,POS_PERIGEE,NULL,&p);
  if (p) hdr->cls1.perigee=atoi(p)/1000.;

  n=POS_RAAN-20;
  n+=get_eps_item(fp,POS_RAAN,NULL,&p);
  if (p) hdr->cls1.raan=atoi(p)/1000.;

  n=POS_MANOMAL-20;
  n+=get_eps_item(fp,POS_MANOMAL,NULL,&p);
  if (p) hdr->cls1.anomaly=atoi(p)/1000.;

  n=POS_LATSTART-20;
  n+=get_eps_item(fp,POS_LATSTART,NULL,&p);
  if (p) hdr->cls1.subsat_start.lat=atoi(p)/1000.;

  n=POS_LONSTART-20;
  n+=get_eps_item(fp,POS_LONSTART,NULL,&p);
  if (p) hdr->cls1.subsat_start.lon=atoi(p)/1000.;

  n=POS_LATEND-20;
  n+=get_eps_item(fp,POS_LATEND,NULL,&p);
  if (p) hdr->cls1.subsat_end.lat=atoi(p)/1000.;

  n=POS_LONEND-20;
  n+=get_eps_item(fp,POS_LONEND,NULL,&p);
  if (p) hdr->cls1.subsat_end.lon=atoi(p)/1000.;

  n=POS_MDR-20;         // 20 bytes already readed
  n+=get_eps_item(fp,POS_MDR,NULL,&p);
  if (p) hdr->cls1.total_mdr=atoi(p);

  n=POS_DURATION-20;
  n+=get_eps_item(fp,POS_DURATION,NULL,&p);
  if (p) hdr->cls1.duration_of_product=atoi(p)/1000;  // duration in secs

  return n;
}

/*******************************************
 * Read class 2 header: SPHR (subclass 0, version 3)
 * See [1], page 4 of annex.

 * fp assumed to be located after the 20 bytes record header.
 * See table class 2 on page 48.
 * Return: # bytes readed (123, excluding 20 bytes record header)
 *******************************************/
int rd_cl2hdr(FILE *fp,HDR *hdr)
{
  int n=0;
  char buf[50],*p;

  buf[49]=0;
  n+=fread(buf,1,49,fp);    // SRC_DATA_QUAL

/*-----------*/
  n+=fread(buf,1,38,fp);    // EARTH_VIEWS_PER_SCANLINE

  p=strtok(buf,SCHEID);
  p=strtok(NULL,SCHEID);
  if (p) hdr->cls2.earth_views_per_scanline=atoi(p);

/*-----------*/
  n+=fread(buf,1,36,fp);    // NAV_SAMPLE_RATE

  p=strtok(buf,SCHEID);
  p=strtok(NULL,SCHEID);
  if (p) hdr->cls2.nav_sample_rate=atoi(p);
  return n;
}

/*******************************************
 * Read class 8 header: MDR (subclass 2, version 4)
 * See [1], page 14 of annex.
 *  This record contains one scanned line.
 *  Here only first 4 bytes after record_header is readed.
 * Return: # bytes readed (4, excluding 20 bytes record header)
 *******************************************/
int rd_cl8hdr(FILE *fp,HDR *hdr)
{
  int n=0;
  unsigned char buf[50];
  n+=fread(buf,1,1,fp);          // DEGRADED_INST_MDR
  n+=fread(buf,1,1,fp);          // DEGRADED_PROC_MDR
  n+=fread(buf,1,2,fp);          // EARTH_VIEWS_PER_SCANLINE
  hdr->cls8.earth_views_per_scanline=((int)(buf[0])<<8)+buf[1];
//printf("width=%d  pos=%x\n\n",hdr->cls8.earth_views_per_scanline,ftell(fp));

// ?? geen earth_views_per_scanline in metop-avhrr??
  if (!hdr->cls8.earth_views_per_scanline) hdr->cls8.earth_views_per_scanline=2048;
  return n;
}


/*******************************************
 * Read any record.
 * File pointer is set after this record.
 * Info from record is put into HDR struct.
 * Not known record types: Only file pointer is set after record.
 *
 * Return: record class
 *******************************************/
int read_genrechdr(FILE *fp,HDR *hdr)
{
//  static int nr;
  int n;
  int hdrclass;

  hdrclass=read_rechdr(fp,hdr);   // read first 20 bytes
  switch(hdrclass)
  {
    case 1 : n=rd_cl1hdr(fp,hdr); break;
    case 2 : n=rd_cl2hdr(fp,hdr); break;
    case 8 : n=rd_cl8hdr(fp,hdr); break;
    default: n=0;                 break;
  }
  if (feof(fp)) return -1;
  fseek(fp,hdr->size-20-n,SEEK_CUR);     // Set filepointer after just readed record

  return hdrclass;
}

#ifdef XXX
void xxx(CLASS1HDR c,SEGMENT *segm)
{
  KEPLER kepler;
  ORBIT orbit;
  GROUP *grp=segm->chan->group;
PIC_CHARS pc;
POINT pos;
memset(&kepler,0,sizeof(kepler));
memset(&orbit,0,sizeof(orbit));
printf("%f  %f  %f  %f  epoch: %d  %f\n",c.subsat_start.lon,c.subsat_start.lat,c.subsat_end.lon,c.subsat_end.lat,c.epoch_year,c.epoch_day);
  kepler.eccentricity= c.eccentricity;
  kepler.inclination= c.inclination;
  kepler.perigee= c.perigee;
  kepler.raan= c.raan;
  kepler.anomaly= c.anomaly;
  kepler.epoch_year=c.epoch_year;
  kepler.epoch_day=c.epoch_day;
kepler.motion=14.11096946; // NOAA18
printf("%f\n",kepler.eccentricity);
printf("%f\n",kepler.inclination);
printf("%f\n",kepler.perigee);
printf("%f\n",kepler.raan);
printf("%f\n",kepler.anomaly);
printf("%d\n",kepler.epoch_year);
printf("%f\n",kepler.epoch_day);
orbit.start_tm=grp->grp_tm;
printf(asctime(&grp->grp_tm));
  calc_orbitconst(&kepler,&orbit);
printf("ref_time=%d\n",orbit.ref_time);
pc.width=2048;
pc.lines_per_sec=2;
pc.orbit=orbit;
pc.kepler=kepler;
  xy2lonlat(1024,0,&pc,&pos);
printf("--> %f  %f\n",R2D(pos.lon),R2D(pos.lat));
}
#endif

void get_eps_picsize(FILE *fp,int *width,int *height,int *lps)
{
  int typehdr=0;
  HDR hdr;
  memset(&hdr,0,sizeof(hdr));
  *width=AVHRR_PICWIDTH;           // will be overwritten
  *lps=6;
  rewind(fp);
  while (typehdr!=8)
  {
    typehdr=read_genrechdr(fp,&hdr);
    if (typehdr<0) break;
  }
  *width=hdr.cls8.earth_views_per_scanline;
  *height=hdr.cls1.total_mdr;

  if (hdr.cls1.duration_of_product)
    *lps=hdr.cls1.total_mdr/hdr.cls1.duration_of_product;
  rewind(fp);
}


// scanline:
// hdr class 8 starts with 46 bytes
// then 2071 samples:
//      11 samples meas.     ==>   11*5*10/8=   68.75 bytes
//    2048 samples act. data ==> 2048*5*10/8=12800    bytes
//      12 samples meas.     ==>   12*5*10/8=   75    bytes
// Left:                                         0.25 bytes
// Total                                     12944    bytes
//
// Start sample 8, byte 50+26=76
static int get_scanline(FILE *fp,HDR *hdr,char image_iformat,guint16 *line[5])
{
  int hdrclass;
  int n=0;
  int i,j,c,p;
  hdrclass=read_rechdr(fp,hdr);  // read 20 bytes
  if (hdrclass!=8) return 0;
  if (image_iformat=='y')   // EARS METOP format
  {

// p 0000 0111 1122 2223 3333
// c 1234 5123 4512 3451 2345
    n=76; // 96;                     // start scan data at pos. 46+68.75=114.75 of record class 8
                              //  -20 (laready read), round down ==> 94
    fseek(fp,n,SEEK_CUR);     // Set filepointer at start line
    c=0;
    p=0;
    j=0;
    for (i=-3*5; i<2048*5; )
    {
      unsigned char tmp[25];
      n+=fread(tmp,1,5,fp);
      if (i>=0)
      {
        if (line[c]) line[c][p]=((tmp[0]&0xff)<<2)+((tmp[1]&0xc0)>>6);
        c=(c+1)%5;
        j++; p=j/5;
      }
      i++; 
      if (i>=0)
      {
        if (line[c]) line[c][p]=((tmp[1]&0x3f)<<4)+((tmp[2]&0xf0)>>4);
        c=(c+1)%5;
        j++; p=j/5;
      }
      i++;
      if (i>=0)
      {
        if (line[c]) line[c][p]=((tmp[2]&0x0f)<<6)+((tmp[3]&0xfc)>>2);
        c=(c+1)%5;
        j++; p=j/5;
      }
      i++;
      if ((i>=0) && (p<2048)) // ! Laatste: p=2048
      {
        if (line[c]) line[c][p]=((tmp[3]&0x03)<<8)+((tmp[4]&0xff)>>0); // start byte 94 2 bits
        c=(c+1)%5;
        j++; p=j/5;
      }
      i++;

    }
    fseek(fp,hdr->size-20-n,SEEK_CUR);     // Set filepointer after just readed record
  }
  else
  {
    fseek(fp,4,SEEK_CUR);     // fp after class 8 header (20 bytes); Set filepointer at start line
    if (line[0]) n+=fread(line[0],1,2*hdr->cls8.earth_views_per_scanline,fp);
    if (line[1]) n+=fread(line[1],1,2*hdr->cls8.earth_views_per_scanline,fp);
    if (line[2]) n+=fread(line[2],1,2*hdr->cls8.earth_views_per_scanline,fp);
    if (line[3]) n+=fread(line[3],1,2*hdr->cls8.earth_views_per_scanline,fp);
    if (line[4]) n+=fread(line[4],1,2*hdr->cls8.earth_views_per_scanline,fp);
    fseek(fp,hdr->size-20-n-4,SEEK_CUR);     // Set filepointer after just readed record
  }
  return n;
}


/*************************************
 * Translate avhrr EPS into  guint16 string.
 * If str=NULL then get all 5 channels and
 * connect each of them with a segments in one of 5 chan's.
 *************************************/
#define AVHRR_NRCHANS 5
int eps2str(SEGMENT *segm,
            gboolean get_allchan,
            guint16 *awidth,guint16 *aheight,
            PIC_CHARS *pc)
{
  CHANNEL *chan;
  FILE *fpi;
  guint16 *nstr[AVHRR_NRCHANS];
  guint16 *line[AVHRR_NRCHANS];
  int x,y,ystart,i,x1;
  int err=0;
  int width,height;
  char fnt[200];
  HDR hdr;
  int typehdr=0;
  memset(nstr,0,sizeof(nstr));
  memset(&hdr,0,sizeof(hdr));
  /* Expected size in this segment (see init_noaachars) */  
  width=segm->chan->nc;
  height=segm->chan->nl;
  if (!width)
  {
    segm->corrupt=TRUE;
    return 1;
  }
#define KEEP_COMPRESSED
  *awidth=width;                         /* Output pic width */
  ystart=0;
  /* Bunzip2 into a temp file and open this file (or open file if not zipped) */
  if (str_ends_with(segm->pfn,".bz2"))
  {
#ifdef KEEP_COMPRESSED
    if ((err=bunzip_to_tempfile(segm->pfn,fnt))) return err;
    if (!(fpi=fopen(fnt,"rb")))
    {
      remove(fnt);                    /* Remove temp file */
      segm->corrupt=TRUE;             /* cleared with new generation */
      return Open_Rd;                 /* can't reopen wvt-info file */
    }
#else
    if ((err=bunzip_to_tempfile(segm->pfn,NULL))) return err;
    if (!(fpi=fopen(segm->pfn,"rb")))
    {
      segm->corrupt=TRUE;             /* cleared with new generation */
      return Open_Rd;                 /* can't reopen wvt-info file */
    }
#endif
  }
  else
  {
    *fnt=0;
    if (!(fpi=fopen(segm->pfn,"rb")))
    {
      segm->corrupt=TRUE;             /* cleared with new generation */
      return Open_Rd;                 /* can't open file */
    }
  }  

  /* Determine actual # lines (may be lower for first/last segment)
  */
  if (!segm->prev)
  {
    int ny=0;
    
    while (typehdr!=8)
    {
      typehdr=read_genrechdr(fpi,&hdr);
      if (typehdr<0) break;
    }
    while (typehdr==8)
    {
      ny++;
      typehdr=read_genrechdr(fpi,&hdr);
    }
    rewind(fpi);
    (*aheight)=ny;
  }
  else
  {
    (*aheight)=height;
  }
  if ((*aheight) > height) *aheight=height;

/* first and last segm may have lower amount of lines.
   Make missing lines'black'.
   Startpoint > 0 only for first segment -> ystart
*/
  if (!segm->prev) ystart=height - (*aheight);  // First: ystart !=0

/* read records until type 8 (=start-of-scanlines) */
  while (typehdr!=8)
  {
    typehdr=read_genrechdr(fpi,&hdr);
    if (typehdr<0) break;
  }

  if (typehdr!=8)
  {
    fclose(fpi);
    segm->corrupt=TRUE;
    return -2;    // error;file contains  no type 8 record 
  }
//printf("%f  %f\n",hdr.cls1.subsat_start.lon,hdr.cls1.subsat_start.lat);
/*
  Allocate mem for channels to process.
  This is either 1 channel (chan->chan_nr=1...5) or all 5 channels.
*/
  for (chan=segm->chan; chan->prev; chan=chan->prev);
  for (; chan; chan=chan->next)
  {
    nstr[chan->chan_nr-1]=calloc((*awidth)*(height),2);
  }

/* Prepare for scan reading. allocate lines for 5 channels */
  for (i=0; i<AVHRR_NRCHANS; i++)
  {
    line[i]=malloc(hdr.cls8.earth_views_per_scanline*sizeof(guint16));
  }

  /* Set to start of first line */
  fseek(fpi,-1*hdr.size,SEEK_CUR);
  for (y=0; y<(*aheight); y++)
  {
    if (!get_scanline(fpi,&hdr,pc->image_iformat,line)) break;

    for (x1=0; x1<(*awidth); x1++)
    {
      x=x1;

      /* Store pix in string. Only for channels for which nstr is allocated.
         NOTE Values may be > 16384, occupieing bits 14 and 15. 
         These are reserved for overlay, so values are clipped for the moment.
         (Change this with separate mask array!)
      */
      for (i=0; i<AVHRR_NRCHANS; i++)
      {
        if ((line[i]) && (nstr[i]))
        {
          guint16 pix;
          if (pc->image_iformat=='y')
            pix=line[i][x];
          else
            pix=GUINT16_FROM_BE(line[i][x]);
          if (pix>0x7fff) pix=0;    // there are values of 0xffff and 0xfffd in pic?
#ifndef DONT_CLIP
          if (pix>= 0x4000) pix=0x4000-1;
#endif
          *(nstr[i]+(x1+(ystart+y)*(*awidth))*1)=pix;
        }
      }
    }
  }
  fclose(fpi);
  if (*fnt) remove(fnt);                /* Remove temp file */
  for (i=0; i<AVHRR_NRCHANS; i++)
  {
    if (line[i]) free(line[i]);
  }

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
            segml->width=*awidth;
            segml->height=*aheight;
          }
          else
          {
            if (segml==segm)
            {
              segml->chnk=nstr[chan->chan_nr-1];
              segml->width=*awidth;
              segml->height=*aheight;
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



