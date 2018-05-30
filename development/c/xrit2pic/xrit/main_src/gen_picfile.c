/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * File generation functions
 ********************************************************************/
#include "xrit2pic.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// Determine what to add in pgm header
#define ADD_IMG_OBS_TIME
#define ADD_CCSDSn

extern PREFER prefer;
extern GLOBINFO globinfo;

void set_lminmax(guint16 *pix,LUMINFO *li,int pic_lummax,int lummax,int *rgbpilmaxrgb)
{
  int lmin=li->lmin;
  int lmax=li->lmax;
  int *lminrgb=li->lminrgb;
  int *lmaxrgb=li->lmaxrgb;
  int ilmin,ilmax;
  int i;
  if (lmax>lmin)
  {
    for (i=0; i<3; i++)
    {
      if (li->sep_rgblum)
      {
        ilmin=lminrgb[i];
        ilmax=lmaxrgb[i];
        pix[i]=(pic_lummax/2*(MAX(pix[i]-ilmin,0)))/(ilmax-ilmin);
      }
      ilmin=lmin;
      ilmax=lmax;
      pix[i]=(lummax*(MAX(pix[i]-ilmin,0)))/(ilmax-ilmin);

#ifdef XXX
wat was dit toch voor rotzooi... (2-5-2009)
      pix[i]=(lummax*(MAX(pix[i]-lminrgb[i],0)))/(lmaxrgb[i]-lminrgb[i]);
      if (rgbpilmaxrgb[i])
      {
        pix[i]=pic_lummax/2*(MAX(pix[i]-lmin,0))/(lmax-lmin);
      }
#endif
    }
  }
}

static void add_str(PIC_CHARS *pc,PLOTTEXT pt,int type,guchar *line,int y0)
{
  char ts[800]; // max. str=800/8=100 karakters
  int i;
  int size,ytxt;
  size=pt.size;
  if (!size)
    size=pc->o_height/450;
  if (size<1) size=1;

  ytxt=(y0-pt.y)/size;
  if ((ytxt>=0) && (ytxt<8))
  {
    get_strline(pt.str,(ytxt),ts,790);
    for (i=0; i<strlen(pt.str)*8*size; i++)
    {
      if (i+pt.x >= pc->o_width) break;
      if (type=='c')
      {
        line[3*(i+pt.x)+0]= ts[i/size];
        line[3*(i+pt.x)+1]= ts[i/size];
        line[3*(i+pt.x)+2]= ts[i/size];
      }
      else if (!strchr("sS",type))
      {
        line[2*(i+pt.x)+0]= ts[i/size];
        line[2*(i+pt.x)+1]= ts[i/size];
      }
      else
      {
        line[i+pt.x]= ts[i/size];        
      }
    }
  }
}


/***********************************************
 * Convert full line in rgbl into line to save:
 *  .  Anaglyph conversion
 *  .  Do x-zoom
 *  .  mirroring
 *  .  Lum adaption
 *  .  Inverting
 *  .  Add overlay
 *  .  Add text
 ***********************************************/
static void convert_line(guint16 *rgbl[4],guchar *line,GROUP *grp,PIC_CHARS *pc,GENMODES *gmode,gboolean flip,int type,int y0,float lat)
{
  int i;
  int lummax;
  gboolean is_space;
  gboolean invert;
  int x0,x1,x;
  int ovlrgb[3],llrgb[3];
  int lmin=gmode->lmin;
  int lmax=gmode->lmax;
  int lminrgb[3],lmaxrgb[3];
  float gamma=gmode->gamma;
  int gtbl[0x400];
  int pic_lummax=(1<<pc->depth)-1;
  LUMINFO li;
  if (!gamma) gamma=1.;
  if (lmin==lmax) { lmin=0; lmax=0x3ff; }
  li.lmin=lmin;
  li.lmax=lmax;
  li.sep_rgblum=FALSE;
  for (i=0; i<3; i++)
  {
    if (gmode->lmaxrgb[i])  // Use individual lum-settings for RGB
    {
      li.sep_rgblum=TRUE;
      lminrgb[i]=gmode->lminrgb[i];
      lmaxrgb[i]=gmode->lmaxrgb[i];
    }
    else                    // Use common lum-settings
    {
      lminrgb[i]=lmin;
      lmaxrgb[i]=lmax;
    }

    if (lmaxrgb[i]==lminrgb[i])      // Make sure no division by zero!
    {
      if (lminrgb[i]) lminrgb[i]--; else lmaxrgb[i]++;
    }
    li.lminrgb[i]=lminrgb[i];
    li.lmaxrgb[i]=lmaxrgb[i];
  }

  if (pc->depth>8)
  {
    if (gmode->adapt_lum)
    {
      lummax=0x3ff;
    }
    else
    {
      switch(type)
      {
        case 'T': lummax=0xff;
        case 'S': lummax=0xff<<pc->bitshift; break;  /* 1 byte from 2 bytes */
        case 'c': lummax=0xff<<pc->bitshift; break;  /* compose; 1 byte from 2 bytes */
        default:  lummax=0x3ff;
      }
    }
  }
  else
  {
    lummax=0xff;
  }

  if (gamma!=1.)
  {
    int i;
    for (i=0; i<lummax; i++)
    {
      gtbl[i]=igamma(i,lummax,gamma);
    }
  }

  ovlrgb[0]=((prefer.ovl_clr>>8)&0xf)*gmode->ol_lum/0xf;
  ovlrgb[1]=((prefer.ovl_clr>>4)&0xf)*gmode->ol_lum/0xf;
  ovlrgb[2]=((prefer.ovl_clr>>0)&0xf)*gmode->ol_lum/0xf;
  llrgb[0]=((prefer.lonlat_clr>>8)&0xf)*gmode->ol_lum/0xf;
  llrgb[1]=((prefer.lonlat_clr>>4)&0xf)*gmode->ol_lum/0xf;
  llrgb[2]=((prefer.lonlat_clr>>0)&0xf)*gmode->ol_lum/0xf;

  invert=gmode->spm.invert;
  if ((is_ir_group(grp)) && (gmode->spm.inv_ir)) invert=(invert? FALSE : TRUE); // in case single IR channel selected

  for (x0=0; x0<pc->o_width; x0++)
  {
    guint16 pix[3]={0,0,0};
    gboolean overlay=FALSE;
    gboolean lonlat=FALSE;
    gboolean fire=FALSE;
    gboolean mark=FALSE;
    
    if (flip)
    {
      x1=pc->o_width-1-x0;
    }
    else
    {
      x1=x0;
    }
    x=x1*gmode->dx+pc->o_xoffset;

    if ((pc->orbittype==POLAR) && (pc->avhrr_lin) && (pc->gmap==normal))
    {
      x= avhrr_linearize(x,pc->width);
    }

    if ((x<0) || (x>=pc->width))       /* out-of-range */
      continue;

    if (rgbl[0]) pix[0]=rgbl[0][x];            /* catch pixel */
    if (rgbl[1]) pix[1]=rgbl[1][x];            /* catch pixel */
    if (rgbl[2]) pix[2]=rgbl[2][x];            /* catch pixel */
    if (pix[0]==0) is_space=TRUE; else is_space=FALSE;
    if (gmode->tempmap)  // use 0...255 for standard temp. translation
    {
      float t;
      if (grp->pro_epi)
      {
        if ((grp->chan->chan_nr>=4) && (grp->chan->chan_nr<12))
        {
          t=pix2temp(grp->pro_epi->pro,grp->chan,pix[0]);
          pix[0]=temp2val(t);
          pix[1]=pix[0];
          pix[2]=pix[0];
        }
      }
    }
    else if (((pc->map_temp)||(pc->map_temp_G_mA)) && (grp->pro_epi) && (is_ir_group(grp)))
    {  // single channel, IR

      if (pc->map_temp_G_mA)
      {
        pix[0]=temp2val(pix2temp(grp->pro_epi->pro,grp->chan,pix[0]));
        pix[1]=pix[2]=pix[0];
      }
      else
      {
        temp_clrmap(pix2temp(grp->pro_epi->pro,grp->chan,pix[0]),pix);
        if (gmode->adapt_lum)
        {
          set_lminmax(pix,&li,pic_lummax,lummax,li.lmaxrgb);
        }
      }
    }
    else
    {
/* if adapt_lum then adapt lum to settings defined in lmin and lmax */
      if (gmode->adapt_lum)
      {
        set_lminmax(pix,&li,pic_lummax,lummax,li.lmaxrgb);
        if (lmax>lmin)
        {
          if (gamma!=1.)
          {
            pix[0]=gtbl[pix[0]];
            pix[1]=gtbl[pix[1]];
            pix[2]=gtbl[pix[2]];
          }
        }
      }
    }
    if ((invert) && (!is_space))
    {
      pix[0]=MAX(lummax-pix[0],0);
      pix[1]=MAX(lummax-pix[1],0);
      pix[2]=MAX(lummax-pix[2],0);
    }

    overlay=(rgbl[3]) && (rgbl[3][x]&CMASK) && (gmode->add_overlay);
    lonlat =(rgbl[3]) && (rgbl[3][x]&LMASK) && (gmode->add_lonlat);
    fire   =(rgbl[3]) && (rgbl[3][x]&FMASK) && (globinfo.spm.fire_detect);
    mark   =(rgbl[3]) && (rgbl[3][x]&MMASK) && (globinfo.add_marks);
/* Here we have pix at pos. (x0+xa,y0+ya); boolean overlay for this pix. */

    if (type=='c')  /* color */
    {
      unsigned char rgb[3]={0,0,0};
/* Translation from depth > 8 to 8 => choose which bits to save (bitshift) */
      if (pc->depth>8)
      {
        if (gmode->adapt_lum)  // lum determined by lmin/lmax; max. pix=3ff
        {                      // -> set to 0ff
          rgb[0]=MIN(pix[0]>>2,0xff);
          rgb[1]=MIN(pix[1]>>2,0xff);
          rgb[2]=MIN(pix[2]>>2,0xff);
        }
        else                   // lum determined by bitshift
        {
          rgb[0]=MIN(pix[0]>>pc->bitshift,0xff);
          rgb[1]=MIN(pix[1]>>pc->bitshift,0xff);
          rgb[2]=MIN(pix[2]>>pc->bitshift,0xff);
        }
      }
      else                     // lum is max. ff; don't use bitshift
      {
        rgb[0]=MIN(pix[0],0xff);
        rgb[1]=MIN(pix[1],0xff);
        rgb[2]=MIN(pix[2],0xff);
      }

      if (gmode->agl.ol_3d)
      {
        if ((rgbl[3]) && (rgbl[3][x]&(CMASK|CLMASK)))
        {
          if (rgbl[3][x]&CLMASK)
          {
            rgb[0]=MIN(rgb[0]+gmode->ol_lum,0xff);
          }
          if (rgbl[3][x]&CMASK)
          {
            rgb[1]=MIN(rgb[1]+gmode->ol_lum,0xff);
            rgb[2]=MIN(rgb[2]+gmode->ol_lum,0xff);
          }
        }
      }
      else
      {
        if (overlay)
        {
          rgb[0]=MIN(rgb[0]+ovlrgb[0],0xff);
          rgb[1]=MIN(rgb[1]+ovlrgb[1],0xff);
          rgb[2]=MIN(rgb[2]+ovlrgb[2],0xff);
        }
        if (lonlat)
        {
          rgb[0]=MIN(rgb[0]+llrgb[0],0xff);
          rgb[1]=MIN(rgb[1]+llrgb[1],0xff);
          rgb[2]=MIN(rgb[2]+llrgb[2],0xff);
        }
        if (fire)
        {
          rgb[0]=0xff;
          rgb[1]=0;
          rgb[2]=0;
        }
        if (mark)
        {
          rgb[0]=0xff;
          rgb[1]=0;
          rgb[2]=0;
        }
      }
      memcpy(line+x0*3,rgb,3);
    }
    else /* !pc->is_color */
    {
      if (type=='s')  /* 1-byte per pixel */
      {
        guint8 ch8;
        ch8=MIN(pix[0],0xff);
        if (overlay) ch8=MIN((int)ch8+gmode->ol_lum,0xff);
        if (lonlat)  ch8=MIN((int)ch8+gmode->ol_lum,0xff);
        if (mark)    ch8=MIN((int)ch8+gmode->ol_lum,0xff);
        if (fire)    ch8=MIN((int)ch8+gmode->ol_lum,0xff);
        line[x0]=ch8;
      }
      else if (type=='S') /* 2-bytes to 1 byte pp */
      {
        guint8 ch8;
        if (gmode->adapt_lum)  // lum determined by lmin/lmax; max. pix=3ff
          ch8=MIN((pix[0]>>2),0xff);
        else                   // lum determined by bitshift
          ch8=MIN(pix[0]>>pc->bitshift,0xff);

        if (overlay) ch8=MIN((int)ch8+gmode->ol_lum,0xff);
        if (lonlat)  ch8=MIN((int)ch8+gmode->ol_lum,0xff);
        if (mark)    ch8=MIN((int)ch8+gmode->ol_lum,0xff);
        if (fire)    ch8=MIN((int)ch8+gmode->ol_lum,0xff);

        line[x0]=ch8;
      }
      else                   /* 2 bytes pp */
      {
        guint16 ch16;
        ch16=pix[0];
        if (overlay) ch16=MIN((int)ch16+(0x3ff*gmode->ol_lum/0xff),0x3ff);
        if (lonlat)  ch16=MIN((int)ch16+(0x3ff*gmode->ol_lum/0xff),0x3ff);
        if (mark)    ch16=MIN((int)ch16+(0x3ff*gmode->ol_lum/0xff),0x3ff);
        if (fire)    ch16=MIN((int)ch16+(0x3ff*gmode->ol_lum/0xff),0x3ff);

        ch16=GUINT16_TO_BE(ch16);
        memcpy(line+x0*2,&ch16,2);
      }
    }

  }
  if (*gmode->pt.str)  add_str(pc,gmode->pt,type,line,y0);
  if (*gmode->pt2.str) add_str(pc,gmode->pt2,type,line,y0);

#ifdef XXX
  if (*gmode->pt.str)
  {
    char ts[200];
    int i;
    int size,ytxt;
    size=pc->o_height/450;
    if (size<1) size=1;
    ytxt=(y0/size) - gmode->pt.y;
    if ((ytxt>=0) && (ytxt<8))
    {
      get_strline(gmode->pt.str,(ytxt),ts,190);
      for (i=0; i<strlen(gmode->pt.str)*8*size; i++)
      {
        if (i+gmode->pt.x >= pc->o_width) break;
        if (type=='c')
        {
          line[3*(i+gmode->pt.x)+0]= ts[i/size];
          line[3*(i+gmode->pt.x)+1]= ts[i/size];
          line[3*(i+gmode->pt.x)+2]= ts[i/size];
        }
        else if (!strchr("sS",type))
        {
          line[2*(i+gmode->pt.x)+0]= ts[i/size];
          line[2*(i+gmode->pt.x)+1]= ts[i/size];
        }
        else
        {
          line[i+gmode->pt.x]= ts[i/size];        
        }
      }
    }
  }
#endif
}

extern GtkWidget *progress_gen;

//#define TSTRLEN 900
/***********************************************
 * Generate one pic file (jpeg or pgm/ppm)
 * Output characteristics are defined in gmode.
 *   May be part of a movie.
 ***********************************************/
int gen_a_picfile(GROUP *grp_sel,         // group containing all channels for pic
                  PIC_CHARS *pc,
                  char *fno,              // output file
                  gboolean keep_chnk,     // keep generated chunk in memory
		  gboolean show_progress, // show progress translation
		  GENMODES *gmode)        // generate-characteristics
{
  FILE *fpo;
  GtkWidget *progress=NULL;
  gboolean do_xymapping=FALSE;
  ANAGL *anagl=NULL;

  int y,y1,y0;
  int nry_written=0,y_fact=1;
  gboolean flip;
  guchar *line;
  int retval=0;
  int bpp;
  int type=0;
  char *tstr=NULL;
  float lat;
  guint16 *rgbl1[4]={NULL,NULL,NULL,NULL};
//  DEBUG_STR("gen_a_picfile");

//  if (globinfo.overlaytype=='v') globinfo.overlaytype='V';
  do_xymapping=((pc->orbittype==GEO) && (pc->gmap!=normal));

  if ((pc->orbittype==GEO) && ((pc->gmap==polar_n) || (pc->gmap==polar_s)))
    y_fact=2;      // werk-omheen: polar: halve hoogte

 /* Set some output-format parameters */
  if ((gmode->gen_film) || (gmode->image_oformat=='P'))
  {
    gmode->odepth=8;
  }
  else
  {
    gmode->odepth=pc->depth;
  }

/* next only for error message 'overlay not found' */
  if ((globinfo.add_overlay) && (grp_sel->orbittype!=POLAR))
  {
#ifndef __NOGUI__
    if (show_progress)
      get_overlay(prefer.overlay,globinfo.overlaytype,grp_sel->chan,(void *)Create_Message);
    else
#endif
      get_overlay(prefer.overlay,globinfo.overlaytype,grp_sel->chan,NULL);
  }
  if (globinfo.dim3)
  {
    anagl=&gmode->agl;
    pc->is_color=TRUE;
  }
  if (globinfo.spm.fire_detect) pc->is_color=TRUE;

  if (gmode->image_oformat=='j'){ bpp=3; type='c'; }  /* JPEG 24, BW */
  else if (pc->is_color)        { bpp=3; type='c'; }  /* color */
  else if (globinfo.dim3)       { bpp=3; type='c'; }  /* color */
  else if (pc->depth<=8)        { bpp=1; type='s'; }  /* 1 byte */
  else if (gmode->odepth<=8)    { bpp=1; type='S'; }  /* 1 byte from 2 bytes */
  else                          { bpp=2; type='d'; }  /* 2 bytes */

#ifdef SCHAKEL_TEMPMAP
// alleen voor test! Ergens anders deze keuze maken, OOK voor preview!
// als wel compose maar geen kleur dan maak temp.map in file.
  if ((globinfo.spm.compose) && (!pc->is_color))
  {
    if ((grp_sel->pro_epi) && (grp_sel->pro_epi->pro))
    {
      gmode->tempmap=TRUE;
    }
  }
#endif

  if ((pc->scan_dir!='n') && ((prefer.scandir_correct)||(gmode->adapt_lum)))
    flip=TRUE;
  else
    flip=FALSE;
  line=(guchar *)malloc(pc->o_width*bpp);
  if (!line)
  {
//    if (globinfo.overlaytype=='V') globinfo.overlaytype='v';
    return 1;
  }
  if (!(fpo=fopen(fno,"wb")))
  {
//    if (globinfo.overlaytype=='V') globinfo.overlaytype='v';
    return Open_Wr;
  }

  sprintfd(&tstr,"%s  ",pc->chan);
  {
    char time[100];
    strftime(time,100,"%y-%m-%d %H:%M\n",&grp_sel->grp_tm);
    sprintfd(&tstr,"%s%s",tstr,time);
  }
#ifdef ADD_CCSDS
  {
    SEGMENT *segm;
    int j;
    FILE *fp;
    sprintfd(&tstr,"%sCCSDS:(",tstr);
    for (segm=grp_sel->chan->segm; segm; segm=segm->next)
    {
      // read all headers for CCSDS time stamp
      if (fp=open_xritimage(segm->pfn,&segm->xh,segm->xh.xrit_frmt,segm->chan)) fclose(fp);
      sprintfd(&tstr,"%s%d:",tstr,(flip? grp_sel->chan->nr_segm+1-segm->xh.segment : segm->xh.segment));
      for (j=0; j<7; j++) sprintfd(&tstr,"%s%02x",tstr,(unsigned char)segm->xh.ccsds[j]);
      sprintfd(&tstr,"%s,",tstr);
    }
    tstr[strlen(tstr)-1]=0;
    sprintfd(&tstr,"%s)\n",tstr);
  }
#endif
#ifdef ADD_IMG_OBS_TIME
    SEGMENT *segm;
    FILE *fp;
    sprintfd(&tstr,"%sHEADERTYPE 131:{\n",tstr);
    for (segm=grp_sel->chan->segm; segm; segm=segm->next)
    {
      // read all headers for CCSDS time stamp
      tfreenull(&segm->xh.img_obs_time);
      if ((fp=open_xritimage(segm->pfn,&segm->xh,segm->xh.xrit_frmt,segm->chan))) fclose(fp);
      if (!segm->xh.img_obs_time) continue;
      sprintfd(&tstr,"%s%d:",tstr,(flip? grp_sel->chan->nr_segm+1-segm->xh.segment : segm->xh.segment));
      sprintfd(&tstr,"%s%s\n",tstr,segm->xh.img_obs_time);
    }
    sprintfd(&tstr,"%s}\n",tstr);

#endif

// calibration only if 1 channel selected
  if (!grp_sel->chan->next)
  {
    int i;
    sprintfd(&tstr,"%sCal_Offset=%f\n",tstr,grp_sel->chan->Cal_Offset[0]);
    sprintfd(&tstr,"%sCal_Slope=%f\n",tstr,grp_sel->chan->Cal_Slope[0]);
    if (grp_sel->chan->cal.nrcalpoints)
    {
      sprintfd(&tstr,"%sCAL_TBL:%s {\n",tstr,
        (grp_sel->chan->cal.caltbl_type=='k'? "KELVIN" : grp_sel->chan->cal.caltbl_type=='a'? "ALBEDO" : "PERCENT"));
      for (i=0; i<grp_sel->chan->cal.nrcalpoints; i++)
      {
        sprintfd(&tstr,"%s%d,%.2f\n",tstr,
          (int)(grp_sel->chan->cal.caltbl[0][i]+0.1),grp_sel->chan->cal.caltbl[1][i]);
      }
      sprintfd(&tstr,"%s}\n",tstr);
    }
  }

// Add lon/lat info
  switch(gmode->gmap)
  {
    case normal:
    {
      sprintfd(&tstr,"%scoff=%ld\n",tstr,(long)pc->gchar.coff);
      sprintfd(&tstr,"%scfac=%ld\n",tstr,(long)pc->gchar.cfac);
      sprintfd(&tstr,"%sloff=%ld\n",tstr,(long)pc->gchar.loff);
      sprintfd(&tstr,"%slfac=%ld\n",tstr,(long)pc->gchar.lfac);
      sprintfd(&tstr,"%slon_subsat=%.2f\n",tstr,pc->gchar.sub_lon);
      break;
    }
    case plate_carree:
    {
      float lon_o,lon_i,lat_o,lat_i;
      if (grp_sel->orbittype==POLAR)
      {
        int xoff=pc->xoff;
        int xfac=pc->xfac;
        int yoff=pc->yoff;
        int yfac=pc->yfac;
        if (!xfac)
        {
          xfac=pc->width/180;
          xoff=90;
        }
        if (!yfac)
        {
          yfac=pc->height/180;
          yoff=90;
        }
        dxy2lonlat_area(0,0,xoff,yoff,xfac,yfac,&lon_o,&lat_o,pc);
        dxy2lonlat_area(pc->width-1,pc->height-1,xoff,yoff,xfac,yfac,&lon_i,&lat_i,pc);
        lon_i=(lon_i-lon_o)/pc->width;
        lat_i=(lat_i-lat_o)/pc->height;
      }
      else
      {
// Toevoegen: bereik als ingezoemd
        lon_o=-90.;
        lon_i=180./(float)pc->width;
        lat_o=90.;
        lat_i=-180./(float)pc->height;
      }
      sprintfd(&tstr,"%slon_offs=%.5f\n",tstr,lon_o);
      sprintfd(&tstr,"%slat_offs=%.5f\n",tstr,lat_o);
      sprintfd(&tstr,"%slon_incr=%.5f\n",tstr,lon_i);
      sprintfd(&tstr,"%slat_incr=%.5f\n",tstr,lat_i);
      break;
    }
    default: break;
  }

  if (gmode->image_oformat=='j')
  {
    if (!(write_jpghdr(fpo,pc->o_width,pc->o_height/y_fact,8,75)))
    {
//      if (globinfo.overlaytype=='V') globinfo.overlaytype='v';
      if (tstr) free(tstr); tstr=NULL;
      return Aborted;  // jpeg err; message already given
    }
  }
  else /* if (gmode->image_oformat=='p') */
  {
#ifdef DONT_CLIP
    gmode->odepth=16;
#endif
    write_pgmhdr(fpo,pc->o_width,pc->o_height/y_fact,gmode->odepth,pc->is_color,tstr);
  }
  if (tstr) free(tstr);

  if (show_progress)
  {
    if (gmode->image_oformat!='j')
      progress=Create_Progress(NULL,"Translate...",TRUE);
    else
    {
      /* !!! ABorting JPEG: Crash! Something wrong with:
            ERREXIT(cinfo, JERR_TOO_LITTLE_DATA); (jerror.h)
         in
            jpeg_finish_compress(cinfo);          (jcapimin.c)
      (
      #define ERREXIT(cinfo,code)  \
        ((cinfo)->err->msg_code = (code), \
         (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
      )
         Probably def. of error func not OK.
      */

      progress=Create_Progress(NULL,"Translate...",FALSE);
    }
  }

#define POS_TID_X 8
#define POS_TID_Y 8
  *gmode->pt.str=0;
  if (gmode->timeid)
  {
    strftime(gmode->pt.str,100,"%Y-%m-%d %H:%M",&grp_sel->grp_tm);
    if (!gmode->pt.x) gmode->pt.x=POS_TID_X;
    if (!gmode->pt.y) gmode->pt.y=POS_TID_Y;
  }
  if (!gmode->pt2.x) gmode->pt2.x=POS_TID_X;
  if (!gmode->pt2.y) gmode->pt2.y=POS_TID_Y+8;

  if (!gmode->dx) gmode->dx=1;
  if (!gmode->dy) gmode->dy=1;

  if (do_xymapping)
  {
    grp_sel->keep_in_mem=TRUE;
    rgbl1[0]=calloc(pc->width,sizeof(**grp_sel->rgbpicstr));
    rgbl1[1]=calloc(pc->width,sizeof(**grp_sel->rgbpicstr));
    rgbl1[2]=calloc(pc->width,sizeof(**grp_sel->rgbpicstr));
    rgbl1[3]=calloc(pc->width,sizeof(**grp_sel->rgbpicstr));
  }

  for (y0=0; y0<pc->height; y0+=y_fact)
  {
    guint16 *rgbl[4]={NULL,NULL,NULL,NULL};
    if (flip)
      y1=pc->o_height-1-y0;
    else
      y1=y0;
      
    y=y1*gmode->dy+pc->o_yoffset;
    if (!do_xymapping)
    {
      /* Do channel mapping and convert init. value rgbl[] MUST be 3xNULL! */
      get_composed_line(grp_sel,y,rgbl,anagl,globinfo.spm.fire_detect,globinfo.lut.use);
      Add_Overlaybits(grp_sel,rgbl[3],y);
      convert_line(rgbl,line,grp_sel,pc,gmode,flip,type,y0,lat);
    }
    else
    {
      int x,xo,yo;
      for (x=0; x<pc->width; x++)
      {
        if (dxy2fxy(x,y,&xo,&yo,&grp_sel->pc))
        {
          rgbl1[0][x]=0;
          rgbl1[1][x]=0;
          rgbl1[2][x]=0;
          rgbl1[3][x]=0;
        }
        else
        {
          store_composed_line(grp_sel,yo,rgbl,anagl,globinfo.spm.fire_detect,globinfo.lut.use);
          if (rgbl[0]) rgbl1[0][x]=rgbl[0][xo];            /* catch pixel */
          if (rgbl[1]) rgbl1[1][x]=rgbl[1][xo];            /* catch pixel */
          if (rgbl[2]) rgbl1[2][x]=rgbl[2][xo];            /* catch pixel */
          if (rgbl[3]) rgbl1[3][x]=rgbl[3][xo];            /* catch pixel */
        }
      }
      convert_line(rgbl1,line,grp_sel,pc,gmode,flip,type,y0,lat);
    }
    
    if (nry_written++>=(pc->o_height/y_fact)) break;
    if (gmode->image_oformat=='j')
    {
      if (write_jpgline(line)) break;
    }
    else /* if (gmode->image_oformat=='p') */
    {
      fwrite(line,pc->o_width,bpp,fpo);
    }

/*
   If get_composed_line used: deallocate rgbl.
   If store_composed_line used Don't deallocate; is attached to grp.
      Will be deallocated by Remove_Grp!
*/
    if (!do_xymapping)
    {
      if (rgbl[0]) free(rgbl[0]); rgbl[0]=NULL;
      if (rgbl[1]) free(rgbl[1]); rgbl[1]=NULL;
      if (rgbl[2]) free(rgbl[2]); rgbl[2]=NULL;
      if (rgbl[3]) free(rgbl[3]); rgbl[3]=NULL;
    }

    if (show_progress)
    {
      if (!(y0%100))
      {
        if (Update_Progress(progress,y0,pc->o_height))
        {
          #if PARANOIDE > 0
            Create_Message("Warning","Aborted after %d of %d lines.",y0,pc->o_height);
          #endif
          retval=Aborted;
          break;
        }
        if (retval)
        {
          #if PARANOIDE > 1
            Create_Message("Error","One or more segments missing.");
          #endif
        }
      }
    }
    else if (progress_gen)
    {
      if (Get_Progress_state(progress_gen))
      {
        retval=Aborted;
        if (gmode->image_oformat!='j') break;
      }
#ifndef __NOGUI__
      while (g_main_iteration(FALSE));
#endif
    }
  }

  if (rgbl1[0]) free(rgbl1[0]); rgbl1[0]=NULL;
  if (rgbl1[1]) free(rgbl1[1]); rgbl1[1]=NULL;
  if (rgbl1[2]) free(rgbl1[2]); rgbl1[2]=NULL;
  if (rgbl1[3]) free(rgbl1[3]); rgbl1[3]=NULL;

  /* Deallocate */
  free(line);
  if (globinfo.add_overlay)      add_overlay(&prefer,0,NULL,NULL); // free mem
  if (gmode->image_oformat=='j') close_wr_jpeg();             // free jpeg-things
  if (show_progress)             Close_Progress(progress);

  fclose(fpo);
//  if (globinfo.overlaytype=='V') globinfo.overlaytype='v';
  return retval;
}

