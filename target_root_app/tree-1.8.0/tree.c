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

static char *version ="$Version: $ tree v1.8.0 (c) 1996 - 2018 by Steve Baker, Thomas Moore, Francesc Rocher, Florian Sesser, Kyosuke Tokoro $";
static char *hversion="\t\t tree v1.8.0 %s 1996 - 2018 by Steve Baker and Thomas Moore <br>\n"
		      "\t\t HTML output hacked and copyleft %s 1998 by Francesc Rocher <br>\n"
		      "\t\t JSON output hacked and copyleft %s 2014 by Florian Sesser <br>\n"
		      "\t\t Charsets / OS/2 support %s 2001 by Kyosuke Tokoro\n";

/* Globals */
bool dflag, lflag, pflag, sflag, Fflag, aflag, fflag, uflag, gflag;
bool qflag, Nflag, Qflag, Dflag, inodeflag, devflag, hflag, Rflag;
bool Hflag, siflag, cflag, Xflag, Jflag, duflag, pruneflag;
bool noindent, force_color, nocolor, xdev, noreport, nolinks, flimit, dirsfirst;
bool ignorecase, matchdirs, fromfile;
bool reverse;
char *pattern = NULL, *ipattern = NULL, *host = NULL, *title = "Directory Tree", *sp = " ", *_nl = "\n";
char *file_comment = "#", *file_pathsep = "/";
char *timefmt = NULL;
const char *charset = NULL;

struct _info **(*getfulltree)(char *d, u_long lev, dev_t dev, off_t *size, char **err) = unix_getfulltree;
off_t (*listdir)(char *, int *, int *, u_long, dev_t) = unix_listdir;
int (*cmpfunc)() = alnumsort;

char *sLevel, *curdir, *outfilename = NULL;
FILE *outfile;
int Level, *dirs, maxdirs;

int mb_cur_max;

#ifdef __EMX__
const u_short ifmt[]={ FILE_ARCHIVED, FILE_DIRECTORY, FILE_SYSTEM, FILE_HIDDEN, FILE_READONLY, 0};
#else
  #ifdef S_IFPORT
  const u_int ifmt[] = {S_IFREG, S_IFDIR, S_IFLNK, S_IFCHR, S_IFBLK, S_IFSOCK, S_IFIFO, S_IFDOOR, S_IFPORT, 0};
  const char fmt[] = "-dlcbspDP?";
  const char *ftype[] = {"file", "directory", "link", "char", "block", "socket", "fifo", "door", "port", "unknown", NULL};
  #else
  const u_int ifmt[] = {S_IFREG, S_IFDIR, S_IFLNK, S_IFCHR, S_IFBLK, S_IFSOCK, S_IFIFO, 0};
  const char fmt[] = "-dlcbsp?";
  const char *ftype[] = {"file", "directory", "link", "char", "block", "socket", "fifo", "unknown", NULL};
  #endif
#endif

struct sorts {
  char *name;
  int (*cmpfunc)();
} sorts[] = {
  {"name", alnumsort},
  {"version", versort},
  {"size", fsizesort},
  {"mtime", mtimesort},
  {"ctime", ctimesort},
  {NULL, NULL}
};

/* Externs */
/* hash.c */
extern struct xtable *gtable[256], *utable[256];
extern struct inotable *itable[256];
/* color.c */
extern bool colorize, ansilines, linktargetcolor;
extern char *leftcode, *rightcode, *endcode;
extern const struct linedraw *linedraw;


int main(int argc, char **argv)
{
  char **dirname = NULL;
  int i,j=0,k,n,optf,p,q,dtotal,ftotal,colored = FALSE;
  struct stat st;
  char sizebuf[64], *stmp;
  off_t size = 0;
  mode_t mt;
  bool needfulltree;

  q = p = dtotal = ftotal = 0;
  aflag = dflag = fflag = lflag = pflag = sflag = Fflag = uflag = gflag = FALSE;
  Dflag = qflag = Nflag = Qflag = Rflag = hflag = Hflag = siflag = cflag = FALSE;
  noindent = force_color = nocolor = xdev = noreport = nolinks = reverse = FALSE;
  ignorecase = matchdirs = dirsfirst = inodeflag = devflag = Xflag = Jflag = FALSE;
  duflag = pruneflag = FALSE;
  flimit = 0;
  dirs = xmalloc(sizeof(int) * (maxdirs=4096));
  memset(dirs, 0, sizeof(int) * maxdirs);
  dirs[0] = 0;
  Level = -1;

  setlocale(LC_CTYPE, "");
  setlocale(LC_COLLATE, "");

  charset = getcharset();
  if (charset == NULL && strcmp(nl_langinfo(CODESET), "UTF-8") == 0) {
    charset = "UTF-8";
  }

/* Until I get rid of this hack, make it linux/cygwin/HP nonstop only: */
#if defined (LINUX) || defined (CYGWIN) || defined (__TANDEM)
  mb_cur_max = (int)MB_CUR_MAX;
#else
  mb_cur_max = 1;
#endif

  memset(utable,0,sizeof(utable));
  memset(gtable,0,sizeof(gtable));
  memset(itable,0,sizeof(itable));

  optf = TRUE;
  for(n=i=1;i<argc;i=n) {
    n++;
    if (optf && argv[i][0] == '-' && argv[i][1]) {
      for(j=1;argv[i][j];j++) {
	switch(argv[i][j]) {
	case 'N':
	  Nflag = TRUE;
	  break;
	case 'q':
	  qflag = TRUE;
	  break;
	case 'Q':
	  Qflag = TRUE;
	  break;
	case 'd':
	  dflag = TRUE;
	  break;
	case 'l':
	  lflag = TRUE;
	  break;
	case 's':
	  sflag = TRUE;
	  break;
	case 'h':
	  hflag = TRUE;
	  sflag = TRUE; /* Assume they also want -s */
	  break;
	case 'u':
	  uflag = TRUE;
	  break;
	case 'g':
	  gflag = TRUE;
	  break;
	case 'f':
	  fflag = TRUE;
	  break;
	case 'F':
	  Fflag = TRUE;
	  break;
	case 'a':
	  aflag = TRUE;
	  break;
	case 'p':
	  pflag = TRUE;
	  break;
	case 'i':
	  noindent = TRUE;
	  _nl = "";
	  break;
	case 'C':
	  force_color = TRUE;
	  break;
	case 'n':
	  nocolor = TRUE;
	  break;
	case 'x':
	  xdev = TRUE;
	  break;
	case 'P':
	  if (argv[n] == NULL) {
	    fprintf(stderr,"tree: missing argument to -P option.\n");
	    exit(1);
	  }
	  pattern = argv[n++];
	  break;
	case 'I':
	  if (argv[n] == NULL) {
	    fprintf(stderr,"tree: missing argument to -I option.\n");
	    exit(1);
	  }
	  ipattern = argv[n++];
	  break;
	case 'A':
	  ansilines = TRUE;
	  break;
	case 'S':
	  charset = "IBM437";
	  break;
	case 'D':
	  Dflag = TRUE;
	  break;
	case 't':
	  cmpfunc = mtimesort;
	  break;
	case 'c':
	  cmpfunc = ctimesort;
	  cflag = TRUE;
	  break;
	case 'r':
	  reverse = TRUE;
	  break;
	case 'v':
	  cmpfunc = versort;
	  break;
	case 'U':
	  cmpfunc = NULL;
	  break;
	case 'X':
	  Hflag = FALSE;
	  Xflag = TRUE;
	  break;
	case 'J':
	  Jflag = TRUE;
	  break;
	case 'H':
	  Hflag = TRUE;
	  Xflag = FALSE;
	  if (argv[n] == NULL) {
	    fprintf(stderr,"tree: missing argument to -H option.\n");
	    exit(1);
	  }
	  host = argv[n++];
	  sp = "&nbsp;";
	  break;
	case 'T':
	  if (argv[n] == NULL) {
	    fprintf(stderr,"tree: missing argument to -T option.\n");
	    exit(1);
	  }
	  title = argv[n++];
	  break;
	case 'R':
	  Rflag = TRUE;
	  break;
	case 'L':
	  if ((sLevel = argv[n++]) == NULL) {
	    fprintf(stderr,"tree: Missing argument to -L option.\n");
	    exit(1);
	  }
	  Level = strtoul(sLevel,NULL,0)-1;
	  if (Level < 0) {
	    fprintf(stderr,"tree: Invalid level, must be greater than 0.\n");
	    exit(1);
	  }
	  break;
	case 'o':
	  if (argv[n] == NULL) {
	    fprintf(stderr,"tree: missing argument to -o option.\n");
	    exit(1);
	  }
	  outfilename = argv[n++];
	  break;
	case '-':
	  if (j == 1) {
	    if (!strcmp("--", argv[i])) {
	      optf = FALSE;
	      break;
	    }
	    if (!strcmp("--help",argv[i])) {
	      usage(2);
	      exit(0);
	    }
	    if (!strcmp("--version",argv[i])) {
	      char *v = version+12;
	      printf("%.*s\n",(int)strlen(v)-1,v);
	      exit(0);
	    }
	    if (!strcmp("--inodes",argv[i])) {
	      j = strlen(argv[i])-1;
	      inodeflag=TRUE;
	      break;
	    }
	    if (!strcmp("--device",argv[i])) {
	      j = strlen(argv[i])-1;
	      devflag=TRUE;
	      break;
	    }
	    if (!strcmp("--noreport",argv[i])) {
	      j = strlen(argv[i])-1;
	      noreport = TRUE;
	      break;
	    }
	    if (!strcmp("--nolinks",argv[i])) {
	      j = strlen(argv[i])-1;
	      nolinks = TRUE;
	      break;
	    }
	    if (!strcmp("--dirsfirst",argv[i])) {
	      j = strlen(argv[i])-1;
	      dirsfirst = TRUE;
	      break;
	    }
	    if (!strncmp("--filelimit",argv[i],11)) {
	      j = 11;
	      if (*(argv[i]+11) == '=') {
		if (*(argv[i]+12)) {
		  flimit=atoi(argv[i]+12);
		  j = strlen(argv[i])-1;
		  break;
		} else {
		  fprintf(stderr,"tree: missing argument to --filelimit=\n");
		  exit(1);
		}
	      }
	      if (argv[n] != NULL) {
		flimit = atoi(argv[n++]);
		j = strlen(argv[i])-1;
	      } else {
		fprintf(stderr,"tree: missing argument to --filelimit\n");
		exit(1);
	      }
	      break;
	    }
	    if (!strncmp("--charset",argv[i],9)){
	      j = 9;
	      if (*(argv[i]+j) == '=') {
		if (*(charset = (argv[i]+10))) {
		  j = strlen(argv[i])-1;
		  break;
		} else {
		  fprintf(stderr,"tree: missing argument to --charset=\n");
		  exit(1);
		}
	      }
	      if (argv[n] != NULL) {
		charset = argv[n++];
		j = strlen(argv[i])-1;
	      } else {
		initlinedraw(1);
		exit(1);
	      }
	      break;
	    }
	    if (!strncmp("--si", argv[i], 4)) {
	      j = strlen(argv[i])-1;
	      sflag = TRUE;
	      hflag = TRUE;
	      siflag = TRUE;
	      break;
	    }
	    if (!strncmp("--du",argv[i],4)) {
	      j = strlen(argv[i])-1;
	      sflag = TRUE;
	      duflag = TRUE;
	      break;
	    }
	    if (!strncmp("--prune",argv[i],7)) {
	      j = strlen(argv[i])-1;
	      pruneflag = TRUE;
	      break;
	    }
	    if (!strncmp("--timefmt",argv[i],9)) {
	      j = 9;
	      if (*(argv[i]+j) == '=') {
		if (*(argv[i]+ (++j))) {
		  timefmt=scopy(argv[i]+j);
		  j = strlen(argv[i])-1;
		  break;
		}else {
		  fprintf(stderr,"tree: missing argument to --timefmt=\n");
		  exit(1);
		}
	      } else if (argv[n] != NULL) {
		timefmt = scopy(argv[n]);
		n++;
		j = strlen(argv[i])-1;
	      } else {
		fprintf(stderr,"tree: missing argument to --timefmt\n");
		exit(1);
	      }
	      Dflag = TRUE;
	      break;
	    }
	    if (!strncmp("--ignore-case",argv[i],13)) {
	      j = strlen(argv[i])-1;
	      ignorecase = TRUE;
	      break;
	    }
	    if (!strncmp("--matchdirs",argv[i],11)) {
	      j = strlen(argv[i])-1;
	      matchdirs = TRUE;
	      break;
	    }
	    if (!strncmp("--sort",argv[i],6)) {
	      j = 6;
	      if (*(argv[i]+j) == '=') {
		if (*(argv[i]+(++j))) {
		  stmp = argv[i]+j;
		  j = strlen(argv[i])-1;
		} else {
		  fprintf(stderr,"tree: missing argument to --sort=\n");
		  exit(1);
		}
	      } else if (argv[n] != NULL) {
		stmp = argv[n++];
	      } else {
		fprintf(stderr,"tree: missing argument to --sort\n");
		exit(1);
	      }
	      cmpfunc = NULL;
	      for(k=0;sorts[k].name;k++) {
		if (strcasecmp(sorts[k].name,stmp) == 0) {
		  cmpfunc = sorts[k].cmpfunc;
		  break;
		}
	      }
	      if (cmpfunc == NULL) {
		fprintf(stderr,"tree: sort type '%s' not valid, should be one of: ", stmp);
		for(k=0; sorts[k].name; k++)
		  printf("%s%c", sorts[k].name, sorts[k+1].name? ',': '\n');
		exit(1);
	      }
	      break;
	    }
	    if (!strncmp("--fromfile",argv[i],10)) {
	      j = strlen(argv[i])-1;
	      fromfile=TRUE;
	      break;
	    }
	    fprintf(stderr,"tree: Invalid argument `%s'.\n",argv[i]);
	    usage(1);
	    exit(1);
	  }
	default:
	  fprintf(stderr,"tree: Invalid argument -`%c'.\n",argv[i][j]);
	  usage(1);
	  exit(1);
	  break;
	}
      }
    } else {
      if (!dirname) dirname = (char **)xmalloc(sizeof(char *) * (q=MINIT));
      else if (p == (q-2)) dirname = (char **)xrealloc(dirname,sizeof(char *) * (q+=MINC));
      dirname[p++] = scopy(argv[i]);
    }
  }
  if (p) dirname[p] = NULL;
  
  if (outfilename == NULL) {
#ifdef __EMX__
    _fsetmode(outfile=stdout,Hflag?"b":"t");
#else
    outfile = stdout;
#endif
  } else {
#ifdef __EMX__
    outfile = fopen(outfilename,Hflag?"wb":"wt");
#else
    outfile = fopen(outfilename,"w");
#endif
    if (outfile == NULL) {
      fprintf(stderr,"tree: invalid filename '%s'\n",outfilename);
      exit(1);
    }
  }

  
  if (timefmt)
    setlocale(LC_TIME,"");

  parse_dir_colors();
  initlinedraw(0);

  needfulltree = duflag || pruneflag || matchdirs || fromfile;

  if (fromfile) {
    getfulltree = file_getfulltree;
  }

  /* Set our listdir function and sanity check options. */
  if (Hflag) {
    listdir = needfulltree ? html_rlistdir : html_listdir;
    Xflag = FALSE;
  } else if (Xflag) {
    listdir = needfulltree ? xml_rlistdir : xml_listdir;
    colorize = FALSE;
    colored = FALSE; /* Do people want colored XML output? */
  } else if (Jflag) {
    listdir = needfulltree ? json_rlistdir : json_listdir;
    colorize = FALSE;
    colored = FALSE; /* Do people want colored JSON output? */
  } else {
    listdir = needfulltree ? unix_rlistdir : unix_listdir;
  }
  if (dflag) pruneflag = FALSE;	/* You'll just get nothing otherwise. */

  if (Rflag && (Level == -1))
    Rflag = FALSE;

  if (Hflag) {
    emit_html_header(charset, title, version);

    fflag = FALSE;
    if (nolinks) {
      if (force_color) fprintf(outfile, "<b class=\"NORM\">%s</b>",host);
      else fprintf(outfile,"%s",host);
    } else {
      if (force_color) fprintf(outfile,"<a class=\"NORM\" href=\"%s\">%s</a>",host,host);
      else fprintf(outfile,"<a href=\"%s\">%s</a>",host,host);
    }
    curdir = gnu_getcwd();
  } else if (Xflag) {
    fprintf(outfile,"<?xml version=\"1.0\"");
    if (charset) fprintf(outfile," encoding=\"%s\"",charset);
    fprintf(outfile,"?>%s<tree>%s",_nl,_nl);
  } else if (Jflag)
    fputc('[',outfile);

  if (dirname) {
    for(colored=i=0;dirname[i];i++,colored=0) {
      if (fflag) {
	do {
	  j=strlen(dirname[i]);
	  if (j > 1 && dirname[i][j-1] == '/') dirname[i][--j] = 0;
	} while (j > 1 && dirname[i][j-1] == '/');
      }
      if ((n = lstat(dirname[i],&st)) >= 0) {
	saveino(st.st_ino, st.st_dev);
	if (colorize) colored = color(st.st_mode,dirname[i],n<0,FALSE);
	size += st.st_size;
      }
      if (Xflag || Jflag) {
	mt = st.st_mode & S_IFMT;
	for(j=0;ifmt[j];j++)
	  if (ifmt[j] == mt) break;
        if (Xflag)
	  fprintf(outfile,"%s<%s name=\"%s\">", noindent?"":"  ", ftype[j], dirname[i]);
        else if (Jflag) {
	  if (i) fprintf(outfile, ",");
          fprintf(outfile,"%s{\"type\":\"%s\",\"name\":\"%s\",\"contents\":[", noindent?"":"\n  ", ftype[j], dirname[i]);
	}
      } else if (!Hflag) printit(dirname[i]);
      if (colored) fprintf(outfile,"%s",endcode);
      if (!Hflag) size += listdir(dirname[i],&dtotal,&ftotal,0,0);
      else {
	if (chdir(dirname[i])) {
	  fprintf(outfile,"%s [error opening dir]\n",dirname[i]);
	  exit(1);
	} else {
	  size += listdir(".",&dtotal,&ftotal,0,0);
	  chdir(curdir);
	}
      }
      if (Xflag) fprintf(outfile,"%s</%s>\n",noindent?"":"  ", ftype[j]);
      if (Jflag) fprintf(outfile,"%s]}",noindent?"":"  ");
    }
  } else {
    if ((n = lstat(".",&st)) >= 0) {
      saveino(st.st_ino, st.st_dev);
      if (colorize) colored = color(st.st_mode,".",n<0,FALSE);
      size = st.st_size;
    }
    if (Xflag) fprintf(outfile,"%s<directory name=\".\">",noindent?"":"  ");
    else if (Jflag) fprintf(outfile, "{\"type\":\"directory\",\"name\": \".\",\"contents\":[");
    else if (!Hflag) fprintf(outfile,".");
    if (colored) fprintf(outfile,"%s",endcode);
    size += listdir(".",&dtotal,&ftotal,0,0);
    if (Xflag) fprintf(outfile,"%s</directory>%s",noindent?"":"  ", _nl);
    if (Jflag) fprintf(outfile,"%s]}",noindent?"":"  ");
  }

  if (Hflag)
    fprintf(outfile,"\t<br><br>\n\t</p>\n\t<p>\n");

  if (!noreport) {
    if (Xflag) {
      fprintf(outfile,"%s<report>%s",noindent?"":"  ", _nl);
      if (duflag) fprintf(outfile,"%s<size>%lld</size>%s", noindent?"":"    ", (long long int)size, _nl);
      fprintf(outfile,"%s<directories>%d</directories>%s", noindent?"":"    ", dtotal, _nl);
      if (!dflag) fprintf(outfile,"%s<files>%d</files>%s", noindent?"":"    ", ftotal, _nl);
      fprintf(outfile,"%s</report>%s",noindent?"":"  ", _nl);
    } else if (Jflag) {
      fprintf(outfile, ",%s{\"type\":\"report\"",noindent?"":"\n  ");
      if (duflag) fprintf(outfile,",\"size\":%lld", (long long int)size);
      fprintf(outfile,",\"directories\":%d", dtotal);
      if (!dflag) fprintf(outfile,",\"files\":%d", ftotal);
      fprintf(outfile, "}");
    } else {
      if (duflag) {
	psize(sizebuf, size);
	fprintf(outfile,"\n%s%s used in ", sizebuf, hflag || siflag? "" : " bytes");
      } else fputc('\n', outfile);
      if (dflag)
	fprintf(outfile,"%d director%s\n",dtotal,(dtotal==1? "y":"ies"));
      else
	fprintf(outfile,"%d director%s, %d file%s\n",dtotal,(dtotal==1? "y":"ies"),ftotal,(ftotal==1? "":"s"));
    }
  }

  if (Hflag) {
    fprintf(outfile,"\t<br><br>\n\t</p>\n");
    fprintf(outfile,"\t<hr>\n");
    fprintf(outfile,"\t<p class=\"VERSION\">\n");
    fprintf(outfile,hversion,linedraw->copy, linedraw->copy, linedraw->copy, linedraw->copy);
    fprintf(outfile,"\t</p>\n");
    fprintf(outfile,"</body>\n");
    fprintf(outfile,"</html>\n");
  } else if (Xflag) {
    fprintf(outfile,"</tree>\n");
  } else if (Jflag) {
      fprintf(outfile, "%s]\n",_nl);
  }

  if (outfilename != NULL) fclose(outfile);

  return 0;
}

void usage(int n)
{
  /*     123456789!123456789!123456789!123456789!123456789!123456789!123456789!123456789! */
  /*     \t9!123456789!123456789!123456789!123456789!123456789!123456789!123456789! */
  fprintf(n < 2? stderr: stdout,
	"usage: tree [-acdfghilnpqrstuvxACDFJQNSUX] [-H baseHREF] [-T title ]\n"
	"\t[-L level [-R]] [-P pattern] [-I pattern] [-o filename] [--version]\n"
	"\t[--help] [--inodes] [--device] [--noreport] [--nolinks] [--dirsfirst]\n"
	"\t[--charset charset] [--filelimit[=]#] [--si] [--timefmt[=]<f>]\n"
	"\t[--sort[=]<name>] [--matchdirs] [--ignore-case] [--fromfile] [--]\n"
	"\t[<directory list>]\n");
  if (n < 2) return;
  fprintf(stdout,
	"  ------- Listing options -------\n"
	"  -a            All files are listed.\n"
	"  -d            List directories only.\n"
	"  -l            Follow symbolic links like directories.\n"
	"  -f            Print the full path prefix for each file.\n"
	"  -x            Stay on current filesystem only.\n"
	"  -L level      Descend only level directories deep.\n"
	"  -R            Rerun tree when max dir level reached.\n"
	"  -P pattern    List only those files that match the pattern given.\n"
	"  -I pattern    Do not list files that match the given pattern.\n"
	"  --ignore-case Ignore case when pattern matching.\n"
	"  --matchdirs   Include directory names in -P pattern matching.\n"
	"  --noreport    Turn off file/directory count at end of tree listing.\n"
	"  --charset X   Use charset X for terminal/HTML and indentation line output.\n"
	"  --filelimit # Do not descend dirs with more than # files in them.\n"
	"  --timefmt <f> Print and format time according to the format <f>.\n"
	"  -o filename   Output to file instead of stdout.\n"
	"  ------- File options -------\n"
	"  -q            Print non-printable characters as '?'.\n"
	"  -N            Print non-printable characters as is.\n"
	"  -Q            Quote filenames with double quotes.\n"
	"  -p            Print the protections for each file.\n"
	"  -u            Displays file owner or UID number.\n"
	"  -g            Displays file group owner or GID number.\n"
	"  -s            Print the size in bytes of each file.\n"
	"  -h            Print the size in a more human readable way.\n"
	"  --si          Like -h, but use in SI units (powers of 1000).\n"
	"  -D            Print the date of last modification or (-c) status change.\n"
	"  -F            Appends '/', '=', '*', '@', '|' or '>' as per ls -F.\n"
	"  --inodes      Print inode number of each file.\n"
	"  --device      Print device ID number to which each file belongs.\n"
	"  ------- Sorting options -------\n"
	"  -v            Sort files alphanumerically by version.\n"
	"  -t            Sort files by last modification time.\n"
	"  -c            Sort files by last status change time.\n"
	"  -U            Leave files unsorted.\n"
	"  -r            Reverse the order of the sort.\n"
	"  --dirsfirst   List directories before files (-U disables).\n"
	"  --sort X      Select sort: name,version,size,mtime,ctime.\n"
	"  ------- Graphics options -------\n"
	"  -i            Don't print indentation lines.\n"
	"  -A            Print ANSI lines graphic indentation lines.\n"
	"  -S            Print with CP437 (console) graphics indentation lines.\n"
	"  -n            Turn colorization off always (-C overrides).\n"
	"  -C            Turn colorization on always.\n"
	"  ------- XML/HTML/JSON options -------\n"
	"  -X            Prints out an XML representation of the tree.\n"
	"  -J            Prints out an JSON representation of the tree.\n"
	"  -H baseHREF   Prints out HTML format with baseHREF as top directory.\n"
	"  -T string     Replace the default HTML title and H1 header with string.\n"
	"  --nolinks     Turn off hyperlinks in HTML output.\n"
	"  ------- Input options -------\n"
	"  --fromfile    Reads paths from files (.=stdin)\n"
	"  ------- Miscellaneous options -------\n"
	"  --version     Print version and exit.\n"
	"  --help        Print usage and this help message and exit.\n"
	"  --            Options processing terminator.\n");
  exit(0);
}


struct _info **read_dir(char *dir, int *n)
{
  static char *path = NULL, *lbuf = NULL;
  static long pathsize, lbufsize;
  struct _info **dl;
  struct dirent *ent;
  struct stat lst,st;
  DIR *d;
  int ne, p = 0, len, rs;

  if (path == NULL) {
    pathsize = lbufsize = strlen(dir)+4096;
    path=xmalloc(pathsize);
    lbuf=xmalloc(lbufsize);
  }

  *n = 1;
  if ((d=opendir(dir)) == NULL) return NULL;

  dl = (struct _info **)xmalloc(sizeof(struct _info *) * (ne = MINIT));

  while((ent = (struct dirent *)readdir(d))) {
    if (!strcmp("..",ent->d_name) || !strcmp(".",ent->d_name)) continue;
    if (Hflag && !strcmp(ent->d_name,"00Tree.html")) continue;
    if (!aflag && ent->d_name[0] == '.') continue;

    if (strlen(dir)+strlen(ent->d_name)+2 > pathsize) path = xrealloc(path,pathsize=(strlen(dir)+strlen(ent->d_name)+4096));
    sprintf(path,"%s/%s",dir,ent->d_name);
    if (lstat(path,&lst) < 0) continue;
    if ((lst.st_mode & S_IFMT) == S_IFLNK) {
      if ((rs = stat(path,&st)) < 0) memset(&st, 0, sizeof(st));
    } else {
      rs = 0;
      st.st_mode = lst.st_mode;
      st.st_dev = lst.st_dev;
      st.st_ino = lst.st_ino;
    }

#ifndef __EMX__
    if ((lst.st_mode & S_IFMT) != S_IFDIR && !(lflag && ((st.st_mode & S_IFMT) == S_IFDIR))) {
      if (pattern && patmatch(ent->d_name,pattern) != 1) continue;
    }
    if (ipattern && patmatch(ent->d_name,ipattern) == 1) continue;
#endif

    if (dflag && ((st.st_mode & S_IFMT) != S_IFDIR)) continue;
#ifndef __EMX__
//    if (pattern && ((lst.st_mode & S_IFMT) == S_IFLNK) && !lflag) continue;
#endif

    if (p == (ne-1)) dl = (struct _info **)xrealloc(dl,sizeof(struct _info *) * (ne += MINC));
    dl[p] = (struct _info *)xmalloc(sizeof(struct _info));

    dl[p]->name = scopy(ent->d_name);
    /* We should just incorporate struct stat into _info, and elminate this unecessary copying.
     * Made sense long ago when we had fewer options and didn't need half of stat.
     */
    dl[p]->mode = lst.st_mode;
    dl[p]->uid = lst.st_uid;
    dl[p]->gid = lst.st_gid;
    dl[p]->size = lst.st_size;
    dl[p]->dev = st.st_dev;
    dl[p]->inode = st.st_ino;
    dl[p]->ldev = lst.st_dev;
    dl[p]->linode = lst.st_ino;
    dl[p]->lnk = NULL;
    dl[p]->orphan = FALSE;
    dl[p]->err = NULL;
    dl[p]->child = NULL;

    dl[p]->atime = lst.st_atime;
    dl[p]->ctime = lst.st_ctime;
    dl[p]->mtime = lst.st_mtime;

#ifdef __EMX__
    dl[p]->attr = lst.st_attr;
#else

    if ((lst.st_mode & S_IFMT) == S_IFLNK) {
      if (lst.st_size+1 > lbufsize) lbuf = xrealloc(lbuf,lbufsize=(lst.st_size+8192));
      if ((len=readlink(path,lbuf,lbufsize-1)) < 0) {
	dl[p]->lnk = scopy("[Error reading symbolic link information]");
	dl[p]->isdir = FALSE;
	dl[p++]->lnkmode = st.st_mode;
	continue;
      } else {
	lbuf[len] = 0;
	dl[p]->lnk = scopy(lbuf);
	if (rs < 0) dl[p]->orphan = TRUE;
	dl[p]->lnkmode = st.st_mode;
      }
    }
#endif

    /* These should be elminiated, as they're barely used */
    dl[p]->isdir = ((st.st_mode & S_IFMT) == S_IFDIR);
    dl[p]->issok = ((st.st_mode & S_IFMT) == S_IFSOCK);
    dl[p]->isfifo = ((st.st_mode & S_IFMT) == S_IFIFO);
    dl[p++]->isexe = (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) ? 1 : 0;
  }
  closedir(d);
  *n = p;
  dl[p] = NULL;
  return dl;
}

/* This is for all the impossible things people wanted the old tree to do.
 * This can and will use a large amount of memory for large directory trees
 * and also take some time.
 */
struct _info **unix_getfulltree(char *d, u_long lev, dev_t dev, off_t *size, char **err)
{
  char *path;
  long pathsize = 0;
  struct _info **dir, **sav, **p, *sp;
  struct stat sb;
  int n;
  u_long lev_tmp;
  char *tmp_pattern = NULL, *start_rel_path;

  *err = NULL;
  if (Level >= 0 && lev > Level) return NULL;
  if (xdev && lev == 0) {
    stat(d,&sb);
    dev = sb.st_dev;
  }
  // if the directory name matches, turn off pattern matching for contents
  if (matchdirs && pattern) {
    lev_tmp = lev;
    start_rel_path = d + strlen(d);
    for (start_rel_path = d + strlen(d); start_rel_path != d; --start_rel_path) {
      if (*start_rel_path == '/')
        --lev_tmp;
      if (lev_tmp <= 0) {
        if (*start_rel_path)
          ++start_rel_path;
        break;
      }
    }
    if (*start_rel_path && patmatch(start_rel_path,pattern) == 1) {
      tmp_pattern = pattern;
      pattern = NULL;
    }
  }
  sav = dir = read_dir(d,&n);
  if (tmp_pattern) {
    pattern = tmp_pattern;
    tmp_pattern = NULL;
  }
  if (dir == NULL) {
    *err = scopy("error opening dir");
    return NULL;
  }
  if (n == 0) {
    free_dir(sav);
    return NULL;
  }
  path = malloc(pathsize=4096);
  
  if (flimit > 0 && n > flimit) {
    sprintf(path,"%d entries exceeds filelimit, not opening dir",n);
    *err = scopy(path);
    free_dir(sav);
    free(path);
    return NULL;
  }
  if (cmpfunc) qsort(dir,n,sizeof(struct _info *),cmpfunc);
  
  if (lev >= maxdirs-1) {
    dirs = xrealloc(dirs,sizeof(int) * (maxdirs += 1024));
  }
  
  while (*dir) {
    if ((*dir)->isdir && !(xdev && dev != (*dir)->dev)) {
      if ((*dir)->lnk) {
	if (lflag) {
	  if (findino((*dir)->inode,(*dir)->dev)) {
	    (*dir)->err = scopy("recursive, not followed");
	  } else {
	    saveino((*dir)->inode, (*dir)->dev);
	    if (*(*dir)->lnk == '/')
	      (*dir)->child = unix_getfulltree((*dir)->lnk,lev+1,dev,&((*dir)->size),&((*dir)->err));
	    else {
	      if (strlen(d)+strlen((*dir)->lnk)+2 > pathsize) path=xrealloc(path,pathsize=(strlen(d)+strlen((*dir)->name)+1024));
	      if (fflag && !strcmp(d,"/")) sprintf(path,"%s%s",d,(*dir)->lnk);
	      else sprintf(path,"%s/%s",d,(*dir)->lnk);
	      (*dir)->child = unix_getfulltree(path,lev+1,dev,&((*dir)->size),&((*dir)->err));
	    }
	  }
	}
      } else {
	if (strlen(d)+strlen((*dir)->name)+2 > pathsize) path=xrealloc(path,pathsize=(strlen(d)+strlen((*dir)->name)+1024));
	if (fflag && !strcmp(d,"/")) sprintf(path,"%s%s",d,(*dir)->name);
	else sprintf(path,"%s/%s",d,(*dir)->name);
	saveino((*dir)->inode, (*dir)->dev);
	(*dir)->child = unix_getfulltree(path,lev+1,dev,&((*dir)->size),&((*dir)->err));
      }
      // prune empty folders, unless they match the requested pattern
      if (pruneflag && (*dir)->child == NULL &&
	  !(matchdirs && pattern && patmatch((*dir)->name,pattern) == 1)) {
	sp = *dir;
	for(p=dir;*p;p++) *p = *(p+1);
	n--;
	free(sp->name);
	if (sp->lnk) free(sp->lnk);
	free(sp);
	continue;
      }
    }
    if (duflag) *size += (*dir)->size;
    dir++;
  }
  free(path);
  if (n == 0) {
    free_dir(sav);
    return NULL;
  }
  return sav;
}

/* Sorting functions */
int alnumsort(struct _info **a, struct _info **b)
{
  int v;

  if (dirsfirst && ((*a)->isdir != (*b)->isdir)) {
    return (*a)->isdir ? -1 : 1;
  }
  v = strcoll((*a)->name,(*b)->name);
  return reverse? -v : v;
}

int versort(struct _info **a, struct _info **b)
{
  int v;

  if (dirsfirst && ((*a)->isdir != (*b)->isdir)) {
    return (*a)->isdir ? -1 : 1;
  }
  v = strverscmp((*a)->name,(*b)->name);
  return reverse? -v : v;
}

int mtimesort(struct _info **a, struct _info **b)
{
  int v;

  if (dirsfirst && ((*a)->isdir != (*b)->isdir)) {
    return (*a)->isdir ? -1 : 1;
  }
  if ((*a)->mtime == (*b)->mtime) {
    v = strcoll((*a)->name,(*b)->name);
    return reverse? -v : v;
  }
  v =  (*a)->mtime == (*b)->mtime? 0 : ((*a)->mtime < (*b)->mtime ? -1 : 1);
  return reverse? -v : v;
}

int ctimesort(struct _info **a, struct _info **b)
{
  int v;

  if (dirsfirst && ((*a)->isdir != (*b)->isdir)) {
    return (*a)->isdir ? -1 : 1;
  }
  if ((*a)->ctime == (*b)->ctime) {
    v = strcoll((*a)->name,(*b)->name);
    return reverse? -v : v;
  }
  v = (*a)->ctime == (*b)->ctime? 0 : ((*a)->ctime < (*b)->ctime? -1 : 1);
  return reverse? -v : v;
}

int sizecmp(off_t a, off_t b)
{
  return (a == b)? 0 : ((a < b)? 1 : -1);
}

int fsizesort(struct _info **a, struct _info **b)
{
  int v;

  if (dirsfirst && ((*a)->isdir != (*b)->isdir)) {
    return (*a)->isdir ? -1 : 1;
  }
  v = sizecmp((*a)->size, (*b)->size);
  if (v == 0) v = strcoll((*a)->name,(*b)->name);
  return reverse? -v : v;
}


void *xmalloc (size_t size)
{
  register void *value = malloc (size);
  if (value == 0) {
    fprintf(stderr,"tree: virtual memory exhausted.\n");
    exit(1);
  }
  return value;
}

void *xrealloc (void *ptr, size_t size)
{
  register void *value = realloc (ptr,size);
  if (value == 0) {
    fprintf(stderr,"tree: virtual memory exhausted.\n");
    exit(1);
  }
  return value;
}

void free_dir(struct _info **d)
{
  int i;
  
  for(i=0;d[i];i++) {
    free(d[i]->name);
    if (d[i]->lnk) free(d[i]->lnk);
    free(d[i]);
  }
  free(d);
}

char *gnu_getcwd()
{
  int size = 100;
  char *buffer = (char *) xmalloc (size);
     
  while (1)
    {
      char *value = getcwd (buffer, size);
      if (value != 0)
	return buffer;
      size *= 2;
      free (buffer);
      buffer = (char *) xmalloc (size);
    }
}

static inline char cond_lower(char c)
{
  return ignorecase ? tolower(c) : c;
}

/*
 * Patmatch() code courtesy of Thomas Moore (dark@mama.indstate.edu)
 * '|' support added by David MacMahon (davidm@astron.Berkeley.EDU)
 * Case insensitive support added by Jason A. Donenfeld (Jason@zx2c4.com)
 * returns:
 *    1 on a match
 *    0 on a mismatch
 *   -1 on a syntax error in the pattern
 */
int patmatch(char *buf, char *pat)
{
  int match = 1,m,n;
  char *bar = strchr(pat, '|');

  /* If a bar is found, call patmatch recursively on the two sub-patterns */

  if (bar) {
    /* If the bar is the first or last character, it's a syntax error */
    if (bar == pat || !bar[1]) {
      return -1;
    }
    /* Break pattern into two sub-patterns */
    *bar = '\0';
    match = patmatch(buf, pat);
    if (!match) {
      match = patmatch(buf,bar+1);
    }
    /* Join sub-patterns back into one pattern */
    *bar = '|';
    return match;
  }

  while(*pat && match) {
    switch(*pat) {
    case '[':
      pat++;
      if(*pat != '^') {
	n = 1;
	match = 0;
      } else {
	pat++;
	n = 0;
      }
      while(*pat != ']'){
	if(*pat == '\\') pat++;
	if(!*pat /* || *pat == '/' */ ) return -1;
	if(pat[1] == '-'){
	  m = *pat;
	  pat += 2;
	  if(*pat == '\\' && *pat)
	    pat++;
	  if(cond_lower(*buf) >= cond_lower(m) && cond_lower(*buf) <= cond_lower(*pat))
	    match = n;
	  if(!*pat)
	    pat--;
	} else if(cond_lower(*buf) == cond_lower(*pat)) match = n;
	pat++;
      }
      buf++;
      break;
    case '*':
      pat++;
      if(!*pat) return 1;
      while(*buf && !(match = patmatch(buf++,pat)));
      return match;
    case '?':
      if(!*buf) return 0;
      buf++;
      break;
    case '\\':
      if(*pat)
	pat++;
    default:
      match = (cond_lower(*buf++) == cond_lower(*pat));
      break;
    }
    pat++;
    if(match<1) return match;
  }
  if(!*buf) return match;
  return 0;
}


/**
 * They cried out for ANSI-lines (not really), but here they are, as an option
 * for the xterm and console capable among you, as a run-time option.
 */
void indent(int maxlevel)
{
  int i;

  if (ansilines) {
    if (dirs[0]) fprintf(outfile,"\033(0");
    for(i=0; (i <= maxlevel) && dirs[i]; i++) {
      if (dirs[i+1]) {
	if (dirs[i] == 1) fprintf(outfile,"\170   ");
	else printf("    ");
      } else {
	if (dirs[i] == 1) fprintf(outfile,"\164\161\161 ");
	else fprintf(outfile,"\155\161\161 ");
      }
    }
    if (dirs[0]) fprintf(outfile,"\033(B");
  } else {
    if (Hflag) fprintf(outfile,"\t");
    for(i=0; (i <= maxlevel) && dirs[i]; i++) {
      fprintf(outfile,"%s ",
	      dirs[i+1] ? (dirs[i]==1 ? linedraw->vert     : (Hflag? "&nbsp;&nbsp;&nbsp;" : "   ") )
			: (dirs[i]==1 ? linedraw->vert_left:linedraw->corner));
    }
  }
}


#ifdef __EMX__
char *prot(long m)
#else
char *prot(mode_t m)
#endif
{
#ifdef __EMX__
  const u_short *p;
  static char buf[6];
  char*cp;

  for(p=ifmt,cp=strcpy(buf,"adshr");*cp;++p,++cp)
    if(!(m&*p))
      *cp='-';
#else
  static char buf[11], perms[] = "rwxrwxrwx";
  int i, b;

  for(i=0;ifmt[i] && (m&S_IFMT) != ifmt[i];i++);
  buf[0] = fmt[i];

  /**
   * Nice, but maybe not so portable, it is should be no less portable than the
   * old code.
   */
  for(b=S_IRUSR,i=0; i<9; b>>=1,i++)
    buf[i+1] = (m & (b)) ? perms[i] : '-';
  if (m & S_ISUID) buf[3] = (buf[3]=='-')? 'S' : 's';
  if (m & S_ISGID) buf[6] = (buf[6]=='-')? 'S' : 's';
  if (m & S_ISVTX) buf[9] = (buf[9]=='-')? 'T' : 't';

  buf[10] = 0;
#endif
  return buf;
}

#define SIXMONTHS (6*31*24*60*60)

char *do_date(time_t t)
{
  static char buf[256];
  struct tm *tm;

  tm = localtime(&t);

  if (timefmt) {
    strftime(buf,255,timefmt,tm);
    buf[255] = 0;
  } else {
    time_t c = time(0);
    /* Use strftime() so that locale is respected: */
    if (t > c || (t+SIXMONTHS) < c)
      strftime(buf,255,"%b %e  %Y",tm);
    else
      strftime(buf,255,"%b %e %R", tm);
  }
  return buf;
}

/**
 * Must fix this someday
 */
void printit(char *s)
{
  int c;

  if (Nflag) {
    if (Qflag) fprintf(outfile, "\"%s\"",s);
    else fprintf(outfile,"%s",s);
    return;
  }
  if (mb_cur_max > 1) {
    wchar_t *ws, *tp;
    ws = xmalloc(sizeof(wchar_t)* (c=(strlen(s)+1)));
    if (mbstowcs(ws,s,c) != (size_t)-1) {
      if (Qflag) putc('"',outfile);
      for(tp=ws;*tp && c > 1;tp++, c--) {
	if (iswprint(*tp)) fprintf(outfile,"%lc",(wint_t)*tp);
	else {
	  if (qflag) putc('?',outfile);
	  else fprintf(outfile,"\\%03o",(unsigned int)*tp);
	}
      }
      if (Qflag) putc('"',outfile);
      free(ws);
      return;
    }
    free(ws);
  }
  if (Qflag) putc('"',outfile);
  for(;*s;s++) {
    c = (unsigned char)*s;
#ifdef __EMX__
    if(_nls_is_dbcs_lead(*(unsigned char*)s)){
      putc(*s,outfile);
      putc(*++s,outfile);
      continue;
    }
#endif
    if((c >= 7 && c <= 13) || c == '\\' || (c == '"' && Qflag) || (c == ' ' && !Qflag)) {
      putc('\\',outfile);
      if (c > 13) putc(c, outfile);
      else putc("abtnvfr"[c-7], outfile);
    } else if (isprint(c)) putc(c,outfile);
    else {
      if (qflag) {
	if (mb_cur_max > 1 && c > 127) putc(c,outfile);
	else putc('?',outfile);
      } else fprintf(outfile,"\\%03o",c);
    }
  }
  if (Qflag) putc('"',outfile);
}

int psize(char *buf, off_t size)
{
  static char *iec_unit="BKMGTPEZY", *si_unit = "dkMGTPEZY";
  char *unit = siflag ? si_unit : iec_unit;
  int idx, usize = siflag ? 1000 : 1024;

  if (hflag || siflag) {
    for (idx=size<usize?0:1; size >= (usize*usize); idx++,size/=usize);
    if (!idx) return sprintf(buf, " %4d", (int)size);
    else return sprintf(buf, ((size/usize) >= 10)? " %3.0f%c" : " %3.1f%c" , (float)size/(float)usize,unit[idx]);
  } else return sprintf(buf, sizeof(off_t) == sizeof(long long)? " %11lld" : " %9lld", (long long int)size);
}

char Ftype(mode_t mode)
{
  int m = mode & S_IFMT;
  if (!dflag && m == S_IFDIR) return '/';
  else if (m == S_IFSOCK) return '=';
  else if (m == S_IFIFO) return '|';
  else if (m == S_IFLNK) return '@'; /* Here, but never actually used though. */
#ifdef S_IFDOOR
  else if (m == S_IFDOOR) return '>';
#endif
  else if ((m == S_IFREG) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH))) return '*';
  return 0;
}

char *fillinfo(char *buf, struct _info *ent)
{
  int n;
  buf[n=0] = 0;
  #ifdef __USE_FILE_OFFSET64
  if (inodeflag) n += sprintf(buf," %7lld",(long long)ent->linode);
  #else
  if (inodeflag) n += sprintf(buf," %7ld",(long int)ent->linode);
  #endif
  if (devflag) n += sprintf(buf+n, " %3d", (int)ent->ldev);
  #ifdef __EMX__
  if (pflag) n += sprintf(buf+n, " %s",prot(ent->attr));
  #else
  if (pflag) n += sprintf(buf+n, " %s", prot(ent->mode));
  #endif
  if (uflag) n += sprintf(buf+n, " %-8.32s", uidtoname(ent->uid));
  if (gflag) n += sprintf(buf+n, " %-8.32s", gidtoname(ent->gid));
  if (sflag) n += psize(buf+n,ent->size);
  if (Dflag) n += sprintf(buf+n, " %s", do_date(cflag? ent->ctime : ent->mtime));
  
  return buf;
}
