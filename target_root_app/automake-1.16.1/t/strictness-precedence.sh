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

# On the command line, in AM_INIT_AUTOMAKE, and in AUTOMAKE_OPTIONS,
# strictness specified later should take precedence over strictness
# specified earlier.

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

# Leave out only one of the required files, to avoid too much
# repetition in the error messages.
touch INSTALL NEWS AUTHORS ChangeLog COPYING

$ACLOCAL
ko --foreign --gnu
ok --gnu --foreign

set_strictness '' Makefile.am
set_strictness 'gnu foreign' configure.ac
rm -rf autom4te*.cache
$ACLOCAL
ok
set_strictness 'foreign gnu' configure.ac
rm -rf autom4te*.cache
$ACLOCAL
ko

set_strictness '' configure.ac
rm -rf autom4te*.cache
$ACLOCAL
set_strictness 'gnu foreign' Makefile.am
ok
set_strictness 'foreign gnu' Makefile.am
ko

:
