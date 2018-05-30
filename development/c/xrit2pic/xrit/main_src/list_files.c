/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * File list functions
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include "vcdu.h"
#include "xrit2pic.h"

/*
#include "sgtk_functions.h"
*/
extern PREFER prefer;
extern GLOBINFO globinfo;

/* Destroy complete list. 
   !!! links cparent and tparent are explicitely NOT destroyed because they can 
   point to different list!!!
*/
void Destroy_List(LIST_FILES *lf,gboolean all)
{
  LIST_FILES *lf1,*lf1next;
  if (!lf) return;

  for (lf1=lf; lf1; lf1=lf1next)
  {
    lf1next=lf1->next;
    if (all)
    {
/* Next pointers may be copied to segm, so do NOT destroy them always! */
      if (lf1->fn)   free(lf1->fn);
      if (lf1->pfn)  free(lf1->pfn);
    }
    if (lf1->path) free(lf1->path);   /* path is only used on list_files */
    free(lf1);
  }
}


/* Remove and destroy 1 item from list */
void Destroy_Listitem(LIST_FILES *lf)
{
  if (lf->prev) lf->prev->next=lf->next;
  if (lf->next) lf->next->prev=lf->prev;
  if (lf->fn) free(lf->fn);
  if (lf->pfn) free(lf->pfn);
  if (lf->path) free(lf->path);
  free(lf);
}


/* Create a new list item and add to end of list */
LIST_FILES *Create_List(LIST_FILES *lf,char *fn)
{
  LIST_FILES *lfnw;

  lfnw=calloc(1,sizeof(LIST_FILES));
  if (fn)
  {
    lfnw->fn=malloc(strlen(fn)+10);
    strcpy(lfnw->fn,fn);
  }

  if (lf)
  {
    while (lf->next) lf=lf->next;
    lf->next=lfnw;
    lfnw->prev=lf;
  }

  return lfnw;
}


/*******************************************************
 * Update list of files:
 *   . detect xrit files and extract some properties
 *   . mark xrit files with ID-flag
 *   . move or remove unknown files if requested
 *   . detect done dir
 *   . remove all directories from list
 *******************************************************/ 
void update_filelist(LIST_FILES *lf,gboolean move_unk,int progress_func())
{
  int prgrscnt=0,prgrsmax=0;

  LIST_FILES *lft,*lftnext;
  XRIT_HDR xrit_hdr;
  if (!lf) return;
  
  memset(&xrit_hdr,0,sizeof(xrit_hdr));

  prgrsmax=0;
  for (lft=lf; lft; lft=lft->next) prgrsmax++;           /* Determine # files */
  globinfo.nrfiles+=prgrsmax;

  if (progress_func) 
    progress_func('s',"Loading...",0,prgrsmax);

/* Check all files; remove non-XRIT files from list. First lf has no info. */
  globinfo.donedir_exist=FALSE;
  for (lft=lf->next; lft; lft=lftnext)
  {
    lftnext=lft->next;
    if (!strcmp(lft->fn,prefer.done_dir)) globinfo.donedir_exist=TRUE;
    if (is_a_dir(lft->pfn))
    {
      Destroy_Listitem(lft);
      continue;
    }
    
    memset(&xrit_hdr,0,sizeof(xrit_hdr));

/* 
 * Note: If xrit-info from file-header: 
 *      Type (JPEG etc.), size, segment and amount of segments not determined here!
 */
    if (!fn2xrit(lft->fn,&xrit_hdr))                /* Determine xrit-info using file name */
    {                                               /* If not possible: */
      FILE *fpi; 
      if (!(fpi=open_xritimage(lft->pfn,&xrit_hdr,UNKDTYPE,NULL)))/* Open file to get info from actual header */
      {                                             /* If not possible: */
        if (is_a_dir(lft->pfn))                      /* slow, but not that many files here. */
        {
          lft->is_dir=TRUE;
        }
        else
        {
          lft->unknown=TRUE;                          /* Unknown file type */
          if ((globinfo.move_to_done) && (move_unk))  /* move unknown files to 'done' */
          {
            char *nwfn=NULL;
            strcpyd(&nwfn,globinfo.src_donedir);
            strcatd(&nwfn,lft->fn);
            if (!(rename(lft->pfn,nwfn)))
            {
              free(lft->pfn);    /* remove old name */
              lft->pfn=nwfn;     /* Add new name */
            }
            else
            {
              free(nwfn);
            }
          }
        }
        continue;                                   /* Skip rest */
      }
      tfreenull(&xrit_hdr.img_obs_time);
      fclose(fpi);
    }

/* If in AVHRR mode: Remove MSG from list */
    switch(xrit_hdr.xrit_frmt)
    {
      case EXRIT:
        if (prefer.avhrr_mode)
        {
          Destroy_Listitem(lft);
          continue;
        }
      break;
      default:
      break;
    }
    
/* Determine unique picture number. 
   For NOAA, time segments of 1 pic is not same, doesn't matter, 
   id is used after this is fixed (see update_avhrr)
   MSG:  yyy ddddddddd hh mm
   NOAA: yyy ddddddddd <id>
*/
    switch(xrit_hdr.xrit_frmt)
    {
      case EXRIT: case STDFRMT: case NHRPT: case METOP:
        xrit_hdr.pic_id=(xrit_hdr.time.tm_hour<<8)+xrit_hdr.time.tm_min;
      break;
      case NXRIT:
        xrit_hdr.pic_id=(xrit_hdr.time.tm_hour<<8)+xrit_hdr.time.tm_min;
      break;
      case BUFR:
        xrit_hdr.pic_id=(xrit_hdr.time.tm_hour<<8)+xrit_hdr.time.tm_min;
      break;
      case DWDSAT:
        if (xrit_hdr.hl=='g')
          xrit_hdr.pic_id=(xrit_hdr.time.tm_hour<<8);
        else
          xrit_hdr.pic_id=(xrit_hdr.time.tm_hour<<8)+xrit_hdr.time.tm_min;
      break;
      default: // UNKDTYPE
        xrit_hdr.pic_id=(xrit_hdr.time.tm_hour<<8)+xrit_hdr.time.tm_min;
      break;
    }

    xrit_hdr.pic_id+=((xrit_hdr.time.tm_yday<<16) + ((xrit_hdr.time.tm_year-100)<<25));
    memcpy(&lft->pxh,&xrit_hdr,sizeof(xrit_hdr));
    prgrscnt++;


    if (progress_func)
      if (!(progress_func('w',"Loading aborted.\nProbably all pictures are incomplete!",
                                                                       prgrscnt,prgrsmax)))
        break;  /* progress indication */

  }
  if (progress_func) progress_func('e',NULL,0,prgrsmax);
  return;
}

gboolean file_in_list(LIST_FILES *lf,char *f)
{
  for (; lf; lf=lf->next)
  {
    if (!lf->fn) continue;
    if (!strcmpwild(lf->fn,f)) return TRUE;
  }
  return FALSE;
}

/*******************************************************
 * create list with all files / dir at location pointed to by 'ipath'
 *******************************************************/
#define MAXFILELEN 100
LIST_FILES *create_filelist(char *ipath)
{
  int n=0;
  int nr_files=0;
  GtkWidget *progress;
  gboolean show_progress=FALSE;
  LIST_FILES *lf=NULL;
  DIR *directory;
  struct stat statbuf;
  BUFFER path,new_path;
  struct dirent *dirEntry;
  path.buffer=NULL;
  path.size=0;
  new_path.buffer=NULL;
  new_path.size=0;

/* Create first member of list and add path name to it */
  lf=Create_List(lf,NULL);
  path=construct_dirpath(ipath);
  strcpyd(&lf->path,path.buffer);
  if (RCHAR(path.buffer) == DIR_SEPARATOR) RCHAR(path.buffer)=0;

/* opendir(): For Windows path MUST end with '.' 
   (In construct_dirpath a bit extra space is allocated so easy strcat should do)
*/
  if (RCHAR(path.buffer) != '.')
  {
    finish_path(path.buffer);
    strcat(path.buffer,".");
  }

  directory = opendir(path.buffer);

  if (!directory) return NULL;

/* Create list; 1 item for each file in 'directory'. */
  if (show_progress)
    progress=Create_Progress(NULL,"Read dir. content...",TRUE);

  while ((dirEntry = readdir(directory)))
  {
    char txt[MAXFILELEN+2];

    construct_new_path(&new_path,&path,dirEntry->d_name);
    strncpy(txt,dirEntry->d_name,MAXFILELEN);

    /* ! Keep everything in list; e.g. presence done-dir is determined in update_filelist! */
    lf=Create_List(lf,dirEntry->d_name); /* filename */
    strcpyd(&lf->pfn,new_path.buffer);   /* path + filename */
    stat(lf->pfn,&statbuf);
    lf->time=statbuf.st_mtime;
    nr_files++;
    n++; if (n>=10000) n=0;

    if (show_progress)
      if (Update_Progress(progress,n,10000)) break;
  }
  closedir(directory);
  if (path.size) free(path.buffer);
  if (new_path.size) free(new_path.buffer);
  rewind_s(lf);
  if (show_progress) Close_Progress(progress);
  lf->nr_files=nr_files;
  return lf;
}


#include <stdlib.h>
gboolean Move_File(SEGMENT *segm,gboolean to_done)
{
  char *nwfn;
  if (!segm) return FALSE;
  if ((to_done) && (globinfo.donedir_tomake))
  {
    if (!make_dir(globinfo.src_donedir))
    {
      globinfo.donedir_tomake=FALSE;
    }
    else
    {
      return FALSE;
    }
  }

  if (to_done)
    strcpyd(&nwfn,globinfo.src_donedir);
  else
    strcpyd(&nwfn,globinfo.src_dir);

  strcatd(&nwfn,segm->fn);
  if (!(rename(segm->pfn,nwfn)))
  {
//printf("%s\n",segm->fn); /* log */
    free(segm->pfn);    /* remove old name */
    segm->pfn=nwfn;     /* Add new name */
    segm->where=(to_done? in_done : in_received);
    return TRUE;
  }
  else
  {
    free(nwfn);
    return FALSE;
  }
}

extern char *channellist[];
extern char *satlist[];
gboolean is_nomsg(char *sat)
{
  if (!strcmp(sat,"MTSAT1R"))  return TRUE;
  if (!strncmp(sat,"GOES",4))  return TRUE;
  if (!strncmp(sat,"MET",3))   return TRUE;
  if (!strncmp(sat,"NOAA",4))  return TRUE;
  if (!strncmp(sat,"METOP",5)) return TRUE;
  if (!strcmp(sat,"DWDSAT"))   return TRUE; 
  if (!strcmp(sat,"BUFR"))   return TRUE; 
  return FALSE;
}

gboolean is_mfg(char *sat)
{
  if (strncmp(sat,"MET",3)) return FALSE;
  if (!strncmp(sat,"METO",4)) return FALSE;
  return TRUE;
}

gboolean in_satchanlist(char *sat,char *chan,char pe,SATCHAN_LIST list)
{
  int i,j;
  if (!list.use_list) return TRUE;
  for (i=0; i<nrsatlist; i++)
  {
    if (!strcmp(sat,satlist[i]))                  // sat is in the 'keep' list
    {
      if (list.sat_sel[i])                        // selected -> Keep.
      {
        if (!strcmp(sat,"SERVICE"))  return TRUE; // services -> keep
        if (pe) return TRUE;                      // always keep pro/epi
        if (is_nomsg(sat)) return TRUE;           // don't look at sep. chans
        for (j=0; j<nrchanlist; j++)              // for remaining sats: examine channels
        {
          if (!strcmp(chan,channellist[j])) 
          {
            if (list.chan_sel[j]) return TRUE;    // chan selected to keep
            return FALSE;
          }
        }
      }
    }
  }
  return FALSE;
}


