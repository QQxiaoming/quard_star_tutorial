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

# Make sure that our macro 'AM_MAINTAINER_MODE' adds proper text to
# the configure help screen.  Also make sure the "checking ..." messages
# related to the enabling/disabling of maintainer mode are correct (see
# automake bug#9890).

. test-init.sh

set_maintmode ()
{
  rm -rf autom4te*.cache # Just to be sure not to use a stale cache.
  echo "AC_INIT([$me], [1.0])" > configure.ac
  case $1 in
    DEFAULT) echo AM_MAINTAINER_MODE;;
          *) echo "AM_MAINTAINER_MODE([$*])";;
  esac >> configure.ac
}

check_configure_message_with ()
{
  answer=$1; shift
  ./configure ${1+"$@"} >stdout || { cat stdout; exit 1; }
  cat stdout
  grep "^checking whether to enable maintainer-specific.*\\.\\.\\. $answer$" stdout
  test $(grep -c 'checking.*maint' stdout) -eq 1
}

set_maintmode "DEFAULT"

$ACLOCAL

$AUTOCONF --force

grep_configure_help --enable-maintainer-mode 'enable make rules'

check_configure_message_with "no"
check_configure_message_with "yes" --enable-maintainer-mode

set_maintmode "disable"

$AUTOCONF --force
grep_configure_help --enable-maintainer-mode 'enable make rules'

check_configure_message_with "no"
check_configure_message_with "yes" --enable-maintainer-mode

set_maintmode "enable"

$AUTOCONF --force
./configure --help >stdout || { cat stdout; exit 1; }
cat stdout
grep_configure_help --disable-maintainer-mode 'disable make rules'

check_configure_message_with "yes"
check_configure_message_with "no" --disable-maintainer-mode

:
