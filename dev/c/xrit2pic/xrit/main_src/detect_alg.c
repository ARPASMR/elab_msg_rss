/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Common functions for Preview and File generation
 ********************************************************************/
#include "xrit2pic.h"
#include <string.h>

#include <math.h>
typedef struct ir1cal
{
  double wlen;
  double NUc;    // [cm-1]
  double alpha;
  double beta;   // [K]
} IRCAL1;

typedef struct ircal
{
  IRCAL1 ircal[8];
} IRCAL;


IRCAL ircal=
{
  {
    {  3.9,      2569.094,   0.9959,   3.471 },
    {  6.2,      1598.566,   0.9963,   2.219 },
    {  7.3,      1362.142,   0.9991,   0.485 },
    {  8.7,      1149.083,   0.9996,   0.181 },
    {  9.7,      1034.345,   0.9999,   0.060 },
    { 10.8,       930.659,   0.9983,   0.627 },
    { 12.0,       839.661,   0.9988,   0.397 },
    { 13.4,       752.381,   0.9981,   0.576 },
  }
};

/* Translate radiance into temperature (IR channels only). 
  Inverse formula of Black body radiance; see pdf_msg_seviri_rad2bright.pdf
*/
float rad2temp(int irn,double R)
{
//  const double c=299792458;     //  ms **-1
//  const double h=6.6260755e-34; //  Js
//  const double k=1.380662e-23;  //  JK-  1
  const double C1=1.19104e-16;  // Wm2
  const double C2=1.43877e-2;   // Km
  double NUc,a,b,t;
  IRCAL1 ic;
  ic=ircal.ircal[irn];
  NUc=ic.NUc;
  a=ic.alpha;
  b=ic.beta;
  if (R<=0) return 0;
  t=(C2*NUc*100.)/(a*log((C1*pow(NUc,3)*1000000.)/(R/100000.) + 1)) - b;
  return (float)t;
}

/* Translate pix-value in range 0...255 to temp.
  using GOES mode-A (dog-leg) scaling for temperature
  see:
    http://goes.gsfc.nasa.gov/text/imager.calibration.html#modeA

Scene Temperatures to Mode-A (8-bit) Counts
NOAA-NESDIS uses the following two-ramp dog-leg scheme for converting scene temperatures 
into an unsigned 8-bit integer in the range 0 to 255:

First, clip all scene temperatures to be in the range 163K to 330K
Then
     Mode-A (counts) = 418K - Tscene     (for Tscene between 163K and 242K)
     Mode-A (counts) = 660K - 2*Tscene   (for Tscene between 242K and 330K)

This inverts the intensity -- 330K corresponds to zero counts, while 163K corresponds to 255 counts. 
It also maps the warmest 88K into the first 176 counts (0.5K/count), 
and maps the coldest 80K into the last 80 counts (1.0K/count). 
Consequently, typical scene temperatures between 220K and 300K map into middle-grey levels 
between 200 and 60 counts, respectively.

*/
float val2temp(int pix)
{
  float T;
  if (pix < 176)
    T=(660-pix)/2.;
  else
    T=418-pix;
  return T;
}


int temp2val(float T)
{
  int pix;
  if (T > 242.)
    pix=660-2*T;
  else
    pix=418-T;

  return pix;
  
}


float pix2temp(SEGMENT *pro,CHANNEL *chan,int pix)
{
  float R,T;
  int channr=chan->chan_nr;
  /* If file from MDM assume GOES mode-A (dog-leg) scaling for temperature */
  if ((chan->segm) && (chan->segm->xh.xrit_frmt==STDFRMT))
  {
    return (float)val2temp(pix);
  }
  if ((pro) && (pro->chan->Cal_Offset[channr-1]) && (pro->chan->Cal_Slope[channr-1]))
  {
    if (channr<4) return 0.; // VIS channel
    R=pro->chan->Cal_Offset[channr-1]+pro->chan->Cal_Slope[channr-1]*pix;
    T=rad2temp(channr-4,R);
  }
  else if (chan->cal.caltbl[0])
  {
    int i;
    T=0;
    for (i=0; i<chan->cal.nrcalpoints; i++)
    {
      if (chan->cal.caltbl[0][i]>=pix)
      {
        if (i)
          T=chan->cal.caltbl[1][i-1]+((float)pix-chan->cal.caltbl[0][i-1])*(chan->cal.caltbl[1][i]-chan->cal.caltbl[1][i-1])/
                                       (chan->cal.caltbl[0][i]-chan->cal.caltbl[0][i-1]);
        else
          T=chan->cal.caltbl[1][i];
        break;
      }
    }
//printf("%d  %f   %f  %f\n",pix,T,chan->cal.caltbl[1][i-1],chan->cal.caltbl[1][i]);
  }
#ifdef OUD
  else if (chan->caltbl)
  {
    T=0;
    if (chan->cal.caltbl_bpp>8)
    {
      if (chan->caltbl[0] != chan->caltbl[1])
      {
        T=chan->caltbl[0]+(float)pix*(chan->caltbl[1]-chan->caltbl[0])/1000.;
      }
    }
    else
    {
      if ((pix<256) && (chan->caltbl[pix]))
      {
        T=chan->caltbl[pix];
      }
    }
//printf("T2=%f\n",T);
  }
#endif
  else
  {
    return 0.;    // no IR cal. available
  }
  return T;
}


