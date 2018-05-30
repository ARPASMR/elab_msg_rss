/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * JPEG interface functions
 ********************************************************************/
#include <stdio.h>
#include <setjmp.h>
#include "xrit2pic.h"

#include "jinclude.h"
#include "jpeglib.h"
#include "cdjpeg.h"
#include "xrit2pic_funcs.h"

#include "jpeg_funcs.h"

int jpg2str1(char *fn,guchar **str,int *W,int *H)
{
  struct jpeg_decompress_struct cinfo;
  JSAMPARRAY buffer;		/* Output row buffer */
  FILE *fpi;
  int width;

  if (!(fpi=fopen(fn,"rb")))
  {
    return 0; 
  }

  if (open_jpeg(fpi,&cinfo,&buffer,&width))
  {
    fclose(fpi);
    return 0;
  }

  width/=cinfo.output_components;
  *W=width;
  *H=cinfo.output_height;
  *str=malloc(cinfo.output_height*width*3);
  while (cinfo.output_scanline < cinfo.output_height)
  {
    int x;
    int y=cinfo.output_scanline;
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    for (x=0; x<width; x++)
    {
      (*str)[(x+y*width)*3+0]=buffer[0][x*cinfo.output_components];
      (*str)[(x+y*width)*3+1]=buffer[0][x*cinfo.output_components+1];
      (*str)[(x+y*width)*3+2]=buffer[0][x*cinfo.output_components+2];
    }
  }
  close_jpeg(&cinfo,buffer);
  fclose(fpi);

  return 1;
}


int jpg2str(SEGMENT *segm,guint16 **str,guint16 *W,guint16 *H,guint16 *D)
{
  struct jpeg_decompress_struct cinfo;
  JSAMPARRAY buffer;		/* Output row buffer */
  FILE *fpi;
  int width;
  if (!(fpi=open_xritimage(segm->pfn,NULL,segm->xh.xrit_frmt,segm->chan)))
  {
    segm->corrupt=TRUE;         /* cleared with new generation */
    return 1; 
  }

  if (open_jpeg(fpi,&cinfo,&buffer,&width))
  {
    fclose(fpi);
    segm->corrupt=TRUE;         /* cleared with new generation */
    return 1;
  }
  
  width/=cinfo.output_components;
  *W=width;
  *H=cinfo.output_height;
  *str=malloc(cinfo.output_height*width*2);
  while (cinfo.output_scanline < cinfo.output_height)
  {
    int x;
    int y=cinfo.output_scanline;
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    for (x=0; x<width; x++)
    {
      (*str)[x+y*width]=buffer[0][x*cinfo.output_components];
    }
  }
  close_jpeg(&cinfo,buffer);
  fclose(fpi);

  return 0;
}


int gen_jpgfile(GROUP *grp,PIC_CHARS *pc,char *fno,gboolean show_progress)
{
  SEGMENT *segm;
  FILE *fpi;
  FILE *fpt;
  FILE *fpo;
  int rstmarker=0;
  int err=0;
  int nr=0;
  GtkWidget *progress;

  int debug=0;

  if (!(fpo=fopen(fno,"wb"))) return Open_Wr;

  if (show_progress) progress=Create_Progress(NULL,"Translate...",TRUE);

  for (segm=grp->chan->segm; segm; segm=segm->next)
  {
/* Open files */
    if (!(fpi=open_xritimage(segm->pfn,NULL,segm->xh.xrit_frmt,segm->chan))) { err=Open_Rd; goto end; }
    if (debug)
    {
      if (!(fpt=fopen("tmp_debug.jpg","w+"))) { err=Open_Wrt; goto end; }
    }
    else
    {
      if (!(fpt=tmpfile())) { err=Tmp_fn; goto end; }
    }
/* Set restart markers to end-of-image and write to temp file (Uses IJG!) */
    if ((err=jpeg_rst1(fpi,fpt))) goto end;

/* Close current i-file */ 
    fclose(fpi);
    rewind(fpt);
   
/* Read/write bytes between markers */
    err=copy_jpegbytes(fpt,fpo,(segm==grp->chan->segm),pc->nrchuncks,&rstmarker);
    fclose(fpt);
    if (err) goto end;

    if ((show_progress) && (Update_Progress(progress,++nr,pc->nrchuncks)))
    {
      Create_Message("Warning","Aborted after %d of %d pieces.",nr,pc->nrchuncks);
      err=Aborted;
      break;
    }
  }
  write_marker(fpo,EOI);                   /* Write EOI after last added file */

end:
  if (show_progress) Close_Progress(progress);
  fclose(fpo);

  return err;
}

#include <setjmp.h>
struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

void error_jpeg(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message) (cinfo, buffer);

  Create_Message("JPEG Error!",buffer);
//  (*cinfo->err->output_message) (cinfo);
//  jpeg_destroy(cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

int open_wr_jpeg(FILE *fpo,int width,int height,int depth,struct jpeg_compress_struct *cinfo,int quality)
{
//  struct jpeg_error_mgr jerr;
  struct my_error_mgr jerr;
  /* Step 1: allocate and initialize JPEG compression object */
  cinfo->err = jpeg_std_error(&jerr);
  jerr.pub.error_exit=error_jpeg;
  if (setjmp(jerr.setjmp_buffer))
  {
    /* If we get here, the JPEG code has signaled an error.
     */
    return -1;
  }

  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(cinfo);

  /* Step 2: specify data destination (eg, a file) */
  jpeg_stdio_dest(cinfo, fpo);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo->image_width = width; 	/* image width and height, in pixels */
  cinfo->image_height = height;
  cinfo->input_components = 3;		/* # of color components per pixel */
  cinfo->in_color_space = JCS_RGB; 	/* colorspace of input image */

  jpeg_set_defaults(cinfo);
  cinfo->data_precision=depth;
  
  cinfo->bits_in_jsample=cinfo->data_precision;
  cinfo->MAXJSAMPLE=(1<<cinfo->bits_in_jsample)-1;
  cinfo->CENTERJSAMPLE=(1<<(cinfo->bits_in_jsample-1))-1; 

  jpeg_set_quality(cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  jpeg_start_compress(cinfo, TRUE);
  return 0;
}

/* Open jpeg-file for write. If fp=null: just return pointer to cinfo */
j_compress_ptr write_jpghdr(FILE *fp,int width,int height,int depth,int quality)
{
  static struct jpeg_compress_struct cinfo;
  if (fp)
  {
    if (open_wr_jpeg(fp, width, height, 8, &cinfo,quality))
    {
      return NULL;   // Internal JPEG error.
    }
  }
  return &cinfo;
}

void close_wr_jpeg()
{

  j_compress_ptr cinfo;
  cinfo=write_jpghdr(NULL,0,0,0,0);  /* just get cinfo from static */
  if (!cinfo) return;  // internal JPEG error
  
  /* Step 6: Finish compression */

  jpeg_finish_compress(cinfo);

  /* Step 7: release JPEG compression object */

  jpeg_destroy_compress(cinfo);

}



int write_jpgline(guchar *str)
{
  int x,x3,row_stride;
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  JSAMPLE *image_buffer;          /* Points to large array of R,G,B-order data */
  j_compress_ptr cinfo;
  cinfo=write_jpghdr(NULL,0,0,0,0);  /* just get cinfo from static */
  if (!cinfo) return -1;  // internal JPEG error
  
  row_stride = cinfo->image_width * cinfo->input_components; /* JSAMPLEs per row */
  image_buffer=malloc(row_stride*sizeof(JSAMPLE));

  for (x=0; x<cinfo->image_width; x++)
  {
    x3=x*3;
    image_buffer[x3+0]=str[x3+0];
    image_buffer[x3+1]=str[x3+1];
    image_buffer[x3+2]=str[x3+2];
  }

  row_pointer[0] = &image_buffer[0];

  jpeg_write_scanlines(cinfo, row_pointer, 1);
  free(image_buffer);
  return 0;
}

