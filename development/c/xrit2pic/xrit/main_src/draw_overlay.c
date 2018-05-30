#include <stdio.h>
#include <malloc.h>

#include "xrit2pic.h"
#include "overlay.h"
extern PREFER prefer;
#define DLON 10
#define DLAT 10

/****************************************
 ****************************************
 * Draw overlays: in chunk:          in drawable:
 * -------------------------------------------------
 *  shores etc. : add_shore()        draw_shore()
 *  mark        : add_landmark()     draw_landmark()
 *  cities      : add_waypoints()    draw_waypoints()
 *  lon/lat     : geo_add_lonlat()   draw_lonlat()
 *
 ****************************************
 ****************************************/
// (xp,yp) - (x,y) to segm ovlchnk using mask
static void draw_line_in_ovlchnk(SEGMENT *segm,int xp,int yp,int x,int y,char mask)
{
  int chnkpos;
  int xmin,xmax,dx,ymin,ymax,dy;
//printf("%d  %d  %d  %d\n",xp,yp,x,y);
  if ((ABS(xp-x)) > (ABS(yp-y)))
  {
    if ((xp!=-1) && (x!=xp))
    {
      if (x<xp)
      {
        xmin=x; xmax=xp;
        ymin=y; ymax=yp;
      }
      else
      {
        xmin=xp; xmax=x;
        ymin=yp; ymax=y;
      }
      for (dx=xmin; dx<=xmax; dx++)
      {
        dy=(dx-xmin)*(ymax-ymin)/(xmax-xmin)+ymin;
        chnkpos=dx+dy*segm->width;
        if ((chnkpos>=0) && (chnkpos<segm->width*segm->height))
        {      
          segm->ovlchnk[chnkpos] |=mask;
        }
      }
    }
  }
  else
  {
    if ((yp!=-1) && (y!=yp))
    {
      if (y<yp)
      {
        xmin=x; xmax=xp;
        ymin=y; ymax=yp;
      }
      else
      {
        xmin=xp; xmax=x;
        ymin=yp; ymax=y;
      }
      for (dy=ymin; dy<=ymax; dy++)
      {
        dx=(dy-ymin)*(xmax-xmin)/(ymax-ymin)+xmin;
        chnkpos=dx+dy*segm->width;
        if ((chnkpos>=0) && (chnkpos<segm->width*segm->height))
        {      
          segm->ovlchnk[chnkpos] |=mask;
        }
      }
    }
  }
}


/****************************************
 * draw lon/lat lines to ovlchunk
 ****************************************/
#define ABSMAXLON 70
#define ABSMAXLAT 70

#define DLON 10
#define DLAT 10
#define DDL 1
void geo_add_lonlat(PREFER *prefer,SEGMENT *segm,PIC_CHARS *pc)
{
  float lon,lat;
  int x,y,ystart;
  if (!pc) return;
  ystart=((segm->xh.segment-1)-(segm->xh.seq_start-1))*segm->height;
  for (lon=-1*ABSMAXLON; lon<=ABSMAXLON; lon+=DLON)
  {
    int xp=-1,yp=-1;
    for (lat=-1*ABSMAXLAT; lat<=ABSMAXLAT; lat+=DDL)
    {
      glonlat2xy_xcorr(D2R(lon/*+pc->gchar.sub_lon*/),D2R(lat),&x,&y,&pc->gchar);
      y=y-ystart;
      if (((x<0)&&(xp<0)) || ((x>=segm->width) && (xp>=segm->width)))
      { xp=x; yp=y; continue; }
      if (((y<0)&&(yp<0)) || ((y>=segm->height) && (yp>=segm->height)))
      { xp=x; yp=y; continue; }
      draw_line_in_ovlchnk(segm,xp,yp,x,y,LMASK);
      xp=x;
      yp=y;
    }
  }

  for (lat=-1*ABSMAXLAT; lat<=ABSMAXLAT; lat+=DLAT)
  {
    int xp=-1,yp=-1;
    for (lon=-1*ABSMAXLON; lon<=ABSMAXLON; lon+=DDL)
    {
      glonlat2xy_xcorr(D2R(lon/*+pc->gchar.sub_lon*/),D2R(lat),&x,&y,&pc->gchar);
      y=y-ystart;
      if (((x<0)&&(xp<0)) || ((x>=segm->width) && (xp>=segm->width)))
      { xp=x; yp=y; continue; }
      if (((y<0)&&(yp<0)) || ((y>=segm->height) && (yp>=segm->height)))
      { xp=x; yp=y; continue; }
      draw_line_in_ovlchnk(segm,xp,yp,x,y,LMASK);

      xp=x;
      yp=y;
    }
  }
}

static void add_polygon(FILE *fp,GSHHS *h,SEGMENT *segm,PIC_CHARS *pc)
{
  int k;
  double lon, lat;
  int     max_east = 270000000;
  int xp,x,yp,y;
  gboolean restart_polygon=TRUE;
  int ystart;
  POINTx p;
  PIC_CHARS pci;
  if (!pc) return;
  pci=*pc;
  if (pc->orbittype==GEO)
  {
    pci.gmap=normal; // drawn in orig. pic, so ignore special projections!
  }
  ystart=((segm->xh.segment-1)-(segm->xh.seq_start-1))*segm->height;
  for (k=0; k<h->n; k++)
  {
    if (fread ((void *)&p,(size_t)sizeof(POINTx),(size_t)1, fp) != 1)
      break;
    p.x = GINT32_FROM_BE(p.x);
    p.y = GINT32_FROM_BE(p.y);
    lon = p.x * 1.0e-6;
    lon = (h->greenwich && p.x > max_east) ? lon - 360.0 : lon;
    lat = p.y * 1.0e-6;
    if (lon>180) lon-=360;
    if ((lonlat2dxy_bekijk(lon,lat,&x,&y,&pci)))
    {
      restart_polygon=TRUE;    // point of polygon out-of-range
      continue;
    }
    if (x>=segm->width)
    {
      restart_polygon=TRUE;
      continue;
    }

    if (pc->orbittype!=POLAR)
    {
      y=y-ystart;
    }

    if (!restart_polygon)
    {
      draw_line_in_ovlchnk(segm,xp,yp,x,y,CMASK);
    }
    restart_polygon=FALSE;
    xp=x; yp=y;
  }
}

// Add polygon to chnk
int add_shore(OVERLAY *ovl,SEGMENT *segm,PIC_CHARS *pc)
{
  FILE *fp;
  GSHHS h;
  int n;
  char *fn=NULL;
  if (ovl->dir)
  {
    strcpyd(&fn,ovl->dir);
    finish_path(fn);
  }
  strcatd(&fn,ovl->fname);
  if (!(fp=fopen(fn,"rb")))
  {
    free(fn);
    return -1;
  }
  free(fn);
  while ((n=read_polygon(fp,&h,-180,180,-90,90,0,10000)))
  {
    if (n>0)
    {
      add_polygon(fp,&h,segm,pc);
    }
  }
  fclose(fp);
  return 0;
}


/****************************************
 * Add border to chunk (vector or pic)
 ****************************************/
static int add_borders_to_chnk(PREFER *prefer,OVERLAYTYPE overlaytype,SEGMENT *segm,PIC_CHARS *pc)
{
  int x,width,y,height,ystart;
  int is_hrv,is_hrv_mdm;
  OVERLAY sel_ovl;
  guchar *line=NULL;
  if (!segm)    // free mem
  {
    load_overlay(prefer,NULL,overlaytype,&sel_ovl,&width,&height);
    return 0;
  }

  // add vector border overlay to ovlchnk
  if (overlaytype==coast_v)
  {
    OVERLAY *ovl=get_overlay(prefer->overlay,overlaytype,NULL,NULL);

    if (!ovl) return -1;                    //should never occur
    if (add_shore(ovl,segm,pc)) return -2;  // alleen nodig als naar file schrijven!
    return 0;
  }

  // add pic border overlay to ovlchnk
  line=(guchar *)load_overlay(prefer,segm->chan,overlaytype,&sel_ovl,&width,&height);
  if (!line) return -1;   // error

  if (segm->chan->scan_dir=='s')
    ystart=(segm->chan->segm->xh.seq_end-segm->xh.segment)*segm->height;
  else
    ystart=(segm->xh.segment-1)*segm->height;

  if (ystart<0) ystart=0; // ystart<0 should never occur  

  is_hrv=(!strcmp(segm->chan->chan_name,"HRV"));
  is_hrv_mdm=(is_hrv && (segm->xh.seq_end==1));   // one segment -> hrv from MDM
  for (y=0; y<segm->height; y++)
  {
    int dshift=0;
    if (y+ystart>=height) continue;  /* possibly wrong overlay file? */
    if (is_hrv)
    {
    // No info found about exact overlay shift. Seems to be 1876??
      if (sel_ovl.rapidscan)
        dshift=1876; // x=0 in segm equals x1=1876 in overlay file???
      else
        dshift=5568; // x=0 in segm equals x1=11136-5568=5568 in overlay file

      if (y+ystart < 11136-segm->chan->shift_ypos)
        dshift-=segm->chan->upper_x_shift;
      else
        dshift-=segm->chan->lower_x_shift;
    }

    if (is_hrv_mdm)
    {
      dshift-=segm->chan->upper_x_shift;    // only >0 if MDM, ch12g or ch12e
      dshift-=(segm->width-5568);
    }

    for (x=0; x<segm->width; x++)        // x=pos. in segment
    {
      int chnkpos,x1;
      if (segm->chan->scan_dir=='s')
        chnkpos=(segm->width-1-x)+(segm->height-1-y)*segm->width;
      else
        chnkpos=x+y*segm->width;

      x1=x+dshift;                       // x1=pos. in overlay file

      if (x1>=width) continue;  /* possibly wrong overlay file? */
      if (line[x1+width*(y+ystart)])
      {
        segm->ovlchnk[chnkpos] |=CMASK;
      }
      else
      {
        segm->ovlchnk[chnkpos] &=(~CMASK);
      }
    }
  }
  return 0;
}


// add a string to chunk
static void add_str2chnk(SEGMENT *segm,int x,int y,char *str,int flip)
{
  char ts[200];
  int size,ytxt,i,y0;
  int xstr,ystr;
  int chnkpos,ystart;
  ystart=(segm->xh.segment-segm->xh.seq_start)*segm->height;
  size=1;
  xstr=strlen(str)*8*size;
  ystr=8*size;
  for (y0=0; y0<ystr; y0++)
  {
    int y1=y0;
    if (flip) y1=ystr-1-y0;
    ytxt=(y1/size);
    get_strline(str,ytxt,ts,190);
    for (i=0; i<xstr; i++)
    {
      int i1=i;
      if (flip) i1=xstr-1-i;
      if (ts[i1/size])
      {
        int y1=y+y0-ystart;
        int x1=x;
        if (flip) x1=x-xstr-5;
        chnkpos=i+x1+y1*segm->width;
        if (y1<segm->height)
        {
          segm->ovlchnk[chnkpos] |=MMASK;
        }
      }
    }
  }
}

/****************************************
 * add one landmark to chunk:
 *   cross or dot
 *   add name
 ****************************************/
#define LMARKSIZEDOT 1
#define LMARKSIZECROSS 3
static void add_landmark(SEGMENT *segm,PIC_CHARS *pc,POINT pos,gboolean cross,char *name)
{
  int x1,y1,x,y,ystart,npix;
  int chnkpos;
  if (!pc) return;
  glonlat2xy_xcorr(D2R(pos.lon),D2R(pos.lat),&x,&y,&pc->gchar);
  ystart=(segm->xh.segment-segm->xh.seq_start)*segm->height;
  npix=segm->width*segm->height;
  if (cross)   // Cross
  {
    for (y1=y-LMARKSIZECROSS; y1<=y+LMARKSIZECROSS; y1++)
    {
      chnkpos=x+(y1-ystart)*segm->width;
      if ((chnkpos>=0) && (chnkpos<npix))
      {      
        segm->ovlchnk[chnkpos] |=MMASK;
      }
    }
    for (x1=x-LMARKSIZECROSS; x1<=x+LMARKSIZECROSS; x1++)
    {
      chnkpos=x1+(y-ystart)*segm->width;
      if ((chnkpos>=0) && (chnkpos<npix))
      {      
        segm->ovlchnk[chnkpos] |=MMASK;
      }
    }
  }
  else   // Dot
  {
    for (y1=y-LMARKSIZEDOT; y1<=y+LMARKSIZEDOT; y1++)
    {
      for (x1=x-LMARKSIZEDOT; x1<=x+LMARKSIZEDOT; x1++)
      {
        chnkpos=x1+(y1-ystart)*segm->width;
        if ((chnkpos>=0) && (chnkpos<segm->width*segm->height))
        {      
          segm->ovlchnk[chnkpos] |=MMASK;
        }
      }
    }
  }
  if (name)
  {
    add_str2chnk(segm,x,y,name,1);
  }

  return;
}

static void clear_overlay(SEGMENT *segm,int mask)
{
  int x,y,chnkypos;
  if (!segm->loaded_ovl) return;
  for (y=0; y<segm->height; y++)
  {
    chnkypos=y*segm->width;
    for (x=0; x<segm->width; x++)
    {
      segm->ovlchnk[x+chnkypos] &=(~mask);
    }
  }
  segm->loaded_ovl=0;
}

static void add_waypoints(SEGMENT *segm,PIC_CHARS *pc,PREFER *prefer)
{
  PLACES *pl;
  for (pl=prefer->places; pl; pl=pl->next)
    add_landmark(segm,pc,pl->p,FALSE,pl->name);
}

