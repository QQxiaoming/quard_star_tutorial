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

# The stub rules emitted to work around the "deleted header problem"
# for '.am' files shouldn't prevent "make" from diagnosing a missing
# required '.am' file from a distribution tarball.
# See discussion about automake bug#9768.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
include $(srcdir)/foobar.am
include $(srcdir)/zardoz.am
END

: > foobar.am
: > zardoz.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# A faulty distribution tarball, with a required '.am' file missing.
# Building from it should fail, both for in-tree and VPATH builds.
ocwd=$(pwd) || fatal_ "cannot get current working directory"
for vpath in false :; do
  $MAKE distdir
  test -f $distdir/zardoz.am # Sanity check.
  rm -f $distdir/zardoz.am
  if $vpath; then
    # We can't just build in a subdirectory of $distdir, otherwise
    # we'll hit automake bug#10111.
    mkdir vpath-distcheck
    cd vpath-distcheck
    ../$distdir/configure
  else
    cd $distdir
    ./configure
  fi
  run_make -e FAIL -M
  # This error comes from automake, not make, so we can be stricter
  # in our grepping of it.
  grep 'cannot open.*zardoz\.am' output
  grep 'foobar\.am' output && exit 1 # No spurious error, please.
  cd "$ocwd" || fatal_ "cannot chdir back to top-level test directory"
done

:
