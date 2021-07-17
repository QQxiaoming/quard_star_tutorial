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
extern bool Dflag, inodeflag, devflag, Rflag;
extern bool noindent, force_color, xdev, nolinks, flimit;
//extern char *title,
extern char *host, *sp;

extern struct _info **(*getfulltree)(char *d, u_long lev, dev_t dev, off_t *size, char **err);
extern void (*listdir)(char *, int *, int *, u_long, dev_t);
extern int (*cmpfunc)();
extern FILE *outfile;
extern int Level, *dirs, maxdirs;

extern bool colorize, linktargetcolor;
extern char *endcode;

void html_encode(FILE *, char *), url_encode(FILE *, char *);
void url_encode(FILE *fd, char *s);

void emit_html_header(const char *charset, char *title, char *version)
{
  fprintf(outfile,
	"<!DOCTYPE html>\n"
	"<html>\n"
	"<head>\n"
	" <meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n"
	" <meta name=\"Author\" content=\"Made by 'tree'\">\n"
	" <meta name=\"GENERATOR\" content=\"%s\">\n"
	" <title>%s</title>\n"
	" <style type=\"text/css\">\n  <!-- \n"
	"  BODY { font-family : ariel, monospace, sans-serif; }\n"
	"  P { font-weight: normal; font-family : ariel, monospace, sans-serif; color: black; background-color: transparent;}\n"
	"  B { font-weight: normal; color: black; background-color: transparent;}\n"
	"  A:visited { font-weight : normal; text-decoration : none; background-color : transparent; margin : 0px 0px 0px 0px; padding : 0px 0px 0px 0px; display: inline; }\n"
	"  A:link    { font-weight : normal; text-decoration : none; margin : 0px 0px 0px 0px; padding : 0px 0px 0px 0px; display: inline; }\n"
	"  A:hover   { color : #000000; font-weight : normal; text-decoration : underline; background-color : yellow; margin : 0px 0px 0px 0px; padding : 0px 0px 0px 0px; display: inline; }\n"
	"  A:active  { color : #000000; font-weight: normal; background-color : transparent; margin : 0px 0px 0px 0px; padding : 0px 0px 0px 0px; display: inline; }\n"
	"  .VERSION { font-size: small; font-family : arial, sans-serif; }\n"
	"  .NORM  { color: black;  background-color: transparent;}\n"
	"  .FIFO  { color: purple; background-color: transparent;}\n"
	"  .CHAR  { color: yellow; background-color: transparent;}\n"
	"  .DIR   { color: blue;   background-color: transparent;}\n"
	"  .BLOCK { color: yellow; background-color: transparent;}\n"
	"  .LINK  { color: aqua;   background-color: transparent;}\n"
	"  .SOCK  { color: fuchsia;background-color: transparent;}\n"
	"  .EXEC  { color: green;  background-color: transparent;}\n"
	"  -->\n </style>\n"
	"</head>\n"
	"<body>\n"
	"\t<h1>%s</h1><p>\n\t",charset ? charset : "iso-8859-1", version, title, title);
}

off_t html_listdir(char *d, int *dt, int *ft, u_long lev, dev_t dev)
{
  char *path;
  bool nlf = FALSE, colored = FALSE;
  long pathsize = 0;
  struct _info **dir, **sav;
  struct stat sb;
  int i,n,c;
  char hclr[20], *hdir, *hcmd;
  
  if ((Level >= 0) && (lev > Level)) {
    fprintf(outfile, "<br>\n");
    return 0;
  }
  
  if (xdev && lev == 0) {
    stat(d,&sb);
    dev = sb.st_dev;
  }
  
  sav = dir = read_dir(d,&n);
  if (!dir && n) {
    fprintf(outfile," [error opening dir]<br>\n");
    return 0;
  }
  if (!n) {
    fprintf(outfile,"<br>\n");
    free_dir(sav);
    return 0;
  }
  if (flimit > 0 && n > flimit) {
    fprintf(outfile," [%d entries exceeds filelimit, not opening dir]<br>\n",n);
    free_dir(sav);
    return 0;
  }
  
  if (cmpfunc) qsort(dir,n,sizeof(struct _info *),cmpfunc);
  if (lev >= maxdirs-1) {
    dirs = xrealloc(dirs,sizeof(int) * (maxdirs += 1024));
    memset(dirs+(maxdirs-1024), 0, sizeof(int) * 1024);
  }
  dirs[lev] = 1;
  if (!*(dir+1)) dirs[lev] = 2;
  fprintf(outfile,"<br>\n");
  
  path = malloc(pathsize=4096);
  
  while(*dir) {
    if (!noindent) indent(lev);
    
    fillinfo(path,*dir);
    if (path[0] == ' ') {
      path[0] = '[';
      for(i=0;path[i];i++) {
	if (path[i] == ' ') fprintf(outfile,"%s",sp);
	else fprintf(outfile,"%c", path[i]);
      }
      fprintf(outfile,"]%s%s", sp, sp);
    }
    
    if (colorize) {
      if ((*dir)->lnk && linktargetcolor) colored = color((*dir)->lnkmode,(*dir)->name,(*dir)->orphan,FALSE);
      else colored = color((*dir)->mode,(*dir)->name,(*dir)->orphan,FALSE);
    }
    
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

    /* This is really hackish and should be done over. */
    if (Rflag && (lev == Level) && (*dir)->isdir) {
      if (nolinks) fprintf(outfile,"%s",(*dir)->name);
      else {
	fprintf(outfile,"<a href=\"%s",host);
	url_encode(outfile,d+1);
	putc('/',outfile);
	url_encode(outfile,(*dir)->name);
	fprintf(outfile,"/00Tree.html\">");
	html_encode(outfile,(*dir)->name);
	fprintf(outfile,"</a><br>\n");
      }
	
      hdir = gnu_getcwd();
      if (sizeof(char) * (strlen(hdir)+strlen(d)+strlen((*dir)->name)+2) > pathsize)
	path = xrealloc(path, pathsize = sizeof(char) * (strlen(hdir)+strlen(d)+strlen((*dir)->name) + 1024));
	
      sprintf(path,"%s%s/%s",hdir,d+1,(*dir)->name);
      fprintf(stderr,"Entering directory %s\n",path);

      hcmd = xmalloc(sizeof(char) * (49 + strlen(host) + strlen(d) + strlen((*dir)->name)) + 10 + (2*strlen(path)));
      sprintf(hcmd,"tree -n -H \"%s%s/%s\" -L %d -R -o \"%s/00Tree.html\" \"%s\"\n", host,d+1,(*dir)->name,Level+1,path,path);
      system(hcmd);
      free(hdir);
      free(hcmd);
    } else {
      if (nolinks) {
	if (force_color) {
	  /*
	   * Note that the B element has been set to normal weight in the
	   * style portion of the output. so using <b> will just gives us a element
	   * for which we can assign a color class to.
	   */
	  fprintf(outfile, "<b class=\"%s\">%s</b>",
	    (*dir)->isdir ?  "DIR"  :
	    (*dir)->isexe ?  "EXEC" :
	    (*dir)->isfifo ? "FIFO" :
	    (*dir)->issok ?  "SOCK" : "NORM", (*dir)->name);
	} else
	  fprintf(outfile,"%s",(*dir)->name);
      } else {
	if (force_color) {
	  sprintf(hclr, "%s",
	    (*dir)->isdir ?  "DIR"  :
	    (*dir)->isexe ?  "EXEC" :
	    (*dir)->isfifo ? "FIFO" :
	    (*dir)->issok ?  "SOCK" : "NORM");
	  fprintf(outfile,"<a class=\"%s\" href=\"%s%s/%s%s\">%s</a>", hclr, host,d+1,(*dir)->name,
	    ((*dir)->isdir?"/":""),(*dir)->name);
	} else {
	  fprintf(outfile,"<a href=\"%s",host);
	  url_encode(outfile,d+1);
	  putc('/',outfile);
	  url_encode(outfile,(*dir)->name);
	  fprintf(outfile,"%s\">",((*dir)->isdir?"/":""));
	  html_encode(outfile,(*dir)->name);
	  fprintf(outfile,"</a>");
	}
      }
    }
    
    if (colored) fprintf(outfile,"%s",endcode);
    if (Fflag && !(*dir)->lnk) {
      if ((c = Ftype((*dir)->mode))) fputc(c, outfile);
    }
    
    if ((*dir)->isdir) {
      if ((*dir)->lnk) {
	if (lflag && !(xdev && dev != (*dir)->dev)) {
	  if (findino((*dir)->inode,(*dir)->dev)) {
	    fprintf(outfile,"  [recursive, not followed]");
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
    if (nlf) nlf = FALSE;
    else fprintf(outfile,"<br>\n");
    dir++;
  }
  dirs[lev] = 0;
  free(path);
  free_dir(sav);
  return 0;
}

off_t html_rlistdir(char *d, int *dt, int *ft, u_long lev, dev_t dev)
{
  struct _info **dir;
  off_t size = 0;
  char *err;
  
  dir = getfulltree(d, lev, dev, &size, &err);
  
  memset(dirs, 0, sizeof(int) * maxdirs);
  
  htmlr_listdir(dir, d, dt, ft, lev);
  
  return size;
}

void htmlr_listdir(struct _info **dir, char *d, int *dt, int *ft, u_long lev)
{
  char *path;
  long pathsize = 0;
  bool nlf = FALSE, colored = FALSE;
  struct _info **sav = dir;
  int i,c;
  char hclr[20], *hdir, *hcmd;
  
  if (dir == NULL) return;

  dirs[lev] = 1;
  if (!*(dir+1)) dirs[lev] = 2;
  fprintf(outfile,"<br>\n");
  
  path = malloc(pathsize=4096);
  
  while(*dir) {
    if (!noindent) indent(lev);
    
    fillinfo(path,*dir);
    if (path[0] == ' ') {
      path[0] = '[';
      for(i=0;path[i];i++) {
	if (path[i] == ' ') fprintf(outfile,"%s",sp);
	else fprintf(outfile,"%c", path[i]);
      }
      fprintf(outfile,"]%s%s", sp, sp);
    }
    
    if (colorize) {
      if ((*dir)->lnk && linktargetcolor) colored = color((*dir)->lnkmode,(*dir)->name,(*dir)->orphan,FALSE);
      else colored = color((*dir)->mode,(*dir)->name,(*dir)->orphan,FALSE);
    }
    
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

    /* This is really hackish and should be done over. */
    if (Rflag && (lev == Level) && (*dir)->isdir) {
      if (nolinks) fprintf(outfile,"%s",(*dir)->name);
      else {
	fprintf(outfile,"<a href=\"%s",host);
	url_encode(outfile,d+1);
	putc('/',outfile);
	url_encode(outfile,(*dir)->name);
	fprintf(outfile,"/00Tree.html\">");
	html_encode(outfile,(*dir)->name);
	fprintf(outfile,"</a><br>\n");
      }
      
      hdir = gnu_getcwd();
      if (sizeof(char) * (strlen(hdir)+strlen(d)+strlen((*dir)->name)+2) > pathsize)
	path = xrealloc(path, pathsize = sizeof(char) * (strlen(hdir)+strlen(d)+strlen((*dir)->name) + 1024));
      
      sprintf(path,"%s%s/%s",hdir,d+1,(*dir)->name);
      fprintf(stderr,"Entering directory %s\n",path);
      
      hcmd = xmalloc(sizeof(char) * (49 + strlen(host) + strlen(d) + strlen((*dir)->name)) + 10 + (2*strlen(path)));
      sprintf(hcmd,"tree -n -H \"%s%s/%s\" -L %d -R -o \"%s/00Tree.html\" \"%s\"\n", host,d+1,(*dir)->name,Level+1,path,path);
      system(hcmd);
      free(hdir);
      free(hcmd);
    } else {
      if (nolinks) {
	if (force_color) {
	  /*
	   * Note that the B element has been set to normal weight in the
	   * style portion of the output. so using <b> will just gives us a element
	   * for which we can assign a color class to.
	   */
	  fprintf(outfile, "<b class=\"%s\">%s</b>",
		  (*dir)->isdir ?  "DIR"  :
		  (*dir)->isexe ?  "EXEC" :
		  (*dir)->isfifo ? "FIFO" :
		  (*dir)->issok ?  "SOCK" : "NORM", (*dir)->name);
	} else
	  fprintf(outfile,"%s",(*dir)->name);
      } else {
	if (force_color) {
	  sprintf(hclr, "%s",
		  (*dir)->isdir ?  "DIR"  :
		  (*dir)->isexe ?  "EXEC" :
		  (*dir)->isfifo ? "FIFO" :
		  (*dir)->issok ?  "SOCK" : "NORM");
	  fprintf(outfile,"<a class=\"%s\" href=\"%s%s/%s%s\">%s</a>", hclr, host,d+1,(*dir)->name,
		  ((*dir)->isdir?"/":""),(*dir)->name);
	} else {
	  fprintf(outfile,"<a href=\"%s",host);
	  url_encode(outfile,d+1);
	  putc('/',outfile);
	  url_encode(outfile,(*dir)->name);
	  fprintf(outfile,"%s\">",((*dir)->isdir?"/":""));
	  html_encode(outfile,(*dir)->name);
	  fprintf(outfile,"</a>");
	}
      }
    }
    
    if (colored) fprintf(outfile,"%s",endcode);
    if (Fflag && !(*dir)->lnk) {
      if ((c = Ftype((*dir)->mode))) fputc(c, outfile);
    }

    if ((*dir)->child) {
      if (strlen(d)+strlen((*dir)->name)+2 > pathsize) path=xrealloc(path,pathsize=(strlen(d)+strlen((*dir)->name)+1024));
      if (!strcmp(d,"/")) sprintf(path,"%s%s",d,(*dir)->name);
      else sprintf(path,"%s/%s",d,(*dir)->name);
      htmlr_listdir((*dir)->child, path, dt, ft, lev+1);
      nlf = TRUE;
      *dt += 1;
    } else {
      if ((*dir)->isdir) *dt += 1;
      else *ft += 1;
    }

    if (*(dir+1) && !*(dir+2)) dirs[lev] = 2;
    if (nlf) nlf = FALSE;
    else fprintf(outfile,"<br>\n");
    dir++;
  }
  dirs[lev] = 0;
  free(path);
  free_dir(sav);
  return;
}

void html_encode(FILE *fd, char *s)
{
  for(;*s;s++) {
    switch(*s) {
      case '<':
	fputs("&lt;",fd);
	break;
      case '>':
	fputs("&gt;",fd);
	break;
      case '&':
	fputs("&amp;",fd);
	break;
      case '"':
	fputs("&quot;",fd);
	break;
      default:
	fputc(*s,fd);
	//	fputc(isprint(*s)?*s:'?',fd);
	break;
    }
  }
}

void url_encode(FILE *fd, char *s)
{
  for(;*s;s++) {
    switch(*s) {
      case ' ':
      case '"':
      case '#':
      case '%':
      case '<':
      case '>':
      case '[':
      case ']':
      case '^':
      case '\\':
      case '?':
      case '+':
	fprintf(fd,"%%%02X",*s);
	break;
      case '&':
	fprintf(fd,"&amp;");
	break;
      default:
	fprintf(fd,isprint((u_int)*s)?"%c":"%%%02X",(u_char)*s);
	break;
    }
  }
}

