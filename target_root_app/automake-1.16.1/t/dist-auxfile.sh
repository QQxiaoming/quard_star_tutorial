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

# Files specified by AC_REQUIRE_AUX_FILE must exist, and they get
# automatically distributed by the top-level Makefile.am.
# Automake bug#9651 reported that this didn't happen when the build-aux
# directory was set to '.' by AC_CONFIG_AUX_DIR.

am_create_testdir=empty
. test-init.sh

i=0
for auxdir in build-aux ''; do

  echo "*** Testing with auxdir '$auxdir' ***"

  i=$(($i + 1))
  mkdir T$i.d
  cd T$i.d

  unindent > configure.ac <<END
    AC_INIT([$me], [1.0])
    AC_CONFIG_AUX_DIR([${auxdir:-.}])
    AM_INIT_AUTOMAKE
    AC_REQUIRE_AUX_FILE([foo.txt])
    AC_REQUIRE_AUX_FILE([bar.sh])
    AC_REQUIRE_AUX_FILE([zardoz])
    AC_CONFIG_FILES([Makefile subdir/Makefile])
    AC_OUTPUT
END

  echo auxdir = ${auxdir:+"$auxdir/"} > Makefile.am
  unindent >> Makefile.am <<'END'
    SUBDIRS = subdir
    test: distdir
	ls -l $(distdir) $(distdir)/*      ;: For debugging.
	@echo DIST_COMMON = $(DIST_COMMON) ;: Likewise.
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]$(auxdir)foo.txt '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]$(auxdir)bar.sh '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]$(auxdir)zardoz '
	test -f $(distdir)/$(auxdir)foo.txt
	test -f $(distdir)/$(auxdir)bar.sh
	test -f $(distdir)/$(auxdir)zardoz
    .PHONY: test
    check-local: test
END

  mkdir subdir
  : > subdir/Makefile.am

  test -z "$auxdir" || mkdir "$auxdir" || exit 99

  if test -z "$auxdir"; then
    auxdir=.
    auxdir_rx='\.'
  else
    auxdir_rx=$auxdir
  fi

  $ACLOCAL
  $AUTOCONF

  AUTOMAKE_fails
  grep "^configure\\.ac:4:.* required file.*$auxdir_rx/foo\\.txt" stderr
  grep "^configure\\.ac:5:.* required file.*$auxdir_rx/bar\\.sh"  stderr
  grep "^configure\\.ac:6:.* required file.*$auxdir_rx/zardoz"    stderr

  touch $auxdir/foo.txt $auxdir/bar.sh $auxdir/zardoz
  cp "$am_scriptdir"/missing "$am_scriptdir"/install-sh $auxdir \
    || fatal_ "fetching auxiliary scripts 'missing' and 'install-sh'"
  $AUTOMAKE
  ./configure
  $MAKE test
  $EGREP '(foo\.txt|bar\.sh|zardoz)' subdir/Makefile.in && exit 1

  $MAKE distcheck

  cd ..

done

:
