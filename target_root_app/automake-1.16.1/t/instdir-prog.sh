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

# If $(bindir), $(libdir) or $(pyexecdir) is the empty string, then
# nothing should be installed there.
# This test exercises the prog and libs code paths.

required=cc
. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AM_PATH_PYTHON
AC_OUTPUT
END

mkdir sub

cat >Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = p
nobase_bin_PROGRAMS = np sub/np
lib_LIBRARIES = libfoo.a
nobase_lib_LIBRARIES = libnfoo.a sub/libnfoo.a
pyexec_PROGRAMS = py
nobase_pyexec_PROGRAMS = npy sub/npy
END

cat >p.c <<'END'
int main () { return 0; }
END
cp p.c np.c
cp p.c sub/np.c
cp p.c libfoo.c
cp p.c libnfoo.c
cp p.c sub/libnfoo.c
cp p.c py.c
cp p.c npy.c
cp p.c sub/npy.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

cwd=$(pwd) || fatal_ "getting current working directory"
instdir=$cwd/inst
destdir=$cwd/dest
mkdir build
cd build
../configure --prefix="$instdir" PYTHON="echo" \
             am_cv_python_pythondir="$instdir/python" \
             am_cv_python_pyexecdir="$instdir/pyexec"

nulldirs='bindir= libdir= pyexecdir='
null_install

:
