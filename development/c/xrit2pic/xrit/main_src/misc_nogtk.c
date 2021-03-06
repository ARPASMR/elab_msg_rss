#include "xrit2pic_nogtk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

union fl_lng
{
  float f;
  guint32 l;
};


float float32_from_le(float f)
{
  union fl_lng conv;
  conv.f=f;
  conv.l=GUINT32_FROM_LE(conv.l);
  return conv.f;
}

float float32_to_le(float f)
{
  union fl_lng conv;
  conv.f=f;
  conv.l=GUINT32_TO_LE(conv.l);
  return conv.f;
}

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#define RCHAR(l) l[strlen(l)-1]

int make_dir_hier(char *dir)
{
  char *idir,*p;
  int err=0;
  if (!(idir=malloc(strlen(dir)+2))) return -1;
  strcpy(idir,dir);
  p=idir;
  do
  {
    if ((p=strchr(p,DIR_SEPARATOR))) { *p=0; }
    err=make_dir(idir);
    if (p) { *p=DIR_SEPARATOR; p++; }
    if ((p) && (!*p)) break;
  } while (p);
  free(idir);
  return err;
}

int make_dir(char *dir)
{
  #if __GTK_WIN32__ == 1
    return mkdir(dir);
  #else
    return mkdir(dir,0xfff);
  #endif
}

/*************************************
 * Construct directory path and place into BUFFER struct.
 * Relative path is translated into absolute path.
 *************************************/
#define EXTRA_MEMALLOC 5
BUFFER construct_dirpath(char *ipath)
{
  BUFFER path;
  path.size=0;

  if ((ipath) && ((*ipath=='/') || (*ipath=='\\') || (*(ipath+1)==':')))
  {                                            /* absolute path */
/* Copy absolute path.
   Extra chars allocated so there is space to add dir-separator if needed.
*/
    path.size=strlen(ipath)+EXTRA_MEMALLOC;
    path.buffer=(char *)malloc(path.size);
    strcpy(path.buffer,ipath);
  }
  else
  {
/* If relative path then construct absolute path. */
    path.buffer=(char *)getcwd(NULL,0);
/* Some OS don't support size=0; try with size!=0 */
    if (!path.buffer) path.buffer=(char *)getcwd(NULL,200);
    if (!path.buffer) return path;              /* Error! */

    path.size=strlen(path.buffer);

    if ((ipath) && (*ipath))
    {
      path.size+=(strlen(ipath)+EXTRA_MEMALLOC);
      path.buffer=(char *)realloc(path.buffer,path.size);
      sprintf(path.buffer,"%s%c%s",path.buffer,DIR_SEPARATOR,ipath);
    }
    else
    {
      path.size+=EXTRA_MEMALLOC;
      path.buffer=(char *)realloc(path.buffer,path.size);
      sprintf(path.buffer,"%s%c",path.buffer,DIR_SEPARATOR);
    }
  }
/* Add closing dir-separator if not present */
  if (RCHAR(path.buffer) != DIR_SEPARATOR)
    sprintf(path.buffer,"%s%c",path.buffer,DIR_SEPARATOR);

  return path;
}

void construct_new_path(BUFFER *new_path,BUFFER *path,char *d_name)
{
/* Allocate extra space in path buffer if needed */
  if (new_path->size < strlen(path->buffer)+strlen(d_name)+4)
  {
    new_path->size=strlen(path->buffer)+strlen(d_name)+4;
    new_path->buffer=realloc(new_path->buffer,new_path->size);
  }

/* Construct new path */
  sprintf(new_path->buffer, "%s%c%s", 
                     path->buffer,DIR_SEPARATOR,d_name); 
}

/*************************************
 * directory help functions
 *************************************/
gboolean is_a_dir(char *path)
{
  struct stat buf;
  gboolean ret=FALSE;
  #if __GTK_WIN32__ == 1
    char *ip;
    strcpyd(&ip,path);
    if (ip)
    {
      if (RCHAR(ip)==DIR_SEPARATOR) RCHAR(ip)=0;
      ret=((stat(ip, &buf) >= 0) && (S_ISDIR(buf.st_mode)));
      free(ip);
    }
  #else
    ret=((stat(path, &buf) >= 0) && (S_ISDIR(buf.st_mode)));
  #endif
  return ret;
}

/*************************************
 * remove content of dir.
 * return: amount of items deleted
 *************************************/
int remove_dircontent(char *dir,char *filter,gboolean doit)
{
  DIR *directory;
  int n=0;
  char s[1000];
  struct dirent *dirEntry;
/* opendir(): For Windows path MUST end with '.' */
  strcpy(s,dir);
  if (RCHAR(s)!=DIR_SEPARATOR) sprintf(s+strlen(s),"%c",DIR_SEPARATOR);
  if (RCHAR(s)!='.') strcat(s,".");
  directory = opendir(s);
  if (directory) while ((dirEntry = readdir(directory)))
  {
    if (RCHAR(dir)!=DIR_SEPARATOR)
      sprintf(s,"%s%c%s",dir,DIR_SEPARATOR,dirEntry->d_name);
    else
      sprintf(s,"%s%s",dir,dirEntry->d_name);
    if ((!filter) || (!strcmpwild(s,filter)))
    {
      if (doit)
      {
        if (*dirEntry->d_name != '.')
        {
          if (is_a_dir(s))
          {
            rmdir(s);
          }
          else
          {
            if (!(remove(s))) n++;
          }
        }
      }
      else n++;
    }
  }
  if (directory) closedir(directory);
  return n;
}

/*************************************
 * move content of dir
 *************************************/
int move_dircontent(char *dir,char *dirdest)
{
  DIR *directory;
  int ret=-1;
  char s[1000];
  char d[1000];
  struct dirent *dirEntry;
/* opendir(): For Windows path MUST end with '.' */
  strcpy(s,dir);
  if (RCHAR(s)!='.') strcat(s,".");
  directory = opendir(s);
  if (directory) while ((dirEntry = readdir(directory)))
  {
    sprintf(s,"%s%s",dir,dirEntry->d_name);
    sprintf(d,"%s%s",dirdest,dirEntry->d_name);
    
    if (*dirEntry->d_name != '.')
    {
      remove(d);              // needed for windhoos...
      ret&=rename(s,d);
    }
  }
  if (directory) closedir(directory);
  return ret;
}

/*************************************
 * ASCII bitmap for draw_rgbstring
 *************************************/
static char ta[][8]={ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                      {0x30,0x78,0x78,0x78,0x30,0x00,0x30,0x00},
                      {0x6c,0x6c,0x6c,0x00,0x00,0x00,0x00,0x00},
                      {0x6c,0x6c,0xfe,0x6c,0xfe,0x6c,0x6c,0x00},
                      {0x30,0x7c,0xc0,0x78,0x0c,0xf8,0x30,0x00},
                      {0x00,0xc6,0xcc,0x18,0x30,0x66,0xc6,0x00},
                      {0x38,0x6c,0x38,0x76,0xdc,0xcc,0x76,0x00},
                      {0x60,0x60,0xc0,0x00,0x00,0x00,0x00,0x00},
                      {0x18,0x30,0x60,0x60,0x60,0x30,0x18,0x00},
                      {0x60,0x30,0x18,0x18,0x18,0x30,0x60,0x00},
                      {0x00,0x66,0x3c,0xfe,0x3c,0x66,0x00,0x00},
                      {0x00,0x30,0x30,0xfc,0x30,0x30,0x00,0x00},
                      {0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60},
                      {0x00,0x00,0x00,0xfc,0x00,0x00,0x00,0x00},
                      {0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00},
                      {0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x00},
                      {0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00},
                      {0x30,0x70,0x30,0x30,0x30,0x30,0xfc,0x00},
                      {0x78,0xcc,0x0c,0x38,0x60,0xcc,0xfc,0x00},
                      {0x78,0xcc,0x0c,0x38,0x0c,0xcc,0x78,0x00}, 
                      {0x1c,0x3c,0x6c,0xcc,0xfe,0x0c,0x1e,0x00},
                      {0xfc,0xc0,0xf8,0x0c,0x0c,0xcc,0x78,0x00},
                      {0x38,0x60,0xc0,0xf8,0xcc,0xcc,0x78,0x00},
                      {0xfc,0xcc,0x0c,0x18,0x30,0x30,0x30,0x00},
                      {0x78,0xcc,0xcc,0x78,0xcc,0xcc,0x78,0x00},
                      {0x78,0xcc,0xcc,0x7c,0x0c,0x18,0x70,0x00},
                      {0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x00},
                      {0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x60},
                      {0x18,0x30,0x60,0xc0,0x60,0x30,0x18,0x00},
                      {0x00,0x00,0xfc,0x00,0x00,0xfc,0x00,0x00},
                      {0x60,0x30,0x18,0x0c,0x18,0x30,0x60,0x00},
                      {0x78,0xcc,0x0c,0x18,0x30,0x00,0x30,0x00},
                      {0x7c,0xc6,0xde,0xde,0xde,0xc0,0x78,0x00},
                      {0x38,0x6c,0xc6,0xc6,0xfe,0xc6,0xc6,0x00},
                      {0xfc,0xc6,0xc6,0xfc,0xc6,0xc6,0xfc,0x00},
                      {0x7c,0xc6,0xc6,0xc0,0xc0,0xc6,0x7c,0x00}, 
                      {0xf8,0xcc,0xc6,0xc6,0xc6,0xcc,0xf8,0x00},
                      {0xfe,0xc0,0xc0,0xfc,0xc0,0xc0,0xfe,0x00},
                      {0xfe,0xc0,0xc0,0xfc,0xc0,0xc0,0xc0,0x00},
                      {0x7c,0xc6,0xc0,0xce,0xc6,0xc6,0x7e,0x00},
                      {0xc6,0xc6,0xc6,0xfe,0xc6,0xc6,0xc6,0x00},
                      {0x78,0x30,0x30,0x30,0x30,0x30,0x78,0x00},
                      {0x1e,0x06,0x06,0x06,0xc6,0xc6,0x7c,0x00},
                      {0xc6,0xcc,0xd8,0xf0,0xd8,0xcc,0xc6,0x00},
                      {0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xfe,0x00},
                      {0xc6,0xee,0xfe,0xd6,0xc6,0xc6,0xc6,0x00},
                      {0xc6,0xe6,0xf6,0xde,0xce,0xc6,0xc6,0x00},
                      {0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00},
                      {0xfc,0xc6,0xc6,0xfc,0xc0,0xc0,0xc0,0x00},
                      {0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x06},
                      {0xfc,0xc6,0xc6,0xfc,0xc6,0xc6,0xc6,0x00},
                      {0x78,0xcc,0x60,0x30,0x18,0xcc,0x78,0x00},
                      {0xfc,0x30,0x30,0x30,0x30,0x30,0x30,0x00},
                      {0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00},
                      {0xc6,0xc6,0xc6,0xc6,0xc6,0x6c,0x38,0x00},
                      {0xc6,0xc6,0xc6,0xd6,0xfe,0xee,0xc6,0x00}, 
                      {0xc6,0xc6,0x6c,0x38,0x6c,0xc6,0xc6,0x00},
                      {0xc2,0xc2,0x66,0x3c,0x18,0x18,0x18,0x00},
                      {0xfe,0x0c,0x18,0x30,0x60,0xc0,0xfe,0x00},
                      {0x3c,0x30,0x30,0x30,0x30,0x30,0x3c,0x00},
                      {0xc0,0x60,0x30,0x18,0x0c,0x06,0x02,0x00},
                      {0x3c,0x0c,0x0c,0x0c,0x0c,0x0c,0x3c,0x00},
                      {0x00,0x38,0x6c,0xc6,0x00,0x00,0x00,0x00},
                      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe},
                      {0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00},
                      {0x00,0x00,0x7c,0x06,0x7e,0xc6,0x7e,0x00},
                      {0xc0,0xc0,0xfc,0xc6,0xc6,0xe6,0xdc,0x00},
                      {0x00,0x00,0x7c,0xc6,0xc0,0xc0,0x7e,0x00},
                      {0x06,0x06,0x7e,0xc6,0xc6,0xce,0x76,0x00},
                      {0x00,0x00,0x7c,0xc6,0xfe,0xc0,0x7e,0x00},
                      {0x1e,0x30,0x7c,0x30,0x30,0x30,0x30,0x00},
                      {0x00,0x00,0x7e,0xc6,0xce,0x76,0x06,0x7c},
                      {0xc0,0xc0,0xfc,0xc6,0xc6,0xc6,0xc6,0x00},
                      {0x18,0x00,0x38,0x18,0x18,0x18,0x3c,0x00},
                      {0x18,0x00,0x38,0x18,0x18,0x18,0x18,0xf0},
                      {0xc0,0xc0,0xcc,0xd8,0xf0,0xd8,0xcc,0x00},
                      {0x38,0x18,0x18,0x18,0x18,0x18,0x3c,0x00},
                      {0x00,0x00,0xcc,0xfe,0xd6,0xc6,0xc6,0x00},
                      {0x00,0x00,0xfc,0xc6,0xc6,0xc6,0xc6,0x00},
                      {0x00,0x00,0x7c,0xc6,0xc6,0xc6,0x7c,0x00},
                      {0x00,0x00,0xfc,0xc6,0xc6,0xe6,0xdc,0xc0},
                      {0x00,0x00,0x7e,0xc6,0xc6,0xce,0x76,0x06},
                      {0x00,0x00,0x6e,0x70,0x60,0x60,0x60,0x00},
                      {0x00,0x00,0x7c,0xc0,0x7c,0x06,0xfc,0x00},
                      {0x30,0x30,0x7c,0x30,0x30,0x30,0x1c,0x00},
                      {0x00,0x00,0xc6,0xc6,0xc6,0xc6,0x7e,0x00},
                      {0x00,0x00,0xc6,0xc6,0xc6,0x6c,0x38,0x00},
                      {0x00,0x00,0xc6,0xc6,0xd6,0xfe,0x6c,0x00},
                      {0x00,0x00,0xc6,0x6c,0x38,0x6c,0xc6,0x00},
                      {0x00,0x00,0xc6,0xc6,0xce,0x76,0x06,0x7c},
                      {0x00,0x00,0xfc,0x18,0x30,0x60,0xfc,0x00},
                      {0x1c,0x30,0x30,0xe0,0x30,0x30,0x1c,0x00},
                      {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},
                      {0xe0,0x30,0x30,0x1c,0x30,0x30,0xe0,0x00},
                      {0x76,0xdc,0x00,0x00,0x00,0x00,0x00,0x00}
                    };
void get_strline(char *s,int y,char *l,int slen)
{
  int m,x,n;
  *l=0;
  for (m=0; *s; s++,m++)
  {
    n=(*s)-' '; if (n>95) return;
    if (m*8+8>slen) return;
    for (x=0; x<8; x++)
    {
      if (ta[n][y]&(0x80>>x)) l[x+m*8]=0xff; else l[x+m*8]=0;
    }
  }
}

char *g_get_current_dir()
{
  char *r=NULL;
  r=malloc(10);
  strcpy(r,".");
  return r;
}

char *g_get_home_dir()
{
  char *p,*r=NULL;
  p=getenv("HOME");
  if (p)
  {
    r=malloc(strlen(p)+10);
    strcpy(r,p);
  }
  
  return r;
}

char *g_getenv(char *p)
{
  char *r=NULL;
  p=getenv(p);
  if (p)
  {
    r=malloc(strlen(p)+10);
    strcpy(r,p);
  }
  return r;
}
char *g_dirname()
{
  char *r=NULL;
  r=malloc(10);
  strcpy(r,".");
  return r;
}
char *g_basename(char *s)
{
  char *p;
  if ((p=strrchr(s,DIR_SEPARATOR))) return p+1;
  return s;
}



/*************************************
 * Get location of program.
 * Takes care of soft-links (Unix).
 *************************************/
#define BUFFLEN 1024
#define DEFPATH "."
char *get_path(char *iprogname)
{
  char *p,
       *pp,
       *path,
       *prog,
       *totname,
        buffer[BUFFLEN];

  FILE  *fp;
  int status;
  int len_totname;
  
  prog=(char *)g_basename(iprogname);

  if (strrchr(iprogname,DIR_SEPARATOR)) 
  {                               /* path name in progname */
    path=g_dirname(iprogname); 
  }
  else
  {                               /* get path from env. variable */
    if (!(p=(char *)g_getenv("PATH"))) 
    {
      p = DEFPATH;
    }
    path = malloc(strlen(p) + 10);
    if (p[0] == PATH_SEPARATOR)
    {
      sprintf(path, ".%s", p);
    }
    else if ((pp = strstr(p, "::")))
    {
      *pp = 0;
      sprintf(path, "%s%c.%s", p, PATH_SEPARATOR,pp + 1);
    }
    else if (p[strlen(p) - 1] == PATH_SEPARATOR)
    {
      sprintf(path, "%s.", p);
    }
    else
    {
      strcpy(path, p);
    }
    #if __GTK_WIN32__ == 1
      strcat(path,";"); strcat(path,".");
    #endif
  }
  len_totname=strlen(path) + strlen(prog) + 15;
  totname = malloc(len_totname);

  p = strtok(path, PATH_SEPARATOR_STR);
  do
  {
    #if __GTK_WIN32__ == 1
      if ((strlen(prog) > 4) && (!strcmp(prog+strlen(prog)-4,".exe")))
        sprintf(totname, "%s%c%s", p, DIR_SEPARATOR, prog);
      else
        sprintf(totname, "%s%c%s.exe", p, DIR_SEPARATOR, prog);
    #else
      sprintf(totname, "%s%c%s", p, DIR_SEPARATOR, prog);
    #endif
    if ((fp = fopen(totname, "r")))
    {
      fclose(fp);
      break;
    }
  } while ((p = strtok(NULL, PATH_SEPARATOR_STR)));

  if (p)
  {
    #if __GTK_WIN32__ == 0
      status = readlink(totname, buffer, BUFFLEN);
      while (status >= 0)
      {
        buffer[status] = '\0';
        if (strlen(buffer) > len_totname)
        {
          len_totname=strlen(buffer)+10;
          totname=realloc(totname,len_totname);
        }
        strcpy(totname, buffer);
        status = readlink(totname, buffer, BUFFLEN);
      }
    #endif
    free(path);
    path=g_dirname(totname); 
  }
  else
  {
    free(path);
    path = NULL;
  }
  free(totname);
  return path;
}

