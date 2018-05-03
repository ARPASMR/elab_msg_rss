/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include "gtk/gtk.h"
#include "gdk/gdkkeysyms.h"

extern GLOBINFO globinfo;
extern PREFER prefer;

#define LAB_PROGTEXT "Program info"
static void func(GtkWidget *w)
{
  int size=120;
  Add_Text(w,size,"\n---------------------------------------------\n");
  Add_Text(w,size," Search bzip2 for AVHRR:\n");

  #if __GTK_WIN32__ == 1
  if (!exist_prog(globinfo.prog_bzip2," -h -q"))
  #else
  if (!exist_prog(globinfo.prog_bzip2," -h -q 2> /dev/null"))
  #endif
    Add_Text(w,size,"   WARNING: %s not found!\n   Needed for AVHRR.\n",BZIPPROG);
  else
    Add_Text(w,size,"   %s found.\n",BZIPPROG);
    Add_Text(w,size,"   Full program name: %s.\n",globinfo.prog_bzip2);

#ifdef XXX
  int res;
  Add_Text(w,size,"\n---------------------------------------------\n");

// Kepler file
  Add_Text(w,size," Search Kepler file '%s' in:\n",prefer.noradfile);
  Add_Text(w,size,"    (1)  %s\n",prefer.cur_dir);
  Add_Text(w,size,"    (2)  %s\n",prefer.home_dir);
  Add_Text(w,size,"    (3)  %s\n\n",prefer.prog_dir);
  res=search_file(prefer.noradfile,NULL,prefer.cur_dir,prefer.home_dir,prefer.prog_dir);

  switch(res)
  {
    case 1:  Add_Text(w,size,"   Used '%s' in '%s'\n",prefer.noradfile,prefer.cur_dir); break;
    case 2:  Add_Text(w,size,"   Used '%s' in '%s'\n",prefer.noradfile,prefer.home_dir); break;
    case 3:  Add_Text(w,size,"   Used '%s' in '%s'\n",prefer.noradfile,prefer.prog_dir); break;
    default: Add_Text(w,size,"   Not found.\n"); break;
  }
#endif
}

GtkWidget *nb_proginfo(char *releasenr)
{
  char cpr;
  int size=120;
  GtkWidget *w[4];
  w[1]=Create_Text("x",FALSE,NULL);
  w[0]=Pack(NULL,'v',w[1],1,NULL);

#if __GTK_WIN32__ == 1
  cpr='c';
#else
  cpr=GDK_copyright;  /* gaat fout t.g.v. font? */
  cpr='c';
#endif
  Clear_Text(w[1]);
  Add_Text(w[1],size," xrit2pic version %s\n\n",releasenr);
  Add_Text(w[1],size," %c R. Alblas\n",cpr);
  Add_Text(w[1],size," werkgroep Kunstmanen\n Nederland\n\n");
  Add_Text(w[1],size," Written in C using GTK libs\n");
  Add_Text(w[1],size," Compiled with gcc\n\n");
  Add_Text(w[1],size," JPEG decompression:\n");
  Add_Text(w[1],size," IJG version 6b, adapted to 8/12 bits\n\n");
  Add_Text(w[1],size," Wavelet decompression:\n");
  Add_Text(w[1],size," %c 2003 Eumetsat\n",cpr);
  Add_Text(w[1],size," Compiled with g++\n\n");
  Add_Text(w[1],size," Summary:\n");
  Add_Text(w[1],size," Translation of MSG LRIT/HRIT files\n");
  Add_Text(w[1],size," into JPEG, PGM and AVI formats.\n\n");
  func(w[1]);
  return w[0];
}
