/**************************************************
 * RCSId: $Id$
 *
 * Header for GIF related routines
 * Project: WSAT
 * Author: R. Alblas
 *
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
#ifndef GIFHEADER
#define GIFHEADER

#define GIFSIGN "GIF"

#include <stdio.h>
#ifdef __NOGUI__
  #include "xrit2pic_nogtk.h"
#else
  #include "gtk/gtk.h"
#endif

#define gs_error_interrupt (-6)
#define gs_error_invalidaccess (-7)
#define gs_error_invalidfileaccess (-9)
#define gs_error_invalidfont (-10)
#define gs_error_ioerror (-12)
#define gs_error_limitcheck (-13)
#define gs_error_nocurrentpoint (-14)
#define gs_error_rangecheck (-15)
#define gs_error_typecheck (-20)
#define gs_error_undefined (-21)
#define gs_error_undefinedresult (-23)
#define gs_error_VMerror (-25)
#define gs_error_Fatal (-100)

#define globalcolor_shift 7
#define colorres_shift 4
#define sort_shift 3
#define colorsize_shift 0

#define MAX_BITS 12            /* this is max for GIF. */
#define TABLE_SIZE 5123        /* this is max for 12-bit codes */
#define TABLE_HASH_SHIFT 2     /* size < 4095 + (4095 >> shift) */

#define sizeof_gifheader 13
#define GIF_GL_CLRMAP 0x80

#define MAX_PCICLR 256
typedef struct clrmap
{
  int size;
  int red[MAX_PCICLR],green[MAX_PCICLR],blue[MAX_PCICLR];
} CLRMAP;

typedef struct
{
  guchar    signature[3];       /* magic number == 'GIF' */
  guchar    version[3];         /* version # '87a' or '89a' */
  guint16 width;               /* screen width */
  guint16 height;              /* screen height */
  guchar   flags;
  guchar   background;          /* background color index */
  guchar   aspect;              /* pixel aspect ratio = (aspect + 15) / 64 */
} GIF_HEADER;



typedef struct image_descriptor_s
{
  guint16  left_pos;          /* image left pos (pixels) */
  guint16  top_pos;           /* image top  pos (pixels) */
  guint16  width;             /* image width    (pixels) */
  guint16  height;            /* image height   (pixels) */
  guchar  flags;
} image_descriptor;

/* State of LZW encoder */
typedef struct code_entry_s {
  int      code_value;
  guint16  prefix_code;
  guchar    append_character;
} code_entry;

#ifdef XXX
typedef struct lzw_encoder_s {
  int         bits;
  guint16     Max_Code;
  guint16     Clear_code;
  guint16     next_code;
  FILE        *file;
  code_entry  *table;
  guint16     string_code;
  guchar       output_bit_buffer;
  int         output_bit_count;  /* # of valid low-order bits (0 .. 7) in buffer */
  guint        byte_count;
  char        gif_buffer[260];
  int         width,height,depth;
} lzw_encoder;

#endif

#define sizeof_gifscrhdr 12
struct gifscrhdr
{
  char     sign[6];
  guint16  width,height;
  guchar    flags;
  guchar    backclr;
};

#define sizeof_gifimghdr 9
struct gifimghdr
{
  guint16  left,top;
  guint16  width,height;
  guchar    flags;
};


typedef struct dec_param
{
  guint16 width,height,depth;
  short  curr_size,clear,ending,newcodes,top_slot,slot;
  short  navail_bytes,nbits_left;
  int    code_size;
  guchar  *stack,*suffix;
  guint16 *prefix;
  int    oc,fc;
  int    startflag;

/* encoder */
  int        bits;
  guint16    Max_Code;
  guint16    Clear_code;
  guint16    next_code;
  code_entry *table;
  guint16    string_code;
  char       output_bit_buffer;	/* State of output buffer */
  int        output_bit_count;	/* # of valid low-order bits */
			        /* (between 0 and 7) in buffer */
  guint      byte_count;
  char       gif_buffer[260];
  CLRMAP clrmap;
} GIF_PICINFO;



int read_gif_header(FILE *fp, CLRMAP *clrmap, GIF_PICINFO *gif_pci);
void read_gif_line(FILE *fp,GIF_PICINFO *gif_pci,int y,guchar *line);
void rewind_gif(FILE *fp, CLRMAP *clrmap, GIF_PICINFO *gif_pci);
int gif2str(char *fn,guchar **str,int *w,int *h);

#endif

