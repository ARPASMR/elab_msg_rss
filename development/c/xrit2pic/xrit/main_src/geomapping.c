/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 * Geographic Mappings for geostat. and polar satellites.
 *
 * Ext. funcs:
 *   int avhrr_linearize(int x,int avhrr_picwidth)
 *   int inv_ymap(int y,float *alpha,PIC_CHARS *pc)
 *   int inv_xmap(int x,float alpha,PIC_CHARS *pc)
 *   int dxy2fxy(int xi,int yi,int *xo,int *yo,PIC_CHARS *pc)
 *   int glonlat2xy(float lon,float lat,int *c,int *l,GEOCHAR *gchar)
 *   int glonlat2xy_xcorr(float lon,float lat,int *c,int *l,GEOCHAR *gchar)
 *   void gxy2lonlat(int c,int l,float *lon,float *lat,GEOCHAR *gchar)
 *   int fxy2lonlat(int x,int y,float *lon,float *lat,PIC_CHARS *pc)
 *   int lonlat2fxy(float lon,float lat,int *x,int *y,PIC_CHARS *pc)
 *   void dxy2lonlat(int x,int y,float *lon,float *lat,PIC_CHARS *pc)
 *   void dxy2lonlat_area(int x,int y,int xoff,int yoff,int xfac,int yfac,float *lon,float *lat,PIC_CHARS *pc)
 *   int lonlat2dxy(float lon,float lat,int *x,int *y,PIC_CHARS *pc)
 *   
 *
 ********************************************************************/
#include "xrit2pic.h"
#include "avhrr.h"
#include <math.h>
#include <stdlib.h>
extern GLOBINFO globinfo;

#define OFFS 30

// min/max for geostat. 
#define MAXLON D2R(75.)
#define MINLON D2R(-75.)
#define MAXLAT D2R(79.)
#define MINLAT D2R(-79.)

//#define MAXLON D2R(70.)
//#define MINLON D2R(-70.)
//#define MAXLAT D2R(70.)
//#define MINLAT D2R(-70.)

/*************************************************************************
 *************************************************************************
 ** HRPT linearize functions.
 *************************************************************************
 *************************************************************************/

/*************************************
 * Translate x-value to angle position on earth.
 * (angle between line "sat <--> middle point earth" and "pos. earth <--> mp"
 * Needed: sat_height     : height satellite
 *         x              : x-pos. (for HRPT: 0 ... 1023)
 *         xh             : half value of x (1023 for HRPT)
 *         max_sens_angle : max. angle sensor
 * If x=0  ==> alpha=max_sens_angle ==> beta=beta_max
 * if x=xh ==> alpha=0 ==> beta=0        
 *************************************/
// Zie ook sat_orbit.c: te combineren!
static float x2earthangle2(float sat_height,            /* height sat */
                   int x,int xh,                 
                   float max_sens_angle)
{
  float alpha,beta,tmp1,tmp2;

/* angle sensor in satellite */
  alpha=(xh-x)*D2R(max_sens_angle)/xh;

/* translate into radius angle */
  tmp1=(sat_height+Rearth)/Rearth;
  tmp2=(tmp1*tmp1*sin(alpha)*sin(alpha));
  beta=acos(tmp1*sin(alpha)*sin(alpha)+
                 cos(alpha)*sqrt(1-tmp2));
  if (alpha<0) beta*=-1;
  return beta;
}

/*************************************
 * Generate table to linearize HRPT
 * Needed: sat_height     : height satellite
 *         max_sens_angle : max. angle sensor
 * Return: pointer to table; table[x]=actual_x
 *************************************/
static int *mk_lintbl(PICINFO  *pci,int expand_l)
{
  int *xtbl,*lintbl;

  int x;
  float max_sens_angle=pci->max_sens_angle; /* angle */
  float height=pci->height;
  float beta,beta_max;

  int pox=pci->ox;
  int width=pci->width;

  int i_width=pci->width_original*expand_l;
  int i_hwidth=i_width/2;
  if (i_hwidth*2<i_width) i_hwidth++;
  
  if (!pci->width_original) return NULL;
/* If no height is specified: Make it 800 km. */
  if (!height) height=800000;

  if ((lintbl=(int *)calloc(i_width,sizeof(*lintbl)))==NULL) return NULL;
  if ((xtbl=(int *)calloc(i_width,sizeof(*xtbl)))==NULL) return NULL;

/* Calculate max. angle projected on earth surface. */
  beta_max=x2earthangle2(height,0,1,max_sens_angle);

/*
  Create lin. table.
  First expand, to prevent loss of pixels:
    0 1 2 3 4... ==> 0 0 1 1 2 2 3 3 4 4
  Then calulate and load x-mapping in lintbl:
    0 0 0 0 1 1 1 1 ... 1019 1020 1021 1022 1023 ... 2047 2047 2047 2047 
*/
  {
    int lxp=0,ilx,lx;

    lintbl[0]=0;
    lintbl[i_width-1]=pci->width_original-1;

/* calculate for half width; second half is same */
    for (x=0; x<i_hwidth; x++)
    {
/* Angle point_1024 -> center_earth -> point_x */
      beta=x2earthangle2(height,x,i_hwidth,max_sens_angle);

/* lx runs from 0 (beta=beta_max) to 2047 (beta=0) */ 
      lx=(i_hwidth-1)-(int)((i_hwidth-1)*beta/beta_max);
      for (ilx=lxp; ilx<=lx; ilx++)
      {
        lintbl[ilx]=x/expand_l;
        lintbl[i_width-1-ilx]=pci->width_original-1-lintbl[ilx];
      }
      lxp=lx+1;
    }
  }

  if (pox+width > pci->width_original) pox=pci->width_original-width; 
  pox*=expand_l;

/* Load x-mapping. Picture-offset 'pci->ox' is taken into account here. 
   Mapping to xtbl is done such that xtbl[0]=0 and xtbl[width-1]=width-1;
   So, some resolution in the middle gets lost!
   Load inversed-table too.
*/
  for (x=0; x<i_width; x++)
  {
    int xlin;
    if (x+pox > ((pci->width_original*expand_l)-1)) break;

    xlin=((long)(lintbl[x+pox]-lintbl[pox])*(long)(width-1)) /
                        (long)(lintbl[expand_l*(width-1)+pox]-lintbl[pox]);

    xtbl[x]=xlin;
  }

  free(lintbl);


  return xtbl;
}

static int *ld_hrptlintbl(int iwidth,int expand_l)
{
  int *xtbl;
  PICINFO pci;
  pci.width=iwidth;
  pci.height=800000;
  pci.width_original=iwidth;
  pci.ox=0;
  pci.max_sens_angle=DEF_NOAA_ALPHAMAX;
  xtbl=mk_lintbl(&pci,expand_l);
  return xtbl;
}

int avhrr_linearize(int x,int avhrr_picwidth)
{
  static int *xtbl;
  static int avhrr_picwidth_sav;
  if ((avhrr_picwidth!=avhrr_picwidth_sav) && (xtbl))
  {
    free(xtbl); xtbl=NULL;
  }
  if (!xtbl) xtbl=ld_hrptlintbl(avhrr_picwidth,1);
  avhrr_picwidth_sav=avhrr_picwidth;

  if ((x>=0) && (x<avhrr_picwidth))
    x=xtbl[x];
  return x;
}

// Uh.. Vervangen door nxy2lonlat?
static void pxy2lonlat(POINT *pos,GEOMAP gmap,int xoff,int yoff,int xfac,int yfac,int x,int y,int height)
{
  float lon,lat;
  switch (gmap)
  {
    case normal: 
    // not supported
    break;
    case plate_carree:
      y=height-y-1;
      lon=(float)x/(float)xfac-xoff;
      lat=(float)y/(float)yfac-yoff;
    break;
    case mercator:
      y=height-y-1;
      lon=(float)x/(float)xfac-xoff;
      lat=(float)y/(float)yfac-yoff;
      lat=(-90.)*log(tan((float)D2R(90-lat)/2.))/PI;
    break;
    case polar_n: case polar_s:
    case polar_cn: case polar_cs: // aanpassen?
    {
      int d=(gmap==polar_n? 1 : -1);
      float xp,yp,r;
      
      xp=((float)x/(float)xfac)-xoff;
      yp=((float)y/(float)yfac)-yoff;
      r=sqrt(xp*xp+yp*yp);
      lat=(90.-r)*d;
// was: acos(xp/r); is verkeerd bij polaire sat. (niet bij geo...?)
// Moet nog aangepast worden voor lon>90 en < -90
      lon=R2D(asin(-1.*xp/r)); if (yp>0) lon*=-1;
    }
  }
  pos->lon=D2R(lon);
  pos->lat=D2R(lat);
}

void lonlat2xyz(float lon,float lat,float *xyz)
{
  lon=D2R(lon);
  lat=D2R(lat);
  xyz[0]=sin(lon)*cos(lat);
  xyz[1]=cos(lon)*cos(lat);
  xyz[2]=sin(lat);
}

static float x2earthanglex(float sat_height,            /* height sat */
                   SCANDIR scan_hor,            /* scan direction sensor */
                   int x,int xh,                 
                   float max_sens_angle)
{
  float alpha,beta,tmp1,tmp2;

/* angle sensor in satellite */
  alpha=(xh-x)*D2R(max_sens_angle)/xh;
  if (scan_hor==W_E) alpha*=-1;

/* translate into radius angle */
  tmp1=(sat_height+Rearth)/Rearth;
  tmp2=(tmp1*tmp1*sin(alpha)*sin(alpha));
  beta=acos(tmp1*sin(alpha)*sin(alpha)+
                 cos(alpha)*sqrt(1-tmp2));
  if (alpha<0) beta*=-1;
  return beta;
}

void calc_x2satobsx(KEPLER *kepler,ORBIT *orbit,
                   POINT *pos_sati,POINT *pos_sato,
                   int width_original,float max_sens_angle,
                   int x)
{
  float beta;
  beta=x2earthanglex(orbit->height,E_W,
                    x,width_original/2,max_sens_angle);
printf("beta=%f\n",beta);
//  calc_satobs(kepler,orbit,pos_sati,pos_sato,beta);
}

#ifdef XXX
static float dist(float *xyz,float *xyz1)
{
  float d;
  d=(((xyz[0]-xyz1[0])*(xyz[0]-xyz1[0])) +
     ((xyz[1]-xyz1[1])*(xyz[1]-xyz1[1])) +
     ((xyz[2]-xyz1[2])*(xyz[2]-xyz1[2])));
 return d;
}

#define SNEL
static int plonlat2xy(float lon,float lat,int *c,int *l,PIC_CHARS *pc)
{
  int x1,y1,x,y;
  float xyz[3],xyz1[3],d,d1;
  POINT pos_sati,pos_sato;
  if (!pc->ypos_tbl[0]) return 1;
  if (!pc->ypos_tbl[1]) return 1;
  lonlat2xyz(lon,lat,xyz);
  for (y=0; y<pc->height; y++)
  {
    lonlat2xyz(pc->ypos_tbl[0][y],pc->ypos_tbl[1][y],xyz1);
    d=dist(xyz,xyz1);
    if ((y==0) || (d<d1))
    {
      y1=y;
      d1=d;
      pos_sati.x=xyz1[0];
      pos_sati.y=xyz1[1];
      pos_sati.z=xyz1[2];
    }
//break;
  }
#ifdef SNEL
  lonlat2xyz(pc->ypos_tbl[0][y1],pc->ypos_tbl[1][y1],xyz1);
  {
    float alpha,cb;
    cb=dist(xyz,xyz1);
    x1=R2D(cb)/3.16*1024+1024;
printf("cb=%f  %f\n",R2D(cb),cb);
//    alpha=acos((pc->orbit.height+Rearth*(1-cb))/(sqrt(2*(1-cb)*Rearth*(Rearth+pc->orbit.height)+pc->orbit.height*pc->orbit.height)));
//    x1=pc->width/2-alpha*pc->width/2/D2R(DEF_NOAA_ALPHAMAX);
//int xx;
//    calc_x2satobsx(&pc->kepler,&pc->orbit,
//                   &pos_sati,&pos_sato,
 //                  2048,DEF_NOAA_ALPHAMAX,
 //                  &xx);
  }
#else
  for (x=0; x<pc->width; x++)
  {
// (xyz1, xyz1) ==> beta ==> x
    calc_x2satobs(&pc->kepler,&pc->orbit,
                   &pos_sati,&pos_sato,
                   2048,DEF_NOAA_ALPHAMAX,
                   x);
                   
    d=(((xyz[0]-pos_sato.x)*(xyz[0]-pos_sato.x)) +
       ((xyz[1]-pos_sato.y)*(xyz[1]-pos_sato.y)) +
       ((xyz[2]-pos_sato.z)*(xyz[2]-pos_sato.z)));
    if ((x==0) || (d<d1))
    {
      x1=x;
      d1=d;
    }
  }
#endif
  *c=x1;
  *l=y1;
  return 0;
}
#endif

static int plonlat2xy_area(POINT *pos,PIC_CHARS *pc,int xoff,int yoff,int xfac,int yfac,int *xo,int *yo)
{
  float lon=R2D(pos->lon);
  float lat=R2D(pos->lat);
  int oor=0;
  if ((!xfac) || (!yfac))
  {
    xoff=yoff=0;
    xfac=yfac=1;
  }
  switch (pc->gmap)
  {
    case normal:
    // not supported
    break;
    case plate_carree:
      *xo=(lon+xoff)*xfac;
      *yo=(lat+yoff)*yfac;
    break;
    case mercator:
      lat=R2D((PI/2.-(2.*atan(exp(PI*lat/-90.)))));
      *xo=(lon+xoff)*xfac;
      *yo=(lat+yoff)*yfac;
    break;
    case polar_n: case polar_s:
    case polar_cn: case polar_cs:  // aanpassen?
    {
      int d=(pc->gmap==polar_n? 1 : -1);
      float xp,yp,r;
      
      r=(90.-(d*lat));
      xp=r*sin(D2R(lon));
      yp=-1.*r*cos(D2R(lon));

      *xo=(xp+xoff)*xfac;
      *yo=(yp+yoff)*yfac;
    }
    break;
  }
  *yo=pc->height-1-*yo;
  if (*xo<0) { *xo=0; oor=1; }
  if (*xo>=pc->width) { *xo=pc->width-1; oor=1; }
  if (*yo<0) { *yo=0; oor=1; }
  if (*yo>=pc->height) { *yo=pc->height-1; oor=1; }
  return oor;  
}

/************************* End Polar *************************/

/************************* Next only used in conv_bnd *************************/
#define OFFS1 0
int inv_ymap(int y,float *alpha,PIC_CHARS *pc)
{
  int pheight=ABS(pc->y_s-pc->y_n);
  if ((pc->orbittype==GEO) && (pc->gmap==plate_carree))
  {
    *alpha=asin((float)(y-OFFS1)/(pheight-OFFS1*2)*2.-1);
    y=(*alpha)*(pheight/2.)/(PI/2.)+(pheight/2.);
  }
  return y;
  
}

int inv_xmap(int x,float alpha,PIC_CHARS *pc)
{
  return x;
}

/***********************************************************
 * Conversion coordinates geostat. sats normal mapping
 *  window                       file
 * [wx,wy] <=================> [fx,fy] <==================> [lon,lat]
 *            wx2px,wy2py -->               gxy2lonlat -->
 *         <--px2wx,py2wy               <-- glonlat2xy
 * 
 * Conversion coordinates geostat. sats special mapping
 *  window                       comp                                                        file
 * [wx,wy] <=================> [nx,ny] <==================> [lon,lat] <===================> [fx,fy] = [c,l]
 *            wx2px,wy2py -->               nxy2lonlat -->                 glonlat2xy  -->
 *         <--px2wx,py2wy               <-- lonlat2nxy                 <-- gxy2lonlat
 * 
 ***********************************************************/


/****************************************
 * Translate lon,lat
 *   to (xi,yi)
 *   using certain mapping
 *   (lon,lat) in radians
 ****************************************/
static int lonlat2nxy(float lon,float lat,int *xi,int *yi,PIC_CHARS *pc)
{
  float xp,yp;                 // range: [-1,1]
  lon-=D2R(pc->gchar.sub_lon);
  switch(pc->gmap)
  {
    case normal:
    // N.A.
    break;
    case plate_carree:
      xp=lon*2./PI;
      yp=lat*(-2.)/PI;
    break;
    case mercator:
      xp=lon*2./PI;
      yp=(log(tan((PI/2.-lat)/2.)))/PI;    // lat: -85 ... +85
    break;
    case polar_n: case polar_cn:
    {
      yp=sqrt((tan((PI/2.-lat)/2.)*tan((PI/2.-lat)/2.))/(tan(lon)*tan(lon)+1));
      xp=yp*tan(lon);
      if (pc->gmap==polar_n) yp=yp*2.-1;
    }
    break;

    case polar_s: case polar_cs:
    {
      yp=sqrt((tan((PI/2.+lat)/2.)*tan((PI/2.+lat)/2.))/(tan(lon)*tan(lon)+1));
      xp=yp*tan(lon);

      if (pc->gmap==polar_s) yp=1.-yp*2.;
    }
    break;
  }
  if (pc->scan_dir=='s')
  {
    *xi=(1.-xp)*pc->width/2.;
    *yi=(1.-yp)*pc->height/2.;
  }
  else
  {
    *xi=(1.+xp)*pc->width/2.;
    *yi=(1.+yp)*pc->height/2.;
  }

  if ((*xi<0) || (*xi>=pc->width)) return 1;
  if ((*yi<0) || (*yi>=pc->height)) return 1;
  if (pc->orbittype==GEO)
  {
    if ((lon<-PI/2.) || (lon>PI/2.)) return 1;
    if ((lon<MINLON) || (lon>MAXLON)) return 1; 
    if ((lat<MINLAT) || (lat>MAXLAT)) return 1; 
  }
  return 0;
}

/****************************************
 * Translate (xi,yi) on drawable
 *   to lon,lat
 *   using certain mapping
 ****************************************/
static void nxy2lonlat(int xi,int yi,float *lon,float *lat,PIC_CHARS *pc)
{
  float xp, yp;
  if (pc->scan_dir=='s')
  {
    xp=1.-(xi*2./pc->width);               // normalize to [1...-1]
    yp=1.-(yi*2./pc->height);              // normalize to [1...-1]
  }
  else
  {
    xp=-1.+(xi*2./pc->width);               // normalize to [1...-1]
    yp=-1.+(yi*2./pc->height);              // normalize to [1...-1]
  }

  // Determine position in lon/lat
  switch(pc->gmap)
  {
    case normal:
    // N.A.
    break;
    case plate_carree:
      *lon=xp * PI/2.;                     // lon: +90 ... -90
      *lat=(yp * PI/2.)*-1.;               // lat: -90 ... +90
    break;
    case mercator:
      *lon=xp * PI/2.;                     // lon: +90 ... -90
      *lat=PI/2.-(2.*atan(exp(PI*yp)));    // lat: -85 ... +85
    break;
    case polar_n: case polar_cn:
      if (pc->gmap==polar_n) yp=(yp+1)/2.;   // N: 1...0 

      *lon=atan2(xp,yp);
      *lat=((PI/2.) - 2.*atan(sqrt(xp*xp+yp*yp)));
    break;
    case polar_s: case polar_cs:
      if (pc->gmap==polar_s) yp=(yp-1)/2.;     // S: 0...-1

      *lon=atan2(xp,-1.*yp);
      *lat=((PI/-2.) + 2.*atan(sqrt(xp*xp+yp*yp)));
    break;
  }

  *lon=MAX(MIN(*lon,PI/2.),PI/-2.);             // clip (lon,lat)
  *lat=MAX(MIN(*lat,PI/2.),PI/-2.);
  *lon+=D2R(pc->gchar.sub_lon);                 // add sub-satellite longitude
}

#define PIDIV2 (PI/2.)
/****************************************
 * Translate visible ("screen")-coord. (xi,yi)
 *   to coord. in file (xo,yo)
 *   using certain mapping
 ****************************************/
int dxy2fxy(int xi,int yi,int *xo,int *yo,PIC_CHARS *pc)
{
  int oor=0;
//  int pheight=ABS(pc->y_s-pc->y_n);// niet nodig?
  float lon,lat; 

  if (pc->orbittype==GEO)
  {
    // Determine x/y position
    if (pc->gmap==normal)
    {
      *xo=xi; *yo=yi;
    }
    else        // use calculated lon/lat for x/y
    {
      nxy2lonlat(xi,yi,&lon,&lat,pc);                   // (x,y) rel. pos. in file to (lon,lat)
                                                        // used as if (x,y)=pos. in window
      oor=glonlat2xy(lon,lat,xo,yo,&pc->gchar);         // calc. (x,y) in file from (lon,lat)
    }
  }
  else // meaning: if (pc->orbittype==POLAR)
  {

    *xo=xi; *yo=yi;
    if (pc->gmap==normal)
    {
      *yo=yi;
      if (pc->avhrr_lin)
        *xo= avhrr_linearize(xi,pc->width);
      else
        *xo=xi;
    }
    else         // uses generated pic! See gen_polarearth
    {
      *xo=xi;
      *yo=yi;
    }
  }
  
  return oor;
}


/*************************************************
 *************************************************
 * Geostat. translations between (x,y) and (lon,lat).
 *************************************************
 *************************************************/

static int geo_xshiftcorrect(int c,int l,GEOCHAR *gchar)
{
  if (gchar)
  {
    if (l>gchar->shift_ypos)
    {
      c=c-gchar->upper_x_shift;
    }
    else
    {
      c=c-gchar->lower_x_shift;
    }
  }
  return c;
}

// Doc.: LRIT_HRIT_Global_Spec.pdf
/* Conversion lon/lat to x/y for MSG */
int glonlat2xy(float lon,float lat,int *c,int *l,GEOCHAR *gchar)
{
  int oor=0;
  float rpol= 6356.7523;
  float req = 6378.1370;
  float dv  =42142.5833;
  float sub_lon=0;
  
  float rep=(rpol*rpol)/(req*req);

  float re;
  float psi;
  float r1,r2,r3,rn,x,y;
  int coff=0;
  int cfac=0;
  int loff=0;
  int lfac=0;

  if (gchar)
  {
    coff=gchar->coff;
    cfac=gchar->cfac;
    loff=gchar->loff;
    lfac=gchar->lfac;
    sub_lon=D2R(gchar->sub_lon);
  }
  lon-=sub_lon;
  if (lon < MINLON) oor=1;
  if (lon > MAXLON) oor=1;
  if (lat < MINLAT) oor=1;
  if (lat > MAXLAT) oor=1;
  lon=MIN(MAX(lon,MINLON),MAXLON);
  lat=MIN(MAX(lat,MINLAT),MAXLAT);

  if (!coff) coff=1856;      // 0x740;
  if (!cfac) cfac=-13642337; // 0xff2fd59f;
  if (!loff) loff=1856;      // 0x740;
  if (!lfac) lfac=-13642337; // 0xff2fd59f;

  psi=atan(rep * tan(lat));
  re=rpol/(sqrt(1-(1-rep)*cos(psi)*cos(psi)));
  r1=dv - re*cos(psi)*cos(lon);
  r2=-1.*re*cos(psi)*sin(lon);
  r3=re*sin(psi);
  rn=sqrt(r1*r1+r2*r2+r3*r3);
  x=atan(-1.*r2/r1);
  y=asin(-1.*r3/rn);
  x=R2D(x);
  y=R2D(y);
  *c=coff+(int)(x*cfac/0x10000);
  *l=loff+(int)(y*lfac/0x10000);

  return oor;
}

/* glonlat2xy also for HRV */
int glonlat2xy_xcorr(float lon,float lat,int *c,int *l,GEOCHAR *gchar)
{
  int oor;
  oor=glonlat2xy(lon,lat,c,l,gchar);
  *c=geo_xshiftcorrect(*c,*l,gchar);
  if (*c<0) oor=1;
  return oor;
}

// Doc.: LRIT_HRIT_Global_Spec.pdf
/* Conversion x/y to lon/lat for MSG */
void gxy2lonlat(int c,int l,float *lon,float *lat,GEOCHAR *gchar)
{
  float rpol= 6356.7523;
  float req = 6378.1370;
  float dv  =42142.5833;
  float q2=(req*req)/(rpol*rpol);
  float d2=dv*dv-req*req;
  float sub_lon=0.;
  float a1,a2;
  float cosx,cosy,sinx,siny;
  float s1,s2,s3,sn,sd,sxy;
  float x,y;

  int coff=0;
  int cfac=0;
  int loff=0;
  int lfac=0;

  if (gchar)
  {
    coff=gchar->coff;
    cfac=gchar->cfac;
    loff=gchar->loff;
    lfac=gchar->lfac;
    sub_lon=D2R(gchar->sub_lon);
  }
  if (!coff) coff=1856;      // 0x740;
  if (!cfac) cfac=-13642337; // 0xff2fd59f;
  if (!loff) loff=1856;      // 0x740;
  if (!lfac) lfac=-13642337; // 0xff2fd59f;
  
  x=(float)(c-coff)*(float)0x10000/(float)cfac;
  y=(float)(l-loff)*(float)0x10000/(float)lfac;
/*
MSG normal:
South  : l=0   , l-loff=-1856
equator: l=1856, l-loff=0
North  : l=3711, l-loff=+1856
*/
  cosx=cos((float)(D2R(x)));
  cosy=cos((float)(D2R(y)));
  sinx=sin((float)(D2R(x)));
  siny=sin((float)(D2R(y)));
  a1=dv*dv*cosx*cosx*cosy*cosy;
  a2=(cosy*cosy+q2*siny*siny)*d2;
  if (a1>=a2)
    sd=sqrt( a1 - a2 );
  else
  {
    *lon=*lat=0;
    return;
  }

  sn=(dv*cosx*cosy-sd)/(cosy*cosy+q2*siny*siny);
  s1=dv-sn*cosx*cosy;
  s2=sn*sinx*cosy;
  s3=-1.*sn*siny;
  sxy=sqrt(s1*s1+s2*s2);
  if (s1==0)
  {
    if (s2>0)
      *lon=D2R(90)+sub_lon;
    else
      *lon=D2R(-90)+sub_lon;
  }
  else
  {
    *lon=atan(s2/s1)+sub_lon;
  }

  if (sxy==0)
  {
    if (q2*s3>0)
      *lat=D2R(90);
    else
      *lat=D2R(-90);
  }
  else
  {
    *lat=atan(q2*s3/sxy);
  }
}



/*************************************************
 *************************************************
 * Higher level translations between (x,y) and (lon,lat).
 * If possible use only these!
 *************************************************
 *************************************************/

/***** Translation between [lon,lat] and [x,y] in file. *****/


/*  [x,y] ==> [lon,lat]  ([x,y] in orig. file) */
int fxy2lonlat(int x,int y,float *lon,float *lat,PIC_CHARS *pc)
{
  int ok=1;
  if (pc->orbittype==POLAR)
  {
    POINT pos;
    ok=xy2lonlat(x,y,pc,&pos);
    *lon=R2D(pos.lon);
    *lat=R2D(pos.lat);
  }
  else
  {
    gxy2lonlat(x,y,lon,lat,&pc->gchar); 
    *lon=R2D(*lon);
    *lat=R2D(*lat);
  }
  return ok;
}

/* [lon,lat] ==> [x,y]  ([x,y] in orig. file; HRV-shifts taken into account) */
int lonlat2fxy(float lon,float lat,int *x,int *y,PIC_CHARS *pc)
{
  int oor;
  if (pc->orbittype==POLAR)
  {
//    oor=plonlat2xy(lon,lat,x,y,pc);
    oor=1;
  }
  else
  {
    oor=glonlat2xy_xcorr(D2R(lon),D2R(lat),x,y,&pc->gchar); 
  }
  return oor;
}

/***** Translation between [lon,lat] and [x,y] in drawable. *****/

/*  [x,y] ==> [lon,lat]  ([x,y] in drawable) */
void dxy2lonlat(int x,int y,float *lon,float *lat,PIC_CHARS *pc)
{
  if (pc->gmap==normal)
  {
    if ((pc->orbittype==POLAR) && (pc->avhrr_lin))
      x= avhrr_linearize(x,pc->width);
    fxy2lonlat(x,y,lon,lat,pc);
  }
  else
  {
    nxy2lonlat(x,y,lon,lat,pc);
    *lon=R2D(*lon);
    *lat=R2D(*lat);
  }
}

void dxy2lonlat_area(int x,int y,
                     int xoff,int yoff,int xfac,int yfac,
                     float *lon,float *lat,
                     PIC_CHARS *pc)
{
  if ((pc->orbittype==POLAR) && (pc->gmap!=normal))
  {
    POINT pos; 
    pxy2lonlat(&pos,pc->gmap,xoff,yoff,xfac,yfac,x,y,pc->height);
    *lon=R2D(pos.lon);    
    *lat=R2D(pos.lat);    
  }
  else
  {
    dxy2lonlat(x,y,lon,lat,pc);
  }
}

/* [lon,lat] ==> [x,y]  ([x,y] in drawable) */
static int lonlat2dxy1(float lon,float lat,int *x,int *y,PIC_CHARS *pc)
{
  int oor=0;
  if (pc->gmap==normal)
  {
    if (pc->orbittype==POLAR)
    {
//    oor=plonlat2xy(lon,lat,x,y,pc);
      oor=1;
    }
    else
    {
      oor=glonlat2xy(D2R(lon),D2R(lat),x,y,&pc->gchar); 
    }
  }
  else
  {
    oor=lonlat2nxy(D2R(lon),D2R(lat),x,y,pc);
  }
  return oor;
}

static int lonlat2fxy1(float lon,float lat,int *x,int *y,PIC_CHARS *pc)
{
  int oor=0;
  if (pc->gmap==normal)
  {
    oor=lonlat2fxy(lon,lat,x,y,pc);  // (x,y) in file = (x,y) in drawable
  }
  else
  {
    oor=lonlat2nxy(D2R(lon),D2R(lat),x,y,pc);
  }
  return oor;
}

int lonlat2dxy(float lon,float lat,int *x,int *y,PIC_CHARS *pc)
{
  int oor=0;
  if ((pc->orbittype==POLAR) && (pc->gmap!=normal))
  {
    POINT pos; 
    pos.lon=D2R(lon); pos.lat=D2R(lat);
    oor=plonlat2xy_area(&pos,pc,pc->xoff,pc->yoff,pc->xfac,pc->yfac,x,y);
  }
  else
  {
    oor=lonlat2dxy1(lon,lat,x,y,pc);
  }
  return oor;
}

// Dit is lonlat naar x/y in File voor HRV!
int lonlat2dxy_bekijk(float lon,float lat,int *x,int *y,PIC_CHARS *pc)
{
  int oor=0;
  if ((pc->orbittype==POLAR) && (pc->gmap!=normal))
  {
    POINT pos; 
    pos.lon=D2R(lon); pos.lat=D2R(lat);
    oor=plonlat2xy_area(&pos,pc,pc->xoff,pc->yoff,pc->xfac,pc->yfac,x,y);
  }
  else
  {
    oor=lonlat2fxy1(lon,lat,x,y,pc);
  }
  return oor;
}

#ifdef XXX
// alleen scat en gen_polarearth
// alleen polar spec. mapping kan beperkte lon/lat bereik hebben; dit wordt gecompenseerd met xoff/xfac enz.
static int lonlat2dxy_area(float lon,float lat,
                    int xoff,int yoff,     // offset lon/lat
                    int xfac,int yfac,     // factor to map lon,lat to x,y
                    int *x,int *y,
                    PIC_CHARS *pc)
{
  int oor=0;
  if ((pc->orbittype==POLAR) && (pc->gmap!=normal))
  {
    POINT pos; 
    pos.lon=D2R(lon); pos.lat=D2R(lat);
    oor=plonlat2xy_area(&pos,pc,xoff,yoff,xfac,yfac,x,y);
  }
  else
  {
    oor=lonlat2dxy1(lon,lat,x,y,pc);
  }
  return oor;
}
#endif
