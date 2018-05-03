/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include <string.h>

extern GLOBINFO globinfo;
extern PREFER prefer;


/*******************************************************************
 * Convert 'Europe' boundaries for geo-mappings.
 * fix_... = size-spec "Europe"
 ********************************************************************/
static void conv_bnd(struct picpart *pp,PIC_CHARS *pc)
{
  float alpha;
  int x1,x2;
  int y1,y2;
  y1=pp->fix_hoffset;
  y2=pp->fix_hoffset+pp->fix_height;
  x1=pp->fix_woffset;
  x2=pp->fix_woffset+pp->fix_width;

  y1=inv_ymap(y1,&alpha,pc);
  x1=inv_xmap(x1,alpha,pc);

  y2=inv_ymap(y2,&alpha,pc);
  x2=inv_xmap(x2,alpha,pc);

  pp->fix_height=y2-y1;
  pp->fix_hoffset=y1;
  pp->fix_width=x2-x1;
  pp->fix_woffset=x1;
}

/*******************************************************************
 * Prepair generation.
 ********************************************************************/
/* Determine size pic in file 
 * Problem with HRV and movies: pc->width changes between frames!
 *
 * rgbi->width/height    = size drawable
 * rgbpi->pwidth/pheight = size array filled with pic (normally size of pic itself)
 * pc->o_width/o_height  = size generated pic.
 *   Differs from org. pic if:
 *     avhrr: doubled width to do linearizing
 *     HRV: width depends on shift north/south
 *     correction is done if pixel isn't square
 * HRV only:
 *   pc->org_width         = pc->width or pc->height (HRV full width)
*/
static void size_pic(GROUP *grp_sel,GENMODES *gmode)
{
  PIC_CHARS *pc=&grp_sel->pc;
  CHANNEL *hrvchan=NULL;
  int width;
  struct picpart pp_norm;
  struct picpart pp_hrv;
  struct point center;
  struct point delta;

  pp_norm=gmode->geoarea.pp_norm;
  pp_hrv=gmode->geoarea.pp_hrv;
  center=gmode->geoarea.center;
  delta=gmode->geoarea.delta;

  width=pc->width;

  if (Chan_Contributes(grp_sel,"HRV"))  // contains HRV
  {
    pc->org_width=pc->height;
    hrvchan=Get_Chan(grp_sel->chan,"HRV");
  }
  else
  {
    pc->org_width=pc->width;
  }
/* Actual boundaries globe */
  if (pc->scan_dir=='s')
  {
    pc->x_w=pc->width-1;
    pc->y_n=pc->height-1;
    pc->x_e=0;
    pc->y_s=0;
  }
  else
  {
    pc->x_w=0;
    pc->y_n=0;
    pc->x_e=pc->width-1;
    pc->y_s=pc->height-1;
  }
  if (pc->y_s > pc->x_e) pc->y_s=pc->x_e;  // correction e.g. GOES
  
  if (gmode->gen_film)
  {
    if (gmode->frame_minwidth) width=gmode->frame_minwidth;
  }

  if (!pp_norm.fix_width)
  {
    if (pp_norm.pct_width)
    {
      pp_norm.fix_width=width*pp_norm.pct_width/100;
      pp_norm.fix_woffset=width*pp_norm.pct_woffset/100;
    }
    else
    {
      pp_norm.fix_width=width*22/100;
      pp_norm.fix_woffset=width*36/100;
    }
  }
  if (!pp_norm.fix_height)
  {
    if (pp_norm.pct_height)
    {
      pp_norm.fix_height=pc->height*pp_norm.pct_height/100;
      pp_norm.fix_hoffset=pc->height*pp_norm.pct_hoffset/100;
    }
    else
    {
      pp_norm.fix_height=pc->height*17/100;
      pp_norm.fix_hoffset=pc->height*82/100;
    }
  }
  if ((pc->scan_dir=='n') && ((!delta.lon) || (!delta.lat)))
  {
    pp_norm.fix_hoffset=pc->height-pp_norm.fix_hoffset-pp_norm.fix_height;
    pp_norm.fix_woffset=pc->width-pp_norm.fix_woffset-pp_norm.fix_width;
  }
  conv_bnd(&pp_norm,pc);

  if (!pp_hrv.fix_width)
  {
    int hrvwidth=7600; // width;   // only O if 'europe' side is upper part
    if (pp_hrv.pct_width)
    {
      pp_hrv.fix_width=hrvwidth*pp_hrv.pct_width/100;
      pp_hrv.fix_woffset=hrvwidth*pp_hrv.pct_woffset/100;
    }
    else
    {
      pp_hrv.fix_width=hrvwidth*33/100;
      pp_hrv.fix_woffset=hrvwidth*51/100; // /2+100;
    }
  }
  if (!pp_hrv.fix_height)
  {
    if (pp_hrv.pct_height)
    {
      pp_hrv.fix_height=pc->height*pp_hrv.pct_height/100;
      pp_hrv.fix_hoffset=pc->height*pp_hrv.pct_hoffset/100;
    }
    else
    {
      pp_hrv.fix_height=pc->height*17/100;
      pp_hrv.fix_hoffset=pc->height*82/100;
    }
  }
  if ((pc->scan_dir=='n') && ((!delta.lon) || (!delta.lat)))
  {
    pp_hrv.fix_hoffset=pc->height-pp_hrv.fix_hoffset-pp_hrv.fix_height;
    pp_hrv.fix_woffset=pc->width-pp_hrv.fix_woffset-pp_hrv.fix_width;
  }
  conv_bnd(&pp_hrv,pc);

  if (IS_GEO(grp_sel))
  {
    if (hrvchan)                           // contains HRV
    {
      pc->gchar.shift_ypos   =hrvchan->shift_ypos;
      pc->gchar.upper_x_shift=hrvchan->upper_x_shift;
      pc->gchar.lower_x_shift=hrvchan->lower_x_shift;
    }
    else
    {
      if (grp_sel->chan)                   // expected always !=0
      {
        pc->gchar.shift_ypos   =grp_sel->chan->shift_ypos;
        pc->gchar.upper_x_shift=grp_sel->chan->upper_x_shift;
        pc->gchar.lower_x_shift=grp_sel->chan->lower_x_shift;
      }
    }
  }

  if ((gmode->area_nr) && (grp_sel->orbittype==GEO))
  {
    if (hrvchan)                             // contains HRV
    {
      pc->o_width=pp_hrv.fix_width;
      pc->o_xoffset=pp_hrv.fix_woffset;      // Offset from first pixel of line = east
      pc->o_height=pp_hrv.fix_height;
      pc->o_yoffset=pp_hrv.fix_hoffset;      // Offset from first line = south
    }
    else
    {
      pc->o_width=pp_norm.fix_width;
      pc->o_xoffset=pp_norm.fix_woffset;
      pc->o_height=pp_norm.fix_height;
      pc->o_yoffset=pp_norm.fix_hoffset;     // Offset from first line = south
    }
    pc->gmap=gmode->gmap;
  }
  else if (grp_sel->orbittype==POLAR)
  {
    pc->o_width=width;
    pc->o_xoffset=0;
    pc->o_height=pc->height;
    pc->o_yoffset=0;
    pc->avhrr_lin=gmode->avhrr_lin;
    pc->avhrr_lin_dblx=gmode->avhrr_lin_dblx;
    if ((pc->avhrr_lin) && (pc->avhrr_lin_dblx)) pc->o_width*=2;
  }
  else
  {
    pc->o_width=width;
    pc->o_xoffset=0;
    pc->o_height=pc->height;
    pc->o_yoffset=0;
    pc->gmap=gmode->gmap;
    if (pc->pixel_shape)
    {
      if (pc->pixel_shape>1.) pc->o_height*=pc->pixel_shape;
      if (pc->pixel_shape<1.) pc->o_width/=pc->pixel_shape;
    }
  }
}


// new size: o_xoffset ... o_xoffset+o_width
void zoom_size(GENMODES *gmode,PIC_CHARS *pc)
{
  if (!gmode->zx) return;
  if (!gmode->zy) return;
  if (pc->scan_dir=='n')
  {
    pc->o_xoffset+=gmode->ox;
    pc->o_yoffset+=gmode->oy;
  }
  else
  {
    pc->o_xoffset+=(pc->o_width*(1-1/gmode->zx)-gmode->ox);
    pc->o_yoffset+=(pc->o_height*(1-1/gmode->zy)-gmode->oy);
  }
  pc->o_width=pc->o_width/gmode->zx;     // o_width: already taken into account frx!
  pc->o_height=pc->o_height/gmode->zy;
}

void adapt_filmsize(GENMODES *gmode,PIC_CHARS *pc)
{
  gmode->dx=1;
  gmode->dy=1;
  {
    /* Make sure size doesn't exceed predefined value */
    gmode->dx=pc->o_width/prefer.film_size + 1;   // increment x
    gmode->dy=pc->o_height/prefer.film_size + 1;  // increment y
    gmode->dx=MAX(gmode->dx,gmode->dy);           // dx>=dy
    gmode->dy=MAX(gmode->dx,gmode->dy);           // dy>=dx ==> dx=dy=max(org_dx,org_dy)
    pc->o_width/=gmode->dx;                       // new width output
    pc->o_height/=gmode->dy;                      // new height output
    pc->o_width=(pc->o_width/16)*16;              // make width multiple of 16 (needed for jpeg/avi?)
  }
}

void replace_spaces(char *l)
{
  for (; *l; l++) if (*l==' ') *l='_';
}

int replace_str(char *l,char *s1,char *s2)
{
  char *p,*p1;
  int n1=strlen(s1);
  int n2=strlen(s2);
  if ((p=strstr(l,s1)))
  {
    if (n1 >= n2)
    {
      strcpy(p+n2,p+n1);
      strncpy(p,s2,n2);
    }
    else
    {
      for (p1=l+strlen(l)+(n2-n1); p1>p; p1--) *p1=*(p1-(n2-n1));
      strncpy(p,s2,n2);
    
    }
    if (strstr(s2,s1)) return 0;
    return 1;
  }
  return 0;
}


/*
%k		grp->h_or_l
%s		grp->sat_src
%c		pc.chan
%t		timestr
%n		channelnumber
%y/%M/%d/%h/%m	tijd
%r		pc.first_chunck
Example:
"%p%S_%c_%t%r"
*/

static void mk_filename(GROUP *grp,CHANNEL *chan,char *frmt)
{
  PIC_CHARS pc=grp->pc;
  char str[200];
  char timestr[200];
  /* Gen timestring for filename generation */
  *timestr=0;
  strftime(timestr,20,"%y%m%d_%H%M",&grp->grp_tm);

  if (frmt)
  {
    strcpy(pc.fno,frmt);

    sprintf(str,"%c",grp->h_or_l);
    replace_str(pc.fno,"%k",str);

    replace_str(pc.fno,"%s",grp->sat_src);
    replace_str(pc.fno,"%c",pc.chan);
    replace_str(pc.fno,"%t",timestr);

    if ((!grp->selected) || (!grp->pc.is_color))
    {
      sprintf(str,"%02d",chan->chan_nr);
      replace_str(pc.fno,"%n",str);
      replace_str(pc.fno,"%C","");
    }
    else
    {
      replace_str(pc.fno,"%n"," ");
      replace_str(pc.fno,"%C",globinfo.clrmapspec.name_sh);
    }
    
    if ((!grp->selected) && (!chan->selected))
    {
      sprintf(str,"%dto%d",pc.first_chunck,pc.last_chunck);
      replace_str(pc.fno,"%r",str);
    }
    else
    {
      replace_str(pc.fno,"%r","");
    }
    strftime(timestr,200,pc.fno,&grp->grp_tm);
    strcpy(pc.fno,timestr);
  }
  else
  {
    strcpy(pc.fno,pc.picname);
  }
  replace_spaces(pc.fno);
  grp->pc=pc;
}

static void mk_picname(GROUP *grp,CHANNEL *chan1)
{
  PIC_CHARS pc=grp->pc;
  char timestr[100];
  /* Gen timestring for filename generation */
  *timestr=0;
  strftime(timestr,20,"%y%m%d_%H%M",&grp->grp_tm);

/* Determine picname */
  if (((grp->selected) || ((chan1) && (chan1->selected))) && (globinfo.segmrange[0]==0))           /* full picture */
  {
    if (*pc.chan)
      sprintf(pc.picname,"%c%s_%s_%s",grp->h_or_l,grp->sat_src,pc.chan,timestr);
    else
      sprintf(pc.picname,"%c%s_%s",grp->h_or_l,grp->sat_src,timestr);
  }
  else if ((grp->pro_epi) &&
       (((grp->pro_epi->pro) && (grp->pro_epi->pro->selected)) ||
        ((grp->pro_epi->epi) && (grp->pro_epi->epi->selected)))) /* pro/epi */
  {
    sprintf(pc.picname,"%c%s%s_%s",grp->h_or_l,grp->sat_src,pc.chan,timestr);
  }
  else if (pc.first_chunck==pc.last_chunck)
  {
    sprintf(pc.picname,"%c%s_%s_s%d_%s",
                   grp->h_or_l,grp->sat_src,pc.chan,pc.first_chunck,timestr);
  }
  else                                                /* partial picture */
  {
    sprintf(pc.picname,"%c%s_%s_s%dto%d_%s",
                   grp->h_or_l,grp->sat_src,pc.chan,pc.first_chunck,pc.last_chunck,timestr);
  }
  switch(pc.gmap)
  {
    case plate_carree: strcat(pc.picname,"_pc"); break;
    case mercator    : strcat(pc.picname,"_mc"); break;
    case polar_n     : strcat(pc.picname,"_pn"); break;
    case polar_cn    : strcat(pc.picname,"_pn"); break;
    case polar_s     : strcat(pc.picname,"_ps"); break;
    case polar_cs    : strcat(pc.picname,"_ps"); break;
    case normal      :                           break;
  }

  grp->pc=pc;
}

//#define SWAP(a,b) a=a^b; b=a^b; a=a^b   // werkt niet goed hier!??
static void SWAP(int *a,int *b)
{
  int t;
  t=*a; *a=*b; *b=t;
}

static CHANNEL *get_nonhrv(GROUP *grp)
{
  CHANNEL *chan1;
  for (chan1=grp->chan; chan1; chan1=chan1->next)
  {
    if ((chan1->chan_name) && ((strcmp(chan1->chan_name,"HRV")))) break;
  }
  return chan1;
}

// If area not full globe: for the moment ensure ALL segments are present!
#define DETECT_LIM_SEGM 0
/***********************************************
 * Determine some picture characteristics
 ***********************************************/
PIC_CHARS pic_info(GROUP *grp,GtkWidget *wdgt,GENMODES *modes)
{
  CHANNEL *chan,*chan1=NULL;
  SEGMENT *segm;
  gboolean chan1_maybe_not_used=FALSE;
  PIC_CHARS pc=grp->pc;
  /* Clear picinfo struct except...*/
  if (modes) pc.gmap=modes->gmap;         // take over geomapping (waarom niet al in grp->pc?)

  chan1=get_nonhrv(grp);                  // get first non-hrv channel
  if (!chan1) chan1=grp->chan;            // no non-hrv channel: get first channel anyway (will be HRV) */
  if ((chan1) && (!chan1->selected)) chan1_maybe_not_used=TRUE; // channel is not selected

/* If compose and chan1 not selected then find first chan which is selected. */
  if ((!grp->compose) && (chan1) && (!chan1->selected))
  {
    for (chan1=grp->chan; ((chan1) && (!chan1->selected)); chan1=chan1->next);
    if (!chan1)                          // nothing selected:
    {
      chan1=get_nonhrv(grp);             //  get first non-hrv cghannel
      if (!chan1) chan1=grp->chan;       // or get (waarom al dit?)
      if (chan1) chan1_maybe_not_used=TRUE;
    }
  }
  if (!chan1) chan1=grp->pro_epi;  /* if not: asume pro/epi file to get */
  if (!chan1) return pc;          /* if not: error, return */

/* Determine mapping */
  if (grp->compose)
  {
    pc.is_color=TRUE;
    /* Get custom mapping. If no clrmap-window opened: Load default mapping. 
       chan->r,g,b will get multiplication factors for each channel.
    */
    if ((!modes) || (!modes->wnd_clrmap))
    {
      get_map(wdgt,grp->chan,LAB_COLOR,NULL);
    }
    else
    {
      get_map(wdgt,grp->chan,modes->wnd_clrmap,NULL);
    }
  }
  else if ((globinfo.spm.map_temp) || (globinfo.spm.map_temp_G_mA))
  {
    pc.is_color=TRUE;
    pc.map_temp=globinfo.spm.map_temp;
    pc.map_temp_G_mA=globinfo.spm.map_temp_G_mA;
    chan1->r=chan1->g=chan1->b=1.;
  }
  else
  {
    chan1->r=chan1->g=chan1->b=1.;
  } 

/* Determine characteristics of (composed) pic */
  pc.widthmax=0;
  pc.heightmax=0;
  for (chan=grp->chan; chan; chan=chan->next)
  {
    if (!chan->chan_name) continue;
    if ((!strcmp(chan->chan_name,"HRV"))) chan->lum=globinfo.hrv_lum;
    if ((!chan->r) && (!chan->g) && (!chan->b) && (!chan->lum)) continue; // not visible
    chan->max_x_shift=0;
    if ((!strcmp(chan->chan_name,"HRV"))) // && (chan->selected))
    {
      chan->width_totalglobe=11136;
      if ((chan->segm) && (chan->segm->xh.xrit_frmt==STDFRMT))
      {
  // upper_x_shift and lower_x_shift: Means 'shift to east'.
  //   Org. file: upside-down, so shift = 'shift pic to higher x-value and increase width'.
  //   stdfrmt from MDM: Not upside-down, so shift = increase width, leave pic at left (i.e., shift east). 
  //    For stdfrmt (comon
  //           suf.    width    position       range of total (from east)
  //           A       5568	left=0            0 ... 5568
  //           E       5568	2112=0         2112 ... 7680      
  //           G       5568	center=0       2784 ... 8352
  //           W       7680	right=90          0 ... 7680
  //           F       11136	center=0          0 ... 11136
  //
  //           N=top 3248 lines only (7 segments)

        chan->upper_x_shift=0;
        if (strchr(chan->segm->xh.mdm_pos_code,'g'))
        {
          chan->upper_x_shift=2784;
        }
        if (strchr(chan->segm->xh.mdm_pos_code,'e'))
        {
          chan->upper_x_shift=2112;
        }
        if (strchr(chan->segm->xh.mdm_pos_code,'n'))
        {
          chan->height_totalglobe=11136;
        }
        chan->lower_x_shift=0;       // upper_x_shift !=0 makes pic wider, result: shift pic to east
        chan->shift_ypos=11136;      // giving space right of pic (north-to-south!)
        chan->max_x_shift=MAX(chan->upper_x_shift,chan->lower_x_shift);
      }
      else // not from std-format, so take into account lower/upper shifts
      {
        if ((grp->pro_epi) && (grp->pro_epi->pro))  /* contains shift-info HRV */
        {
          chan->lower_x_shift=grp->pro_epi->pro->lower_east_col-1;
          chan->upper_x_shift=grp->pro_epi->pro->upper_east_col-1;
          chan->shift_ypos=grp->pro_epi->pro->lower_nord_row-1;
        }
        else                       /* no shift-info HRV available; take def. */
        {
          chan->lower_x_shift=0;
          chan->upper_x_shift=HRV_XSHIFT;    // never 100% correct...
          chan->shift_ypos=HRV_SHIFT_YPOS;   // same
        }
        // x_shift determines extra width because of HRV-shift
        chan->max_x_shift=MAX(chan->upper_x_shift,chan->lower_x_shift);
      }
    }
    /* Determine original height of tallest channel */ 
    pc.heightmax=MAX(pc.heightmax,chan->nl*(chan->seq_end-chan->seq_start+1));

    /* Determine original width of tallest channel; keep shift in HRV pic */ 
    pc.widthmax=MAX(pc.widthmax,chan->nc+chan->max_x_shift);

    if (prefer.full_globe)
      pc.widthmax=MAX(pc.widthmax,chan->width_totalglobe);
  }

  /* Determine first and last chunck to use for pic (if 1 channel is used) */
  if ((grp->selected) || (chan1->selected))         /* full picture */
  {
    int first_chunck,last_chunck;
    first_chunck=chan1->seq_start;
    last_chunck=chan1->seq_end;
/*
NIETDOEN: benodigde segmenten hangt af van 'area' en wat in prefs staat!
Dus dan maar alles compleet hoewel misschien niet nodig.
*/
    #if DETECT_LIM_SEGM != 0
    if ((modes) && (modes->area_nr) && (!IS_POLAR(grp)))
      first_chunck=last_chunck-chan1->seq_end/4+1;
    #endif

    if (!chan1_maybe_not_used)
    {
      if (chan_complete(chan1,first_chunck,last_chunck))
      {
        pc.incomplete=FALSE;
      }
      else
      {
        pc.incomplete=TRUE;
      }
    }
    pc.first_chunck=first_chunck;
    pc.last_chunck=last_chunck;
  }
  else
  {
    for (segm=chan1->segm; segm; segm=segm->next)
    {
      if (!pc.first_chunck)
        pc.first_chunck=segm->xh.segment;
      else
        pc.first_chunck=MIN(pc.first_chunck,segm->xh.segment);

      if (!pc.last_chunck) 
        pc.last_chunck=segm->xh.segment;
      else
        pc.last_chunck=MAX(pc.last_chunck,segm->xh.segment);
    }
    if (pc.last_chunck-pc.first_chunck+1 != chan1->nr_segm)
    {
// Don't set 'incomplete'; limited # segms selected.
//      pc.incomplete=TRUE;
    }
  }

/* Next is only for first chan, so only really valid if just 1 chan is selected */
  pc.nrchuncks=(pc.last_chunck-pc.first_chunck)+1;  // # chnks selected

  if (chan1->seq_end-chan1->seq_start+1 != pc.nrchuncks) pc.partly=TRUE;
  
  pc.width=pc.widthmax; 
//  pc.height=chan1->nl*pc.nrchuncks;
  pc.height=pc.heightmax;
  pc.y_chunck=chan1->nl;
  pc.depth=chan1->nb;
  pc.image_iformat=chan1->ifrmt;
  pc.scan_dir=chan1->scan_dir;

/* Determine part of name for pic to generate: mapping */
  if (grp->compose)
  {
    char tmp[20];
    *tmp=0;
    *pc.chan=0;
    if ((globinfo.spm.compose_type!=map_vis_hmsg))
    {
      sprintf(pc.chan,"%s%s",pc.chan,globinfo.clrmapspec.name_sh);
    }
    else
    {
      for (chan=grp->chan; chan; chan=chan->next)
      {
        if (chan->r) sprintf(tmp,"%s%x",tmp,chan->chan_nr);
        if (strlen(tmp) > 3) break;      // limit in case of idiot mapping
      }
      if (*tmp) sprintf(pc.chan,"%sR%s",pc.chan,tmp);

      *tmp=0;
      for (chan=grp->chan; chan; chan=chan->next)
      {
        if (chan->g) sprintf(tmp,"%s%x",tmp,chan->chan_nr);
        if (strlen(tmp) > 3) break;      // limit in case of idiot mapping
      }
      if (*tmp) sprintf(pc.chan,"%sG%s",pc.chan,tmp);

      *tmp=0;
      for (chan=grp->chan; chan; chan=chan->next)
      {
        if (chan->b) sprintf(tmp,"%s%x",tmp,chan->chan_nr);
        if (strlen(tmp) > 3) break;      // limit in case of idiot mapping
      }
      if (*tmp) sprintf(pc.chan,"%sB%s",pc.chan,tmp);
    }
    *tmp=0;
    for (chan=grp->chan; chan; chan=chan->next)
    {
      if (chan->lum) sprintf(tmp,"%s%x",tmp,chan->chan_nr);
      if (strlen(tmp) > 3) break;      // limit in case of idiot mapping
    }
    if (*tmp) sprintf(pc.chan,"%sH%s",pc.chan,tmp);
  }
  else if (grp->gen_rah)
  {
    strcpy(pc.chan,"");
  }
  else if (chan1->chan_name)
  {
    strcpy(pc.chan,chan1->chan_name);
  }
  strcpy(pc.sat_src,grp->sat_src);

  grp->pc=pc;
  mk_picname(grp,chan1);
  if (prefer.use_fn_frmt)
    mk_filename(grp,chan1,prefer.fn_frmt);
  else
    mk_filename(grp,chan1,NULL);

  grp->pc.bitshift=globinfo.bitshift;
  grp->pc.orbittype=grp->orbittype;

  if (grp->pc.orbittype==POLAR)
  {
    grp->pc.lines_per_sec=chan1->lines_per_sec;
    load_kepler(grp);
    grp->pc.lines_per_sec=chan1->lines_per_sec;
    if (grp->pc.gmap==normal)
      grp->pc.scan_dir=polar_dir(grp);
    else
      grp->pc.scan_dir='n';
  }
  pc=grp->pc;

  /* Determine col offset/line offset for lon/lat calc. from x/y */
  if (IS_GEO(grp))
  {
    CHANNEL *hrvchan=NULL;
    pc.orbit.valid=TRUE;         // geo has always(?) position info
    if (Chan_Contributes(grp,"HRV"))  // contains HRV
    {
      hrvchan=Get_Chan(grp->chan,"HRV");
    }
    if (hrvchan)                           // contains HRV
    {
      if (hrvchan->segm)
      {
        int loff_start;
        // loff_start normally 0, except if first segment(s) are missing
        loff_start=(hrvchan->segm->xh.segment-hrvchan->seq_start)*grp->chan->segm->xh.nl;
        pc.gchar.coff         =hrvchan->segm->xh.coff;
        pc.gchar.cfac         =hrvchan->segm->xh.cfac;
        pc.gchar.loff         =hrvchan->segm->xh.loff + loff_start;
        pc.gchar.lfac         =hrvchan->segm->xh.lfac;
        pc.gchar.sub_lon      =hrvchan->segm->xh.sub_lon;

        // Correct coff. Mapping is always such that pic starts east with 90 degrees, so for hrv coff always 5566.
        if (hrvchan->segm->xh.nl*(hrvchan->segm->xh.segment-1) < hrvchan->shift_ypos)
          pc.gchar.coff+=hrvchan->lower_x_shift;
        else
          pc.gchar.coff+=hrvchan->upper_x_shift;
      }
    }
    else
    {
      CHANNEL *chan1;
      chan1=get_nonhrv(grp);
      if (!chan1) chan1=grp->chan;
      if ((chan1) && (chan1->segm))                   // expected always !=0
      {
        int loff_start;
        // loff_start normally 0, except if first segment(s) are missing
        loff_start=(chan1->segm->xh.segment-chan1->seq_start)*chan1->segm->xh.nl;
        pc.gchar.coff         =chan1->segm->xh.coff;
        pc.gchar.cfac         =chan1->segm->xh.cfac;
        pc.gchar.loff         =chan1->segm->xh.loff + loff_start;
        pc.gchar.lfac         =chan1->segm->xh.lfac;
        pc.gchar.sub_lon      =chan1->segm->xh.sub_lon;
      }
    }
    if ((grp->pro_epi) && (grp->pro_epi->pro)) 
      if (grp->pro_epi->pro->ProjectionDescription.LongitudeOfSSP)
        pc.gchar.sub_lon      =grp->pro_epi->pro->ProjectionDescription.LongitudeOfSSP;
  }  /* grp->orbittype==GEO */

  grp->pc=pc;

  /* Determine size of pic */
  if (modes)
  {
    if (globinfo.area_nr>0)
    {
      modes->geoarea=prefer.geoarea[globinfo.area_nr-1];  // Boundaries channels
    }

    if ((IS_GEO(grp)) && (pc.gchar.cfac) &&               // coff/loff etc. available
       (modes->geoarea.delta.lon) && (modes->geoarea.delta.lat))  //   and dlon/dlat speccified
    {                                                             //     => use lon/lat for boundaries
      int x1,x2,y1,y2,x,y;
      float lon1,lon2,lat1,lat2;
      GEOCHAR gchar=pc.gchar;
// Reset shift parameters, otherwise wrong fixed area HRV. They are re-filled in by size_pic();
      gchar.upper_x_shift=0; 
      gchar.lower_x_shift=0; 
      gchar.shift_ypos=0;
      lon1=modes->geoarea.center.lon-modes->geoarea.delta.lon;
      if (lon1 <  pc.gchar.sub_lon-90.) lon1=-90.;
      lon2=modes->geoarea.center.lon+modes->geoarea.delta.lon;
      if (lon2 > pc.gchar.sub_lon+90.) lon2=90.;

      glonlat2xy_xcorr(D2R(lon1),D2R(modes->geoarea.center.lat),&x1,&y,&gchar);
      glonlat2xy_xcorr(D2R(lon2),D2R(modes->geoarea.center.lat),&x2,&y,&gchar);
      /* Stretch to edge (glonlat2xy doesn't work near +/- 90 degrees) */
      if (lon1<=-89)
      {
        if (pc.gchar.cfac < 0) x1=pc.width-1; else x1=0;
      }
      if (lon2>=89)
      {
        if (pc.gchar.cfac < 0)  x2=0; else x2=pc.width-1;
      }

      lat1=modes->geoarea.center.lat+modes->geoarea.delta.lat;
      if (lat1 > 90.) lat1=90.;
      lat2=modes->geoarea.center.lat-modes->geoarea.delta.lat;
      if (lat2 < -90.) lat2=-90.;
      glonlat2xy_xcorr(D2R(modes->geoarea.center.lon),D2R(lat1),&x,&y1,&pc.gchar);
      glonlat2xy_xcorr(D2R(modes->geoarea.center.lon),D2R(lat2),&x,&y2,&pc.gchar);
      /* Stretch to edge (glonlat2xy doesn't work near +/- 90 degrees) */
      if (lat2<=-89)
      {
        if (pc.gchar.lfac < 0) y2=0; else y2=pc.height-1; 
      }
      if (lat1>=89)
      {
        if (pc.gchar.lfac < 0)   y1=pc.height-1; else y1=0;
      }

      if (x1<x2) SWAP(&x1,&x2);
      if (y1<y2) SWAP(&y1,&y2);
      modes->geoarea.pp_norm.fix_width=x1-x2;
      modes->geoarea.pp_norm.fix_woffset=x2;
      modes->geoarea.pp_norm.fix_height=y1-y2;
      modes->geoarea.pp_norm.fix_hoffset=y2;
      modes->geoarea.pp_hrv.fix_width=x1-x2;
      modes->geoarea.pp_hrv.fix_woffset=x2;
      modes->geoarea.pp_hrv.fix_height=y1-y2;
      modes->geoarea.pp_hrv.fix_hoffset=y2;
    }
    size_pic(grp,modes);
  }
  return grp->pc;
}

