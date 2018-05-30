/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * JPEG write functions (uses IJG for basic functions)
 ********************************************************************/
#include "xrit2pic.h"

#include "jinclude.h"
#include "jpeglib.h"
#include "cdjpeg.h"
#include "jpeg_funcs.h"

void write_marker(FILE *fp,unsigned char mark)
{
  unsigned char l[2];
  l[0]=0xff;
  l[1]=mark;
  fwrite(l,2,1,fp);
}

void wr_rstmark(FILE *fp,int n)
{
  unsigned char mark;
  mark=RST0+(n%8);
  write_marker(fp,mark);
}

int write_jpeg(FILE *fp,struct jpeg_decompress_struct *icinfo,jvirt_barray_ptr *src_coef_arrays)
{
  struct jpeg_compress_struct ocinfo;
  struct jpeg_error_mgr jerr;
  int rst;
  ocinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&ocinfo);
  jpeg_copy_critical_parameters(icinfo,&ocinfo);

  if (ocinfo.image_height%8) return -1;
  rst=ocinfo.image_height/8;
  ocinfo.restart_in_rows=rst;

  jpeg_stdio_dest(&ocinfo, fp);
  jpeg_write_coefficients(&ocinfo,src_coef_arrays);
  jpeg_finish_compress(&ocinfo);
  jpeg_destroy_compress(&ocinfo);
  return 0;
}

int write_ppm(FILE *fp,struct jpeg_decompress_struct *icinfo,djpeg_dest_ptr dest_mgr)
{
  int num_scanlines;
  dest_mgr->big_endian=TRUE; 

  /* Process data */
  while (icinfo->output_scanline < icinfo->output_height)
  {
    num_scanlines = jpeg_read_scanlines(icinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (icinfo, dest_mgr, num_scanlines);
  }
/*
  (*dest_mgr->finish_output) (icinfo, dest_mgr);
*/
  return 0;
}

int jpeg_rst1(FILE *fpi,FILE *fpo)
{
  struct jpeg_decompress_struct icinfo;
  jvirt_barray_ptr *src_coef_arrays;
  struct jpeg_error_mgr jerr;
  icinfo.err = jpeg_std_error(&jerr);

  if (!(read_jpeg(fpi,&icinfo))) return JPG_Rd;
  if (!(src_coef_arrays=jpeg_read_coefficients(&icinfo))) return JPG_Rd;

/* Write JPEG data of one part */
  if (write_jpeg(fpo,&icinfo,src_coef_arrays)) return JPG_Wr;

/* Close JPEG */
  jpeg_finish_decompress(&icinfo);

  jpeg_destroy_decompress(&icinfo);

  return 0;
}


int jpeg2pgm(FILE *fpi,FILE *fpo,int first,int nrfiles)
{
  struct jpeg_decompress_struct icinfo;
  djpeg_dest_ptr dest_mgr;
  struct jpeg_error_mgr jerr;
  icinfo.err = jpeg_std_error(&jerr);

/*
  struct my_error_mgr jerr;

  if (setjmp(jerr.setjmp_buffer))
  {
    jpeg_destroy_decompress(&icinfo);
    return 0;
  }
*/

/* Open JPEG file */
  if (!(read_jpeg(fpi,&icinfo))) return JPG_Rd;

/* Init PGM write */
  dest_mgr = jinit_write_ppm(&icinfo);
  dest_mgr->output_file=fpo;

  jpeg_start_decompress(&icinfo);

/* Write PGM header if this is first part */
  if (first)
  {
    struct jpeg_decompress_struct ocinfo;
    ocinfo=icinfo;
    ocinfo.output_height*=nrfiles;
    (*dest_mgr->start_output) (&ocinfo, dest_mgr);
  }

/* Write PGM data of one part */
  if (write_ppm(fpo,&icinfo,dest_mgr)) return JPG_Wr;

/* Close PGM writing */
  (*dest_mgr->finish_output) (&icinfo, dest_mgr);

/* Close JPEG */
  jpeg_finish_decompress(&icinfo);
  jpeg_destroy_decompress(&icinfo);

  return 0;
}



void copy_jpegimage(FILE *fp,FILE *fpo)
{
  unsigned char c;
  while (fread(&c,1,1,fp))
  {
    if (c==0xff)
    {
      fread(&c,1,1,fp);
      if (c==0x00)
      {
        c=0xff; fwrite(&c,1,1,fpo);
        c=0x00; fwrite(&c,1,1,fpo);
        continue;
      }
      else
      {
        fseek(fp,-2,SEEK_CUR);
        break;
      }
    }
    fwrite(&c,1,1,fpo);
  }
}

int copy_jpegbytes(FILE *fpt,FILE *fpo,int first,int nrfiles,int *rstmarker)
{
  static long pos_sos;
  unsigned char *l=NULL;
  int mlen;
  int mark;
  while ((mark=read_marker(fpt,&l,&mlen)) > 0)
  {
    switch(mark)
    {
      case SOF0: case SOF1:
      {
        guint16 height,*heightp=(guint16 *)(l+5);
        if (first)
        {
          height=GUINT16_FROM_BE(*heightp);
          height=height*(nrfiles);         /* Adapt height */
          *heightp=GUINT16_TO_BE(height);
          fwrite(l,mlen,1,fpo);            /* Write SOF0 of first file only */
        }
      }
      break;
      case SOS:
        if ((!first) && (ftell(fpt) !=pos_sos))
          return JPG_Conc;
        pos_sos=ftell(fpt);
      case RST0:
        if (first)
        {
          fwrite(l,mlen,1,fpo);             /* Write SOS of first file only */
        }
        copy_jpegimage(fpt,fpo);            /* Copy actual image */
      break;
      case EOI:
        wr_rstmark(fpo,(*rstmarker)++);     /* Replace by restart marker */
      break;
      default:
        if (first)
        {
          fwrite(l,mlen,1,fpo);             /* Write headers of first file only */
        }
      break;
    }
    if (l) { free(l); l=NULL; }
  }
  if (l) { free(l); l=NULL; }
  return 0;
}

/*
  JPEG-file:
    <SOI>                                  first written, rest skip
    <other_markers & data>                 first written, rest skip
    <SOF0> <.. .. .. .. .. height ......>  change height, first written, rest skip
    <other_markers & data>                 first written, rest skip
    <SOS>                                  first written, rest skip
    <image data>                           copy
    <EOI>                                  replace by restart, add EOI to last
*/

