/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Main function XRIT2PIC
 ********************************************************************/
int gdebug;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "xrit2pic.h"
#include "avhrr.h"

#ifndef RELEASE
#define RELEASE "Undefined"
#endif

char releasenr[100];
PREFER prefer;
GLOBINFO globinfo;
GROUP *globgrp;
XRIT_DBASE dbase;

/* see #define nrchanlist 12 */
char *channellist[]={"VIS006","VIS008","IR_016","IR_039","WV_062","WV_073",
                     "IR_087","IR_097","IR_108","IR_120","IR_134","HRV"};

/* see #define nrsatlist 15 */
char *satlist[]    ={"HMSG1","LMSG1","HMSG2","LMSG2",
                     "MET5","MET6","MET7",
                     "GOES9","GOES10","GOES12","MTSAT1R",
                     "NOAA","GAC","METOP-A","SERVICE","DWDSAT","BUFR"};
                
void examples()
{
  FILE *fp=stdout;
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"Example MSG:\n");
  fprintf(fp,"   xrit2pic -nogui -type H -chan VIS* -segm 3-7 -date 04-02-* -time 21:00 \n");
  fprintf(fp,"Example MSG colour:\n");
  fprintf(fp,"   xrit2pic -nogui -type H -compose\n");
  fprintf(fp,"   xrit2pic -nogui -type H -r VIS006=90 -r VIS008=-90 -go 10 -g VIS006 -b IR_016\n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"Example AVHRR: translate all avhrr files into .rah format \n");
  fprintf(fp,"  xrit2pic -nogui -type A -rah \n");
  fprintf(fp,"Example AVHRR in colour \n");
  fprintf(fp,"  xrit2pic -nogui -type A -colour \n");
  fprintf(fp,"  xrit2pic -nogui -type A -r %s -g %s -b %s \n",A_CH1,A_CH2,A_CH4);
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"Example overlay options: \n");
  fprintf(fp,"  xrit2pic -nogui -ol \n");
  fprintf(fp,"  xrit2pic -nogui -ol=80 \n");
  fprintf(fp,"  xrit2pic -nogui -ol=80 coast\n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"Example Anaglyph options: \n");
  fprintf(fp,"  xrit2pic -nogui -anagl \n");
  fprintf(fp,"  xrit2pic -nogui -anagl 10 \n");
  fprintf(fp,"  xrit2pic -nogui -anagl 10:20 \n");
  fprintf(fp,"  xrit2pic -nogui -anagl 10:20-200 \n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"Example MSG complex compose:\n");
  fprintf(fp,"  xrit2pic -nogui -type H -compose dust\n");
  fprintf(fp,"\n");
  fprintf(fp,"Example MSG complex compose, custom: (result is same as 'dust' mapping)\n");
  fprintf(fp,"  xrit2pic -nogui -type H -r [IR_120,IR_108,-4,2,1.] -g [IR_108,IR_087,0,15,2.5] -b [IR_108,,261,289,1]\n");
  fprintf(fp,"Calculation [chanp,chann,t1,t2,g]:\n");
  fprintf(fp,"  (chanp-chann)=t1 ==> pixvalue=0\n");
  fprintf(fp,"  (chanp-chann)=t2 ==> pixvalue=255\n");
  fprintf(fp,"  t1 < (chanp-chann) < t2 ==> use gamma 'g'\n");
  fprintf(fp,"------------------------------------------------\n");
}

void usage()
{
  FILE *fp=stdout;
  fprintf(fp," +------------------------------------------------+\n");
  fprintf(fp," |  xrit2pic release %s                      |\n",RELEASE);
  fprintf(fp," | Translate XRIT/AVHRR files into pictures.      |\n");
  fprintf(fp," | R. Alblas                                      |\n");
  fprintf(fp," | werkgroep Kunstmanen                           |\n");
  fprintf(fp," +------------------------------------------------+\n");
  fprintf(fp,"\n");
  fprintf(fp," xrit2pic [options]\n");
  fprintf(fp,"  options:\n");
  fprintf(fp,"  -h                       this help\n");
  fprintf(fp,"  -e                       non-gui examples\n");
  fprintf(fp,"  -overlay_files           show available overlayfiles\n");
  fprintf(fp,"  -nogui                   non-gui mode\n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"    options for locations:\n");
  fprintf(fp,"      -src <path>          path to source XRIT files (def.: see ini-file)\n");
  fprintf(fp,"      -dest <path>         path to destination dir (def.: see ini-file)\n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"    options for file formats:\n");
  fprintf(fp,"      -pgm                 generate PGM  (see below for defaults)\n");
  fprintf(fp,"      -pgm8                generate PGM (forced to 8 bits pp)\n");
  fprintf(fp,"      -jpg                 generate JPEG (see below for defaults)\n");
  fprintf(fp,"      -cjpg                generate JPEG (no interm. decompression)\n");
  fprintf(fp,"      -rah                 generate rah from AVHRR\n");
  fprintf(fp,"      -nolin               don't linearize AVHRR\n");

  fprintf(fp,"      -overwrite           overwrite if file exist (default: not)\n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"    options for choosing what to generate:\n");
  fprintf(fp,"      -europe              Europe part only (MSG)\n");
  fprintf(fp,"      -area <nr>           fixed areas defined in prefs tab Mapping.\n");
  fprintf(fp,"                              0=full, 1/2/3/4: see prefs(def.: 1=europe)\n");
  fprintf(fp,"      -area <area_name>    same, but define name instead of number.\n");
//  fprintf(fp,"      -satdir <to_n or to_s>  for polar sat: use this direction.\n");

  fprintf(fp,"      -proj <type>         projection: plate_carree, mercator\n");
  fprintf(fp,"                                       polar_n, polar_s,\n");
  fprintf(fp,"                                       normal (default)\n");

  fprintf(fp,"      -movie               generate movie file\n");
  fprintf(fp,"      -timestamp           add date/time to pic.\n");
  fprintf(fp,"      -timestamp@[<x,y>]s<size>\n");
  fprintf(fp,"                           add date/time at position (x,y) fontsize <size>\n");

  fprintf(fp,"      -add_text@[<x,y>]s<size> <text>\n");
  fprintf(fp,"                           add text to pic at position (x,y) fontsize <size>\n");
  fprintf(fp,"                             e.g. -add_text(0,0)s8 \"This is added text\"\n");
  fprintf(fp,"                             s=0: auto size\n");
  fprintf(fp,"      -sat <sat_source>    translate only this sat (MSG1, MET7...)\n");
  fprintf(fp,"      -type <H or L or A>  translate only this type (HRIT/LRIT/AVHRR)\n");
  fprintf(fp,"      -chan <chan_name>    translate only this channel\n");
  fprintf(fp,"      -segm <range>        translate only these segments\n");
  fprintf(fp,"      -date <yy-mm-dd>     translate only this date\n");
  fprintf(fp,"      -time <hh:mm>        translate only this time\n");
  fprintf(fp,"      -pro                 extract also prologue file\n");
  fprintf(fp,"      -epi                 extract also epilogue file\n");
  fprintf(fp,"      -inv                 invert\n");
  fprintf(fp,"      -temp                generate temperature mapping (IR only)\n");
  fprintf(fp,"      -temp_bw             same, GOES mode-A mapping (IR only)\n");
  fprintf(fp,"      -anagl [anagl_opts]  generate anaglyph\n");
  fprintf(fp,"         anagl_opts: <shft[:lmin[-lmax]]>\n");
  fprintf(fp,"                           shft:  pixel-shift left/right (def. 20)\n");
  fprintf(fp,"                           lmin:  max shift at this lum  (def. 0)\n");
  fprintf(fp,"                           lmax:  min shift at this lum  (def. 512)\n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"    options for multi-channel compositions:\n");
  fprintf(fp,"      -compose [type]      make false-color pics\n");
  fprintf(fp,"                           type=optional: vis, airm, dust,\n");
  fprintf(fp,"                                          nfog, dfog,\n");
  fprintf(fp,"                                          uph_cloud, uph_dust,uph_ash,\n");
  fprintf(fp,"                                          uph_night, uph_day,\n");
  fprintf(fp,"                                          conv_storm, snow_fog\n");
  fprintf(fp,"                                vis=default\n");
  fprintf(fp,"      -hrvlum              use HRV for lum (high-res colour)\n");
  fprintf(fp,"      -inv <channame>      invert this channel\n");
  fprintf(fp,"      -lmin <val>          minval; this val mapped on 'black'\n");
  fprintf(fp,"      -lmax <val>          maxval; this val mapped on 'white'\n");
  fprintf(fp,"      -gamma <val>         gamma\n");
  fprintf(fp,"      -inv <channame>      invert this channel\n");
  fprintf(fp,"\n");
  fprintf(fp,"      -r <channame>[=val]  use this channel for red\n");
  fprintf(fp,"      -g <channame>[=val]  use this channel for green\n");
  fprintf(fp,"      -b <channame>[=val]  use this channel for blue\n");
  fprintf(fp,"                           val=-100...100 (def. 100)\n");
  fprintf(fp,"      -ro <offset>         offset for red (def. 0)\n");
  fprintf(fp,"      -go <offset>         offset for green (def. 0)\n");
  fprintf(fp,"      -bo <offset>         offset for blue (def. 0)\n");
  fprintf(fp,"\n");
  fprintf(fp,"    complex compose: <chanp>-<chann>, values between \n");
  fprintf(fp,"                     kmin and kmax mapped between 0 and 255, using gamma\n");
  fprintf(fp,"      -r [<chanp>,<chann>,<kmin>,<kmax>,<gamma>]  special mapping for red\n");
  fprintf(fp,"      -g [<chanp>,<chann>,<kmin>,<kmax>,<gamma>]  special mapping for green\n");
  fprintf(fp,"      -b [<chanp>,<chann>,<kmin>,<kmax>,<gamma>]  special mapping for blue\n");
  fprintf(fp,"                           kmin, kmax in Kelvin\n");
  fprintf(fp,"\n");
  fprintf(fp,"      -lut <lut_fname>{n}  Use this lut file, line n (see preferences for loc.)\n");
  fprintf(fp,"      -lut {n}             only specify line nr (LUT file in Prefs)\n");
  fprintf(fp,"\n");
  fprintf(fp,"      -msb <msb>           msb for translation 10 bits->8 bits\n");
  fprintf(fp,"                           <msb>=9, 8 or 7\n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"    Other options:\n");
  fprintf(fp,"      -test                don't generate, just show what will happen\n");
  fprintf(fp,"      -log                 generate log file 'xrit2pic.log' (in dest. dir)\n");
  fprintf(fp,"      -ol[=<lum>] [<type>] add overlay to picture\n");
  fprintf(fp,"                           <lum> = overlay lum, 0..255 (def. 255)\n");
  fprintf(fp,"                           <type> = coa(st), cou(ntry),\n");
  fprintf(fp,"                                    l(atlong), b(oth) (b=default)\n");
  fprintf(fp,"      -lonlat[=<lum>]      add lon/lat lines to picture\n");
  fprintf(fp,"\n");
  fprintf(fp,"------------------------------------------------\n");
  fprintf(fp,"Default generated formats:\n");
  fprintf(fp,"   from Wavelet: pgm\n");
  fprintf(fp,"   from Jpeg: jpeg\n");
  fprintf(fp,"\n");
  fprintf(fp,"For options -chan, -date and -time wildcards are allowed.\n");
  fprintf(fp,"\n");
  fprintf(fp,"------------------------------------------------\n");
}

int main(int argc,char **argv)
{
  char *src_dir=NULL;
  char *dest_dir=NULL;
  char *p;
  float  f;
  int i;
  char *sarea=NULL;
  GENMODES genmode;
  CHANMAP *cm;
  gboolean gui=TRUE;
  gboolean test=FALSE;
  gboolean log=FALSE;
  gboolean debug=FALSE;
  gboolean show_overlay=FALSE;
  gdebug=0;
  
  strcpy(releasenr,RELEASE);
  memset(&prefer,0,sizeof(prefer));
  memset(&genmode,0,sizeof(genmode));
  memset(&globinfo,0,sizeof(globinfo));
  memset(&dbase,0,sizeof(dbase));

/* For MSG: nog nodig???*/
//  Create_Chanmap(&globinfo.chanmap,"IR_016",1.,0.,0.);  // default red
//  Create_Chanmap(&globinfo.chanmap,"VIS008",0.,1.,0.);  // default green
//  Create_Chanmap(&globinfo.chanmap,"VIS006",0.,0.,1.);  // default blue

/* For NOAA (doesn't bite with MSG!) Wel dus; is dit nog nodig???*/
//  Create_Chanmap(&globinfo.chanmap,A_CH1,1.,0.,0.);  // default red
//  Create_Chanmap(&globinfo.chanmap,A_CH2,0.,1.,0.);  // default green
//  Create_Chanmap(&globinfo.chanmap,A_CH4,0.,0.,1.);  // default blue

  globinfo.keep_chanlist.chan_sel[0]=TRUE;
  globinfo.keep_chanlist.chan_sel[1]=TRUE;
  globinfo.keep_chanlist.chan_sel[2]=TRUE;
  globinfo.keep_chanlist.sat_sel[0]=TRUE;   /* MSG HRIT */
  globinfo.keep_chanlist.sat_sel[9]=TRUE;   /* Services */

  globinfo.bitshift=1;
  globinfo.shift_3d=MAX_3DSHIFT;

/*zie popup_wnd*/
//  genmode.frx=1;
//  genmode.fry=1;
  genmode.overwrite_mode=SKIP;
  genmode.avhrr_lin=TRUE;

  /* Clear default rgb-mapping if 1 or more of -r/-g/-b options */
  for (i=1; i<argc; i++)
  {
    if ((!strcmp(argv[i],"-r")) || (!strcmp(argv[i],"-g")) || (!strcmp(argv[i],"-b")))
    {
      genmode.spm.compose=TRUE;
      genmode.spm.compose_type=map_cust;
      Remove_Chanmap(&globinfo.chanmap);
    }
  }
  for (i=1; i<argc; i++)
  {
    if (!strcmp(argv[i],"-h")) { usage(); return 0; }
    if (!strcmp(argv[i],"-e")) { examples(); return 0; }
    if (!strcmp(argv[i],"-gdebug")) gdebug=1;

    else if (!strcmp(argv[i],"-v"))
    {
      fprintf(stdout,"Release %s\n",RELEASE);
      return 0;
    }
    else if (!strcmp(argv[i],"-debug"))     debug=TRUE;
    else if (!strcmp(argv[i],"-nogui"))     gui=FALSE;
    else if (!strcmp(argv[i],"-pgm"))       genmode.cformat='p';
    else if (!strcmp(argv[i],"-pgm8"))      genmode.cformat='P';
    else if (!strcmp(argv[i],"-jpg"))       genmode.cformat='j';
    else if (!strcmp(argv[i],"-cjpg"))      genmode.cformat='J';
    else if (!strcmp(argv[i],"-rah"))       genmode.cformat='r';
    else if (!strcmp(argv[i],"-movie"))     genmode.gen_film=TRUE;
    else if ((!strcmp(argv[i],"-compose")) ||
             (!strcmp(argv[i],"-colour")) || (!strcmp(argv[i],"-color")))
    {
      genmode.spm.compose=TRUE;
      genmode.spm.compose_type=map_vis_hmsg;
      if ((i+1<argc) && (*argv[i+1]!='-'))
      {
        i++;
        if (!strncmp(argv[i],"vis",3))
          genmode.spm.compose_type=map_vis_hmsg;
        else if (!strncmp(argv[i],"noaavis",3))
          genmode.spm.compose_type=map_noaavis;
        else if (!strncmp(argv[i],"metopvis",3))
          genmode.spm.compose_type=map_metopvis;
        else if (!strncmp(argv[i],"airmass",4))
          genmode.spm.compose_type=map_airm;
        else if (!strcmp(argv[i],"dust"))
          genmode.spm.compose_type=map_dust;
        else if (!strcmp(argv[i],"fog"))
          genmode.spm.compose_type=map_nfog;
        else if (!strcmp(argv[i],"nfog"))
          genmode.spm.compose_type=map_nfog;
        else if (!strcmp(argv[i],"dfog"))
          genmode.spm.compose_type=map_dfog;
        else if (!strcmp(argv[i],"uph_cloud"))
          genmode.spm.compose_type=map_ucld;
        else if (!strcmp(argv[i],"uph_dust"))
          genmode.spm.compose_type=map_udust;
        else if (!strcmp(argv[i],"uph_ash"))
          genmode.spm.compose_type=map_uash;
        else if (!strcmp(argv[i],"uph_night"))
          genmode.spm.compose_type=map_unight;
        else if (!strcmp(argv[i],"uph_day"))
          genmode.spm.compose_type=map_uday;
        else if (!strcmp(argv[i],"conv_storm"))
          genmode.spm.compose_type=map_cnv_strm;
        else if (!strcmp(argv[i],"snow_fog"))
          genmode.spm.compose_type=map_snowfog;

        else
        {
          fprintf(stderr,"Error: compose type %s; use airm/dust/fog.\n",argv[i]);
          return 1;
        }
      }
    }
    else if (!strcmp(argv[i],"-temp"))      globinfo.spm.map_temp=TRUE;
    else if (!strcmp(argv[i],"-temp_bw"))   globinfo.spm.map_temp_G_mA=TRUE;
    else if (!strcmp(argv[i],"-hrvlum"))    genmode.use_hrv_lum=TRUE;
    else if (!strcmp(argv[i],"-nolin"))     genmode.avhrr_lin=FALSE;
    else if (!strncmp(argv[i],"-europe",3)) genmode.area_nr=1;
    else if (!strcmp(argv[i],"-area"))
    {
      i++;
      if (isdigit(*argv[i]))
        genmode.area_nr=atoi(argv[i]);
      else
        sarea=argv[i];
    }
/*
    else if (!strcmp(argv[i],"-satdir"))
    {
      i++;
      if (!strcmp(argv[i],"to_n")) genmode.geoarea.pol_dir=1;
      else if (!strcmp(argv[i],"to_s")) genmode.geoarea.pol_dir=-1;
      else printf("Err: satdir=to_n or to_s\n");
    }
*/
    else if (!strcmp(argv[i],"-proj"))
    {
      genmode.gmap=TRUE;
      if ((i+1<argc) && (*argv[i+1]!='-'))
      {
        i++;
        genmode.gmap=(!strcmp(argv[i],"plate_carree")? plate_carree:
                      !strcmp(argv[i],"mercator")? mercator:
                      !strcmp(argv[i],"polar_n")? polar_n:
                      !strcmp(argv[i],"polar_s")? polar_s:
                       normal);
      }
    }
    else if (!strcmp(argv[i],"-pro"))       genmode.gen_pro=TRUE;
    else if (!strcmp(argv[i],"-epi"))       genmode.gen_epi=TRUE;
    else if (!strcmp(argv[i],"-overwrite")) genmode.overwrite_mode=REPLACE;
    else if (!strcmp(argv[i],"-skip_incomplete")) genmode.skip_incomplete=TRUE;
    else if (!strcmp(argv[i],"-test"))      test=TRUE;
    else if (!strcmp(argv[i],"-log"))       log=TRUE;
    else if (!strcmp(argv[i],"-overlay_files")) show_overlay=TRUE;
    else if (!strcmp(argv[i],"-anagl"))
    {
      char tmp[10];
      globinfo.dim3=TRUE;
      genmode.agl.shift_3d=20;
      genmode.agl.lmin=0;
      genmode.agl.lmax=511;
      if ((i+1<argc) && (*argv[i+1]!='-'))
      {
        i++;
        strncpy(tmp,argv[i],10); tmp[9]=0;
        genmode.agl.shift_3d=atoi(tmp);
        if ((p=strchr(tmp,':')))
        {
          p++;
          genmode.agl.lmin=atoi(p);
          if ((p=strchr(p,'-')))
          {
            p++;
            genmode.agl.lmax=atoi(p);
          }
        }
      }
    }
    else if (!strncmp(argv[i],"-ol",3))   
    {
      char *pl=strchr(argv[i],'=');
      genmode.ol_lum=0xff;
      if (i+1>=argc)                      genmode.overlaytype='b'; // no arg
      else if (*argv[i+1]=='-')           genmode.overlaytype='b'; // no arg
      else
      {
        i++;
        if (!strncmp(argv[i],"coa",3))      genmode.overlaytype='C';
        else if (!strncmp(argv[i],"cou",3)) genmode.overlaytype='c';
        else if (!strncmp(argv[i],"l",1))   genmode.overlaytype='l';
        else if (!strncmp(argv[i],"b",1))   genmode.overlaytype='b';
        else
        {
          fprintf(stderr,"Error: Wrong overlay type %s\n",argv[i]);
          return 1;
        }
      }
      if (pl)
      {
        genmode.ol_lum=atoi(pl+1);
        genmode.ol_lum=MIN(genmode.ol_lum,255);
        genmode.ol_lum=MAX(genmode.ol_lum,0);
      }
    }
    else if (!strncmp(argv[i],"-lonlat",6))
    {
      char *pl=strchr(argv[i],'=');
      genmode.ol_lum=0xff;
      if (pl)
      {
        genmode.ol_lum=atoi(pl+1);
        genmode.ol_lum=MIN(genmode.ol_lum,255);
        genmode.ol_lum=MAX(genmode.ol_lum,0);
      }
    }   
    else if (!strcmp(argv[i],"-inv"))
    {
      if ((i+1>=argc) || (*argv[i+1]=='-'))
      {
        globinfo.spm.invert=TRUE;
      }
      else
      {
        i++;
        if (!(cm=Get_Chanmap(globinfo.chanmap,argv[i])))
          cm=Create_Chanmap(&globinfo.chanmap,argv[i],0.,0.,0.); 
        cm->invert=TRUE;
      }
    }
    else if (!strncmp(argv[i],"-timestamp",strlen("-timestamp")))
    {
      char *p=argv[i]+strlen("-timestamp");
      genmode.timeid=TRUE;
      if (*p == '@')
      {
        p++; if (*p=='[')  genmode.pt.x=atoi(p+1);
        if ((p=strchr(p,','))) genmode.pt.y=atoi(p+1);
        if ((p=strchr(p,']')) &&
            (p=strchr(p,'s')) &&
            (*(p+1)))             genmode.pt.size=atoi(p+1);
      }
    }

    else if (i<argc-1)
    {
      if (!strncmp(argv[i],"-add_text",strlen("-add_text")))
      {
        char *p=argv[i]+strlen("-add_text");
        if (*p == '@')
        {
          p++; if (*p=='[')  genmode.pt2.x=atoi(p+1);
          if ((p=strchr(p,','))) genmode.pt2.y=atoi(p+1);
          if ((p=strchr(p,']')) &&
              (p=strchr(p,'s')) &&
              (*(p+1)))             genmode.pt2.size=atoi(p+1);
        }

        strncpy(genmode.pt2.str,argv[++i],90);
      }

      else if (!strcmp(argv[i],"-src"))  src_dir=argv[++i];
      else if (!strcmp(argv[i],"-dest")) dest_dir=argv[++i];
      else if (!strcmp(argv[i],"-chan")) genmode.chan_name=argv[++i];
      else if (!strcmp(argv[i],"-segm")) genmode.segm_range=argv[++i];
      else if (!strcmp(argv[i],"-type")) genmode.type=*argv[++i];
      else if (!strcmp(argv[i],"-sat"))  genmode.sat=argv[++i];
      else if (!strcmp(argv[i],"-date")) genmode.date=argv[++i];
      else if (!strcmp(argv[i],"-time")) genmode.time=argv[++i];
      else if (!strcmp(argv[i],"-msb"))
      {
        i++;
        if (!strcmp(argv[i],"9"))      globinfo.bitshift=2;
        else if (!strcmp(argv[i],"8")) globinfo.bitshift=1;
        else if (!strcmp(argv[i],"7")) globinfo.bitshift=0;
        else
        {
          fprintf(stderr,"Error: Wrong msb %s; use 9, 8 or 7\n",argv[i]);
          return 1;
        }
      }
      else if (!strcmp(argv[i],"-lmin")) 
      {
        genmode.adapt_lum=TRUE;
        genmode.lmin=atoi(argv[++i]);
      }
      else if (!strcmp(argv[i],"-lmax")) 
      {
        genmode.adapt_lum=TRUE;
        genmode.lmax=atoi(argv[++i]);
      }
      else if (!strcmp(argv[i],"-gamma")) 
      {
        genmode.adapt_lum=TRUE;
        genmode.gamma=atof(argv[++i]);
      }

      else if (!strcmp(argv[i],"-ro")) 
      {
        globinfo.offset[0]=atoi(argv[++i]);
      }  
      else if (!strcmp(argv[i],"-go")) 
      {
        globinfo.offset[1]=atoi(argv[++i]);
      }  
      else if (!strcmp(argv[i],"-bo")) 
      {
        globinfo.offset[2]=atoi(argv[++i]);
      }  
      else if ((!strcmp(argv[i],"-r")) || (!strcmp(argv[i],"-g")) || (!strcmp(argv[i],"-b")))
      {
        char rgb=argv[i][1];
        i++;
        if (*argv[i]=='[')   // (1.,wv062,wv073,-25,0)
        {
          char *p1,*p2;
          RGBMAPS *rgbm=(rgb=='r'? &globinfo.clrmapspec.r:
                         rgb=='g'? &globinfo.clrmapspec.g:
                         &globinfo.clrmapspec.b);
          if ((p2=strchr(argv[i],']'))) { *p2=0; p2=argv[i]+1; }
          if (p2) { p1=p2; if ((p2=strchr(p1,','))) { *p2=0; p2++; } strcpy(rgbm->chanp,p1); }  // + chan
          if (p2) { p1=p2; if ((p2=strchr(p1,','))) { *p2=0; p2++; } strcpy(rgbm->chann,p1); }  // - chan
          if (p2) { p1=p2; if ((p2=strchr(p1,','))) { *p2=0; p2++; } rgbm->valmin=atof(p1);  }  // minval
          if (p2) { p1=p2; if ((p2=strchr(p1,','))) { *p2=0; p2++; } rgbm->valmax=atof(p1);  }  // maxval
          if (p2) { p1=p2; if ((p2=strchr(p1,','))) { *p2=0; p2++; } rgbm->gamma=atof(p1);   }  // gamma
          globinfo.clrmapspec.use_temp=TRUE;
          strcpy(globinfo.clrmapspec.name,"custom");
          strcpy(globinfo.clrmapspec.name_sh,"cust");
          genmode.spm.compose_type=map_cust1;
        }
        else
        {
          f=1.;
          if ((p=strchr(argv[i],'=')))
          {
            *p=0;
            f=atoi(p+1)/100.;
          }
          if (!(cm=Get_Chanmap(globinfo.chanmap,argv[i])))
            cm=Create_Chanmap(&globinfo.chanmap,argv[i],0,0.,0.); 
          if (rgb=='r') cm->r=f;
          if (rgb=='g') cm->g=f;
          if (rgb=='b') cm->b=f;
        }
      }
      else if (!strcmp(argv[i],"-lut"))
      {
        i++;
        if ((*argv[i] != '[') && (*argv[i] != '{') && (*argv[i] != '('))
        {
          strncpy(globinfo.lut.fname,argv[i],30);
          if ((p=strchr(globinfo.lut.fname,'[')))
          {
            *p=0; p++;
            globinfo.lut.sellut=atoi(p);
          }
        }
        else
        {
          globinfo.lut.sellut=atoi(argv[i]+1);
        }
        globinfo.lut.use=TRUE;
      }
      else
      {
        fprintf(stderr,"Error: Wrong option %s \n",argv[i]);
        return 1;
      }
    }
    else
    {
      fprintf(stderr,"Error: Wrong option %s or missing argument\n",argv[i]);
      return 1;
    }
  }
  globinfo.avhrr_lin=genmode.avhrr_lin;
  globinfo.gmap=genmode.gmap;
  globinfo.area_nr=genmode.area_nr;
  /* Allow '-' as separator instead of ':' (':' not always accepted as arg) */
  if (genmode.time) 
  {
    char *p;
    while ((p=strchr(genmode.time,'-'))) *p=':';
  }

  if (genmode.gen_film) 
  {
    genmode.skip_incomplete=TRUE;
//    genmode.frx=genmode.fry=4;
  }
  if (debug) dump_dbase(globgrp);

  get_dirs(argv[0],&prefer.cur_dir,&prefer.prog_dir,&prefer.prog_xdir,&prefer.home_dir);
  search_file(PREFFILE,prefer.used_preffile,
              prefer.cur_dir,prefer.home_dir,prefer.prog_dir);

  if (exist_file_in_dir(prefer.prog_dir,BZIPPROG))
  {
    sprintf(globinfo.prog_bzip2,"\"%s%c%s\"",prefer.prog_dir,DIR_SEPARATOR,BZIPPROG);
  }
  else
  {
    strcpy(globinfo.prog_bzip2,BZIPPROG);
  }
  
  printf("Using preferences %s...\n",prefer.used_preffile);
  Read_Pref(NULL,&prefer);
  globinfo.spm.inv_ir=prefer.invert_ir;
  if (show_overlay)
  {
    pri_overlayfiles(stdout,prefer.overlay,(void *)fprintf);
    return 0;
  }

  if (sarea)
  {
    for (i=0; i<NRGEODEFS; i++)
    {
      if (!strcmp(sarea,prefer.geoarea[i].part_name))
      {
        globinfo.area_nr=genmode.area_nr=i+1;
        break;
      }
    }
    if (i==NRGEODEFS)
    {
      printf("ERROR: No area %s. Defined areas:\n",sarea);
      for (i=0; i<NRGEODEFS; i++)
      {
        puts(prefer.geoarea[i].part_name);
      }
      return 1;
    }
  }

  if (src_dir) strcpy(prefer.src_dir,src_dir);
  strcpy(globinfo.src_dir,prefer.src_dir);
  finish_path(globinfo.src_dir);

  if (dest_dir) strcpy(prefer.dest_dir,dest_dir);
  strcpy(globinfo.dest_dir,prefer.dest_dir);
  finish_path(globinfo.dest_dir);

  related_dirs(&globinfo,&prefer);

  strcpy(globinfo.lut.dir,prefer.lut_dir);
  finish_path(globinfo.lut.dir);
  if (!*globinfo.lut.fname)
    strcpy(globinfo.lut.fname,prefer.lut_file);

  strcpy(globinfo.lut.name,globinfo.lut.fname);
  if ((p=strchr(globinfo.lut.name,'.'))) *p=0;
#ifndef __NOGUI__
  if (gui)
  {
    dbase.log=log;
    create_gui(argc,argv);
    return 0;
  }
  else
#endif
  {
    if (!(nogui(&genmode,&prefer,test,log)))
    {
      fprintf(stderr,"ERROR: Can't read XRIT source directory '%s'\n",
                                        (src_dir? src_dir : prefer.src_dir));
      return 1;
    }
  }

  return 0;
}
