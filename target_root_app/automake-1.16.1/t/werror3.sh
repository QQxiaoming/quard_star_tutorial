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

# Make sure a per-Makefile.am -Werror setting is not carried over
# to another Makefile.am.

. test-init.sh

cat >>configure.ac <<\END
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

mkdir sub
cat >sub/Makefile.am <<\END
AUTOMAKE_OPTIONS = -Werror
END
cat >Makefile.am <<\END
VAR = foo
VAR = bar
END

$ACLOCAL

# The issue would not manifest with threaded execution.
unset AUTOMAKE_JOBS
AUTOMAKE_run -Wno-error
grep 'VAR multiply defined' stderr

AUTOMAKE_JOBS=2
export AUTOMAKE_JOBS
AUTOMAKE_run -Wno-error
grep 'VAR multiply defined' stderr

:
