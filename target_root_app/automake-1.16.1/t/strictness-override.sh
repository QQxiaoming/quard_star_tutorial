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

# The strictness specified in Makefile.am:AUTOMAKE_OPTIONS should
# override that specified in configure.ac:AM_INIT_AUTOMAKE, and both
# should override the strictness specified on the command line.
# NOTE: the current semantics might not be the best one (even if it has
# been in place for quite a long time); see also Automake bug #7673.
# Update this test if the semantics are changed.

. test-init.sh

# We want complete control over automake options.
AUTOMAKE=$am_original_AUTOMAKE

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS =
END

set_strictness ()
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
  $AUTOMAKE -Werror $*
}

ko ()
{
  AUTOMAKE_fails $*
  grep 'required file.*README' stderr
}

$ACLOCAL

# Leave out only one of the required files, to avoid too much
# repetition in the error messages.
touch INSTALL NEWS AUTHORS ChangeLog COPYING

rm -rf autom4te*.cache
set_strictness '' Makefile.am
set_strictness '' configure.ac
ko --gnu
ko
ok --foreign

rm -rf autom4te*.cache
set_strictness 'gnu' Makefile.am
set_strictness '' configure.ac
ko --gnu
ko
ko --foreign

rm -rf autom4te*.cache
set_strictness '' Makefile.am
set_strictness 'gnu' configure.ac
ko --gnu
ko
ko --foreign

rm -rf autom4te*.cache
set_strictness 'foreign' Makefile.am
set_strictness '' configure.ac
ok --gnu
ok
ok --foreign

rm -rf autom4te*.cache
set_strictness '' Makefile.am
set_strictness 'foreign' configure.ac
ok --gnu
ok
ok --foreign

rm -rf autom4te*.cache
set_strictness 'gnu' Makefile.am
set_strictness 'gnu' configure.ac
ko --gnu
ko
ko --foreign

rm -rf autom4te*.cache
set_strictness 'foreign' Makefile.am
set_strictness 'foreign' configure.ac
ok --gnu
ok
ok --foreign

rm -rf autom4te*.cache
set_strictness 'foreign' Makefile.am
set_strictness 'gnu' configure.ac
ok --gnu
ok
ok --foreign

rm -rf autom4te*.cache
set_strictness 'gnu' Makefile.am
set_strictness 'foreign' configure.ac
ko --gnu
ko
ko --foreign

:
