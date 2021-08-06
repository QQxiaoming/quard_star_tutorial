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

# Test 'py-compile --help', 'py-compile --version', and that 'py-compile'
# correctly complains on wrong usage.

. test-init.sh

cp "$am_scriptdir/py-compile" . \
  || fatal_ "failed to fetch auxiliary script py-compile"

# --help

./py-compile --help >stdout 2>stderr \
  || { cat stdout; cat stderr >&2; exit 1; }
cat stdout
test -s stderr && { cat stderr >&2; exit 1; }
grep '^Usage: py-compile .' stdout
$FGREP ' [--basedir DIR]' stdout
$FGREP ' [--destdir DIR]' stdout

# --version

./py-compile --version >stdout 2>stderr \
  || { cat stdout; cat stderr >&2; exit 1; }
cat stdout
test -s stderr && { cat stderr >&2; exit 1; }
year='20[0-9][0-9]' # Hopefully automake will be obsolete in 80 years ;-)
month='(0[0-9]|1[012])'
day='([012][0-9]|3[01])'
hour='([01][0-9]|2[0123])'
LC_ALL=C $EGREP "^py-compile $year-$month-$day\.$hour" stdout
test $(wc -l <stdout) -eq 1

# Unknown option.

for opt in -b -d --foo; do
  ./py-compile $opt 2>stderr && { cat stderr >&2; exit 1; }
  cat stderr >&2
  grep "^py-compile: unrecognized option '$opt'" stderr
  grep "^Try 'py-compile --help' for more information" stderr
done

# Missing option argument.

for opt in --basedir --destdir; do
  ./py-compile $opt 2>stderr && { cat stderr >&2; exit 1; }
  cat stderr >&2
  grep "^py-compile: option '$opt' requires an argument" stderr
  grep "^Try 'py-compile --help' for more information" stderr
done

# Missing files.

for args in '' '--basedir dir' '--destdir dir'; do
  ./py-compile $args 2>stderr && { cat stderr >&2; exit 1; }
  cat stderr >&2
  grep '^py-compile: no files given' stderr
  grep "^Try 'py-compile --help' for more information" stderr
done

:
