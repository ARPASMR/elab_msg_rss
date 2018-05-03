
typedef struct gshhs
{ /* Global Self-consistent Hierarchical High-resolution Shorelines */
  gint32 id;			   /* Unique polygon id number, starting at 0 */
  gint32 n;			   /* Number of points in this polygon */
  gint32 level;			   /* 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake */
  gint32 west, east, south, north; /* min/max extent in micro-degrees */
  gint32 area;			   /* Area of polygon in 1/10 km^2 */
  gint32 version;                  /* Version of GSHHS polygon (3 is latest and first with this item) */
  gint16 greenwich;                /* Greenwich is 1 if Greenwich is crossed */
  gint16 source;                   /* 0 = CIA WDBII, 1 = WVS */
} GSHHS;

typedef struct	pointx 
{ /* Each lon, lat pair is stored in micro-degrees in 4-byte integer format */
  gint32 x;
  gint32 y;
} POINTx;

