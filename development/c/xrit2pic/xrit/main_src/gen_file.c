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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

extern XRIT_DBASE dbase;
extern GLOBINFO globinfo;
extern PREFER prefer;
extern GtkCTree *globtree;
extern GROUP *globgrp;

/***********************************************
 ***********************************************
 ** File generation functions
 ***********************************************
 ***********************************************/

/***********************************************
 * Generate admin file
 ***********************************************/
int gen_admin(GtkWidget *window,GROUP *grp_sel,PREFER *prefer)
{
  FILE *fpi,*fpo;
  char tfno[1000];
  char *fn=grp_sel->chan->segm->pfn;
  char *ext=".txt";
  PIC_CHARS *pc=&grp_sel->pc;
  if (!(fpi=open_xritimage(fn,NULL,EXRIT,NULL))) return Open_Rd;

  make_filename(prefer->dest_dir,pc,ext,tfno,NULL,prefer->split_destdir);

  if (overwrite(tfno,globinfo.overwrite_mode))
  {
    if ((fpo=fopen(tfno,"w")))
    {
      char l[1000];

      while (fgets(l,1000,fpi))
      {
        l[strlen(l)-2]='\n';
        l[strlen(l)-1]=0;
        fprintf(fpo,"%s",l);
      }
      fclose(fpo);
      Set_Entry(window,LAB_INFO,"Created: %s%s",pc->picname,ext);
    }
    else
    {
      return Open_Wr;
    }
  }
  else
  {
    Set_Entry(window,LAB_INFO,"Exist: %s%s",pc->picname,ext);
  }

  fclose(fpi);
  return 0;
}

static char *sattype(char *s)
{
  if (!strcmp(s,"MET5"))          return "MFG"; 
  if (!strcmp(s,"MET6"))          return "MFG"; 
  if (!strcmp(s,"MET7"))          return "MFG"; 
  if (!strncmp(s,"GOES",4))       return "GOES"; 
  if (!strncmp(s,"MTSAT",4))      return "GOES"; 
  if (!strncmp(s,"MSG",3))        return "MSG"; 
  if (!strcmp(s,"Srvc"))          return "Messages";
  if (!strncasecmp(s,"noaa",4))   return "NOAA"; 
  if (!strncasecmp(s,"METOP",5))  return "METOP";
  if (!strncmp(s,"DWDSAT",6))     return "DWDSAT";
  return "rest";
}

void make_destdir(char *dest_dir,PIC_CHARS *pc,char *tfno)
{
  *tfno=0;
  if (*dest_dir)
  {
    strcpy(tfno,dest_dir);
    if (RCHAR(tfno)!=DIR_SEPARATOR) sprintf(tfno,"%s%c",tfno,DIR_SEPARATOR);
    if (prefer.split_destdir) sprintf(tfno,"%s%s%c",tfno,sattype(pc->sat_src),DIR_SEPARATOR);
    if (!make_dir(tfno));
//      Create_Message("Info","Created new dir\n%s.",tfno);
  }
}

void make_filename(char *dest_dir,PIC_CHARS *pc,char *ext,char *tfno,GENMODES *modes,gboolean do_split)
{
  *tfno=0;
  if (*dest_dir)
  {
    strcpy(tfno,dest_dir);
    if (RCHAR(tfno)!=DIR_SEPARATOR) sprintf(tfno,"%s%c",tfno,DIR_SEPARATOR);
    if (prefer.split_destdir) sprintf(tfno,"%s%s%c",tfno,sattype(pc->sat_src),DIR_SEPARATOR);
    if (!make_dir(tfno));
//      Create_Message("Info","Created new dir\n%s.",tfno);
  }
  if ((modes) && (modes->image_oformat=='t')) return;

  strcat(tfno,pc->fno);

  if (ext)
  {
    strcat(tfno,ext);
  }
  else if (modes)
  {
    if (modes->image_oformat=='r')
    {
      strcat(tfno,".rah");
    }
    else if ((modes->image_oformat=='j') || (modes->image_oformat=='J'))
    {
      strcat(tfno,".jpg");
    }
    else if (modes->image_oformat=='P')
    {
      strcat(tfno,".pgm");
    }
    else if (modes->image_oformat=='p')
    {
      if (pc->is_color)
      {
        strcat(tfno,".ppm");
      }
      else
      {
        strcat(tfno,".pgm");
      }
    }
  }
}

#if __GTK_WIN32__ == 1
#define COPY_PROG "copy"
#else
#define COPY_PROG "cp"
#endif
int copy_file(char *a,char *b)
{
  char cmd[1000];
  sprintf(cmd,"%s %s %s",COPY_PROG,a,b);
  return system(cmd);
}

// Run a script each time a pic is completed.
// fn=file generated
// command: see preferences, tab 'Generation', field 'Command'
// E.g.: cp %s abc/%s
static int run_script(char *fn,GtkWidget *widget)
{
  char *cmd;
  int err=0;
  int i,n=0,len;

  // Count # strings
  for (i=0; i<strlen(prefer.run_cmd); i++)
    if (!strncmp(prefer.run_cmd+i,"%s",2)) n++;

  len=strlen(prefer.run_cmd)+n*strlen(fn)+10;
  cmd=malloc(len);
  if (!cmd) return 1;
  switch(n)
  {
    case 0: snprintf(cmd,len,prefer.run_cmd); break;
    case 1: snprintf(cmd,len,prefer.run_cmd,fn); break;
    case 2: snprintf(cmd,len,prefer.run_cmd,fn,fn); break;
    case 3: snprintf(cmd,len,prefer.run_cmd,fn,fn,fn); break;
    case 4: snprintf(cmd,len,prefer.run_cmd,fn,fn,fn,fn); break;
    default: strcpy(cmd,prefer.run_cmd); break;
  }
  #if __GTK_WIN32__ == 1
    err=execcmd(cmd,NULL,NULL,TRUE);
  #else
    err=system(cmd);
  #endif
  if (widget)
  {
    if (err) Set_Entry(widget,LAB_INFO,"Failed: %s.",cmd);
    else     Set_Entry(widget,LAB_INFO,"Cmd: %s.",cmd);
  }
  free(cmd);
  return err;
}


/***********************************************
 * Generate a file in any format (JPEG, PGM, PPM)
 * from picture in any format (JPEG, Wavelet)
 * filename is generated, and available in *tfno
 ***********************************************/
int gen_picfile(GtkWidget *window,
                GROUP *grp_sel,          /* contains 1 picture to generate */
                PREFER *prefer,
                char *tfno,              /* output generated file, incl. path */
                gboolean keep_chnk,      /* keep chunk in mem? */
                gboolean show_progress,
                GENMODES *modes)
{
  int err=0;
  FILE *fpi;
  char *fn;
  char *fn1;
  PIC_CHARS pc1=grp_sel->pc;
  PIC_CHARS *pc=&pc1;
  DATATYPE dtype=grp_sel->chan->segm->xh.xrit_frmt;
  if (!grp_sel) return -1;
  if (!grp_sel->chan) return -1;
  if (!grp_sel->chan->segm) return -1;
  fn=grp_sel->chan->segm->pfn;
/*
    if (pc->pixel_shape)
    {
      if (pc->pixel_shape>1.) pc->o_height*=pc->pixel_shape;
      if (pc->pixel_shape<1.) pc->o_width/=pc->pixel_shape;
    }
*/
// adapt_lum betekent ook dat zoom-factor wordt meegenomen!
  if (modes->adapt_lum) zoom_size(modes,pc);
  if (modes->gen_film) adapt_filmsize(modes,pc);
  if (fn)
  {
    if (!(fpi=open_xritimage(fn,NULL,dtype,grp_sel->chan))) return Open_Rd;
    fclose(fpi);
  }
/* Determine output format */
  switch(modes->cformat)
  {
    case 'r':
      modes->image_oformat='r';           /* generate rah (AVHRR) */
    break;
    case 'j':
      modes->image_oformat='j';           /* generate jpeg */
    break;
    case 'J':
      modes->image_oformat='J';           /* generate jpeg without decompressing */
    break;
    case 'p':
      modes->image_oformat='p';           /* generate pgm */
    break;
    case 'P':
      modes->image_oformat='P';           /* generate pgm  8 bpp*/
    break;
    default:
      switch (pc->image_iformat)
      {
        case 'j':
          modes->image_oformat='j';       /* generate jpeg without decompressing */
        break;
#ifdef XXX
        case 'Z':
          modes->image_oformat='p';       /* generate ppm */
        break;
#endif
        case 'z': case 'Z':
          if ((grp_sel->gen_rah) && (!grp_sel->compose))
            modes->image_oformat='r';       /* generate rah */
          else
            modes->image_oformat='p';       /* generate ppm */
        break;
        case 't':
          modes->image_oformat='t';       /* tiff */
        break;
        default:
          modes->image_oformat='p';         /* generate pgm */
        break;
      }
    break;
  }
  if ((pc->is_color) || (globinfo.dim3))
  {
    if (modes->image_oformat=='J')
      modes->image_oformat='j';
  }
  
/* For avi film, files will be generated in temp dir. */
  if (modes->gen_film)
  {
    char fd1[1000],fd2[1000],*ptfno;
    if (!(make_dir_hier(globinfo.dest_tmpdir)))
    {
      if (window)
      {
        Set_Entry(window,LAB_INFO,"Created dir. %s.",globinfo.dest_tmpdir);
      }
    }

    make_filename(globinfo.dest_tmpdir,pc,NULL,tfno,modes,FALSE); /* generate in temp-dir */

/* Check if already available; move to temp-dir if so.
   Later check if already in temp-dir, then generation is skipped.
*/
    if ((ptfno=strrchr(tfno,DIR_SEPARATOR)))
    {
      int w1=0,h1=0;
      sprintf(fd1,"%s%s",globinfo.dest_tmpsavdir,ptfno+1);  // filename in sav dir
      
      /* Check if file has correct size. If so, move it to collect dir */
      get_jpegsize(fd1,&w1,&h1);                            // catch size
      if ((w1==pc->o_width) && (h1==pc->o_height))          // size equals what's expected?
      {
        sprintf(fd2,"%s%s",globinfo.dest_tmpdir,ptfno+1);   // make filename in tmp-dir
        rename(fd1,fd2);                                    // move from sav dir to tmp dir
        grp_sel->done=TRUE;
      }
    }
  }
  else
  {
    make_filename(globinfo.dest_dir,pc,NULL,tfno,modes,prefer->split_destdir);    /* generate in dest dir */
    if (modes->image_oformat=='t') strcat(tfno,grp_sel->chan->segm->fn);
  }

/* strip path from filename; only for messages */
  fn1=strrchr(tfno,DIR_SEPARATOR);
  if (fn1) fn1++; else fn1=tfno;

  if (modes->test) return 0;
  switch(modes->image_oformat)
  {
    case 'r':
      if (overwrite(tfno,modes->overwrite_mode))
      {
        if (raw16torah(grp_sel,tfno,show_progress))
        {
          err=Aborted;
          if (window) Set_Entry(window,LAB_INFO,"Exist: %s.",fn1);
        }
      }
      else
      {
        err=Exist_fn;
        if (window) Set_Entry(window,LAB_INFO,"Exist: %s.",fn1);
      }

    break;
    case 'p': case 'P': case 'j':  /* jpeg, pgm8/16, for pic and movie */
      if ((( modes->gen_film) && (overwrite(tfno,modes->filmframe_ovrwrmode))) ||
          ((!modes->gen_film) && (overwrite(tfno,modes->overwrite_mode))))
      { /* film -> don't check if file exist */
        err=gen_a_picfile(grp_sel,pc,tfno,keep_chnk,show_progress,modes);
        grp_sel->done=TRUE;
        if (window)
        {
          switch (err)
          {
            case Aborted : Set_Entry(window,LAB_INFO,"Aborted: %s.",fn1); break;
            case Mis_segm: Set_Entry(window,LAB_INFO,"Segments in %s missing.",fn1); break;
            case 0:        Set_Entry(window,LAB_INFO,"Created: %s.",fn1); break;
            default:       Set_Entry(window,LAB_INFO,"Segments in %s missing.",fn1); break;
          }
        }
      }
      else if ((modes->gen_film) && (modes->filmframe_ovrwrmode==2))
      {
        grp_sel->done=TRUE;
      }
      else
      {
        err=Exist_fn;
        if (window) Set_Entry(window,LAB_INFO,"Exist: %s.",fn1);
      }
    break;
    case 'J':                       /* jpeg, without decompressing */
      if (pc->is_color)
      {
        return JPG_Conc;
      }
   
      if (overwrite(tfno,globinfo.overwrite_mode))
      {
        err=gen_jpgfile(grp_sel,pc,tfno,show_progress);
        if (window) Set_Entry(window,LAB_INFO,"Created: %s.",fn1);
      }
      else
      {
        err=Exist_fn;
        if (window) Set_Entry(window,LAB_INFO,"Exist: %s.",fn1);
      }
    break;
    case 't':
      if (copy_file(grp_sel->chan->segm->pfn,tfno))
      {
        if (window)
          Set_Entry(window,LAB_INFO,"Failed Copy of %s.",grp_sel->chan->segm->fn);
      }
      else
      {
        if (window)
          Set_Entry(window,LAB_INFO,"Copied: %s.",grp_sel->chan->segm->fn);
      }
    break;
  }
  if ((pc->incomplete) && (err==0)) err=Mis_segm;
  return err;
}

void handle_spaces(char *s,char dirsep) /* s='aap noot/wsat -i' */
{
  char *p;
  p=strrchr(s,dirsep);                   /* p='/wsat -i' */
  if (!p) return;
  p=strchr(p,' ');                       /* p=' -i' */
  if (p) *p=0;                           /* p='. -i" ; s="aap noot/wsat' */
  if ((strchr(s,' ')) && (*s!='"'))
  {
    if (p) memmove(p+2,p,strlen(p+1)+2); /* p='...-i' */
    memmove(s+1,s,strlen(s)+1);          /* s=' aap noot/wsat' ; p='t..-i' */
    *s='"';                              /* s='"aap noot/wsat' */
    if (p)
    {
      *(s+strlen(s))='"';                /* s='"aap noot/wsat"' ; p='t".-i' */
      p+=2; *p=' ';                      /* s='"aap noot/wsat" -i" */
    }
    else
    {
      strcat(s,"\"");
    }
  }
  else if (p) *p=' ';
}

#include <stdlib.h>
char *launch_viewer(PREFER *prefer,PIC_CHARS *pc,char *tfno,GENMODES *gmode)
{
#if __GTK_WIN32__ == 1
  const char nwproc=0;
#else
  const char nwproc='&';
#endif
  static char cmd[1000];


  handle_spaces(prefer->prog_jpg,DIR_SEPARATOR);     /* determine launcher JPEG */
  handle_spaces(prefer->prog_pgm,DIR_SEPARATOR);     /* determine launcher PRM */

  switch(gmode->image_oformat)
  {
    case 't':                                        /* launch TIFF viewer */
      sprintf(cmd,"%s %s %s %c",
                  prefer->prog_jpg,tfno,(pc->scan_dir=='s'? prefer->ud_jpg : ""),nwproc);
    case 'J':                                        /* launch JPEG viewer */
      sprintf(cmd,"%s %s %s %c",
                  prefer->prog_jpg,tfno,(pc->scan_dir=='s'? prefer->ud_jpg : ""),nwproc);
    break;
    case 'j':                                        /* launch JPEG viewer */
      sprintf(cmd,"%s %s %s %c",
                  prefer->prog_jpg,tfno,(pc->scan_dir=='s' && !prefer->scandir_correct? prefer->ud_jpg : ""),nwproc);

    break;
    case 'p': case 'P':                              /* launch PGM viewer */
      sprintf(cmd,"%s %s %s %c",
                prefer->prog_pgm,tfno,
                 (pc->scan_dir=='s' && !prefer->scandir_correct? prefer->ud_pgm : ""),nwproc);

    break;
    default:
      *cmd=0;
    break;
  }
  if (system(cmd)) return cmd;                                      /* Launch viewer */
  return NULL; 
}

/* Extract as raw data */
int gen_bin(GtkWidget *window,GROUP *grp_sel,PREFER *prefer,char *tfno,gboolean
test)
{
  FILE *fpi,*fpo;
  SEGMENT *segm;
  char *fn;
  char *ext=".bin";
  PIC_CHARS *pc=&grp_sel->pc;
  if ((!grp_sel->chan) || (!grp_sel->chan->segm)) return 1;
  fn=grp_sel->chan->segm->pfn;
  if (!(fpi=open_xritimage(fn,NULL,UNKDTYPE,NULL))) return Open_Rd;
  fclose(fpi);

  make_filename(prefer->dest_dir,pc,ext,tfno,NULL,prefer->split_destdir);
  if (test) return 0;

  if (overwrite(tfno,globinfo.overwrite_mode))
  {
    if (!(fpo=fopen(tfno,"wb"))) return Open_Wr;
    
    for (segm=grp_sel->chan->segm; segm; segm=segm->next)
    {
      int c;
      fn=segm->pfn;
      if (!(fpi=open_xritimage(fn,NULL,UNKDTYPE,NULL)))
      {
        Create_Message("ERROR","Missing file: %s.",fn);
        continue;
      }
      while ((c=fgetc(fpi))!=EOF) fputc(c,fpo);
      fclose(fpi);
    }
    fclose(fpo);
    Set_Entry(window,LAB_INFO,"Created: %s%s.",pc->picname,ext); 
  }
  else
  {
    Set_Entry(window,LAB_INFO,"Exist: %s%s.",pc->picname,ext);
  }

  return 0;
}

int gen_bufr(GtkWidget *window,GROUP *grp_sel,PREFER *prefer,char *tfno,gboolean test)
{
  FILE *fpi;
  char *fn,*p;
  char *errmess;
  int nrp=0;
  PIC_CHARS *pc=&grp_sel->pc;
  if ((!grp_sel->chan) || (!grp_sel->chan->segm)) return 1;
  fn=grp_sel->chan->segm->pfn;
  if (!(fpi=open_xritimage(fn,NULL,BUFR,NULL))) return Open_Rd;
  fclose(fpi);

#ifdef EXTRACT_SUBBUFR
  strcpy(pc->fno,grp_sel->chan->segm->fn);
  make_filename(prefer->dest_dir,pc,NULL,tfno,NULL,prefer->split_destdir);
  if (test) return 0;
  if ((fpi=fopen(fn,"rb")))
  {
    char *ext=".bufr";
    int n;
    n=extract_bufr(fpi,tfno,ext);
    fclose(fpi);
    Set_Entry(window,LAB_INFO,"Extracted: %d bufr parts from %s.",n,grp_sel->chan->chan_name); 
  }
#else
  strcpy(pc->fno,grp_sel->chan->segm->fn);
  if ((p=strrchr(pc->fno,'.'))) *p=0;
  strcat(pc->fno,".txt");
  make_filename(prefer->dest_dir,pc,NULL,tfno,NULL,prefer->split_destdir);
  errmess=scat_extract_to(fn,tfno,&nrp);

  if (!errmess)
  {
    Set_Entry(window,LAB_INFO,"Translation failed!"); 
  }
  else if (*errmess)
  {
    Set_Entry(window,LAB_INFO,"%s",errmess); 
  }
  else
  {
    char *p;
    if ((p=strrchr(tfno,DIR_SEPARATOR))) p++; else p=tfno;
    Set_Entry(window,LAB_INFO,"Extracted %d points to %s.",nrp,p); 
  }

#endif  
  return 0;
}

int gen_avi(char *mfno,PREFER *prefer,FILELIST *fl)
{
  int res=0;
  if (prefer->extern_moviegen)
  {
    char cmd[1000];
    sprintf(cmd,prefer->prog_genmovie,globinfo.dest_tmpdir,"*.jpg","",mfno);
//    strcat(cmd," ");
//    strcat(cmd,prefer->opt_speed);
    if (system(cmd)) res=0;
  }
  else
  {
    res=jpeg2avi(fl,mfno,prefer->speed);
  }
  return res;
}

int gen_cpgm(char *mfno,PREFER *prefer)
{
  DIR *directory;
  struct dirent *dirEntry;
  FILE *fpi,*fpo;
  char fn[1000];
  char dir[1000];
  int c;
  if (!(fpo=fopen(mfno,"wb"))) return -2;

/* opendir(): For Windows path MUST end with '.' */
  sprintf(dir,"%s.",globinfo.dest_tmpdir);
  if (!(directory = opendir(dir)))
  {
    fclose(fpo);
    return -2;
  }
  if (directory) while ((dirEntry = readdir(directory)))
  {
    sprintf(fn,"%s%s",globinfo.dest_tmpdir,dirEntry->d_name);
    if (*dirEntry->d_name != '.')
    {
      if (!(fpi=fopen(fn,"rb"))) continue;
      while ((c=fgetc(fpi)) != EOF) fputc(c,fpo);
      fclose(fpi);
    }      
  }
  closedir(directory);
  fclose(fpo);

  return 0;
}

void Delete_Use_List(GROUP *igrp,GtkCTree *ctree,GtkWidget *widget)
{
  GROUP *grp,*grpnext;
  CHANNEL *chan,*channext;
  for (grp=igrp; grp; grp=grpnext)
  {
    char sat[10];
    grpnext=grp->next;

/* Distinquish between MSG1 and 2. 
   If MSG-2 operational: Don't dist. between 1 and 2!
*/
    if (!strncmp(grp->sat_src,"MSG1",4))
      sprintf(sat,"%c%s",grp->h_or_l,"MSG1");
    else if (!strncmp(grp->sat_src,"MSG2",4))
      sprintf(sat,"%c%s",grp->h_or_l,"MSG2");
    else
      strcpy(sat,grp->sat_src);
    
    for (chan=grp->chan; chan; chan=channext)
    {
      channext=chan->next;
      if (!in_satchanlist(sat,chan->chan_name,0,globinfo.keep_chanlist))
      {
        Set_Entry(widget,LAB_INFO,"Removed after: %s",chan->chan_name);
        Delete_Chan(chan,ctree);
      }
    }
    if (!grp->chan)
    {
      if (globgrp==grp)
        globgrp=grp->next;
      Delete_Grp(grp,ctree);
    }
  }
}

/* Auto translate pic which is also selected as 'live' */
void auto_translate(GENMODES *gmode,GROUP *grp,PREFER *pref,GtkWidget *widget)
{
  for (; grp; grp=grp->next)    // go through all groups
  {
    CHANNEL *chan;
    if (!grp->new) continue;    // already processed
    
    if (gmode->spm.compose)          
    {
      grp->selected=TRUE;      // select whole group
      chan=grp->chan;          // take first chan, maybe this isn't used!
    }
    else   // one channel used
    {
      for (chan=grp->chan; chan; chan=chan->next)
        chan->r=chan->g=chan->b=0;      // 'reset' all chans

      for (chan=grp->chan; chan; chan=chan->next)
      {
        if (contributes_to_pic(chan,globinfo.upd_sat,globinfo.upd_chan,gmode->essential_only))
        {
          chan->selected=TRUE;        // this single channel selected
          chan->r=chan->g=chan->b=1;
          break;
        }
      }
    }

// If either chan=selected or composed then 'contributes_to_pic'.
// No wild-char!
    if (contributes_to_pic(chan,globinfo.upd_sat,globinfo.upd_chan,gmode->essential_only))
    {
    
      if (picture_complete(chan,NULL,gmode))
      {
        int err=0;
        int nrfr=0;
        char tfno[1000];
        char mfno[1000];
        char *p;
        *tfno=0;
        *mfno=0;
        if (widget)
        {
          Set_Entry(widget,LAB_INFO,"Translating...");
          Set_Entry(widget,LAB_RECINFO,"Translating...");
        }
        grp->compose=gmode->spm.compose;

        pic_info(grp,widget,gmode);
        gmode->adapt_lum=FALSE;
        gmode->spm.invert=globinfo.spm.invert;
        gmode->spm.inv_ir=globinfo.spm.inv_ir;
        gmode->zx=1.;
        gmode->zy=1.;
        if (!(err=gen_picfile(NULL,grp,pref,tfno,FALSE,FALSE,gmode)))
        {
          if (gmode->gen_film)
          {
            sprintf(mfno,"%s%c%s",prefer.dest_dir,DIR_SEPARATOR,grp->pc.picname);
            if ((p=strrchr(mfno,'_'))) *p=0;
            if (gmode->area_nr) strcat(mfno,"_e");
            strcat(mfno,".avi");
            if (pref->extern_moviegen)
              gen_avi(mfno,pref,NULL);
            else
            {
              nrfr=add_frame(mfno,tfno,pref->speed);
            }
          }
          if (widget)
          {
            if (gmode->gen_film)
            {
              if (!(p=strrchr(mfno,DIR_SEPARATOR))) p=mfno; else p++;
              Set_Entry(widget,LAB_INFO,"Created: %s %d frames.",p,nrfr);
              Set_Entry(widget,LAB_RECINFO,"Created %s %d frames.",p,nrfr);
            }
            else
            {
              if (!(p=strrchr(tfno,DIR_SEPARATOR))) p=tfno; else p++;
              Set_Entry(widget,LAB_INFO,"Created: %s.",p);
              Set_Entry(widget,LAB_RECINFO,"Created %s.",p);
            }
          }
          if (gmode->run_script)
          {
            run_script(tfno,widget);
          }
        }
        else
        {
          if (!(p=strrchr(tfno,DIR_SEPARATOR))) p=tfno; else p++;
          Set_Entry(widget,LAB_INFO,"(Skipped: %s; %s (%d)",p,report_genpicerr_1line(err),err);
          Set_Entry(widget,LAB_RECINFO,"(Skipped: %s; %s (%d)",p,report_genpicerr_1line(err),err);
        }
        free_chnks(grp);
        grp->new=FALSE;
        grp->done=TRUE;  // ook al wat gedaan in gen_picfile
       
        if (globinfo.rm_raw)
        {
          if (dbase.grp_last)
          {
            Delete_Use_List(dbase.grp_last->next,dbase.main_tree,widget);
          }
          else
          {
            Delete_Use_List(globgrp,dbase.main_tree,widget);
          }
//          Set_Entry(widget,LAB_RECINFO,"Deleted raw data: %d.",n);

          break;      // (from for (; grp...) Done; grp is destroyed, so MUST stop!!!
        }
        break;        // complete pic found, so stop.
      }
    }
  }
}

