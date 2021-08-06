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

# -Werror and local -Werror settings should be flagged for the user.

. test-init.sh

cat >>configure.ac <<\END
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

mkdir sub
cat >sub/Makefile.am <<\END
AUTOMAKE_OPTIONS = -Werror
SUB = foo
SUB = bar
END
cat >Makefile.am <<\END
AUTOMAKE_OPTIONS = -Werror
VAR = foo
VAR = bar
END

$ACLOCAL

AUTOMAKE_fails -Wno-error
grep 'VAR multiply defined' stderr
grep 'SUB multiply defined' stderr
test $(grep -c 'warnings are treated as errors' stderr) -eq 1

sed '/AUTOMAKE_OPTIONS/d' sub/Makefile.am > t
mv -f t sub/Makefile.am

AUTOMAKE_fails -Wno-error
grep 'VAR multiply defined' stderr
grep 'SUB multiply defined' stderr
test $(grep -c 'warnings are treated as errors' stderr) -eq 1

sed '/AUTOMAKE_OPTIONS/d' Makefile.am > t
mv -f t Makefile.am

AUTOMAKE_fails -Werror
grep 'VAR multiply defined' stderr
grep 'SUB multiply defined' stderr
test $(grep -c 'warnings are treated as errors' stderr) -eq 1

AUTOMAKE_run -Wno-error
grep 'VAR multiply defined' stderr
grep 'SUB multiply defined' stderr
grep 'warnings are treated as errors' stderr && exit 1

:
