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

# Make sure the file name translation in the 'compile' script works
# correctly

. test-init.sh

get_shell_script compile

# Use a dummy cl, since cl isn't readily available on all systems
cat >cl <<'END'
#! /bin/sh
echo "$@"
END

chmod +x ./cl

cat >>configure.ac << 'END'
AC_CANONICAL_HOST
AC_CONFIG_FILES([check_host], [chmod +x check_host])
AC_OUTPUT
END

: >Makefile.am

# This will be sourced, nor executed.
cat >check_host.in << 'END'
case '@host_os@' in
  mingw*)
    ;;
  *)
    skip_ "target OS is not MinGW"
    ;;
esac
case @build_os@ in
  mingw* | cygwin*)
    ;;
  *)
    winepath -w / \
      || skip_ "not on MinGW or Cygwin, and winepath not available"
    ;;
esac
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
. ./check_host

cwd=$(pwd) || fatal_ "cannot get current directory"

# POSIX mandates that the compiler accepts a space between the -I,
# -l and -L options and their respective arguments.  Traditionally,
# this should work also without a space.  Try both usages.
for sp in '' ' '; do
  # Check if "compile cl" transforms absolute file names to
  # host format (e.g /somewhere -> c:/msys/1.0/somewhere).
  res=$(./compile ./cl -L${sp}"$cwd" | sed -e 's/-link -LIBPATH://')
  case $res in
    ?:[\\/]*) ;;
    *) exit 1 ;;
  esac
done

:
