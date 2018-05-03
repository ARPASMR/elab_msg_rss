/**************************************************
 * RCSId: $Id$
 *
 * GIF related routines
 * Project: WSAT
 * Author: R. Alblas
 *
 * BUG: Reading GIF-pictures with big smooth areas doesn't work OK
 *      Making #define DEBUG_R 1 will show where things go wrong.
 * History: 
 * $Log$
 **************************************************/
/*******************************************************************
 * Copyright (C) 2000 R. Alblas. 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 * 02111-1307, USA.
 ********************************************************************/
/********************************************************/
/* LZW routines are based on:				*/
/* Dr. Dobbs Journal --- Oct. 1989. 			*/
/* Article on LZW Data Compression by Mark R. Nelson 	*/
/********************************************************/

#include <stdio.h>
#include "gif.h"
#include <stdlib.h>
#include <string.h>

#define DEBUG_R 0
void get_next_block(unsigned char *byte_buf,short *navail_bytes, FILE *fp)
{
  *navail_bytes=fgetc(fp);
  if (feof(fp))
    return;
  else if (*navail_bytes)
  {
    if (!fread(byte_buf,*navail_bytes,1,fp)) return;
  }
  return;
}

short get_next_code(short curr_size, FILE *fp)
{
  unsigned long ret;
  static short navail_bytes = 0;            /* # bytes left in block */
  static short nbits_left = 0;              /* # bits left in current byte */
  static unsigned char b1;                  /* Current byte */
  static unsigned char byte_buff[257];      /* Current block */

  static unsigned char *pbytes;             /* Pointer to next byte in block */
  static long code_mask[13] =  {
    0,
    0x0001, 0x0003,
    0x0007, 0x000F,
    0x001F, 0x003F,
    0x007F, 0x00FF,
    0x01FF, 0x03FF,
    0x07FF, 0x0FFF
  };

  if (fp==NULL) { navail_bytes=0; nbits_left=0; return 0; }
  if (nbits_left == 0)
  {
    if (navail_bytes <= 0)
    {
      get_next_block(byte_buff,&navail_bytes,fp);
      pbytes = byte_buff;
    }
    b1 = *pbytes++;
    nbits_left = 8;
    --navail_bytes;
  }
  ret = b1 >> (8 - nbits_left);
  while (curr_size > nbits_left)
  {
    if (navail_bytes <= 0)
    {
      get_next_block(byte_buff,&navail_bytes,fp);
      pbytes = byte_buff;
    }
    b1 = *pbytes++;
    ret |= b1 << nbits_left;
    nbits_left += 8;
    --navail_bytes;
  }
  nbits_left -= curr_size;
  ret &= code_mask[curr_size];
  return((short)(ret));
}

int read_gif_close(GIF_PICINFO *decoder,FILE *fp)
{
  int linewidth=0;
  if (fp) linewidth=fgetc(fp);
  if (decoder->stack) free(decoder->stack);
  if (decoder->prefix) free(decoder->prefix);
  if (decoder->suffix) free(decoder->suffix);
  if (fp) if (feof(fp)) return 0;
  return linewidth;
}

int read_gif_palette(CLRMAP *clrmap,FILE *fp)
{
  int i;
  unsigned char rgb[3];

  for (i=0; i<clrmap->size; i++) 
  {
    if (!(fread(rgb,3,1,fp))) return 1;

    clrmap->red[i]=rgb[0];
    clrmap->green[i]=rgb[1];
    clrmap->blue[i]=rgb[2];
  }
  return 0;
}

int read_gif_gbl_hdr(GIF_HEADER *hdr,CLRMAP *clrmap,FILE *fp)
{
  int nrpl;
/* read screen header */
  if (!(fread(hdr,13,1,fp))) return 1; /* screen header */

  hdr->width=GUINT16_FROM_LE(hdr->width);
  hdr->height=GUINT16_FROM_LE(hdr->height);

  nrpl=(hdr->flags&0x7) + 1;
  clrmap->size=(1<<nrpl);
/* read global colormap */
  if (hdr->flags & GIF_GL_CLRMAP)
    read_gif_palette(clrmap,fp);

  return 0;
}


int read_gif_img_hdr(struct gifimghdr *img,CLRMAP *clrmap,FILE *fp)
{

/* lees image header */
  if (!(fread(img,sizeof_gifimghdr,1,fp))) return 1; /* image header */

  img->left=GUINT16_FROM_LE(img->left);
  img->top=GUINT16_FROM_LE(img->top);
  img->width=GUINT16_FROM_LE(img->width);
  img->height=GUINT16_FROM_LE(img->height);


/* read local colormap */

  if (img->flags & GIF_GL_CLRMAP)
  {
    int nrpl;
    nrpl=(img->flags&0x7) + 1;
    clrmap->size=(1<<nrpl);
    read_gif_palette(clrmap,fp);
  }
  return 0;
}

GIF_PICINFO init_dec_param(unsigned char code_size)
{
  GIF_PICINFO dec_param;
  dec_param.code_size = code_size + 1;
  dec_param.curr_size = code_size + 1;
  dec_param.top_slot = 1 << dec_param.curr_size;
  dec_param.clear = 1 << code_size;
  dec_param.ending = dec_param.clear + 1;
  dec_param.slot = dec_param.newcodes = dec_param.ending + 1;
  dec_param.navail_bytes = dec_param.nbits_left = 0;
  dec_param.depth=8;
  dec_param.startflag=1;
  return dec_param;
}


#define MAX_CODES 4095
/*************************************
 * Read GIF header into struct
 *************************************/
int read_gifhdr_file(FILE *fp, CLRMAP *clrmap, GIF_PICINFO *gif_pci)
{
  GIF_HEADER hdr;
  struct gifimghdr img;
  char code_size; 
  int b;
  get_next_code(0,NULL);

  if ((read_gif_gbl_hdr(&hdr,clrmap,fp))!=0) return 1;
  while ((b=fgetc(fp)) != EOF)
  {
    switch(b)
    {
      case ',':
        if ((read_gif_img_hdr(&img,clrmap,fp))!=0) return 2;
        if (!(fread(&code_size,1,1,fp))) return 3;     /* codesize */
        if ((code_size < 2) || (code_size > 9)) return 4;

        *gif_pci=init_dec_param(code_size);
        gif_pci->width=img.width;
        gif_pci->height=img.height;
        gif_pci->stack=(unsigned char *)malloc(MAX_CODES+1);
        gif_pci->suffix=(unsigned char *)malloc(MAX_CODES+1);
        gif_pci->prefix=(unsigned short *)calloc(MAX_CODES+1,2);
        gif_pci->oc=gif_pci->fc=0;
        gif_pci->code_size=code_size;
        return 0;
      case '!': break;
      case ';': return 5;
      default :
        printf("Unexpected char: %c  %d\n",b,b);
      return 6; 
    }
  }
  return 7;
}

/*************************************
 * Read GIF format header
   into GIF_PICINFO struct.
 * Return value:
 *   0 if OK
 *   !0 if error
 *************************************/
int read_gif_header(FILE *fp, CLRMAP *clrmap, GIF_PICINFO *gif_pci)
{
  int i;

  rewind(fp);

/* Read header */
  read_gif_close(gif_pci,NULL);  /* afsluiten lezen header */
  if ((i=read_gifhdr_file(fp,clrmap,gif_pci))) return i;
/*
  if ((i=read_gifhdr_file(pci))) return i;
*/

  return 0;
}

int get_gif_nextline(FILE *fp,GIF_PICINFO *dp,guchar *line)
{
  int bad_code_count=0;
  short c;
  short bufcnt=dp->width;
  guchar *bufptr=line;
  unsigned char *suffix=dp->suffix,*stack=dp->stack;
  unsigned short *prefix=dp->prefix;
  static short code;
  static unsigned char *sp,spvlag=0;
  int cnt=0;
  if (dp->startflag) { sp=dp->stack; dp->startflag=0; spvlag=1; }
  if (spvlag==2)
  {
    while (sp>stack)
    {
      *bufptr++=*(--sp);
      if (--bufcnt==0)  return bufcnt;
    }
    spvlag=1;
  }
#if DEBUG_R
  if (dp->ending != 257) { printf("Ending error: %d\n",dp->ending); return 0; }
#endif
  while ((c=get_next_code(dp->curr_size,fp)) != dp->ending)
  {
    if (c <0) return 0;
    if (c==dp->clear)
    {
      dp->curr_size=dp->code_size+1;
      dp->slot=dp->newcodes;
      dp->top_slot=1<<dp->curr_size;
      while ((c=get_next_code(dp->curr_size,fp)) == dp->clear);
      if (c==dp->ending) break;
      if (c >= dp->slot) c=0;
      dp->oc=dp->fc=c;
      *bufptr++=c;
      if (--bufcnt==0) return bufcnt;
    }
    else
    {
      code=c;
      if (code>=dp->slot)
      {
        if (code>dp->slot) ++bad_code_count;
        code=dp->oc;
        *sp++=dp->fc; /* printf("A:           %d\n",*(sp-1));*/
      }
      cnt=0;
      while (code>=dp->newcodes)
      {
        cnt++;
        if (cnt>8000)
        {
#if DEBUG_R
          printf("!!! Endless loop???\n");
{static long ccc;
if (ccc<cnt) { ccc=cnt; printf("%d\n",ccc);}
}
#endif
          break; 
        }
        *sp++=suffix[code]; /* printf("B:           %d\n",*(sp-1));*/
        if (code==prefix[code])
        {
#if DEBUG_R
          printf("!!! Endless loop: code=prefix[code]= %d\n",code);
#endif
          break;
        }
        code=prefix[code];
      }
      *sp++=code;  /* printf("C:           %d\n",*(sp-1)); */
      if (dp->slot < dp->top_slot)
      {
        suffix[dp->slot]=dp->fc=code;
        prefix[dp->slot++]=dp->oc;
        dp->oc=c;
      }
      if (dp->slot>=dp->top_slot)
        if (dp->curr_size<12)
        {
          dp->top_slot<<=1;
          ++dp->curr_size;
        }
      while (sp>stack)
      {
        *bufptr++=*(--sp);
        if (--bufcnt==0) { spvlag=2;  return bufcnt; }
      }
    }
  }
  return bufcnt;
}

/*************************************
 * Read line 'y' 
 * Return value:
 *   Amount of read bytes (=width picture)
 *************************************/
void read_gif_line(FILE *fp,GIF_PICINFO *gif_pci,int y,guchar *line)
{
  static int py;

  if (y<py) py=-1;
  while (y>py)
  {
    get_gif_nextline(fp,gif_pci,line);
    py++;
  }
  py=y;
}


void rewind_gif(FILE *fp, CLRMAP *clrmap, GIF_PICINFO *gif_pci)
{
  read_gif_header(fp, clrmap, gif_pci);
}

int gif2str(char *fn,guchar **str,int *w,int *h)
{
  FILE *fp;
  CLRMAP  clrmap;
  GIF_PICINFO gif_pci;
  guchar *line;
  int x,y;
  int ret;
  if (!((fp=fopen(fn,"rb")))) return 0;
  memset(&gif_pci,0,sizeof(gif_pci));
  ret=read_gif_header(fp, &clrmap, &gif_pci);

  *w=gif_pci.width;
  *h=gif_pci.height;

  line=malloc(gif_pci.width*2);
  *str=malloc((*w)*(*h)*3);
  for (y=0; y<(*h); y++)
  {
    get_gif_nextline(fp,&gif_pci,line);
    for (x=0; x<(*w); x++)
    {
      (*str)[0+(x+(*w)*y)*3]=clrmap.red[line[x]];
      (*str)[1+(x+(*w)*y)*3]=clrmap.green[line[x]];
      (*str)[2+(x+(*w)*y)*3]=clrmap.blue[line[x]];
    }
  }
  read_gif_close(&gif_pci,NULL);  /* afsluiten lezen header */
  fclose(fp);
  free(line);
  return 1;
}
