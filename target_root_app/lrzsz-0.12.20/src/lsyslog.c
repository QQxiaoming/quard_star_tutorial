/*
  lsyslog.c - wrapper for the syslog function
  Copyright (C) 1997 Uwe Ohse

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

*/
#include "config.h"
#ifdef ENABLE_SYSLOG
#include "zglobal.h"
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#endif

#if __STDC__
#  include <stdarg.h>
#  define VA_START(args, lastarg) va_start(args, lastarg)
#  define WAYTOGO
#else
#  include <varargs.h>
#  define VA_START(args, lastarg) va_start(args)
#endif

void
#ifdef WAYTOGO
lsyslog(int prio, const char *format, ...)
#else
lsyslog(prio,format,va_alist) 
	int prio; 
	const char *format; 
	va_dcl
#endif
{
#ifdef ENABLE_SYSLOG
	static char *username=NULL;
	static char uid_string[20]=""; /* i'd really hate this function to fail! */
	char *s=NULL;
	static int init_done=0;
    va_list ap;
	if (!enable_syslog)
		return;
	if (!init_done) {
		uid_t uid;
		struct passwd *pwd;
		init_done=1;
		uid=getuid();
		pwd=getpwuid(uid);
		if (pwd && pwd->pw_name && *pwd->pw_name) {
			username=strdup(pwd->pw_name);
		}
		if (!username) {
			username=uid_string;
			sprintf(uid_string,"#%lu",(unsigned long) uid);
		}
	}

    VA_START(ap, format);
    vasprintf(&s,format, ap);
    va_end(ap);
    syslog(prio,"[%s] %s",username,s);
	free(s);
#else
	(void) prio; /* get rid of warning */
	(void) format; /* get rid of warning */
#endif
}

