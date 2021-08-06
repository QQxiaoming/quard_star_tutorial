#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Check that AM_INSTALLCHECK_STD_OPTIONS_EXEMPT works.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

# We use the same basename for all targets on purpose.  This way
# we make sure that 'AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = nok'
# will not match anything containing 'nok'.
cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = gnits

nobase_bin_PROGRAMS = nok sub/nok
nok_SOURCES = nok.c
sub_nok_SOURCES = nok.c

nobase_bin_SCRIPTS = nok.sh sub/nok.sh

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = nok$(EXEEXT) nok.sh

grep-stderr:
	grep 'sub/pnok$(EXEEXT) does not support' stderr
	grep 'sub/pnok\.sh does not support' stderr
## Only two failures please.
	test `grep 'does not support --help' stderr | wc -l` = 2
	test `grep 'does not support --version' stderr | wc -l` = 2
END

cat > nok.c <<'END'
int main (void)
{
  return 0;
}
END

mkdir sub

cat >nok.sh <<EOF
#!/bin/sh
echo "Which version? Which usage?"
exit 1
EOF

cp nok.sh sub/nok.sh

chmod +x nok.sh
chmod +x sub/nok.sh

# Files required by Gnits.
: > INSTALL
: > NEWS
: > README
: > COPYING
: > AUTHORS
: > ChangeLog
: > THANKS

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkdir build
cd build

# Use --program-prefix to make sure the std-options check honors it.
../configure "--prefix=$(pwd)/../inst-dir" --program-prefix=p
$MAKE
$MAKE install
$MAKE installcheck && exit 1
# Don't trust th exit status of "make -k" for non-GNU makes.
if using_gmake; then status=FAIL; else status=IGNORE; fi
run_make -e $status -E -- -k installcheck
$MAKE grep-stderr

# Make sure there is no more error when all targets are exempted.
cd ..
cat >> Makefile.am <<'END'
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT += sub/nok$(EXEEXT) sub/nok.sh
END
$AUTOMAKE
cd build
./config.status  # Don't rely on the rebuild rules (they need GNU make).
$MAKE installcheck

:
