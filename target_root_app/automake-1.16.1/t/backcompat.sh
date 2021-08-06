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

# Test usage of AM_INIT_AUTOMAKE with two or three arguments, for
# backward-compatibility.

. test-init.sh

cat > Makefile.am <<'END'
.PHONY: test display

## Might be useful for debugging.
display:
## The following should be substituted by AM_INIT_AUTOMAKE.
	@echo PACKAGE = $(PACKAGE)
	@echo VERSION = $(VERSION)
## The following should not be substituted, as we used the
## old form of AC_INIT.
	@echo PACKAGE_NAME = $(PACKAGE_NAME)
	@echo PACKAGE_VERSION = $(PACKAGE_VERSION)
	@echo PACKAGE_TARNAME = $(PACKAGE_TARNAME)
	@echo PACKAGE_STRING = $(PACKAGE_STRING)

test: display
	test x'$(PACKAGE)' = x'FooBar'
	test x'$(VERSION)' = x'0.7.1'
	test x'$(PACKAGE_NAME)' = x
	test x'$(PACKAGE_VERSION)' = x
	test x'$(PACKAGE_TARNAME)' = x
	test x'$(PACKAGE_STRING)' = x
END

for ac_init in 'AC_INIT' 'AC_INIT([Makefile.am])'; do
  for am_extra_args in '' ', []' ', [:]' ', [false]'; do
    rm -rf autom4te*.cache config* Makefile.in Makefile
    unindent > configure.ac <<END
      $ac_init
      AM_INIT_AUTOMAKE([FooBar], [0.7.1]$am_extra_args)
      AC_CONFIG_FILES([Makefile])
      AC_OUTPUT
END
    cat configure.ac # For debugging.
    $ACLOCAL
    $AUTOCONF
    $AUTOMAKE -Wno-obsolete
    ./configure
    $MAKE test
  done
done

:
