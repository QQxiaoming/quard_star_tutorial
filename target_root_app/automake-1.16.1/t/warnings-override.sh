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

# The warnings specified in Makefile.am:AUTOMAKE_OPTIONS should override
# those specified in configure.ac:AM_INIT_AUTOMAKE, and both should
# override the warnings specified on the command line.
# NOTE: the current semantics might not be the best one (even if it has
# been in place for quite a long time); see also Automake bug #7673.
# Update this test if the semantics are changed.

. test-init.sh

# We want (almost) complete control over automake options.
AUTOMAKE="$am_original_AUTOMAKE -Werror"

cat > Makefile.am <<'END'
FOO := bar
AUTOMAKE_OPTIONS =
END

set_warnings ()
{
  set +x
  sed <$2 >$2-t -e "s|^\\(AUTOMAKE_OPTIONS\\) *=.*|\\1 = $1|" \
                -e "s|^\\(AM_INIT_AUTOMAKE\\).*|\\1([$1])|"
  mv -f $2-t $2
  set -x
  cat $2
}

ok ()
{
  $AUTOMAKE $*
}

ko ()
{
  AUTOMAKE_fails $*
  grep '^Makefile\.am:1:.*:=.*not portable' stderr
}

$ACLOCAL

# Files required in gnu strictness.
touch README INSTALL NEWS AUTHORS ChangeLog COPYING

rm -rf autom4te*.cache
set_warnings '-Wno-portability' Makefile.am
set_warnings '' configure.ac

ok -Wportability
ok

rm -rf autom4te*.cache
set_warnings '' Makefile.am
set_warnings '-Wno-portability' configure.ac

ok -Wportability
ok

rm -rf autom4te*.cache
set_warnings '-Wno-portability' Makefile.am
set_warnings '-Wno-portability' configure.ac

ok -Wportability

rm -rf autom4te*.cache
set_warnings '-Wportability' Makefile.am
set_warnings '' configure.ac

ko
ko -Wno-portability

rm -rf autom4te*.cache
set_warnings '' Makefile.am
set_warnings '-Wportability' configure.ac

ko
ko -Wno-portability

rm -rf autom4te*.cache
set_warnings '-Wportability' Makefile.am
set_warnings '-Wportability' configure.ac
ko -Wno-portability

rm -rf autom4te*.cache
set_warnings '-Wno-portability' Makefile.am
set_warnings '-Wportability' configure.ac
ok
ok -Wportability

rm -rf autom4te*.cache
set_warnings '-Wportability' Makefile.am
set_warnings '-Wno-portability' configure.ac
ko
ko -Wno-portability

:
