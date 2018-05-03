#Choose linux or windows
OS="linux"

#Choose gtk version: 2.0 or 1.2
GTK_REL="2.0"

DBGFLAG=

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
endif

ifeq ($(OS),"linux")
  ifeq ($(GTK_REL),"2.0")
    GTKCONFIG_CFLAG=`pkg-config --cflags gtk+-2.0` -D__GTK_20__
  else
    GTKCONFIG_CFLAG=`gtk-config --cflags`
  endif
else
  GTKCONFIG_CFLAG=-Ic:/bin/wingnu/gtk/include/gppm3 -Ic:/bin/wingnu/gtk/include/gtkp -Ic:/bin/wingnu/gtk/include/gtkp/gdk
  CFLAGS_EXTRA=-D__GTK_WIN32__=1
endif

CC=gcc -Wall $(INCL_OPT) $(GTKCONFIG_CFLAG) $(DBGFLAG)

LOC_SGTK=.
#GTK sgtk files
SRC_GUI=$(LOC_SGTK)/sgtk_misc.c $(LOC_SGTK)/windows.c $(LOC_SGTK)/filemngr.c \
        $(LOC_SGTK)/canvas.c $(LOC_SGTK)/rgbpic.c \
        $(LOC_SGTK)/buttons.c $(LOC_SGTK)/listtree.c \
        $(LOC_SGTK)/adjust.c $(LOC_SGTK)/menus.c $(LOC_SGTK)/packing.c \
        $(LOC_SGTK)/rgbdraw.c $(LOC_SGTK)/gtk_led.c
INC_GUI=$(LOC_SGTK)/sgtk.h $(LOC_SGTK)/gtk_led.h $(LOC_SGTK)/sgtk_functions.h

ALL_SRC=$(SRC_GUI) 

ALL_INC=$(INC_GUI)
ALL_OBJ=$(ALL_SRC:.c=.o)

all: $(ALL_OBJ) 

%.o: %.c $(ALL_INC)
	$(CC) -c $(CCFLAGS) $(CFLAGS_EXTRA) -o $@ $<

arch:
	zip sgtk_src.zip $(SRC_GUI) $(INC_GUI) makefile
