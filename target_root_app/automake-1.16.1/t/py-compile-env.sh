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

# Make sure 'py-compile' honours the PYTHON environment variable.

. test-init.sh

cp "$am_scriptdir/py-compile" . \
  || fatal_ "failed to fetch auxiliary script py-compile"

cat > my-py <<'END'
#!/bin/sh
: > my-py.run
END
chmod a+x my-py

mkdir sub1
cd sub1

PYTHON=: ../py-compile foo.py
ls | grep . && exit 1

PYTHON=false ../py-compile foo.py && exit 1
ls | grep . && exit 1

PYTHON='echo GrEpMe AndMeToo' ../py-compile foo.py
PYTHON='echo GrEpMe AndMeToo' ../py-compile foo.py | grep 'GrEpMe AndMeToo'
ls | grep . && exit 1

cd ..
mkdir sub2
cd sub2

PYTHON=../my-py ../py-compile foo.py
test -f my-py.run
ls | grep -v '^my-py\.run$' | grep . && exit 1

cd ..
mkdir sub3
cd sub3
PATH=..$PATH_SEPARATOR$PATH; export PATH
PYTHON=my-py py-compile foo.py
test -f my-py.run
ls | grep -v '^my-py\.run$' | grep . && exit 1

:
