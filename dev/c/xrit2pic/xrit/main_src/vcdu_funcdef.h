/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * Defs XRIT functions
 ********************************************************************/
#include "vcdu.h"

int read_vcasdu_hdr(FILE *fp,CVCDU_HDR *cvcdu_hdr);
int read_vca_sdu(FILE *fp,char *l);
int read_mpdu_hdr(FILE *fp);
int read_cppdu_hdr(FILE *fp,CPPDU_HDR *cppdu_hdr);
int catch_cppdu_hdr(unsigned char *l,CPPDU_HDR *cppdu_hdr);
unsigned long catch_tfh_hdr(unsigned char *l,TFH_HDR *tfh_hdr);
int read_cppdu(FILE *fp,unsigned char *l,CPPDU_HDR *cppdu_hdr);

int find_xrit(FILE *fp,unsigned char *l,int file_counter,TFH_HDR *tfh_hdr);
void list_xrit(FILE *fp,FILE *fpo);
int extract_xrit(FILE *fp,FILE *fpo,char *chan,int nr,int mkimage);





