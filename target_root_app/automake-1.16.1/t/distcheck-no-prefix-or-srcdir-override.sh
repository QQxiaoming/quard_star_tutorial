#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Check that "make distcheck" overrides any --srcdir or --prefix flag
# (mistakenly) defined in $(AM_DISTCHECK_CONFIGURE_FLAGS) or
# $(DISTCHECK_CONFIGURE_FLAGS).  See automake bug#14991.

. test-init.sh

echo AC_OUTPUT >> configure.ac

orig_cwd=$(pwd); export orig_cwd

cat > Makefile.am << 'END'
# configure should choke on non-absolute prefix or non-existent
# srcdir.  We'll sanity-check that later.
AM_DISTCHECK_CONFIGURE_FLAGS = --srcdir am-src --prefix am-pfx
END

# Same comments as above applies.
DISTCHECK_CONFIGURE_FLAGS='--srcdir user-src --prefix user-pfx'
export DISTCHECK_CONFIGURE_FLAGS

$ACLOCAL
$AUTOMAKE
$AUTOCONF

# Sanity check: configure should choke on non-absolute prefix
# or non-existent srcdir.
./configure --prefix foobar 2>stderr && { cat stderr >&2; exit 99; }
cat stderr >&2
grep "expected an absolute directory name for --prefix" stderr || exit 99
./configure --srcdir foobar 2>stderr && { cat stderr >&2; exit 99; }
cat stderr >&2
grep "cannot find sources.* in foobar" stderr || exit 99

./configure
run_make -E -O distcheck
test ! -s stderr
# Sanity check: the flags have been actually seen.
$PERL -e 'undef $/; $_ = <>; s/ \\\n/ /g; print;' <stdout >t
grep '/configure .* --srcdir am-src'   t || exit 99
grep '/configure .* --prefix am-pfx'   t || exit 99
grep '/configure .* --srcdir user-src' t || exit 99
grep '/configure .* --prefix user-pfx' t || exit 99

:
