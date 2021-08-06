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

# Check that the default warnings triggered by a strictness specified
# in AUTOMAKE_OPTIONS take precedence over explicit warnings given in
# AM_INIT_AUTOMAKE.

. test-init.sh

# We want (almost) complete control over automake options.
AUTOMAKE="$am_original_AUTOMAKE -Werror"

cat > Makefile.am <<END
AUTOMAKE_OPTIONS =
FOO := bar
END

set_am_opts ()
{
  set +x
  sed <$2 >$2-t -e "s|^\\(AUTOMAKE_OPTIONS\\) *=.*|\\1 = $1|" \
                -e "s|^\\(AM_INIT_AUTOMAKE\\).*|\\1([$1])|"
  mv -f $2-t $2
  set -x
  cat $2
}

set_am_opts '-Wportability' configure.ac
set_am_opts 'foreign' Makefile.am

$ACLOCAL
$AUTOMAKE

rm -rf autom4te*.cache

# Files required in gnu strictness.
touch README INSTALL NEWS AUTHORS ChangeLog COPYING

set_am_opts '-Wno-portability' configure.ac
set_am_opts 'gnu' Makefile.am

AUTOMAKE_fails
$ACLOCAL
grep '^Makefile\.am:2:.*:=.*not portable' stderr

:
