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

# Check that, on the command line, explicitly-defined warnings take
# precedence over implicit strictness-implied warnings.

. test-init.sh

# We want complete control over automake options.
AUTOMAKE=$am_original_AUTOMAKE

ok ()
{
  AUTOMAKE_run $*
  test ! -s stderr
}

ko ()
{
  AUTOMAKE_run $*
  grep '^Makefile\.am:.*:=.*not portable' stderr
  test $(wc -l <stderr) -eq 1
}

set_am_opts()
{
  set +x
  sed <$2 >$2-t -e "s|^\\(AUTOMAKE_OPTIONS\\) *=.*|\\1 = $1|" \
                -e "s|^\\(AM_INIT_AUTOMAKE\\).*|\\1([$1])|"
  mv -f $2-t $2
  set -x
  cat $2
}

# Files required in gnu strictness.
touch README INSTALL NEWS AUTHORS ChangeLog COPYING

cat > Makefile.am <<END
AUTOMAKE_OPTIONS =
FOO := bar
END

$ACLOCAL

ko --foreign -Wportability
ko -Wportability --foreign
ok --gnu -Wno-portability
ok -Wno-portability --gnu

set_am_opts '' Makefile.am

rm -rf autom4te*.cache
set_am_opts 'foreign -Wportability' configure.ac
ko
rm -rf autom4te*.cache
set_am_opts '-Wportability foreign' configure.ac
ko
rm -rf autom4te*.cache
set_am_opts 'gnu -Wno-portability' configure.ac
ok
rm -rf autom4te*.cache
set_am_opts '-Wno-portability gnu' configure.ac
ok

rm -rf autom4te*.cache
set_am_opts '' configure.ac

set_am_opts 'foreign -Wportability' Makefile.am
ko
set_am_opts '-Wportability foreign' Makefile.am
ko
set_am_opts 'gnu -Wno-portability' Makefile.am
ok
set_am_opts '-Wno-portability gnu' Makefile.am
ok

:
