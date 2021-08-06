#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Make sure that, if AC_CONFIG_AUX_DIR is not specified, Automake tries
# to use '.', '..' and '../..', in precisely that order.

. test-init.sh

nil=__no_such_program

unset NONESUCH

cat >>configure.ac << END
AM_MISSING_PROG([NONESUCH],[$nil])
AC_OUTPUT
END

mkdir d3
mkdir d3/d2
mkdir d3/d2/d1
mkdir d3/d2/d1/d0

echo 'echo %%d3%% $*' > d3/missing
chmod +x d3/missing
echo 'echo %%d2%% $*' > d3/d2/missing
chmod +x d3/d2/missing
echo 'echo %%d1%% $*' > d3/d2/d1/missing
chmod +x d3/d2/d1/missing
echo 'echo %%d0%% $*' > d3/d2/d1/d0/missing
chmod +x d3/d2/d1/d0/missing

mv configure.ac d3/d2/d1/d0/

cd d3/d2/d1/d0

cat > Makefile.am << 'EOF'
.PHONY: test
test:
	$(NONESUCH) >$(out)
EOF

$ACLOCAL
$AUTOCONF

# ------------------------------------------- #
:  We must end up with AC_CONFIG_AUX_DIR = .  #
# ------------------------------------------- #

: > install-sh
$AUTOMAKE
./configure
out=out0 $MAKE test
cat out0
grep "%%d0%%.*$nil" out0
grep '%%d[123]' out0 && exit 1

rm -f missing install-sh

# -------------------------------------------- #
:  We must end up with AC_CONFIG_AUX_DIR = ..  #
# -------------------------------------------- #

# Automake finds 'install-sh' in '.', so it assumes that auxdir is '.';
# but it won't find 'missing' in '.', so it will fail.
: > install-sh
AUTOMAKE_fails
grep 'required file.*[^.]\./missing.*not found' stderr
rm -f install-sh

# Now things should work.
: > ../install-sh
$AUTOMAKE
./configure
out=out1 $MAKE test
cat out1
grep "%%d1%%.*$nil" out1
grep '%%d[023]' out1 && exit 1

rm -f ../missing ../install-sh

# ----------------------------------------------- #
:  We must end up with AC_CONFIG_AUX_DIR = ../..  #
# ----------------------------------------------- #

# Automake finds 'install-sh' in '.', so it assumes that auxdir is '.';
# but it won't find 'missing' in '.', so it will fail.
: > install-sh
AUTOMAKE_fails
grep 'required file.*[^.]\./missing.*not found' stderr
rm -f install-sh

# Automake finds 'install-sh' in '..', so it assumes that auxdir is '..';
# but it won't find 'missing' in '.', so it will fail.
: > ../install-sh
AUTOMAKE_fails
grep 'required file.*[^.]\.\./missing.*not found' stderr
rm -f ../install-sh

# Now things should work.
: > ../../install-sh
$AUTOMAKE
./configure
out=out2 $MAKE test
cat out2
grep "%%d2%%.*$nil" out2
grep '%%d[013]' out2 && exit 1

rm -f ../../missing ../../install-sh

# --------------------------------------------------------- #
:  AC_CONFIG_AUX_DIR will not be found: Automake must fail  #
# --------------------------------------------------------- #

AUTOMAKE_fails
grep 'required file.*missing.*not found' stderr

:
