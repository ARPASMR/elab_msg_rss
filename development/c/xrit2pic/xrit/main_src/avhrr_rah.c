#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "xrit2pic.h"
#include "avhrr.h"

/*************************************************************************
 *************************************************************************
 ** RAH format functions.
 *************************************************************************
 *************************************************************************/

/*************************************
 * Calculate # bytes per frame for HRPT-related formats
 *************************************/
static FRAMEINFO calc_hrptframeinfo(gboolean *channels,int width)
{
  FRAMEINFO fri;
  int i;
  fri.header_len=0;
  fri.leader_len=0;

/* === Determine nr. of non-picture data === */
/* frame header present */
  fri.header_len=750;
  fri.leader_len=100;

/* === Determine words per line === */
  fri.words_per_frame=0;

/* Contribution of actual picture data */
  fri.inter_chan=0;
  for (i=1; i<NR_CHANNELS_MAX; i++)
    if (channels[i]) fri.inter_chan++;

  fri.words_per_frame=width*fri.inter_chan;

/* Contribution of header and leader. 
   Leader contains extra bytes to make words_per_frame*10/8 integer.
*/
  if (channels[0])
  {
    fri.words_per_frame+=fri.header_len;
    fri.words_per_frame+=fri.leader_len;
  }
  fri.words_per_line=fri.words_per_frame+((4-(fri.words_per_frame&0x3))&0x3);

/* Calculate file bytes per line */
  fri.bytes_per_line=fri.words_per_line*10/8;

/* Add contribution of extra 'frame separation' bytes */
  fri.bytes_per_line+=NR_INTER_BYTES;

  return fri;
}

/*************************************
 * Write hrpt header from struct
 *************************************/
static int write_hrpt1hdr_file(HRPTHDR *hrpthdr,FILE *fp)
{
  if (fp==NULL) return 1;

/* Write hrpt header in 4 steps, to make it portable between DOS and Linux */

/* Write first part: sign ...  tm_comp_sat_vlg */
  if (!(fwrite(hrpthdr,131,1,fp))) return 0;

/* Write second part: offset_lon ... bo */
  if (!(fwrite((char *)&hrpthdr->offset_lon,42,1,fp))) return 0;

/* Write third part: kr[0] ... channel[9]*/
  if (!(fwrite((char *)&hrpthdr->kr[0],105,1,fp))) return 0;

/* Write fourth part: widthmax ... end */
  if (!(fwrite((char *)&hrpthdr->widthmax,15722,1,fp))) return 0;

  return 0;
}


/*************************************
 * Write rah header 
 *************************************/
static int write_rah_hdr(PICINFO *pci)
{
  HRPTHDR *hrpthdr;
  KEPLER kepler;
  int i,k;
  int chan_max=0;
  int res=0;
  if (!pci->fp) return 0;

/* allocate mem for hrpthdr struct */
  if (!(hrpthdr=(HRPTHDR *)calloc(sizeof(HRPTHDR),1))) return 0;
  strncpy(hrpthdr->sign,HRPT1SIGN,LENSIGN_H);
  chan_max=5;

  if (!pci->width) pci->width=DEF_NOAA_XRES;

  hrpthdr->width=GUINT16_TO_LE(pci->width);
  hrpthdr->widthmax=GUINT16_TO_LE(DEF_NOAA_XRES);
  hrpthdr->alphamax=DEF_NOAA_ALPHAMAX;

  for (k=0; k<chan_max; k++) hrpthdr->dmin[k]=GUINT16_TO_LE(0);
  for (k=0; k<chan_max; k++) hrpthdr->dmax[k]=GUINT16_TO_LE(1023);

  pci->fri=calc_hrptframeinfo(pci->channels_recorded,pci->width);

  strncpy(hrpthdr->sat_name,pci->sat_name,LENSATNAME_HDR1);

  hrpthdr->satdir=0; // n->s
  hrpthdr->x_offset=0;
  hrpthdr->y_offset=0;

  for (i=0; i<=chan_max; i++)
  {
    hrpthdr->channel[i]=TRUE;
  }

  pci->orbit.start_tm_sat=pci->orbit.start_tm;
  pci->orbit.start_ms_sat=pci->orbit.start_ms;

  pci->orbit.start_tm_comp=pci->orbit.start_tm_sat;

/* starttime from satellite */
  hrpthdr->tm_satstrt.tm_year=GUINT16_TO_LE(pci->orbit.start_tm_sat.tm_year);
  hrpthdr->tm_satstrt.tm_yday=GUINT16_TO_LE(pci->orbit.start_tm_sat.tm_yday);
  hrpthdr->tm_satstrt.tm_hour=GUINT16_TO_LE(pci->orbit.start_tm_sat.tm_hour);
  hrpthdr->tm_satstrt.tm_min=GUINT16_TO_LE(pci->orbit.start_tm_sat.tm_min);
  hrpthdr->tm_satstrt.tm_sec=GUINT16_TO_LE(pci->orbit.start_tm_sat.tm_sec);
  hrpthdr->tm_satstrt.tm_mon=GUINT16_TO_LE(pci->orbit.start_tm_sat.tm_mon);
  hrpthdr->tm_satstrt.tm_mday=GUINT16_TO_LE(pci->orbit.start_tm_sat.tm_mday);
  hrpthdr->tm_satstrt.tm_isdst=GUINT16_TO_LE(pci->orbit.start_tm_sat.tm_isdst);

/* starttime from computer */
  hrpthdr->tm_comp.tm_year=GUINT16_TO_LE(pci->orbit.start_tm_comp.tm_year);
  hrpthdr->tm_comp.tm_yday=GUINT16_TO_LE(pci->orbit.start_tm_comp.tm_yday);
  hrpthdr->tm_comp.tm_hour=GUINT16_TO_LE(pci->orbit.start_tm_comp.tm_hour);
  hrpthdr->tm_comp.tm_min=GUINT16_TO_LE(pci->orbit.start_tm_comp.tm_min);
  hrpthdr->tm_comp.tm_sec=GUINT16_TO_LE(pci->orbit.start_tm_comp.tm_sec);
  hrpthdr->tm_comp.tm_mon=GUINT16_TO_LE(pci->orbit.start_tm_comp.tm_mon);
  hrpthdr->tm_comp.tm_mday=GUINT16_TO_LE(pci->orbit.start_tm_comp.tm_mday);
  hrpthdr->tm_comp.tm_isdst=GUINT16_TO_LE(pci->orbit.start_tm_comp.tm_isdst);

  hrpthdr->tm_comp_sat_vlg=(pci->orbit.use_sattime? 's' : 'c');
  hrpthdr->height=GUINT16_TO_LE(pci->height);
  strcpy(hrpthdr->hdr_version,HRPTHDRVERSION);
  hrpthdr->hdrlen=GUINT16_TO_LE(16000); /* sizeof(*hrpthdr); */
  strncpy(hrpthdr->endhdr,ENDHDR,8);

/* Catch Kepler data */
  kepler=pci->kepler;
  hrpthdr->kepler.epoch_year=GINT32_TO_LE(kepler.epoch_year);
  hrpthdr->kepler.epoch_day=float32_to_le(kepler.epoch_day);
  hrpthdr->kepler.decay_rate=float32_to_le(kepler.decay_rate);
  hrpthdr->kepler.inclination=R2D(float32_to_le(kepler.inclination));
  hrpthdr->kepler.raan=R2D(float32_to_le(kepler.raan));
  hrpthdr->kepler.eccentricity=float32_to_le(kepler.eccentricity);
  hrpthdr->kepler.perigee=R2D(float32_to_le(kepler.perigee));
  hrpthdr->kepler.anomaly=R2D(float32_to_le(kepler.anomaly));
  hrpthdr->kepler.motion=float32_to_le(kepler.motion);
  hrpthdr->kepler.epoch_rev=GINT32_TO_LE(kepler.epoch_rev);
  
  write_hrpt1hdr_file(hrpthdr,pci->fp);

  free(hrpthdr);
  /* res=0-> No kepler data found */
  return res;
}

/*************************************
 * Define and write rah header 
 *************************************/
static PICINFO write_rah_header(FILE *fp,int height,GROUP *grp)
{
  PICINFO pci;
  int i;
  memset(&pci,0,sizeof(pci));
  pci.fp=fp;
  pci.height=height;
  for (i=0; i<=5; i++) pci.channels_recorded[i]=TRUE;
  if (grp) strcpy(pci.sat_name,grp->sat_src);
  strtoupper(pci.sat_name);
  
  pci.orbit.use_sattime=TRUE;
  if (grp)
  {
    pci.kepler=grp->pc.kepler;
    pci.orbit.start_tm=grp->grp_tm;
  }

  write_rah_hdr(&pci);
  return pci;
}

/*************************************
 * Write a line in hrpt1-format.
 * Each line starts with a 7-byte header 'FRAMExy"; 
 * xy=amount of bytes of actual line.
 * fri.bytes_per_line includes the 7 intermediate bytes;
 * so value of 'xy' is fri.bytes_per_line-NR_INTER_BYTES (=7).
 *************************************/
static int write_hrpt1_line(PICINFO *pci,guint16 *line,int line_stat)
{
  guchar tmp[6];
  FILE *fp=pci->fp;
  short bytes_per_line=pci->fri.bytes_per_line-NR_INTER_BYTES;
  guint16 *pline;
  int nr_qwords=pci->fri.words_per_line/4;
  int i,j;
  if (!fp) return 0;

  if (line_stat & MASK_HRPT1_SYNC_OK_PRE)
    strcpy((char *)tmp,"FRAME");
  else
    strcpy((char *)tmp,"FRAMe");

  fwrite(tmp,5,1,fp);
  fwrite(&bytes_per_line,1,2,fp);

  pline=line;
  for (i=0; i<nr_qwords; i++)
  {

    tmp[4]=0;
    for (j=0; j<4; j++)
    {
      tmp[j]=(*pline)>>2;

      tmp[4]=tmp[4] | ((*pline) & 0x03) << ((3-j)*2);
      pline++;
    }

    fwrite(tmp,5,1,fp);
  }
  return 1;
}

/*************************************
 * Write all data from raw16 into rah format
 *************************************/
static int write_rah_lines(PICINFO *pci,FILE *fpi)
{
  int i;
  int nrl=0;
  guint16 line[12000];
  while (fread(line,2,2048*5+850,fpi))
  {
    for (i=0; i<11090; i++) line[i]=GUINT16_FROM_BE(line[i]);
    write_hrpt1_line(pci,line,0xff);
    nrl++;
  }
  return nrl;
}

extern GtkWidget *progress_gen;
/*************************************
 * Translate raw16 file into rah format
 *************************************/
static int noaa_raw16torah(GROUP *grp,char *fno,gboolean show_progress)
{
  CHANNEL *chan=grp->chan;
  GtkWidget *progress=NULL;
  SEGMENT *segm;
  FILE *fpi,*fpo;
  PICINFO pci;
  int height=0,exp_height=0;
  int start_sec=0;
  char fnt[200];
  int err=0;

  if (!(fpo=fopen(fno,"wb"))) return 1;

  if (show_progress)
  {
    progress=Create_Progress(NULL,"Translate...",TRUE);
    for (segm=chan->segm; segm; segm=segm->next)
      exp_height+=360;
  }
      
  pci=write_rah_header(fpo,height,chan->group);
  for (segm=chan->segm; segm; segm=segm->next)
  {
    int hsegm;
    *fnt=0;

#ifndef __NOGUI__
    if (progress)
    {
      if (Update_Progress(progress,height,exp_height))
      {
        err=1;
        break;
      }
    }
    else
    {
      if ((progress_gen) && (Get_Progress_state(progress_gen)))
      {
        err=1;
        break;
      }
      while (g_main_iteration(FALSE));
    }
#endif
    if (str_ends_with(segm->pfn,".bz2"))
    {
      if ((bunzip_to_tempfile(segm->pfn,fnt))) break;
      if (!(fpi=fopen(fnt,"rb")))
      {
        err=1;
        remove(fnt);                    /* Remove temp file */
        break;                          /* can't reopen temp file */
      }
    }
    else
    {
      if (!(fpi=fopen(segm->pfn,"rb")))
      {
        err=1;
        break;
      }
    }
    
    hsegm=write_rah_lines(&pci,fpi);
    if (!height) start_sec=60-(hsegm/6);
    height+=hsegm;
    fclose(fpi);
    if (*fnt) remove(fnt);                    /* Remove temp file */
  }
  rewind(fpo);
  pci.height=height;
  pci.orbit.start_tm.tm_sec=start_sec;
  pci.kepler=grp->pc.kepler;
  if (!write_rah_hdr(&pci)) grp->has_kepler=TRUE;
  fclose(fpo);
  if (show_progress) Close_Progress(progress);
  return err;
}

int get_segmset(SEGMENT *segm,SEGMENT *csegm[])
{
  CHANNEL *chan;
  int nrsegm=segm->xh.segment;
  int nch=0;
  chan=segm->chan;
  while (chan->prev) chan=chan->prev;
  for (nch=0; ((nch<5) && (chan)); chan=chan->next,nch++)
  {
    for (segm=chan->segm; segm; segm=segm->next)
    {
      if (segm->xh.segment==nrsegm)
      {
        csegm[nch]=segm;
        break;
      }
    }
  }
  return nch;
}

static int metop_write_rah_lines(PICINFO *pci,SEGMENT *segm)
{
  guint16 line[12000];
  int i,nrl;
  CHANNEL *chan;
  SEGMENT *csegm[5];
  guint16 awidth,aheight;
  memset(line,0,sizeof(line));
  eps2str(segm,TRUE,&awidth,&aheight,NULL);
  get_segmset(segm,&csegm);
  for (nrl=0; nrl<aheight; nrl++)
  {
    for (i=0; i<2048; i++)
    {
      int nrch=0;
      for (chan=segm->chan; chan; chan=chan->next)
      {
        line[750+i*5+nrch]=((csegm[nrch]->chnk[i+nrl*2048])>>4) & 0x3ff;
        nrch++;
        if (nrch>=5) break;
      }
    }
    write_hrpt1_line(pci,line,0xff);
  }
  return aheight;
}


static int metop_raw16torah(GROUP *grp,char *fno,gboolean show_progress)
{
  CHANNEL *chan=grp->chan;
  GtkWidget *progress=NULL;
  SEGMENT *segm;
  FILE *fpo;
  PICINFO pci;
  int height=0,exp_height=0;
  int start_sec=0;
  int err=0;
  
  if (!(fpo=fopen(fno,"wb"))) return 1;

  if (show_progress)
  {
    progress=Create_Progress(NULL,"Translate...",TRUE);
    for (segm=chan->segm; segm; segm=segm->next)
      exp_height+=1080;
  }
  pci=write_rah_header(fpo,height,chan->group);
  for (segm=chan->segm; segm; segm=segm->next)
  {
    int hsegm;

#ifndef __NOGUI__
    if (progress)
    {
      if (Update_Progress(progress,height,exp_height))
      {
        err=1;
        break;
      }
    }
    else
    {
      if ((progress_gen) && (Get_Progress_state(progress_gen)))
      {
        err=1;
        break;
      }
      while (g_main_iteration(FALSE));
    }
#endif

#ifdef XXX
    if ((bunzip_to_tempfile(segm->pfn,fnt))) break;
    if (!(fpi=fopen(fnt,"rb")))
    {
      err=1;
      remove(fnt);                    /* Remove temp file */
      break;                          /* can't reopen temp file */
    }
#endif
    hsegm=metop_write_rah_lines(&pci,segm);
    if (!height) start_sec=180-(hsegm/6);
    height+=hsegm;
#ifdef XXX
    fclose(fpi);
    remove(fnt);                    /* Remove temp file */
#endif
  }
  rewind(fpo);
  pci.height=height;
  pci.orbit.start_tm.tm_sec=start_sec;
  pci.kepler=grp->pc.kepler;
  if (!write_rah_hdr(&pci)) grp->has_kepler=TRUE;
  fclose(fpo);
  if (show_progress) Close_Progress(progress);
  return err;
}

int raw16torah(GROUP *grp,char *fno,gboolean show_progress)
{
  if (grp->h_or_l=='M')
    return metop_raw16torah(grp,fno,show_progress);
  else
    return noaa_raw16torah(grp,fno,show_progress);
}
