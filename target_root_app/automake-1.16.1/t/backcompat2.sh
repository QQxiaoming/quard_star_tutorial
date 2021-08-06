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

# Backward-compatibility test: check that AM_INIT_AUTOMAKE with two or
# three arguments does AC_DEFINE the symbols PACKAGE and VERSION iff the
# third argument is empty or non-existent.

am_create_testdir=empty
. test-init.sh

# A trick to make the test run muuuch faster, by avoiding repeated
# runs of aclocal (one order of magnitude improvement in speed!).
echo 'AC_INIT(x,0) AM_INIT_AUTOMAKE' > configure.ac
$ACLOCAL
rm -rf configure.ac autom4te.*

touch install-sh missing

cat > config.h.in <<'END'
#undef PACKAGE
#undef VERSION
END

for am_arg3 in ':' 'false' '#' ' '; do
  unindent > configure.ac <<END
    AC_INIT
    AC_CONFIG_HEADERS([config.h])
    AM_INIT_AUTOMAKE([pkgname], [pkgversion], [$am_arg3])
    AC_OUTPUT
END
  cat configure.ac # For debugging.
  $AUTOCONF
  ./configure
  cat config.h # For debugging.
  # The non-empty third argument should prevent PACKAGE and VERSION
  # from being AC_DEFINE'd.
  $EGREP 'pkg(name|version)' config.h && exit 1
  # This is required because even relatively-recent versions of the
  # BSD shell wrongly exit when the 'errexit' shell flag is active if
  # the last command of a compound statement fails, even if it should
  # be protected by the use of "&&".
  :
done

for am_extra_args in '' ',' ', []'; do
  unindent > configure.ac <<END
    AC_INIT
    AC_CONFIG_HEADERS([config.h])
    AM_INIT_AUTOMAKE([pkgname], [pkgversion]$am_extra_args)
    AC_OUTPUT
END
  cat configure.ac # For debugging.
  $AUTOCONF
  ./configure
  cat config.h # For debugging.
  grep '^ *# *define  *PACKAGE  *"pkgname" *$' config.h
  grep '^ *# *define  *VERSION  *"pkgversion" *$' config.h
done

:
