#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Demo of a package using pre-processing substitutions '%reldir%' and
# '%canon_reldir%', and their respective shorthands '%D%' and '%C%'.

am_create_testdir=empty
required=cc
. test-init.sh

if cross_compiling; then
  WE_ARE_CROSS_COMPILING=yes
else
  WE_ARE_CROSS_COMPILING=no
fi
export WE_ARE_CROSS_COMPILING

SAFE_PRINT_FAIL=; unset SAFE_PRINT_FAIL

cat > configure.ac << 'END'
AC_INIT([GNU Demo], [0.7], [bug-automake@gnu.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.12.6 foreign subdir-objects -Wall])
AM_CONDITIONAL([NATIVE_BUILD], [test $WE_ARE_CROSS_COMPILING != yes])
AC_CONFIG_FILES([Makefile])
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AR
AC_PROG_RANLIB
AC_OUTPUT
END

mkdir build-aux lib lib/tests src tests

## Top level.

cat > Makefile.am << 'END'
bin_PROGRAMS =
check_PROGRAMS =
noinst_LIBRARIES =
AM_CPPFLAGS =
AM_TESTS_ENVIRONMENT =
CLEANFILES =
EXTRA_DIST =
LDADD =
TESTS =

include $(srcdir)/src/progs.am
include $(srcdir)/lib/gnulib.am
include $(srcdir)/tests/check.am
END

## Src subdir.

cat > src/progs.am <<'END'
bin_PROGRAMS += %reldir%/hello

bin_PROGRAMS += %D%/goodbye
%canon_reldir%_goodbye_SOURCES = %D%/hello.c
%C%_goodbye_CPPFLAGS = $(AM_CPPFLAGS) -DGREETINGS='"Goodbye"'

# The testsuite should have access to our built programs.
AM_TESTS_ENVIRONMENT += \
  PROGDIR='$(top_builddir)/%reldir%'; \
  export PROGDIR; \
  PATH='$(abs_builddir)/%reldir%'$(PATH_SEPARATOR)$$PATH; \
  export PATH;
END

cat > src/hello.c <<'END'
#include "safe-print.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef GREETINGS
#  define GREETINGS "Hello"
#endif

int
main (void)
{
  safe_print (stdout, GREETINGS ", World!\n");
  exit (EXIT_SUCCESS);
}
END

## Lib subdir.

cat > lib/gnulib.am << 'END'
noinst_LIBRARIES += %D%/libgnu.a

AM_CPPFLAGS += -I%D% -I$(top_srcdir)/%D%
LDADD += $(noinst_LIBRARIES)

%C%_libgnu_a_SOURCES = \
  %D%/safe-print.c \
  %D%/safe-print.h

if NATIVE_BUILD
include %D%/tests/gnulib-check.am
endif
END

cat > lib/safe-print.c <<'END'
#include "safe-print.h"
#include <string.h>
#include <stdlib.h>

void
safe_print (FILE *fp, const char * str)
{
  if (fprintf (fp, "%s", str) != strlen (str)
       || fflush (fp) != 0 || ferror (fp))
    {
      fprintf (stderr, "I/O error\n");
      exit (EXIT_FAILURE);
    }
}

END

cat > lib/safe-print.h <<'END'
#include <stdio.h>
void safe_print (FILE *, const char *);
END

## Lib/Tests (sub)subdir.

cat > lib/tests/gnulib-check.am <<'END'
check_PROGRAMS += %D%/safe-print-test
TESTS += $(check_PROGRAMS)
END

cat > lib/tests/safe-print-test.c <<'END'
#include "safe-print.h"
int
main (void)
{
  safe_print (stdout, "dummy\n");
  return 0;
}
END

## Tests subdir.

cat > tests/check.am <<'END'
TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SHELL)

AM_TESTS_ENVIRONMENT += EXEEXT='$(EXEEXT)'; export EXEEXT;

handwritten_TESTS = \
  %D%/hello.sh \
  %D%/built.sh
TESTS += $(handwritten_TESTS)
EXTRA_DIST += $(handwritten_TESTS)

TESTS += %D%/goodbye.sh
CLEANFILES += %D%/goodbye.sh
%D%/goodbye.sh: %D%/hello.sh
	$(MKDIR_P) %D%
	rm -f $@ $@-t
	sed -e 's/hello/goodbye/' \
	    -e 's/Hello/Goodbye/' \
          < $(srcdir)/%D%/hello.sh >$@-t
	chmod a-w,a+x $@-t && mv -f $@-t $@
END

cat > tests/hello.sh <<'END'
#!/bin/sh
set -x -e
if test "$WE_ARE_CROSS_COMPILING" = yes; then
  echo Skipping: cannot run in cross-compilation mode
  exit 77
else
  hello || exit 1
  test "`hello`" = 'Hello, World!' || exit 1
fi
END

cat > tests/built.sh <<'END'
#!/bin/sh
set -x
test -n "$PROGDIR" || exit 99
test -f "$PROGDIR/hello$EXEEXT" || exit 1
test -x "$PROGDIR/hello$EXEEXT" || exit 1
test -f "$PROGDIR/goodbye$EXEEXT" || exit 1
test -x "$PROGDIR/goodbye$EXEEXT" || exit 1
END


## Go.

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing --copy
test ! -e compile
test -f build-aux/compile

./configure

$MAKE

run_make -O check VERBOSE=x
cat tests/built.log
cat tests/hello.log
cat tests/goodbye.log
if cross_compiling; then
  test ! -e lib/tests/safe-print-test.log
  count_test_results total=3 pass=1 fail=0 xpass=0 xfail=0 skip=2 error=0
else
  count_test_results total=4 pass=4 fail=0 xpass=0 xfail=0 skip=0 error=0
fi

$MAKE distcheck

:
