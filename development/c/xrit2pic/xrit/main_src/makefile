#Define xrit2pic release
RELEASE_=\"2012.1\"

##########################################################
# Choose how to compile
##########################################################
#Detemine whether wavelet funcs have to be linked or not
HAS_WVT="yes"
#HAS_WVT="no"

#Choose linux or windows or solaris
OS="linux"

#Choose gtk version: 2.0 or 1.2 or no
ifeq ($(OS),"linux1")
  GTK_REL="1.2"
  DBGFLAG=-g
  OS="linux"
else
  ifeq ($(OS),"linux")
    GTK_REL="2.0"
  else
    GTK_REL="2.0"
  endif
endif

#GTK_REL="no"

#For GTK_REL="no": Choose endian type (L or B; default: L)
endian=\'L\'
##########################################################

  
ifneq ($(OS),"linux")
  ifneq ($(OS),"windows")
    ifneq ($(OS),"solaris")
all:
	echo "Wrong choice $(OS)."

    endif
  endif
endif

ifeq ($(OS),"solaris")
  OS="linux"
  OS2="solaris"
else
  OS2=
endif

#Compile/link commands 
ifeq ($(HAS_WVT),"no")
  CC=gcc -Wall -Wno-format-y2k -D__HAS_NO_WVT__=1 $(DBGFLAG)
else
  CC=gcc -Wall -Wno-format-y2k $(DBGFLAG)
endif
# -DDEMO
LN=gcc 

ifeq ($(OS),"linux")
########################## Defs for Linux ##########################
  CPP=g++
  PROGRAM=xrit2pic
  ifeq ($(GTK_REL),"no")
    GTKCONFIG_CFLAG=
    GTKCONFIG_LLIBS=
    LOC_SGTK=../../sgtk/sgtk_src
    CFLAGS_EXTRA=-D__NOGUI__ -D__ENDIAN__=$(endian)
  else
    CFLAGS_EXTRA=
    ifeq ($(GTK_REL),"2.0")
      GTKCONFIG_CFLAG=`pkg-config --cflags gtk+-2.0` -D__GTK_20__ 
      GTKCONFIG_LLIBS=`pkg-config --libs gtk+-2.0` -lm
    else
      GTKCONFIG_CFLAG=`gtk-config --cflags`
      GTKCONFIG_LLIBS=`gtk-config --libs`
    endif
    LOC_SGTK=../../sgtk/sgtk_src
  endif
  LOC_JPEGSRC=../../../jpeg812_src
  DLIB_JPEG=-L$(LOC_JPEGSRC) -ljpeg812
  LOC_WVT=../wvt/DecompWT_Library/Src
  #LOC_WVTLIB=wvt
  LOC_WVTLIB=../../../decompr_64
  DLIB_WVT=-L$(LOC_WVTLIB) -lwvt

  ifeq ($(OS2),"solaris")
    CFLAGS_EXTRA=$(CFLAGS_EXTRA) -D__SOLARIS__=1
  endif
  Remove=rm
######################################################################
else
########################## Defs for Windows ##########################
  CPP=g++
  PROGRAM=xrit2pic.exe
  ifeq ($(GTK_REL),"no")
    GTKCONFIG_CFLAG=
    GTKCONFIG_LLIBS=
    LOC_SGTK=""
    CFLAGS_EXTRA=-D__GTK_WIN32__=1 -D__NOGUI__ -D__ENDIAN__=$(endian)
  else
    ifeq ($(GTK_REL),"2.0")
#      GTKCONFIG_CFLAG=`pkg-config --cflags gtk+-2.0` -D__GTK_20__ 
#      GTKCONFIG_LLIBS=`pkg-config --libs gtk+-2.0`
      GTKCONFIG_CFLAG=-mms-bitfields -Ic:/tools/wingtk20/include/gtk-2.0 -Ic:/tools/wingtk20/lib/gtk-2.0/include -Ic:/tools/wingtk20/include/atk-1.0 -Ic:/tools/wingtk20/include/cairo -Ic:/tools/wingtk20/include/pango-1.0 -Ic:/tools/wingtk20/include/glib-2.0 -Ic:/tools/wingtk20/lib/glib-2.0/include -Ic:/tools/wingtk20/include/libpng12
      GTKCONFIG_LLIBS= -Lc:/tools/wingtk20/lib -lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lpangowin32-1.0 -lgdi32 -lpangocairo-1.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lintl -mwindows
      CFLAGS_EXTRA=-D__GTK_WIN32__=1 -D__GTK_20__ 
    else
      GTKCONFIG_CFLAG=-Ic:/bin/wingnu/gtk/include/gppm3 -Ic:/bin/wingnu/gtk/include/gtkp -Ic:/bin/wingnu/gtk/include/gtkp/gdk
      GTKCONFIG_LLIBS=-Lc:/bin/wingnu/gtk/lib -lgtk-1.3 -lgdk-1.3 -lglib-1.3 
# -mwindows
      CFLAGS_EXTRA=-D__GTK_WIN32__=1
    endif
    LOC_SGTK=../sgtk
  endif

  LOC_JPEGSRC=../jpeg812
  DLIB_JPEG=-L$(LOC_JPEGSRC) -ljpeg812
  LOC_WVT=wvt/v2/DecompWT_Library/Src
  LOC_WVTLIB=wvt
  DLIB_WVT=-L$(LOC_WVTLIB) -lwvt

  Remove=delete
######################################################################
endif

CCFLAGS=$(LOC_INCS) $(GTKCONFIG_CFLAG) $(CFLAGS_EXTRA)

#Locations
ifeq ($(LOC_SGTK),"")
  LOC_INCS=-I$(LOC_JPEGSRC)
else
  LOC_INCS=-I$(LOC_JPEGSRC) -I$(LOC_SGTK)
endif

#Source files
SRCA = xrit2pic.c \
       xrit2pic_nogui.c \
       xrithdr_extr.c \
       wvt_funcs.c \
       jpeg_readfuncs.c \
       jpeg_writefuncs.c \
       jpeg_funcs.c \
       pgm_funcs.c \
       metop_funcs.c \
       avhrr_funcs.c \
       avhrr_rah.c \
       kepler_calc.c \
       gif.c \
       list_files.c \
       dbg_helpfuncs.c \
       prefer.c \
       gen_file.c \
       bufr_rts.c \
       gen_common.c \
       gen_polar_smap.c \
       sat_orbit.c \
       selecting.c \
       clrmap.c \
       clrmap_gui.c \
       detect_alg.c \
       prepare.c \
       mkdbase_rts.c \
       mkdbase.c \
       misc.c \
       gen_bintext.c \
       gen_pro_epi.c \
       gen_avi.c \
       gen_picfile.c \
       geomapping.c \
       file_mngr.c \
       overlay.c \
       draw_overlay.c \
       lut.c \
       win_funcs.c

SRCG = gen_live.c \
       gen_prev.c \
       prev_draw.c \
       prev_scat.c \
       polar_map.c \
       mktree.c \
       popup_wnd.c \
       nb_main.c \
       nb_record.c \
       nb_dirsel.c \
       nb_filemngr.c \
       nb_expmngr.c \
       nb_prefs.c \
       nb_proginfo.c \
       earthmap.c \
       xrit2pic_gui.c

SRCN = misc_nogtk.c   


#GTK sgtk files
SRC_SGTK1=$(LOC_SGTK)/sgtk_misc.c $(LOC_SGTK)/windows.c $(LOC_SGTK)/filemngr.c \
        $(LOC_SGTK)/canvas.c $(LOC_SGTK)/rgbpic.c $(LOC_SGTK)/buttons.c \
        $(LOC_SGTK)/listtree.c \
        $(LOC_SGTK)/adjust.c $(LOC_SGTK)/menus.c $(LOC_SGTK)/packing.c \
        $(LOC_SGTK)/rgbdraw.c $(LOC_SGTK)/gtk_led.c

############################################################################
# Test alloc
SRC_SGTK=$(SRC_SGTK1) # $(LOC_SGTK)/test_alloc.c

OBJ_SGTK= $(SRC_SGTK:.c=.o)


ifeq ($(GTK_REL),"no")
  SRC=$(SRCA) $(SRCN)
  SRC_ALL=$(SRC)
else
  SRC=$(SRCA) $(SRCG)
  SRC_ALL=$(SRC) $(SRC_SGTK)
  INC_SGTK=$(LOC_SGTK)/sgtk.h $(LOC_SGTK)/gtk_led.h
  INC_SGTKALL=$(INC_SGTK) $(LOC_SGTK)/sgtk_functions.h
endif

OBJ_ALL= $(SRC_ALL:.c=.o)

#######################
#wavelet routines 
#lib files
LIBOBJ_JPG=$(DLIB_JPEG)
LIBOBJ_WVT=$(DLIB_WVT)

#######################
#jpeg includes needed for compile
INC_JPG=$(LOC_JPEGSRC)/jinclude.h \
        $(LOC_JPEGSRC)/cderror.h \
        $(LOC_JPEGSRC)/cdjpeg.h \
        $(LOC_JPEGSRC)/jmorecfg.h \
        $(LOC_JPEGSRC)/jpeglib.h \
        $(LOC_JPEGSRC)/jconfig.h \
        $(LOC_JPEGSRC)/jerror.h

#Intermediate files
#OBJ = $(SRC:.c=.o)
#OBJ_SGTK = $(SRC_SGTK:.c=.o)

INC1 = vcdu.h vcdu_funcdef.h jpeg_funcs.h \
       xrit2pic.h eps.h gif.h avifrmt.h avhrr.h xrit_prefs.h preview.h overlay.h
ifeq ($(GTK_REL),"no")
  INC = $(INC1) xrit2pic_nogtk.h 
else
  INC = $(INC1)
endif

INC_ALL=$(INC) xrit2pic_funcs.h  xrit2pic_nogtk.h $(INC_SGTKALL)

############################################################################
#Dynamic link
$(PROGRAM): $(OBJ_ALL) $(OBJ_WVT)
ifeq ($(HAS_WVT),"no")
	#$(LN) $(LDFLAGS) -o $@ $(OBJ)  $(LIBOBJ_JPG) $(OBJ_SGTK) $(GTKCONFIG_LLIBS)
	$(LN) $(LDFLAGS) -o $@ $(OBJ) $(OBJ_SGTK) $(LIBOBJ_JPG) $(GTKCONFIG_LLIBS)
else
	$(LN) $(LDFLAGS) -o $@ $(OBJ_ALL) $(LIBOBJ_WVT) $(LIBOBJ_JPG) $(GTKCONFIG_LLIBS)
endif

############################################################################


%.o: %.c $(INC) $(INC_SGTK)
	$(CC) -c $(CCFLAGS) -o $@ $<
clrmap.o: clrmap.c msgmappings.h xrit2pic.h
	$(CC) -c $(CCFLAGS) -o $@ $<
 
xrit2pic.o: xrit2pic.c makefile $(INC)
	$(CC) -c $(CCFLAGS) -DRELEASE=$(RELEASE_) -o $@ $<

clean:
	$(Remove) $(OBJ_ALL)

ifeq ($(GTK_REL),"2.0")
web: xrit2pic_k26.tgz
xrit2pic_k26.tgz: xrit2pic
	tar -czf xrit2pic_k26.tgz xrit2pic
else
web: xrit2pic.tgz
xrit2pic.tgz: xrit2pic
	tar -czf xrit2pic.tgz xrit2pic
endif

arch: main_src.zip
main_src.zip: $(SRCA) $(SRCG) $(SRCN) $(INC) $(INC_JPG) xrit2pic_funcs.h xrit2pic_nogtk.h msgmappings.h makefile
	zip main_src.zip $(SRCA) $(SRCG) $(SRCN) $(INC) $(INC_JPG) xrit2pic_funcs.h xrit2pic_nogtk.h msgmappings.h makefile

arch_all: xrit2pic_all.zip
xrit2pic_all.zip: $(SRCA) $(SRCG) $(SRCN) $(SRC_SGTK) $(INC_ALL) makefile ../sgtk/makefile
	zip xrit2pic_all.zip $(SRCA) $(SRCG) $(SRCN) $(SRC_SGTK) $(INC_ALL) makefile ../sgtk/makefile

icon: xrit2pic_icon.exe
xrit2pic_icon.exe: xrit2pic.exe
	mv xrit2pic.exe xrit2pic_noicon.exe
	ResHacker -add xrit2pic_noicon.exe,xrit2pic.exe,wbol.ico,ICONGROUP,ikoon,

