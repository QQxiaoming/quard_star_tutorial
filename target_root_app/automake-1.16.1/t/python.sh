#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test to make sure py_compile and am__py_compile are correctly defined.

. test-init.sh

echo 'AM_PATH_PYTHON' >> configure.ac

cat > Makefile.am << 'END'
python_PYTHON = foo.py
END

cat > Makefile2.am << 'END'
python_PYTHON = a.py
nodist_python_PYTHON = b.py
nobase_python_PYTHON = x/c.py
mydir = ${prefix}
my_PYTHON = d.py
END

# For automake bug#10995.
cat > Makefile3.am << 'END'
noinst_PYTHON = un.py
python_PYTHON = in.py
END

$ACLOCAL

AUTOMAKE_fails
grep '^Makefile\.am:1:.* required file.*py-compile' stderr

$AUTOMAKE -a
grep '^py_compile *=' Makefile.in
test -f py-compile

$AUTOMAKE Makefile2
$AUTOMAKE Makefile3

grep "py" Makefile.in Makefile2.in Makefile3.in # For debugging.

for f in Makefile.in Makefile2.in Makefile3.in; do
  test $(grep -c '^py_compile =' $f) -eq 1
  test $(grep -c '^am__py_compile =' $f) -eq 1
done

:
