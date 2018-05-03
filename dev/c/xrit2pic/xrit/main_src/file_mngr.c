/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/

#include "xrit2pic.h"
#include <string.h>

extern PREFER prefer;
extern GLOBINFO globinfo;


/* Determine if this chan is mentioned in 'sel_sat/sel_chan' 
   If sel_chan==NULL: don't look at chan name.
   If composed      : don't look at chan name.
*/ 
gboolean contributes_to_pic(CHANNEL *chan,char *sel_sat,char *sel_chan,gboolean essential)
{
  GROUP *grp;
  char satname[20];
  if (!chan) return FALSE;
  grp=chan->group;
  if (!grp) return FALSE;
  if (essential)
  {
    if (((grp->grp_tm.tm_hour%6)!=0) || (grp->grp_tm.tm_min!=0)) return FALSE;
  }

// Make MSG HRIT/LRIT sat names unique
  if (!strncmp(grp->sat_src,"MSG",3))
    sprintf(satname,"%c%s",grp->h_or_l,grp->sat_src);
  else if (grp->h_or_l=='G')
    sprintf(satname,"GAC_%s",grp->sat_src);
  else
    strcpy(satname,grp->sat_src);

  if (((!*sel_chan) ||                                  /* no channel defined SO OK */
       ((chan->chan_name) && (!strcmpwild(chan->chan_name,sel_chan))) ||  /* chan selected SO OK */
       (globinfo.spm_live.compose))                     /* colour enabled SO OK */
                  &&
      ((!*sel_sat) ||                                   /* no sat defined SO OK */
       (!strcasecmpwild(satname,sel_sat))))             /* or this sat selected SO OK */
  {
    return TRUE;
  }
  return FALSE;
}

/* Determine if avhrr-group is ready -> may be moved to done */
gboolean avhrr_grp_ready(GROUP *grp)
{
  GROUP *gn;
  int nrmin;
  if (!strchr("GAM",grp->h_or_l)) return TRUE;
  
  nrmin=1;
  if (grp->chan)
  {
    nrmin=grp->chan->nr_segm;    // minutes in grp
  }

  for (gn=grp->next; gn; gn=gn->next)
  {
    long dt;
    if (gn->h_or_l!=grp->h_or_l) continue;
    if (strcmp(gn->sat_src,grp->sat_src)) continue;
    dt=mktime_ntz(&gn->grp_tm)-mktime_ntz(&grp->grp_tm);
    
    if (dt>(nrmin+20)*60)      // Diff between prev and current segm is more than 20 minutes?
    {
      return TRUE;             // assume grp is complete
    }
  }
  return FALSE;
}

/******************************************************
 * Process just received segment.
 ******************************************************/
#define MAXSHOW_1RUN 20     /* Max. # segments shown in 1 update-run. */
int Process_Segm(GtkWidget *widget,GtkWidget *canvas,GROUP *grp,
                 gboolean ido_move,gboolean move_avhrr,
                 gboolean do_show)
{
  CHANNEL *chan;
  SEGMENT *segm,*segmnext;
  int n=0;
  int ns=0;
  gboolean do_move=ido_move;
  gboolean shown=FALSE;
  gboolean first=FALSE;
  for (; grp; grp=grp->next)
  {
    do_move=ido_move;
    if (!move_avhrr)   // don't move avhrr to done (needed for sorting during record)
    {
      if (!avhrr_grp_ready(grp)) do_move=FALSE; // continue;
    }

    for (chan=grp->pro_epi; chan; chan=chan->next)
    {
      for (segm=chan->pro; segm; segm=segm->next)
        if ((do_move) && (segm->where==in_received))
          if (Move_File(segm,TRUE)) n++;      // move global pro

      for (segm=chan->epi; segm; segm=segm->next)
        if ((do_move) && (segm->where==in_received))
          if (Move_File(segm,TRUE)) n++;      // move global epi
    }
  
    first=TRUE;
    for (chan=grp->chan; chan; chan=chan->next)
    {
      CHANMAP *cm;
      if ((cm=Get_Chanmap(globinfo.chanmap,chan->chan_name)))
      {
        chan->r=cm->r;
        chan->g=cm->g;
        chan->b=cm->b;
      }

      for (segm=chan->pro; segm; segm=segm->next)
        if ((do_move) && (segm->where==in_received))
          if (Move_File(segm,TRUE)) n++;      // move channel pro
      for (segm=chan->epi; segm; segm=segm->next)
        if ((do_move) && (segm->where==in_received))
          if (Move_File(segm,TRUE)) n++;      // move channel epi

      for (segm=chan->segm; segm; segm=segmnext)
      {
        gboolean where=segm->where;
        segmnext=segm->next;

        // ==> Move to 'done'
        if (segm->where==in_received)
        {
          Set_Entry(widget,LAB_INFO,"Received: %s-%d.",
                                 chan->chan_name,segm->xh.segment);
          if (do_move)
          {
/* If file to move is still being written Tellicast used to lock it until ready.
   New tellicast Linux (TLS00973) seems not to lock anymore!!! 
   Still, files are not corrupted if moved and written at the same time....???
   To do: Add test to check if file is ready (just in case)
*/
            if (Move_File(segm,TRUE))      // move segment
            {
              n++;
              Set_Entry(widget,LAB_INFO,"Received: %c %s-%d -> done",
                            *segm->fn,chan->chan_name,segm->xh.segment);
            }
            else
            {
              Set_Entry(widget,LAB_INFO,"Trying to move %s-%d...",
                              chan->chan_name,segm->xh.segment);
            }
          }
        }

        // ==> Show
        if ((do_show) && (!segm->shown))
        {
          if (!do_move) n++;
          if (contributes_to_pic(chan,globinfo.upd_sat,globinfo.upd_chan,FALSE))
          {
            int stop=0;
#ifndef __NOGUI__
            stop=Show_Segm(widget,canvas,segm,globinfo.bitshift,first);
            first=FALSE;
#endif
            ns++;
/* Free, unless needed for later generation (prevent double translation) */
            if (!globinfo.gen_new)
            {
              free_segmchnk(segm);
            }

            /* limit # of segments drawn per cycle (needed at start) */
            if (ns>MAXSHOW_1RUN) do_show=FALSE;
            /* stop showing if window was closed */
            if (stop) do_show=FALSE;
            shown=TRUE;
          }
        }
        segm->shown=TRUE;        // segm is done, whether really shown or not

        // ==> Do 'delete'
        if (where==in_received)
        {
/* Only reached if segment was in received dir: in_received=TRUE */
          if (globinfo.gen_new)
          {
            if (!grp->done) grp->new=TRUE; // Mark as new, needed for auto-generate
          }
          else if (globinfo.keep_chanlist.use_list)
          {
// Dit ipv filter_list in mkdbase zodat 'show' van weggooi-dingen mogelijk
//            Delete_Use_List(GROUP *igrp,GtkCTree *ctree,GtkWidget *widget)
          }
        }
      }
/* If no show-channel defined and no color (i.e. show all channels to update)
         -> Stop after displaying 1 channel per update */
      if ((!*globinfo.upd_chan) && (!globinfo.spm_live.compose) && (shown)) do_show=FALSE; 
    }
  }
  return n;
}

int count_files(GROUP *grp,int *ntop,int *ndone)
{
  CHANNEL *chan;
  SEGMENT *segm;
  *ntop=*ndone=0;

  for (; grp; grp=grp->next)
  {
    for (chan=grp->pro_epi; chan; chan=chan->next)
    {
      for (segm=chan->pro; segm; segm=segm->next)
      {
        if (segm->where==in_done) (*ndone)++; else (*ntop)++;
      }
      for (segm=chan->epi; segm; segm=segm->next)
      {
        if (segm->where==in_done) (*ndone)++; else (*ntop)++;
      }
    }

    for (chan=grp->chan; chan; chan=chan->next)
    {
      for (segm=chan->pro; segm; segm=segm->next)
      {
        if (segm->where==in_done) (*ndone)++; else (*ntop)++;
      }
      for (segm=chan->epi; segm; segm=segm->next)
      {
        if (segm->where==in_done) (*ndone)++; else (*ntop)++;
      }
      for (segm=chan->segm; segm; segm=segm->next)
      {
        if (segm->where==in_done) (*ndone)++; else (*ntop)++;
      }
    }
  }
  return (*ndone)+(*ntop);
}



void move_segmfiles(SEGMENT *segm,char *todo,gboolean selected)
{
  for (; segm; segm=segm->next)
  {
    if ((selected) && (segm->chan) && (!((segm->nselected) || (segm->chan->nselected)))) continue;
    if ((segm->where==in_done) && (!strcmp(todo,LAB_FROMDONE)))
    {
      Move_File(segm,FALSE);
    }
    if ((segm->where==in_received) && (!strcmp(todo,LAB_TODONE)))
    {
      Move_File(segm,TRUE);
    }
  }
}

#define FNSTRLEN 500   // !! Limited by Create_Message()!!!
char *fn_selected(GROUP *igrp)
{
  GROUP *grp;
  CHANNEL *chan;
  SEGMENT *segm;
  int n=0;
  static char fnstr[FNSTRLEN];
  *fnstr=0;
  
  for (grp=igrp; grp; grp=grp->next)
  {
    for (chan=grp->chan; chan; chan=chan->next)
    {
      for (segm=chan->segm; segm; segm=segm->next)
      {
        if ((grp->nselected) || (chan->nselected) || (segm->nselected))
        {
          if (segm->fn)
          {
            if (strlen(fnstr)+strlen(segm->fn)<FNSTRLEN-50)
              sprintf(fnstr,"%s%s\n",fnstr,segm->fn);
            else 
              n++;
          }
        }
      }
      if (grp->orbittype==POLAR) break; // all channels in 1 file
    }
  }
  if (n)
    sprintf(fnstr,"%s(%d more)",fnstr,n);
  return fnstr;
}


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
float size_files_Segm(SEGMENT *segm)
{
  struct stat stat1;
  stat(segm->pfn,&stat1);
  return (float)stat1.st_size;
}
float size_files_Chan(CHANNEL *chan)
{
  SEGMENT *segm;
  float fsize=0;
  for (segm=chan->segm; segm; segm=segm->next)
  {
    fsize+=size_files_Segm(segm);
  }
  for (segm=chan->pro; segm; segm=segm->next)
  {
    fsize+=size_files_Segm(segm);
  }
  for (segm=chan->epi; segm; segm=segm->next)
  {
    fsize+=size_files_Segm(segm);
  }
  return fsize;
}

float size_files_Grp(GROUP *grp)
{
  CHANNEL *chan;
  float fsize=0;
  for (chan=grp->pro_epi; chan; chan=chan->next)
  {
    fsize+=size_files_Chan(chan);
  }
  for (chan=grp->chan; chan; chan=chan->next)
  {
    fsize+=size_files_Chan(chan);
  }
  return fsize;
}

float show_info_sel(GROUP *grp)
{
  float fsize=0;
  CHANNEL *chan;
  SEGMENT *segm;
  for (; grp; grp=grp->next)
  {
    if (grp->nselected)
    {
      fsize+=size_files_Grp(grp);
    }
    else
    {
      for (chan=grp->pro_epi; chan; chan=chan->next)
      {
        if (chan->nselected)
        {
          fsize+=size_files_Chan(chan);
        }
        else
        {
          for (segm=chan->pro; segm; segm=segm->next)
          {
            if (segm->nselected)
            {
              fsize+=size_files_Segm(segm);
            }
          }
          for (segm=chan->epi; segm; segm=segm->next)
          {
            if (segm->nselected)
            {
              fsize+=size_files_Segm(segm);
            }
          }
        }
      }
      for (chan=grp->chan; chan; chan=chan->next)
      {
        if (chan->nselected)
        {
          fsize+=size_files_Chan(chan);
        }
        else
        {
          for (segm=chan->segm; segm; segm=segm->next)
          {
            if (segm->nselected)
            {
              fsize+=size_files_Segm(segm);
            }
          }
          for (segm=chan->pro; segm; segm=segm->next)
          {
            if (segm->nselected)
            {
              fsize+=size_files_Segm(segm);
            }
          }
          for (segm=chan->epi; segm; segm=segm->next)
          {
            if (segm->nselected)
            {
              fsize+=size_files_Segm(segm);
            }
          }
        }
      }
    }
  }
  return fsize;
}

/* Delete old files. Only if in received or done. */
int delete_old(GROUP **igrp,GtkCTree *ctree,int *days_old,int days_max,gboolean round_days)
{
  time_t now=time(NULL);
  gint32 inow;
  struct tm nowtm;
  struct tm grptm;
  double dt;
  int dtd,dth;
  int nrdel=0;
  GROUP *grp;
  GROUP *grpnext;
  CHANNEL *chan;
  SEGMENT *segm;
  if (now<0) return 0;
  nowtm=*gmtime(&now);
  if (round_days) nowtm.tm_hour=nowtm.tm_min=nowtm.tm_sec=0;
    
  inow=mktime_ntz(&nowtm);
  if ((!days_old[0]) && (!days_old[1])) return 0;
  for (grp=*igrp; grp; grp=grpnext)
  {
    grpnext=grp->next;
    segm=NULL;
    for (chan=grp->chan; chan; chan=chan->next)
    {
      if ((segm=chan->segm)) break;
    }

    if (!segm)
    {
      if ((chan=grp->pro_epi))
      {
        if (!(segm=chan->pro))
        {
          if (!(segm=chan->epi))
          {
            continue;
          }
        }
      }
      else
      {
        continue;
      }
    }

    if ((segm->where!=in_received) && (segm->where!=in_done)) continue;
    grptm=grp->grp_tm;
    if (round_days) grptm.tm_hour=grptm.tm_sec=grptm.tm_min=0;

    if (grptm.tm_year<80)     // no valid time defined -> use file timestamp
      dt=difftime(now,segm->time);
    else                            // use sat-time
      dt=inow-mktime_ntz(&grptm);

    dt/=3600.;       // total hours
    dtd=dt/24;       // days
    dth=dt-(dtd*24); // hours
    if (((dtd>days_old[0]) || ((dtd==days_old[0]) && (dth>=days_old[1]))) &&
        ((!days_max) || (dtd<=days_max)))
    {
      if (ctree)
      {
        if (grp==*igrp) *igrp=grp->next; // change first grp if deleted
        nrdel=Delete_Grp(grp,ctree);  // delete and count # files deleted
      }
      else
      {
        nrdel++;                      // count # groups to delete
      }
    }
  }
  return nrdel;
}

#ifndef __NOGUI__
int delete_selected(GROUP **igrp,GtkCTree *ctree)
{
  int nrdel=0;
  GROUP *grp;
  GROUP *grpnext;
  CHANNEL *chan,*channext;
  SEGMENT *segm,*segmnext;
  for (grp=*igrp; grp; grp=grpnext)
  {
    grpnext=grp->next;
    if (grp->nselected)
    {
      if ((ctree) && (grp==*igrp)) *igrp=grp->next; // change first grp if deleted
      if ((ctree) && (grp->h_or_l=='A'))
      {
        char *fn;
        if ((fn=mk_noradname(grp)))
        {
          char pfn[300];
          sprintf(pfn,"%s%s",globinfo.src_dir,fn);
          if (remove(pfn))
          {
            sprintf(pfn,"%s%s",globinfo.src_donedir,fn);
            remove(pfn);
          }
        }
      }
      nrdel+=Delete_Grp(grp,ctree);

    }
    else
    {
      for (chan=grp->pro_epi; chan; chan=channext)
      {
        channext=chan->next;
        if (chan->nselected)
        {
          nrdel+=Delete_Chan(chan,ctree);
        }
        else
        {
          for (segm=chan->pro; segm; segm=segmnext)
          {
            segmnext=segm->next;
            if (segm->nselected)
            {
              nrdel+=Delete_Segm(segm,ctree);
            }
          }
          for (segm=chan->epi; segm; segm=segmnext)
          {
            segmnext=segm->next;
            if (segm->nselected)
            {
              nrdel+=Delete_Segm(segm,ctree);
            }
          }
        }
      }
      for (chan=grp->chan; chan; chan=channext)
      {
        channext=chan->next;
        if (chan->nselected)
        {
          if ((grp->h_or_l=='A') || (grp->h_or_l=='M'))
          {
            if (ctree) Create_Message("Note:","Select group to delete AVHRR!");
            break;
          }
          nrdel+=Delete_Chan(chan,ctree);
        }
        else
        {
          for (segm=chan->segm; segm; segm=segmnext)
          {
            segmnext=segm->next;
            if (segm->nselected)
            {
              if ((grp->h_or_l=='A') || (grp->h_or_l=='M'))
              {
                if (ctree) Create_Message("Note:","Select group to delete AVHRR!");
                break;
              }
              nrdel+=Delete_Segm(segm,ctree);
            }
          }
          for (segm=chan->pro; segm; segm=segmnext)
          {
            segmnext=segm->next;
            if (segm->nselected)
            {
              nrdel+=Delete_Segm(segm,ctree);
            }
          }
          for (segm=chan->epi; segm; segm=segmnext)
          {
            segmnext=segm->next;
            if (segm->nselected)
            {
              nrdel+=Delete_Segm(segm,ctree);
            }
          }
        }
      }
    }
  }
  return nrdel;
}

static char *apath;
static int res_arch;
static int mv_filesarch(GROUP *grp,CHANNEL *chan,SEGMENT *segm)
{
  res_arch=mv_files_to(grp,chan,segm,apath);
  return res_arch;
}

int archive_selected(GROUP **grp,GtkCTree *ctree,char *path)
{
  apath=path;
  if (!*apath) apath=NULL;
  forall_selgrp(*grp,NULL,NULL,mv_filesarch,TRUE);
  return res_arch;
}

#endif
