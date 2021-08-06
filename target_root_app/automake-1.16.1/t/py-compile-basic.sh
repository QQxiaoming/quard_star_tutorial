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

# Test more basic functionalities of the 'py-compile' script,
# with "believable" python sources.  See also related test
# 'py-compile-basic2.sh'.

required=python
. test-init.sh

cp "$am_scriptdir/py-compile" . \
  || fatal_ "failed to fetch auxiliary script py-compile"

cat > foo.py <<'END'
# Try out some non-trivial syntax in here.

'''Module docstring'''

def foo (*args, **kwargs):
  """Function docstring
  with embedded newline"""
  return 1

class Foo:
  r"""Class docstring"""
  def __init__(self):
    r'''Method docstring
        with
        embedded
        newlines'''
    pass

bar = baz = (1, (2,), [3, 4]); zardoz = 0;
END

mkdir sub
cat > sub/bar.py <<'END'
# Import of non-existent modules, or assertion of false conditions,
# shouldn't cause problems, as it should be enough for the code to
# be syntactically correct.
import Automake.No.Such.Module
assert False
END

# An empty file in a more deeply-nested directory should be ok as well.
mkdir -p 1/_/2/_/3/_
: > 1/_/2/_/3/_/0.py

./py-compile foo.py sub/bar.py 1/_/2/_/3/_/0.py
py_installed foo.pyc
py_installed foo.pyo
py_installed sub/bar.pyc
py_installed sub/bar.pyo
py_installed 1/_/2/_/3/_/0.pyc
py_installed 1/_/2/_/3/_/0.pyo

:
