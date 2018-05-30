/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Header for jpeg functions interfacing with IJG
 ********************************************************************/
#define SOI 0xd8
#define SOS 0xda
#define EOI 0xd9
#define DRI 0xdd
#define DQT 0xdb
#define DHT 0xc4
#define SOF0 0xc0
#define SOF1 0xc1
#define RST0 0xd0
#define RST1 0xd1
#define RST2 0xd2
#define RST3 0xd3
#define RST4 0xd4
#define RST5 0xd5
#define RST6 0xd6
#define RST7 0xd7

#define ZERO 0x100
typedef struct jpghdr
{
  char sign[2];
  int len;
  char bpp;
  int width;
  int height;
  long sos;
  long eoi;
} JPGHDR;


int read_jpeg(FILE *fp,struct jpeg_decompress_struct *icinfo);
int read_marker(FILE *fp,unsigned char **l,int *mlen);
int jpeg_rst1(FILE *fpi,FILE *fpo);
int copy_jpegbytes(FILE *fpt,FILE *fpo,int first,int nrfiles,int *rstmarker);
void write_marker(FILE *fp,unsigned char mark);
int jpeg2pgm(FILE *fpi,FILE *fpo,int first,int nrfiles);
int open_jpeg(FILE *fpi,struct jpeg_decompress_struct *cinfo,
          JSAMPARRAY *buffer,  int *row_stride);
void close_jpeg(struct jpeg_decompress_struct *cinfo,JSAMPARRAY buffer);
void write_marker(FILE *fp,unsigned char mark);
int jpeg2pgm(FILE *fpi,FILE *fpo,int first,int nrfiles);


