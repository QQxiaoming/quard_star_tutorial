#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check parallel-tests features:
# - concurrent program compilation and testing (with EXTRA_PROGRAMS)
#
# Actually, this test doesn't ensure that things happen concurrently.
# It merely serves as demonstration.  :-)

required='cc native GNUmake'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = $(programs) $(scripts)
EXTRA_PROGRAMS = $(programs)
CLEANFILES = $(EXTRA_PROGRAMS)
dist_noinst_SCRIPTS = $(scripts)
programs =
scripts =
END

for i in 1 2 3 4 5 6 7 8; do
  echo "scripts += foo$i.test" >> Makefile.am
  echo "programs += foo$i.prog" >> Makefile.am
  unindent >foo$i.test <<'END'
    #! /bin/sh
    echo "this is $0"
    exit 0
END
  cat >foo$i.c <<'END'
    int main (void)
    {
      return 0;
    }
END
  chmod a+x foo$i.test
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE -j4 check
$MAKE distcheck

:
