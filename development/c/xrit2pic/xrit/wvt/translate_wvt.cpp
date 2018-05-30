/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
// Interface C+ Wavelet code with C main code.
// See 3 last functions.
#define GENERATE_QFILE 0

using namespace std;
#include <fstream>
#include <string>
#include "CWTDecoder.h"

#include "glib.h"

/*************************************************
 * Open, close and read wavelet file
 * Not to be used directly; see 3 last functions:
 *   wvt_open, wvt_read, wvt_close
 * Return values:
 *   'b': busy (file already opened)
 *   'i': can't open file
 *   'g': can't allocate buffer
 *   'r': read error
 *   't': translate error
 *    0 : OK
 *************************************************/
int cpp_do_wvt(char *fni,guint16 *W,guint16 *H,guint16 *D,
                                 guint16 **obuf,
                                 guint16 *chunck_height,
                                 unsigned char *chunck_bpp,
                                 char act)
{
  //Open actual compressed file
  static std::ifstream ifile;
  static bool in_process;

  switch(act)
  {
    case 'o':
    {
      if (in_process) return 'b';
      std::string sifile (fni);

      ifile.open(sifile.c_str(), std::ios::in|std::ios::binary);
      if (!ifile.good()) return 'i';
      in_process=TRUE;

      // Get size info of total pic
      ifile.read((char *)W,2);
      *W=GUINT16_FROM_BE(*W);

      ifile.read((char *)H,2);
      *H=GUINT16_FROM_BE(*H);

      ifile.read((char *)D,2);
      *D=GUINT16_FROM_BE(*D);

      ifile.seekg(2,ios::cur);

    }
    break;
    case 'c':
      ifile.close();
      in_process=FALSE;
    break;
    case 'r':
    {
      std::vector <short> QualityInfo;
      unsigned long nb_ibytes;
      unsigned long nb_owords;

      guint16 ch_h,ch_d;
      guint32 nb_b;

      #if GENERATE_QFILE == 1
        std::string sofile ("Qfile");
        sofile += ".q";
        std::ofstream ofile (sofile.c_str(), std::ios::out);
        Assert (ofile.good(), Util::CNTException ());
        ofile << H << std::endl;
        Assert (ofile.good(), Util::CNTException ());
        ofile << W << std::endl;
        Assert (ofile.good(), Util::CNTException ());
      #endif

      // Get size info from file
      ifile.read((char *)&ch_d,2);
      *chunck_bpp=GUINT16_FROM_BE(ch_d);

      ifile.read((char *)&ch_h,2);
      *chunck_height=GUINT16_FROM_BE(ch_h);

      ifile.read((char *)&nb_b,4);
      nb_ibytes=GUINT32_FROM_BE(nb_b);

      nb_owords=(*chunck_height)*(*W);

      *obuf=(guint16 *)malloc((nb_owords)*sizeof(guint16));
      if (!(*obuf)) return 'a';
      
      // Now actual wvt format follows
      std::auto_ptr< unsigned char > ibuf( new unsigned char[nb_ibytes]);
      if (!(ibuf.get())) return 'g';

      ifile.read( (char *)ibuf.get(), nb_ibytes);
      if (!ifile.good()) return 'r';
      Util::CDataFieldCompressedImage img_compressed(ibuf.release(),
                                                     nb_ibytes*8,
                                                     *chunck_bpp,
                                                     *W,
                                                     *chunck_height);

      Util::CDataFieldUncompressedImage img_uncompressed;

      //****************************************************
      //*** Here comes the wavelets decompression routine
      try
      { 
        COMP::DecompressWT(img_compressed,*chunck_bpp,img_uncompressed,QualityInfo);
      }
      catch(...)
      {
        return 't';
      }

      //****************************************************

      {
        COMP::CImage cimg (img_uncompressed);
        unsigned char *p=img_uncompressed.get();
        for( unsigned long i=0; i < nb_owords; i+=1)
        {
          guint16 tmp;
          tmp=cimg.Get()[i];
          (*obuf)[i] = tmp;
        }
      }

      #if GENERATE_QFILE == 1
        // now, write the qualityinfo array...
        for (unsigned short i=0 ; i<img_uncompressed.GetNL() ; i++)
        {
          ofile << i << "  " << QualityInfo[i] << std::endl;
          Assert (ofile.good(), Util::CNTException());
        }
      #endif

    }

    break;
  }

  return 0;
}

/************************************
 * Interface functions
 ************************************/

/************************************
 * Open wavelet file. Note: Just open 1 file at a time!
 * Return values:
 *   'b': busy (file already opened)
 *   'i': can't open file
 *    0 : OK
 ************************************/
extern "C" int wvt_open(char *fni,          /* input file name */
                        guint16 *W,         /* returns width segment */
                        guint16 *H,         /* returns height segment */
                        guint16 *D)         /* returns depth (bits/pixel) */
{
  return cpp_do_wvt(fni,W,H,D,NULL,NULL,NULL,'o');
}

/************************************
 * Close wavelet file
 ************************************/
extern "C" void wvt_close()
{
  cpp_do_wvt(NULL,NULL,NULL,NULL,NULL,NULL,NULL,'c');
}

/************************************
 * Read wavelet segment and place result in str
 * Return values:
 *   'g': can't allocate buffer
 *   'r': read error
 *   't': translate error
 ************************************/
extern "C" int wvt_read(guint16 W,            /* width */
                        guint16 **str,        /* returned string=with-pixelvalues */
                        guint16 *height,      /* returned height */
                        unsigned char *bpp)   /* returned bts per pixel */
{
  return cpp_do_wvt(NULL,&W,NULL,NULL,str,height,bpp,'r');
}

