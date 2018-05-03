/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include "preview.h"
#include <stdlib.h>
#include "gdk/gdkkeysyms.h"
#include <math.h>
extern GLOBINFO globinfo;
extern PREFER prefer;
extern gboolean is_drawing;


/****************************************
 * px=x-coord. on screen (window-coord.)
 * Return: x-coord. in file, given zoom-factor and offset
 * px has range 0...rgbi->width (drawable width)
 ****************************************/
static int wx2px(int px,RGBPICINFO *rgbpi,RGBI *rgbi,PIC_CHARS *pc)
{
  int x,pw;
  int ox=rgbpi->ox; // *pc->o_width/rgbpi->pwidth;
  pw=rgbpi->pwidth;
//  pw=pc->o_width; // 9-12-2008 gewijzigd; rgbpi->pwidth=width pic,
                  //                      o_width houdt rekening met width bestemming
  if (pc->scan_dir!='n')
  {
    ox=pw*(1-1/rgbpi->zx)-ox;
    x=(rgbi->width-1-px)*pw/rgbi->width/rgbpi->zx+ox+pc->o_xoffset;
  }
  else
  {
    x=px*pw/rgbi->width/rgbpi->zx+ox+pc->o_xoffset;
  }
  return x;
}


/****************************************
 * py=y-coord. on screen (window-coord.)
 * Return: y-coord. in file, given zoom-factor and offset
 * py has range 0...rgbi->height (drawable height)
 ****************************************/
static int wy2py(int py,RGBPICINFO *rgbpi,RGBI *rgbi,PIC_CHARS *pc)
{
  int y;
  int oy=rgbpi->oy; // *pc->o_height/rgbpi->pheight;

  if (pc->scan_dir!='n')
  {
    oy=pc->o_height*(1-1/rgbpi->zy)-oy;
    y=(rgbi->height-1-py)*pc->o_height/rgbi->height/rgbpi->zy+oy+pc->o_yoffset;
  }
  else
  {
    y=py*pc->o_height/rgbi->height/rgbpi->zy+oy+pc->o_yoffset;
  }
  return y;
}

/****************************************
 * reverse function of wx2px
 ****************************************/
static int px2wx(int x,RGBPICINFO *rgbpi,RGBI *rgbi,PIC_CHARS *pc)
{
  int px;
  if (!pc)
  {
    px=(x-rgbpi->ox)*rgbi->width*rgbpi->zx/rgbpi->pwidth;
  }
  else
  {
    if (pc->scan_dir!='n')
    {
      int ox=rgbpi->pwidth*(1-1/rgbpi->zx)-rgbpi->ox;
      px=rgbi->width-1-(x-pc->o_xoffset-ox)*rgbi->width*rgbpi->zx/rgbpi->pwidth;
    }
    else
    {
      px=(x-pc->o_xoffset-rgbpi->ox)*rgbi->width*rgbpi->zx/rgbpi->pwidth;
    }
  }
  return px;
}

/****************************************
 * reverse function of wy2py
 ****************************************/
static int py2wy(int y,RGBPICINFO *rgbpi,RGBI *rgbi,PIC_CHARS *pc)
{
  int py;
  if (!pc)
  {
    py=(y-rgbpi->oy)*rgbi->height*rgbpi->zy/rgbpi->pheight;
  }
  else
  {
    if (pc->scan_dir!='n')
    {
      int oy;
      oy=pc->o_height*(1-1/rgbpi->zy)-rgbpi->oy;
      py=rgbi->height-1-(y-oy-pc->o_yoffset)*rgbi->height*rgbpi->zy/pc->o_height;
    }
    else
    {
      py=(y-rgbpi->oy-pc->o_yoffset)*rgbi->height*rgbpi->zy/pc->o_height;
    }
  }
  return py;
}

void polar_zoomxy(float fx,float fy,int *px,int *py,RGBPICINFO *rgbpi,RGBI *rgbi,PIC_CHARS *pc)
{
  *px=(fx-((float)(rgbpi->ox+pc->o_xoffset)*(float)rgbi->width/(float)rgbpi->pwidth))*rgbpi->zx;
  *py=(fy-((float)(rgbpi->oy+pc->o_yoffset)*(float)rgbi->height/(float)pc->o_height))*rgbpi->zy;
}



/***********************************************
 * Add markings: overlay, lon/lat, fire detection etc
 ***********************************************/
static void add_marking(DRAW_INFO *di,gboolean is_space,RGBPICINFO *rgbpi,RGBI *rgbi,guint16 *rgbl,
                        int x, GdkColor *clrdraw,PIC_CHARS *pc)
{
  int ovlrgb[3],llrgb[3];
  GdkColor clr=*clrdraw;
  ovlrgb[0]=((prefer.ovl_clr>>8)&0xf)*di->ol_lum/0xf;
  ovlrgb[1]=((prefer.ovl_clr>>4)&0xf)*di->ol_lum/0xf;
  ovlrgb[2]=((prefer.ovl_clr>>0)&0xf)*di->ol_lum/0xf;
  llrgb[0]=((prefer.lonlat_clr>>8)&0xf)*di->ol_lum/0xf;
  llrgb[1]=((prefer.lonlat_clr>>4)&0xf)*di->ol_lum/0xf;
  llrgb[2]=((prefer.lonlat_clr>>0)&0xf)*di->ol_lum/0xf;
  if (!rgbl) return;

  if ((di->add_overlay) && (globinfo.overlaytype!=coast_v))
  {
    int xx;
    int dx=rgbpi->pwidth/rgbi->width/rgbpi->zx;
    for (xx=x; xx<=x+dx; xx++)
    {
      if (di->anagl.ol_3d)
      {
        if ((rgbl) && (rgbl[xx]&(CMASK|CLMASK)))
        {
          if (rgbl[xx]&CLMASK)
          {
            clr.red  =MIN(clr.red  +di->ol_lum,0xff);
          }
          if (rgbl[xx]&CMASK)
          {
            clr.green=MIN(clr.green+di->ol_lum,0xff);
            clr.blue =MIN(clr.blue +di->ol_lum,0xff);
          }
          *clrdraw=clr;
          break;
        }
      }
      else
      {
        if ((rgbl) && (rgbl[xx] & (CMASK)))
        {
          clr.red  =MIN(clr.red  +ovlrgb[0],0xff);
          clr.green=MIN(clr.green+ovlrgb[1],0xff);
          clr.blue =MIN(clr.blue +ovlrgb[2],0xff);
          *clrdraw=clr;
          break;
        }
      }
    }
  }

  if ((pc->orbittype==POLAR) && (pc->gmap==normal))
  {
    if ((di->add_lonlat) && (!is_space))
    {
      clr.red  =MIN(clr.red  +llrgb[0],0xff);
      clr.green=MIN(clr.green+llrgb[1],0xff);
      clr.blue =MIN(clr.blue +llrgb[2],0xff);
      if (rgbl[x]&LMASK)
      {
        *clrdraw=clr;
      }
    }
  }

  if (di->fire_detect)
  {
    GdkColor clrf;
    clrf.red=0xff;
    clrf.green=0;
    clrf.blue=0;
    if (rgbl[x]&FMASK)
    {
      *clrdraw=clrf;
    }
  }

  if ((di->add_marks) && (!globinfo.marktype_vec))
  {
    GdkColor clrf;
    clrf.red=0xff;
    clrf.green=0;
    clrf.blue=0;
    if (rgbl[x]&MMASK)    // ??? x-upper_x_shift, maar dit zou al in gen_common.c 
    {                     // regel 509 moeten zijn afgehandeld!
      *clrdraw=clrf;
    }
  }
}



static void update_lumstat(RGBPICINFO *rgbpi,PIC_CHARS *pc,guint16 *pix,LUMINFO *li)
{
  guint32 *lumrgb[3];
  int lpos;
  lpos=pix[0]/li->lumscale; // *(NLUMSTAT-1)/rgbpi->lummax;
  lumrgb[0]=rgbpi->lumstatrgb[0];
  lumrgb[1]=rgbpi->lumstatrgb[1];
  lumrgb[2]=rgbpi->lumstatrgb[2];
  if (lpos<NLUMSTAT)
  {
    li->lumstat[lpos]++;
    lumrgb[0][lpos]++;
  }
  if (pc->is_color)
  {
    lpos=pix[1]/li->lumscale; // *(NLUMSTAT-1)/rgbpi->lummax;
    if (lpos<NLUMSTAT) li->lumstat[lpos]++;
    if (lpos<NLUMSTAT) lumrgb[1][lpos]++;

    lpos=pix[2]/li->lumscale; // *(NLUMSTAT-1)/rgbpi->lummax;
    if (lpos<NLUMSTAT) li->lumstat[lpos]++;
    if (lpos<NLUMSTAT) lumrgb[2][lpos]++;
//printf("%d  %d  %d\n",lpos,NLUMSTAT,lumrgb[2][lpos]);
  }
}


static gboolean prepare_pixel(guint16 *rgbl[4],int x,GdkColor *clr,
                              RGBPICINFO *rgbpi,PIC_CHARS *pc,gboolean is_irchan,LUMINFO *li,
                              float gamma,CHANNEL *chan,SEGMENT *pro)
{
  guint16 pix[3]={0,0,0};
  gboolean is_space;
  if (rgbl[0]) pix[0]=rgbl[0][x];            /* catch pixel */
  if (rgbl[1]) pix[1]=rgbl[1][x];            /* catch pixel */
  if (rgbl[2]) pix[2]=rgbl[2][x];            /* catch pixel */
  if ((pix[0]<=1) && (pix[1]<=1) && (pix[2]<=1))
    is_space=TRUE;
  else
    is_space=FALSE;

  /* update lumstat */
  update_lumstat(rgbpi,pc,pix,li);

  /* Either convert pix-val to temp and then map on colours or do lmin/lmax */
  if (((pc->map_temp)||(pc->map_temp_G_mA)) && (is_irchan))
  {
    if (pc->map_temp_G_mA)
    {
      pix[0]=temp2val(pix2temp(pro,chan,pix[0]));
      pix[1]=pix[2]=pix[0];
    }
    else
    {
      temp_clrmap(pix2temp(pro,chan,pix[0]),pix);
    }
  }
//  else
  {
  /* adapt lum to settings defined in lmin and lmax */
    set_lminmax(pix,li,rgbpi->lummax,0xff,rgbpi->lmaxrgb);

    if (gamma!=1.)
    {
      pix[0]=igamma(pix[0],0xff,gamma);
      pix[1]=igamma(pix[1],0xff,gamma);
      pix[2]=igamma(pix[2],0xff,gamma);
    }
  }
  clr->red  =MIN(pix[0],0xff);
  clr->green=MIN(pix[1],0xff);
  clr->blue =MIN(pix[2],0xff);

  if ((pc->invert) && (!is_space))
  {
    clr->red=0xff-clr->red;
    clr->green=0xff-clr->green;
    clr->blue=0xff-clr->blue;
  }

  return is_space;
}


/***********************************************
 * draw arrow on (x,y)
 ***********************************************/
void draw_rgbarrow(RGBI *rgbi,GdkColor *clr,int x,int y,int x2,int y2)
{
  int l=10;
  float dir=atan2((float)y-y2,(float)(x-x2));
  float r=sqrt((y2-y)*(y2-y) + (x2-x)*(x2-x));
  float n;

  l=r/2;
  for (n=-0.2; n<=0.2; n+=0.01)
  {
    draw_rgbline(rgbi,clr,x2,y2,x2+l*cos(dir+n),y2+l*sin(dir+n));
  }
}

void draw_scatarrow(RGBPICINFO *rgbpi,RGBI *rgbi,PIC_CHARS *pc,GdkColor *clr,int x,int y,int x2,int y2)
{
  if (rgbpi)
  {
    x=px2wx(x,rgbpi,rgbi,pc);      // the actual position
    y=py2wy(y,rgbpi,rgbi,pc);
    x2=px2wx(x2,rgbpi,rgbi,pc);    // end point arrow
    y2=py2wy(y2,rgbpi,rgbi,pc);
  }
  if ((ABS(x-x2))>200) return;       // ridiculous arrow size
  if ((ABS(y-y2))>200) return;
  draw_rgbline(rgbi,clr,x,y,x2,y2);
  draw_rgbarrow(rgbi,clr,x,y,x2,y2);
}


static void draw_scat1(GtkWidget *drawing_area,int xoff,int xfac,int yoff,int yfac,
                       RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *ipc,SCATPOINT *sp,int iclr)
{
  float lon2,lat2;
  int x,y,x2,y2;
  GdkColor clr;
  PIC_CHARS pc=*ipc;
  if (!sp) return;
//  Set upper_x_shift=lower_x_shift=0: already taken care of in draw.
  pc.gchar.upper_x_shift=0;
  pc.gchar.lower_x_shift=0;

  clr.red  =((iclr>>8)&0xf)<<4;
  clr.green=((iclr>>4)&0xf)<<4;
  clr.blue =((iclr>>0)&0xf)<<4;
  lon2=sp->lon-sp->speed/30.*sin(D2R(sp->sdir));  // sdir=90 -> to west
  lat2=sp->lat-sp->speed/30.*cos(D2R(sp->sdir));  // sdir= 0 -> to south

// For polar: not always full lon/lat range. For this compensated using xoff/xfac etc.
// geo: uses lonlat2dxy
// polar normal: sca not supported!
//  lonlat2dxy_area(sp->lon,sp->lat,xoff,yoff,xfac,yfac,&x,&y,&pc);
//  lonlat2dxy_area(lon2,lat2,xoff,yoff,xfac,yfac,&x2,&y2,&pc);
//printf("S: %d  %d  %d  %d  %d  %d  %d  %d\n",xoff,pc.xoff,yoff,pc.yoff,xfac,pc.xfac,yfac,pc.yfac);

// !! lonlat2dxy verandred; geen rekening met HRV-shift.
// Zou niet moeten, want direct naar drawable. Nog niet getest.
  lonlat2dxy(sp->lon,sp->lat,&x,&y,&pc);
  lonlat2dxy(lon2,lat2,&x2,&y2,&pc);

  if ((pc.orbittype!=POLAR) || (pc.gmap!=normal))
    draw_scatarrow(rgbpi,rgbi,&pc,&clr,x,y,x2,y2);
}


static void draw_scat(GtkWidget *drawing_area,int xoff,int xfac,int yoff,int yfac,
             RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,SCATPOINT *sp,int clr)
{
  for (; sp; sp=sp->next)
  {
    draw_scat1(drawing_area,xoff,xfac,yoff,yfac,rgbi,rgbpi,pc,sp,clr);
  }
  Refresh_Rect(drawing_area,0,0,rgbi->width,rgbi->height);    /* Make new row visible */
}

/***********************************************
 * Actual drawing function
 ***********************************************/
