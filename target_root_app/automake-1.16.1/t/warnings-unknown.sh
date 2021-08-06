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

# Check that automake complains about unknown warnings.

. test-init.sh

# We want (almost) complete control over automake options.
AUTOMAKE="$am_original_AUTOMAKE --foreign -Werror"

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([-Wno-zardoz])
AC_CONFIG_FILES([Makefile])
END

cat > Makefile.am <<END
AUTOMAKE_OPTIONS = -Wfoobar
END

$ACLOCAL

AUTOMAKE_fails -Wbazquux
grep '^configure\.ac:2:.* unknown warning category.*zardoz' stderr
grep '^Makefile\.am:1:.* unknown warning category.*foobar' stderr
grep "^automake-$APIVERSION:.* unknown warning category.*bazquux" stderr

# Check that we can override warnings about unknown warning categories.
$AUTOMAKE -Wno-unsupported -Wbazquux

:
