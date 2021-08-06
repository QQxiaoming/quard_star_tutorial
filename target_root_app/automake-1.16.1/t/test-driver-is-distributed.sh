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

# Check regression of parallel-tests:
#  - 'test-driver' script not correctly distributed when TESTS is
#    defined in a subdir Makefile

am_create_testdir=empty
. test-init.sh

ocwd=$(pwd) || fatal_ "getting current working directory"

for i in 1 2; do
  mkdir D$i
  cd D$i
  if test $i -eq 1; then
    auxdir=.
    test_driver=test-driver
  else
    auxdir=build-aux
    test_driver=$auxdir/test-driver
    mkdir $auxdir
  fi
  mkdir tests
  unindent > configure.ac << END
    AC_INIT([$me], [1.0])
    AC_CONFIG_AUX_DIR([$auxdir])
    AM_INIT_AUTOMAKE
    AC_CONFIG_FILES([Makefile tests/Makefile])
    AC_OUTPUT
END
  unindent > Makefile.am << END
    SUBDIRS = tests
    check-local: test-top
    test-top: distdir
	ls -l \$(distdir) \$(distdir)/* ;: For debugging.
	test -f \$(distdir)/$test_driver
    .PHONY: test-top
END
  unindent > tests/Makefile.am << END
    check-local: test-sub
    test-sub:
	echo ' ' \$(DIST_COMMON) ' ' | grep '[ /]$test_driver '
    TESTS = foo.test
    EXTRA_DIST = \$(TESTS)
END
  unindent > tests/foo.test << 'END'
    #!/bin/sh
    exit 0
END
  chmod a+x tests/foo.test
  $ACLOCAL
  $AUTOCONF
  $AUTOMAKE -a
  ./configure
  $MAKE test-top
  cd tests
  $MAKE test-sub
  cd ..
  $MAKE distcheck
  # Try code path without automatic installation of required files.
  mv -f Makefile.in Makefile.sav
  mv -f tests/Makefile.in tests/Makefile.sav
  $AUTOMAKE
  diff Makefile.in Makefile.sav
  diff tests/Makefile.in tests/Makefile.sav
  cd "$ocwd" || fatal_ "cannot chdir back to '$ocwd'"
done

:
