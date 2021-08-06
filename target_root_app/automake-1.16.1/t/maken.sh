#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Ensure that 'make -n dist' and 'make -n distcheck' do not change files
# on disk, due to GNU make executing rules containing '$(MAKE)'.
# Also, ensure that 'make -n dist' and 'make -n distcheck' show what
# would happen, at least when using GNU make.

. test-init.sh

mkdir sub

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
distcheck-hook:
	@: > stamp-distcheck-hook
test-no-distdir:
	test ! -d $(distdir)
	test ! -f stamp-distcheck-hook
	test ! -f sub/stamp-sub-dist-hook
END
cat >sub/Makefile.am <<'END'
dist-hook:
	@: > stamp-sub-dist-hook
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

echo stamp > stampfile
$sleep
for target in dist distcheck; do
  $MAKE -n $target
  if using_gmake; then
    $MAKE -n $target | grep stamp-sub-dist-hook || exit 1
  fi
  $MAKE test-no-distdir
  # No file has been actually touched or created.
  is_newest stampfile $(find . -type f) sub
done

:
