/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * PGM related funcs
 ********************************************************************/
#include "xrit2pic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PGM_SIGN "P5"
#define PPM_SIGN "P6"

/***********************************************
 * Write PGM header
 ***********************************************/
void write_pgmhdr(FILE *fp,int Width,int Height,int Depth,int is_color,char *cmt)
{
  if (is_color)
  {
    Depth=MIN(Depth,8);
    fprintf(fp,"%s\n",PPM_SIGN);
  }
  else
  {
    fprintf(fp,"%s\n",PGM_SIGN);
  }

  if (cmt)
  {
    fprintf(fp,"#");
    while (*cmt)
    {
      fprintf(fp,"%c",*cmt);
      if ((*cmt=='\n') && (*(cmt+1))) fprintf(fp,"#");
      cmt++;
    }
    if (*(cmt-1) != '\n') fprintf(fp,"\n");
  }
  fprintf(fp,"%d %d\n%d\n",Width,Height,(1<<Depth)-1);

}

int read_pgmhdr(FILE *fp,int *Width,int *Height,int *Depth)
{
  char tmp[100],*p,c;
  int wpp;
  int err=0;
  fread(tmp,2,1,fp);
/* ---- Check sign ---- */
  if (!memcmp(tmp,PPM_SIGN,strlen(PPM_SIGN)))      wpp=3;
  else if (!memcmp(tmp,PGM_SIGN,strlen(PGM_SIGN))) wpp=1;
  else                                             return 1;
/* ---- Get comment ---- */
  while ((c=fgetc(fp)) && (c!=0x0a))       /* goto new line */
    if (c==EOF) return 2;
  while ((c=fgetc(fp)) && (c==' '));       /* skip spaces */
  fseek(fp,-1,SEEK_CUR);

  while ((c=fgetc(fp)) && (c=='#'))        /* detect comment */
  {
    while ((c=fgetc(fp)) && (c!=0x0a))
    {
      if (c==EOF) return 2;
    }
  }

  fseek(fp,-1,SEEK_CUR);

/* ---- get size ---- */

  p=tmp;
  while ((c=fgetc(fp)) && (c!=0x0a))
  {
    if (c==EOF) return 2;
    *p=c; p++;        
    if (p-tmp >= 99) return 3;
  }
  *p=0;
  if (!(p=strtok(tmp," "))) return 4; *Width=atoi(p);
  if (!(p=strtok(NULL," "))) return 5; *Height=atoi(p);

/* get max. value; goto end header */
  p=tmp;
  while ((c=fgetc(fp)) && (c!=0x0a))
  {
    if (c==EOF) return 2;
    *p=c; p++;        
    if (p-tmp >= 99) return 3;
  }
  *p=0;
  if (!(p=strtok(tmp," "))) return 4;
  if (Depth) *Depth=atoi(p) + 1;
  return err;
}

int get_pgmsize_opened(FILE *fpi,int *width,int *height)
{
  int err=0;
  err=read_pgmhdr(fpi,width,height,NULL);
//  rewind(fpi);
  return err;
}

int get_pgmsize(char *fni,int *width,int *height)
{
  FILE *fpi;
  int err=0;
  
  fpi=fopen(fni,"rb");
  if (!fpi) return 0;
  err=get_pgmsize_opened(fpi,width,height);

  fclose(fpi);
  if (err) return 0;
  return 1;
}


int pgm2str(SEGMENT *segm,guint16 **str,guint16 *W,guint16 *H,guint16 *D)
{
  FILE *fpi;
  int width;
  int height;
  int x,y;
  unsigned char c;
  
  if (!(fpi=open_xritimage(segm->pfn,NULL,segm->xh.xrit_frmt,segm->chan)))
  {
    segm->corrupt=TRUE;         /* cleared with new generation */
    return 1; 
  }

  if (get_pgmsize_opened(fpi,&width,&height))
  {
    fclose(fpi);
    segm->corrupt=TRUE;         /* cleared with new generation */
    return 1;
  }  

  *W=width;
  *H=height;
  *str=malloc(height*width*2);
  for (y=0; y < height; y++)
  {
    for (x=0; x<width; x++)
    {
      fread(&c,1,1,fpi);
      *((*str)+x+y*width)=c;
    }
  }

  fclose(fpi);

  return 0;
}
