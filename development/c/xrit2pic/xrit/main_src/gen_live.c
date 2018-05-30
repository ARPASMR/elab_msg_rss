/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Preview functions
 ********************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include "xrit2pic.h"
extern GLOBINFO globinfo;
extern PREFER prefer;

/*******************************************************
 * Show 1 segment.
 * Uses global picwidth, picheight, picstr
 *******************************************************/
static void segm2pic(GtkWidget *window,GtkWidget *canvas,SEGMENT *segm,
                     RGBPICINFO *rgbpi,int bitshift,gboolean first)
{
/*
  GtkWidget *window=Find_Parent_Window(widget);
*/
  CHANNEL *chan=segm->chan;
  GROUP *grp=(chan? chan->group : NULL);
  guint16 *rgbstr=NULL;
  PIC_CHARS *pc=(grp? &grp->pc : NULL);
  char *picstr;
  int width,height,pos;
  int x,y,px,py,words_chunck;
  int pxp,pyp,nx,ny,nxmin,nxmax,nymin,nymax,x1,y1;
  int pos_picstr;
  int ovlrgb[3];
  int ovl;
//  int lower_east_col,upper_east_col,lower_nord_row;
  int scan_dir=segm->chan->scan_dir;
  int ol_lum=0xff;
  int xshift=0;
  static int lmin,lmax;
  static int lmin_p,lmax_p;
  static int nrsegs_plotted;
  gboolean range_lum=FALSE;
  if (!chan) return;
  if ((chan->group) && (chan->group->orbittype==POLAR)) range_lum=TRUE;
  // Get overlay colour
  ovlrgb[0]=((prefer.ovl_clr>>8)&0xf)*ol_lum/0xf;
  ovlrgb[1]=((prefer.ovl_clr>>4)&0xf)*ol_lum/0xf;
  ovlrgb[2]=((prefer.ovl_clr>>0)&0xf)*ol_lum/0xf;
  picstr=(char *)rgbpi->str8;
  width=rgbpi->pwidth;
  height=rgbpi->pheight;
  if (!height) return;
  if (!picstr) return;
  if ((grp) && (grp->orbittype==POLAR))
  { 
    pos=height-chan->segm->xh.nl;

  }
  else
  {
    pos=chan->nl*(segm->xh.segment-chan->seq_start);
  }


  pos=pos%height; // prevent overflow; for polar max. pos may be n*height!
  words_chunck=chan->nc*chan->nl;                     // # pixels in segment

  // translate segment; result in rgbstr
// Voor overlay vector is pc nodig!!! (M.n. pc.gchar.)
  if (pic2str(segm,&rgbstr,pc,TRUE))           // if transl. failed
  {
    #if __GTK_WIN32__==0
    sleep(1);                               // wait 1 sec (change this!)
    #endif
    if (pic2str(segm,&rgbstr,pc,TRUE)) return; // do 1 more attempt
  }
  if (globinfo.spm_live.compose) get_map(window,chan,LAB_COLORFIX,NULL);

  if ((range_lum) && (!nrsegs_plotted))
  {
    lmin=0xffff;
    lmax=0;
    for (y=0; y<chan->nl; y++)
    {
      for (x=0; x<chan->nc; x++)
      {
        int pix;
        int strp;
        strp=x+y*chan->nc;                 /* pointer to str having (x,y) */
        if (strp >= words_chunck) break;
        pix=rgbstr[strp];
        if ((is_ir_chan(chan)) && (globinfo.spm_live.inv_ir) && (!globinfo.spm_live.compose))
          if (pix>=0) pix=rgbpi->lummax-pix;
        if ((pix>=0) && (pix<=0xffff)) rgbpi->lumstat[pix]++;
      }
    }
    lumrange(rgbpi->lumstat,rgbpi->lummax,1,&lmin,&lmax);
  }
  nrsegs_plotted=(nrsegs_plotted+1)%6;
nrsegs_plotted=0;
  pyp=-1;

//printf("%d  %d  %d  %d\n",lmin_p,lmin,lmax_p,lmax);
  if ((!lmin_p) && (!lmax_p))
  {
    lmin_p=lmin;
    lmax_p=lmax;
  }
  else
  {
    if ((lmax<lmax_p-200) || (lmax>lmax_p+50)) lmax_p=lmax; else lmax=lmax_p;
    if ((lmin<lmin_p-100) || (lmin>lmin_p+50)) lmin_p=lmin; else lmin=lmin_p;
  }

  // shift old segments (polar)
  if (first)
  {
    if ((grp) && (grp->orbittype==POLAR))
    { 
      int h_segm=chan->segm->xh.nl;
      for (y=0; y<height-h_segm; y++)
      {
        for (x=0; x<width*3; x++)
        {
          if (scan_dir=='n')
            picstr[x + y*width*3]=picstr[x + (y+h_segm)*width*3];
          else
            picstr[x + (height-1-y)*width*3]=picstr[x + (height-1-y-h_segm)*width*3];
        }
      }

      for (; y<height; y++)
      {
        for (x=0; x<width*3; x++)
        {
          if (scan_dir=='n')
            picstr[x + y*width*3]=0;
          else
            picstr[x + (height-1-y)*width*3]=0;
        }
      }
    }
  }

  for (y=0; y<MIN(chan->nl,segm->height); y++)
  {
    y1=pos+y;
    if (scan_dir!='n')
      py=(height-1-y1);
    else
      py=y1;
    pxp=-1;

    if (y1>=chan->shift_ypos)        // zie prepare.c: >= ==> upper?
      xshift=chan->upper_x_shift;
    else
      xshift=chan->lower_x_shift;

    for (x=0; x<chan->nc; x++)
    {
      int pix;
      GdkColor clr;
      int strp,ovlp;

      x1=x+xshift;         /* pixel-x */

      if (scan_dir!='n') x1=width-1-x1;
      if (x1>=width) continue;
      if (x1<0) continue;

      px=x1;

      strp=x+y*chan->nc;                 /* pointer to str having (x,y) */
      if (strp >= words_chunck) break;

      pix=rgbstr[strp]; //  & (~OMASK);            /* catch pixel */

      ovlp=x+y*chan->nc;
//      if (!strcmp(chan->chan_name,"HRV"))
//        ovlp+=(5568-MAX(chan->upper_x_shift,chan->lower_x_shift));

      if ((globinfo.add_overlay) && (segm->ovlchnk) && (ovlp<words_chunck))
        ovl=segm->ovlchnk[ovlp] & CMASK;
      else
        ovl=0;
      if ((is_ir_chan(chan)) && (globinfo.spm_live.inv_ir) && (!globinfo.spm_live.compose))
        if (pix>0) pix=rgbpi->lummax-pix;
      pix=MAX(pix,0);
      if (range_lum)
      {
        if (lmax>lmin)
        {
          pix=(255*(pix-lmin))/(lmax-lmin);
        }
        if (pix<0) pix=0;
        if (pix>255) pix=255;
      }
      else
      {
        pix>>=bitshift;
      }
      if (pyp<0) pyp=py;
      if (pxp<0) pxp=px;
      nymin=MIN(py,pyp);
      nymax=MAX(py,pyp);
      nxmin=MIN(px,pxp);
      nxmax=MAX(px,pxp);

      for (ny=nymin; ny<=nymax; ny++) for (nx=nxmin; nx<=nxmax; nx++)
      {
        pos_picstr=nx + ny*width;
        if (pos_picstr<0) continue;

        if (globinfo.spm_live.compose)
        {
          /* Get previous pix value */
          clr.red  =picstr[pos_picstr*3+0];
          clr.green=picstr[pos_picstr*3+1];
          clr.blue =picstr[pos_picstr*3+2];

          /* Overwrite with current value */
          if (chan->r)
          {
            clr.red  =MIN(pix,0xff);
            if (chan->invert) clr.red=0xff-clr.red;
            if (ovl)          clr.red  =MIN(clr.red  +ovlrgb[0],0xff);
          }
          if (chan->g)
          {
            clr.green=MIN(pix,0xff);
            if (chan->invert) clr.green=0xff-clr.green;
            if (ovl)          clr.green=MIN(clr.green+ovlrgb[1],0xff);
          }
          if (chan->b)
          {
            clr.blue =MIN(pix,0xff);
            if (chan->invert) clr.blue=0xff-clr.blue;
            if (ovl)          clr.blue =MIN(clr.blue +ovlrgb[2],0xff);
          }
        }
        else
        {
          clr.red=clr.green=clr.blue=MIN(pix,0xff);
          if (ovl)
          {
            clr.red  =MIN(clr.red  +ovlrgb[0],0xff);
            clr.green=MIN(clr.green+ovlrgb[1],0xff);
            clr.blue =MIN(clr.blue +ovlrgb[2],0xff);
          }
        }

        /* Copy pix-value, now including overlay, back to window string */
        picstr[pos_picstr*3+0]=clr.red;
        picstr[pos_picstr*3+1]=clr.green;
        picstr[pos_picstr*3+2]=clr.blue;
      }
      pxp=px;
    }
    pyp=py;
  }

  // draw lines 0...picheight-1 in drawing in window.
  if (canvas)
  {
    GtkWidget *drawable=gtk_object_get_data(GTK_OBJECT(canvas),CHILD_ID);
    RGB_Pic_drawableupdate(drawable,0,height-1,FALSE);
  }
  else
  {
    GtkWidget *drawable=Find_Widget(window,"GTK_DRAWING_AREA");
    RGB_Pic_drawableupdate(drawable,0,height-1,FALSE);
  }
  if (segm->ovlchnk) free(segm->ovlchnk); segm->ovlchnk=NULL;
  if (rgbstr) free(rgbstr); rgbstr=NULL;
}


static GtkWidget *wnd;
static gboolean busy;
static gboolean close_req;
static char *picstr;
static void close_segmwnd(GtkWidget *widget)
{
  if (!busy)
  {
    Close_RGBPic(widget);
    wnd=NULL;
    picstr=NULL;  /* !!! picstr is free'd by Close_RGBPic!!! */
  }
  else
  {
    close_req=TRUE;
  }
}

// Show one segment.
// !! Use only once, because of statics!!
int Show_Segm(GtkWidget *widget,GtkWidget *canvas,SEGMENT *segm,int bitshift,gboolean first)
{
  CHANNEL *chan=segm->chan;
  GROUP *grp;
  RGBPICINFO *rgbpi=NULL;
  RGBI *rgbi=NULL;
  GtkWidget *drawable=NULL;
  static int picwidth;
  static int picheight;
  static int nrallocpix;
  int nrpix;
  if (!chan) return 0;
  grp=chan->group;
  if (grp) pic_info(grp,NULL,NULL);
  if (globinfo.spm_live.compose)
  {
    if ((!chan->r) && (!chan->g) && (!chan->b)) return 0;
  }
  busy=TRUE;

// NOODOPLOSSING: HRV verstoort live.
if (!strcmp(chan->chan_name,"HRV"))
{
//printf("SLAOVER: %s rgb: %f  %f  %f\n",chan->chan_name,chan->r,chan->g,chan->b);
return 0;
}

  // If proloque present, and segm is of HRV: Get lower/upper offset.
  chan->lower_x_shift=chan->upper_x_shift=chan->shift_ypos=0;
  chan->ncext=chan->nc;
  if (!strcmp(chan->chan_name,"HRV"))
  {
    chan->ncext=11136;
//    chan->ncext=chan->nc+(MAX(chan->lower_x_shift,chan->upper_x_shift));
    if ((chan->group) && (chan->group->pro_epi) && (chan->group->pro_epi->pro))
    {
      chan->lower_x_shift=segm->chan->group->pro_epi->pro->lower_east_col;
      chan->upper_x_shift=segm->chan->group->pro_epi->pro->upper_east_col;
      chan->shift_ypos=segm->chan->group->pro_epi->pro->lower_nord_row;
    }
  }
  picwidth=chan->ncext;
  if ((grp) && (grp->orbittype==POLAR))
    picheight=picwidth; 
  else
    picheight=chan->nl*(chan->seq_end-chan->seq_start+1);  // Total height full pic
  nrpix=picwidth*picheight;
  if (!nrpix) return 0;               // apparently not a picture

  // Create window if not done yet, and no canvas defined (just 1 live wnd!)
  if ((!wnd) && (!canvas))
  {
    wnd=Create_RGB_1bPic(widget,"Live",
                   prefer.lv_wndwidth,prefer.lv_wndheight,
                   picwidth,picheight,
                   NULL,          // loading rgbpi->str8 done later
                   TRUE,FALSE,
                   NULL,NULL,close_segmwnd);
    if (!wnd) return 0;
  }

  // get rgbpi
  if (wnd)
  {
    drawable=Find_Widget(wnd,"GTK_DRAWING_AREA");
    rgbpi=gtk_object_get_data(GTK_OBJECT(drawable),RGBPI_DL);
  }
  if (canvas)
  {
    drawable=gtk_object_get_data(GTK_OBJECT(canvas),CHILD_ID);
    rgbpi=gtk_object_get_data(GTK_OBJECT(drawable),RGBPI_DL);
  }
  if (!rgbpi) return 0;

  if ((segm->xh.cfac) && (segm->xh.lfac))
  {
    rgbpi->pixel_shape=(float)segm->xh.cfac/(float)segm->xh.lfac;
  }
  else
  {
    rgbpi->pixel_shape=1.;
  }
  rgbpi->lummax=(1<<chan->nb)-1;
  rgbpi->do_shape=TRUE;
  rgbi=Get_RGBI(drawable);
  if (rgbi) makeit_square(rgbi->width,rgbi->height,rgbpi);

//printf(">>> %x  %d  %d  %d  %d\n",picstr,nrpix,nrallocpix,picwidth,picheight);
  if ((!picstr) || (nrpix!=nrallocpix))
  {
    if (picstr) free(picstr); nrallocpix=0; 
    picstr=calloc(nrpix,3*sizeof(*picstr));
    if (!picstr) return 0;
    nrallocpix=nrpix;
    
    // Load new sizes and buffer 
    rgbpi->str8=(unsigned char *)picstr;
    rgbpi->pwidth=picwidth;
    rgbpi->pheight=picheight;
  }

  // Set info of current segment in window border.
  if (wnd)
  {
    char tmp[80],*chnm;
    strftime(tmp,40,"\"Live\" %Y-%m-%d  %H:%M ",&segm->xh.time);
    if ((!segm->chan) || (!(chnm=segm->chan->chan_name)))
      chnm="";
    sprintf(tmp,"%s (last segment done: %d of %s)",tmp,segm->xh.segment,chnm);
    gtk_window_set_title(GTK_WINDOW(wnd),tmp);
  }
  if (canvas)
  {
    char tmp[80],*chnm;
    GtkWidget *cwnd=Find_Parent_Window(canvas);

    strftime(tmp,40,"\"Live\" %Y-%m-%d  %H:%M ",&segm->xh.time);
    if ((!segm->chan) || (!(chnm=segm->chan->chan_name)))
      chnm="";
    sprintf(tmp,"%s (last segment done: %d of %s)",tmp,segm->xh.segment,chnm);
    if (cwnd) gtk_window_set_title(GTK_WINDOW(cwnd),tmp);
  }
  while (g_main_iteration(FALSE));

  // Draw 1 segment in window.
  segm2pic(wnd,canvas,segm,rgbpi,bitshift,first);
  busy=FALSE;

  if ((wnd) && (close_req))
  {
    close_segmwnd(wnd);
    close_req=FALSE;
    return 1;
  }
  return 0;
}

