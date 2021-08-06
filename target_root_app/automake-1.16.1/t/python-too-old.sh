#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Test detection of too old Python, also forcing the python to use.
# See also related test 't/python-am-path-missing.sh'.

am_create_testdir=empty
required=python
. test-init.sh

py_too_old ()
{
  ./configure ${1+"PYTHON=$1"} >stdout 2>stderr && {
    cat stdout
    cat stderr >&2
    exit 1
  }
  cat stdout
  cat stderr >&2
  grep "whether $1 version is >= 9999\\.9\\.\\.\\. no *$" stdout
  grep '[Pp]ython interpreter is too old' stderr
}

saved_PYTHON=$PYTHON; export saved_PYTHON
unset PYTHON

cat > configure.ac <<END
AC_INIT([$me], [1.0])
# Hopefully the Python team will never release such a version.
AM_PATH_PYTHON([9999.9])
END

$ACLOCAL
$AUTOCONF

py_too_old python

mkdir bin
cat > bin/my-python << 'END'
#! /bin/sh
exec $saved_PYTHON ${1+"$@"}
END
chmod a+x bin/my-python
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH

py_too_old my-python

:
