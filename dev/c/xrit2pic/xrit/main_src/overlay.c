#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gif.h"
#include "stdio.h"
#include "xrit2pic.h"

extern GLOBINFO globinfo;
/* codes in overlay files: ('_' = separator)
   met8, met7, met6, met5
   hrv, vis, ir
   full, eur, glf, frs
   latlong, countries
*/

char ovlfiles[][50]={"met8_0d_full.gif",
                     "met8_0d_full_latlong.gif",
                     "met8_0d_full_countries.gif",
                     "met8_0d_full_latlong_countries.gif",
                     "met8_0d_full_hrv.gif",
                     "met8_0d_full_hrv_latlong.gif",
                     "met8_0d_full_hrv_countries.gif",
                     "met8_0d_full_hrv_latlong_countries.gif",
                     
                     "met9_0d_full.gif",
                     "met9_0d_full_latlong.gif",
                     "met9_0d_full_countries.gif",
                     "met9_0d_full_latlong_countries.gif",
                     "met9_0d_full_hrv.gif",
                     "met9_0d_full_hrv_latlong.gif",
                     "met9_0d_full_hrv_countries.gif",
                     "met9_0d_full_hrv_latlong_countries.gif",

                     "msg_0d_full_hrv.gif",
                     "msg_0d_full_hrv_countries.gif",
                     "msg_0d_full_hrv_latlong.gif",
                     "msg_0d_full_hrv_latlong_countries.gif",
                     "msg_0d_full_ir.gif",
                     "msg_0d_full_ir_countries.gif",
                     "msg_0d_full_ir_latlong.gif",
                     "msg_0d_full_ir_latlong_countries.gif",

                     "msg_9.5d_rss_hrv.gif",
                     "msg_9.5d_rss_hrv_countries.gif",
                     "msg_9.5d_rss_hrv_latlong.gif",
                     "msg_9.5d_rss_hrv_latlong_countries.gif",
                     "msg_9.5d_rss_ir.gif",
                     "msg_9.5d_rss_ir_countries.gif",
                     "msg_9.5d_rss_ir_latlong.gif",
                     "msg_9.5d_rss_ir_latlong_countries.gif",

                     "met7_57d_full_ir.gif",
                     "met7_57d_full_ir_latlong.gif",
                     "met7_57d_full_ir_countries.gif",
                     "met7_57d_full_ir_latlong_countries.gif",
                     "met7_57d_full_vis.gif",
                     "met7_57d_full_vis_latlong.gif",
                     "met7_57d_full_vis_countries.gif",
                     "met7_57d_full_vis_latlong_countries.gif",

                     "met7_0d_full_ir.gif",
                     "met7_0d_full_ir_latlong.gif",
                     "met7_0d_full_ir_countries.gif",
                     "met7_0d_full_ir_latlong_countries.gif",
                     "met7_0d_full_vis.gif",
                     "met7_0d_full_vis_latlong.gif",
                     "met7_0d_full_vis_countries.gif",
                     "met7_0d_full_vis_latlong_countries.gif",

                     "met7_full_ir.gif",
                     "met7_full_ir_latlong.gif",
                     "met7_full_ir_countries.gif",
                     "met7_full_ir_latlong_countries.gif",
                     "met7_full_vis.gif",
                     "met7_full_vis_latlong.gif",
                     "met7_full_vis_countries.gif",
                     "met7_full_vis_latlong_countries.gif",

                     "met5_63d_full_ir.gif",
                     "met5_63d_full_ir_latlong.gif",
                     "met5_63d_full_ir_countries.gif",
                     "met5_63d_full_ir_latlong_countries.gif",
                     "met5_63d_full_vis.gif",
                     "met5_63d_full_vis_latlong.gif",
                     "met5_63d_full_vis_countries.gif",
                     "met5_63d_full_vis_latlong_countries.gif",
                     "met5_63d_glf_ir.gif",
                     "met5_63d_glf_ir_latlong.gif",
                     "met5_63d_glf_ir_countries.gif",
                     "met5_63d_glf_ir_latlong_countries.gif",
                     "met5_63d_glf_vis.gif",
                     "met5_63d_glf_vis_latlong.gif",
                     "met5_63d_glf_vis_countries.gif",
                     "met5_63d_glf_vis_latlong_countries.gif",

                     "met6_0d_frs_ir.gif",
                     "met6_0d_frs_ir_latlong.gif",
                     "met6_0d_frs_ir_countries.gif",
                     "met6_0d_frs_ir_latlong_countries.gif",
                     "met6_0d_frs_vis.gif",
                     "met6_0d_frs_vis_latlong.gif",
                     "met6_0d_frs_vis_countries.gif",
                     "met6_0d_frs_vis_latlong_countries.gif",
                     "met6_0d_eur_ir.gif",
                     "met6_0d_eur_ir_latlong.gif",
                     "met6_0d_eur_ir_countries.gif",
                     "met6_0d_eur_ir_latlong_countries.gif",
                     "met6_0d_eur_vis.gif",
                     "met6_0d_eur_vis_latlong.gif",
                     "met6_0d_eur_vis_countries.gif",
                     "met6_0d_eur_vis_latlong_countries.gif",
                     "met6_10d_frs_ir.gif",
                     "met6_10d_frs_ir_latlong.gif",
                     "met6_10d_frs_ir_countries.gif",
                     "met6_10d_frs_ir_latlong_countries.gif",
                     "met6_10d_frs_vis.gif",
                     "met6_10d_frs_vis_latlong.gif",
                     "met6_10d_frs_vis_countries.gif",
                     "met6_10d_frs_vis_latlong_countries.gif",
                     "met6_10d_eur_ir.gif",
                     "met6_10d_eur_ir_latlong.gif",
                     "met6_10d_eur_ir_countries.gif",
                     "met6_10d_eur_ir_latlong_countries.gif",
                     "met6_10d_eur_vis.gif",
                     "met6_10d_eur_vis_latlong.gif",
                     "met6_10d_eur_vis_countries.gif",
                     "met6_10d_eur_vis_latlong_countries.gif",
                     "met6_10d_glf_ir.gif",
                     "met6_10d_glf_ir_latlong.gif",
                     "met6_10d_glf_ir_countries.gif",
                     "met6_10d_glf_ir_latlong_countries.gif",
                     "met6_10d_glf_vis.gif",
                     "met6_10d_glf_vis_latlong.gif",
                     "met6_10d_glf_vis_countries.gif",
                     "met6_10d_glf_vis_latlong_countries.gif",

                     "goes11_-135d_full_countries.gif",
                     "goes11_-135d_full.gif",
                     "goes11_-135d_full_latlong_countries.gif",
                     "goes11_-135d_full_latlong.gif",
                     "goes11_-135d_full_wv_countries.gif",
                     "goes11_-135d_full_wv.gif",
                     "goes11_-135d_full_wv_latlong_countries.gif",
                     "goes11_-135d_full_wv_latlong.gif",
                     "goes12_-75d_full_countries.gif",
                     "goes12_-75d_full.gif",
                     "goes12_-75d_full_latlong_countries.gif",
                     "goes12_-75d_full_latlong.gif",
                     "goes12_-75d_full_wv_countries.gif",
                     "goes12_-75d_full_wv.gif",
                     "goes12_-75d_full_wv_latlong_countries.gif",
                     "goes12_-75d_full_wv_latlong.gif",
                     "mtsat1r_140d_full_countries.gif",
                     "mtsat1r_140d_full.gif",
                     "mtsat1r_140d_full_latlong_countries.gif",
                     "mtsat1r_140d_full_latlong.gif",
                     "gshhs_i.b",
                     ""
                    };

//                     "goes10_countries.gif",
//                     "goes11_countries.gif",
//                     "goes12_countries.gif",
//                     "mtsat1r_countries.gif",


static OVERLAY *Create_Overlay(OVERLAY *ol,char *fn)
{
  OVERLAY *olnw;
  olnw=calloc(1,sizeof(*olnw));

  if (ol)
  {
    while (ol->next) ol=ol->next;
    ol->next=olnw;
    olnw->prev=ol;
  }
  if (fn) strcpy(olnw->fname,fn);

  return olnw;

}

// read (next) waypoint from GPX file
static int rd_wp(FILE *fp,float *lat, float *lon,char *name)
{
  char l[1000],*p;
  int ret=0;
  int flag=0;
  while  (fgets(l,1000,fp))
  {
//   <wpt lat="52.500000" lon="4.400000">
    if (strstr(l,"<wpt")) flag=1;
    if (flag)
    {
      if ((p=strstr(l,"lat=\"")))
      {
        p+=5;
        *lat=atof(p);
        ret|=0x1;
      }
      if ((p=strstr(l,"lon=\"")))
      {
        p+=5;
        *lon=atof(p);
        ret|=0x2;
      }
      if ((p=strstr(l,"<name>")))
      {
        p+=6;
        strncpy(name,p,20);
        if ((p=strstr(name,"</name>"))) *p=0;
        ret|=0x2;
      }
    }
    if (strstr(l,"</wpt")) break; 
  }
  if (((ret&0x3)==0x3)) return 1;
  return 0;
}

// load place locations (waypoints) from GPX file
void load_places(PREFER *prefs)
{
  FILE *fp;
  PLACES *pl;
  float lon,lat;
  char placefile[300];
  char name[40];
  Remove_Places(prefs->places); prefs->places=NULL;
  if (!(search_file(prefs->placesfile,placefile,prefs->cur_dir,prefs->home_dir,prefs->prog_dir))) return;
  if (!(fp=fopen(placefile,"r"))) return;
  while (rd_wp(fp,&lat,&lon,name))
  {
    pl=Create_Places(&prefs->places);
    pl->p.lon=lon;
    pl->p.lat=lat;
    strcpy(pl->name,name);
  }
  fclose(fp);
}

// create linked list with overlay file info
OVERLAY *load_overlaysfiles(PREFER *prefs)
{
  int i;
  OVERLAY *overlay=NULL;
  for (i=0; ; i++)
  {
    if (!*ovlfiles[i]) break;
    overlay=Create_Overlay(overlay,ovlfiles[i]);
    if (exist_file_in_dir(prefs->cur_dir,overlay->fname)) overlay->dir=prefs->cur_dir;
    else if (exist_file_in_dir(prefs->home_dir,overlay->fname)) overlay->dir=prefs->home_dir;
    else if (exist_file_in_dir(prefs->prog_dir,overlay->fname)) overlay->dir=prefs->prog_dir;
    else if (exist_file_in_dir(prefs->prog_xdir,overlay->fname)) overlay->dir=prefs->prog_xdir;
    else overlay->dir=NULL;

    if (!strstr(overlay->fname,".gif")) 
    {
      overlay->overlaytype=coast_v;
      strcpy(overlay->satname,"All");
      overlay->sattype=-1;
      continue;
    }
    overlay->overlaytype='C';
    overlay->rapidscan=FALSE;
    if (strstr(overlay->fname,"latlong")) overlay->overlaytype='l';
    if (strstr(overlay->fname,"countries")) overlay->overlaytype=(overlay->overlaytype=='l'? 'b' : 'c');
    if (strstr(overlay->fname,"_hrv")) strcpy(overlay->channel,"hrv");
    if (strstr(overlay->fname,"_ir")) strcpy(overlay->channel,"ir");
    if (strstr(overlay->fname,"_wv")) strcpy(overlay->channel,"wv");
    if (strstr(overlay->fname,"_vis")) strcpy(overlay->channel,"vis");
    if (strstr(overlay->fname,"_frs")) overlay->rapidscan=TRUE;
    if (strstr(overlay->fname,"_rss")) overlay->rapidscan=TRUE;
    overlay->satpos=999;     // ignore position sat
    if (!strncmp(overlay->fname,"met",3))
    {
      strcpy(overlay->satname,"met");
      overlay->sattype=atoi(overlay->fname+3);
      if ((isdigit(*(overlay->fname+5))) || (*(overlay->fname+5)=='-'))
        overlay->satpos=atoi(overlay->fname+5);
    }
    if (!strncmp(overlay->fname,"msg",3))
    {
      strcpy(overlay->satname,"msg");
      overlay->sattype=0;
      if ((isdigit(*(overlay->fname+4))) || (*(overlay->fname+4)=='-'))
        overlay->satpos=atoi(overlay->fname+4);
    }
    if (!strncmp(overlay->fname,"goes",4))
    {
      strcpy(overlay->satname,"goes");
      overlay->sattype=atoi(overlay->fname+4); 
      if ((isdigit(*(overlay->fname+7))) || (*(overlay->fname+7)=='-'))
        overlay->satpos=atoi(overlay->fname+7);
    }
    if (!strncmp(overlay->fname,"mtsat",5))
    {
      strcpy(overlay->satname,"mtsat");
      overlay->sattype=atoi(overlay->fname+5); 
      if ((isdigit(*(overlay->fname+8))) || (*(overlay->fname+8)=='-'))
        overlay->satpos=atoi(overlay->fname+8);
    }
  }

  rewind_s(overlay);
  return overlay;
}

// Correct this for vectorized ol!
OVERLAY *get_overlay(OVERLAY *ovl,OVERLAYTYPE overlaytype,CHANNEL *chan,void errmess())
{
  for (; ovl; ovl=ovl->next)
  {
    if (!ovl->dir) continue;
    if (overlaytype!=ovl->overlaytype) continue;
    if (overlaytype==coast_v) return ovl;   // ?? slaat nergens op om hier te doen!?

    if ((!strcmp(ovl->satname,"msg")) && (!strncmp(chan->data_src,"MSG",3)) && (ovl->satpos==(int)chan->segm->xh.sub_lon))
    {
      if (!strcmp(chan->chan_name,"HRV"))
      {
        if (!strcmp(ovl->channel,"hrv"))  return ovl;
      }
      else
      {
        if (!strcmp(ovl->channel,""))  return ovl;
        if (!strcmp(ovl->channel,"ir")) return ovl;
      }
    }

    if ((!strcmp(ovl->satname,"met")) && (!strncmp(chan->data_src,"MSG",3)) && (ovl->satpos==(int)chan->segm->xh.sub_lon))
    {
      if (((ovl->sattype==8) && (!strncmp(chan->data_src,"MSG1",4))) ||
          ((ovl->sattype==9) && (!strncmp(chan->data_src,"MSG2",4))))
      {
        if (!strcmp(chan->chan_name,"HRV"))
        {
          if (!strcmp(ovl->channel,"hrv"))  return ovl;
        }
        else if (!strncmp(chan->chan_name,"VIS",3))
        {
          if (!strcmp(ovl->channel,""))  return ovl;
          if (!strcmp(ovl->channel,"vis")) return ovl;
        }
        else
        {
          if (!strcmp(ovl->channel,""))  return ovl;
          if (!strcmp(ovl->channel,"ir")) return ovl;
        }
      }
    }

    if ((!strcmp(ovl->satname,"met")) && (!strncmp(chan->data_src,"MET",3)) && (ovl->satpos==(int)chan->segm->xh.sub_lon))
    {
      if (((ovl->sattype==7) && (!strncmp(chan->data_src,"MET7",4))) ||
          ((ovl->sattype==6) && (!strncmp(chan->data_src,"MET6",4))) ||
          ((ovl->sattype==5) && (!strncmp(chan->data_src,"MET5",4))))
      {
        if (!strncmp(chan->chan_name,"VIS",3))
        {
          if (!strcmp(ovl->channel,""))  return ovl;
          if (!strcmp(ovl->channel,"vis")) return ovl;
        }
        else
        {
          if (!strcmp(ovl->channel,""))  return ovl;
          if (!strcmp(ovl->channel,"ir")) return ovl;
        }
      }
    }
    if (!strcmp(ovl->satname,"goes"))
    {
      if (((ovl->sattype==10) && (!strcmp(chan->data_src,"GOES10"))) ||
          ((ovl->sattype==11) && (!strcmp(chan->data_src,"GOES11"))) ||
          ((ovl->sattype==12) && (!strcmp(chan->data_src,"GOES12"))))
      {
        if (!strncmp(chan->chan_name,"VIS",3))
        {
          if (!strcmp(ovl->channel,""))  return ovl;
          if (!strcmp(ovl->channel,"vis")) return ovl;
        }
        else if (!strncmp(chan->chan_name,"WV",2))
        {
          if (!strcmp(ovl->channel,"wv"))  return ovl;
        }
        else
        {
          if (!strcmp(ovl->channel,""))  return ovl;
          if (!strcmp(ovl->channel,"ir")) return ovl;
        }
      }
    }
    if (!strcmp(ovl->satname,"mtsat"))
    {
      if (!strcmp(chan->data_src,"MTSAT1R")) return ovl;
    }
  }

  if (errmess)
  {
    errmess("Error","Overlay %s for this picture:\n  %s channel %s type '%s'\n\nSee preferences window for search-locations\nand menu 'File -> Show overlayfiles'.",
      (ovl? "not found" : "unknown"),
      chan->data_src,chan->chan_name,
        (overlaytype=='c'? "country" : 
         overlaytype=='l'? "lat_lon" : 
         overlaytype=='b'? "country and latlon" : 
                           "coast only"));
  }
  return NULL;
}


// Load or get already loaded overlay file.
char *load_overlay(PREFER *prefer,CHANNEL *chan,OVERLAYTYPE overlaytype,OVERLAY *sel_ovl,int *width,int *height)
{
  FILE *fp;
  GIF_PICINFO gif_pci;
  static guchar *lines;   // all lines of overlay pic
  static int iwidth,iheight;
  int y;
  char *ofn=NULL;
  OVERLAY *ovl=prefer->overlay;  // linked record of available overlay files
  static OVERLAY *prev_ovl;
  // If this func is called without specifying a overlay file 
  // then clear prev. ovelay.
  if ((!overlaytype) || (overlaytype==coast_v))  //no overlay file
  {
    if (lines) free(lines); lines=NULL;
    return NULL;
  }

  // Get info about overlayfile to use.
  ovl=get_overlay(ovl,overlaytype,chan,NULL);
  if (!ovl) return NULL;  // overlay file not available
  // Now ovl has info about overlay file to use.
//printf("%s  %s  %s  chan=%s  pos=%d\n",ovl->dir,ovl->fname,ovl->satname,ovl->channel,ovl->satpos);

  // If 'lines' and ovl didn't change then overlay already loaded.
  if ((lines) && (ovl==prev_ovl))
  {
    *width=iwidth;
    *height=iheight;
    *sel_ovl=*ovl;
    return (char *)lines;
  }

  // Determine file + path to file
  if (ovl->dir)
  {
    strcpyd(&ofn,ovl->dir);
    finish_path(ofn);
    strcatd(&ofn,ovl->fname);
  }
  if (!ofn) return NULL;  // error (out-of-mem)
  
  fp=fopen(ofn,"rb");
  free(ofn);
  if (!fp)
  {
    return NULL;
  }

  // read header of ovelay file
  memset(&gif_pci,0,sizeof(gif_pci));
  read_gif_header(fp,&gif_pci.clrmap, &gif_pci);

  // Allocate space for new overlay pic
  if (lines) free(lines);
  lines=malloc(gif_pci.width*gif_pci.height);

  if (!lines)   // error
  {
    read_gif_close(&gif_pci,fp);
    fclose(fp);
    return NULL;
  }
  prev_ovl=ovl;    // save current overlay type
  
  // Read overlay file into 'lines'
  read_gif_line(fp,&gif_pci,-1,lines);  // init
  for (y=0; y<gif_pci.height; y++)
    read_gif_line(fp,&gif_pci,y,lines+y*gif_pci.width);
  read_gif_close(&gif_pci,fp);
  fclose(fp);

  iwidth=gif_pci.width;
  iheight=gif_pci.height;

  *width=iwidth;
  *height=iheight;
  *sel_ovl=*ovl;
  return (char *)lines;
}

#define OLTYPE2STR(ot) (\
   ot==coast_p?   "coast pic"   : \
   ot==lonlat_p?  "latlon pic"  : \
   ot==country_p? "country pic" : \
   ot==both_p?    "both pic"    : \
   ot==coast_v?   "coast"       : \
   "unknown")

void pri_overlayfiles(FILE *fp,OVERLAY *ol,void func())
{
  func(fp,"%-6s | %-7s | %-4s | File   \n","Sat","Overlay","Chan");
  for (; ol; ol=ol->next)
  {
    func(fp,"meteo%d | %-7s | %-4s |%s/%s   \n",
      ol->sattype,
      OLTYPE2STR(ol->overlaytype),
      ol->channel,
      (ol->dir? ol->dir : "????"),ol->fname
    );
  }
}
extern PREFER prefer;
#ifndef __NOGUI__
void list_overlay(GtkWidget *widget)
{
  GtkWidget *wnd,*wl;
  char *tmp[5];
  char str[20];
  int row=0;
  OVERLAY *ol;
  wnd=Create_Window(widget,550,400,"Overlay files",NULL);
  wl=Create_Clist("Clist",NULL,NULL,NULL,5,
    "Satellite",6,"Overlay",6,"Channel",6,"File",28,"Location",20,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),wl->parent);

  gtk_widget_show(wnd);
  prefer.overlay=load_overlaysfiles(&prefer);
  for (ol=prefer.overlay; ol; ol=ol->next)
  {
    if (ol->sattype>=0)
      sprintf(str,"%s%d",ol->satname,ol->sattype);
    else
      sprintf(str,"%s",ol->satname);
    tmp[0]=str;
    tmp[1]=OLTYPE2STR(ol->overlaytype);
    tmp[2]=ol->channel;
    tmp[3]=ol->fname;
    tmp[4]=(ol->dir? ol->dir : "Not found");
    gtk_clist_append(GTK_CLIST(wl), tmp);
    gtk_clist_set_selectable(GTK_CLIST(wl),row++,FALSE);
  }
} 


#endif
#include "overlay.h"

int read_polygon(FILE *fp,GSHHS *h,int lonmin,int lonmax,int latmin,int latmax,int level,int iarea)
{
  double w, e, s, n, area;
  char source;

  if (!(fread(h,sizeof(GSHHS),1,fp))) return 0;
  h->id       =GINT32_FROM_BE(h->id);
  h->n        =GINT32_FROM_BE(h->n);
  h->level    =GINT32_FROM_BE(h->level);
  h->west     =GINT32_FROM_BE(h->west);
  h->east     =GINT32_FROM_BE(h->east);
  h->south    =GINT32_FROM_BE(h->south);
  h->north    =GINT32_FROM_BE(h->north);
  h->area     =GINT32_FROM_BE(h->area);
  h->greenwich=GINT16_FROM_BE(h->greenwich);
  h->source   =GINT16_FROM_BE(h->source);

  w = h->west  * 1.0e-6;	/* Convert from microdegrees to degrees */
  e = h->east  * 1.0e-6;
  s = h->south * 1.0e-6;
  n = h->north * 1.0e-6;

  if (w>180) w-=360;
  if (e>180) e-=360;

  source = (h->source == 1) ? 'W' : 'C';	/* Either WVS or CIA (WDBII) pedigree */
  area = 0.1 * h->area;			/* Now im km^2 */

//printf("%d ... %d  %d ... %d  %d ... %d  %d ... %d\n",lonmin,lonmax,latmin,latmax,(int)w,(int)e,(int)n,(int)s);

//printf("%d  %d\n",(int)area,area);
  if ((lonmax>w) && (lonmin<e) && (latmax>s) && (latmin<n) && (area>iarea) && ((!level)||(level>=h->level)))
  {
    return 1;
  }
  else
  {   
    fseek (fp, (long)(h->n * sizeof(POINTx)), SEEK_CUR);
    return -1;
  }
}

OVERLAYTYPE change_ovltype(PREFER *prefer,OVERLAYTYPE overlaytype)
{
  if (prefer->prf_ovltype=='V')
  {
    if (overlaytype==coast_p) overlaytype=coast_v;
  }
  else
  {
    if (overlaytype==coast_v) overlaytype=coast_p;
  }
  return overlaytype;
}

#ifdef OUD
/* Return overlay info for type 'overlaytype' of channel 'chan' */
OVERLAY *get_overlay1(OVERLAY *ovl,OVERLAYTYPE overlaytype,CHANNEL *chan,void errmess())
{
  for (; ovl; ovl=ovl->next)
  {
    if (overlaytype==coast_v)
    {
      if (ovl->overlaytype==overlaytype) return ovl;
      continue;
    }
    if (!strcmp(ovl->satname,"msg"))
    {
      if ((ovl->sattype==0) && (strncmp(chan->data_src,"MSG",3)) && (ovl->satpos!=(int)chan->segm->xh.sub_lon)) continue;
      if ((!strcmp(ovl->channel,"hrv")) && (strcmp(chan->chan_name,"HRV"))) continue;
      if ((!strcmp(ovl->channel,"")) && (!strcmp(chan->chan_name,"HRV"))) continue;
      if ((!strcmp(ovl->channel,"ir")) && (!strcmp(chan->chan_name,"HRV"))) continue;
    }
    if (!strcmp(ovl->satname,"met"))
    {
      if ((ovl->sattype==8) && ((strncmp(chan->data_src,"MSG1",4)) && (strcmp(chan->data_src,"MSG")) && (chan->satpos!=0))) continue;
      if ((ovl->sattype==9) && (strncmp(chan->data_src,"MSG2",4))) continue;
      if ((ovl->sattype==7) && (strcmp(chan->data_src,"MET7"))) continue;
      if ((ovl->sattype==6) && (strcmp(chan->data_src,"MET6"))) continue;
      if ((ovl->sattype==5) && (strcmp(chan->data_src,"MET5"))) continue;
      if ((!strcmp(ovl->channel,"hrv")) && (strcmp(chan->chan_name,"HRV"))) continue;
      if ((!strcmp(ovl->channel,"")) && (!strcmp(chan->chan_name,"HRV"))) continue;
      if ((!strcmp(ovl->channel,"vis")) && (strncmp(chan->chan_name,"VIS",3))) continue;
      if ((!strcmp(ovl->channel,"ir")) && (strncmp(chan->chan_name,"IR",2))) continue;
    }
    if (!strcmp(ovl->satname,"goes"))
    {
      if ((ovl->sattype==10) && (strcmp(chan->data_src,"GOES10"))) continue;
      if ((ovl->sattype==11) && (strcmp(chan->data_src,"GOES11"))) continue;
      if ((ovl->sattype==12) && (strcmp(chan->data_src,"GOES12"))) continue;
    }
    if ((ovl->satpos!=999) && (ovl->satpos!=chan->satpos)) continue;
    if (overlaytype==ovl->overlaytype) break;
  }

  if ((!ovl) || (!ovl->dir))
  {
    if (errmess)
    {
      errmess("Error","Overlay %s for this picture:\n  %s channel %s type '%s'\n\nSee preferences window for search-locations\nand menu 'File -> Show overlayfiles'.",
        (ovl? "not found" : "unknown"),
        chan->data_src,chan->chan_name,
          (overlaytype=='c'? "country" : 
           overlaytype=='l'? "lat_lon" : 
           overlaytype=='b'? "country and latlon" : 
                             "coast only"));
    }
    return NULL;
  }
  return ovl;
}
#endif
