#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Test and that vapi files are correctly handled by Vala support.

required='pkg-config valac cc GNUmake'
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_VALAC([0.7.3])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_OUTPUT
END

cat > Makefile.am <<'END'
bin_PROGRAMS = zardoz
AM_CFLAGS = $(GOBJECT_CFLAGS)
LDADD = $(GOBJECT_LIBS)
zardoz_SOURCES = zardoz.vala foo.vapi foo.h
END

cat > zardoz.vala <<'END'
using GLib;
public class Zardoz {
  public static void main () {
    stdout.printf (BARBAR);
  }
}
END

# Use printf, not echo, to avoid '\n' being considered and escape
# sequence and printed as a newline in 'foo.h'.
printf '%s\n' '#define BARBAR "Zardoz!\n"' > foo.h

cat > foo.vapi <<'END'
[CCode (cprefix="", lower_case_cprefix="", cheader_filename="foo.h")]
public const string BARBAR;
END

if ! cross_compiling; then
  unindent >> Makefile.am <<'END'
    check-local: test2
    .PHONY: test1 test2
    test1:
	./zardoz
	./zardoz | grep 'Zardoz!'
    test2:
	./zardoz
	./zardoz | grep 'Quux!'
END
fi

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

./configure --enable-dependency-tracking

$MAKE
ls -l        # For debugging.
cat zardoz.c # Likewise.
grep 'BARBAR' zardoz.c
cross_compiling || $MAKE test1 || exit 1

# Simple check on remake rules.
$sleep
# Use printf, not echo, to avoid '\n' being considered and escape
# sequence and printed as a newline in 'foo.h'.
printf '%s\n' '#define BAZBAZ "Quux!\n"' > foo.h
sed 's/BARBAR/BAZBAZ/' zardoz.vala > t && mv -f t zardoz.vala || exit 99
$MAKE && exit 1
sed 's/BARBAR/BAZBAZ/' foo.vapi > t && mv -f t foo.vapi || exit 99
$MAKE
cat zardoz.c # For debugging.
grep 'BAZBAZ' zardoz.c
cross_compiling || $MAKE test2 || exit 1

# Check the distribution.
$MAKE distcheck

:
