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

# Test to make sure tags are processed also for files with non-standard
# extensions.  See automake bug#12372.

required='cc etags'
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_CONFIG_FILES([sub/Makefile])
# Fake linking.  Help avoid possible spurious errors from make
# or from the linker; errors that are irrelevant to this test.
AC_SUBST([LINK], ['echo $(CCLD) $(CFLAGS) $(LDFLAGS) -o $@'])
AC_OUTPUT
END

cat > Makefile.am <<'END'
all-local: tags
.pc.o:
	sed -e 's/\[/{/' -e 's/\]/}/' $(srcdir)/$*.pc >$*.c
	$(CC) $(DEFS) $(CPPFLAGS) $(CFLAGS) -c $*.c
	rm -f $*.c

noinst_PROGRAMS = foo
foo_SOURCES = foo-main.pc barbar.c
SUBDIRS = sub
END

mkdir sub
cat > sub/Makefile.am <<'END'
all-local: tags
.pc.o:
	sed -e 's/@/a/g' $(srcdir)/$*.pc >$*.c
	$(CC) $(DEFS) $(CPPFLAGS) $(CFLAGS) -c $*.c
	rm -f $*.c

noinst_PROGRAMS = zap
zap_SOURCES = zardoz.pc
END

echo 'int main(void) [ return bar(1); ]' > foo-main.pc
echo 'int bar(int x) { return !x; }' > barbar.c
echo 'int m@in(void) { return 0; }' > sub/zardoz.pc

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# Exuberant Ctags (at least version 5.8) doesn't generate any tags
# for file extensions it doesn't recognize.  This can be fixed by
# using the '--langmap' option.  But we must be careful, because
# etags from Emacs (at least version 23.4) doesn't recognize that
# option.
if etags --help | grep '.*--langmap'; then
  run_make ETAGSFLAGS="--langmap=c:+.pc"
else
  $MAKE
fi

cat TAGS
cat sub/TAGS
$FGREP foo-main.pc TAGS
$FGREP barbar.c TAGS
$FGREP zardoz.pc sub/TAGS

$MAKE distcheck

:
