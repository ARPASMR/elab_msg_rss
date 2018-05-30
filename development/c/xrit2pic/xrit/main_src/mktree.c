/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
#include "xrit2pic.h"
#include "gtk/gtk.h"
#include <math.h>
#define LIST_NRCOL 10

void Add_Proepi_totree(GtkCTree *ctree,GROUP *grp1,CHANNEL *chan1,SEGMENT *segm)
{
  char *tmpc[LIST_NRCOL];
  int i;
  GdkColor clr_pe={0,0xffff,0x8888,0};  /* color channel item complete */

  if (chan1->hide) return;

  for (i=0; i<LIST_NRCOL; i++) tmpc[i]="";
  if (segm->is_pro)
    tmpc[0]="PRO";           /*   */
  else if (segm->is_epi)
    tmpc[0]="EPI";           /*   */
  else
    tmpc[0]="???";           /*   */

  if (!chan1)
    segm->node=Add_CLeaf(ctree,grp1->node,tmpc);   /* next image segment */
  else
    segm->node=Add_CLeaf(ctree,chan1->node,tmpc);   /* next image segment */

  gtk_ctree_node_set_foreground(ctree,segm->node,&clr_pe);
}

void Add_Segm_totree(GtkCTree *ctree,GROUP *grp1,CHANNEL *chan1,SEGMENT *segm)
{
  GdkColor clr_chpe={0,0xffff,0x0000,0xffff};       /* color pro/epi */
  char *tmpc[LIST_NRCOL];
  char strsegmnr[50];
  char strsegmsize[50];
  char strlatrange[50];
  char strpicnr[50];
  char strtime[10];
  int i;
  if (!ctree) return;
  *strsegmnr=0;
  *strsegmsize=0;
  *strlatrange=0;
  *strpicnr=0;

  if (!chan1) return;                               /* should never occur */
  for (i=0; i<LIST_NRCOL; i++) tmpc[i]="";

  if (segm->xh.segment)
    sprintf(strsegmnr,"%d  ",segm->xh.segment);           /* segment nr. */
  sprintf(strpicnr,"%d",chan1->pic_nr);                   /* picture nr. */

  if (grp1->h_or_l=='B')
  {
    char *p;
    if ((p=strchr(segm->fn,'_')) && (p=strchr(p+1,'_')))
    {
      strncpy(strtime,p+1,2);
      strtime[2]=':'; strtime[3]=0;
      strncat(strtime,p+1+2,2);
      strtime[5]=0;
      tmpc[3]=strtime;
    }
  }
  if (grp1->h_or_l=='g')
  {
    char *p;
    if ((p=strstr(segm->fn,"afsv")))
      tmpc[2]=p;
    if ((p=strchr(segm->fn,'-')) && (p=strchr(p+1,'-')))
    {
      strncpy(strtime,p+1+6,2);
      strtime[2]=':'; strtime[3]=0;
      strncat(strtime,p+1+6+2,2);
      strtime[5]=0;
      tmpc[3]=strtime;
    }
  }
  else if (segm->is_pro)
  {
    tmpc[2]="PRO";
  }
  else if (segm->is_epi)
  {
    tmpc[2]="EPI";
  }
  else
  {
    if ((chan1->ifrmt) && (chan1->ifrmt) && (strchr("pjwzZ?",chan1->ifrmt)))
    {
      sprintf(strsegmsize,"%d x %d ",segm->chan->nc,segm->chan->nl);
    }
    else
    {
      *strsegmsize=0;
      if (!chan1->segm->next) return;
    }

    {
      const int table8[]={90,44,27,15,0,-15,-27,-44,-90};
      const int table24[]={90,62,52,44,37,32,27,23,19,14,9,4,0,
                    -4,-9,-14,-19,-23,-27,-32,-37,-44,-52,-62,-90};
      if ((chan1->seq_end) && (chan1->ifrmt) && (strchr("pjw",chan1->ifrmt)))
      {
        float lat1,lat2;
        lat1=R2D(asin(1.-((segm->xh.segment-1)*2./(float)chan1->seq_end)));
        lat2=R2D(asin(1.-((segm->xh.segment)*2./(float)chan1->seq_end)));
        if (chan1->seq_end==8)
        {
          lat1=table8[segm->xh.segment-1];
          lat2=table8[segm->xh.segment];
        }
        if (chan1->seq_end==24)
        {
          lat1=table24[segm->xh.segment-1];
          lat2=table24[segm->xh.segment];
        }
/*
Check bij lijf!
      if (grp1->scan_dir == 'n')
*/
        if (chan1->scan_dir == 's')
        {
          lat1*=-1;
          lat2*=-1;
        }
        sprintf(strlatrange,"  [%3d .. %3d]  ",(int)lat1,(int)(lat2));
      }
      else
        *strlatrange=0;
    }
    tmpc[2]=strlatrange;       /* lattitude range */
    tmpc[5]=strsegmsize;       /* segment size */
    tmpc[6]=strsegmnr;         /* segment part nr. */
    tmpc[7]=strsegmnr;         /* segment part nr. */
  }

/* Text for segment items */
  tmpc[0]=strpicnr;          /* picture nr. */
  tmpc[1]="";                /* (segment nr) */
  tmpc[4]="";                /* (date) */

  segm->node=Add_CLeaf(ctree,chan1->node,tmpc);   /* next image segment */
  if ((segm->is_pro) || (segm->is_epi))
    gtk_ctree_node_set_foreground(ctree,segm->node,&clr_chpe);


  return;
}

/* Add channel and underlying segments to tree */
gboolean Add_Chan_totree(GtkCTree *ctree,GROUP *grp1,CHANNEL *chan1)
{
  SEGMENT *segm1;
  GdkColor clr_chan;
  GdkColor clr_chok={0,0x0000,0x9999,0x0000};  /* color channel item complete */
  GdkColor clr_chpr={0,0xffff,0x9999,0x0000};  /* color HRV no prologue */
  GdkColor clr_chft={0,0xffff,0x0000,0x0000};  /* color channel item not complete */
  GdkColor clr_chpe={0,0xffff,0x0000,0xffff};  /* color pro/epi */
  int n,i;
  gboolean complete=FALSE;

/* Next are help-vars to define text in tree */

  char strtime[50],strdate[50];
  char *tmpt[LIST_NRCOL];
  char strptype[50];
  char strrange[50];
  char strpicnr[50];
  if (!ctree) return FALSE;

  if (chan1->hide) return FALSE;

  strftime(strtime,20,"%H:%M  ",&grp1->grp_tm);             /* time */
  strftime(strdate,20,"%d-%m-%y  ",&grp1->grp_tm);          /* date */

/* Define colours of different items */
  if (chan1 == grp1->pro_epi)
  {
    clr_chan=clr_chpe;
  }
  else if (!strcmp(grp1->sat_src,"DWDSAT"))
  {
    clr_chan=clr_chok;
    complete=TRUE;
  }
  else if (grp1->h_or_l=='B')
  {
    clr_chan=clr_chok;
    complete=TRUE;
  }
  else if ((n=channel_complete(chan1)))
  {
    if (n<0)
    {
      clr_chan=clr_chpr;
      complete=FALSE;
    }
    else
    {
      clr_chan=clr_chok;
      complete=TRUE;
    }
  }    
/*
  else if (chan1->nr_segm==chan1->seq_end-chan1->seq_start+1)
  {
    if ((chan1->chan_name) && (!strcmp(chan1->chan_name,"HRV")) && (!grp1->pro_epi))
      clr_chan=clr_chpr;
    else
      clr_chan=clr_chok;
  }
*/
  else
  {
    clr_chan=clr_chft;
  }

//  sprintf(strpicnr,"%d",chan1->pic_nr);                   /* picture nr. */
  if (chan1->chan_nr>0)
    sprintf(strpicnr,"%d",chan1->chan_nr);                   /* picture nr. */
  else
    *strpicnr=0;
  *strptype=0;
  *strrange=0;
  if (chan1->nb)
  {
    sprintf(strptype,"%s  %d",(chan1->ifrmt=='j'? "Jpeg" :
                               chan1->ifrmt=='w'? "Wavelet" :
                               chan1->ifrmt=='y'? "Raw1" :
                               chan1->ifrmt=='z'? "Raw2" :
                               chan1->ifrmt=='Z'? "Raw3" :
                               chan1->ifrmt=='t'? "tiff" :
                               chan1->ifrmt=='p'? "pgm" :
                                                  "plain"),chan1->nb);

   if (chan1->seq_end)
      sprintf(strrange,"%d of %d ",chan1->nr_segm,chan1->seq_end-chan1->seq_start+1);
  }

  for (i=0; i<LIST_NRCOL; i++) tmpt[i]="";

/* Text for channel items */
  tmpt[0]=strpicnr;           /* picture nr. */
  tmpt[1]=chan1->data_src;    /* source */
  
  if (chan1 == grp1->pro_epi)
    tmpt[2]="PRO_EPI";        /* channel */
  else
    tmpt[2]=chan1->chan_name; /* channel */
  tmpt[3]=strtime;            /* time */
  tmpt[4]=strdate;            /* date */
  tmpt[5]=strptype;           /* format */
  tmpt[6]=strrange;           /* tot. # segments */
  tmpt[7]=strrange;           /* tot. # segments */   // weview
  chan1->node=Add_CLeaf(ctree,grp1->node,tmpt);   /* next channel */
  gtk_ctree_node_set_foreground(ctree,chan1->node,&clr_chan);

  for (segm1=chan1->pro; segm1; segm1=segm1->next)
  {
    Add_Segm_totree(ctree,grp1,chan1,segm1);
  }
  for (segm1=chan1->epi; segm1; segm1=segm1->next)
  {
    Add_Segm_totree(ctree,grp1,chan1,segm1);
  }
  for (segm1=chan1->segm; segm1; segm1=segm1->next)
  {
    Add_Segm_totree(ctree,grp1,chan1,segm1);
  }
  return complete;
}


static void create_xrittree(GROUP *grp,GtkCTree *ctree,gboolean show_complete)
{
  GROUP *grp1;
  CHANNEL *chan1;

/* Next are help-vars to define text in tree */
  char strtype[50];
  char strtime[50],strdate[50];
  char strchsum[50];
  char start[50];
  char path[50];
  char end[50];
  char agestr[50];
  int age,i;
  
  char *tmpm[LIST_NRCOL];
  gboolean complete1=FALSE;
  gboolean completea=TRUE;
  int grxxxx=0xcccc;
  
/* Colors tree items */
  GdkColor clr_scan  ={0,0x0000,0x0000,0xffff};  /* color scan item (certain time/type) */
  GdkColor clr_compl0={0,0xffff,grxxxx,grxxxx};  /* color scan item no chan complete */
  GdkColor clr_compl1={0,0xffff,0xffff,grxxxx};  /* color scan item >=1 chan complete */
  GdkColor clr_compla={0,grxxxx,0xffff,grxxxx};  /* color scan item all complete */
  gtk_clist_freeze((GtkCList *)ctree); /* Freeze list for fast creation */

/* Create tree with all XRIT files */
  for (grp1=grp; grp1; grp1=grp1->next)
  {
    complete1=FALSE;
    completea=TRUE;
    if (grp1->hide) continue;

    switch(grp1->h_or_l)
    {
      case 'H': strcpy(strtype,"HRIT");                                    break;
      case 'L': strcpy(strtype,"LRIT");                                    break;
      case 'l': strcpy(strtype,"lrit");                                    break;
      case 'A': strcpy(strtype,"AVHRR");                                   break;
      case 'M': strcpy(strtype,"AVHRR");                                   break;
      case 'G': strcpy(strtype,"GAC");                                     break;
      case 's': strcpy(strtype,"SAT");                                     break;
      case 'f': strcpy(strtype,"FX");                                      break;
      case 'p': strcpy(strtype,"PS");                                      break;
      case 'g': strcpy(strtype,"GTS");                                     break;
      case 'R': strcpy(strtype,"GRB");                                     break;
      case 'c': strcpy(strtype,"CGM");                                     break;
      case 'r': strcpy(strtype,"RAD");                                     break;
      case 'w': strcpy(strtype,"WST");                                     break;
      case 'b': strcpy(strtype,"BUF");                                     break;
      case 'n': strcpy(strtype,"NXX");                                     break;
      case 'B': strcpy(strtype,"BUFR");                                    break;
      case 'h': strcpy(strtype,"hrit");                                    break;
      default : sprintf(strtype,"Unk: (%d) %c",grp1->h_or_l,grp1->h_or_l); break;
    }

    strftime(strtime,20,"%H:%M  ",&grp1->grp_tm);             /* time */
    strftime(strdate,20,"%d-%m-%y  ",&grp1->grp_tm);          /* date */
    sprintf(strchsum,"%s  ",grp1->chanstr);
    age=noaa_coverage(grp1,start,path,end,0,0);
    if ((age>=0) && (age<9999)) sprintf(agestr,"%5d",age); else *agestr=0;
// is een beetje traag. Alleen doen bij aanklikken regel.
//    {
//      int nsp;
//      if ((nsp=scatt_coverage(grp1,start))>=0) sprintf(path,"%d points",nsp);
//    }

/* Text for top-tree items */
    for (i=0; i<LIST_NRCOL; i++) tmpm[i]="";
    tmpm[0]="";
    tmpm[1]=grp1->sat_src;   /*  */
    tmpm[2]=strtype;         /* type */
    tmpm[3]=strtime;         /* time */
    tmpm[4]=strdate;         /* date */
    tmpm[5]=strchsum;        /* content */
    tmpm[6]=start;           /* (part) */
    tmpm[7]=path;
    tmpm[8]=end;
    tmpm[9]=agestr;
    
    grp1->node=Add_CLeaf(ctree,NULL,tmpm);                /* new channel set */
    gtk_ctree_node_set_foreground(ctree,grp1->node,&clr_scan);

    for (chan1=grp1->pro_epi; chan1; chan1=chan1->next)
    {
      Add_Chan_totree(ctree,grp1,chan1);
    }


    for (chan1=grp1->chan; chan1; chan1=chan1->next)
    {
      gboolean complete;
      complete=Add_Chan_totree(ctree,grp1,chan1);
      if (complete) complete1=TRUE;
      if (!complete) completea=FALSE;
    }

    if ((show_complete) && (strcmp(grp1->sat_src,"DWDSAT")) && (grp1->h_or_l!='B'))
    {
      if (completea)
        gtk_ctree_node_set_background(ctree,grp1->node,&clr_compla);
      else if (complete1)
        gtk_ctree_node_set_background(ctree,grp1->node,&clr_compl1);
      else 
        gtk_ctree_node_set_background(ctree,grp1->node,&clr_compl0);
    }
  }

/* Unfreeze tree, i.e., make result visible */
  gtk_clist_thaw((GtkCList *)ctree);
}


extern GLOBINFO globinfo;
/*******************************************************
 * Show tree.
 * clear=true  -> Clear prev. tree and start from scratch.
 * clear=false -> Read new items and add new items to existing tree.
 *******************************************************/
void show_tree(GtkCTree *ctree,GROUP *grp,gboolean clear,gboolean check)
{
  GROUP *grp1;
  if (!ctree) return;
  if (!grp) return;
 
//  Set_Button(ctree,LAB_EXPCOL,FALSE);  /* set exp/col button to 'collapsed' */
  for (grp1=grp; grp1; grp1=grp1->next)
  {
    gboolean is_service=!strcmp(grp1->sat_src,"Srvc");
    gboolean is_foreign=!strcmp(grp1->sat_src,"Frgn");
    gboolean is_mpef   =!strcasecmp(grp1->sat_src,"MPEF");
    gboolean is_meteof =!strncmp(grp1->sat_src,"MET",3) && (strncmp(grp1->sat_src,"METOP",5));
    gboolean is_meteo5 =!strncmp(grp1->sat_src,"MET5",4);
    gboolean is_meteo6 =!strncmp(grp1->sat_src,"MET6",4);
    gboolean is_meteo7 =!strncmp(grp1->sat_src,"MET7",4);
    gboolean is_goes   =!strncmp(grp1->sat_src,"GOES",4);
    gboolean is_goes9  =!strncmp(grp1->sat_src,"GOES9",5);
    gboolean is_goes10 =!strncmp(grp1->sat_src,"GOES10",6);
    gboolean is_goes12 =!strncmp(grp1->sat_src,"GOES12",6);
    gboolean is_mtsat  =!strncmp(grp1->sat_src,"MTSAT",5);
/*
    gboolean is_msg=!strncmp(grp1->sat_src,"MSG",3);
*/
    gboolean is_hrit1=((grp1->h_or_l=='H') && (strchr(grp1->sat_src,'1')));
    gboolean is_hrit2=((grp1->h_or_l=='H') && (strchr(grp1->sat_src,'2')));
    gboolean is_hrit=(grp1->h_or_l=='H');
    gboolean is_lrit1_msg=((grp1->h_or_l=='L')&& (strchr(grp1->sat_src,'1'))&&(!is_foreign) &&
                          (!is_service)&&(!is_meteof)&&(!is_mtsat)&&(!is_goes)&&(!is_mpef));
    gboolean is_lrit2_msg=((grp1->h_or_l=='L')&& (strchr(grp1->sat_src,'2'))&&(!is_foreign) &&
                          (!is_service)&&(!is_meteof)&&(!is_mtsat)&&(!is_goes)&&(!is_mpef));

    gboolean is_lrit_msg=((grp1->h_or_l=='L')&&(!is_foreign));
    gboolean is_avhrr  =((!strncasecmp(grp1->sat_src,"noaa",4)) && (grp1->h_or_l!='B'));  // NOAA avhrr or GAC; no BUFR
    gboolean is_metop  =!strncmp(grp1->sat_src,"METOP",5);
    gboolean is_metop2  =!strncmp(grp1->sat_src,"metop",5);
    gboolean is_dwdsat =!strcmp(grp1->sat_src,"DWDSAT");
    gboolean is_bufr =(grp1->h_or_l=='B');
    gboolean is_ers =!strncmp(grp1->sat_src,"ers",3);
    gboolean is_geo =is_hrit1 | is_hrit2 | is_lrit1_msg | is_lrit2_msg | is_meteof | is_goes | is_mtsat;
    gboolean is_pol =is_metop || is_metop2 || is_avhrr;

    if ((is_goes9) || (is_goes10) || (is_goes12) || (is_mtsat)) is_foreign=TRUE;
    
    grp1->hide=FALSE;
    switch(globinfo.vis_mode)
    {
       case 'S'      : if (!is_service)                      grp1->hide=TRUE; break;
       case 'h'      : if ((!is_hrit1)    || (is_service))   grp1->hide=TRUE; break;
       case 'g'      : if ((!is_hrit2)    || (is_service))   grp1->hide=TRUE; break;
       case 'H'      : if ((!is_hrit))                       grp1->hide=TRUE; break;
       case 'l'      : if ((!is_lrit1_msg))                  grp1->hide=TRUE; break;
       case 'k'      : if ((!is_lrit2_msg))                  grp1->hide=TRUE; break;
       case 'L'      : if ((!is_lrit_msg))                   grp1->hide=TRUE; break;
       case 'f'      : if ((!is_foreign))                    grp1->hide=TRUE; break;
       case 'm'      : if ((!is_mpef))                       grp1->hide=TRUE; break;
       case '5'      : if ((!is_meteo5))                     grp1->hide=TRUE; break;
       case '6'      : if ((!is_meteo6))                     grp1->hide=TRUE; break;
       case '7'      : if ((!is_meteo7))                     grp1->hide=TRUE; break;
       case 'A'      : if ((!is_avhrr))                      grp1->hide=TRUE; break;
       case 'M'      : if ((!is_metop) && (!is_metop2))      grp1->hide=TRUE; break;
       case 'D'      : if ((!is_dwdsat))                     grp1->hide=TRUE; break;
       case 'B'      : if ((!is_bufr))                       grp1->hide=TRUE; break;
       case 'E'      : if ((!is_ers))                        grp1->hide=TRUE; break;
       case 's'      : if ((!is_ers) && (!is_geo))           grp1->hide=TRUE; break;
       case S_pol_all: if (!is_pol)                          grp1->hide=TRUE; break;
       case S_geo_all: if (!is_geo)                          grp1->hide=TRUE; break;
       default:                                              grp1->hide=FALSE;break;
    }
/*
grp1->hide=TRUE;
if (is_hrit2) grp1->hide=FALSE;
if (is_ers) grp1->hide=FALSE;
*/
  }

  clearsel_remnodetree(grp);

  if (clear)
  {
    gtk_clist_clear(GTK_CLIST(ctree));
  }

  create_xrittree(grp,ctree,check); // check=true -> detect/show completeness 
}

