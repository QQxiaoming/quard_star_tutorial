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

# Check that config.h.bot and config.h.top are automatically
# distributed if the AC_CONFIG_HADERS macro is used and they
# exist at automake runtime.
# Related to automake bug#7819.

. test-init.sh

cat >> configure.ac <<END
AC_CONFIG_HEADERS([config.h sub/config.h cfg2.h:conf2.hin])
AC_OUTPUT
END

cat > Makefile.am <<'END'
.PHONY: test
test: distdir
	ls -l $(distdir) $(distdir)/sub
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]config\.h\.bot '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]config\.h\.top '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]cfg2\.h\.bot '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]cfg2\.h\.top '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]sub/config\.h\.bot '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]sub/config\.h\.top '
	test -f $(distdir)/config.h.bot
	test -f $(distdir)/config.h.top
	test -f $(distdir)/cfg2.h.bot
	test -f $(distdir)/cfg2.h.top
	test -f $(distdir)/sub/config.h.bot
	test -f $(distdir)/sub/config.h.top
END

mkdir sub
touch config.h.in config.h.top config.h.bot \
      conf2.hin cfg2.h.top cfg2.h.bot \
      sub/config.h.in sub/config.h.top sub/config.h.bot

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
$MAKE test

:
