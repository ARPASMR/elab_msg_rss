/**************************************************
 * RCSId: $Id$
 *
 * Calculate current observation point for polar satellites 
 * Project: WSAT
 * Author: R. Alblas
 *
 * History: 
 * $Log$
 **************************************************/
/*******************************************************************
 * Copyright (C) 2000 R. Alblas 
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
/**************************************************
  Main functions:
    xy2lonlat(): translates (x,y) of picture into (lon,lat)
    calc_orbitconst()
 **************************************************/

#include "xrit2pic.h"
#include "avhrr.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>





// Calc. sub-sat positions if not available (AVHRR format)
void calc_polar_segmsubsats(GROUP *grp,gboolean force)
{
  SEGMENT *segm;
  int xmax,ymax;
  POINT pos1;
  if (!grp) return;
  if (grp->orbittype!=POLAR) return;
  if (!grp->chan) return;
  if (!grp->chan->segm) return;
  xmax=grp->chan->nc;
  ymax=0;
  for (segm=grp->chan->segm; segm; segm=segm->next)
  {
    if (!xy2lonlat(xmax/2,ymax,&grp->pc,&pos1)) return;
    // if subsat_start lon and lat both 0.0: set them to calculated value
    if (((!segm->orbit.subsat_start.lon) && (!segm->orbit.subsat_start.lat)) || (force))
    {
      segm->orbit.subsat_start.lon=R2D(pos1.lon);
      segm->orbit.subsat_start.lat=R2D(pos1.lat);
    }

    if (!xy2lonlat(xmax/2,ymax+grp->chan->nl-1,&grp->pc,&pos1)) return;
    // if subsat_end lon and lat both 0.0: set them to calculated value
    if (((!segm->orbit.subsat_end.lon) && (!segm->orbit.subsat_end.lat)) || (force))
    {
      segm->orbit.subsat_end.lon=R2D(pos1.lon);
      segm->orbit.subsat_end.lat=R2D(pos1.lat);
    }
    ymax+=grp->chan->nl;
  }
}

// in adapt_lastsegmnr: max. of 102 min if t_stop=0; here not done???
/*
 t_start=first used segment 
 t_stop =last used segment 
 */
int noaa_coverage(GROUP *grp,char *s1,char *s2,char *s3,int t_start,int t_stop)
{
  POINT pos1,pos2,pos3;
  SEGMENT *segma,*segmz;
  int xmax,ymax;
  int age=99999;    // this ret-value means: no Kepler
  *s1=*s2=*s3=0;
  if (!grp->chan) return 0;
  segma=grp->chan->segm;
  for (segmz=segma; segmz && segmz->next; segmz=segmz->next);

  if (t_start) t_start--;
  t_start*=(grp->chan->nl);  // offset was in minutes, now in lines; start of first segment
  t_stop*=(grp->chan->nl);   // offset was in minutes, now in lines; end of last segment
  age=load_kepler(grp);

  xmax=grp->chan->nc;
  grp->pc.width=xmax;
  grp->pc.lines_per_sec=grp->chan->lines_per_sec;

  // ?? grp->chan->seq_start=1 altijd, dus ymaxkomt overeen met maximale t_stop??
  ymax=grp->chan->nl*(grp->chan->seq_end-grp->chan->seq_start+1);  // amount of lines

  if (t_stop) ymax=MIN(ymax,t_stop);

  if (grp->orbittype!=POLAR) return age;
  sprintf(s2,"%2d min.",ymax/360);
  if (!xy2lonlat(xmax/2,t_start,&grp->pc,&pos1)) return age;   // pos. start of first segment
  if (!xy2lonlat(xmax/2,t_start+10,&grp->pc,&pos2)) return age;
  if (!xy2lonlat(xmax/2,ymax,&grp->pc,&pos3)) return age;
  *s1=*s3=0;
  if ((segma) && (segma->orbit.orbit_start)) sprintf(s1,"%d ",segma->orbit.orbit_start);
  if ((segmz) && (segmz->orbit.orbit_start)) sprintf(s3,"%d ",segmz->orbit.orbit_start);
  sprintf(s1,"%s[%4d,%4d]",s1,(int)(R2D(pos1.lon)),(int)(R2D(pos1.lat)));
  sprintf(s2,"%2d min. going %s",ymax/360,((pos2.lat > pos1.lat)? "N" : "S"));
  sprintf(s3,"%s[%d,%d]",s3,(int)(R2D(pos3.lon)),(int)(R2D(pos3.lat)));
  return age;
}

/*
grp: starttime of first segment selected.
*/
int polar_dir(GROUP *grp)
{
  POINT pos1,pos2;
  int t_start,t_stop;
  int width=grp->chan->nc;
  t_start=grp->chan->seq_start;
  t_stop=grp->chan->seq_end;
  grp->pc.width=width;

  if (t_start) t_start--;
  t_start*=(grp->chan->nl);  // offset was in minutes, now in lines; start of first segment
  t_stop*=(grp->chan->nl);   // offset was in minutes, now in lines; end of last segment

  t_stop=t_stop-t_start;     // grp->tm is start of first segment selected!
  t_start=0;                 // so start at 0
  if (!xy2lonlat(width/2,t_start,&grp->pc,&pos1)) return 0;
  if (!xy2lonlat(width/2,t_stop,&grp->pc,&pos2)) return 0;
  if (pos2.lat > pos1.lat) return 's';
  return 'n';
}

/*
       * (h1,v1)=pos11             * (h2,v1)=pos12
       
       * (h1,v2)=pos21             * (h2,v2)=pos22
*/

#define METH_NEW

#ifdef METH_NEW

typedef struct
{
  int x,y;
  float l;
  float lon,lat;
  int oor;
} POINT2;

// (x1,x2) = (l1,l2) ; l1<=l<=l2 ==> x1<x<=x2
static int conv(int x1,int x2,float l1,float l2,int l)
{
  int x;
  x=(float)(x2-x1)*(float)(l-l1)/(l2-l1) + x1;
  if (((x<x1)&&(x<x2)) || ((x>x1)&&(x>x2))) x=-1;
  return x;  
}

/*
    p11   [1]    p12

    [4]         [2]

    p21   [3]    p22
 */

/* */
// Bepaal x-pos. l links, tussen p11.l en p21.l
// Als buiten bereik: maak oor TRUE
POINT2 cross_l(POINT2 p11,POINT2 p12,POINT2 p21,POINT2 p22,int l)
{
  POINT2 pp;
  pp.oor=FALSE;
  pp.y=conv(p21.y,p11.y,p21.l,p11.l,l);
  pp.x=p21.x;
  if (pp.y>=0) return pp;
  pp.oor=TRUE;
  return pp;
}

// Bepaal x-pos. l onder, tussen p22.l en p21.l
// Als buiten bereik: ga naar cross_l
POINT2 cross_b(POINT2 p11,POINT2 p12,POINT2 p21,POINT2 p22,int l)
{
  POINT2 pp;
  pp.oor=FALSE;
  pp.x=conv(p21.x,p22.x,p21.l,p22.l,l);
  pp.y=p21.y;
  if (pp.x>=0) return pp;
  return cross_l(p11,p12,p21,p22,l);
}

// Bepaal x-pos. l rechts, tussen p12.l en p22.l
// Als buiten bereik: ga naar cross_b
POINT2 cross_r(POINT2 p11,POINT2 p12,POINT2 p21,POINT2 p22,int l)
{
  POINT2 pp;
  pp.oor=FALSE;
  pp.y=conv(p12.y,p22.y,p12.l,p22.l,l);
  pp.x=p12.x;
  if (pp.y>=0) return pp;
  return cross_b(p11,p12,p21,p22,l);
}

// p11,p12,p21,p22: (pab.x,pab.y) en (pab.lat,pab.lon) op 4 hoekpunten
// Maak pab.l=pab.lat of pab.lon
// Bepaal x-pos. l boven, tussen p11.l en p12.l
// Als buiten bereik: ga naar cross_r
POINT2 cross_t(POINT2 p11,POINT2 p12,POINT2 p21,POINT2 p22,int l)
{
  POINT2 pp;
  pp.oor=FALSE;
  pp.x=conv(p11.x,p12.x,p11.l,p12.l,l);
  pp.y=p11.y;
  if (pp.x>=0) return pp;
  return cross_r(p11,p12,p21,p22,l);
}

void draw_segmline(SEGMENT *segm,int x1,int y1,int x2,int y2) /* start/end point */
{
  int ix,iy,chnkpos;
  if ((ABS(x2-x1)) > (ABS(y2-y1)))
  {
    if (x1>x2)
    {
      ix=x1; x1=x2; x2=ix;
      iy=y1; y1=y2; y2=iy;
    }

    for (ix=x1; ix<=x2; ix++)
    {
      if (x2!=x1)
        iy=(float)(ix-x1)/(float)(x2-x1)*(y2-y1)+y1;
      else
        iy=y2;

/* Test out-of-range */
      if ((ix<0) || (ix>=segm->width)) continue;
      if ((iy<0) || (iy>=segm->height)) continue;

      chnkpos=ix+iy*segm->width;
      segm->ovlchnk[chnkpos] |=LMASK;
    }
  }
  else
  {
    if (y1>y2)
    {
      ix=x1; x1=x2; x2=ix;
      iy=y1; y1=y2; y2=iy;
    }

    for (iy=y1; iy<=y2; iy++)
    {
      if (y2!=y1)
        ix=(float)(iy-y1)/(float)(y2-y1)*(x2-x1)+x1;
      else
        ix=x2;

/* Test out-of-ramge */
      if ((ix<0) || (ix>=segm->width)) continue;
      if ((iy<0) || (iy>=segm->height)) continue;

      chnkpos=ix+iy*segm->width;
      segm->ovlchnk[chnkpos] |=LMASK;
    }
  }
}

POINT2 cross_x(POINT2 p11,POINT2 p12,POINT2 p21,POINT2 p22,int l);
void teken(char hv,POINT2 pa,POINT2 pb,
           POINT2 p11,POINT2 p12,POINT2 p21,POINT2 p22,
           POINT2 cross_x(),SEGMENT *segm)
{
  POINT2 pp;
  int il,xy;
  for (il=MIN(pa.l,pb.l); il<MAX(pa.l,pb.l)+1; il++)
  {
    if ((ABS(pa.lat))>85) continue;
    if ((ABS(pb.lat))>85) continue;
    if ((il%5)) continue;
    if (hv=='h')
      xy=conv(pa.x,pb.x,pa.l,pb.l,il);          // boven
    else
      xy=conv(pa.y,pb.y,pa.l,pb.l,il);          // boven
    if (xy<0) continue;
    pp=cross_x(p11,p12,p21,p22,il);                    // zoek r/o/l
    if (!pp.oor)
    {
      if (hv=='h')
        draw_segmline(segm,xy,pa.y,pp.x,pp.y);
      else
        draw_segmline(segm,pa.x,xy,pp.x,pp.y);
    }
  }
}

/*
  teken in vierkant (h1,v2)-(h2,v2) lon en lat lijnen
*/
void draw_lonlat_pix(int h1,int v1,int h2,int v2,
                     POINT pos11,POINT pos12,POINT pos21,POINT pos22,
                     SEGMENT *segm)
{
  POINT2 p11,p12,p21,p22;
  p11.x=h1; p11.y=v1; p11.lat=R2D(pos11.lat);  p11.lon=R2D(pos11.lon);
  p12.x=h2; p12.y=v1; p12.lat=R2D(pos12.lat);  p12.lon=R2D(pos12.lon);
  p21.x=h1; p21.y=v2; p21.lat=R2D(pos21.lat);  p21.lon=R2D(pos21.lon);
  p22.x=h2; p22.y=v2; p22.lat=R2D(pos22.lat);  p22.lon=R2D(pos22.lon);

  p11.l=p11.lon; p12.l=p12.lon; p21.l=p21.lon; p22.l=p22.lon;
  teken('h',p11,p12,p11,p12,p21,p22,cross_r,segm);
  teken('v',p12,p22,p11,p12,p21,p22,cross_b,segm);
  teken('h',p21,p22,p11,p12,p21,p22,cross_l,segm);

  p11.l=p11.lat; p12.l=p12.lat; p21.l=p21.lat; p22.l=p22.lat;
  teken('h',p11,p12,p11,p12,p21,p22,cross_r,segm);
  teken('v',p12,p22,p11,p12,p21,p22,cross_b,segm);
  teken('h',p21,p22,p11,p12,p21,p22,cross_l,segm);

}

/*************************************
 * Draw lon/lat lines
 * Method: Scan all pixels and test on lon/lat=value to draw
 *************************************/
int polar_add_lonlat(SEGMENT *segm,PIC_CHARS *pci)
{
  int wnd_width =segm->width;
  int wnd_height=segm->height;
  int v;
  POINT *pos_prevline;
  PIC_CHARS pc=*pci;

/* Abort if no valid Kepler data */
  if (!pc.orbit.valid) return 0;

  pc.orbit.start_tm=segm->xh.time;  // start position this segment

  pos_prevline=(POINT *)malloc(wnd_width*sizeof(*pos_prevline));

#define DH 10
#define DV 10
/* 2b. Scan picture and draw. */
  for (v=segm->y_offset; v<segm->y_offset+wnd_height-1; v+=DV)
  {
    int h1;
/* In h-dir: 2 loops; first: increment DH; second: fill the DH-gap.
   To make sure complete area is scanned the upper-limit
     of first loop is incremented with DH.
*/
    for (h1=0; h1<wnd_width-1+DH; h1+=DH)
    {
/* Calc. current lon/lat */
      POINT pos11,pos12,pos21,pos22;
      if (h1>=wnd_width) break;
      xy2lonlat(h1,v,&pc,&pos22);
      pos12=pos_prevline[h1];
      if ((h1) && (v>segm->y_offset))
      {
        draw_lonlat_pix(h1-DH,v-DV,h1,v,pos11,pos12,pos21,pos22,segm);
      }
      pos21=pos22;
      pos11=pos12;
      pos_prevline[h1]=pos22;   
    }
  }

/* Ready, reset cursor. */
  free(pos_prevline);
  return 1;
}

void add_str2chnk(SEGMENT *segm,int x,int y,char *str,int flip)
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
    if (flip) y1=ystr-y0;
    ytxt=(y1/size);
    get_strline(str,ytxt,ts,190);
    chnkpos=x+(y-ystart)*segm->width;
    for (i=0; i<xstr; i++)
    {
      int i1=i;
      if (flip) i1=xstr-1-i;
      if (ts[i1/size])
      {
        int y1=y+y0-ystart;
        int x1=x;
        chnkpos=i+x1+y1*segm->width;
        if (y1<segm->height)
        {
          segm->ovlchnk[chnkpos] |=MMASK;
        }
      }
    }
  }
}
#else


niet gebruikt
static void set_chnk_segm(SEGMENT *segm,long chnkpos)
{
  CHANNEL *ch;
  SEGMENT *s;
  for (ch=segm->chan; ((ch)&&(ch->prev)); ch=ch->prev);
  for (; ch; ch=ch->next)
  {
    for (s=ch->segm; s; s=s->next)
    {
      if (s->xh.segment==segm->xh.segment)
      {
        s->ovlchnk[chnkpos] |=LMASK;
        break;
      }
    }
  }
}


/*************************************
 * Draw lon/lat lines
 * Method: Scan all pixels and test on lon/lat=value to draw
 *************************************/
niet gebruikt
static void Draw_Lonlatpix(int x,int y,  /* current position in window */
                    POINT *pos,          /* current lon/lat */
                    POINT *pospx,        /* previous lon/lat in x-dir. */
                    POINT *pospy,        /* previous lon/lat in y-dir. */
                    int dlon,int dlat,   /* interval between 2 lon/lats to draw */
                    SEGMENT *segm)       /* contains drawing buffer */
{
  int ilon =(int)R2D(pos->lon);
  int ilonpx=(int)R2D(pospx->lon);
  int ilonpy=(int)R2D(pospy->lon);
  int ilat =(int)R2D(pos->lat);
  int ilatpx=(int)R2D(pospx->lat);
  int ilatpy=(int)R2D(pospy->lat);
  int max_abs_ilonx=MAX((ABS(ilon)),(ABS(ilonpx)));
  int max_abs_ilony=MAX((ABS(ilon)),(ABS(ilonpy)));
  int max_abs_ilatx=MAX((ABS(ilat)),(ABS(ilatpx)));
  int max_abs_ilaty=MAX((ABS(ilat)),(ABS(ilatpy)));
  int chnkpos;
  chnkpos=x+y*segm->width;
/* Detect if lon is passed which has to be drawn */
  if ( ((ilon!=ilonpx) && (!(max_abs_ilonx%dlon))) ||
       (SIGN(pos->lon)!=SIGN(pospx->lon)) ||
       ((ilon!=ilonpy) && (!(max_abs_ilony%dlon))) ||
       (SIGN(pos->lon)!=SIGN(pospy->lon)) )
  {
/* This position contains lon line to draw, so set pixel */
    segm->ovlchnk[chnkpos] |=LMASK;
    set_chnk_segm(segm,chnkpos);
/* Add value */
    if (y==1)
    {
      char tmp[10]; sprintf(tmp,"%d",NINT(R2D(pos->lon)));
      add_str2chnk(segm,x+3,6,tmp,1);
    }
  }

/* Detect if lat is passed which has to be drawn */
  if ( ((ilat!=ilatpx) && (!(max_abs_ilatx%dlat))) ||
       (SIGN(pos->lat)!=SIGN(pospx->lat)) ||
       ((ilat!=ilatpy) && (!(max_abs_ilaty%dlat))) ||
       (SIGN(pos->lat)!=SIGN(pospy->lat)) )
  {
/* This position contains lat line to draw, so set pixel */

    segm->ovlchnk[chnkpos] |=LMASK;
    set_chnk_segm(segm,chnkpos);

/* Add value */
    if (x==1)
    {
      char tmp[10]; sprintf(tmp,"%d",NINT(R2D(pos->lat)));
      add_str2chnk(segm,8,y+5,tmp,1);
    }
  }
}

/*************************************
 * Draw lon/lat lines
 * Method: Scan all pixels and test on lon/lat=value to draw
 *************************************/
niet gebruikt
int draw_lonlat(SEGMENT *segm,PIC_CHARS *pci)
{
  int wnd_width =segm->width;
  int wnd_height=segm->height;
  POINT pos,pos_px,pos_py;
  int dlon=1,dlat=1;
  int h,v;
  POINT *pos_prevline;
  PIC_CHARS pc=*pci;

/* Abort if no valid Kepler data */
  if (!pc.orbit.valid) return 0;

  pc.orbit.start_tm=segm->xh.time;  // start position this segment

  pos_prevline=(POINT *)malloc(wnd_width*sizeof(*pos_prevline));

  dlon=dlat=5;
/* 2.
   Scan all h/v of current window 
   Compare lon/lat of each pix. with pix. left ("posx") and pix. above ("posy")
   to detect positions crossed by lon/lat lines.
   Scanning is done in x-dir; after 1 scan, y is incremented. 
   So, posx is simply previous pos, but for posy previous line is needed.
   This line is saved in pos_prevline.
 ........y..............
 .......xo..............

*/

/* 2a. To speed up: Increment by "DH" in X-direction.
       Increment in y-dir is 1; >1 is more difficult, and there is 
       not much improve because other calcs have now the upper hand.
*/ 
#define DH 5
#define DV 1
/* 2b. Scan picture and draw. */
  for (v=segm->y_offset; v<segm->y_offset+wnd_height-1; v+=DV)
  {
    int h1;
    POINT pos1,posp;
/* In h-dir: 2 loops; first: increment DH; second: fill the DH-gap.
   To make sure complete area is scanned the upper-limit
     of first loop is incremented with DH.
*/
    for (h1=0; h1<wnd_width-1+DH; h1+=DH)
    {
/* Calc. current lon/lat */
      xy2lonlat(h1,v,&pc,&pos1);
      if ((ABS(R2D(pos1.lat))) > 85) continue; // don't draw too close at the pole

/* Interpolate to fill the DH-gap */
      if (h1) for (h=h1-DH; h<h1; h++)
      {
        if (h>=wnd_width) break; /* Run out of picture */
/* Interpolate lon/lat between the 2 by xy2lonlat calculated positions */
        pos.lon=posp.lon+(pos1.lon-posp.lon)*(h-(h1-DH))/DH;
        pos.lat=posp.lat+(pos1.lat-posp.lat)*(h-(h1-DH))/DH;

/* Copy lon/lat of pixel in prev. line (present in pos_prevline) to pos_py */
        pos_py=pos_prevline[h];  
/* Save current pos to pos_prevline, needed to process next line 
   (old pos_prevline not needed anymore) 
*/
        pos_prevline[h]=pos;   

/* Set a point if it is on a lon or lat line */
        if ((h>0) && (v>0))
        {
          Draw_Lonlatpix(h,v,&pos,&pos_px,&pos_py,dlon,dlat,segm);
        }
        pos_px=pos;
      }
      posp=pos1;
    }
  }

/* Ready, reset cursor. */
  free(pos_prevline);
  return 1;
}

#endif

#ifdef XXXX
float vkw(float a,float b,float c,int pm)
{
  float z;
//printf("a=%f  b=%f  c=%f\n",a,b,c);
  if (a)
    z=(-1.*b + pm*(sqrt(b*b-4*a*c)))/(2.*a);
  else
    z=-1*c/b;
  return z;
}


/*************************************
 * Translate lon/lat into x/y position
 * y=0 -> time of start-of-pic, in grp->grp_tm
 *************************************/
int lonlat2xy(POINT *pos_sensor,        /* output position satellite */
              PIC_CHARS *pci,           /* contains orbit info */
              int *x,int *y)              /* input coordinates */
{
  ORBIT *orbit=&pci->orbit;
  KEPLER *kepler=&pci->kepler;
  float dift,alpha,f1;
  POINT ps;
  int avhrr_linespsec=AVHRR_LINESPSEC;
  int n=orbit->loop_nr+1;     // om de een of andere reden 1 omloop mis, met +1 corrigeren...
  int secs,secs_orbit_strttime;
  ps.z=sin(pos_sensor->lat);
  f1=ps.z/sin(kepler->inclination);
  f1=MIN(f1,1.);
  f1=MAX(f1,-1.);
  alpha=asin(f1);  // =orbit_angle+dh_n
  alpha=PI-alpha;
  avhrr_linespsec=pci->lines_per_sec;
  {
    float a,b,c;
    a=kepler->decay_rate*2.*PI;
    b=orbit->k2_n+(kepler->motion*2.*PI);
    c=(alpha-kepler->perigee-kepler->anomaly+n*2.*PI)*-1.;
    dift=vkw(a,b,c,1);
  }
//printf("lonlat2xy >>>>>>>> dift=%f  lat=%f  z=%f  alpha=%f  %f  n=%d  incl=%f\n",dift,R2D(pos_sensor->lat),ps.z,R2D(alpha),ps.z/sin(kepler->inclination),n,R2D(kepler->inclination));

  secs=dift*24*3600+orbit->ref_time;   // tijd 
  secs_orbit_strttime=mktime_ntz(&orbit->start_tm);
  *y=(secs-secs_orbit_strttime)*avhrr_linespsec;
//printf("lonlat2xy: secs=%d  secs_orbit_strttime=%d  dift=%f\n",secs,secs_orbit_strttime,dift);
// als x in midden dan y OK.
// Nu correctie x aanbrengen...

*x=0;
  return 0;
}
#endif

#ifdef XXX
void draw_polaroverlay(SEGMENT *segm,PIC_CHARS *pci)
{
  int wnd_width =segm->width;
  int wnd_height=segm->height;
  POINT pos,pos_px,pos_py;
  int dlon=1,dlat=1;
  int h,v;
  POINT *pos_prevline;
  PIC_CHARS pc=*pci;

/* Abort if no valid Kepler data */
  if (!pc.orbit.valid) return 0;

  pc.orbit.start_tm=segm->xh.time;  // start position this segment
  for (v=segm->y_offset; v<segm->y_offset+wnd_height-1; v+=DV)
  {
    int h1;
    POINT pos1,posp;
/* In h-dir: 2 loops; first: increment DH; second: fill the DH-gap.
   To make sure complete area is scanned the upper-limit
     of first loop is incremented with DH.
*/
    for (h1=0; h1<wnd_width-1+DH; h1+=DH)
    {
/* Calc. current lon/lat */
      xy2lonlat(h1,v,&pc,&pos1);
    }
  }
}
#endif
