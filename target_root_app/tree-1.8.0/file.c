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

extern bool dflag, Fflag, aflag, fflag, pruneflag;
extern bool noindent, force_color, flimit, matchdirs;
extern bool reverse;
extern char *pattern, *ipattern;

extern int (*cmpfunc)();
extern FILE *outfile;
extern int Level, *dirs, maxdirs;

extern bool colorize;
extern char *endcode;

extern char *file_comment, *file_pathsep;

enum ftok { T_PATHSEP, T_DIR, T_FILE, T_EOP };

char *nextpc(char **p, int *tok)
{
  static char prev = 0;
  char *s = *p;
  if (!**p) {
    *tok = T_EOP;	// Shouldn't happen.
    return NULL;
  }
  if (prev) {
    prev = 0;
    *tok = T_PATHSEP;
    return NULL;
  }
  if (strchr(file_pathsep, **p) != NULL) {
    (*p)++;
    *tok = T_PATHSEP;
    return NULL;
  }
  while(**p && strchr(file_pathsep,**p) == NULL) (*p)++;

  if (**p) {
    *tok = T_DIR;
    prev = **p;
    *(*p)++ = '\0';
  } else *tok = T_FILE;
  return s;
}

struct _info *newent(char *name) {
  struct _info *n = xmalloc(sizeof(struct _info));
  memset(n,0,sizeof(struct _info));
  n->name = strdup(name);
  n->child = NULL;
  n->tchild = n->next = NULL;
  return n;
}

// Should replace this with a Red-Black tree implementation or the like
struct _info *search(struct _info **dir, char *name)
{
  struct _info *ptr, *prev, *n;
  int cmp;

  if (*dir == NULL) return (*dir = newent(name));

  for(prev = ptr = *dir; ptr != NULL; ptr=ptr->next) {
    cmp = strcmp(ptr->name,name);
    if (cmp == 0) return ptr;
    if (cmp > 0) break;
    prev = ptr;
  }
  n = newent(name);
  n->next = ptr;
  if (prev == ptr) *dir = n;
  else prev->next = n;
  return n;
}

void freefiletree(struct _info *ent)
{
  struct _info *ptr = ent, *t;

  while (ptr != NULL) {
    if (ptr->tchild) freefiletree(ptr->tchild);
    t = ptr;
    ptr = ptr->next;
    free(t);
  }
}

/**
 * Recursively prune (unset show flag) files/directories of matches/ignored
 * patterns:
 */
struct _info **fprune(struct _info *head, bool matched, bool root)
{
  struct _info **dir, *new = NULL, *end = NULL, *ent, *t;
  int show, count = 0;

  for(ent = head; ent != NULL;) {
    if (ent->tchild) ent->isdir = 1;

    show = 1;
    if (dflag && !ent->isdir) show = 0;
    if (!aflag && !root && ent->name[0] == '.') show = 0;
    if (show && !matched) {
      if (!ent->isdir) {
	if (pattern && patmatch(ent->name, pattern) == 0) show = 0;
	if (ipattern && patmatch(ent->name, pattern) == 1) show = 0;
      }
      if (ent->isdir && show && matchdirs && pattern) {
	if (patmatch(ent->name, pattern) == 1) matched = TRUE;
      }
    }
    if (pruneflag && !matched && ent->isdir && ent->tchild == NULL) show = 0;
    if (show && ent->tchild != NULL) ent->child = fprune(ent->tchild, matched, FALSE);

    t = ent;
    ent = ent->next;
    if (show) {
      if (end) end = end->next = t;
      else new = end = t;
      count++;
    } else {
      t->next = NULL;
      freefiletree(t);
    }
  }
  if (end) end->next = NULL;

  dir = xmalloc(sizeof(struct _info *) * (count+1));
  for(count = 0, ent = new; ent != NULL; ent = ent->next, count++) {
    dir[count] = ent;
  }
  dir[count] = NULL;

  return dir;
}

struct _info **file_getfulltree(char *d, u_long lev, dev_t dev, off_t *size, char **err)
{
  FILE *fp = (strcmp(d,".")? fopen(d,"r") : stdin);
  char *path, *spath, *s;
  long pathsize;
  struct _info *root = NULL, **cwd, *ent;
  int l, tok;

  size = 0;
  if (fp == NULL) {
    fprintf(stderr,"Error opening %s for reading.\n", d);
    return NULL;
  }
  // 64K paths maximum
  path = xmalloc(sizeof(char *) * (pathsize = (64 * 1024)));

  while(fgets(path, pathsize, fp) != NULL) {
    if (file_comment != NULL && strcmp(path,file_comment) == 0) continue;
    l = strlen(path);
    while(l && isspace(path[l-1])) path[--l] = '\0';
    if (l == 0) continue;

    spath = path;
    cwd = &root;
    do {
      s = nextpc(&spath, &tok);
      if (tok == T_PATHSEP) continue;
      switch(tok) {
	case T_PATHSEP: continue;
	case T_FILE:
	case T_DIR:
	  // Should probably handle '.' and '..' entries here
	  ent = search(cwd, s);
	  // Might be empty, but should definitely be considered a directory:
	  if (tok == T_DIR) {
	    ent->isdir = 1;
	    ent->mode = S_IFDIR;
	  } else {
	    ent->mode = S_IFREG;
	  }
	  cwd = &(ent->tchild);
	  break;
      }
    } while (tok != T_FILE && tok != T_EOP);
  }
  if (fp != stdin) fclose(fp);

  // Prune accumulated directory tree:
  return fprune(root, FALSE, TRUE);
}

// void f_listdir(struct _info *dir, char *d, int *dt, int *ft, u_long lev)
// {
//   char *path;
//   long pathsize = 0;
//   bool nlf = FALSE, colored = FALSE;
// 
//   if (dir == NULL) return;
// 
//   dirs[lev] = (dir->next? 1 : 2);
//   fprintf(outfile,"\n");
// 
//   path = malloc(pathsize=4096);
// 
//   while(dir) {
//     if (!noindent) indent(lev);
// 
//     if (colorize) {
//       colored = color(dir->isdir? S_IFDIR : S_IFREG, dir->name, FALSE, FALSE);
//     }
// 
//     if (fflag) {
//       if (sizeof(char) * (strlen(d)+strlen(dir->name)+2) > pathsize)
// 	path=xrealloc(path,pathsize=(sizeof(char) * (strlen(d)+strlen(dir->name)+1024)));
//       if (!strcmp(d,"/")) sprintf(path,"%s%s",d,dir->name);
//       else sprintf(path,"%s/%s",d,dir->name);
//     } else {
//       if (sizeof(char) * (strlen(dir->name)+1) > pathsize)
// 	path=xrealloc(path,pathsize=(sizeof(char) * (strlen(dir->name)+1024)));
//       sprintf(path,"%s",dir->name);
//     }
//     
//     printit(path);
//     
//     if (colored) fprintf(outfile,"%s",endcode);
//     if (Fflag && dir->isdir) fputc(Ftype(S_IFDIR), outfile);
//     
//     if (dir->child) {
//       if (fflag) {
// 	if (strlen(d)+strlen(dir->name)+2 > pathsize) path=xrealloc(path,pathsize=(strlen(d)+strlen(dir->name)+1024));
// 	if (!strcmp(d,"/")) sprintf(path,"%s%s",d,dir->name);
// 	else sprintf(path,"%s/%s",d,dir->name);
//       }
//       f_listdir(dir->child, fflag? path : NULL, dt, ft, lev+1);
//       nlf = TRUE;
//       *dt += 1;
//     } else {
//       if (dir->isdir) *dt += 1;
//       else *ft += 1;
//     }
// 
//     if (dir->next && !dir->next->next) dirs[lev] = 2;
//     if (nlf) nlf = FALSE;
//     else fprintf(outfile,"\n");
//     dir=dir->next;
//   }
//   dirs[lev] = 0;
//   free(path);
// }

// void file_listdir(char *d, int *dt, int *ft, u_long lev)
// {
//   FILE *fp = (d != NULL? fopen(d,"r") : stdin);
//   struct _info *root;
//   
//   if (fp == NULL) {
//     fprintf(stderr,"Error opening %s for reading.\n", d);
//     return;
//   }
//   root = getfulltree(fp, lev);
//   if (d != NULL) fclose(fp);
// 
//   memset(dirs, 0, sizeof(int) * maxdirs);
// 
//   f_listdir(root, "/", dt, ft, lev);
// 
//   freefiletree(root);
//   return;
// }
