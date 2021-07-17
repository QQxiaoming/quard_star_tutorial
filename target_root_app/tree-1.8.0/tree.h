/* $Copyright: $
 * Copyright (c) 1996 - 2018 by Steve Baker (ice@mama.indstate.edu)
 * All Rights reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#ifdef __EMX__  /* for OS/2 systems */
#  define INCL_DOSFILEMGR
#  define INCL_DOSNLS
#  include <os2.h>
#  include <sys/nls.h>
#  include <io.h>
  /* On many systems stat() function is idential to lstat() function.
   * But the OS/2 does not support symbolic links and doesn't have lstat() function.
   */
#  define         lstat          stat
#  define         strcasecmp     stricmp
  /* Following two functions, getcwd() and chdir() don't support for drive letters.
   * To implement support them, use _getcwd2() and _chdir2().
   */
#  define getcwd _getcwd2
#  define chdir _chdir2
#endif

#include <locale.h>
#include <langinfo.h>
#include <wchar.h>
#include <wctype.h>

#ifdef __ANDROID
#define mbstowcs(w,m,x) mbsrtowcs(w,(const char**)(& #m),x,NULL)
#endif

/* Should probably use strdup(), but we like our xmalloc() */
#define scopy(x)	strcpy(xmalloc(strlen(x)+1),(x))
#define MINIT		30	/* number of dir entries to initially allocate */
#define MINC		20	/* allocation increment */

#ifndef TRUE
typedef enum {FALSE=0, TRUE} bool;
#else
typedef int bool;
#endif

struct _info {
  char *name;
  char *lnk;
  bool isdir;
  bool issok;
  bool isfifo;
  bool isexe;
  bool orphan;
  mode_t mode, lnkmode;
  uid_t uid;
  gid_t gid;
  off_t size;
  time_t atime, ctime, mtime;
  dev_t dev, ldev;
  ino_t inode, linode;
  #ifdef __EMX__
  long attr;
  #endif
  char *err;
  struct _info **child, *next, *tchild;
};
/* hash.c */
struct xtable {
  unsigned int xid;
  char *name;
  struct xtable *nxt;
};
struct inotable {
  ino_t inode;
  dev_t device;
  struct inotable *nxt;
};

/* color.c */
struct colortable {
  char *term_flg, *CSS_name, *font_fg, *font_bg;
};
struct extensions {
  char *ext;
  char *term_flg, *CSS_name, *web_fg, *web_bg, *web_extattr;
  struct extensions *nxt;
};
struct linedraw {
  const char **name, *vert, *vert_left, *corner, *copy;
};

/* Function prototypes: */
/* tree.c */
void usage(int);
struct _info **unix_getfulltree(char *d, u_long lev, dev_t dev, off_t *size, char **err);
struct _info **read_dir(char *, int *);

int alnumsort(struct _info **, struct _info **);
int versort(struct _info **a, struct _info **b);
int reversealnumsort(struct _info **, struct _info **);
int mtimesort(struct _info **, struct _info **);
int ctimesort(struct _info **, struct _info **);
int sizecmp(off_t a, off_t b);
int fsizesort(struct _info **a, struct _info **b);

void *xmalloc(size_t), *xrealloc(void *, size_t);
char *gnu_getcwd();
int patmatch(char *, char *);
void indent(int maxlevel);
void free_dir(struct _info **);
#ifdef __EMX__
char *prot(long);
#else
char *prot(mode_t);
#endif
char *do_date(time_t);
void printit(char *);
int psize(char *buf, off_t size);
char Ftype(mode_t mode);
char *fillinfo(char *buf, struct _info *ent);

/* unix.c */
off_t unix_listdir(char *d, int *dt, int *ft, u_long lev, dev_t dev);
off_t unix_rlistdir(char *d, int *dt, int *ft, u_long lev, dev_t dev);
void r_listdir(struct _info **dir, char *d, int *dt, int *ft, u_long lev);

/* html.c */
void emit_html_header(const char *charset, char *title, char *version);
off_t html_listdir(char *d, int *dt, int *ft, u_long lev, dev_t dev);
off_t html_rlistdir(char *d, int *dt, int *ft, u_long lev, dev_t dev);
void htmlr_listdir(struct _info **dir, char *d, int *dt, int *ft, u_long lev);
void html_encode(FILE *fd, char *s);

/* xml.c */
off_t xml_listdir(char *d, int *dt, int *ft, u_long lev, dev_t dev);
off_t xml_rlistdir(char *d, int *dt, int *ft, u_long lev, dev_t dev);
void xmlr_listdir(struct _info **dir, char *d, int *dt, int *ft, u_long lev);
void xml_indent(int maxlevel);
void xml_fillinfo(struct _info *ent);

/* json.c */
off_t json_listdir(char *d, int *dt, int *ft, u_long lev, dev_t dev);
off_t json_rlistdir(char *d, int *dt, int *ft, u_long lev, dev_t dev);
void jsonr_listdir(struct _info **dir, char *d, int *dt, int *ft, u_long lev);
void json_indent(int maxlevel);
void json_fillinfo(struct _info *ent);

/* color.c */
void parse_dir_colors();
int color(u_short mode, char *name, bool orphan, bool islink);
const char *getcharset(void);
void initlinedraw(int);

/* hash.c */
char *uidtoname(uid_t uid);
char *gidtoname(gid_t gid);
int findino(ino_t, dev_t);
void saveino(ino_t, dev_t);

/* file.c */
struct _info **file_getfulltree(char *d, u_long lev, dev_t dev, off_t *size, char **err);

/* We use the strverscmp.c file if we're not linux */
#if ! defined (LINUX)
int strverscmp (const char *s1, const char *s2);
#endif