/* Map -60C ...50C = 213K ... 323K to blue...red. */
#define TMIN -40
#define TMAX 60
#define T3RNG ((TMAX-TMIN)/3)
void temp_clrmap(float t,guint16 *pix)
{
  pix[0]=0; pix[1]=0; pix[2]=0;
  if (t<=3.) return;
  t-=273;
  t=MIN(t,TMAX-1);
  t=MAX(t,TMIN);
  
  if (t<TMIN+T3RNG)
  {
    pix[0]=0;
    pix[1]=255*(t-TMIN)/T3RNG;
    pix[2]=255-(pix[0]+pix[1])/2;
  }

  else if (t<TMIN+2*T3RNG)
  {
    pix[0]=255*(t-TMIN-T3RNG)/T3RNG;
    pix[1]=255;
    pix[2]=255-(pix[0]+pix[1])/2;
  }
  else
  {
    pix[0]=255;
    pix[1]=255-(255*(t-TMIN-2*T3RNG))/T3RNG;
    pix[2]=255-(pix[0]+pix[1])/2;
  }
  return;
}
/**********************************************
 * Convert 1 line into anaglyph.
 **********************************************/
void conv_to_3d(guint16 *rgbstr[4],guint16 *xlum,int xmax,int pixvalmax,char scandir,ANAGL anagl)
{
  guint16 rgbsav[4][MAX_3DSHIFT];
  int lum,dx,xs,xsd_l,xsd_r,x;
  anagl.shift_3d=MIN(anagl.shift_3d,MAX_3DSHIFT);
  memset(rgbsav,0,sizeof(rgbsav));
  if (anagl.shift_3d==0) return;
  for (x=0; x<xmax; x++)
  {
    if (xlum)
      lum=xlum[x];
    else 
      lum=(rgbstr[0][x]+rgbstr[1][x]+rgbstr[2][x])/3;

/* For anaglyph setting */
    if (anagl.init)
    {
      if (lum<anagl.lmin) rgbstr[1][x]=rgbstr[2][x]=0;  // red if lower than lbnd
      if (lum>anagl.lmax) rgbstr[0][x]=rgbstr[2][x]=0;  // green if higher than hbnd
      continue;
    }

/* Clip lum between min and max; convert between 0 and pixvalmax */
    if (anagl.lmax<=anagl.lmin) anagl.lmax=anagl.lmin+1;
    lum=MIN(lum,anagl.lmax);
    lum=MAX(lum,anagl.lmin);
    lum=((lum-anagl.lmin)*pixvalmax)/(anagl.lmax-anagl.lmin);

/* calc lum dependent shift. lum=pixvalmax -> dx=0, lum=0 -> dx=anagl.shift_3d */
    dx=((pixvalmax-lum)*anagl.shift_3d)/pixvalmax;

/* clip dx between 0 and anagl.shift_3d */
    dx=MIN(dx,anagl.shift_3d-1);
    dx=MAX(dx,0);
    
/* Make xs=x modulo shift_3d */
    xs=x%anagl.shift_3d;

/* determine left/right position of current pixel;
   take into account north bound / south bound 
*/
    if (scandir=='n')
    {
      xsd_r=(xs-dx+anagl.shift_3d)%anagl.shift_3d;
      xsd_l=xs;
    }
    else
    {
      xsd_l=(xs-dx+anagl.shift_3d)%anagl.shift_3d;
      xsd_r=xs;
    }

/* cyclic save of x value; remove overlay */
    rgbsav[0][xs]=rgbstr[0][x];
    rgbsav[1][xs]=rgbstr[1][x];
    rgbsav[2][xs]=rgbstr[2][x];
    rgbsav[3][xs]=rgbstr[3][x];
    if (rgbsav[3][xs]&CMASK) rgbsav[3][xs]|=CLMASK;

/* restore and shift */
    rgbstr[0][x]=rgbsav[0][xsd_l];
    rgbstr[1][x]=rgbsav[1][xsd_r];
    rgbstr[2][x]=rgbsav[2][xsd_r];
    rgbstr[3][x]=(rgbsav[3][xsd_l]&CLMASK) | (rgbsav[3][xsd_r]&(~CLMASK));
  }
}

typedef struct firedet_vals
{
  int th1;
  int th2;
  int th3;
  int th4[2];
  int th5[2];
  int th6;
} FIREDETVALS;
/*
T3<315K or NDVIdiff > mean+1*s.d ==> No fire
T3-T4<14K ==> Warm background
T4<260K ==> Cloud and bright surface
T4-T5>=4K and (T3-T4)<=19K ==> Thin clouds with warm background
R1+R2 >= 75% and R2 >=30% ==> Cloud and bright surface
|R1-R2| <= 1% ==> sunlight

Else: Fire.
*/
static gboolean fire_filter(GROUP *grp,SEGMENT *segm[5],int x,int spos,int dshft)
{
  CHANNEL *ch[5];
  guint16 pix[6];
  int i;
  FIREDETVALS ft={315,14,260, {4,19}, {768,307}, 10};
//  ft.th6=30;
  if (!grp->pro_epi) return FALSE;
  
  for (i=0; i<5; i++)
  {
    if (!segm[i]) return FALSE;
    ch[i]=segm[i]->chan;
    if (x>ch[i]->nc) return FALSE;
    pix[i+1]=segm[i]->chnk[spos+x];         // get pix, remove overlay
  }
  pix[3]=pix2temp(grp->pro_epi->pro,ch[2],pix[3]);
  pix[4]=pix2temp(grp->pro_epi->pro,ch[3],pix[4]);
  pix[5]=pix2temp(grp->pro_epi->pro,ch[4],pix[5]);

  if (pix[3]        < ft.th1) return FALSE;
  if (pix[3]-pix[4] < ft.th2) return FALSE;
  if (pix[4]        < ft.th3) return FALSE;
  if ((pix[4]-pix[5] >= ft.th4[0]) && (pix[3]-pix[4] <= ft.th4[1])) return FALSE;
  if ((pix[1]+pix[2] >= ft.th5[0]) && (pix[2]>=ft.th5[1])) return FALSE;
  if ((ABS(pix[1]-pix[2])) <= ft.th6) return FALSE;

  return TRUE;
}

void fire_filter_line(GROUP *grp,int y1,int ncmaxext,int xmax,int ymax,guint16 *rgbstr[4])
{
  CHANNEL *chan,*chan1=NULL;
  SEGMENT *segm[5];
  int spos,dshft=0;
  gboolean has_hrv=FALSE;
  int x;
  memset(segm,0,sizeof(segm));
  
  for (chan=grp->chan; chan; chan=chan->next)
  {
    if (!strcmp(chan->chan_name,"VIS006")) // noaa: 0.65
    {
      if (!chan1) chan1=chan;
      if (!(segm[0]=get_linefromsegm(chan,y1,has_hrv,ymax,&spos,&dshft,&grp->pc))) return;
      if (!segm[0]->chnk) return;
    }
    if (!strcmp(chan->chan_name,"VIS008"))  // noaa: 0.86
    {
      if (!chan1) chan1=chan;
      if (!(segm[1]=get_linefromsegm(chan,y1,has_hrv,ymax,&spos,&dshft,&grp->pc))) return;
      if (!segm[1]->chnk) return;
    }
    if (!strcmp(chan->chan_name,"IR_039"))  // noaa: 3.8
    {
      if (!chan1) chan1=chan;
      if (!(segm[2]=get_linefromsegm(chan,y1,has_hrv,ymax,&spos,&dshft,&grp->pc))) return;
      if (!segm[2]->chnk) return;
    }
    if (!strcmp(chan->chan_name,"IR_108"))  // noaa: 10.8
    {
      if (!chan1) chan1=chan;
      if (!(segm[3]=get_linefromsegm(chan,y1,has_hrv,ymax,&spos,&dshft,&grp->pc))) return;
      if (!segm[3]->chnk) return;
    }
    if (!strcmp(chan->chan_name,"IR_120"))  // noaa: 11.9
    {
      if (!chan1) chan1=chan;
      if (!(segm[4]=get_linefromsegm(chan,y1,has_hrv,ymax,&spos,&dshft,&grp->pc))) return;
      if (!segm[4]->chnk) return;
    }
  }
  if (!chan1) return;

  for (x=0; x<ncmaxext; x++)                // copy line
  {
    int xx;
    xx=x*chan1->ncext/ncmaxext;            // offset in chunck for this x
    if (chan1->scan_dir=='n')
    if (chan1->ncext!=ncmaxext) xx=xx+chan1->ncext-(chan1->ncext*xmax)/ncmaxext; //    3712-2560;

    if (xx<0) break;
    if (xx>chan1->nc) break;

    if (fire_filter(grp,segm,xx,spos,dshft))
    {
      if (dshft+x>=xmax) continue;
      rgbstr[3][dshft+x]|=FMASK;
    }
  }
}
