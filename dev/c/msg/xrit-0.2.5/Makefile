CXX = g++
CC = gcc
FC = g77
INCLUDE = -I.
LIBDIR = lib
BINDIR = bin
SRCDIR = src
OBJDIR = obj
LIBRARY = -L. -L$(LIBDIR)
LDFLAGS = -lxrit
CFLAGS = -Wall -Wno-trigraphs -std=gnu99 -O1 -g $(INCLUDE) $(LIBRARY) # do not use more aggressive optimization level with gcc 4.* 
                                                                      # the metadata are not correctly read with -O2 and more
                                                                      #  (seems that the packed directive is disabled by -O2)
                                                                      # no apparent trouble with gcc 3.*

all : $(LIBDIR)/libxrit.a \
      $(BINDIR)/xrit2raw \
      $(BINDIR)/read_seviri_prologue \
      $(BINDIR)/read_xrit_header includes \
      $(BINDIR)/put_xrit_segments_together.pl

includes :
	cp src/*.h include

$(BINDIR)/put_xrit_segments_together.pl : $(SRCDIR)/put_xrit_segments_together.pl
	cp $(SRCDIR)/put_xrit_segments_together.pl $(BINDIR)/put_xrit_segments_together.pl

$(BINDIR)/xrit2raw : $(SRCDIR)/xrit2raw.c $(LIBDIR)/libxrit.a
	$(CC) $(CFLAGS) $(SRCDIR)/xrit2raw.c -o $(BINDIR)/xrit2raw -lxrit

$(BINDIR)/read_xrit_header : $(SRCDIR)/read_xrit_header.c $(LIBDIR)/libxrit.a
	$(CC) $(CFLAGS) $(SRCDIR)/read_xrit_header.c -o $(BINDIR)/read_xrit_header -lxrit

$(BINDIR)/read_seviri_prologue: $(SRCDIR)/read_seviri_prologue.c $(LIBDIR)/libxrit.a
	$(CC) $(CFLAGS) $(SRCDIR)/read_seviri_prologue.c -o $(BINDIR)/read_seviri_prologue -lxrit

$(LIBDIR)/libxrit.a : $(OBJDIR)/read_xrit.o $(OBJDIR)/xrit_swap.o $(OBJDIR)/xrit_met8_channel.o
	ar cru $(LIBDIR)/libxrit.a $(OBJDIR)/read_xrit.o $(OBJDIR)/xrit_swap.o $(OBJDIR)/xrit_met8_channel.o
	ranlib $(LIBDIR)/libxrit.a

$(OBJDIR)/read_xrit.o : $(SRCDIR)/read_xrit.c $(SRCDIR)/read_xrit.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/read_xrit.c -o $(OBJDIR)/read_xrit.o

$(OBJDIR)/xrit_swap.o : $(SRCDIR)/xrit_swap.c $(SRCDIR)/xrit_swap.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/xrit_swap.c -o $(OBJDIR)/xrit_swap.o

$(OBJDIR)/xrit_met8_channel.o : $(SRCDIR)/xrit_met8_channel.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/xrit_met8_channel.c -o $(OBJDIR)/xrit_met8_channel.o

forceit :

clean :
	rm -f $(LIBDIR)/*.a $(OBJDIR)/*.o $(BINDIR)/* $(SRCDIR)/*~

