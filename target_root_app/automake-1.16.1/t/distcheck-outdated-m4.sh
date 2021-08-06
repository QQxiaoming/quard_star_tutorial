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

# Test that we can define a distcheck-hook to diagnose outdated m4
# files in a dist tarball (interaction with '--install').
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
AC_OUTPUT
MY_FOO
MY_BAR
MY_BAZ
END

mkdir m4 acdir
echo 'AC_DEFUN([MY_FOO], [:])' > m4/foo.m4
echo 'AC_DEFUN([MY_BAR], [:])' > acdir/bar.m4
cat > acdir/baz.m4 << 'END'
# serial 1
AC_DEFUN([MY_BAZ], [:])
END

ACLOCAL="$ACLOCAL --system-acdir=$cwd/acdir"; export ACLOCAL

# The use of '--install' here won't help when the installed file '.m4'
# will become out-of-date w.r.t. the one in the system acdir.
$ACLOCAL -I m4 --install
$AUTOCONF
$AUTOMAKE

./configure
$MAKE distcheck # Sanity check.

check_no_spurious_error ()
{
  $EGREP -i 'mkdir:|:.*(permission|denied)' output && exit 1
  # On failure, some make implementations (such as Solaris make) print the
  # whole failed recipe on stdout.  The first grep works around this.
  grep -v 'rm -rf ' output | grep -i 'autom4te.*\.cache' && exit 1
  : To placate 'set -e'.
}

# We start to use a new "third-party" macro in a new version
# of a pre-existing third-party m4 file, but forget to re-run
# "aclocal --install" by hand, relying on automatic remake
# rules.  Our distcheck-hook should catch this too.
echo MY_ZARDOZ >> configure.ac

cat > acdir/baz.m4 << 'END'
# serial 2
AC_DEFUN([MY_BAZ], [:])
AC_DEFUN([MY_ZARDOZ], [:])
END

$MAKE # Rebuild configure and makefiles.
run_make -M -e FAIL distcheck
$EGREP "required m4 file.* outdated.* baz.m4( |$)" output
check_no_spurious_error
# Check that we don't complain for files that aren't outdated.
$EGREP " (foo|bar).m4" output && exit 1

# Now we again use '--install' explicitly, and "make distcheck"
# should pass.
$ACLOCAL -I m4 --install
using_gmake || $MAKE Makefile
$MAKE distcheck

# Similar to what have been done above, but this time we:
#  - use ACLOCAL_PATH, and
#  - do not add the use of a new macro.

echo MY_FNORD >> configure.ac

mkdir pth
cat > pth/fnord.m4 << 'END'
# serial 1
AC_DEFUN([MY_FNORD], [:])
END

ACLOCAL_PATH="$cwd/pth"; export ACLOCAL_PATH

# The explicit use of '--install' here won't help when the installed file
# '.m4' will become out-of-date w.r.t. the one in the system acdir.
$ACLOCAL -I m4 --install
using_gmake || $MAKE Makefile
$MAKE distcheck

# Only increase serial number, without changing the other contents; this
# is deliberate.
cat > pth/fnord.m4 << 'END'
# serial 2
AC_DEFUN([MY_FNORD], [:])
END

$MAKE # Rebuild configure and makefiles.
run_make -M -e FAIL distcheck
$EGREP "required m4 file.* outdated.* fnord.m4( |$)" output
check_no_spurious_error
# Check that we don't complain for files that aren't outdated.
$EGREP " (foo|bar|baz).m4" output && exit 1

# Now we again use '--install' explicitly, and "make distcheck"
# should pass.
$ACLOCAL -I m4 --install
using_gmake || $MAKE Makefile
$MAKE distcheck

:
