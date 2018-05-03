/*******************************************************************
 * Copyright (C) 2006 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************
 * Header file for non-GTK compile.
 * NOTE: Adapt Endian conversion, if needed!!!
 ********************************************************************/
/**********************************************************************
 * Endian conversions.
 * For Intel etc.: Set IS_LITTLE_ENDIAN to '1'.
 * For Motorola, SUN etc.: Set IS_LITTLE_ENDIAN to '0'.
 **********************************************************************/
#ifndef __ENDIAN__
#define __ENDIAN__ 'L'
#endif

#ifndef XR2P_NOHUIHDR
#define XR2P_NOHUIHDR
typedef enum {FALSE=0,TRUE=1} gboolean;
typedef short gint16;
typedef long gint32;
typedef unsigned long guint32;
typedef unsigned short guint16;
typedef unsigned int guint;
typedef unsigned char guchar;
typedef unsigned char guint8;
typedef char gchar;
typedef int gpointer;

typedef long long gint64;

typedef int GtkWidget;
typedef struct gdkc { int n; int red; int green; int blue; }  GdkColor;
typedef int GtkCTreeNode;
typedef int GtkCTree;
typedef int GtkCList;

#define UNIX_DIRSEP '/'
#define DOS_DIRSEP '\\'
#define UNIX_PATHSEP ':'
#define UNIX_PATHSEP_STR ":"
#define DOS_PATHSEP ';'
#define DOS_PATHSEP_STR ";"


#if __ENDIAN__ == 'B'
  #define GUINT16_TO_BE(a) (a)
  #define GUINT16_TO_LE(a) ((((a)&0x00ff)<<8)+(((a)&0xff00)>>8))

  #define GUINT32_TO_BE(a) (a)
  #define GUINT32_TO_LE(a) ((((a)&0x000000ff)<<24)+(((a)&0x0000ff00)<<8)+ \
                            (((a)&0x00ff0000)>> 8)+(((a)&0xff000000)>>24))

  #define GINT32_TO_BE(a) GUINT32_TO_BE(a)
  #define GINT32_TO_LE(a) GUINT32_TO_LE(a)


  #define GUINT64_TO_BE(a) (a)
  #define GUINT64_TO_LE(a) ((((a)&0x000000ff)<<56)+(((a)&0x0000ff00)<<40)+ \
                            (((a)&0x00ff0000)<<24)+(((a)&0xff000000)<<8) + \
                            (((a)&0x000000ff00000000)>>8) +(((a)&0x0000ff0000000000)>>24) + \
                            (((a)&0x00ff000000000000)>>40)+(((a)&0xff00000000000000)>>56))
#else
  #define GUINT16_TO_LE(a) (a)
  #define GUINT16_TO_BE(a) ((((a)&0x00ff)<<8)+(((a)&0xff00)>>8))

  #define GINT16_TO_BE(a)  ((((a)&0x00ff)<<8)+(((a)&0xff00)>>8))

  #define GUINT32_TO_LE(a) (a)
  #define GUINT32_TO_BE(a) ((((a)&0x000000ff)<<24)+(((a)&0x0000ff00)<<8)+ \
                            (((a)&0x00ff0000)>> 8)+(((a)&0xff000000)>>24))

  #define GINT32_TO_LE(a) GUINT32_TO_LE(a)
  #define GINT32_TO_BE(a) GUINT32_TO_BE(a)


  #define GUINT64_TO_LE(a) (a)
  #define GUINT64_TO_BE(a) ((((a)&0x000000ff)<<56)+(((a)&0x0000ff00)<<40)+ \
                            (((a)&0x00ff0000)<<24)+(((a)&0xff000000)<<8) + \
                            (((a)&0x000000ff00000000)>>8) +(((a)&0x0000ff0000000000)>>24) + \
                            (((a)&0x00ff000000000000)>>40)+(((a)&0xff00000000000000)>>56))

#endif

#define GUINT16_FROM_LE(a) GUINT16_TO_LE(a)
#define GUINT16_FROM_BE(a) GUINT16_TO_BE(a)
#define GINT16_FROM_BE(a)  GINT16_TO_BE(a)
#define GUINT32_FROM_LE(a) GUINT32_TO_LE(a)
#define GUINT32_FROM_BE(a) GUINT32_TO_BE(a)
#define GINT32_FROM_LE(a)  GINT32_TO_LE(a)
#define GINT32_FROM_BE(a)  GINT32_TO_BE(a)
#define GUINT64_FROM_LE(a) GUINT64_TO_LE(a)
#define GUINT64_FROM_BE(a) GUINT64_TO_BE(a)

/**********************************************************************/
#ifdef __GTK_WIN32__
  #if __GTK_WIN32__ == 0
    #define DIR_SEPARATOR UNIX_DIRSEP
    #define PATH_SEPARATOR UNIX_PATHSEP
    #define PATH_SEPARATOR_STR UNIX_PATHSEP_STR
  #else
    #define DIR_SEPARATOR DOS_DIRSEP
    #define PATH_SEPARATOR DOS_PATHSEP
    #define PATH_SEPARATOR_STR DOS_PATHSEP_STR
  #endif
#else
  #define DIR_SEPARATOR UNIX_DIRSEP
  #define PATH_SEPARATOR UNIX_PATHSEP
  #define PATH_SEPARATOR_STR UNIX_PATHSEP_STR
#endif
#define GDK_CLOCK 0
#define GDK_TOP_LEFT_ARROW 0
#define GDK_WATCH 0

#ifndef FLT_MAX
  #define FLT_MAX 9e99
#endif
#ifndef ABS
  #define ABS(a) (a)<0? (-1*(a)) : (a)
#endif

typedef struct
{
  char *buffer;
  int size;
} BUFFER;
BUFFER construct_dirpath(char *ipath);

float float32_to_le(float f);
int strcmpwild(char *s,char *v);
int remove_dircontent(char *dir,char *filter,gboolean doit);
int move_dircontent(char *dir,char *dirdest);
void get_strline(char *s,int y,char *l,int slen);
gboolean is_a_dir(char *path);
void construct_new_path(BUFFER *new_path,BUFFER *path,char *d_name);
char *g_get_current_dir();
char *g_get_home_dir();
char *get_path(char *iprogname);

/* Some gui functions here defined as 'nothing' */
#define Create_Progress( a, b, c) NULL
#define Update_Progress( a, b, c) 0
#define Close_Progress( a )
#define Get_Progress_state( a ) 0
#define Set_Entry( a, b,...) 
#define Create_Message(a,b,...)
#define Create_Cursor(a,b)
#define Set_Led(a,b,c)
#define Find_Window(a,b) NULL
#define Get_Optionsmenu(a,b) 0
#define Add_Chan_totree(a,b,c)
#define Add_Segm_totree(a,b,c,d)
#endif
