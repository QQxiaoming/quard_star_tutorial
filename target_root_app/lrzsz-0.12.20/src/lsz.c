/*
  lsz - send files with x/y/zmodem
  Copyright (C) until 1988 Chuck Forsberg (Omen Technology INC)
  Copyright (C) 1994 Matt Porter, Michael D. Black
  Copyright (C) 1996, 1997 Uwe Ohse

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.

  originally written by Chuck Forsberg
*/
#include "zglobal.h"

/* char *getenv(); */

#define SS_NORMAL 0
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>

#ifndef R_OK
#  define R_OK 4
#endif

#if defined(HAVE_SYS_MMAN_H) && defined(HAVE_MMAP)
#  include <sys/mman.h>
size_t mm_size;
void *mm_addr=NULL;
#else
#  undef HAVE_MMAP
#endif
#include "timing.h"
#include "long-options.h"
#include "xstrtoul.h"
#include "error.h"

#ifndef STRICT_PROTOTYPES
extern time_t time();
extern char *strerror();
extern char *strstr();
#endif

#ifndef HAVE_ERRNO_DECLARATION
extern int errno;
#endif

unsigned Baudrate=2400;	/* Default, should be set by first mode() call */
unsigned Txwindow;	/* Control the size of the transmitted window */
unsigned Txwspac;	/* Spacing between zcrcq requests */
unsigned Txwcnt;	/* Counter used to space ack requests */
size_t Lrxpos;		/* Receiver's last reported offset */
int errors;
enum zm_type_enum protocol;
int under_rsh=FALSE;
extern int turbo_escape;
static int no_unixmode;

int Canseek=1; /* 1: can; 0: only rewind, -1: neither */

static int zsendfile __P ((struct zm_fileinfo *zi, const char *buf, size_t blen));
static int getnak __P ((void));
static int wctxpn __P ((struct zm_fileinfo *));
static int wcs __P ((const char *oname, const char *remotename));
static size_t zfilbuf __P ((struct zm_fileinfo *zi));
static size_t filbuf __P ((char *buf, size_t count));
static int getzrxinit __P ((void));
static int calc_blklen __P ((long total_sent));
static int sendzsinit __P ((void));
static int wctx __P ((struct zm_fileinfo *));
static int zsendfdata __P ((struct zm_fileinfo *));
static int getinsync __P ((struct zm_fileinfo *, int flag));
static void countem __P ((int argc, char **argv));
static void chkinvok __P ((const char *s));
static void usage __P ((int exitcode, const char *what));
static int zsendcmd __P ((const char *buf, size_t blen));
static void saybibi __P ((void));
static int wcsend __P ((int argc, char *argp[]));
static int wcputsec __P ((char *buf, int sectnum, size_t cseclen));
static void usage1 __P ((int exitcode));

#ifdef ENABLE_SYSLOG
#define DO_SYSLOG(message) do { \
    if (enable_syslog) { \
        const char *shortname; \
        if (!zi->fname) \
            shortname="no.name"; \
		else { \
            shortname=strrchr(zi->fname,'/'); \
            if (!shortname) \
                shortname=zi->fname; \
            else \
                shortname++; \
		} \
        lsyslog message ; \
	 } \
  } while(0)
#else
#define DO_SYSLOG(message) do { } while(0)
#endif

#define ZSDATA(x,y,z) \
	do { if (Crc32t) {zsda32(x,y,z); } else {zsdata(x,y,z);}} while(0)
#ifdef HAVE_MMAP
#define DATAADR (mm_addr ? ((char *)mm_addr)+zi->bytes_sent : txbuf)
#else
#define DATAADR (txbuf)
#endif

int Filesleft;
long Totalleft;
size_t buffersize=16384;
#ifdef HAVE_MMAP
int use_mmap=1;
#endif

/*
 * Attention string to be executed by receiver to interrupt streaming data
 *  when an error is detected.  A pause (0336) may be needed before the
 *  ^C (03) or after it.
 */
#ifdef READCHECK
char Myattn[] = { 0 };
#else
char Myattn[] = { 03, 0336, 0 };
#endif

FILE *input_f;

#define MAX_BLOCK 8192
char txbuf[MAX_BLOCK];

long vpos = 0;			/* Number of bytes read from file */

char Lastrx;
char Crcflg;
int Verbose=0;
int Restricted=0;	/* restricted; no /.. or ../ in filenames */
int Quiet=0;		/* overrides logic that would otherwise set verbose */
int Ascii=0;		/* Add CR's for brain damaged programs */
int Fullname=0;		/* transmit full pathname */
int Unlinkafter=0;	/* Unlink file after it is sent */
int Dottoslash=0;	/* Change foo.bar.baz to foo/bar/baz */
int firstsec;
int errcnt=0;		/* number of files unreadable */
size_t blklen=128;		/* length of transmitted records */
int Optiong;		/* Let it rip no wait for sector ACK's */
int Totsecs;		/* total number of sectors this file */
int Filcnt=0;		/* count of number of files opened */
int Lfseen=0;
unsigned Rxbuflen = 16384;	/* Receiver's max buffer length */
unsigned Tframlen = 0;	/* Override for tx frame length */
unsigned blkopt=0;		/* Override value for zmodem blklen */
int Rxflags = 0;
int Rxflags2 = 0;
size_t bytcnt;
int Wantfcs32 = TRUE;	/* want to send 32 bit FCS */
char Lzconv;	/* Local ZMODEM file conversion request */
char Lzmanag;	/* Local ZMODEM file management request */
int Lskipnocor;
char Lztrans;
char zconv;		/* ZMODEM file conversion request */
char zmanag;		/* ZMODEM file management request */
char ztrans;		/* ZMODEM file transport request */
int command_mode;		/* Send a command, then exit. */
int Cmdtries = 11;
int Cmdack1;		/* Rx ACKs command, then do it */
int Exitcode;
int enable_timesync=0;
size_t Lastsync;		/* Last offset to which we got a ZRPOS */
int Beenhereb4;		/* How many times we've been ZRPOS'd same place */

int no_timeout=FALSE;
size_t max_blklen=1024;
size_t start_blklen=0;
int zmodem_requested;
time_t stop_time=0;
int tcp_flag=0;
char *tcp_server_address=0;
int tcp_socket=-1;

int error_count;
#define OVERHEAD 18
#define OVER_ERR 20

#define MK_STRING(x) #x

#ifdef ENABLE_SYSLOG
#  if defined(ENABLE_SYSLOG_FORCE) || defined(ENABLE_SYSLOG_DEFAULT)
int enable_syslog=TRUE;
#  else
int enable_syslog=FALSE;
#  endif
#endif

jmp_buf intrjmp;	/* For the interrupt on RX CAN */

static long min_bps;
static long min_bps_time;

static int io_mode_fd=0;
static int zrqinits_sent=0;
static int play_with_sigint=0;

/* called by signal interrupt or terminate to clean things up */
RETSIGTYPE
bibi (int n)
{
	canit(STDOUT_FILENO);
	fflush (stdout);
	io_mode (io_mode_fd,0);
	if (n == 99)
		error (0, 0, _ ("io_mode(,2) in rbsb.c not implemented\n"));
	else
		error (0, 0, _ ("caught signal %d; exiting"), n);
	if (n == SIGQUIT)
		abort ();
	exit (128 + n);
}

/* Called when ZMODEM gets an interrupt (^C) */
static RETSIGTYPE
onintr(int n LRZSZ_ATTRIB_UNUSED)
{
	signal(SIGINT, SIG_IGN);
	longjmp(intrjmp, -1);
}

int Zctlesc;	/* Encode control characters */
const char *program_name = "sz";
int Zrwindow = 1400;	/* RX window size (controls garbage count) */

static struct option const long_options[] =
{
  {"append", no_argument, NULL, '+'},
  {"twostop", no_argument, NULL, '2'},
  {"try-8k", no_argument, NULL, '8'},
  {"start-8k", no_argument, NULL, '9'},
  {"try-4k", no_argument, NULL, '4'},
  {"start-4k", no_argument, NULL, '5'},
  {"ascii", no_argument, NULL, 'a'},
  {"binary", no_argument, NULL, 'b'},
  {"bufsize", required_argument, NULL, 'B'},
  {"cmdtries", required_argument, NULL, 'C'},
  {"command", required_argument, NULL, 'c'},
  {"immediate-command", required_argument, NULL, 'i'},
  {"dot-to-slash", no_argument, NULL, 'd'},
  {"full-path", no_argument, NULL, 'f'},
  {"escape", no_argument, NULL, 'e'},
  {"rename", no_argument, NULL, 'E'},
  {"help", no_argument, NULL, 'h'},
  {"crc-check", no_argument, NULL, 'H'},
  {"1024", no_argument, NULL, 'k'},
  {"1k", no_argument, NULL, 'k'},
  {"packetlen", required_argument, NULL, 'L'},
  {"framelen", required_argument, NULL, 'l'},
  {"min-bps", required_argument, NULL, 'm'},
  {"min-bps-time", required_argument, NULL, 'M'},
  {"newer", no_argument, NULL, 'n'},
  {"newer-or-longer", no_argument, NULL, 'N'},
  {"16-bit-crc", no_argument, NULL, 'o'},
  {"disable-timeouts", no_argument, NULL, 'O'},
  {"disable-timeout", no_argument, NULL, 'O'}, /* i can't get it right */
  {"protect", no_argument, NULL, 'p'},
  {"resume", no_argument, NULL, 'r'},
  {"restricted", no_argument, NULL, 'R'},
  {"quiet", no_argument, NULL, 'q'},
  {"stop-at", required_argument, NULL, 's'},
  {"syslog", optional_argument, NULL , 2},
  {"timesync", no_argument, NULL, 'S'},
  {"timeout", required_argument, NULL, 't'},
  {"turbo", no_argument, NULL, 'T'},
  {"unlink", no_argument, NULL, 'u'},
  {"unrestrict", no_argument, NULL, 'U'},
  {"verbose", no_argument, NULL, 'v'},
  {"windowsize", required_argument, NULL, 'w'},
  {"xmodem", no_argument, NULL, 'X'},
  {"ymodem", no_argument, NULL, 1},
  {"zmodem", no_argument, NULL, 'Z'},
  {"overwrite", no_argument, NULL, 'y'},
  {"overwrite-or-skip", no_argument, NULL, 'Y'},

  {"delay-startup", required_argument, NULL, 4},
  {"tcp", no_argument, NULL, 5},
  {"tcp-server", no_argument, NULL, 6},
  {"tcp-client", required_argument, NULL, 7},
  {"no-unixmode", no_argument, NULL, 8},
  {NULL, 0, NULL, 0}
};

static void
show_version(void)
{
	printf ("%s (%s) %s\n", program_name, PACKAGE, VERSION);
}


int 
main(int argc, char **argv)
{
	char *cp;
	int npats;
	int dm;
	int i;
	int stdin_files;
	char **patts;
	int c;
	const char *Cmdstr=NULL;		/* Pointer to the command string */
	unsigned int startup_delay=0;

	if (((cp = getenv("ZNULLS")) != NULL) && *cp)
		Znulls = atoi(cp);
	if (((cp=getenv("SHELL"))!=NULL) && (strstr(cp, "rsh") || strstr(cp, "rksh")
		|| strstr(cp, "rbash") || strstr(cp,"rshell")))
	{
		under_rsh=TRUE;
		Restricted=1;
	}
	if ((cp=getenv("ZMODEM_RESTRICTED"))!=NULL)
		Restricted=1;
	from_cu();
	chkinvok(argv[0]);

#ifdef ENABLE_SYSLOG
	openlog(program_name,LOG_PID,ENABLE_SYSLOG);
#endif

	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

	parse_long_options (argc, argv, show_version, usage1);

	Rxtimeout = 600;

	while ((c = getopt_long (argc, argv, 
		"2+48abB:C:c:dfeEghHi:kL:l:m:M:NnOopRrqsSt:TUuvw:XYy",
		long_options, (int *) 0))!=EOF)
	{
		unsigned long int tmp;
		char *tmpptr;
		enum strtol_error s_err;

		switch (c)
		{
		case 0:
			break;
		case '+': Lzmanag = ZF1_ZMAPND; break;
		case '2': Twostop = TRUE; break;
		case '8':
			if (max_blklen==8192)
				start_blklen=8192;
			else
				max_blklen=8192;
			break;
		case '9': /* this is a longopt .. */
			start_blklen=8192;
			max_blklen=8192;
			break;
		case '4':
			if (max_blklen==4096)
				start_blklen=4096;
			else
				max_blklen=4096;
			break;
		case '5': /* this is a longopt .. */
			start_blklen=4096;
			max_blklen=4096;
			break;
		case 'a': Lzconv = ZCNL; Ascii = TRUE; break;
		case 'b': Lzconv = ZCBIN; break;
		case 'B':
			if (0==strcmp(optarg,"auto"))
				buffersize= (size_t) -1;
			else
				buffersize=strtol(optarg,NULL,10);
#ifdef HAVE_MMAP
			use_mmap=0;
#endif
			break;
		case 'C': 
			s_err = xstrtoul (optarg, NULL, 0, &tmp, NULL);
			Cmdtries = tmp;
			if (s_err != LONGINT_OK)
				STRTOL_FATAL_ERROR (optarg, _("command tries"), s_err);
			break;
		case 'i':
			Cmdack1 = ZCACK1;
			/* **** FALL THROUGH TO **** */
		case 'c':
			command_mode = TRUE;
			Cmdstr = optarg;
			break;
		case 'd':
			++Dottoslash;
			/* **** FALL THROUGH TO **** */
		case 'f': Fullname=TRUE; break;
		case 'e': Zctlesc = 1; break;
		case 'E': Lzmanag = ZF1_ZMCHNG; break;
		case 'h': usage(0,NULL); break;
		case 'H': Lzmanag = ZF1_ZMCRC; break;
		case 'k': start_blklen=1024; break;
		case 'L':
			s_err = xstrtoul (optarg, NULL, 0, &tmp, "ck");
			blkopt = tmp;
			if (s_err != LONGINT_OK)
				STRTOL_FATAL_ERROR (optarg, _("packetlength"), s_err);
			if (blkopt<24 || blkopt>MAX_BLOCK)
			{
				char meld[256];
				sprintf(meld,
					_("packetlength out of range 24..%ld"),
					(long) MAX_BLOCK);
				usage(2,meld);
			}
			break;
		case 'l':
			s_err = xstrtoul (optarg, NULL, 0, &tmp, "ck");
			Tframlen = tmp;
			if (s_err != LONGINT_OK)
				STRTOL_FATAL_ERROR (optarg, _("framelength"), s_err);
			if (Tframlen<32 || Tframlen>MAX_BLOCK)
			{
				char meld[256];
				sprintf(meld,
					_("framelength out of range 32..%ld"),
					(long) MAX_BLOCK);
				usage(2,meld);
			}
			break;
        case 'm':
			s_err = xstrtoul (optarg, &tmpptr, 0, &tmp, "km");
			min_bps = tmp;
			if (s_err != LONGINT_OK)
				STRTOL_FATAL_ERROR (optarg, _("min_bps"), s_err);
			if (min_bps<0)
				usage(2,_("min_bps must be >= 0"));
			break;
        case 'M':
			s_err = xstrtoul (optarg, NULL, 0, &tmp, NULL);
			min_bps_time = tmp;
			if (s_err != LONGINT_OK)
				STRTOL_FATAL_ERROR (optarg, _("min_bps_time"), s_err);
			if (min_bps_time<=1)
				usage(2,_("min_bps_time must be > 1"));
			break;
		case 'N': Lzmanag = ZF1_ZMNEWL;  break;
		case 'n': Lzmanag = ZF1_ZMNEW;  break;
		case 'o': Wantfcs32 = FALSE; break;
		case 'O': no_timeout = TRUE; break;
		case 'p': Lzmanag = ZF1_ZMPROT;  break;
		case 'r': 
			if (Lzconv == ZCRESUM) 
				Lzmanag = ZF1_ZMCRC;
			else
				Lzconv = ZCRESUM; 
			break;
		case 'R': Restricted = TRUE; break;
		case 'q': Quiet=TRUE; Verbose=0; break;
		case 's':
			if (isdigit((unsigned char) (*optarg))) {
				struct tm *tm;
				time_t t;
				int hh,mm;
				char *nex;

				hh = strtoul (optarg, &nex, 10);
				if (hh>23)
					usage(2,_("hour to large (0..23)"));
				if (*nex!=':')
					usage(2, _("unparsable stop time\n"));
				nex++;
				mm = strtoul (optarg, &nex, 10);
				if (mm>59)
					usage(2,_("minute to large (0..59)"));

				t=time(NULL);
				tm=localtime(&t);
				tm->tm_hour=hh;
				tm->tm_min=hh;
				stop_time=mktime(tm);
				if (stop_time<t)
					stop_time+=86400L; /* one day more */
				if (stop_time - t <10) 
					usage(2,_("stop time to small"));
			} else {
				s_err = xstrtoul (optarg, NULL, 0, &tmp, NULL);
				stop_time = tmp + time(0);
				if (s_err != LONGINT_OK)
					STRTOL_FATAL_ERROR (optarg, _("stop-at"), s_err);
				if (tmp<10)
					usage(2,_("stop time to small"));
			}
			break;
		case 'S': enable_timesync=1; break;
		case 'T': turbo_escape=1; break;
		case 't':
			s_err = xstrtoul (optarg, NULL, 0, &tmp, NULL);
			Rxtimeout = tmp;
			if (s_err != LONGINT_OK)
				STRTOL_FATAL_ERROR (optarg, _("timeout"), s_err);
			if (Rxtimeout<10 || Rxtimeout>1000)
				usage(2,_("timeout out of range 10..1000"));
			break;
		case 'u': ++Unlinkafter; break;
		case 'U':
			if (!under_rsh)
				Restricted=0;
			else
				error(1,0,
		_("security violation: can't do that under restricted shell\n"));
			break;
		case 'v': ++Verbose; break;
		case 'w':
			s_err = xstrtoul (optarg, NULL, 0, &tmp, NULL);
			Txwindow = tmp;
			if (s_err != LONGINT_OK)
				STRTOL_FATAL_ERROR (optarg, _("window size"), s_err);
			if (Txwindow < 256)
				Txwindow = 256;
			Txwindow = (Txwindow/64) * 64;
			Txwspac = Txwindow/4;
			if (blkopt > Txwspac
			 || (!blkopt && Txwspac < MAX_BLOCK))
				blkopt = Txwspac;
			break;
		case 'X': protocol=ZM_XMODEM; break;
		case 1:   protocol=ZM_YMODEM; break;
		case 'Z': protocol=ZM_ZMODEM; break;
		case 'Y':
			Lskipnocor = TRUE;
			/* **** FALLL THROUGH TO **** */
		case 'y':
			Lzmanag = ZF1_ZMCLOB; break;
		case 2:
#ifdef ENABLE_SYSLOG
#  ifndef ENABLE_SYSLOG_FORCE
			if (optarg && (!strcmp(optarg,"off") || !strcmp(optarg,"no")))
			{
				if (under_rsh)
					error(0,0, _("cannot turnoff syslog"));
				else
					enable_syslog=FALSE;
			}
			else
				enable_syslog=TRUE;
#  else
			error(0,0, _("cannot turnoff syslog"));
#  endif
#endif
			break;
		case 4:
			s_err = xstrtoul (optarg, NULL, 0, &tmp, NULL);
			startup_delay = tmp;
			if (s_err != LONGINT_OK)
				STRTOL_FATAL_ERROR (optarg, _("startup delay"), s_err);
			break;
		case 5:
			tcp_flag=1;
			break;
		case 6:
			tcp_flag=2;
			break;
		case 7:
			tcp_flag=3;
			tcp_server_address=(char *)strdup(optarg);
			if (!tcp_server_address) {
				error(1,0,_("out of memory"));
			}
			break;
		case 8: no_unixmode=1; break;
		default:
			usage (2,NULL);
			break;
		}
	}

	if (getuid()!=geteuid()) {
		error(1,0,
		_("this program was never intended to be used setuid\n"));
	}
	zsendline_init();

	if (start_blklen==0) {
		if (protocol == ZM_ZMODEM) {
			start_blklen=1024;
			if (Tframlen) {
				start_blklen=max_blklen=Tframlen;
			}
		}
		else
			start_blklen=128;
	}

	if (argc<2)
		usage(2,_("need at least one file to send"));

	if (startup_delay)
		sleep(startup_delay);

#ifdef HAVE_SIGINTERRUPT
	/* we want interrupted system calls to fail and not to be restarted. */
	siginterrupt(SIGALRM,1);
#endif


	npats = argc - optind;
	patts=&argv[optind];

	if (npats < 1 && !command_mode) 
		usage(2,_("need at least one file to send"));
	if (command_mode && Restricted) {
		printf(_("Can't send command in restricted mode\n"));
		exit(1);
	}

	if (Fromcu && !Quiet) {
		if (Verbose == 0)
			Verbose = 2;
	}
	vfile("%s %s\n", program_name, VERSION);

	if (tcp_flag==2) {
		char buf[256];
#ifdef MAXHOSTNAMELEN
		char hn[MAXHOSTNAMELEN];
#else
		char hn[256];
#endif
		char *p,*q;
		int d;

		/* tell receiver to receive via tcp */
		d=tcp_server(buf);
		p=strchr(buf+1,'<');
		p++;
		q=strchr(p,'>');
		*q=0;
		if (gethostname(hn,sizeof(hn))==-1) {
			error(1,0, _("hostname too long\n"));
		}
		fprintf(stdout,"connect with lrz --tcp-client \"%s:%s\"\n",hn,p);
		fflush(stdout);
		/* ok, now that this file is sent we can switch to tcp */

		tcp_socket=tcp_accept(d);
		dup2(tcp_socket,0);
		dup2(tcp_socket,1);
	}
	if (tcp_flag==3) {
		char buf[256];
		char *p;
		p=strchr(tcp_server_address,':');
		if (!p)
			error(1,0, _("illegal server address\n"));
		*p++=0;
		sprintf(buf,"[%s] <%s>\n",tcp_server_address,p);

		fprintf(stdout,"connecting to %s\n",buf);
		fflush(stdout);

		/* we need to switch to tcp mode */
		tcp_socket=tcp_connect(buf);
		dup2(tcp_socket,0);
		dup2(tcp_socket,1);
	}


	{
		/* we write max_blocklen (data) + 18 (ZModem protocol overhead)
		 * + escape overhead (about 4 %), so buffer has to be
		 * somewhat larger than max_blklen 
		 */
		char *s=malloc(max_blklen+1024);
		if (!s)
		{
			zperr(_("out of memory"));
			exit(1);
		}
#ifdef SETVBUF_REVERSED
		setvbuf(stdout,_IOFBF,s,max_blklen+1024);
#else
		setvbuf(stdout,s,_IOFBF,max_blklen+1024);
#endif
	}
	blklen=start_blklen;

	for (i=optind,stdin_files=0;i<argc;i++) {
		if (0==strcmp(argv[i],"-"))
			stdin_files++;
	}

	if (stdin_files>1) {
		usage(1,_("can read only one file from stdin"));
	} else if (stdin_files==1) {
		io_mode_fd=1;
	}
	io_mode(io_mode_fd,1);
	readline_setup(io_mode_fd, 128, 256);

	if (signal(SIGINT, bibi) == SIG_IGN)
		signal(SIGINT, SIG_IGN);
	else {
		signal(SIGINT, bibi); 
		play_with_sigint=1;
	}
	signal(SIGTERM, bibi);
	signal(SIGPIPE, bibi);
	signal(SIGHUP, bibi);

	if ( protocol!=ZM_XMODEM) {
		if (protocol==ZM_ZMODEM) {
			printf("rz\r");
			fflush(stdout);
		}
		countem(npats, patts);
		if (protocol == ZM_ZMODEM) {
			/* throw away any input already received. This doesn't harm
			 * as we invite the receiver to send it's data again, and
			 * might be useful if the receiver has already died or
			 * if there is dirt left if the line 
			 */
#ifdef HAVE_SELECT
			struct timeval t;
			unsigned char throwaway;
			fd_set f;
#endif

			purgeline(io_mode_fd);
				
#ifdef HAVE_SELECT
			t.tv_sec = 0;
			t.tv_usec = 0;
				
			FD_ZERO(&f);
			FD_SET(io_mode_fd,&f);
				
			while (select(1,&f,NULL,NULL,&t)) {
				if (0==read(io_mode_fd,&throwaway,1)) /* EOF ... */
					break;
			}
#endif

			purgeline(io_mode_fd);
			stohdr(0L);
			if (command_mode)
				Txhdr[ZF0] = ZCOMMAND;
			zshhdr(ZRQINIT, Txhdr);
			zrqinits_sent++;
#if defined(ENABLE_TIMESYNC)
			if (Rxflags2 != ZF1_TIMESYNC)
				/* disable timesync if there are any flags we don't know.
				 * dsz/gsz seems to use some other flags! */
				enable_timesync=FALSE;
			if (Rxflags2 & ZF1_TIMESYNC && enable_timesync) {
				Totalleft+=6; /* TIMESYNC never needs more */
				Filesleft++;
			}
#endif
			if (tcp_flag==1) {
				Totalleft+=256; /* tcp never needs more */
				Filesleft++;
			}
		}
	}
	fflush(stdout);

	if (Cmdstr) {
		if (getzrxinit()) {
			Exitcode=0200; canit(STDOUT_FILENO);
		}
		else if (zsendcmd(Cmdstr, strlen(Cmdstr)+1)) {
			Exitcode=0200; canit(STDOUT_FILENO);
		}
	} else if (wcsend(npats, patts)==ERROR) {
		Exitcode=0200;
		canit(STDOUT_FILENO);
	}
	fflush(stdout);
	io_mode(io_mode_fd,0);
	if (Exitcode)
		dm=Exitcode;
	else if (errcnt)
		dm=1;
	else
		dm=0;
	if (Verbose)
	{
		fputs("\r\n",stderr);
		if (dm)
			fputs(_("Transfer incomplete\n"),stderr);
		else
			fputs(_("Transfer complete\n"),stderr);
	}
	exit(dm);
	/*NOTREACHED*/
}

static int 
send_pseudo(const char *name, const char *data)
{
	char *tmp;
	const char *p;
	int ret=0; /* ok */
	size_t plen;
	int fd;
	int lfd;
	
	p = getenv ("TMPDIR");
	if (!p)
		p = getenv ("TMP");
	if (!p)
		p = "/tmp";
	tmp=malloc(PATH_MAX+1);
	if (!tmp)
		error(1,0,_("out of memory"));
	
	plen=strlen(p);
	memcpy(tmp,p,plen);	
	tmp[plen++]='/';

	lfd=0;
	do {
		if (lfd++==10) {
			free(tmp);
			vstringf (_ ("send_pseudo %s: cannot open tmpfile %s: %s"),
					 name, tmp, strerror (errno));
			vstring ("\r\n");
			return 1;
		}
		sprintf(tmp+plen,"%s.%lu.%d",name,(unsigned long) getpid(),lfd);
		fd=open(tmp,O_WRONLY|O_CREAT|O_EXCL,0700);
		/* is O_EXCL guaranted to not follow symlinks? 
		 * I don`t know ... so be careful
		 */
		if (fd!=-1) {
			struct stat st;
			if (0!=lstat(tmp,&st)) {
				vstringf (_ ("send_pseudo %s: cannot lstat tmpfile %s: %s"),
						 name, tmp, strerror (errno));
				vstring ("\r\n");
				unlink(tmp);
				close(fd);
				fd=-1;
			} else {
				if (S_ISLNK(st.st_mode)) {
					vstringf (_ ("send_pseudo %s: avoiding symlink trap"),name);
					vstring ("\r\n");
					unlink(tmp);
					close(fd);
					fd=-1;
				}
			}
		}
	} while (fd==-1);
	if (write(fd,data,strlen(data))!=(signed long) strlen(data)
		|| close(fd)!=0) {
		vstringf (_ ("send_pseudo %s: cannot write to tmpfile %s: %s"),
				 name, tmp, strerror (errno));
		vstring ("\r\n");
		free(tmp);
		return 1;
	}

	if (wcs (tmp,name) == ERROR) {
		if (Verbose)
			vstringf (_ ("send_pseudo %s: failed"),name);
		else {
			if (Verbose)
				vstringf (_ ("send_pseudo %s: ok"),name);
			Filcnt--;
		}
		vstring ("\r\n");
		ret=1;
	}
	unlink (tmp);
	free(tmp);
	return ret;
}

static int
wcsend (int argc, char *argp[])
{
	int n;

	Crcflg = FALSE;
	firstsec = TRUE;
	bytcnt = (size_t) -1;

	if (tcp_flag==1) {
		char buf[256];
		int d;

		/* tell receiver to receive via tcp */
		d=tcp_server(buf);
		if (send_pseudo("/$tcp$.t",buf)) {
			error(1,0,_("tcp protocol init failed\n"));
		}
		/* ok, now that this file is sent we can switch to tcp */

		tcp_socket=tcp_accept(d);
		dup2(tcp_socket,0);
		dup2(tcp_socket,1);
	}

	for (n = 0; n < argc; ++n) {
		Totsecs = 0;
		if (wcs (argp[n],NULL) == ERROR)
			return ERROR;
	}
#if defined(ENABLE_TIMESYNC)
	if (Rxflags2 & ZF1_TIMESYNC && enable_timesync) {
		/* implement Peter Mandrellas extension */
		char buf[60];
		time_t t = time (NULL);
		struct tm *tm = localtime (&t);		/* sets timezone */
		strftime (buf, sizeof (buf) - 1, "%H:%M:%S", tm);
		if (Verbose) {
			vstring ("\r\n");
			vstringf (_("Answering TIMESYNC at %s"),buf);
		}
#if defined(HAVE_TIMEZONE_VAR)
		sprintf(buf+strlen(buf),"%ld\r\n", timezone / 60);
		if (Verbose)
			vstringf (" (%s %ld)\r\n", _ ("timezone"), timezone / 60);
#else
		if (Verbose)
			vstringf (" (%s)\r\n", _ ("timezone unknown"));
#endif
		send_pseudo("/$time$.t",buf);
	}
#endif
	Totsecs = 0;
	if (Filcnt == 0) {			/* bitch if we couldn't open ANY files */
#if 0
	/* i *really* do not like this */
		if (protocol != ZM_XMODEM) {
			const char *Cmdstr;		/* Pointer to the command string */
			command_mode = TRUE;
			Cmdstr = "echo \"lsz: Can't open any requested files\"";
			if (getnak ()) {
				Exitcode = 0200;
				canit(STDOUT_FILENO);
			}
			if (!zmodem_requested)
				canit(STDOUT_FILENO);
			else if (zsendcmd (Cmdstr, 1 + strlen (Cmdstr))) {
				Exitcode = 0200;
				canit(STDOUT_FILENO);
			}
			Exitcode = 1;
			return OK;
		}
#endif
		canit(STDOUT_FILENO);
		vstring ("\r\n");
		vstringf (_ ("Can't open any requested files."));
		vstring ("\r\n");
		return ERROR;
	}
	if (zmodem_requested)
		saybibi ();
	else if (protocol != ZM_XMODEM) {
		struct zm_fileinfo zi;
		char *pa;
		pa=alloca(PATH_MAX+1);
		*pa='\0';
		zi.fname = pa;
		zi.modtime = 0;
		zi.mode = 0;
		zi.bytes_total = 0;
		zi.bytes_sent = 0;
		zi.bytes_received = 0;
		zi.bytes_skipped = 0;
		wctxpn (&zi);
	}
	return OK;
}

