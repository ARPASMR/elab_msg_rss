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
 * Copyright (C) 2007 R. Alblas 
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
    int load_kepler(GROUP *grp)
 **************************************************/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "xrit2pic.h"
#include "avhrr.h"
#include "eps.h"

extern PREFER prefer;
/*************************************************************************
 *************************************************************************
 ** Kepler / Orbit functions.
 *************************************************************************
 *************************************************************************/
/*************************************
 * Extract number defined between positions 's' and 'e' from  l
 * and return as float
 *************************************/
static float strpart2float(char *l,int s,int e)
{
  char tmp[100];
  strncpy(tmp,l+s,e-s+1);
  tmp[e-s+1]=0;
  return atof(tmp);
}

/*
Format:
METOP-A
                  epoch_day        decay
1 29499U 06044A   06292.95942425  .00072135  00000-0  33535-1 0 41

         incl     raan    eccentr  perigee  anomaly motion      epoch_rev
2 29499  98.7336 350.5230 0001346 343.4023 016.7613 14.20096852 36

0         1         2         3         4         5         6
01234567890123456789012345678901234567890123456789012345678901234567890
*/

static void read_norad_kepler(char *l1,char *l2,KEPLER *kepler)
{
  char tmp[100];
/* Parse line 1 */
  kepler->epoch_year=(int)strpart2float(l1,18,19);
  if (kepler->epoch_year < 70) kepler->epoch_year+=100;

  kepler->epoch_day=strpart2float(l1,20,31);
  kepler->decay_rate=strpart2float(l1,33,42);

/* Parse line 2 */
  kepler->inclination=D2R(strpart2float(l2,8,15));
  kepler->raan=D2R(strpart2float(l2,17,24));

  sprintf(tmp,"0.%s",l2+26);
  kepler->eccentricity=strpart2float(tmp,0,32-26);

  kepler->perigee=D2R(strpart2float(l2,34,41));
  kepler->anomaly=D2R(strpart2float(l2,43,50));
  kepler->motion=strpart2float(l2,52,62);
  kepler->epoch_rev=strpart2float(l2,63,68);
}

/*************************************
 * Read next Kepler-info from file
 * File should be in NORAD 2 lines format.
 * Actually, there are 3 lines per sat; 
 * first line contains satellite name.
 *
 * Return -1 if failed (EOF)
 * Return diff-time in days if found (>=0)
 * If kepler!=NULL, then kepler-param. in this struct.
 *************************************/
static int read_norad_next_keps(FILE *fp,char *sat_name,struct tm *tm,KEPLER *kepler)
{
  char *l0,*l1,*l2,*p;
  KEPLER kepler1;
  int kepler_time,kepler1_time,sat_time=-1;
  if (tm)
  {
    sat_time=tm->tm_year*365+tm->tm_yday;
  }
  kepler_time=-1;
/* Allocate space for 3 lines */
  if (!(l0=calloc(101,1))) return -1;
  if (!(l1=calloc(101,1))) { free(l0); return -1; }
  if (!(l2=calloc(101,1))) { free(l0); free(l1); return -1; }

/* Search until a valid NOAA entry is found */
  while (fgets(l2,100,fp))
  {
    if (strlen(l2)>=99) continue;

    if ((*l1=='1') && (*l2=='2'))
    {
/* Remove trailing spaces in l0 (ignore trailing [S] etc.) */
      for (p=l0+strlen(l0)-1; p>l0; p--)
      {
        if ((isalnum(*p)) && (*(p+1)!=']'))
        {
          p++; *p=0;
          break;
        }
      }
      if (!strcmp(l0,sat_name))
      {
/* Now, l0 contains sat-name, l1 and l2 the kepler-data */
        read_norad_kepler(l1,l2,&kepler1);
        kepler1_time=kepler1.epoch_year*365+kepler1.epoch_day;

        if ((kepler_time<0) || (sat_time<0) ||
            (((ABS(kepler1_time-sat_time)) < (ABS(kepler_time-sat_time)))))
        {
          if (kepler) *kepler=kepler1;
          kepler_time=kepler1_time;
        }
        if (sat_time<0) break;
      }
    }
    p=l0; l0=l1; l1=l2; l2=p;
  }
  free(l0); 
  free(l1); 
  free(l2); 
  if (kepler_time<0) return -1;      // not found

  return ABS(kepler_time-sat_time);  // time-diff.: >=0
}

/*********************************************************
 * Read orbit-param. from file, with time closest to 'tm'.
 * Return: 
 * 0: File not found or orbit-param. of 'isat_name' ot found.
 * 1: Found.
 *********************************************************/

static int read_norad_keps_from_file(char *kepfile,char *isat_name,struct tm *tm,KEPLER *kepler)
{
  FILE *fpk;
  char sat_name[100];
  int dt;

  if (!(fpk=fopen(kepfile,"r"))) return -1; // can't open file

  strcpy(sat_name,isat_name);
/* Try to change name to what is used in norad files:
  NOAA18 --> NOAA 18
*/
  strtoupper(sat_name);
  if (!strncmp(sat_name,"NOAA",4))
  {
    sprintf(sat_name,"%s ","NOAA");
    strcat(sat_name,isat_name+4);
  }
  dt=read_norad_next_keps(fpk,sat_name,tm,kepler);

  fclose(fpk);

  return dt;
}

static int read_norad_keps(char *isat_name,struct tm tm,KEPLER *kepler)
{
  char kepfile[300];

  if (!(search_file(prefer.noradfile,kepfile,
              prefer.cur_dir,prefer.home_dir,prefer.prog_dir)))
    return -1;                            // file not found
  return read_norad_keps_from_file(kepfile,isat_name,&tm,kepler);
}


/**************************************************
  Used formulas to calculate the satellite orbit:

    Used symbols:
      h=height sat
      Rearth=radius earth
      g0=gravity
      to=time 1 loop (in sec)
      lpd=loops per day = 24*3600/to
      orbit_nr=# orbits since reference time (=time Kepler data)
      dt=time elapsed since reference time
      orbit_angle=remainder orbit (orbit_nr - n*2pi)
      k2_n=deviation orbit north-south
      k2_w=deviation orbit west-east

    Common physics:
      to=2*pi*sqrt(Rearth/g0) * (1+h/Rearth)**(1.5)

          { [         to           ](2/3)     }
      h = { [ -------------------- ]      - 1 } * Rearth
          { [ 2*pi*sqrt(Rearth/g0) ]          }


    Satellite movement:
                                      
                     [  Rearth  ](3.5)            2
      k2_n = Const * [ -------- ]      * {2.5*(cos(incl)-1) + 2}
                     [ h+Rearth ]

      dh_n=dt*k2_n

                     [  Rearth  ](3.5)            
      k2_w = Const * [ -------- ]      * cos(incl)
                     [ h+Rearth ]

      dh_w=dt*k2_w

      orbit_nr=(lpd + decay*dt)*dt

      orbit_angle=(orbit_nr-int(orbit_nr))*2*pi + perigee + anomaly

      x1=sin(orbit_angle+dh_n*dt)*cos(incl)
      y1=cos(orbit_angle+dh_n*dt)
      z1=sin(orbit_angle+dh_n*dt)*sin(incl)

  Take into account sensor movement.
  beta=angle "sat -> earth-middle -> current observation point"
      x=x1*cos(beta)+sin(inclination)*sin(beta);
      y=y1*cos(beta);
      z=z1*cos(beta)-cos(inclination)*sin(beta);

   Translate to lon/lat
      sat_lon=atan(y/x)+raan-dh_w
      sat_lat=asin(z)

    Earth rotation:
      rem_e=(rot_year-int(rot_year))*2*pi
      earth_lon=rem_e + Earth_offsetangle
      earth_lat=0;

    Relative rotation. If beta=0 then this is the sub-satellite point.
      rel_lon=sat_lon-earth_lon
      rel_lat=sat_lat

 **************************************************/
 
 /*************************************
 * Pre-calculate some orbit constants
 *************************************/
static int calc_orbitconst(KEPLER *kepler,ORBIT *orbit)
{
  if (kepler->motion<0.1) return 0;

  orbit->loop_time=24.*3600./kepler->motion;
  orbit->height=(pow(orbit->loop_time/PIx2/sqrt(Rearth/G0),2./3.)-1)*Rearth;

  orbit->k2_n=Const * pow(Rearth/(Rearth+orbit->height),3.5) * 
                          (2.5*pow(cos(kepler->inclination),2.)-0.5);

  orbit->k2_w=Const * pow((Rearth/(Rearth+orbit->height)),3.5) * 
                                            cos(kepler->inclination);

  orbit->k2_n=D2R(orbit->k2_n);
  orbit->k2_w=D2R(orbit->k2_w);

/* Translate reference time into tm struct */
  orbit->ref_tm.tm_isdst=0;
  orbit->ref_tm.tm_year=kepler->epoch_year;
  orbit->ref_tm.tm_yday=kepler->epoch_day-1;
  yday2mday_mon(&orbit->ref_tm);
  orbit->ref_tm.tm_hour=  (kepler->epoch_day-1-orbit->ref_tm.tm_yday)*24;
  orbit->ref_tm.tm_min = ((kepler->epoch_day-1-orbit->ref_tm.tm_yday)*24-
                                               orbit->ref_tm.tm_hour)*60;
  orbit->ref_tm.tm_sec =(((kepler->epoch_day-1-orbit->ref_tm.tm_yday)*24-
                                               orbit->ref_tm.tm_hour)*60-
                                               orbit->ref_tm.tm_min )*60;

  orbit->ref_time=mktime_ntz(&orbit->ref_tm);
  orbit->ref_tm.tm_wday=0;
  orbit->ref_ms=0;

/* Set valid-flag */
  orbit->valid=TRUE;
  return 1;
}


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
// Zie ook geomapping.c: te combineren!
static float x2earthangle(float sat_height,            /* height sat */
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


/*************************************
 * Calculate position of satellite from 
 *   current time and reference time (ref-time is in KEPLER-data)
 * Return: 0 if calculation has failed 
 *         1 if OK
 *************************************/
static int calcpossat(struct tm *cur_tm, int cur_ms,  /* current time */
               KEPLER *kepler,                 /* kepler data */
               ORBIT *orbit,                   /* orbit data */
               POINT *pos_sat)                 /* position satellite */
{
  double dift;
  double loop_nr;
  int i_loopnr;
  float orbit_angle,dh_n,dh_w;

/* Determine time difference with reference in sec */
  dift=mktime_ntz(cur_tm)-orbit->ref_time;
  dift=dift+(float)(cur_ms-orbit->ref_ms)/1000.;

/* Translate difference into days */
  dift/=(24.*3600.);

/* Calculate # loops since ref-time. */
  loop_nr=(kepler->motion+kepler->decay_rate*dift)*dift; 
  if ((ABS(loop_nr/10)) >= FLT_MAX) return -1;     /* loop_nr too large -> error */

/* Calc orbit-angle in radians */
  orbit_angle=(loop_nr-(int)loop_nr)*PIx2;
  i_loopnr=(int)loop_nr;

/* Add starting point */
  orbit_angle=orbit_angle+kepler->perigee+kepler->anomaly;

/* Shift result to [0 ... pi*2] */
  while (orbit_angle<0)    orbit_angle+=PIx2;
  while (orbit_angle>PIx2) orbit_angle-=PIx2;

/* Determine corrections */
  dh_n=dift*orbit->k2_n;
  dh_w=dift*orbit->k2_w;

  dh_n=dh_n-(int)(dh_n/PIx2)*PIx2;
  dh_w=dh_w-(int)(dh_w/PIx2)*PIx2;
  orbit->dh_w=dh_w;

/* Translate to orthonormal coordinates */
  pos_sat->x=sin(orbit_angle+dh_n)*cos(kepler->inclination);
  pos_sat->y=cos(orbit_angle+dh_n);
  pos_sat->z=sin(orbit_angle+dh_n)*sin(kepler->inclination);

  return i_loopnr;
}

/*************************************
 * Calculate observation point of satellite.
 * Takes into account sensor movement.
 * Result in pos_sat->lon,lat
 *************************************/
static void calc_satobs(KEPLER *kepler,ORBIT *orbit,
                 POINT *pos_sati,POINT *pos_sato,float beta)
{
#define OUDBETA
#ifdef OUDBETA
/* No need to calc. if beta=0. Just to speed up calcs. */
  if (beta)
  {
    pos_sato->x=pos_sati->x*cos(beta)+sin(kepler->inclination)*sin(beta);
    pos_sato->y=pos_sati->y*cos(beta);
    pos_sato->z=pos_sati->z*cos(beta)-cos(kepler->inclination)*sin(beta);
//if (beta) printf("(%f,%f,%f) ==>   (%f,%f,%f)  %f\n",
//pos_sati->x,pos_sati->y,pos_sati->z,pos_sato->x,pos_sato->y,pos_sato->z,R2D(beta));
  }
  else
  {
    pos_sato->x=pos_sati->x;
    pos_sato->y=pos_sati->y;
    pos_sato->z=pos_sati->z;
  }
#else
  {
    float mu,nu;
    mu=asin(MIN(MAX(pos_sati->z,-1.),1.));
    nu=atan2(-1.*pos_sati->x,pos_sati->y);
//    nu=acos(MIN(MAX((pos_sati->y/sqrt(1-pos_sati->z*pos_sati->z)),-1.),1.));
//printf("A %f  %f  %f\n",pos_sati->x,pos_sati->y,pos_sati->z);
    mu=mu-beta*cos(kepler->inclination);
    nu=nu+beta*sin(kepler->inclination);
    pos_sato->z=sin(mu);
    pos_sato->y=cos(nu)*cos(mu);
    pos_sato->x=-1*sin(nu)*cos(mu);
  }
#endif
//printf("I %f  %f  %f\n",pos_sati->x,pos_sati->y,pos_sati->z);
//printf("O %f  %f  %f\n",pos_sato->x,pos_sato->y,pos_sato->z);
}

void calc_x2satobs(KEPLER *kepler,ORBIT *orbit,
                   POINT *pos_sati,POINT *pos_sato,
                   int width_original,float max_sens_angle,
                   int x)
{
  float beta;
  beta=x2earthangle(orbit->height,E_W,
                    x,width_original/2,max_sens_angle);
  calc_satobs(kepler,orbit,pos_sati,pos_sato,beta);
}

/*************************************
 * Calculate position of earth since jan 1 
 *************************************/
static void calcposearth(struct tm *cur_tm, int ms, POINT *pos_earth)
{
  float days,rot_earth;
/* days since jan 1 */
  days=cur_tm->tm_yday+1+cur_tm->tm_hour/24.+
                         cur_tm->tm_min/1440.+     
                         cur_tm->tm_sec/86400.+    
                         (float)ms/86400000.;      

/* earth rotations since jan 1. Translate to 365.24 days/year. */
  rot_earth=days*LEN_LEAPYEAR/LEN_YEAR;

/* Translate into rotations. Ignore full rotations and add offset. */
  pos_earth->lon=PIx2*(rot_earth-(int)rot_earth)+D2R(Earth_offsetangle);

/* Translate to range [0 ... 2*pi] */
  while (pos_earth->lon>=PIx2) pos_earth->lon-=PIx2;

/* earth rotates in lon direction only */
  pos_earth->lat=0;
}

/*************************************
 * Calculate position of satellite relative to earth
 *************************************/
static void calcposrel(KEPLER *kepler,ORBIT *orbit,
                POINT *pos_sat,POINT *pos_earth,POINT *pos_rel)
{
/* Translate to long/lat */
  pos_rel->lon=atan2(pos_sat->x,pos_sat->y) +
                          kepler->raan-orbit->dh_w-pos_earth->lon-
                          D2R(orbit->offset_lon);

  pos_rel->lat=asin(pos_sat->z);

/* Shift to range [-pi..+pi] */  
  while (pos_rel->lon < -1*PI) pos_rel->lon+=PIx2;
  while (pos_rel->lon >    PI) pos_rel->lon-=PIx2;
  while (pos_rel->lat < -1*PI) pos_rel->lat+=PIx2;
  while (pos_rel->lat >    PI) pos_rel->lat-=PIx2;
}

/*************************************
 * Translate x/y position into lon/lat
 * y=0 -> time of start-of-pic, in grp->grp_tm
 *************************************/
int xy2lonlat(int x,int y,              /* input coordinates */
              PIC_CHARS *pci,           /* contains orbit info */
              POINT *pos_sensor)        /* output position satellite */
{
  struct tm cur_tm;
  int cur_ms;
  POINT pos_sat1;
  int avhrr_linespsec=AVHRR_LINESPSEC;
  static int yp=-1;
  static POINT pos_sat,pos_earth;
  int width=pci->width;
  if (!width) width=2048;
/* Test on valid data */
  if (!pci->orbit.valid) return 0;
  if (pci->lines_per_sec) avhrr_linespsec=pci->lines_per_sec;
  if (y!=yp)
  {
/* Determine time at line y */
    cur_tm=pci->orbit.start_tm;
    cur_tm.tm_sec=pci->orbit.start_tm.tm_sec+y/avhrr_linespsec;
    cur_ms=pci->orbit.start_ms+((y%avhrr_linespsec)*1000)/avhrr_linespsec;

/* Add corrections if computer time is used */

    if (!pci->orbit.use_sattime)
    {
      cur_tm.tm_hour+=pci->orbit.offset_gmt;
      cur_tm.tm_sec+=pci->orbit.offset_sec;
    }

/* Determine sub-satellite position */
    calcposearth(&cur_tm,cur_ms,&pos_earth);
    calcpossat(&cur_tm,cur_ms,&pci->kepler,&pci->orbit,&pos_sat);
  }
  pos_sat1=pos_sat;

  calc_x2satobs(&pci->kepler,&pci->orbit,&pos_sat,&pos_sat1,
                              width,DEF_NOAA_ALPHAMAX,x);
  calcposrel(&pci->kepler,&pci->orbit,&pos_sat1,&pos_earth,pos_sensor);

  yp=y;
  return 1;
}

static void replace_kep(KEPLER *kepler,SEGMENT *segm,char *sat)
{
  kepler->epoch_year=segm->orbit.epoch_year;
  kepler->epoch_day=segm->orbit.epoch_day;
  kepler->decay_rate=0;   // not needed
  kepler->inclination=D2R(segm->orbit.inclination);
  kepler->raan=D2R(segm->orbit.raan);
  kepler->eccentricity=segm->orbit.eccentricity;
  kepler->perigee=D2R(segm->orbit.perigee);
  kepler->anomaly=D2R(segm->orbit.anomaly);
  // If no kepler-file: take fixed motion (is not in EPS???)
  if (!kepler->motion)
  {
    if (strstr(sat,"NOAA17"))
    {
      kepler->motion=14.238391;  // NOAA-17
    }
    else if (strstr(sat,"NOAA18"))
    {
      kepler->motion=14.110221;  // NOAA-18
    }
    else // if (strstr(sat,"METOP"))
    {
      kepler->motion=14.214759;  // METOP-A
    }
  }
}

static void eps_read_pos(GROUP *grp)
{
  SEGMENT *s;
  FILE *fp;
  HDR hdr;
  int typehdr;
  if (grp->chan->segm->xh.xrit_frmt!=METOP) return;

  for (s=grp->chan->segm; s; s=s->next)
  {
    if ((s->orbit.subsat_start.lat) || (s->orbit.subsat_end.lat)) continue;
    if (!(fp=fopen(s->pfn,"rb"))) continue;
    typehdr=0;
    while (typehdr!=1)
    {
      typehdr=read_genrechdr(fp,&hdr);
      if (typehdr<0) break;
    }
    if (typehdr==1)
    {
      s->orbit.subsat_start=hdr.cls1.subsat_start;
      s->orbit.subsat_end=hdr.cls1.subsat_end;
      s->orbit.duration_of_product=hdr.cls1.duration_of_product;

      s->orbit.orbit_start=hdr.cls1.orbit_start;
      s->orbit.epoch_year=hdr.cls1.epoch_year;
      s->orbit.epoch_day=hdr.cls1.epoch_day;
      s->orbit.eccentricity=hdr.cls1.eccentricity;
      s->orbit.inclination=hdr.cls1.inclination;
      s->orbit.perigee=hdr.cls1.perigee;
      s->orbit.raan=hdr.cls1.raan;
      s->orbit.anomaly=hdr.cls1.anomaly;
      s->orbit.total_mdr=hdr.cls1.total_mdr;
      {
        CHANNEL *ch;
        for (ch=grp->chan->next; ch; ch=ch->next)
        {
          SEGMENT *s2;
          if ((s2=Get_Segm(ch,s->xh.segment)))
            s2->orbit=s->orbit;
        }
      }
    }
    fclose(fp);
  }
}

void check_kep(SEGMENT *segm,PIC_CHARS *pc)
{
  POINT pos;
  int y=0;
  int n=0;
return;
  for (; segm; segm=segm->next)
  {
    xy2lonlat(1024,y,pc,&pos);
    n=segm->xh.segment;
    pos.lon=R2D(pos.lon);
    pos.lat=R2D(pos.lat);

printf("%4d  (%8.2f,%8.2f)  (%8.2f,%8.2f)  (%8.2f,%8.2f)\n",n,
pos.lon,pos.lat,
segm->orbit.subsat_start.lon,segm->orbit.subsat_start.lat,segm->orbit.subsat_start.lon-pos.lon,segm->orbit.subsat_start.lat-pos.lat);
    y=n*segm->chan->nl;
  }
}

// download kepler
// ret: 0: OK
//     'p': download proggram problem
//     'c': connection problem
//     'd': download problem
int replace_kepler(char *prog,char *gfile,char *ofile)
{
  char cmd[200];
  static int no_connection;
  if (!prog) prog="wget";
  if (!gfile)
  {
    sprintf(cmd,"%s -V",prog);
    if (system(cmd))
    {
      no_connection='p';
    }
    else if (no_connection=='p')
    {
      no_connection=0;
    }
    
    if (!no_connection)
    {
      sprintf(cmd,"%s -q --spider %s",prog,HTTP_KEPLER);
      if (system(cmd)) no_connection='c';
    }
  }
  else if (!no_connection)
  {
    sprintf(cmd,"%s -q -O %s %s/%s",prog,ofile,HTTP_KEPLER,gfile);
    if (system(cmd)) no_connection='d';
    if (no_connection)
    {
      remove(ofile);
    }
  }
  return no_connection;  // 0: OK, 'p': connection problem, 'd': download problem
}

#define nrdays(tm) (tm.tm_year*365+tm.tm_yday)
int download_and_get_kepler(GROUP *grp)
{
  int age,days_a,days_b;
  char kep_absfile[200];
  time_t now=time(NULL);
  struct tm nowtm;
  nowtm=*gmtime(&now);

  sprintf(kep_absfile,"%s/%s",prefer.cur_dir,NORADFILE);
  age=read_norad_keps(grp->sat_src,grp->grp_tm,&grp->pc.kepler);
  days_a=nrdays(nowtm);
  days_b=nrdays(grp->grp_tm);
  if ((age>5) && ((ABS(days_a-days_b)) < age))
  {
    replace_kepler(NULL,NORADFILE,kep_absfile);
    age=read_norad_keps(grp->sat_src,grp->grp_tm,&grp->pc.kepler);
  }
  return age;
}

#ifdef NOTUSED
/**************************************************
 * Generate table y => (lon,lat) 
 **************************************************/
static void gen_lonlattable(GROUP *grp)
{
  POINT pos;
  PIC_CHARS *pc=&grp->pc;
  int i,y;
  if (!pc->height) return;
  for (i=0; i<2; i++)
  {
    if (pc->ypos_tbl[i]) free(pc->ypos_tbl[i]); 
    pc->ypos_tbl[i]=malloc(pc->height*sizeof(float));
  }
  for (y=0; y<pc->height; y++)
  {
    xy2lonlat(1024,y,pc,&pos);
    pc->ypos_tbl[0][y]=R2D(pos.lon);
    pc->ypos_tbl[1][y]=R2D(pos.lat);
  }
}
#endif

/**************************************************
 * Load Kepler 
 **************************************************/
int load_kepler(GROUP *grp)
{
  int age;
//  EPS: replace kepler data by EPS values.
//  Kepler file not needed (see 'replace_kep')
  if ((grp->chan->segm) && (grp->chan->segm->xh.xrit_frmt==METOP))
  {
    eps_read_pos(grp);
    grp->has_kepler=TRUE;
    replace_kep(&grp->pc.kepler,grp->chan->segm,grp->sat_src);
    calc_orbitconst(&grp->pc.kepler,&grp->pc.orbit);
    grp->pc.orbit.start_tm=grp->grp_tm;
    age=(grp->grp_tm.tm_year*365+grp->grp_tm.tm_yday)-
        (grp->pc.kepler.epoch_year*365+grp->pc.kepler.epoch_day);
    if (age==-1) age=0;
    check_kep(grp->chan->segm,&grp->pc);
  }
  else
  {
//age=download_and_get_kepler(grp);
    if (!strcmp(grp->sat_src,"metop-A"))
      age=read_norad_keps("METOP-A",grp->grp_tm,&grp->pc.kepler);
    else
      age=read_norad_keps(grp->sat_src,grp->grp_tm,&grp->pc.kepler);
    if (age>=0)
    {
      grp->has_kepler=TRUE;
      calc_orbitconst(&grp->pc.kepler,&grp->pc.orbit);
      grp->pc.orbit.start_tm=grp->grp_tm;
      age=(grp->grp_tm.tm_year*365+grp->grp_tm.tm_yday)-
          (grp->pc.kepler.epoch_year*365+grp->pc.kepler.epoch_day);
    }
  }
  grp->pc.orbit.age_days=age;
//  gen_lonlattable(grp);
  return age;
}

// check if kepler data exist; download if not.
// return: -1: unexpected
//         -2: unexpected; filename too long
//         -3: cannot download
//         -4: cannot find sat in file
//         >=0: age (absolute difference in days!)
#define NORAD_DOWNLOADFILE "weather.txt"
// noaa segm-filename: avhrr_20090820_222000_noaa17.hrp.bz2
// norad filename:     norad_20090820_222000_noaa17.txt

char *mk_noradname(GROUP *grp)
{
  static char fn[100];
  if (!grp) return NULL;
  if (!grp->chan) return NULL;
  if (!grp->chan->segm) return NULL;
  if (!grp->chan->segm->pfn) return NULL;
  strftime(fn,80,"norad_%Y%m%d_%H%M00_",&grp->grp_tm);
  strncat(fn,grp->sat_src,10);
  strcat(fn,".txt");
  return fn;
}

int load_kepler_v2(GROUP *grp,char *prog)
{
  char *fn,*p;
  char *pfn=NULL;
  int dt;
  if (!(fn=mk_noradname(grp))) return -1;
  strcpyd(&pfn,grp->chan->segm->pfn);
  if ((p=strrchr(pfn,DIR_SEPARATOR)))
  {
    if (strlen(p) < strlen(fn))
    {
      free(pfn);
      return -2;
    }
    strcpy(p+1,fn);
  }
  if (!exist_file(pfn))
  {
    if ((replace_kepler(prog,NORAD_DOWNLOADFILE,pfn)))
    {
      free(pfn);
      return -3;        // cannot download
    }
  }

  dt=read_norad_keps_from_file(pfn,grp->sat_src,&grp->grp_tm,&grp->pc.kepler);
  if (dt<0)
  {
    remove(pfn);
    free(pfn);
    return -4;  // sat not found
  }
  free(pfn);
  calc_orbitconst(&grp->pc.kepler,&grp->pc.orbit);
  grp->pc.orbit.age_days=dt;

  grp->pc.orbit.start_tm=grp->grp_tm;
  grp->pc.orbit.valid=TRUE;
  return dt;
}

