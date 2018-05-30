/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/* Some locally used types etc. */
typedef struct draw_info
{
  GtkWidget *drawable;
  GtkWidget *drawable_temp;
  GtkWidget *graphwnd;
  gboolean done;
  gboolean drawing;
  gboolean stop_drawing;
  gboolean invert;
  float gamma;
  gboolean avhrr_lin;
  gboolean map_temp;
  gboolean map_temp_G_mA;
  gboolean use_lut;
  gboolean fire_detect;
  GEOMAP gmap;
  int xoff,xfac,yoff,yfac;
  gboolean freeze_pos;
  
  gboolean add_overlay;
  gboolean add_marks;
  gboolean add_cities;
  gboolean add_lonlat;
  gboolean map_loaded;
  ANAGL anagl;
  int ol_lum;
  gboolean is_preview;    // flag that it is a preview to recognize all previews
} DRAW_INFO;

#define PMENU_SEP "separator/"
#define PMENU_FILE "/File"
#define PMENU_SAVEPREV_JPG "File/Save jpeg"
#define PMENU_SAVEPREV_PGM "File/Save pgm"
#define PMENU_SAVEPREV_PGM8 "File/Save pgm8"
#define PMENU_SCREENDUMP "File/Screendump"
#define PMENU_Dismiss "File/Dismiss"

#define PMENU_VIEW   "/View"
#define PMENU_ZOOM   "/View/Zoom"
#define PMENU_ZOOMV   "View/Zoom/Values"
#define PMENU_ZOOMF   "View/Zoom/Full (f)"
#define PMENU_ZOOMI   "View/Zoom/In (i)"
#define PMENU_ZOOMD   "View/Zoom/Detailed"
#define PMENU_ZOOMO   "View/Zoom/Out (o)"
#define PMENU_REDRAW  "View/Redraw"
#define PMENU_FLIP    "View/Flip"
#define PMENU_INVERT  "View/Invert"
#define PMENU_AVLIN   "View/Linearize"

#define PMENU_GMAP    "/View/Projection"
#define PMENU_GMAPN   "View/Projection/Normal"
#define PMENU_GMAPC   "View/Projection/Plate Carree"
#define PMENU_GMAPM   "View/Projection/Mercator"
#define PMENU_GMAPPN   "View/Projection/Polar N"
#define PMENU_GMAPPS   "View/Projection/Polar S"

#define PMENU_OVERLAY "View/Overlay"
#define PMENU_MARK    "View/Markpoint"
#define PMENU_CITY    "View/Cities"
#define PMENU_LONLAT  "View/Lon-Lat"
#define PMENU_FIRE    "View/Fire"
#define PMENU_TEMP    "View/Temp_CLR"
#define PMENU_TEMPG   "View/Temp_BW"
#define PMENU_LUT     "View/Use lut"
#define PMENU_BUFR    "View/Scatterometer"

#define PMENU_ALUM     "Lum"
#define PMENU_ALUMRGB  "Hue"
#define PMENU_PIXSIZE  "View/Square pixel"
#define PMENU_FAST     "View/Fast"
#define PMENU_HELP     "View/Help"
#define PMENU_LUMGRAPH "View/Lumgraph"

#define LAB_ZI "zi "
#define LAB_ZO "zo"
#define LAB_ZF "zf"
#define LAB_Redraw "Redraw"
#define Lab_LMIN "Lmin"
#define Lab_LMAX "Lmax"
#define LAB_ANAGL_PREV "Set Anagl"
#define LAB_FCLR_PREV "Frz clr"
#define LAB_BUT3D "Anaglyph setings"
#define LAB_INIT3D_PREV "Init"
#define LAB_MOREPREV "More "
#define LAB_MOREBUTPREV "!morebuttonbar"
#define LAB_COORDXY "Xy"
#define LAB_COORDLL "Pos"
#define LAB_PIXVAL "Val"

#define LAB_BUSY "!busy"

