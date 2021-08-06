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

# Test the '--destdir' option of the 'py-compile' script,

required=python
. test-init.sh

cp "$am_scriptdir/py-compile" . \
  || fatal_ "failed to fetch auxiliary script py-compile"

# Should not give false positives when grepping for it, so use
# an "uncommon" string (with ugly CamelCase).
destdir=TheDestDir

mkdir sub $destdir $destdir/sub
echo 'def foo (): return "foo"' > $destdir/foo.py
echo 'def bar (): return "bar"' > $destdir/sub/bar.py

./py-compile --destdir $destdir foo.py sub/bar.py

find $destdir # For debugging.
st=0
for x in c o; do
  for b in foo sub/bar; do
    f=$(pyc_location -p "$destdir/$b.py$x")
    test -f "$f"
    strings "$f" || : # For debugging.
    $FGREP $destdir $f && { echo BAD: $f; st=1; }
  done
done
exit $st

:
