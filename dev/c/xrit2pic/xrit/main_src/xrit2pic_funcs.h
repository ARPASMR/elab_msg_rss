/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Function definitions XRIT2PIC
 ********************************************************************/

/* jpeg and wvt interface funcs */
int gif2str(char *fn,guchar **str,int *w,int *h);
int jpg2str1(char *fn,guchar **str,int *W,int *H);
int jpg2str(SEGMENT *,guint16 **str,guint16 *W,guint16 *H,guint16 *D);
int wvt2str(SEGMENT *,guint16 **str,guint16 *W,guint16 *H,guint16 *D);
int noaa2str(SEGMENT *,gboolean, guint16 *W,guint16 *H,PIC_CHARS *);
int eps2str(SEGMENT *,gboolean,guint16 *W,guint16 *H,PIC_CHARS *);
int pgm2str(SEGMENT *segm,guint16 **str,guint16 *W,guint16 *H,guint16 *D);

int bunzip_to_tempfile(char *ifn,char *ofn);

int gen_jpgfile(GROUP *,PIC_CHARS *,char *,gboolean );
/*
void open_wr_jpeg(FILE *,int ,int ,int ,struct jpeg_compress_struct *,int );
*/
void close_wr_jpeg();
int get_pgmsize_opened(FILE *fpi,int *width,int *height);
void write_pgmhdr(FILE *fp,int Width,int Height,int Depth,int is_color,char *cmt);

/* Next has actually return of j_compress_ptr.
  Return value not important! 
*/

//j_compress_ptr write_jpghdr(FILE *,int ,int ,int ,int );


int write_jpgline(guchar *);

/* Preview functions */
/*void preview(GtkWidget *,GROUP *);*/
void preview_jpg(LIST_FILES *lfi,int nrfiles);
void preview_wvt(LIST_FILES *lfi,int nrfiles);
void Set_Button_All_wnd(GtkWidget *wnd,char *name,gboolean val);
void lumrange(guint32 *lumstat,int lummax,int lumscale,int *ilmin,int *ilmax);
int preview_pic(GtkWidget *,GROUP *,GENMODES *);
void preview_admin(GROUP *,void func());
int Show_Segm(GtkWidget *,GtkWidget *,SEGMENT *,int, gboolean);
void open_textwnd(char *fn,char *name,int width,int height,void mesfunc());
void set_lminmax(guint16 *pix,LUMINFO *li,int pic_lummax,int lummax,int *rgbpilmaxrgb);

/* prev_draw.c */
int draw_pic(GtkWidget *widget);
#ifndef __NOGUI__
void draw_scatarrow(RGBPICINFO *rgbpi,RGBI *rgbi,PIC_CHARS *pc,GdkColor *clr,int x,int y,int x2,int y2);
void draw_line_in_prev(RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int xp,int yp,int x,int y,int,GdkColor *clr);
#endif
/* bufr funcs */
void handle_bufr(GtkWidget *iwnd,GROUP *igrp);
int translate_bufr(char *ifn,char *ofn);
char *scat_extract_to(char *ifn,char *ofn,int *);
GtkWidget *open_earthmap(GtkWidget *, char *,char *,GtkWidget *,GtkWidget *,int dfunc(),void kfunc(),gpointer);
int open_earthmap1(GtkWidget *window,GROUP *grp);
SCATPOINT *nearest_scat(POINT *point);
int scat_extract(char *ifn,SCATPOINT **spx);
ASCATPOINT *Create_AScat(ASCATPOINT **si,SCATPOINT *sp);
int scatt_coverage(GROUP *grp,char *start);
void gen_prev_scat(GtkWidget *wnd,GENMODES *modes,GROUP *grp);
void gen_prev_scatmap(GtkWidget *wnd,GENMODES *modes,GROUP *grp);
int nr_pics_nonbufr(GtkWidget *window,GROUP *grp,GENMODES *gmode);

/* Generate file functions */
int copy_file(char *a,char *b);
void make_destdir(char *dest_dir,PIC_CHARS *pc,char *tfno);
int gen_picfile(GtkWidget *,GROUP *,PREFER *,char *,gboolean,gboolean,GENMODES *);
/*int gen_files(GtkWidget *,GROUP *,PREFER *);*/
int gen_admin(GtkWidget *,GROUP *, PREFER *);
int gen_a_picfile(GROUP *,PIC_CHARS *,char *,gboolean ,gboolean ,GENMODES *);
char *mk_proepi_fn(char *,char *,int );
int gen_proepifile(SEGMENT *,char *);
int gen_proepi_item(GtkWidget *,GROUP *,GENMODES *,PREFER *,char *,gboolean *);
int gen_bin(GtkWidget *,GROUP *,PREFER *,char *,gboolean);
int gen_binitem(GtkWidget *,GROUP *,GENMODES *,PREFER *,char *);
int gen_bufr(GtkWidget *,GROUP *,PREFER *,char *,gboolean );
int gen_bufritem(GtkWidget *,GROUP *,GENMODES *,PREFER *,char *);
char *bufrinfo(GROUP *);
int extract_bufr(FILE *,char *,char *);


int dump_data(SEGMENT *,guint16 **,guint16 *,guint16 *,guint16 *);
int gen_admin_item(GtkWidget *,GROUP *,GENMODES *,PREFER *,char *,gboolean *);
int gen_avi(char *,PREFER *,FILELIST *);
int gen_cpgm(char *,PREFER *);
void auto_translate(GENMODES *,GROUP *,PREFER *,GtkWidget *);

int do_genfile(GROUP *,char *,gboolean,gboolean ,GENMODES *);

gboolean contributes_to_pic(CHANNEL *,char *,char *,gboolean);
int Process_Segm(GtkWidget *,GtkWidget *,GROUP *,gboolean ,gboolean ,gboolean );
int count_files(GROUP *,int *,int *);

/* Common generate functions */
gboolean calc_size(GROUP *,int *,int *,int *);
SEGMENT *y2segm(CHANNEL *chan,int y1,int ymax,int *yi,int *y_chnk);
SEGMENT *get_composed_line(GROUP *,int ,guint16 *rgbstr[3],ANAGL *,gboolean,gboolean);
SEGMENT *store_composed_line(GROUP *grp,int y,guint16 *rgbl[4],ANAGL *anagl,gboolean,gboolean);
void Add_Overlaybits(GROUP *grp,guint16 *rgbstr,int y1);

void load_chanmap(GENMODES *,char *);
gboolean chan_selected(GtkWidget *,char *);
int adapt_lastsegmnr(int tlen,int segm_frst,int segm_last);
GROUP *get_selected_item(GROUP *,GENMODES *);
GROUP *get_selected_item_rng(GROUP *igrp,GENMODES *gmode,gboolean);

int nr_pics(GtkWidget *,GROUP *,GENMODES *);
int chan_complete(CHANNEL *chan,int chnk_first,int chnk_last);

void zoom_size(GENMODES *gmode,PIC_CHARS *pc);
void adapt_filmsize(GENMODES *gmode,PIC_CHARS *pc);
PIC_CHARS pic_info(GROUP *,GtkWidget *,GENMODES *);
int get_rgb(GROUP *,int ,PIC_CHARS *,guint16 **,GENMODES *);
void free_chnks(GROUP *);
void free_segmchnk(SEGMENT *);
char *report_genpicerr(int ,char *);
char *report_genpicerr_1line(int err);
void gen_item(GtkWidget *,GROUP *,GENMODES *,PREFER *);
char *launch_viewer(PREFER *,PIC_CHARS *,char *,GENMODES *);
GROUP *Load_List(GROUP *,GtkCTree *,GtkWidget *,char *,gboolean ,LOCATION_SEGM,gboolean,int *);
GROUP *Load_List_all(GROUP *grp,GtkCTree *ctree,GtkWidget *widget,int *);
void make_filename(char *,PIC_CHARS * ,char *,char *,GENMODES *,gboolean);

int polar_dir(GROUP *grp);
int avhrr_linearize(int x,int avhrr_picwidth);
int inv_xmap(int x,float alpha,PIC_CHARS *pc);
int inv_ymap(int y,float *alpha,PIC_CHARS *pc);
int dxy2fxy(int xi,int yi,int *xo,int *yo,PIC_CHARS *pc);

// next 2 to remove?
int x_map(int x,float alpha,PIC_CHARS *pc);
int y_map(int y,int x,float *alpha,PIC_CHARS *pc);

int glonlat2xy(float lon,float lat,int *c,int *l,GEOCHAR *gchar);
int glonlat2xy_xcorr(float lon,float lat,int *c,int *l,GEOCHAR *gchar);
void gxy2lonlat(int c,int l,float *lon,float *lat,GEOCHAR *gchar);

int fxy2lonlat(int x,int y,float *lon,float *lat,PIC_CHARS *pc);
int lonlat2fxy(float lon,float lat,int *x,int *y,PIC_CHARS *pc);
int lonlat2dxy(float lon,float lat,int *x,int *y,PIC_CHARS *pc);
int lonlat2dxy_bekijk(float lon,float lat,int *x,int *y,PIC_CHARS *pc);
void dxy2lonlat(int x,int y,float *lon,float *lat,PIC_CHARS *pc);
void dxy2lonlat_area(int x,int y,int xoff,int yoff,int xfac,int yfac,float *lon,float *lat,PIC_CHARS *pc);

#ifndef __NOGUI__
void polar_lonlat2xy(POINT pos,RGBI *rgbi,PIC_CHARS *pc,float *px,float *py);
void polar_zoomxy(float fx,float fy,int *px,int *py,RGBPICINFO *rgbpi,RGBI *rgbi,PIC_CHARS *pc);
#endif


int pic2str(SEGMENT *,guint16 **,PIC_CHARS *,gboolean);
int gen_proepi1(SEGMENT *,PIC_CHARS *,PREFER *,char *,GENMODES * );
int gen_proepi2(GtkWidget *,GROUP *,PREFER *,char *,GENMODES *);
FILELIST *Create_Filelist(FILELIST **,char *);
void Remove_Filelist(FILELIST *);
gboolean is_nomsg(char *sat);
gboolean is_mfg(char *sat);
PLACES *Create_Places(PLACES **si);
void Remove_Places(PLACES *s);
LUT *Create_Lut(LUT **si);
void Remove_Lut(LUT *s);
void select_lut(GtkWidget *widget);
//int read_polygon(FILE *fp,GSHHS *h,int lonmin,int lonmax,int latmin,int latmax,int level,int iarea);

/* List functions */
void Destroy_List(LIST_FILES *,gboolean);
void Destroy_Listitem(LIST_FILES *);
void update_filelist(LIST_FILES *,gboolean,int progress_func());
LIST_FILES *create_filelist(char *ipath);
LIST_FILES *Create_List(LIST_FILES *,char *);

gboolean Move_File(SEGMENT *,gboolean);
gboolean in_satchanlist(char *,char *,char,SATCHAN_LIST );

/* popup */
FMAP msgmap2fmap(MSGCMAP mm);

void get_map(GtkWidget *,CHANNEL *,char *,char *);
void set_mapping_from_fixed(COMPOSE_TYPE );
void set_mapping_from_group(GtkWidget *wdgt,GROUP *grp);
void get_mapping(GtkWidget *wdgt,char *wndname);
void rgbmap();
#ifndef __NOGUI__
  void refresh_graph(GtkWidget *window,RGBPICINFO *rgbpi,PIC_CHARS *pc);
#endif
GtkWidget *create_piclist(GtkWidget *,GROUP *,char);
void color_map(GtkWidget *,GROUP *);
void color_map_fixed(GtkWidget *);
void color_map_spec(GtkWidget *window,GROUP *grp);
void set_mapping(GtkWidget *wdgt,int *offset,float *gamma,CHANMAP *chanmap);
void set_mapping_from_cm(GtkWidget *wdgt,MSGCMAP cms);
void fmap2clrwnd(GtkWidget *wnd,FMAP *m);
CHANMAP *Create_Chanmap(CHANMAP **,char *,float,float,float);
CHANNEL *Get_Chan(CHANNEL *chan,char *name);
void Remove_Chanmap(CHANMAP **);
CHANMAP *Get_Chanmap(CHANMAP *,char *);
CHANMAP *Copy_Chanmap(CHANMAP *si);

/* xrit2pic gui */
void create_gui(int argc,char **argv);
void show_tree(GtkCTree *ctree,GROUP *grp,gboolean,gboolean);
int nogui(GENMODES *,PREFER *,gboolean ,gboolean );
void print_log(FILE *fp,char *frmt,...);

void create_colormap_delayed(GtkWidget *twnd);
GtkWidget *nb_main(GtkCTree **);
GtkWidget *nb_prefs(char *);
void save_guistate_all(GtkWidget *widget);
GtkWidget *prefwindow(GtkWidget *,char *);
GtkWidget *nb_record();
GtkWidget *nb_dirsel(GtkCTree *,GtkCList *,char *);
GtkWidget *nb_proginfo(char *);
GtkWidget *nb_filemngr(GtkCList **,GtkCTree *);
GtkWidget *nb_expmngr();
void pol_savesegmselect(GtkWidget *widget,CHANNEL *schan);

void define_and_load_srcdir(GtkWidget *,GtkWidget *);

/* nb_filemngr.c */
int count_dircontent(char *dir);
void forall_selgrp(GROUP *grp,void grpfunc(),void chanfunc(),int segmfunc(),
     gboolean just_selected);
int mv_files_to(GROUP *grp,CHANNEL *chan,SEGMENT *segm,char *archdir);


/* xrit funcs */
FILE *open_xritimage(char *fn,XRIT_HDR *xrit_hdr,DATATYPE,CHANNEL *);
int fn2xrit(char *fn,XRIT_HDR *xh);
void move_segmfiles(SEGMENT *,char *,gboolean);
int delete_old(GROUP **,GtkCTree *,int * ,int, gboolean );
int delete_selected(GROUP **,GtkCTree *);
int archive_selected(GROUP **igrp,GtkCTree *ctree,char *path);
char *fn_selected(GROUP *igrp);
char *finfo_selected(GROUP *igrp);

/* help functions */
int progress_func(char ,char *,int ,int );

/* dbase functions */
void get_pro(FILE *fp,SEGMENT *pro);
GROUP *mk_dbase(LIST_FILES *lf,LOCATION_SEGM,int func());
GROUP *update_dbase(GtkCTree *,GROUP *,LIST_FILES *,LOCATION_SEGM,int func());
void clearsel_remnodetree(GROUP *grp);
void dump_dbase(GROUP *);
int mark_selected(GROUP *,GtkCTreeNode *,gboolean );
void set_selected(GROUP *,GENMODES *);
GROUP *get_first_selected_grp(GROUP *group);

SEGMENT *Get_Segm(CHANNEL *chan,int n);
void Add_Segm(SEGMENT **,SEGMENT *);
SEGMENT *Create_Segm(SEGMENT **);
void Remove_Segms(SEGMENT *);
void Remove_Segm(SEGMENT *);
SEGMENT *Copy_Segm(SEGMENT *);
int Delete_Segm(SEGMENT *,GtkCTree *);
int Delete_Segms(SEGMENT *,GtkCTree *);

void Add_Chan(CHANNEL **,CHANNEL *);
CHANNEL *Create_Chan(CHANNEL **);
void Remove_Chans(CHANNEL *);
void Remove_Chan(CHANNEL *);
CHANNEL *Copy_Chan(CHANNEL *);
CHANNEL *Copy_Chan_Rng(CHANNEL *,int,int);
int Delete_Chan(CHANNEL *,GtkCTree *);
int Delete_Chans(CHANNEL *,GtkCTree *);

GROUP *Create_Grp(GROUP **);
void Remove_Grps(GROUP *);
GROUP *Remove_Grp(GROUP *);
GROUP *Copy_Grp(GROUP *);
GROUP *Copy_Grp_Rng(GROUP *,int,int);
GROUP *Sort_Grp(GROUP *);
int Delete_Grp(GROUP *,GtkCTree *);
int Delete_Grps(GROUP *,GtkCTree *);

#ifndef __NOGUI__
gboolean Add_Chan_totree(GtkCTree *,GROUP *,CHANNEL *);
void Add_Segm_totree(GtkCTree *,GROUP *,CHANNEL *,SEGMENT *);
#endif
gboolean Chan_Contributes(GROUP *grp,char *name);
void load_places(PREFER *prefs);

OVERLAY *load_overlaysfiles(PREFER *);
int add_shore(OVERLAY *ovl,SEGMENT *segm,PIC_CHARS *pc);
OVERLAY *load_overlaysfiles(PREFER *prefs);
char *load_overlay(PREFER *prefer,CHANNEL *chan,OVERLAYTYPE overlaytype,OVERLAY *sel_ovl,int *width,int *height);
#ifndef __NOGUI__
void draw_str_in_prev(RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int x,int y,char *str,GdkColor *clr);
void draw_landmark(GtkWidget *drawing_area,RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int iclr);
void draw_waypoints(GtkWidget *drawing_area,RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int iclr);
#endif
OVERLAYTYPE change_ovltype(PREFER *prefer,OVERLAYTYPE overlaytype);

#ifndef __NOGUI__
void draw_lonlat(GtkWidget *drawing_area,
             RGBI *rgbi,RGBPICINFO *rgbpi,PIC_CHARS *pc,int iclr);
void draw_shore(GtkWidget *drawing_area,RGBI *rgbi,RGBPICINFO *rgbpi,OVERLAY *ovl,PIC_CHARS *pc,int clr);
#endif

void get_lut();

GROUP *gen_polarearth(GROUP *grp,GENMODES *modes,GtkWidget *window);

void Load_Dirfilelist(GtkCList *,char *,char,char *);
int Load_CTree_Dirlist(GtkCTree *,GtkCTreeNode *,char *,int );
SEGMENT *get_linefromsegm(CHANNEL *,int ,gboolean ,int ,int *,int *,PIC_CHARS *);

/* jpeg functions */
int gen_jpgfile(GROUP *,PIC_CHARS *pc,char *fno,gboolean);
int get_jpegsize(char *fni,int *width,int *height);
int get_jpegsize_opened(FILE *fpi,int *width,int *height);

//j_compress_ptr write_jpghdr(FILE *,int ,int ,int ,int );

/* wvt functions */
int wvt_open(char *fni,guint16 *W,guint16 *H,guint16 *D);
void wvt_close();
int wvt_read(guint16 W,guint16 **str,guint16 *chunck_height,unsigned char *bpp);

/* avi functions */
int add_frame(char *,char *,long );
int jpeg2avi(FILELIST *,char *,long );

/* gif.c */
//int read_gif_close(GIF_PICINFO *decoder,FILE *fp);
int read_gif_close(); // GIF_PICINFO not everywhere defined...

/* .rah funcs */
int raw16torah(GROUP *grp,char *fno,gboolean show_progress);
int polar_add_lonlat(SEGMENT *segm,PIC_CHARS *pci);

/* kepler funcs */
int load_kepler(GROUP *grp);
char *mk_noradname(GROUP *grp);
void get_eps_picsize(FILE *fp,int *width,int *height,int *lps);
void calc_orbitpicconst(struct tm *strt_tm,KEPLER *kepler,ORBIT *orbit);
void calc_polar_segmsubsats(GROUP *grp,gboolean force);
int noaa_coverage(GROUP *grp,char *s1,char *s2,char *s3,int,int);
int xy2lonlat(int,int,PIC_CHARS *,POINT *);
void calc_x2satobs(KEPLER *,ORBIT *,POINT *,POINT *,int ,float ,int);

int replace_kepler(char *prog,char *gfile,char *ofile);

int read_genrechdr(/*FILE *fp,HDR *hdr*/);

/* Preferences */
void Load_Defaults(PREFER *);
int Read_Pref(DIRSEL *,PREFER *);
int Save_Pref(GtkCList *,PREFER *,char *);
void pref_to_gui(GtkWidget *,DIRSEL *,PREFER *);
void get_dirs(char *,char **,char **,char **,char **);
float get_gadef(int i,int j);
int search_file(char *,char *,char *,char *,char *);
void finish_path(char *);
void related_dirs(GLOBINFO *,PREFER *);
void Read_PrefDirlist(DIRSEL *dirsel,PREFER *prefer);

/* misc */
char *Create_Entrychoice(char *,char *);
int igamma(int p,int pmax,float g);
void range2int(char *,int *,int *);
char get_cformat(GtkWidget *);
void get_genmode_from_gui(GtkWidget *widget,GENMODES *genmode);
int gen_tempfile(char *);
FILE *open_tempfile(int *,char *);
void close_tempfile(FILE *,int );
int exist_prog(char *fn,char *opt);
int exist_file(char *);
int exist_file_in_dir(char *dir,char *fn);
int overwrite(char *fn,OVERWRMODE mode);
int memcpyd(void **sp,void *s,int n);
int strcpyd(char **sp,char *s);
int strcatd(char **sp,char *s);
int sprintfd(char **s,char *f,...);


void strcatc(char *s,char c);
char *get_strpart(char *l,int s,int e);
void strtoupper(char *a);
gint32 mktime_ntz(struct tm *tm);
int progress_func(char where,char *text,int prgrscnt,int prgrsmax);
void mday_mon2yday(struct tm *tmref);
void yday2mday_mon(struct tm *tmref);
char ftype(char *);
int make_dir(char *);
int make_dir_hier(char *);
int picture_complete(CHANNEL *,PIC_CHARS *, GENMODES *);
int channel_complete(CHANNEL *);
int str_ends_with(char *s,char *e);
int strcasecmpwild(char *s,char *v);
gboolean is_ir_group(GROUP *);
gboolean is_ir_chan(CHANNEL *);
int difftime_tm(struct tm *tm1,struct tm *tm2);
void update_flist1(LIST_FILES *lf);
void tfreenull(char **p);

/* detect funcs */
float val2temp(int pix);
int temp2val(float T);
float pix2temp(SEGMENT *pro,CHANNEL *,int pix);
void temp_clrmap(float t,guint16 *pix);

void fire_filter_line(GROUP *grp,int y1,int ncmaxext,int xmax,int ymax,guint16 *rgbstr[3]);
void conv_to_3d(guint16 *rgbstr[4],guint16 *xlum,int xmax,int pixvalmax,char scandir,ANAGL anagl);


/* overlay */
OVERLAY *get_overlay(OVERLAY *,OVERLAYTYPE ,CHANNEL *,void errmess());
OVERLAYTYPE add_overlay(PREFER *,OVERLAYTYPE ,SEGMENT *,PIC_CHARS *);
void geo_add_lonlat(PREFER *prefer,SEGMENT *segm,PIC_CHARS *pc);
void pri_overlayfiles(FILE *fp,OVERLAY *ol,void func());

/* polar track */
#ifndef __NOGUI__
void pcm_lonlat2xy(RGBI *rgbi,RGBPICINFO *rgbpi,float lon,float lat,int *x,int *y);
void draw_backgrnd(guchar *iaarde,RGBI *rgbi,RGBPICINFO *rgbpi);
#endif
GtkWidget *open_earth(GtkWidget *wndi,gboolean);
void draw_track(GROUP *grp);

/* debug-funcs */
void print_hdrinfo(char *fni,XRIT_HDR *h,FILE *fpo,gboolean one_line);

/* win_funcs.c */
int execcmd(char *cmd,char *env,char *dir,int);
