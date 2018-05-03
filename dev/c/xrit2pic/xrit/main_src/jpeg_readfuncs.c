/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * JPEG read functions (uses IJG for basic functions)
 ********************************************************************/
#include <setjmp.h>
#include "xrit2pic.h"

#include "jinclude.h"
#include "jpeglib.h"
#include "cdjpeg.h"
#include "jpeg_funcs.h"
/* read jpeg */

struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
//  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

int open_jpeg(FILE *fpi,struct jpeg_decompress_struct *cinfo,
          JSAMPARRAY *buffer,  int *row_stride)
{
  static struct my_error_mgr jerr;
  cinfo->err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer))
  {
    /* If we get here, the JPEG code has signaled an error.
     */
    jpeg_destroy_decompress(cinfo);
    return 1;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(cinfo, fpi);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(cinfo, TRUE);

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(cinfo);

  *row_stride = cinfo->output_width * cinfo->output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  *buffer = (*cinfo->mem->alloc_sarray)
		((j_common_ptr) cinfo, JPOOL_IMAGE, *row_stride, 1);
  return 0;
}

void close_jpeg(struct jpeg_decompress_struct *cinfo,JSAMPARRAY buffer)
{
  (void) jpeg_finish_decompress(cinfo);
  jpeg_destroy_decompress(cinfo);
}

/* Open JPEG file */
int read_jpeg(FILE *fp,struct jpeg_decompress_struct *icinfo)
{
/*
  struct my_error_mgr jerr;

  if (setjmp(jerr.setjmp_buffer))
  {
    jpeg_destroy_decompress(icinfo);
    return 0;
  }

  icinfo->err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit; 
*/

  jpeg_create_decompress(icinfo);
  jpeg_stdio_src(icinfo, fp);
  jpeg_read_header(icinfo, TRUE);

  return 1;
}

int get_jpegsize_opened(FILE *fpi,int *width,int *height)
{
  int err=0;
  struct jpeg_decompress_struct cinfo;
  JSAMPARRAY buffer;		/* Output row buffer */

  err=open_jpeg(fpi,&cinfo,&buffer,width);
  if (err) return err;
  *width/=cinfo.output_components;
  *height=cinfo.output_height;
  jpeg_destroy_decompress(&cinfo);
  rewind(fpi);
  return err;
}

int get_jpegsize(char *fni,int *width,int *height)
{
  FILE *fpi;
  int err=0;
  
  fpi=fopen(fni,"rb");
  if (!fpi) return 0;
  err=get_jpegsize_opened(fpi,width,height);

  fclose(fpi);
  if (err) return 0;
  return 1;
}

int read_jpgsign(FILE *fp)
{
  unsigned char s[2];
  fread(&s,2,1,fp);
  if (s[0]!=0xff) return 0;
  if (s[1]!=SOI) return 0;
  return 1;
}

void swap2(int *n)
{
  int tmp;
  (*n)&=0xffff;
  tmp=(*n)&0xff;
  (*n)>>=8;
  (*n)+=(tmp<<8);
}

int read_marker(FILE *fp,unsigned char **l,int *mlen)
{
  unsigned char c,mark;
  *l=NULL;
/* If 0xff: Marker coming up. */
  if (!(fread(&c,1,1,fp))) return -1;
  if (c!=0xff) { fseek(fp,-1,SEEK_CUR); return 0; }

/* Read marker */
  if (!(fread(&mark,1,1,fp))) return -2;
  switch(mark)
  {
    case SOI:
    case EOI: case 0x00: 
    case RST0: case RST1: case RST2: case RST3: 
    case RST4: case RST5: case RST6: case RST7:
      *mlen=0;
      fseek(fp,-2,SEEK_CUR);
    break;
    default:
      if (!(fread(mlen,2,1,fp))) return -3;
      swap2(mlen);
      fseek(fp,-4,SEEK_CUR);
    break;
  }
  *mlen+=2;         /* incl. marker code */
  if ((*mlen>1000) || (*mlen<1)) { printf("mlen=%d\n",*mlen); return -4; }

/* Read marker */
  *l=malloc(*mlen+10);
  if (!(fread(*l,*mlen,1,fp))) return -5;

  if (mark==0x00) return ZERO;
  return mark;
}

