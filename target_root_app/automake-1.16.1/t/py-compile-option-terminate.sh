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

# Check that a non-option argument and the '--' special argument
# explicitly terminate the option list for 'py-compile'.

required=python
. test-init.sh

cp "$am_scriptdir/py-compile" . \
  || fatal_ "failed to fetch auxiliary script py-compile"

: > ./-o.py
: > ./--foo.py
./py-compile -- -o.py --foo.py
py_installed ./-o.pyc
py_installed ./-o.pyo
py_installed ./--foo.pyc
py_installed ./--foo.pyo
rm -f ./-*.py[co]
: > x.py
./py-compile x.py -o.py --foo.py
py_installed ./x.pyc
py_installed ./x.pyo
py_installed ./-o.pyc
py_installed ./-o.pyo
py_installed ./--foo.pyc
py_installed ./--foo.pyo

:
