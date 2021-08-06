#!/bin/sh
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

# Check for bug in variable concatenation with '+=': an extra backslash
# is erroneously retained in the final value.
# See also sister test 'pluseq11b.sh'.

. test-init.sh

cat >>configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
## Use more line continuation to ensure we are robust and can (hopefully)
## cope any number of them, and not just one
FOO = \
\
\
bar
## Both these two variable additions are required to trigger the bug.
FOO +=
FOO += baz

.PHONY: test
test:
	case '$(FOO)' in *\\*) exit 1;; *) exit 0;; esac
END

$ACLOCAL
$AUTOMAKE

grep '^ *FOO *=.*\\.' Makefile.in && exit 1

$AUTOCONF
./configure
$MAKE test

:
