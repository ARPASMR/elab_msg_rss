/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Wavelet interface functions
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "xrit2pic.h"

#if __SOLARIS__ == 1
/*******************************************************************
 * This func is for some reason missing in C++-lib Solaris. 
 * Doesn't do anything?
 * Just define an empty func, to make linker happy.
 ********************************************************************/
void _Unwind_Resume_or_Rethrow()
{
}
#endif

/******************************************************
 * Write Wavelet header file for temp file.
 * Contains size of complete picture.
 ******************************************************/
static void wri_hdr(SEGMENT *segm,FILE *fpo,int nrchuncks)
{
  guint16 w;
  w=GUINT16_TO_BE(segm->chan->nc);
  fwrite(&w,1,2,fpo);

  w=GUINT16_TO_BE((guint16)(segm->chan->nl*nrchuncks));
  fwrite(&w,1,2,fpo);

  w=GUINT16_TO_BE((guint16)(segm->chan->nb));
  fwrite(&w,1,2,fpo);

  w=0;
  fwrite(&w,1,2,fpo);
}

/******************************************************
 * Write Wavelet sub-header file for temp file.
 * Contains size of one chunck of picture.
 ******************************************************/
static void wri_subhdr(XRIT_HDR *xrit_hdr,FILE *fpo)
{
  guint16 w;
  guint32 lw;

  w=GUINT16_TO_BE((guint16)xrit_hdr->nb);
  fwrite(&w,1,2,fpo);

  w=GUINT16_TO_BE((guint16)xrit_hdr->nl);
  fwrite(&w,1,2,fpo);

  lw=GUINT32_TO_BE((guint32)xrit_hdr->data_len);
  fwrite(&lw,1,4,fpo);

}


/******************************************************
 * Write temp. wavelet file, containing 1 chunck.
 * This is NO standard file format!
 ******************************************************/
int wvt_extract1(SEGMENT *segm,char *fno)
{
  XRIT_HDR xrit_hdr1;
  unsigned char c[2];
  FILE *fpi,*fpo;
  int nrb=0;  
  if (!(fpo=fopen(fno,"wb"))) return Open_Wr;
  wri_hdr(segm,fpo,1);                         /* write header */

  if (!(fpi=open_xritimage(segm->pfn,&xrit_hdr1,xrit_hdr1.xrit_frmt,segm->chan)))
  {
    fclose(fpo);
    remove(fno);                              /* Remove header file */
    return Open_Rd;
  }
  wri_subhdr(&xrit_hdr1,fpo);                 /* write sub-header */
  while (fread(c,1,1,fpi))
  {
    fwrite(c,1,1,fpo); /* write data */
    nrb++;
  }
  fclose(fpi);
  if (xrit_hdr1.img_obs_time) free(xrit_hdr1.img_obs_time); xrit_hdr1.img_obs_time=NULL;
  fclose(fpo);
  if (nrb!=xrit_hdr1.data_len) return 'e';
  return 0;
}

int create_file(char *fn)
{
  FILE *fp;
  fp=fopen(fn,"wb");
  if (!fp) return 0;
  fclose(fp);
  return 1;
}

/******************************************************
 * Translate wavelet in XRIT file (1 chunck) into plain pic-words
 * and store in string.
 * str is allocated here!
 ******************************************************/
int wvt2str(SEGMENT *segm,guint16 **str,guint16 *W,guint16 *H,guint16 *D)
{
  guint16 ch;
  unsigned char bpp;
  int fd=0;
  int err;
#if __HAS_NO_WVT__ != 1
  #if __GTK_WIN32__ == 1
    char *fnt;
  #else
    char fnt[50];
  #endif
  if (!segm) return Open_Rd;

/* Create and open temp. file. */
  #if __GTK_WIN32__ == 1
    if ((!(fnt=tmpnam(NULL))) || (!(create_file(fnt))))
    {
      fnt="xrit2pic_temp.tmp";
      if (!(create_file(fnt))) return Tmp_fn;
    } 
  #else
    strcpy(fnt,"/tmp/tmp_xrit2pic_XXXXXX");
    if ((fd=mkstemp(fnt)) == -1) return Tmp_fn;
  #endif
  /* Extract WVT from XRIT-file to temp-file */
  if ((err=wvt_extract1(segm,fnt)))
  {
    #if __GTK_WIN32__ != 1
      close(fd);
    #endif
    segm->corrupt=TRUE;    /* cleared with new generation */
    return err;            /* can't extract wvt-info */
  }

  /* Re-open file */
  if ((err=wvt_open(fnt,W,H,D)))
  {
    #if __GTK_WIN32__ != 1
      close(fd);
    #endif
    remove(fnt);                    /* Remove temp file */
    segm->corrupt=TRUE;             /* cleared with new generation */
    return Open_Rd;                 /* can't reopen wvt-info file */
  }

  /* Translate wvt into plain words in 'str' */
  if ((err=wvt_read(*W,str,&ch,&bpp))) 
  {
    wvt_close();
    #if __GTK_WIN32__ != 1
      close(fd);
    #endif
    remove(fnt);                   /* Remove temp file */
    segm->corrupt=TRUE;            /* cleared with new generation */
    return WVT_Err;                /* can't read wvt-file */
  }

  /* Successfully extracted and decompressed wvt into str */
  wvt_close();                 /* Close temp file */
  #if __GTK_WIN32__ != 1
    close(fd);
  #endif
  remove(fnt);                /* Remove temp file */

  return 0;
#else
  return 1;
#endif
}
