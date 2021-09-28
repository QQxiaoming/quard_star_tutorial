/*
  zperr.c - "stderr" output stuff
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

  originally written by Uwe Ohse
*/
#include "zglobal.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __STDC__
#  define WAYTOGO
#  include <stdarg.h>
#  define VA_START(args, lastarg) va_start(args, lastarg)
#else
#  include <varargs.h>
#  define VA_START(args, lastarg) va_start(args)
#endif

void
#ifdef WAYTOGO
zperr(const char *fmt, ...)
#else
zperr(fmt, va_alist)
	const char *fmt;
	va_dcl
#endif
{
    va_list ap;

	if (Verbose<=0)
		return;
	fprintf(stderr,_("Retry %d: "),errors);
    VA_START(ap, fmt);
    vfprintf(stderr,fmt, ap);
    va_end(ap);
    putc('\n',stderr);
}

void
#ifdef WAYTOGO
zpfatal(const char *fmt, ...)
#else
zpfatal(fmt, va_alist)
	const char *fmt;
	va_dcl
#endif
{
    va_list ap;
    int err=errno;

	if (Verbose<=0)
		return;
	fprintf(stderr,"%s: ",program_name);
    VA_START(ap, fmt);
    vfprintf(stderr,fmt, ap);
    va_end(ap);
	fprintf(stderr,": %s\n",strerror(err));
}

void 
#ifdef WAYTOGO
vfile(const char *format, ...)
#else
vfile(format, va_alist)
	const char *format;
	va_dcl
#endif
{
    va_list ap;

	if (Verbose < 3)
		return;
    VA_START(ap, format);
    vfprintf(stderr,format, ap);
    va_end(ap);
    putc('\n',stderr);
}

#ifndef vstringf
/* if using gcc this function is not needed */
void 
#ifdef WAYTOGO
vstringf(const char *format, ...)
#else
vstringf(format, va_alist)
	const char *format;
	va_dcl
#endif
{
    va_list ap;

    VA_START(ap, format);
    vfprintf(stderr,format, ap);
    va_end(ap);
}
#endif
