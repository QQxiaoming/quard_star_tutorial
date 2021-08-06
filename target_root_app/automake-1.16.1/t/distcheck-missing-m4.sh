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

# Test that we can define a distcheck-hook to diagnose m4 files
# missing from the dist tarball (interaction with '--install').
# See automake bug#9037.

. test-init.sh

cwd=$(pwd) || fatal_ "cannot get current working directory"

cp "$am_testaux_srcdir"/distcheck-hook-m4.am . \
  || fatal_ "cannot fetch makefile fragment 'distcheck-hook-m4.am'"

cat > Makefile.am << 'END'
## The lack of '--install' here is meant.
ACLOCAL_AMFLAGS = -I m4
include $(srcdir)/distcheck-hook-m4.am
END

cat >> configure.ac << 'END'
dnl We *deliberately* don't use m4_pattern_forbid here.
AC_OUTPUT
MY_FOO
MY_BAR
MY_BAZ
MY_ZAR
END

mkdir m4 acdir acdir1 acdir2 pth

cat > acdir/dirlist << END
$cwd/acdir1
$cwd/acdir2
END

echo 'AC_DEFUN([MY_FOO], [:])' > m4/foo.m4
echo 'AC_DEFUN([MY_BAR], [:])' > acdir1/bar.m4
echo 'AC_DEFUN([MY_BAZ], [:])' > acdir1/baz.m4
echo 'AC_DEFUN([MY_QUX], [:])' > acdir2/qux.m4
echo 'AC_DEFUN([MY_ZAR], [:])' > pth/zar.m4
echo 'AC_DEFUN([MY_BLA], [:])' > pth/bla.m4

ACLOCAL="$ACLOCAL --system-acdir=$cwd/acdir"; export ACLOCAL
ACLOCAL_PATH=$cwd/pth; export ACLOCAL_PATH

# We don't use '--install' here.  Our distcheck-hook should catch this.
$ACLOCAL -I m4
$AUTOCONF
$EGREP 'MY_(FOO|BAR|BAZ|ZAR)' configure && exit 1 # Sanity check.
$AUTOMAKE

check_no_spurious_error ()
{
  $EGREP -i 'mkdir:|:.*(permission|denied)' output && exit 1
  # On failure, some make implementations (such as Solaris make) print the
  # whole failed recipe on stdout.  The first grep works around this.
  grep -v 'rm -rf ' output | grep -i 'autom4te.*\.cache' && exit 1
  : To placate 'set -e'.
}

./configure

run_make -M -e FAIL distcheck
for x in bar baz zar; do
  $EGREP "required m4 file.*not distributed.* $x.m4( |$)" output
done
check_no_spurious_error

# Now we use '--install', and "make distcheck" should pass.
$ACLOCAL -I m4 --install
test -f m4/bar.m4 # Sanity check.
test -f m4/baz.m4 # Likewise.
test -f m4/zar.m4 # Likewise.
using_gmake || $MAKE Makefile
$MAKE distcheck

# We start to use new "third-party" macros from new .m4 files, but forget
# to re-run "aclocal --install" by hand, relying on automatic remake rules.
# Our distcheck-hook should catch this too.
cat >> configure.ac << 'END'
MY_QUX
MY_BLA
END

$MAKE
$EGREP 'MY_(FOO|BAR|BAZ|QUX|ZAR|BLA)' configure && exit 1 # Sanity check.

run_make -M -e FAIL distcheck
$EGREP "required m4 file.*not distributed.* qux.m4( |$)" output
$EGREP "required m4 file.*not distributed.* bla.m4( |$)" output
check_no_spurious_error
# Check that we don't complain for files that should have been found.
$FGREP " (bar|baz|zar).m4" output && exit 1

# Now we again use '--install', and "make distcheck" should pass.
$ACLOCAL -I m4 --install
test -f m4/qux.m4 # Sanity check.
test -f m4/bla.m4 # Likewise.
using_gmake || $MAKE Makefile
$MAKE distcheck

:
