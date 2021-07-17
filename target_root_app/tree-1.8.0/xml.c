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
#include "tree.h"

extern bool dflag, lflag, pflag, sflag, Fflag, aflag, fflag, uflag, gflag;
extern bool Dflag, inodeflag, devflag, Rflag, cflag;
extern bool noindent, force_color, xdev, nolinks, flimit;

extern struct _info **(*getfulltree)(char *d, u_long lev, dev_t dev, off_t *size, char **err);
extern const int ifmt[];
extern const char fmt[], *ftype[];

extern void (*listdir)(char *, int *, int *, u_long, dev_t);
extern int (*cmpfunc)();
extern FILE *outfile;
extern int Level, *dirs, maxdirs;

extern char *endcode;

/*
<tree>
  <directory name="name" mode=0777 size=### user="user" group="group" inode=### dev=### time="00:00 00-00-0000">
    <link name="name" target="name" ...>
      ... if link is followed, otherwise this is empty.
    </link>
    <file name="name" mode=0777 size=### user="user" group="group" inode=### dev=### time="00:00 00-00-0000"></file>
    <socket name="" ...><error>some error</error></socket>
    <block name="" ...></block>
    <char name="" ...></char>
    <fifo name="" ...></fifo>
    <door name="" ...></door>
    <port name="" ...></port>
    ...
  </directory>
  <report>
    <size>#</size>
    <files>#</files>
    <directories>#</directories>
  </report>
</tree>
*/


off_t xml_listdir(char *d, int *dt, int *ft, u_long lev, dev_t dev)
{
  char *path;
  bool nlf = FALSE;
  long pathsize = 0;
  struct _info **dir, **sav;
  struct stat sb;
  int t, n, mt;

  if ((Level >= 0) && (lev > Level)) {
    if (!noindent) fputc('\n',outfile);
    return 0;
  }

  if (xdev && lev == 0) {
    stat(d,&sb);
    dev = sb.st_dev;
  }

  sav = dir = read_dir(d,&n);
  if (!dir && n) {
    fprintf(outfile,"<error>opening dir</error>\n");
    return 0;
  }
  if (!n) {
    if (!noindent) fputc('\n', outfile);
    free_dir(sav);
    return 0;
  }
  if (flimit > 0 && n > flimit) {
    fprintf(outfile,"<error>%d entries exceeds filelimit, not opening dir</error>%s",n,noindent?"":"\n");
    free_dir(sav);
    return 0;
  }

  if (cmpfunc) qsort(dir,n,sizeof(struct _info *), cmpfunc);
  if (lev >= maxdirs-1) {
    dirs = xrealloc(dirs,sizeof(int) * (maxdirs += 1024));
    memset(dirs+(maxdirs-1024), 0, sizeof(int) * 1024);
  }
  dirs[lev] = 1;
  if (!*(dir+1)) dirs[lev] = 2;
  if (!noindent) fprintf(outfile,"\n");

  path = malloc(pathsize=4096);

  while(*dir) {
    if (!noindent) xml_indent(lev);

    if ((*dir)->lnk) mt = (*dir)->mode & S_IFMT;
    else mt = (*dir)->mode & S_IFMT;
    for(t=0;ifmt[t];t++)
      if (ifmt[t] == mt) break;
    fprintf(outfile,"<%s", ftype[t]);

    if (fflag) {
      if (sizeof(char) * (strlen(d)+strlen((*dir)->name)+2) > pathsize)
	path=xrealloc(path,pathsize=(sizeof(char) * (strlen(d)+strlen((*dir)->name)+1024)));
      if (!strcmp(d,"/")) sprintf(path,"%s%s",d,(*dir)->name);
      else sprintf(path,"%s/%s",d,(*dir)->name);
    } else {
      if (sizeof(char) * (strlen((*dir)->name)+1) > pathsize)
	path=xrealloc(path,pathsize=(sizeof(char) * (strlen((*dir)->name)+1024)));
      sprintf(path,"%s",(*dir)->name);
    }

    fprintf(outfile, " name=\"");
    html_encode(outfile,path);
    fputc('"',outfile);

    if ((*dir)->lnk) {
      fprintf(outfile, " target=\"");
      html_encode(outfile,(*dir)->lnk);
      fputc('"',outfile);
    }
    xml_fillinfo(*dir);
    fputc('>',outfile);

    if ((*dir)->isdir) {
      if ((*dir)->lnk) {
	if (lflag && !(xdev && dev != (*dir)->dev)) {
	  if (findino((*dir)->inode,(*dir)->dev)) {
	    fprintf(outfile,"<error>recursive, not followed</error>");
	  } else {
	    saveino((*dir)->inode, (*dir)->dev);
	    if (*(*dir)->lnk == '/')
	      listdir((*dir)->lnk,dt,ft,lev+1,dev);
	    else {
	      if (strlen(d)+strlen((*dir)->lnk)+2 > pathsize) path=xrealloc(path,pathsize=(strlen(d)+strlen((*dir)->name)+1024));
	      if (fflag && !strcmp(d,"/")) sprintf(path,"%s%s",d,(*dir)->lnk);
	      else sprintf(path,"%s/%s",d,(*dir)->lnk);
	      listdir(path,dt,ft,lev+1,dev);
	    }
	    nlf = TRUE;
	  }
	}
      } else if (!(xdev && dev != (*dir)->dev)) {
	if (strlen(d)+strlen((*dir)->name)+2 > pathsize) path=xrealloc(path,pathsize=(strlen(d)+strlen((*dir)->name)+1024));
	if (fflag && !strcmp(d,"/")) sprintf(path,"%s%s",d,(*dir)->name);
	else sprintf(path,"%s/%s",d,(*dir)->name);
	saveino((*dir)->inode, (*dir)->dev);
	listdir(path,dt,ft,lev+1,dev);
	nlf = TRUE;
      }
      *dt += 1;
    } else *ft += 1;
    if (*(dir+1) && !*(dir+2)) dirs[lev] = 2;
    if (nlf) {
      nlf = FALSE;
      if (!noindent) xml_indent(lev);
    }
    fprintf(outfile,"</%s>%s",ftype[t],noindent?"":"\n");
    dir++;
  }
  dirs[lev] = 0;
  free(path);
  free_dir(sav);
  return 0;
}

off_t xml_rlistdir(char *d, int *dt, int *ft, u_long lev, dev_t dev)
{
  struct _info **dir;
  off_t size = 0;
  char *err;
  
  dir = getfulltree(d, lev, dev, &size, &err);
  
  memset(dirs, 0, sizeof(int) * maxdirs);
  
  xmlr_listdir(dir, d, dt, ft, lev);
  
  return size;
}

void xmlr_listdir(struct _info **dir, char *d, int *dt, int *ft, u_long lev)
{
  char *path;
  long pathsize = 0;
  struct _info **sav = dir;
  bool nlf = FALSE;
  int mt, t;
  
  if (dir == NULL) return;
  
  dirs[lev] = 1;
  if (!*(dir+1)) dirs[lev] = 2;
  fprintf(outfile,"\n");
  
  path = malloc(pathsize=4096);
  
  while(*dir) {
    if (!noindent) xml_indent(lev);

    if ((*dir)->lnk) mt = (*dir)->mode & S_IFMT;
    else mt = (*dir)->mode & S_IFMT;
    for(t=0;ifmt[t];t++)
      if (ifmt[t] == mt) break;
    fprintf(outfile,"<%s", ftype[t]);

    if (fflag) {
      if (sizeof(char) * (strlen(d)+strlen((*dir)->name)+2) > pathsize)
	path=xrealloc(path,pathsize=(sizeof(char) * (strlen(d)+strlen((*dir)->name)+1024)));
      if (!strcmp(d,"/")) sprintf(path,"%s%s",d,(*dir)->name);
      else sprintf(path,"%s/%s",d,(*dir)->name);
    } else {
      if (sizeof(char) * (strlen((*dir)->name)+1) > pathsize)
	path=xrealloc(path,pathsize=(sizeof(char) * (strlen((*dir)->name)+1024)));
      sprintf(path,"%s",(*dir)->name);
    }

    fprintf(outfile, " name=\"");
    html_encode(outfile,path);
    fputc('"',outfile);

    if ((*dir)->lnk) {
      fprintf(outfile, " target=\"");
      html_encode(outfile,(*dir)->lnk);
      fputc('"',outfile);
    }

    xml_fillinfo(*dir);
    if (mt != S_IFDIR && mt != S_IFLNK && (*dir)->err == NULL) fprintf(outfile,"/>");
    else fputc('>',outfile);

    if ((*dir)->err) {
      fprintf(outfile,"<error>%s</error>", (*dir)->err);
      free((*dir)->err);
      (*dir)->err = NULL;
    }
    if ((*dir)->child) {
      if (fflag) {
	if (strlen(d)+strlen((*dir)->name)+2 > pathsize) path=xrealloc(path,pathsize=(strlen(d)+strlen((*dir)->name)+1024));
	if (!strcmp(d,"/")) sprintf(path,"%s%s",d,(*dir)->name);
	else sprintf(path,"%s/%s",d,(*dir)->name);
      }
      xmlr_listdir((*dir)->child, fflag? path : NULL, dt, ft, lev+1);
      nlf = TRUE;
      *dt += 1;
    } else {
      if ((*dir)->isdir) *dt += 1;
      else *ft += 1;
    }
    
    if (*(dir+1) && !*(dir+2)) dirs[lev] = 2;
    if (nlf) {
      nlf = FALSE;
      if (!noindent) xml_indent(lev);
    }
    if (mt == S_IFDIR || mt == S_IFLNK || (*dir)->err != NULL) fprintf(outfile,"</%s>\n",ftype[t]);
    else putc('\n',outfile);
    dir++;
  }
  dirs[lev] = 0;
  free(path);
  free_dir(sav);
}

void xml_indent(int maxlevel)
{
  int i;
  
  fprintf(outfile, "    ");
  for(i=0; i<maxlevel; i++)
    fprintf(outfile, "  ");
}

void xml_fillinfo(struct _info *ent)
{
  #ifdef __USE_FILE_OFFSET64
  if (inodeflag) fprintf(outfile," inode=\"%lld\"",(long long)ent->inode);
  #else
  if (inodeflag) fprintf(outfile," inode=\"%ld\"",(long int)ent->inode);
  #endif
  if (devflag) fprintf(outfile, " dev=\"%d\"", (int)ent->dev);
  #ifdef __EMX__
  if (pflag) fprintf(outfile, " mode=\"%04o\" prot=\"%s\"",ent->attr, prot(ent->attr));
  #else
  if (pflag) fprintf(outfile, " mode=\"%04o\" prot=\"%s\"", ent->mode & (S_IRWXU|S_IRWXG|S_IRWXO|S_ISUID|S_ISGID|S_ISVTX), prot(ent->mode));
  #endif
  if (uflag) fprintf(outfile, " user=\"%s\"", uidtoname(ent->uid));
  if (gflag) fprintf(outfile, " group=\"%s\"", gidtoname(ent->gid));
  if (sflag) fprintf(outfile, " size=\"%lld\"", (long long int)(ent->size));
  if (Dflag) fprintf(outfile, " time=\"%s\"", do_date(cflag? ent->ctime : ent->mtime));
}
