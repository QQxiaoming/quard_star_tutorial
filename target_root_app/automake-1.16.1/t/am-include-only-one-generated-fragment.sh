#! /bin/sh
# Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

# Check that inclusion of only one '.am' that can be regenerated via
# a user-defined Makefile rule does not incur in spurious automake
# warnings about "target redefinition".

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
include foobar.am
$(srcdir)/foobar.am: $(srcdir)/touch.sh
	$(SHELL) $(srcdir)/touch.sh $(srcdir)/foobar.am
EXTRA_DIST = touch.sh
END

mkdir sub
cat > sub/Makefile.am <<'END'
include $(top_srcdir)/quux.am
$(top_srcdir)/quux.am: $(top_srcdir)/touch.sh
	$(SHELL) $(top_srcdir)/touch.sh $@
END

: > foobar.am
: > quux.am

$ACLOCAL
$AUTOMAKE

:
