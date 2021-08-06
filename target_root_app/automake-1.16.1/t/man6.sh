#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Ensure 'make distcheck' passes when help2man generates man pages,
# even if the 'missing' script is involved.

required=help2man
. test-init.sh

# Avoid a spurious failure due to a known FreeBSD make incompatibility.
useless_vpath_rebuild \
  && skip_ "VPATH useless rebuild detected (see bug#7884)"

cat > Makefile.am << 'END'
dist_man_MANS = $(srcdir)/foobar.1 bazquux.1 zardoz.1
dist_bin_SCRIPTS = foobar bazquux zardoz
$(srcdir)/foobar.1:
	$(HELP2MAN) --output=$@ $(srcdir)/foobar
bazquux.1:
	$(HELP2MAN) --output=$@ $(srcdir)/bazquux
zardoz.1:
	$(HELP2MAN) --output=$(srcdir)/zardoz.1 $(srcdir)/zardoz
END

cat >> configure.ac <<'END'
AM_MISSING_PROG([HELP2MAN], [help2man])
AC_OUTPUT
END

cat > foobar <<'END'
#! /bin/sh
while test $# -gt 0; do
  case $1 in
    -h | --help) echo "usage: $0 [OPTIONS]..."; exit 0;;
    -v | --version) echo "$0 1.0"; exit 0;;
  esac
  shift
done
END

cp foobar bazquux
cp foobar zardoz

chmod +x foobar bazquux zardoz

$ACLOCAL
$AUTOMAKE
$AUTOCONF

mkdir build
cd build

../configure
# Sanity check.
grep '^HELP2MAN *=.*/missing help2man' Makefile

$MAKE
$FGREP foobar ../foobar.1
$FGREP bazquux ./bazquux.1
$FGREP zardoz ../zardoz.1

$MAKE distdir
$FGREP foobar  $me-1.0/foobar.1
$FGREP bazquux $me-1.0/bazquux.1
$FGREP zardoz  $me-1.0/zardoz.1

$MAKE distcheck

cd ..
rm -f *.1 # Remove leftover generated manpages.

./configure
# Sanity check.
grep '^HELP2MAN *=.*/missing help2man' Makefile

$MAKE
$FGREP foobar  foobar.1
$FGREP bazquux bazquux.1
$FGREP zardoz  zardoz.1

$MAKE distdir
$FGREP foobar  $me-1.0/foobar.1
$FGREP bazquux $me-1.0/bazquux.1
$FGREP zardoz  $me-1.0/zardoz.1

$MAKE distcheck

:
