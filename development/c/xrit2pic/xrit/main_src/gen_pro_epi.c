/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Extraction pro and epi files
 ********************************************************************/
#include "xrit2pic.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern GLOBINFO globinfo;

char *mk_proepi_fn(char *fni,char *ep,int n)
{
  char *fno,*p;

  fno=malloc(strlen(fni)+6);
  strcpy(fno,fni); if ((p=strrchr(fno,'.'))) *p=0;
  if (n)
    sprintf(fno,"%s.%s%d",fno,ep,n);
  else
    sprintf(fno,"%s.%s",fno,ep);
  return fno;
}

int gen_proepifile(SEGMENT *ep,char *fn)
{
  FILE *fpi,*fpo;
  int n;
  char chnk[130];
  if (!(fpo=fopen(fn,"wb"))) return Open_Wr;
  if (!(fpi=open_xritimage(ep->pfn,NULL,EXRIT,NULL))) return Open_Rd;
  while ((n=fread(chnk,1,128,fpi))) fwrite(chnk,1,n,fpo);

  fclose(fpo);
  fclose(fpi);

  return 0;
}

int gen_proepi1(SEGMENT *segm_pe,PIC_CHARS *pc,PREFER *prefer,char *tfno,GENMODES *modes)
{
  char *ext;
  if (segm_pe->is_pro)
    ext=".pro";
  else
    ext=".epi";

  make_filename(globinfo.dest_dir,pc,ext,tfno,modes,prefer->split_destdir);
  if (modes->test) return 0;

  if (overwrite(tfno,globinfo.overwrite_mode))
  {
    return gen_proepifile(segm_pe,tfno);
  }
  return Exist_fn;
}

int gen_proepi2(GtkWidget *window,GROUP *grp,PREFER *prefer,char *tfno,GENMODES *modes)
{
  int err=0;
  PIC_CHARS *pc=&grp->pc;
  if ((grp->pro_epi) && (grp->pro_epi->pro) && 
      ((grp->pro_epi->selected) || (grp->pro_epi->pro->selected)))
  {
    if ((err=gen_proepi1(grp->pro_epi->pro,pc,prefer,tfno,modes)))
    {
      char *p=strrchr(tfno,DIR_SEPARATOR);
      if (p) p++; else p=tfno;

      if (window) if (err==Exist_fn) Set_Entry(window,LAB_INFO,"Exist: %s.",p);
    }
  }
  if ((grp->pro_epi) && (grp->pro_epi->epi) &&
      ((grp->pro_epi->selected) || (grp->pro_epi->epi->selected)))
  {
    if ((err=gen_proepi1(grp->pro_epi->epi,pc,prefer,tfno,modes)))
    {
      char *p=strrchr(tfno,DIR_SEPARATOR);
      if (p) p++; else p=tfno;

      if (window) if (err==Exist_fn) Set_Entry(window,LAB_INFO,"Exist: %s.",p);
    }
  }
  return err;
}

#ifndef __NOGUI__
static SEGMENT *prosegm;
void proview(FILE *fpi,GtkWidget *text)
{
  char l[100];
  sprintf(l,"LongitudeOfSSP=%.1f\n",prosegm->ProjectionDescription.LongitudeOfSSP);
  Add_Text(text,120,l);
  sprintf(l,"HRV Shift ypos=%d\n",prosegm->lower_nord_row);
  Add_Text(text,120,l);
  sprintf(l,"HRV upper_east_col=%d\n",prosegm->upper_east_col);
  Add_Text(text,120,l);
  sprintf(l,"HRV lower_east_col=%d\n",prosegm->lower_east_col);
  Add_Text(text,120,l);
  if (prosegm->chan)
  {
    CHANNEL *chan=prosegm->chan;
    int i;
    for (i=0; i<12; i++)
    {
      sprintf(l,"channel %2d: Cal_Slope = %f  Cal_Offset = %f\n",i,chan->Cal_Slope[i],chan->Cal_Offset[i]);
      Add_Text(text,120,l);
    }
  }
}

void preview_proepi(SEGMENT *segm)
{
  if (!segm) return;
  prosegm=segm;
  open_textwnd(segm->pfn,"Prologue",500,200,proview);
}
#endif

int gen_proepi_item(GtkWidget *window,GROUP *grp,GENMODES *modes,PREFER *prefer,
                    char *tfno,gboolean *grp_ready)
{
  int err=0;

  if (modes->otype=='v')
  {
#ifndef __NOGUI__
    if (grp->pro_epi)
    { 
      if ((grp->pro_epi->pro) && ((grp->pro_epi->selected) || (grp->pro_epi->pro->selected)))
      {
        preview_proepi(grp->pro_epi->pro);
      }
      if ((grp->pro_epi->epi) && (grp->pro_epi->epi->selected))
      {
        Create_Message("Info","epilogue preview not supported.");
      }
    }
#endif
  }
  else
  {
    err=gen_proepi2(window,grp,prefer,tfno,modes);
    if (modes->log_func) modes->log_func(modes->fplog,grp,tfno,err,modes->test);
  }
  return err;
}

