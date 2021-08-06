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

# Make sure that Automake suggests using AM_PATH_PYTHON.

. test-init.sh

$ACLOCAL

echo 1. pythondir not defined

cat > Makefile.am <<'END'
PYTHON = x
python_PYTHON = foo.py
END

AUTOMAKE_fails -a
grep 'pythondir.*undefined' stderr
grep AM_PATH_PYTHON stderr


echo 2. pkgpythondir not defined

cat > Makefile.am <<'END'
PYTHON = x
pkgpython_PYTHON = foo.py
END

AUTOMAKE_fails -a
grep 'pkgpythondir.*undefined' stderr
grep AM_PATH_PYTHON stderr


echo 3. pyexecdir not defined

cat > Makefile.am <<'END'
PYTHON = x
pyexec_PYTHON = foo.py
END

AUTOMAKE_fails -a
grep 'pyexecdir.*undefined' stderr
grep AM_PATH_PYTHON stderr


echo 4. pkgpyexecdir not defined

cat > Makefile.am <<'END'
PYTHON = x
pkgpyexec_PYTHON = foo.py
END

AUTOMAKE_fails -a
grep 'pkgpyexecdir.*undefined' stderr
grep AM_PATH_PYTHON stderr


echo 5. PYTHON not defined.

cat > Makefile.am <<'END'
pkgpyexecdir = /here/we/go
pkgpyexec_PYTHON = foo.py
END

AUTOMAKE_fails -a
grep 'PYTHON.*undefined' stderr
grep AM_PATH_PYTHON stderr


echo 6. Ok.

cat > Makefile.am <<'END'
PYTHON = x
pkgpyexecdir = /here/we/go
pkgpyexec_PYTHON = foo.py
END

rm -f py-compile
$AUTOMAKE -a
test -f py-compile


echo 7. Ok again.

cat > Makefile.am <<'END'
PYTHON = x
fubardir = /here/we/go
fubar_PYTHON = foo.py
END

$AUTOMAKE -a

:
