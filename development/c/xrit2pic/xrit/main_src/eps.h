typedef struct class1hdr
{
  int orbit_start;
  int epoch_year;
  float epoch_day;
  float eccentricity;
  float inclination;
  float perigee;
  float raan;
  float anomaly;

  int total_mdr;
  int duration_of_product;
  POINT subsat_start;
  POINT subsat_end;
} CLASS1HDR;

typedef struct class2hdr
{
  int earth_views_per_scanline;
  int nav_sample_rate;
} CLASS2HDR;

typedef struct class8hdr
{
  int earth_views_per_scanline;
  int nav_sample_rate;
} CLASS8HDR;

typedef struct hdr
{
  struct class1hdr cls1;
  struct class2hdr cls2;
  struct class8hdr cls8;
  int size;
} HDR;