static int
wcs(const char *oname, const char *remotename)
{
#if !defined(S_ISDIR)
	int c;
#endif
	struct stat f;
	char *name;
	struct zm_fileinfo zi;
#ifdef HAVE_MMAP
	int dont_mmap_this=0;
#endif
#ifdef ENABLE_SYSLOG
	const char *shortname;
	shortname=strrchr(oname,'/');
	if (shortname)
		shortname++;
	else
		shortname=oname;
#endif


	if (Restricted) {
		/* restrict pathnames to current tree or uucppublic */
		if ( strstr(oname, "../")
#ifdef PUBDIR
		 || (oname[0]== '/' && strncmp(oname, MK_STRING(PUBDIR),
		 	strlen(MK_STRING(PUBDIR))))
#endif
		) {
			canit(STDOUT_FILENO);
			vchar('\r');
			error(1,0,
				_("security violation: not allowed to upload from %s"),oname);
		}
	}
	
	if (0==strcmp(oname,"-")) {
		char *p=getenv("ONAME");
		name=alloca(PATH_MAX+1);
		if (p) {
			strcpy(name, p);
		} else {
			sprintf(name, "s%lu.lsz", (unsigned long) getpid());
		}
		input_f=stdin;
#ifdef HAVE_MMAP
		dont_mmap_this=1;
#endif
	} else if ((input_f=fopen(oname, "r"))==NULL) {
		int e=errno;
		error(0,e, _("cannot open %s"),oname);
		++errcnt;
		return OK;	/* pass over it, there may be others */
	} else {
		name=alloca(PATH_MAX+1);
		strcpy(name, oname);
	}
#ifdef HAVE_MMAP
	if (!use_mmap || dont_mmap_this)
#endif
	{
		static char *s=NULL;
		static size_t last_length=0;
		struct stat st;
		if (fstat(fileno(input_f),&st)==-1)
			st.st_size=1024*1024;
		if (buffersize==(size_t) -1 && s) {
			if ((size_t) st.st_size > last_length) {
				free(s);
				s=NULL;
				last_length=0;
			}
		}
		if (!s && buffersize) {
			last_length=16384;
			if (buffersize==(size_t) -1) {
				if (st.st_size>0)
					last_length=st.st_size;
			} else
				last_length=buffersize;
			/* buffer whole pages */
			last_length=(last_length+4095)&0xfffff000;
			s=malloc(last_length);
			if (!s) {
				zpfatal(_("out of memory"));
				exit(1);
			}
		}
		if (s) {
#ifdef SETVBUF_REVERSED
			setvbuf(input_f,_IOFBF,s,last_length);
#else
			setvbuf(input_f,s,_IOFBF,last_length);
#endif
		}
	}
	vpos = 0;
	/* Check for directory or block special files */
	fstat(fileno(input_f), &f);
#if defined(S_ISDIR)
	if (S_ISDIR(f.st_mode) || S_ISBLK(f.st_mode)) {
#else
	c = f.st_mode & S_IFMT;
	if (c == S_IFDIR || c == S_IFBLK) {
#endif
		error(0,0, _("is not a file: %s"),name);
		fclose(input_f);
		return OK;
	}

	if (remotename) {
		/* disqualify const */
		union {
			const char *c;
			char *s;
		} cheat;
		cheat.c=remotename;
		zi.fname=cheat.s;
	} else
		zi.fname=name;
	zi.modtime=f.st_mtime;
	zi.mode=f.st_mode;
#if defined(S_ISFIFO)
	zi.bytes_total= (S_ISFIFO(f.st_mode)) ? DEFBYTL : f.st_size;
#else
	zi.bytes_total= c == S_IFIFO ? DEFBYTL : f.st_size;
#endif
	zi.bytes_sent=0;
	zi.bytes_received=0;
	zi.bytes_skipped=0;
	zi.eof_seen=0;
	timing(1,NULL);

	++Filcnt;
	switch (wctxpn(&zi)) {
	case ERROR:
#ifdef ENABLE_SYSLOG
		if (enable_syslog)
			lsyslog(LOG_INFO, _("%s/%s: error occured"),protname(),shortname);
#endif
		return ERROR;
	case ZSKIP:
		error(0,0, _("skipped: %s"),name);
#ifdef ENABLE_SYSLOG
		if (enable_syslog)
			lsyslog(LOG_INFO, _("%s/%s: skipped"),protname(),shortname);
#endif
		return OK;
	}
	if (!zmodem_requested && wctx(&zi)==ERROR)
	{
#ifdef ENABLE_SYSLOG
		if (enable_syslog)
			lsyslog(LOG_INFO, _("%s/%s: error occured"),protname(),shortname);
#endif
		return ERROR;
	}
	if (Unlinkafter)
		unlink(oname);

	if (Verbose > 1
#ifdef ENABLE_SYSLOG
		|| enable_syslog
#endif
		) {
		long bps;
		double d=timing(0,NULL);
		if (d==0) /* can happen if timing() uses time() */
			d=0.5;
		bps=zi.bytes_sent/d;
		vchar('\r');
		if (Verbose > 1) 
			vstringf(_("Bytes Sent:%7ld   BPS:%-8ld                        \n"),
				(long) zi.bytes_sent,bps);
#ifdef ENABLE_SYSLOG
		if (enable_syslog)
			lsyslog(LOG_INFO, "%s/%s: %ld Bytes, %ld BPS",shortname,
				protname(), (long) zi.bytes_sent,bps);
#endif
	}
	return 0;
}

/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length, mode time and file mode in octal
 *  as provided by the Unix fstat call.
 *  N.B.: modifies the passed name, may extend it!
 */
static int
wctxpn(struct zm_fileinfo *zi)
{
	register char *p, *q;
	char *name2;
	struct stat f;

	name2=alloca(PATH_MAX+1);

	if (protocol==ZM_XMODEM) {
		if (Verbose && *zi->fname && fstat(fileno(input_f), &f)!= -1) {
			vstringf(_("Sending %s, %ld blocks: "),
			  zi->fname, (long) (f.st_size>>7));
		}
		vstringf(_("Give your local XMODEM receive command now."));
		vstring("\r\n");
		return OK;
	}
	if (!zmodem_requested)
		if (getnak()) {
			vfile("getnak failed");
			DO_SYSLOG((LOG_INFO, "%s/%s: getnak failed",
					   shortname,protname()));
			return ERROR;
		}

	q = (char *) 0;
	if (Dottoslash) {		/* change . to . */
		for (p=zi->fname; *p; ++p) {
			if (*p == '/')
				q = p;
			else if (*p == '.')
				*(q=p) = '/';
		}
		if (q && strlen(++q) > 8) {	/* If name>8 chars */
			q += 8;			/*   make it .ext */
			strcpy(name2, q);	/* save excess of name */
			*q = '.';
			strcpy(++q, name2);	/* add it back */
		}
	}

	for (p=zi->fname, q=txbuf ; *p; )
		if ((*q++ = *p++) == '/' && !Fullname)
			q = txbuf;
	*q++ = 0;
	p=q;
	while (q < (txbuf + MAX_BLOCK))
		*q++ = 0;
	/* note that we may lose some information here in case mode_t is wider than an 
	 * int. But i believe sending %lo instead of %o _could_ break compatability
	 */
	if (!Ascii && (input_f!=stdin) && *zi->fname && fstat(fileno(input_f), &f)!= -1)
		sprintf(p, "%lu %lo %o 0 %d %ld", (long) f.st_size, f.st_mtime,
		  (unsigned int)((no_unixmode) ? 0 : f.st_mode), 
		  Filesleft, Totalleft);
	if (Verbose)
		vstringf(_("Sending: %s\n"),txbuf);
	Totalleft -= f.st_size;
	if (--Filesleft <= 0)
		Totalleft = 0;
	if (Totalleft < 0)
		Totalleft = 0;

	/* force 1k blocks if name won't fit in 128 byte block */
	if (txbuf[125])
		blklen=1024;
	else {		/* A little goodie for IMP/KMD */
		txbuf[127] = (f.st_size + 127) >>7;
		txbuf[126] = (f.st_size + 127) >>15;
	}
	if (zmodem_requested)
		return zsendfile(zi,txbuf, 1+strlen(p)+(p-txbuf));
	if (wcputsec(txbuf, 0, 128)==ERROR) {
		vfile("wcputsec failed");
		DO_SYSLOG((LOG_INFO, "%s/%s: wcputsec failed",
				   shortname,protname()));
		return ERROR;
	}
	return OK;
}

static int 
getnak(void)
{
	int firstch;
	int tries=0;

	Lastrx = 0;
	for (;;) {
		tries++;
		switch (firstch = READLINE_PF(100)) {
		case ZPAD:
			if (getzrxinit())
				return ERROR;
			Ascii = 0;	/* Receiver does the conversion */
			return FALSE;
		case TIMEOUT:
			/* 30 seconds are enough */
			if (tries==3) {
				zperr(_("Timeout on pathname"));
				return TRUE;
			}
			/* don't send a second ZRQINIT _directly_ after the
			 * first one. Never send more then 4 ZRQINIT, because
			 * omen rz stops if it saw 5 of them */
			if ((zrqinits_sent>1 || tries>1) && zrqinits_sent<4) {
				/* if we already sent a ZRQINIT we are using zmodem
				 * protocol and may send further ZRQINITs 
				 */
				stohdr(0L);
				zshhdr(ZRQINIT, Txhdr);
				zrqinits_sent++;
			}
			continue;
		case WANTG:
			io_mode(io_mode_fd,2);	/* Set cbreak, XON/XOFF, etc. */
			Optiong = TRUE;
			blklen=1024;
		case WANTCRC:
			Crcflg = TRUE;
		case NAK:
			return FALSE;
		case CAN:
			if ((firstch = READLINE_PF(20)) == CAN && Lastrx == CAN)
				return TRUE;
		default:
			break;
		}
		Lastrx = firstch;
	}
}


static int 
wctx(struct zm_fileinfo *zi)
{
	register size_t thisblklen;
	register int sectnum, attempts, firstch;

	firstsec=TRUE;  thisblklen = blklen;
	vfile("wctx:file length=%ld", (long) zi->bytes_total);

	while ((firstch=READLINE_PF(Rxtimeout))!=NAK && firstch != WANTCRC
	  && firstch != WANTG && firstch!=TIMEOUT && firstch!=CAN)
		;
	if (firstch==CAN) {
		zperr(_("Receiver Cancelled"));
		return ERROR;
	}
	if (firstch==WANTCRC)
		Crcflg=TRUE;
	if (firstch==WANTG)
		Crcflg=TRUE;
	sectnum=0;
	for (;;) {
		if (zi->bytes_total <= (zi->bytes_sent + 896L))
			thisblklen = 128;
		if ( !filbuf(txbuf, thisblklen))
			break;
		if (wcputsec(txbuf, ++sectnum, thisblklen)==ERROR)
			return ERROR;
		zi->bytes_sent += thisblklen;
	}
	fclose(input_f);
	attempts=0;
	do {
		purgeline(io_mode_fd);
		sendline(EOT);
		flushmo();
		++attempts;
	} while ((firstch=(READLINE_PF(Rxtimeout)) != ACK) && attempts < RETRYMAX);
	if (attempts == RETRYMAX) {
		zperr(_("No ACK on EOT"));
		return ERROR;
	}
	else
		return OK;
}

static int 
wcputsec(char *buf, int sectnum, size_t cseclen)
{
	int checksum, wcj;
	char *cp;
	unsigned oldcrc;
	int firstch;
	int attempts;

	firstch=0;	/* part of logic to detect CAN CAN */

	if (Verbose>1) {
		vchar('\r');
		if (protocol==ZM_XMODEM) {
			vstringf(_("Xmodem sectors/kbytes sent: %3d/%2dk"), Totsecs, Totsecs/8 );
		} else {
			vstringf(_("Ymodem sectors/kbytes sent: %3d/%2dk"), Totsecs, Totsecs/8 );
		}
	}
	for (attempts=0; attempts <= RETRYMAX; attempts++) {
		Lastrx= firstch;
		sendline(cseclen==1024?STX:SOH);
		sendline(sectnum);
		sendline(-sectnum -1);
		oldcrc=checksum=0;
		for (wcj=cseclen,cp=buf; --wcj>=0; ) {
			sendline(*cp);
			oldcrc=updcrc((0377& *cp), oldcrc);
			checksum += *cp++;
		}
		if (Crcflg) {
			oldcrc=updcrc(0,updcrc(0,oldcrc));
			sendline((int)oldcrc>>8);
			sendline((int)oldcrc);
		}
		else
			sendline(checksum);

		flushmo();
		if (Optiong) {
			firstsec = FALSE; return OK;
		}
		firstch = READLINE_PF(Rxtimeout);
gotnak:
		switch (firstch) {
		case CAN:
			if(Lastrx == CAN) {
cancan:
				zperr(_("Cancelled"));  return ERROR;
			}
			break;
		case TIMEOUT:
			zperr(_("Timeout on sector ACK")); continue;
		case WANTCRC:
			if (firstsec)
				Crcflg = TRUE;
		case NAK:
			zperr(_("NAK on sector")); continue;
		case ACK: 
			firstsec=FALSE;
			Totsecs += (cseclen>>7);
			return OK;
		case ERROR:
			zperr(_("Got burst for sector ACK")); break;
		default:
			zperr(_("Got %02x for sector ACK"), firstch); break;
		}
		for (;;) {
			Lastrx = firstch;
			if ((firstch = READLINE_PF(Rxtimeout)) == TIMEOUT)
				break;
			if (firstch == NAK || firstch == WANTCRC)
				goto gotnak;
			if (firstch == CAN && Lastrx == CAN)
				goto cancan;
		}
	}
	zperr(_("Retry Count Exceeded"));
	return ERROR;
}

/* fill buf with count chars padding with ^Z for CPM */
static size_t 
filbuf(char *buf, size_t count)
{
	int c;
	size_t m;

	if ( !Ascii) {
		m = read(fileno(input_f), buf, count);
		if (m <= 0)
			return 0;
		while (m < count)
			buf[m++] = 032;
		return count;
	}
	m=count;
	if (Lfseen) {
		*buf++ = 012; --m; Lfseen = 0;
	}
	while ((c=getc(input_f))!=EOF) {
		if (c == 012) {
			*buf++ = 015;
			if (--m == 0) {
				Lfseen = TRUE; break;
			}
		}
		*buf++ =c;
		if (--m == 0)
			break;
	}
	if (m==count)
		return 0;
	else
		while (m--!=0)
			*buf++ = CPMEOF;
	return count;
}

/* Fill buffer with blklen chars */
static size_t
zfilbuf (struct zm_fileinfo *zi)
{
	size_t n;

	n = fread (txbuf, 1, blklen, input_f);
	if (n < blklen)
		zi->eof_seen = 1;
	else {
		/* save one empty paket in case file ends ob blklen boundary */
		int c = getc(input_f);

		if (c != EOF || !feof(input_f))
			ungetc(c, input_f);
		else
			zi->eof_seen = 1;
	}
	return n;
}

static void
usage1 (int exitcode)
{
	usage (exitcode, NULL);
}

static void
usage(int exitcode, const char *what)
{
	FILE *f=stdout;

	if (exitcode)
	{
		if (what)
			fprintf(stderr, "%s: %s\n",program_name,what);
	    fprintf (stderr, _("Try `%s --help' for more information.\n"),
            program_name);
		exit(exitcode);
	}

	fprintf(f, _("%s version %s\n"), program_name,
		VERSION);

	fprintf(f,_("Usage: %s [options] file ...\n"),
		program_name);
	fprintf(f,_("   or: %s [options] -{c|i} COMMAND\n"),program_name);
	fputs(_("Send file(s) with ZMODEM/YMODEM/XMODEM protocol\n"),f);
	fputs(_(
		"    (X) = option applies to XMODEM only\n"
		"    (Y) = option applies to YMODEM only\n"
		"    (Z) = option applies to ZMODEM only\n"
		),f);
	/* splitted into two halves for really bad compilers */
	fputs(_(
"  -+, --append                append to existing destination file (Z)\n"
"  -2, --twostop               use 2 stop bits\n"
"  -4, --try-4k                go up to 4K blocksize\n"
"      --start-4k              start with 4K blocksize (doesn't try 8)\n"
"  -8, --try-8k                go up to 8K blocksize\n"
"      --start-8k              start with 8K blocksize\n"
"  -a, --ascii                 ASCII transfer (change CR/LF to LF)\n"
"  -b, --binary                binary transfer\n"
"  -B, --bufsize N             buffer N bytes (N==auto: buffer whole file)\n"
"  -c, --command COMMAND       execute remote command COMMAND (Z)\n"
"  -C, --command-tries N       try N times to execute a command (Z)\n"
"  -d, --dot-to-slash          change '.' to '/' in pathnames (Y/Z)\n"
"      --delay-startup N       sleep N seconds before doing anything\n"
"  -e, --escape                escape all control characters (Z)\n"
"  -E, --rename                force receiver to rename files it already has\n"
"  -f, --full-path             send full pathname (Y/Z)\n"
"  -i, --immediate-command CMD send remote CMD, return immediately (Z)\n"
"  -h, --help                  print this usage message\n"
"  -k, --1k                    send 1024 byte packets (X)\n"
"  -L, --packetlen N           limit subpacket length to N bytes (Z)\n"
"  -l, --framelen N            limit frame length to N bytes (l>=L) (Z)\n"
"  -m, --min-bps N             stop transmission if BPS below N\n"
"  -M, --min-bps-time N          for at least N seconds (default: 120)\n"
		),f);
	fputs(_(
"  -n, --newer                 send file if source newer (Z)\n"
"  -N, --newer-or-longer       send file if source newer or longer (Z)\n"
"  -o, --16-bit-crc            use 16 bit CRC instead of 32 bit CRC (Z)\n"
"  -O, --disable-timeouts      disable timeout code, wait forever\n"
"  -p, --protect               protect existing destination file (Z)\n"
"  -r, --resume                resume interrupted file transfer (Z)\n"
"  -R, --restricted            restricted, more secure mode\n"
"  -q, --quiet                 quiet (no progress reports)\n"
"  -s, --stop-at {HH:MM|+N}    stop transmission at HH:MM or in N seconds\n"
"      --tcp                   build a TCP connection to transmit files\n"
"      --tcp-server            open socket, wait for connection\n"
"  -u, --unlink                unlink file after transmission\n"
"  -U, --unrestrict            turn off restricted mode (if allowed to)\n"
"  -v, --verbose               be verbose, provide debugging information\n"
"  -w, --windowsize N          Window is N bytes (Z)\n"
"  -X, --xmodem                use XMODEM protocol\n"
"  -y, --overwrite             overwrite existing files\n"
"  -Y, --overwrite-or-skip     overwrite existing files, else skip\n"
"      --ymodem                use YMODEM protocol\n"
"  -Z, --zmodem                use ZMODEM protocol\n"
"\n"
"short options use the same arguments as the long ones\n"
	),f);
	exit(exitcode);
}

/*
 * Get the receiver's init parameters
 */
static int 
getzrxinit(void)
{
	static int dont_send_zrqinit=1;
	int old_timeout=Rxtimeout;
	int n;
	struct stat f;
	size_t rxpos;
	int timeouts=0;

	Rxtimeout=100; /* 10 seconds */
	/* XXX purgeline(io_mode_fd); this makes _real_ trouble. why? -- uwe */

	for (n=10; --n>=0; ) {
		/* we might need to send another zrqinit in case the first is 
		 * lost. But *not* if getting here for the first time - in
		 * this case we might just get a ZRINIT for our first ZRQINIT.
		 * Never send more then 4 ZRQINIT, because
		 * omen rz stops if it saw 5 of them.
		 */
		if (zrqinits_sent<4 && n!=10 && !dont_send_zrqinit) {
			zrqinits_sent++;
			stohdr(0L);
			zshhdr(ZRQINIT, Txhdr);
		}
		dont_send_zrqinit=0;
		
		switch (zgethdr(Rxhdr, 1,&rxpos)) {
		case ZCHALLENGE:	/* Echo receiver's challenge numbr */
			stohdr(rxpos);
			zshhdr(ZACK, Txhdr);
			continue;
		case ZCOMMAND:		/* They didn't see our ZRQINIT */
			/* ??? Since when does a receiver send ZCOMMAND?  -- uwe */
			continue;
		case ZRINIT:
			Rxflags = 0377 & Rxhdr[ZF0];
			Rxflags2 = 0377 & Rxhdr[ZF1];
			Txfcs32 = (Wantfcs32 && (Rxflags & CANFC32));
			{
				int old=Zctlesc;
				Zctlesc |= Rxflags & TESCCTL;
				/* update table - was initialised to not escape */
				if (Zctlesc && !old)
					zsendline_init();
			}
			Rxbuflen = (0377 & Rxhdr[ZP0])+((0377 & Rxhdr[ZP1])<<8);
			if ( !(Rxflags & CANFDX))
				Txwindow = 0;
			vfile("Rxbuflen=%d Tframlen=%d", Rxbuflen, Tframlen);
			if ( play_with_sigint)
				signal(SIGINT, SIG_IGN);
			io_mode(io_mode_fd,2);	/* Set cbreak, XON/XOFF, etc. */
#ifndef READCHECK
			/* Use MAX_BLOCK byte frames if no sample/interrupt */
			if (Rxbuflen < 32 || Rxbuflen > MAX_BLOCK) {
				Rxbuflen = MAX_BLOCK;
				vfile("Rxbuflen=%d", Rxbuflen);
			}
#endif
			/* Override to force shorter frame length */
			if (Tframlen && Rxbuflen > Tframlen)
				Rxbuflen = Tframlen;
			if ( !Rxbuflen)
				Rxbuflen = 1024;
			vfile("Rxbuflen=%d", Rxbuflen);

			/* If using a pipe for testing set lower buf len */
			fstat(0, &f);
#if defined(S_ISCHR)
			if (! (S_ISCHR(f.st_mode))) {
#else
			if ((f.st_mode & S_IFMT) != S_IFCHR) {
#endif
				Rxbuflen = MAX_BLOCK;
			}
			/*
			 * If input is not a regular file, force ACK's to
			 *  prevent running beyond the buffer limits
			 */
			if ( !command_mode) {
				fstat(fileno(input_f), &f);
#if defined(S_ISREG)
				if (!(S_ISREG(f.st_mode))) {
#else
				if ((f.st_mode & S_IFMT) != S_IFREG) {
#endif
					Canseek = -1;
					/* return ERROR; */
				}
			}
			/* Set initial subpacket length */
			if (blklen < 1024) {	/* Command line override? */
				if (Baudrate > 300)
					blklen = 256;
				if (Baudrate > 1200)
					blklen = 512;
				if (Baudrate > 2400)
					blklen = 1024;
			}
			if (Rxbuflen && blklen>Rxbuflen)
				blklen = Rxbuflen;
			if (blkopt && blklen > blkopt)
				blklen = blkopt;
			vfile("Rxbuflen=%d blklen=%d", Rxbuflen, blklen);
			vfile("Txwindow = %u Txwspac = %d", Txwindow, Txwspac);
			Rxtimeout=old_timeout;
			return (sendzsinit());
		case ZCAN:
		case TIMEOUT:
			if (timeouts++==0)
				continue; /* force one other ZRQINIT to be sent */
			return ERROR;
		case ZRQINIT:
			if (Rxhdr[ZF0] == ZCOMMAND)
				continue;
		default:
			zshhdr(ZNAK, Txhdr);
			continue;
		}
	}
	return ERROR;
}

/* Send send-init information */
static int 
sendzsinit(void)
{
	int c;

	if (Myattn[0] == '\0' && (!Zctlesc || (Rxflags & TESCCTL)))
		return OK;
	errors = 0;
	for (;;) {
		stohdr(0L);
		if (Zctlesc) {
			Txhdr[ZF0] |= TESCCTL; zshhdr(ZSINIT, Txhdr);
		}
		else
			zsbhdr(ZSINIT, Txhdr);
		ZSDATA(Myattn, 1+strlen(Myattn), ZCRCW);
		c = zgethdr(Rxhdr, 1,NULL);
		switch (c) {
		case ZCAN:
			return ERROR;
		case ZACK:
			return OK;
		default:
			if (++errors > 19)
				return ERROR;
			continue;
		}
	}
}

/* Send file name and related info */
static int 
zsendfile(struct zm_fileinfo *zi, const char *buf, size_t blen)
{
	int c;
	unsigned long crc;
	size_t rxpos;

	/* we are going to send a ZFILE. There cannot be much useful
	 * stuff in the line right now (*except* ZCAN?). 
	 */
#if 0
	purgeline(io_mode_fd); /* might possibly fix stefan glasers problems */
#endif

	for (;;) {
		Txhdr[ZF0] = Lzconv;	/* file conversion request */
		Txhdr[ZF1] = Lzmanag;	/* file management request */
		if (Lskipnocor)
			Txhdr[ZF1] |= ZF1_ZMSKNOLOC;
		Txhdr[ZF2] = Lztrans;	/* file transport request */
		Txhdr[ZF3] = 0;
		zsbhdr(ZFILE, Txhdr);
		ZSDATA(buf, blen, ZCRCW);
again:
		c = zgethdr(Rxhdr, 1, &rxpos);
		switch (c) {
		case ZRINIT:
			while ((c = READLINE_PF(50)) > 0)
				if (c == ZPAD) {
					goto again;
				}
			/* **** FALL THRU TO **** */
		default:
			continue;
		case ZRQINIT:  /* remote site is sender! */
			if (Verbose)
				vstringf(_("got ZRQINIT"));
			DO_SYSLOG((LOG_INFO, "%s/%s: got ZRQINIT - sz talks to sz",
					   shortname,protname()));
			return ERROR;
		case ZCAN:
			if (Verbose)
				vstringf(_("got ZCAN"));
			DO_SYSLOG((LOG_INFO, "%s/%s: got ZCAN - receiver canceled",
					   shortname,protname()));
			return ERROR;
		case TIMEOUT:
			DO_SYSLOG((LOG_INFO, "%s/%s: got TIMEOUT",
					   shortname,protname()));
			return ERROR;
		case ZABORT:
			DO_SYSLOG((LOG_INFO, "%s/%s: got ZABORT",
					   shortname,protname()));
			return ERROR;
		case ZFIN:
			DO_SYSLOG((LOG_INFO, "%s/%s: got ZFIN",
					   shortname,protname()));
			return ERROR;
		case ZCRC:
			crc = 0xFFFFFFFFL;
#ifdef HAVE_MMAP
			if (use_mmap && !mm_addr)
			{
				struct stat st;
				if (fstat (fileno (input_f), &st) == 0) {
					mm_size = st.st_size;
					mm_addr = mmap (0, mm_size, PROT_READ,
									MAP_SHARED, fileno (input_f), 0);
					if ((caddr_t) mm_addr == (caddr_t) - 1)
						mm_addr = NULL;
					else {
						fclose (input_f);
						input_f = NULL;
					}
				}
			}
			if (mm_addr) {
				size_t i;
				size_t count;
				char *p=mm_addr;
				count=(rxpos < mm_size && rxpos > 0)? rxpos: mm_size;
				for (i=0;i<count;i++,p++) {
					crc = UPDC32(*p, crc);
				}
				crc = ~crc;
			} else
#endif
			if (Canseek >= 0) {
				if (rxpos==0) {
					struct stat st;
					if (0==fstat(fileno(input_f),&st)) {
						rxpos=st.st_size;
					} else
						rxpos=-1;
				}
				while (rxpos-- && ((c = getc(input_f)) != EOF))
					crc = UPDC32(c, crc);
				crc = ~crc;
				clearerr(input_f);	/* Clear EOF */
				fseek(input_f, 0L, 0);
			}
			stohdr(crc);
			zsbhdr(ZCRC, Txhdr);
			goto again;
		case ZSKIP:
			if (input_f)
				fclose(input_f);
#ifdef HAVE_MMAP
			else if (mm_addr) {
				munmap(mm_addr,mm_size);
				mm_addr=NULL;
			}
#endif

			vfile("receiver skipped");
			DO_SYSLOG((LOG_INFO, "%s/%s: receiver skipped",
					   shortname, protname()));
			return c;
		case ZRPOS:
			/*
			 * Suppress zcrcw request otherwise triggered by
			 * lastsync==bytcnt
			 */
#ifdef HAVE_MMAP
			if (!mm_addr)
#endif
			if (rxpos && fseek(input_f, (long) rxpos, 0)) {
				int er=errno;
				vfile("fseek failed: %s", strerror(er));
				DO_SYSLOG((LOG_INFO, "%s/%s: fseek failed: %s",
						   shortname, protname(), strerror(er)));
				return ERROR;
			}
			if (rxpos)
				zi->bytes_skipped=rxpos;
			bytcnt = zi->bytes_sent = rxpos;
			Lastsync = rxpos -1;
	 		return zsendfdata(zi);
		}
	}
}

/* Send the data in the file */
static int
zsendfdata (struct zm_fileinfo *zi)
{
	static int c;
	int newcnt;
	static int junkcount;				/* Counts garbage chars received by TX */
	static size_t last_txpos = 0;
	static long last_bps = 0;
	static long not_printed = 0;
	static long total_sent = 0;
	static time_t low_bps=0;

#ifdef HAVE_MMAP
	if (use_mmap && !mm_addr)
	{
		struct stat st;
		if (fstat (fileno (input_f), &st) == 0) {
			mm_size = st.st_size;
			mm_addr = mmap (0, mm_size, PROT_READ,
							MAP_SHARED, fileno (input_f), 0);
			if ((caddr_t) mm_addr == (caddr_t) - 1)
				mm_addr = NULL;
			else {
				fclose (input_f);
				input_f = NULL;
			}
		}
	}
#endif

	if (play_with_sigint)
		signal (SIGINT, onintr);

	Lrxpos = 0;
	junkcount = 0;
	Beenhereb4 = 0;
  somemore:
	if (setjmp (intrjmp)) {
	  if (play_with_sigint)
		  signal (SIGINT, onintr);
	  waitack:
		junkcount = 0;
		c = getinsync (zi, 0);
	  gotack:
		switch (c) {
		default:
			if (input_f)
				fclose (input_f);
			DO_SYSLOG((LOG_INFO, "%s/%s: got %d",
					   shortname, protname(), c));
			return ERROR;
		case ZCAN:
			if (input_f)
				fclose (input_f);
			DO_SYSLOG((LOG_INFO, "%s/%s: got ZCAN",
					   shortname, protname(), c));
			return ERROR;
		case ZSKIP:
			if (input_f)
				fclose (input_f);
			DO_SYSLOG((LOG_INFO, "%s/%s: got ZSKIP",
					   shortname, protname(), c));
			return c;
		case ZACK:
		case ZRPOS:
			break;
		case ZRINIT:
			return OK;
		}
#ifdef READCHECK
		/*
		 * If the reverse channel can be tested for data,
		 *  this logic may be used to detect error packets
		 *  sent by the receiver, in place of setjmp/longjmp
		 *  rdchk(fdes) returns non 0 if a character is available
		 */
		while (rdchk (io_mode_fd)) {
#ifdef READCHECK_READS
			switch (checked)
#else
			switch (READLINE_PF (1))
#endif
			{
			case CAN:
			case ZPAD:
				c = getinsync (zi, 1);
				goto gotack;
			case XOFF:			/* Wait a while for an XON */
			case XOFF | 0200:
				READLINE_PF (100);
			}
		}
#endif
	}

	newcnt = Rxbuflen;
	Txwcnt = 0;
	stohdr (zi->bytes_sent);
	zsbhdr (ZDATA, Txhdr);

	do {
		size_t n;
		int e;
		unsigned old = blklen;
		blklen = calc_blklen (total_sent);
		total_sent += blklen + OVERHEAD;
		if (Verbose > 2 && blklen != old)
			vstringf (_("blklen now %d\n"), blklen);
#ifdef HAVE_MMAP
		if (mm_addr) {
			if (zi->bytes_sent + blklen < mm_size)
				n = blklen;
			else {
				n = mm_size - zi->bytes_sent;
				zi->eof_seen = 1;
			}
		} else
#endif
			n = zfilbuf (zi);
		if (zi->eof_seen) {
			e = ZCRCE;
			if (Verbose>3)
				vstring("e=ZCRCE/eof seen");
		} else if (junkcount > 3) {
			e = ZCRCW;
			if (Verbose>3)
				vstring("e=ZCRCW/junkcount > 3");
		} else if (bytcnt == Lastsync) {
			e = ZCRCW;
			if (Verbose>3)
				vstringf("e=ZCRCW/bytcnt == Lastsync == %ld", 
					(unsigned long) Lastsync);
#if 0
		/* what is this good for? Rxbuflen/newcnt normally are short - so after
		 * a few KB ZCRCW will be used? (newcnt is never incremented)
		 */
		} else if (Rxbuflen && (newcnt -= n) <= 0) {
			e = ZCRCW;
			if (Verbose>3)
				vstringf("e=ZCRCW/Rxbuflen(newcnt=%ld,n=%ld)", 
					(unsigned long) newcnt,(unsigned long) n);
#endif
		} else if (Txwindow && (Txwcnt += n) >= Txwspac) {
			Txwcnt = 0;
			e = ZCRCQ;
			if (Verbose>3)
				vstring("e=ZCRCQ/Window");
		} else {
			e = ZCRCG;
			if (Verbose>3)
				vstring("e=ZCRCG");
		}
		if ((Verbose > 1 || min_bps || stop_time)
			&& (not_printed > (min_bps ? 3 : 7) 
				|| zi->bytes_sent > last_bps / 2 + last_txpos)) {
			int minleft = 0;
			int secleft = 0;
			time_t now;
			last_bps = (zi->bytes_sent / timing (0,&now));
			if (last_bps > 0) {
				minleft = (zi->bytes_total - zi->bytes_sent) / last_bps / 60;
				secleft = ((zi->bytes_total - zi->bytes_sent) / last_bps) % 60;
			}
			if (min_bps) {
				if (low_bps) {
					if (last_bps<min_bps) {
						if (now-low_bps>=min_bps_time) {
							/* too bad */
							if (Verbose) {
								vstringf(_("zsendfdata: bps rate %ld below min %ld"),
								  last_bps, min_bps);
								vstring("\r\n");
							}
							DO_SYSLOG((LOG_INFO, "%s/%s: bps rate low: %ld <%ld",
									   shortname, protname(), last_bps, min_bps));
							return ERROR;
						}
					} else
						low_bps=0;
				} else if (last_bps < min_bps) {
					low_bps=now;
				}
			}
			if (stop_time && now>=stop_time) {
				/* too bad */
				if (Verbose) {
					vstring(_("zsendfdata: reached stop time"));
					vstring("\r\n");
				}
				DO_SYSLOG((LOG_INFO, "%s/%s: reached stop time",
						   shortname, protname()));
				return ERROR;
			}

			if (Verbose > 1) {
				vchar ('\r');
				vstringf (_("Bytes Sent:%7ld/%7ld   BPS:%-8ld ETA %02d:%02d  "),
					 (long) zi->bytes_sent, (long) zi->bytes_total, 
					last_bps, minleft, secleft);
			}
			last_txpos = zi->bytes_sent;
		} else if (Verbose)
			not_printed++;
		ZSDATA (DATAADR, n, e);
		bytcnt = zi->bytes_sent += n;
		if (e == ZCRCW)
			goto waitack;
#ifdef READCHECK
		/*
		 * If the reverse channel can be tested for data,
		 *  this logic may be used to detect error packets
		 *  sent by the receiver, in place of setjmp/longjmp
		 *  rdchk(fdes) returns non 0 if a character is available
		 */
		fflush (stdout);
		while (rdchk (io_mode_fd)) {
#ifdef READCHECK_READS
			switch (checked)
#else
			switch (READLINE_PF (1))
#endif
			{
			case CAN:
			case ZPAD:
				c = getinsync (zi, 1);
				if (c == ZACK)
					break;
				/* zcrce - dinna wanna starta ping-pong game */
				ZSDATA (txbuf, 0, ZCRCE);
				goto gotack;
			case XOFF:			/* Wait a while for an XON */
			case XOFF | 0200:
				READLINE_PF (100);
			default:
				++junkcount;
			}
		}
#endif							/* READCHECK */
		if (Txwindow) {
			size_t tcount = 0;
			while ((tcount = zi->bytes_sent - Lrxpos) >= Txwindow) {
				vfile ("%ld (%ld,%ld) window >= %u", tcount, 
					(long) zi->bytes_sent, (long) Lrxpos,
					Txwindow);
				if (e != ZCRCQ)
					ZSDATA (txbuf, 0, e = ZCRCQ);
				c = getinsync (zi, 1);
				if (c != ZACK) {
					ZSDATA (txbuf, 0, ZCRCE);
					goto gotack;
				}
			}
			vfile ("window = %ld", tcount);
		}
	} while (!zi->eof_seen);


	if (play_with_sigint)
		signal (SIGINT, SIG_IGN);

	for (;;) {
		stohdr (zi->bytes_sent);
		zsbhdr (ZEOF, Txhdr);
		switch (getinsync (zi, 0)) {
		case ZACK:
			continue;
		case ZRPOS:
			goto somemore;
		case ZRINIT:
			return OK;
		case ZSKIP:
			if (input_f)
				fclose (input_f);
			DO_SYSLOG((LOG_INFO, "%s/%s: got ZSKIP",
					   shortname, protname()));
			return c;
		default:
			if (input_f)
				fclose (input_f);
			DO_SYSLOG((LOG_INFO, "%s/%s: got %d",
					   shortname, protname(), c));
			return ERROR;
		}
	}
}

static int
calc_blklen(long total_sent)
{
	static long total_bytes=0;
	static int calcs_done=0;
	static long last_error_count=0;
	static int last_blklen=0;
	static long last_bytes_per_error=0;
	unsigned long best_bytes=0;
	long best_size=0;
	long this_bytes_per_error;
	long d;
	unsigned int i;
	if (total_bytes==0)
	{
		/* called from countem */
		total_bytes=total_sent;
		return 0;
	}

	/* it's not good to calc blklen too early */
	if (calcs_done++ < 5) {
		if (error_count && start_blklen >1024)
			return last_blklen=1024;
		else 
			last_blklen/=2;
		return last_blklen=start_blklen;
	}

	if (!error_count) {
		/* that's fine */
		if (start_blklen==max_blklen)
			return start_blklen;
		this_bytes_per_error=LONG_MAX;
		goto calcit;
	}

	if (error_count!=last_error_count) {
		/* the last block was bad. shorten blocks until one block is
		 * ok. this is because very often many errors come in an
		 * short period */
		if (error_count & 2)
		{
			last_blklen/=2;
			if (last_blklen < 32)
				last_blklen = 32;
			else if (last_blklen > 512)
				last_blklen=512;
			if (Verbose > 3)
				vstringf(_("calc_blklen: reduced to %d due to error\n"),
					last_blklen);
		}
		last_error_count=error_count;
		last_bytes_per_error=0; /* force recalc */
		return last_blklen;
	}

	this_bytes_per_error=total_sent / error_count;
		/* we do not get told about every error, because
		 * there may be more than one error per failed block.
		 * but one the other hand some errors are reported more
		 * than once: If a modem buffers more than one block we
		 * get at least two ZRPOS for the same position in case
		 * *one* block has to be resent.
		 * so don't do this:
		 * this_bytes_per_error/=2;
		 */
	/* there has to be a margin */
	if (this_bytes_per_error<100)
		this_bytes_per_error=100;

	/* be nice to the poor machine and do the complicated things not
	 * too often
	 */
	if (last_bytes_per_error>this_bytes_per_error)
		d=last_bytes_per_error-this_bytes_per_error;
	else
		d=this_bytes_per_error-last_bytes_per_error;
	if (d<4)
	{
		if (Verbose > 3)
		{
			vstringf(_("calc_blklen: returned old value %d due to low bpe diff\n"),
				last_blklen);
			vstringf(_("calc_blklen: old %ld, new %ld, d %ld\n"),
				last_bytes_per_error,this_bytes_per_error,d );
		}
		return last_blklen;
	}
	last_bytes_per_error=this_bytes_per_error;

calcit:
	if (Verbose > 3)
		vstringf(_("calc_blklen: calc total_bytes=%ld, bpe=%ld, ec=%ld\n"),
			total_bytes,this_bytes_per_error,(long) error_count);
	for (i=32;i<=max_blklen;i*=2) {
		long ok; /* some many ok blocks do we need */
		long failed; /* and that's the number of blocks not transmitted ok */
		unsigned long transmitted;
		ok=total_bytes / i + 1;
		failed=((long) i + OVERHEAD) * ok / this_bytes_per_error;
		transmitted=total_bytes + ok * OVERHEAD  
			+ failed * ((long) i+OVERHEAD+OVER_ERR);
		if (Verbose > 4)
			vstringf(_("calc_blklen: blklen %d, ok %ld, failed %ld -> %lu\n"),
				i,ok,failed,transmitted);
		if (transmitted < best_bytes || !best_bytes)
		{
			best_bytes=transmitted;
			best_size=i;
		}
	}
	if (best_size > 2*last_blklen)
		best_size=2*last_blklen;
	last_blklen=best_size;
	if (Verbose > 3)
		vstringf(_("calc_blklen: returned %d as best\n"),
			last_blklen);
	return last_blklen;
}

/*
 * Respond to receiver's complaint, get back in sync with receiver
 */
static int 
getinsync(struct zm_fileinfo *zi, int flag)
{
	int c;
	size_t rxpos;

	for (;;) {
		c = zgethdr(Rxhdr, 0, &rxpos);
		switch (c) {
		case ZCAN:
		case ZABORT:
		case ZFIN:
		case TIMEOUT:
			return ERROR;
		case ZRPOS:
			/* ************************************* */
			/*  If sending to a buffered modem, you  */
			/*   might send a break at this point to */
			/*   dump the modem's buffer.		 */
			if (input_f)
				clearerr(input_f);	/* In case file EOF seen */
#ifdef HAVE_MMAP
			if (!mm_addr)
#endif
			if (fseek(input_f, (long) rxpos, 0))
				return ERROR;
			zi->eof_seen = 0;
			bytcnt = Lrxpos = zi->bytes_sent = rxpos;
			if (Lastsync == rxpos) {
				error_count++;
			}
			Lastsync = rxpos;
			return c;
		case ZACK:
			Lrxpos = rxpos;
			if (flag || zi->bytes_sent == rxpos)
				return ZACK;
			continue;
		case ZRINIT:
		case ZSKIP:
			if (input_f)
				fclose(input_f);
#ifdef HAVE_MMAP
			else if (mm_addr) {
				munmap(mm_addr,mm_size);
				mm_addr=NULL;
			}
#endif
			return c;
		case ERROR:
		default:
			error_count++;
			zsbhdr(ZNAK, Txhdr);
			continue;
		}
	}
}


/* Say "bibi" to the receiver, try to do it cleanly */
static void
saybibi(void)
{
	for (;;) {
		stohdr(0L);		/* CAF Was zsbhdr - minor change */
		zshhdr(ZFIN, Txhdr);	/*  to make debugging easier */
		switch (zgethdr(Rxhdr, 0,NULL)) {
		case ZFIN:
			sendline('O');
			sendline('O');
			flushmo();
		case ZCAN:
		case TIMEOUT:
			return;
		}
	}
}

/* Send command and related info */
static int 
zsendcmd(const char *buf, size_t blen)
{
	int c;
	pid_t cmdnum;
	size_t rxpos;

	cmdnum = getpid();
	errors = 0;
	for (;;) {
		stohdr((size_t) cmdnum);
		Txhdr[ZF0] = Cmdack1;
		zsbhdr(ZCOMMAND, Txhdr);
		ZSDATA(buf, blen, ZCRCW);
listen:
		Rxtimeout = 100;		/* Ten second wait for resp. */
		c = zgethdr(Rxhdr, 1, &rxpos);

		switch (c) {
		case ZRINIT:
			goto listen;	/* CAF 8-21-87 */
		case ERROR:
		case TIMEOUT:
			if (++errors > Cmdtries)
				return ERROR;
			continue;
		case ZCAN:
		case ZABORT:
		case ZFIN:
		case ZSKIP:
		case ZRPOS:
			return ERROR;
		default:
			if (++errors > 20)
				return ERROR;
			continue;
		case ZCOMPL:
			Exitcode = rxpos;
			saybibi();
			return OK;
		case ZRQINIT:
			vfile("******** RZ *******");
			system("rz");
			vfile("******** SZ *******");
			goto listen;
		}
	}
}

/*
 * If called as lsb use YMODEM protocol
 */
static void
chkinvok (const char *s)
{
	const char *p;

	p = s;
	while (*p == '-')
		s = ++p;
	while (*p)
		if (*p++ == '/')
			s = p;
	if (*s == 'v') {
		Verbose = 1;
		++s;
	}
	program_name = s;
	if (*s == 'l')
		s++;					/* lsz -> sz */
	protocol = ZM_ZMODEM;
	if (s[0] == 's' && s[1] == 'x')
		protocol = ZM_XMODEM;
	if (s[0] == 's' && (s[1] == 'b' || s[1] == 'y')) {
		protocol = ZM_YMODEM;
	}
}

static void
countem (int argc, char **argv)
{
	struct stat f;

	for (Totalleft = 0, Filesleft = 0; --argc >= 0; ++argv) {
		f.st_size = -1;
		if (Verbose > 2) {
			vstringf ("\nCountem: %03d %s ", argc, *argv);
		}
		if (access (*argv, R_OK) >= 0 && stat (*argv, &f) >= 0) {
#if defined(S_ISDIR)
			if (!S_ISDIR(f.st_mode) && !S_ISBLK(f.st_mode)) {
#else
			int c;
			c = f.st_mode & S_IFMT;
			if (c != S_IFDIR && c != S_IFBLK) {
#endif
				++Filesleft;
				Totalleft += f.st_size;
			}
		} else if (strcmp (*argv, "-") == 0) {
			++Filesleft;
			Totalleft += DEFBYTL;
		}
		if (Verbose > 2)
			vstringf (" %ld", (long) f.st_size);
	}
	if (Verbose > 2)
		vstringf (_("\ncountem: Total %d %ld\n"),
				 Filesleft, Totalleft);
	calc_blklen (Totalleft);
}

/* End of lsz.c */


