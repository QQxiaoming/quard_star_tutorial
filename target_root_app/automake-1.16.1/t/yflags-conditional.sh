#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check that automake complains about *_YFLAGS variables which have
# conditional content.

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC

# 'YFLAGS' is AC_SUBST'd by AC_PROG_YACC by default, but we
# don't want this, since it might confuse our error messages.
# Also, AM_SUBST_NOTMAKE seems not to help about this.
# So we simply define $(YACC) by hand.
AC_SUBST([YACC], [yacc])

AM_CONDITIONAL([COND], [:])
END

$ACLOCAL

cat > Makefile.am <<'END'
bin_PROGRAMS = foo zardoz
foo_SOURCES = foo.y
zardoz_SOURCES = zardoz.y
if COND
AM_YFLAGS = -v
zardoz_YFLAGS = -v
endif COND
END

cat > Makefile1.am <<'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.y
## This is a dummy comment to keep line count right.
if COND
YFLAGS = foo
endif COND
END

cat > Makefile2.am <<'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.y
AM_YFLAGS = am_yflags
if COND
YFLAGS = yflags
endif COND
END

cat > Makefile3.am <<'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.y
foo_YFLAGS = foo_yflags
if COND
YFLAGS = yflags
endif COND
END

cat > Makefile4.am <<'END'
bin_PROGRAMS = foo zardoz

foo_SOURCES = foo.y
zardoz_SOURCES = $(foo_SOURCES)

YFLAGS =
AM_YFLAGS = $(COND_VAR1)
zardoz_YFLAGS = $(COND_VAR2:z=r)

COND_VAR2 = foo
if COND
YFLAGS += -v
COND_VAR2 += bar
else !COND
COND_VAR1 = -d
endif !COND
END

cat > Makefile5.am <<'END'
bin_PROGRAMS = foo zardoz
foo_SOURCES = foo.y
zardoz_SOURCES = zardoz.y
YFLAGS = -v
AM_YFLAGS = -v
if COND
zardoz_YFLAGS = -v
endif
END

cat > Makefile6.am <<'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.y
foo_YFLAGS = -v
if COND
quux_YFLAGS = -v
AM_YFLAGS = -v
endif
END

: > ylwrap

LC_ALL=C; export LC_ALL; # For grep regexes below.

AUTOMAKE_fails -Wnone -Wunsupported Makefile
grep '^Makefile\.am:5:.*AM_YFLAGS.* conditional contents' stderr
grep '^Makefile\.am:6:.*zardoz_YFLAGS.* conditional contents' stderr

for i in 1 2 3; do
  AUTOMAKE_fails -Wnone -Wunsupported Makefile$i
  grep "^Makefile$i\\.am:5:.*[^a-zA-Z0-9_]YFLAGS.* conditional contents" \
       stderr
done

AUTOMAKE_fails -Wnone -Wunsupported Makefile4
grep '^Makefile4\.am:6:.*[^a-zA-Z0-9_]YFLAGS.* conditional contents' stderr
grep '^Makefile4\.am:7:.*AM_YFLAGS.* conditional contents' stderr
grep '^Makefile4\.am:8:.*zardoz_YFLAGS.* conditional contents' stderr

# Now let's check we avoid false positives.

# Disable 'gnu' warnings because we override the user variable 'YFLAGS'.
AUTOMAKE_fails -Wno-gnu Makefile5
grep -v '^Makefile5\.am:.*zardoz_YFLAGS' stderr \
 | grep -v ': warnings are treated as errors' \
 | grep . && exit 1

# Disable 'gnu' warnings because we override the user variable 'YFLAGS'.
$AUTOMAKE -Wno-gnu Makefile6

:
