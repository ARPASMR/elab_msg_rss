#include <stdlib.h>
#include "xrit2pic.h"
#include <string.h>
/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
extern GLOBINFO globinfo;

int dump_data(SEGMENT *segm,guint16 **str,guint16 *W,guint16 *H,guint16 *D)
{
  FILE *fpi;
  XRIT_HDR xrit_hdr1;
  unsigned char c[10];
  guint16 *p;
  int n=0;
  int width,height,depth,nrpixs;
  if (!(fpi=open_xritimage(segm->pfn,&xrit_hdr1,UNKDTYPE,segm->chan))) return Open_Rd;
  /* NOTE: Not all segm->pxh's are initialized, only first! */
  width=xrit_hdr1.nc;
  height=xrit_hdr1.nl;
  depth=xrit_hdr1.nb;
  nrpixs=width*height;
  if (nrpixs==0) return Open_Rd;
  *str=malloc(nrpixs*2);
  p=*str;

  segm->xh.coff=xrit_hdr1.coff;
  segm->xh.cfac=xrit_hdr1.cfac;
  segm->xh.loff=xrit_hdr1.loff;
  segm->xh.lfac=xrit_hdr1.lfac;
  tfreenull(&xrit_hdr1.img_obs_time);

  if (depth==10)
  {
    int b;

    /*
    01234567 01
    234567 0123
    4567 012345
    67 01234567
    */
    while ((fread(c,5,1,fpi))&&(n<nrpixs))
    {
      for (b=0; b<4; b++)
      {
        *p=((c[b]<<((b+1)*2))&0x3ff)+(c[b+1]>>((3-b)*2));
        p++;
        n++;
      }
    }
  }
  else if (depth==16) // 16 bpp, big endian 
  {
    while ((fread(c,2,1,fpi))&&(n<nrpixs)) { *p=(c[0]<<8)+c[1]; p++; n++; }
  }
  else
  {
    while ((fread(c,1,1,fpi))&&(n<nrpixs)) { *p=c[0]; p++; n++; }
  }
  fclose(fpi);
  *W=width;
  *H=height;
  *D=depth;
  return 0;
}


int gen_binitem(GtkWidget *window,GROUP *grp,GENMODES *modes,PREFER *prefer,
                    char *tfno)
{
  int err=0;
  if ((globinfo.nr_pics == 1) || (chan_selected(window,grp->chan->chan_name)))
  {
    if (modes->otype=='v')
    {
      Create_Message("Info","data preview not supported.");
    }
    else
    {
      err=gen_bin(window,grp,prefer,tfno,modes->test);
      if (modes->log_func) modes->log_func(modes,grp,tfno,err);
    }
  }
  return err;
}

int gen_bufritem(GtkWidget *window,GROUP *grp,GENMODES *modes,PREFER *prefer,
                    char *tfno)
{
  int err=0;
  if ((globinfo.nr_pics == 1) || (chan_selected(window,grp->chan->chan_name)))
  {
    if (modes->otype=='v')
    {
#ifndef __NOGUI__
      if (!strcmp(grp->sat_src,"ers2"))
      {
        int n;
        n=open_earthmap1(window,grp);
        if (n<0)
          Set_Entry(window,LAB_INFO,"Translator %s not found!?",prefer->prog_bufrextr); 
        else if (n==0)
          Set_Entry(window,LAB_INFO,"No content in bufr."); 
        else
          Set_Entry(window,LAB_INFO,"Extracted %d points.",n); 
      }
      else
        Create_Message("Info",bufrinfo(grp));
#endif
    }
    else
    {
      err=gen_bufr(window,grp,prefer,tfno,modes->test);
      if (modes->log_func) modes->log_func(modes,grp,tfno,err);
      
    }
  }
  return err;
}

#ifndef __NOGUI__
void add_asciimessage(FILE *fpi,GtkWidget *text)
{
  char l[1000];
  while (fgets(l+1,1000,fpi))
  {
    l[0]=' ';
    l[strlen(l)-2]='\n';
    l[strlen(l)-1]=0;
    Add_Text(text,120,l);
  }
}
#endif

int gen_admin_item(GtkWidget *window,GROUP *grp_sel,GENMODES *modes,PREFER *prefer,
                    char *tfno,gboolean *grp_ready)
{
#ifndef __NOGUI__
  GtkWidget *list_wnd=Find_Window(window,LAB_PROGRESS_TRANS);
#endif
  int err=0;

  /* Do now if just 1 item to be done or if this item is selected in list-window. */
  /* Use ID to determine selected item in list-window. */
  if ((globinfo.nr_pics == 1) || (chan_selected(window,grp_sel->chan->chan_name)))
  {
#ifndef __NOGUI__
    Set_Led(list_wnd,grp_sel->chan->id,0xff0); /* yellow */
    Set_Led(list_wnd,grp_sel->id,0xff0);       /* yellow */
#endif

    if (modes->otype=='v')
    {
#ifndef __NOGUI__
      preview_admin(grp_sel,add_asciimessage); /* Do it! */
#endif
      *grp_ready=FALSE;                        /* Remove_Grps done by preview! */
    }
    else
    {
      make_destdir(prefer->dest_dir,&grp_sel->pc,tfno);
      strcat(tfno,grp_sel->chan->segm->fn);
      err=copy_file(grp_sel->chan->segm->pfn,tfno);
      if (modes->log_func) modes->log_func(modes,grp_sel,tfno,0);
    }
    if (!err)
    {
#ifndef __NOGUI__
      Set_Led(list_wnd,grp_sel->chan->id,0x0f0); /* green*/
      Set_Led(list_wnd,grp_sel->id,0x0f0);       /* green*/
#endif
    }
  }
  else
  {
    *grp_ready=TRUE;
  }
  return err;
}

