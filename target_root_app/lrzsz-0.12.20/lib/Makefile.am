noinst_LIBRARIES=libzmodem.a
CFLAGS=@CFLAGS@
AUTOMAKE_OPTIONS=ansi2knr

EXTRA_DIST = alloca.c ansi2knr.1 ansi2knr.c \
	getopt.c getopt1.c mkdir.c mktime.c \
	strdup.c strerror.c strstr.c strpbrk.c strtol.c strtoul.c stpcpy.c \
	strftime.c vasprintf.c

libzmodem_a_SOURCES = long-options.c xstrtol.c xstrtoul.c error.c
libzmodem_a_LIBADD  = @LIBOBJS@ @ALLOCA@
libzmodem_a_DEPENDENCIES  = @ALLOCA@ @LIBOBJS@

noinst_HEADERS = getopt.h long-options.h xstrtol.h xstrtoul.h error.h