int draw_pic(GtkWidget *widget)
{
  GtkWidget *window=Find_Parent_Window(widget);
  DRAW_INFO *di=gtk_object_get_data((gpointer)window,"Draw_Info");
  RGBI *rgbi;
  GtkWidget *drawing_area;
  RGBPICINFO *rgbpi;
  GROUP *grp;
  PIC_CHARS *pc;
  SEGMENT *segm,*pro;
  LUMINFO li;
  int n,i;
  int px,py;
  int pxl,pyl;
  int xi,yi;
  int x,y;
  char scan_dir;
  guint32 *lumrgb[3];
  int pyp;
  gboolean is_space=FALSE;
  float gamma;
  gboolean is_irchan;
  gboolean do_geoearth_map;
  if (!widget) return 0;
  if (!window) return 0;
  if (!di) return 0;
  gamma=di->gamma;
  grp=(GROUP *)gtk_object_get_data((gpointer)window,GRP_DATA);
  if (!grp) return 0;
  pc=&grp->pc;
  pro=(grp->pro_epi? grp->pro_epi->pro : NULL);
  drawing_area=di->drawable;
  rgbpi=Get_RGBPI(drawing_area);
  rgbi=Get_RGBI(drawing_area);
  if (Find_Widget(widget,PMENU_PIXSIZE))
    rgbpi->do_shape=Get_Button(widget,PMENU_PIXSIZE);

  if (rgbpi->do_shape)
  {
    int w=drawing_area->allocation.width;
    int h=drawing_area->allocation.height;
    makeit_square(w,h,rgbpi);
  }

  li.lumstat=rgbpi->lumstat;
  lumrgb[0]=rgbpi->lumstatrgb[0];
  lumrgb[1]=rgbpi->lumstatrgb[1];
  lumrgb[2]=rgbpi->lumstatrgb[2];
  li.lmin=rgbpi->lmin;
  li.lmax=rgbpi->lmax;
  li.sep_rgblum=FALSE;
  if (li.lmin==li.lmax) { li.lmin=0; li.lmax=0x3ff; }
  for (i=0; i<3; i++)
  {
    if (rgbpi->lmaxrgb[i])  // Use individual lum-settings for RGB
    {
      li.sep_rgblum=TRUE;
      li.lminrgb[i]=rgbpi->lminrgb[i];
      li.lmaxrgb[i]=rgbpi->lmaxrgb[i];
    }
    else                    // Use common lum-settings
    {
      li.lminrgb[i]=li.lmin;
      li.lmaxrgb[i]=li.lmax;
    }

    if (li.lmaxrgb[i]==li.lminrgb[i])      // Make sure no division by zero!
    {
      if (li.lminrgb[i]) li.lminrgb[i]--; else li.lmaxrgb[i]++;
    }
  }
  li.lumscale=(rgbpi->lummax+1)/NLUMSTAT;
  if (li.lumscale==0) li.lumscale=1;

  scan_dir=pc->scan_dir; /* save scan dir (maybe changed by 'flip') */

//di->map_loaded niet meer gebruikt, 1e keer wordt al in prepare geladen.
  if ((pc->is_color) && (!Get_Button(widget,LAB_FCLR_PREV)) &&
      ((Find_Window(Find_Parent_Window(widget),LAB_COLOR)) || (Find_Window(Find_Parent_Window(widget),LAB_COLORSPEC))))
  {
    get_map(widget,grp->chan,LAB_COLOR,NULL);
    di->map_loaded=TRUE;
  }
  
  is_irchan=is_ir_group(grp); // in case single IR channel selected

  pc->scan_dir=scan_dir; /* restore scan dir */
  pc->bitshift=globinfo.bitshift;
  pc->invert=di->invert;
  //image_iformat: y -> metop-avhrr, z: NOAA-avhrr  Z: metop-EPS, GAC EPS 
//printf("%d  %c\n",pc->image_iformat,pc->image_iformat);
  if ((is_irchan) && (globinfo.spm.inv_ir))
  {
    if ((!pc->map_temp) && (!di->use_lut)) // no temp.
    {  // GEO and EPS: invert
      if ((pc->orbittype!=POLAR) ||  (pc->image_iformat=='Z'))
        pc->invert=(pc->invert? FALSE : TRUE);
    }
  }
  pc->avhrr_lin=di->avhrr_lin;
  pc->map_temp=di->map_temp;
  pc->map_temp_G_mA=di->map_temp_G_mA;
  pc->use_lut=di->use_lut;
  pc->gmap=di->gmap;
  di->drawing=TRUE;
  di->stop_drawing=FALSE;

  do_geoearth_map=((pc->orbittype==GEO) && (pc->gmap!=normal));
  if (do_geoearth_map)
  {
    grp->keep_in_mem=TRUE;
    Set_Button(widget,PMENU_FAST,TRUE);
  }

  {
    char tmp[300];

    sprintf(tmp,"%s  Z=[%.1f,%.1f]  N=[%d,%d]  D=[%.1f,%.1f]\n",
       pc->picname,
       rgbpi->zx,rgbpi->zy,
       (int)(rgbpi->pwidth/rgbpi->zx),(int)(rgbpi->pheight/rgbpi->zy),
       (float)rgbpi->pwidth/rgbpi->zx/rgbi->width,
       (float)rgbpi->pheight/rgbpi->zy/rgbi->height);
    gtk_window_set_title(GTK_WINDOW(window),tmp);
  }
  
// Determine shape, needed after changing projection
  {
    int w,h;
    float pixel_shape_prev=rgbpi->pixel_shape;
    w=drawing_area->allocation.width;
    h=drawing_area->allocation.height;
    if (pc->orbittype==GEO)
    {
      if ((pc->gmap==polar_n) || (pc->gmap==polar_s))
      {          // Fix for polar views; should be mapped on 2:1.
        rgbpi->pixel_shape=.5;
      }
      else
      {
        if ((pc->gchar.cfac) && (pc->gchar.lfac))
        {
          rgbpi->pixel_shape=(float)pc->gchar.cfac/(float)pc->gchar.lfac;
        }
        else
          rgbpi->pixel_shape=1.;
      }
    }
    else
    {
      rgbpi->pixel_shape=1.;
    }
    pc->pixel_shape=rgbpi->pixel_shape;
    if (rgbpi->pixel_shape!=pixel_shape_prev)
    {
/*
// zit ook in rgbpic.c, func RGB_Pic_keyfunc; hoe onderstaand te doen daar?
      RGBPICINFO rgbpi1=*rgbpi;
      rgbpi1.pwidth=pc->o_width;
      rgbpi1.pheight=pc->o_height;
printf("%d  %d\n",rgbpi->pwidth,rgbpi->pheight);
printf("%d  %d\n",rgbpi1.pwidth,rgbpi1.pheight);
*/
      makeit_square(w,h,rgbpi);
    }
    Set_Scrollbars(window,rgbpi);

    if (w>1) di->done=TRUE;      // first time drawing_area probably not initialized
  }

  if (grp->ovl_in_mem)
  {
    free(grp->ovl_in_mem);   // to allow change overlay
    grp->ovl_in_mem=NULL;
  }
  globinfo.overlaytype=change_ovltype(&prefer,globinfo.overlaytype);

/* Draw chunck by chunck */
  if (pc->scan_dir!='n') pyp=rgbi->height; else pyp=0;

  for (n=0; n<NLUMSTAT; n++) li.lumstat[n]=0;
  for (n=0; n<NLUMSTAT; n++) lumrgb[0][n]=lumrgb[1][n]=lumrgb[2][n]=0;
  Create_Cursor(window,GDK_WATCH);
  Set_Led(window,LAB_BUSY,0xf00);

// o_width=width pic; polar in lon/lat: 3600
// o_height=height pic; polar in lon/lat: 1800
// width=actual width pic; polar: 2048 (non-lin) or  4096 (linearized)
// org_width=original width; polar: 2048
// height=actual height pic; polar: # lines
  {
    for (pyl=0; pyl<rgbi->height; pyl+=1)   // all y's in drawable
    {
      guint16 *rgbl[4]={NULL,NULL,NULL,NULL};
      GdkColor clr;
      py=pyl;
      while (g_main_iteration(FALSE));
      if (di->stop_drawing)
      {
        return 1;  // Window destroyed, stop NOW!!!
      }
      if (rgbpi->redraw) break;

      yi=wy2py(py,rgbpi,rgbi,pc);
      y=yi;

      if (!do_geoearth_map)   // no random x/y access
      {                       // Get 1 composed line for all x-positions
        segm=store_composed_line(grp,y,rgbl,&di->anagl,di->fire_detect,di->use_lut);
      }
      for (pxl=0; pxl<rgbi->width; pxl+=1)  // pxl=pos. in window
      {
        int oor=0;
        px=pxl;
        xi=wx2px(px,rgbpi,rgbi,pc);         // translate to x-coord in file
        oor=dxy2fxy(xi,yi,&x,&y,&grp->pc);  // get (x,y) from file to plot on (xi,yi)

        if (do_geoearth_map)                // special mapping geostat ==>
        {                                   //   random x/y access needed
          segm=store_composed_line(grp,y,rgbl,&di->anagl,di->fire_detect,di->use_lut);
                                            // get and store or recall needed line
        }

        if ((oor) || (x<0) || (x>=pc->width))       /* out-of-range; plot 'black' */
        {
          clr.red=clr.green=clr.blue=0;
          draw_rgbpoint(rgbi,&clr,px,py);
          continue;
        }
        is_space=prepare_pixel(rgbl,x,&clr,rgbpi,pc,is_irchan,&li,gamma,grp->chan,pro);

        add_marking(di,is_space,rgbpi,rgbi,rgbl[3],x,&clr,pc);  // add overlay etc.

//printf("px=%d  width=%d  xi=%d  x=%d  r=%d  pcwidth=%d\n",px,rgbi->width,xi,x,clr.red,pc->width);
        draw_rgbpoint(rgbi,&clr,px,py);

      } // for (px=0; px<rgbi->width; px++)


      /* Fill rest-of-pic with black (if out-of-range occurred) */
      for (; px<rgbi->width; px++)
      {
        clr.red=clr.green=clr.blue=0;
        draw_rgbpoint(rgbi,&clr,px,py);
      }
      Refresh_Rect(drawing_area,0,py,rgbi->width,1);

  /*
      If rgbpicstr!=NULL then rgbl points somewhere into rgbpicstr.
      Otherwise rgbl is allocated; free it here.
  */
      for (i=0; i<4; i++)
      {
        if (!grp->rgbpicstr[i])
        {
          if (rgbl[i]) free(rgbl[i]);
          rgbl[i]=NULL;
        }
      }

      if (di->stop_drawing) break;
      if (CheckKeyPressed(GDK_Escape)) break;  /* stop if escape pressed */

    } // for (py=0; py<rgbi->height; py++)
//    if (py<rgbi->height-step) break;
  }

  if (prefer.low_mem)
    if ((pc->orbittype!=POLAR) || (pc->gmap==normal))
      free_chnks(grp);

  // Add overlay
  if (!rgbpi->redraw)
  {
    if (di->add_lonlat)
    {
      draw_lonlat(drawing_area,rgbi,rgbpi,pc,di->ol_lum);
    }

    if (di->add_overlay)
    {
      if (globinfo.overlaytype==coast_v)
      {
        OVERLAY *ovl=NULL;
        ovl=get_overlay(prefer.overlay,coast_v,NULL,NULL);
        draw_shore(drawing_area,rgbi,rgbpi,ovl,pc,di->ol_lum);
      }
      else
      {
        add_overlay(&prefer,0,NULL,NULL); // free mem
      }
    }

    if (di->add_marks)
    {
      if (globinfo.marktype_vec)
      {
        draw_landmark(drawing_area,rgbi,rgbpi,pc,di->ol_lum);
      }
    }
    if (di->add_cities)
    {
      if (globinfo.marktype_vec)
      {
        draw_waypoints(drawing_area,rgbi,rgbpi,pc,di->ol_lum);
      }
    }
  }

  if (!di->stop_drawing)
  {
    Create_Cursor(window,GDK_TOP_LEFT_ARROW);
    Set_Led(window,LAB_BUSY,0xeee);
  }  
  
  di->drawing=FALSE;
  is_drawing=FALSE;
  if (!rgbpi->redraw)
  {
    refresh_graph(window,rgbpi,pc);
    if (globinfo.scat_plot_all)
    {
      ASCATPOINT *asp;
      for (asp=globinfo.ascatp; asp; asp=asp->next)
        draw_scat(drawing_area,di->xoff,di->xfac,di->yoff,di->yfac,rgbi,rgbpi,pc,asp->sp,prefer.scat_clr);
    }
    draw_scat(drawing_area,di->xoff,di->xfac,di->yoff,di->yfac,rgbi,rgbpi,pc,globinfo.scatp,prefer.scat_selclr);
  }

  return di->stop_drawing;
}

void draw_line_in_prev(RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int xp,int yp,int x,int y,int limit,GdkColor *clr)
{
  int x1,y1,x2,y2;
  if (limit==0) limit=100;
  x1=px2wx(xp,rgbpi,rgbi,pc);      // the actual position
  y1=py2wy(yp,rgbpi,rgbi,pc);
  x2=px2wx(x,rgbpi,rgbi,pc);    // end point arrow
  y2=py2wy(y,rgbpi,rgbi,pc);
  if ((ABS(x-xp))<limit)
    draw_rgbline(rgbi,clr,x1,y1,x2,y2);
}

void draw_str_in_prev(RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int x,int y,char *str,GdkColor *clr)
{
  x=px2wx(x,rgbpi,rgbi,pc);    // end point arrow
  y=py2wy(y,rgbpi,rgbi,pc);
  draw_rgbstring(rgbi,clr,x,y,str);
}

