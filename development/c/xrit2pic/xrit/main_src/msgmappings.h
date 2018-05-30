/*
RGB 10-09,09-07,09:	Dust, Clouds (thickness, phase), Contrails	Day & Night
			Fog, Ash,  SO2, Low-level Humidity
RGB 05-06,08-09,05	Severe Cyclones, Jets, PV Analysis		Day & Night

RGB 10-09,09-04,09:	Clouds, Fog, Contrails, Fires			Night
RGB 02,04r,09:		Clouds, Convection, Snow, Fog, Fires		Day


RGB 05-06,04-09,03-01:	Severe Convection				Day
RGB 02,03,04r:		Snow, Fog					Day
RGB 03,02,01:		Vegetation, Snow, Smoke, Dust, Fog		Day
*/

// airm: red   =WV_062-WV_073, range -25 .. 0 K
//       green =IR_097-IR_108, range -40 .. +5 K
//       blue  =WV_062       , range 243..208 K
// 5-6,8-9,5
MSGCMAP msgmap_airmass=
{
  "Airmass",
  "airmass",
  TRUE,
  {
    1.,
     "WV_062","WV_073" ,   // channels red
     -25,0                 // temp. range
  },
  {
    1.,
    "IR_097","IR_108",     // channels green
     -40,5                 // temp. range
  },
  {
    1.,
    "WV_062",""  ,         // channels blue
    243,208                // temp. range
  }
};

// Dust: red   =IR_120-IR_108, range -4 .. +2 K
//       green =IR_108-IR_087, range  0 .. +15 K  gamma=2.5
//       blue  =IR_108       , range 261..289 K
// 10-9,9-7,9
MSGCMAP msgmap_dust=
{
  "Dust",
  "dust",
  TRUE,
  {
    1.,
     "IR_120","IR_108" ,   // channels red
     -4,2                  // temp. range
  },
  {
    2.5,
    "IR_108","IR_087",     // channels green
     0,15                  // temp. range
  },
  {
    1.,
    "IR_108",""  ,         // channels blue
    261,289                // temp. range
  }
};


// Night fog: red   =IR_120-IR_108, range -4 .. +2 K
//            green =IR_108-IR_039, range  0 .. +10 K
//            blue  =IR_108       , range 243..293 K
// 10-9,9-4,9
MSGCMAP msgmap_nfog=
{
  "Night fog",
  "nightfog",
  TRUE,
  {
    1.,
     "IR_120","IR_108" ,   // channels red
     -4,2                  // temp. range
  },
  {
    1.,
    "IR_108","IR_039",     // channels green
     0,10                  // temp. range
  },
  {
    1.,
    "IR_108",""  ,         // channels blue
    243,293                // temp. range
  }
};


/* From:
Combined and parallel use of MSG composite images and
SAFNWC/MSG products at the Hungarian Meteorological Service
Mária Putsay, Kornél Kolláth and Ildikó Szenyán
Hungarian Meteorological Service
H-1525 Budapest, P. O. Box 38, Hungary

  pdf_conf_p48_s2a_14_putsay_p.pdf)
*/
// Day fog: red   =VIS006
//          green =IR_039 range 200 .. 300
//          blue  =IR_108 range 200 .. 300
// 1,4,9
MSGCMAP msgmap_dfog=
{
  "Day fog",
  "dayfog",
  TRUE,
  {
    1.,
     "VIS006","" ,         // channels red
     0,255                 // temp. range
  },
  {
    1.,
    "IR_039","",           // channels green
     200,300                  // temp. range
  },
  {
    1.,
    "IR_108",""  ,         // channels blue
    200,300                // temp. range
  }
};

// 24h cloud mircophysics
MSGCMAP msgmap_cloud_micro=
{
  "Micro-ph. Cloud",
  "muphcloud",
  TRUE,
  {
    1.,
     "IR_120","IR_108" ,   // channels red
     -4,2                  // temp. range
  },
  {
    1.2,
    "IR_108","IR_087",     // channels green
     0,6                   // temp. range
  },
  {
    1.,
    "IR_108",""  ,         // channels blue
    248,303                // temp. range
  }
};

// 24h dust mircophysics
MSGCMAP msgmap_dust_micro=
{
  "Micro-ph. Dust",
  "muphdust",
  TRUE,
  {
    1.,
     "IR_120","IR_108" ,   // channels red
     -4,2                  // temp. range
  },
  {
    2.5,
    "IR_108","IR_087",     // channels green
     0,15                  // temp. range
  },
  {
    1.,
    "IR_108",""  ,         // channels blue
    261,289                // temp. range
  }
};

// 24h ash mircophysics
MSGCMAP msgmap_ash_micro=
{
  "Micro-ph. Ash",
  "muphash",
  TRUE,
  {
    1.,
     "IR_120","IR_108" ,   // channels red
     -4,2                  // temp. range
  },
  {
    1.,
    "IR_108","IR_087",     // channels green
     -4,5                   // temp. range
  },
  {
    1.,
    "IR_108",""  ,         // channels blue
    243,303                // temp. range
  }
};

// night mircophysics
MSGCMAP msgmap_night_micro=
{
  "Micro-ph. Night",
  "muphnight",
  TRUE,
  {
    1.,
     "IR_120","IR_108" ,   // channels red
     -4,2                  // temp. range
  },
  {
    1.,
    "IR_108","IR_087",     // channels green
     0,10                   // temp. range
  },
  {
    1.,
    "IR_108",""  ,         // channels blue
    243,293                // temp. range
  }
};

// day mircophysics
MSGCMAP msgmap_day_micro=
{
  "Micro-ph. Day",
  "muphday",
  TRUE,
  {
    1.,
     "VIS008","" ,         // channels red
     0,255                 // temp. range
  },
  {
    1.,
    "IR_039","",            // channels green
     0,153                  // temp. range
  },
  {
    1.,
    "IR_108",""  ,         // channels blue
    203,323                // temp. range
  }
};

// Day convective storms
MSGCMAP msgmap_conv_storms=
{
  "Convection storms",
  "convstorm",
  TRUE,
  {
    1.,
     "WV_062","WV_073" ,   // channels red
     -35,5                 // temp. range
  },
  {
    0.5,
    "IR_039","IR_108",     // channels green
     -5,60                 // temp. range
  },
  {
    1.,
    "IR_016","VIS006",     // channels green
     -192,64               // temp. range
  }
};

// Day snow-fog
// NOTE: IR_039 should be:
// max=30%, solar part only.
// See "Best practices for RGB compositing of multi-spectral imagery":
// "
// The solar part of SEVIRI IR3.9 channel may be obtained by deducing the 
// thermal contribution to the radiance from the
// SEVIRI IR10.8 channel and applying the CO2 correction mentioned in note 1. 
// For clouds that are semi-transparent in the
// SEVIRI IR3.9 channel the extraction of the solar part is not valid.
// "
// Note 1 says:
// "
// The SEVIRI IR3.9 channel is influenced by CO2 absorption (correction 
// to be applied may be estimated from the
// brightness temperature differences of the SEVIRI IR10.8 and IR13.4 channels). 
// Comparable channels on MODIS and
// AVHRR are less influenced by CO2 absorption and thus the MIN/MAX range should 
// be shifted by +4 K.
// "

MSGCMAP msgmap_snowfog=
{
  "Snow & Fog",
  "snowfog",
  TRUE,
  {
    1.,
     "VIS008","" ,         // channels red
     0,255                 // temp. range
  },
  {
    1.7,
    "IR_016","",           // channels green
     0,180                 // temp. range
  },
  {
    1.7,
    "IR_039","",          // channels green
     0,300                // temp. range !! best_practices.pdf:
  }
};
