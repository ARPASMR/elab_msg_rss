/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include "avifrmt.h"
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>


static void get_1file_sz(FILELIST *fl)
{
  struct stat s;
  if (stat(fl->fn,&s)==-1)
  {
    fl->size=0;
    fl->offset=4;
  }
  else
  {
    fl->size=s.st_size;
    fl->size+=((4-(fl->size%4)) % 4); /* size file, 4-bytes rounded */

    if (fl->prev)
      fl->offset=fl->prev->offset+fl->prev->size+8;
    else
      fl->offset=4;
  }
}

static unsigned long get_file_sz(FILELIST *fl)
{
  unsigned long tot_size=0;
  for (; fl; fl=fl->next)
  {
    if (fl->dont_use) continue;
    get_1file_sz(fl);
    tot_size+=fl->size;               /* sum sizes */
  }
  return tot_size;
}

static int nr_files(FILELIST *fl)
{
  int n=0;
  for (; fl; fl=fl->next)
  {
    if (!fl->dont_use) n++;
  }
  return n;
}

static void fwrite_quartet(FILE *fpo,unsigned long i)
{
  i=GUINT32_TO_LE(i);
  fwrite(&i,4,1,fpo);
}

static unsigned long fread_quartet(FILE *fp,unsigned long *i)
{
  int r;
  unsigned long tmp;
  r=fread(&tmp,4,1,fp);
  if (i)
    *i=GUINT32_FROM_LE(tmp);
  return r;
}


static int write_frame(FILELIST *fl,FILE *fpo,int avi_width,int avi_height)
{
  FILE *fp;
  int nbw,nbr;
  char buff[512];
  int pic_width,pic_height;

  fprintf(fpo,"00db");
  fwrite_quartet(fpo,fl->size);

  /* Now copy jpeg file into RIFF file */
  if (!((fp = fopen(fl->fn, "rb")))) return -6;
  get_jpegsize_opened(fp,&pic_width,&pic_height);        // catch size

  if ((pic_width!=avi_width) || (pic_height!=avi_height))
  {
printf("ERR: %d  %d  %d %d\n",pic_width,avi_width,pic_height,avi_height);
    return -99;
  }

  nbw = 0;
  if ((nbr = fread(buff,1,6,fp)) != 6) return -7;
  fwrite(buff, nbr, 1, fpo);
  nbw+=6;

  fread(buff,1, 4,fp);
  fwrite("AVI1", 4, 1, fpo);
  nbw+=4;

  while ((nbr = fread(buff, 1,512,fp)) > 0)
  {
    fwrite(buff, nbr, 1, fpo);
    nbw += nbr;
  }
  if (nbw < fl->size)
  {
    fwrite(buff, 1, fl->size-nbw, fpo);
  }
  fclose(fp);
  return 0;
}

static struct AVI_list_hdr init_avihdr(char *id,unsigned long size,char *type)
{
  struct AVI_list_hdr h;
  memcpy(h.id,id,4);
  h.sz=GUINT32_TO_LE(size);
  memcpy(h.type,type,4);
  return h;
}

static struct AVI_list_hdrl init_avihdrl(unsigned long jpg_sz,int frames,long us_per_frame,int width,int height)
{
  struct AVI_list_hdrl h;

  memset(&h,0,sizeof(h));
  /* struct AVI_list_hdr */
  h.list_hdr=init_avihdr("LIST",sizeof(struct AVI_list_hdrl) - 8,"hdrl");
  
  /* chunk avih */
  memcpy(h.avih_id,"avih",4);
  h.avih_sz=GUINT32_TO_LE(sizeof(struct AVI_avih));

  /* struct AVI_avih */
  h.avih.us_per_frame=GUINT32_TO_LE(us_per_frame);
  h.avih.max_bytes_per_sec=GUINT32_TO_LE(1000000 * (jpg_sz/frames) / us_per_frame);
  h.avih.flags=GUINT32_TO_LE(AVIF_HASINDEX);
  h.avih.tot_frames=GUINT32_TO_LE(frames);
  h.avih.streams=GUINT32_TO_LE(1);
  h.avih.width=GUINT32_TO_LE(width);
  h.avih.height=GUINT32_TO_LE(height);
     
  /* struct AVI_list_strl */
  h.strl.list_hdr=init_avihdr("LIST",GUINT32_TO_LE(sizeof(struct AVI_list_strl)-8),"strl");