/* Add overlay to ovlchnk, for file generation */
/****************************************
 * Add overlay to ovlchnk, for file generation:
 *   borders
 *   mark
 ****************************************/
OVERLAYTYPE add_overlay(PREFER *prefer,OVERLAYTYPE overlaytype,SEGMENT *segm,PIC_CHARS *pc)
{
  if (!segm)    // free mem of overlay file
  {
    add_borders_to_chnk(prefer,overlaytype,segm,pc); // free mem
    return 0;
  }

  clear_overlay(segm,(CMASK|CLMASK));

  if (prefer->prf_ovltype=='V')
  {
    if (overlaytype==coast_p) overlaytype=coast_v;
//    if (overlaytype==country_p) overlaytype=country_v;
    if ((add_borders_to_chnk(prefer,overlaytype,segm,pc)) < 0)
    {
      if (overlaytype==coast_v) overlaytype=coast_p;
//      if (overlaytype==country_v) overlaytype=country_p;
      if ((add_borders_to_chnk(prefer,overlaytype,segm,pc)) < 0) return -1;
    }
// nog te doen: tekst in drawable ipv chnk (zie draw_marks)
    add_waypoints(segm,pc,prefer);  
  }
  else
  {
    if (overlaytype==coast_v) overlaytype=coast_p;
//    if (overlaytype==country_v) overlaytype=country_p;
    if ((add_borders_to_chnk(prefer,overlaytype,segm,pc)) < 0)
    {
      if (overlaytype==coast_p) overlaytype=coast_v;
//      if (overlaytype==country_p) overlaytype=country_v;
      if ((add_borders_to_chnk(prefer,overlaytype,segm,pc)) < 0) return -1;
    }

    add_landmark(segm,pc,prefer->lmark,TRUE,NULL);  // mark
    add_waypoints(segm,pc,prefer);  
  }


  segm->loaded_ovl=overlaytype;
  return overlaytype;
}








#ifndef __NOGUI__
/****************************************
 * Draw overlays in drawable:
 *  shores etc.,
 *  cities TE DOEN!
 *  mark
 *  lon/lat
 ****************************************/
static void plot_polygon(FILE *fp,GSHHS *h,GtkWidget *drawing_area,
             RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int iclr)
{
  int k;
  double lon, lat;
  int     max_east = 270000000;
  int xp,x,yp,y;
  int start=1;
  POINTx p;
  PIC_CHARS pci=*pc;
  GdkColor clr;
  if (pc->orbittype==GEO)
  {
    pci.gchar.shift_ypos=0;
    pci.gchar.upper_x_shift=0;
    pci.gchar.lower_x_shift=0;
  }
  clr.red  =(((prefer.ovl_clr>>8)&0xf)*iclr)/0xf;
  clr.green=(((prefer.ovl_clr>>4)&0xf)*iclr)/0xf;
  clr.blue =(((prefer.ovl_clr>>0)&0xf)*iclr)/0xf;
  for (k=0; k<h->n; k++)
  {
    if (fread ((void *)&p,(size_t)sizeof(POINTx),(size_t)1, fp) != 1)
      break;
    p.x = GINT32_FROM_BE(p.x);
    p.y = GINT32_FROM_BE(p.y);
    lon = p.x * 1.0e-6;
    lon = (h->greenwich && p.x > max_east) ? lon - 360.0 : lon;
    lat = p.y * 1.0e-6;
    if (lon>180) lon-=360;
    if ((lonlat2dxy_bekijk(lon,lat,&x,&y,&pci)))
    {
      start=1;
      continue;
    }
    if (!start)
    {
      draw_line_in_prev(rgbi,rgbpi,&pci, xp, yp, x, y,0,&clr);
    }
    start=0;
    xp=x; yp=y;
  }
}

/*
maps: gshhs_c.b: horrible
maps: gshhs_l.b: terrible
maps: gshhs_i.b: best for MSG
maps: gshhs_h.b: good, a bit slow
maps: gshhs_f.b: very slow
*/

/* Add overlay to drawable, for preview */
void draw_shore(GtkWidget *drawing_area,RGBI *rgbi,RGBPICINFO *rgbpi,OVERLAY *ovl,PIC_CHARS *pc,int clr)
{
  FILE *fp;
  GSHHS h;
  int n;

  char *fn=NULL;
  if (!ovl) return;
  if (ovl->dir) strcpyd(&fn,ovl->dir);
  finish_path(fn);
  strcatd(&fn,ovl->fname);
  if (!(fp=fopen(fn,"rb")))
  {
    free(fn);
    return;
  }
  free(fn);

  while ((n=read_polygon(fp,&h,-180,180,-90,90,0,10000)))
  {
    if (n>0)
    {
      plot_polygon(fp,&h,drawing_area,rgbi,rgbpi,pc,clr);
    }
  }
  fclose(fp);
  Refresh_Rect(drawing_area,0,0,rgbi->width,rgbi->height);    /* Make new row visible */
}

// in preview
/*
 * Add special mark to drawable, for preview 
 */
void draw_landmark1(GtkWidget *drawing_area,RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,
                POINT lmark,char *name,int iclr,gboolean cross)
{
  float lon,lat;
  int x,y;
  int marksize;
  GdkColor clr;
  clr.red  =(((0xf00>>8)&0xf)*iclr)/0xf;
  clr.green=(((0xf00>>4)&0xf)*iclr)/0xf;
  clr.blue =(((0xf00>>0)&0xf)*iclr)/0xf;
  lon=lmark.lon;
  lat=lmark.lat;
  lonlat2dxy(lon,lat,&x,&y,pc);
  if (cross)
  {
    marksize=MAX(prefer.size_cmark/rgbpi->zx,1);
    draw_line_in_prev(rgbi,rgbpi,pc, x-marksize, y, x+marksize, y,200,&clr);
    draw_line_in_prev(rgbi,rgbpi,pc, x, y-marksize, x, y+marksize,200,&clr);
  }
  else
  {
    marksize=MAX(10/rgbpi->zx,1);
    for (; marksize>=0; marksize--)
    {
      draw_line_in_prev(rgbi,rgbpi,pc, x-marksize, y-marksize, x+marksize, y-marksize,200,&clr);
      draw_line_in_prev(rgbi,rgbpi,pc, x+marksize, y-marksize, x+marksize, y+marksize,200,&clr);
      draw_line_in_prev(rgbi,rgbpi,pc, x-marksize, y+marksize, x+marksize, y+marksize,200,&clr);
      draw_line_in_prev(rgbi,rgbpi,pc, x-marksize, y-marksize, x-marksize, y+marksize,200,&clr);
      draw_line_in_prev(rgbi,rgbpi,pc, x-marksize, y-marksize, x+marksize, y+marksize,200,&clr);
      draw_line_in_prev(rgbi,rgbpi,pc, x-marksize, y+marksize, x+marksize, y-marksize,200,&clr);
    }
  }

  if (name)
  {
    draw_str_in_prev(rgbi,rgbpi,pc,x-2,y+2,name,&clr);
  }
}

void draw_landmark(GtkWidget *drawing_area,RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,
                int iclr)
{
  POINT p;
  p.lon=prefer.lmark.lon;
  p.lat=prefer.lmark.lat;
  draw_landmark1(drawing_area,rgbi,rgbpi,pc,p,NULL,iclr,(prefer.size_cmark!=0));
}

void draw_waypoints(GtkWidget *drawing_area,RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int iclr)
{
  PLACES *pl;
  for (pl=prefer.places; pl; pl=pl->next)
    draw_landmark1(drawing_area,rgbi,rgbpi,pc,pl->p,pl->name,iclr,FALSE);
}
/*
 * Add lon/lat lines to drawable, for preview 
 * NOTE: Ignore HRV x-shifts; add is on drawable, not in segm!
 */
void draw_lonlat(GtkWidget *drawing_area,
             RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int iclr)
{
  float lon, lat;
  int xp,x,yp,y;
  GdkColor clr;
  clr.red  =(((prefer.lonlat_clr>>8)&0xf)*iclr)/0xf;
  clr.green=(((prefer.lonlat_clr>>4)&0xf)*iclr)/0xf;
  clr.blue =(((prefer.lonlat_clr>>0)&0xf)*iclr)/0xf;
  for (lon=-180.; lon<=180; lon+=DLON)
  {
    int start=1;
    for (lat=-90.; lat<=90; lat+=1)
    {
      if (lonlat2dxy(lon,lat,&x,&y,pc))
      {
        start=1;
        continue;
      }
      if (!start)
        draw_line_in_prev(rgbi,rgbpi,pc, xp, yp, x, y,0,&clr);
      xp=x; yp=y;
      start=0;
    }
  }
  for (lat=-90.; lat<=90; lat+=DLAT)
  {
    int start=1;
    for (lon=-180.; lon<=180; lon+=1)
    {
      if (lonlat2dxy(lon+pc->gchar.sub_lon,lat,&x,&y,pc))
      {
        start=1;
        continue;
      }
      if (!start)
        draw_line_in_prev(rgbi,rgbpi,pc, xp, yp, x, y,0,&clr);
      xp=x; yp=y;
      start=0;
    }
  }
}

#endif