  /* chunk strh */
  memcpy(h.strl.strh_id,"strh",4);
  h.strl.strh_sz=GUINT32_TO_LE(sizeof(struct AVI_strh)),

  /* struct AVI_strh */
  memcpy(h.strl.strh.type,"vids",4);
  memcpy(h.strl.strh.handler,"MJPG",4);
  h.strl.strh.scale=GUINT32_TO_LE(us_per_frame);
  h.strl.strh.rate=GUINT32_TO_LE(1000000);
  h.strl.strh.length=GUINT32_TO_LE(frames);
  
  /* chunk strf */
  memcpy(h.strl.strf_id,"strf",4);
  h.strl.strf_sz=sizeof(struct AVI_strf);

  /* struct AVI_strf */
  h.strl.strf.sz=GUINT32_TO_LE(sizeof(struct AVI_strf));
  h.strl.strf.width=GUINT32_TO_LE(width);
  h.strl.strf.height=GUINT32_TO_LE(height);
  h.strl.strf.planes_bit_cnt=GUINT32_TO_LE(1 + 24*256*256);
  memcpy(h.strl.strf.compression,"MJPG",4);
  h.strl.strf.image_sz=GUINT32_TO_LE(width * height * 3);

  /* struct AVI_list_odml */
  h.strl.list_odml.list_hdr=init_avihdr("LIST",16,"odml");
  memcpy(h.strl.list_odml.id,"dmlh",4);
  h.strl.list_odml.sz=GUINT32_TO_LE(4);
  h.strl.list_odml.frames=GUINT32_TO_LE(frames);
  return h;
}

static int read_hdr(FILE *fp,long *frames,long *jpg_sz,long *riff_sz,long *us_per_frame,int *width,int *height)
{
  struct AVI_list_hdrl hdrl;
  char byte4[4];
  unsigned long rif_size;

  fread(byte4,1,4,fp);    if (strncmp(byte4,"RIFF",4)) return -1; // "RIFF"
  fread(&rif_size,4,1,fp); *riff_sz=GUINT32_FROM_LE(rif_size);
  fread(byte4,1,4,fp);                                            // "AVI " 

  fread(&hdrl, sizeof(hdrl), 1, fp);
  *frames=GUINT32_FROM_LE(hdrl.avih.tot_frames);

  fread(byte4,1,4,fp);                                            // "LIST" 
  fread_quartet(fp,(unsigned long *)jpg_sz);
  *jpg_sz-=(8*(*frames) + 4);


  *us_per_frame=GUINT32_FROM_LE(hdrl.avih.us_per_frame);
  *width=GUINT32_FROM_LE(hdrl.avih.width);
  *height=GUINT32_FROM_LE(hdrl.avih.height);
  return 0;
}
  
static int read_indextbl(FILE *fp,long nrframes,FILELIST **fl)
{
  FILELIST *fl1;
  char byte4[4];
  int i;
  // start offset-table is at 16*nrframes from end
  fseek(fp,(-16)*nrframes,SEEK_END);

  for (i=0; i<nrframes; i++)
  {
    fl1=Create_Filelist(fl,NULL);
    fread(byte4,1,4,fp); if (strncmp(byte4,"00db",4)) return -1;
    fread_quartet(fp,NULL); // 18
    fread_quartet(fp,&fl1->offset);
    fread_quartet(fp,&fl1->size);
  }
  return 0;
}


static void write_hdr(long frames,long jpg_sz,long riff_sz,long us_per_frame,int width,int height,FILE *fpo)
{
  struct AVI_list_hdrl hdrl;
  
  hdrl=init_avihdrl(jpg_sz,frames,us_per_frame,width, height);

  /* printing AVI riff hdr */
  fprintf(fpo,"RIFF");
  fwrite_quartet(fpo,riff_sz);
  fprintf(fpo,"AVI ");

  fwrite(&hdrl, sizeof(hdrl), 1, fpo);
  /* list movi */
  fprintf(fpo,"LIST");
  fwrite_quartet(fpo,jpg_sz + 8*frames + 4);
  fprintf(fpo,"movi");
}

static void write_indextbl(FILELIST *fl,FILE *fpo)
{
  FILELIST *fl1;
  for (fl1=fl; fl1; fl1=fl1->next)
  {
    if (fl1->dont_use) continue;
    fprintf(fpo,"00db");
    fwrite_quartet(fpo,18);
    fwrite_quartet(fpo,fl1->offset);
    fwrite_quartet(fpo,fl1->size);
  }
}

static int check_sizes(FILELIST *fl,int width,int height)
{
  int err=0;
  int pic_width,pic_height;
  for (; fl; fl=fl->next)
  {
    get_jpegsize(fl->fn,&pic_width,&pic_height);        // catch size
    if ((pic_width!=width) || (pic_height!=height))
    {
      fl->dont_use=TRUE;
      err++;
    }
  }
  return err;
}

/*
 * AVI format:
 *   <avi_header>          write_hdr
 *   <frames>              write_frame
 *   <index_table>         write_indextbl
 */
int jpeg2avi(FILELIST *fl,char *mfno,long fps)
{
  FILELIST *fl1;
  unsigned long jpg_totsize,riff_size;
  int nrframes,nrframes_act;
  int width=10;
  int height=10;
  long uspfr;

  FILE *fpo;
  if (!fl) return 0;
  
  if (fps<=0) fps=1;
  
  uspfr=1000000/fps;
  get_jpegsize(fl->fn,&width,&height);        // catch size
  if (check_sizes(fl,width,height)) return Avi_picsize;

  if (!(fpo=fopen(mfno,"wb"))) return Avi_write;

  jpg_totsize = get_file_sz(fl);
  nrframes=nr_files(fl);
// rifsize = filesize-8 (=RIFF <riff_sz>)
// <AVI ><avi_hdr><movi hdr><db-hdr><frame1><db-hdr><frame2>.....<db-hdr><frameN><idx1-hdr><list1><list2>...<listN>
//   4             3x4       Nx8 +  jpg_totsize                                   2x4       4x4xN 
  riff_size = sizeof(struct AVI_list_hdrl) + 4 + 4 + jpg_totsize + 8*nrframes + 8 + 8 + 16*nrframes;
  
  write_hdr(nrframes, jpg_totsize,riff_size,uspfr, width, height,fpo);

  nrframes_act=0;
  for (fl1=fl; fl1; fl1=fl1->next)
  {
    if (fl1->dont_use) continue;
    if ((write_frame(fl1,fpo,width,height))==0) nrframes_act++;
  }

  /* indices */
  fprintf(fpo,"idx1");
  fwrite_quartet(fpo,16 * nrframes_act);
  write_indextbl(fl,fpo);
  
  fclose(fpo);
  if (nrframes_act!=nrframes)
  {
    return Avi_nrframes;
  }
  return nrframes;
}

int add_frame(char *fn_movie,char *fn_frame,long fps)
{
  FILE *fpo;
  FILELIST *fl=NULL,*flnw;
  int err=0;
  long nrframes,riff_size,jpg_totsize;
  int width, height;
  long uspfr;

  // try to open avi file
  if (!(fpo=fopen(fn_movie,"r+b")))       // avi file doesn't exist
  {
    Create_Filelist(&fl,fn_frame);        // create list of files (actuall, 1 file=first frame)
    return jpeg2avi(fl,fn_movie,fps);     // generate avi (contains 1 frame)
  }
  
  // read header avi file
  if (read_hdr(fpo,&nrframes,&jpg_totsize,&riff_size,&uspfr,&width,&height))
  {
    fclose(fpo); return 0;
  }

  // read index table; fl contains list of frames.
  if (read_indextbl(fpo,nrframes,&fl))
  {
    fclose(fpo); return 0;
  }

  flnw=Create_Filelist(&fl,fn_frame);                // Add new file to list
  get_1file_sz(flnw);                                // Determine size of new file to add

  fseek(fpo,-16*nrframes-8,SEEK_END);                // jump to end of last frame
  if ((write_frame(flnw,fpo,width,height))<0) err++; // add new frame to avi

  nrframes++;
  jpg_totsize+=flnw->size;

  fprintf(fpo,"idx1");                               // write start-of-index
  fwrite_quartet(fpo,16 * nrframes);                 // amount of frames (so incremented by 1)
  write_indextbl(fl,fpo);                            // write index

  // Now rewind and update size in header
  rewind(fpo);
  riff_size = sizeof(struct AVI_list_hdrl) + 4 + 4 + jpg_totsize + 8*nrframes + 8 + 8 + 16*nrframes;
  write_hdr(nrframes, jpg_totsize,riff_size,uspfr, width, height,fpo);

  fclose(fpo);
  return nrframes;
}
