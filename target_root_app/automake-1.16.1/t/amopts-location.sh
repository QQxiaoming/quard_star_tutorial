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

# Check that errors about AUTOMAKE_OPTIONS refers to correct
# locations.

. test-init.sh

cat > Makefile.am <<'END'
# comment \
# continued
include Makefile0.am
END

cat > Makefile0.am <<'END'
#1
#2
#3
include Makefile1.am
END

cat > Makefile1.am <<'END'
AUTOMAKE_OPTIONS = tar-pax
# comment
END

cat > Makefile2.am <<'END'
## automake comment
bar:
	:
line = \
continued
AUTOMAKE_OPTIONS = tar-ustar
END

cat > Makefile3.am <<'END'
quux = a
AUTOMAKE_OPTIONS =
quux += b
AUTOMAKE_OPTIONS += tar-v7
zardoz = 1
END

cat >>configure.ac <<'END'
AC_CONFIG_FILES([Makefile2 Makefile3])
END

$ACLOCAL
# Automake options 'tar-v7', 'tar-ustar' and 'tar-pax' can only be used
# as argument to AM_INIT_AUTOMAKE, and not in AUTOMAKE_OPTIONS.
AUTOMAKE_fails

# Check that all the expected line numbers are correctly reported
# in automake warning/error messages.
grep '^Makefile1\.am:1:.*tar-pax' stderr
grep '^Makefile2\.am:6:.*tar-ustar' stderr
grep '^Makefile3\.am:2:.*tar-v7' stderr
grep '^Makefile\.am:3:.*Makefile0\.am.*included from here' stderr
grep '^Makefile0\.am:4:.*Makefile1\.am.*included from here' stderr

# And also check that no botched line number is reported.
cat stderr \
  | grep -v '^Makefile\.am:3:'  \
  | grep -v '^Makefile0\.am:4:' \
  | grep -v '^Makefile1\.am:1:' \
  | grep -v '^Makefile2\.am:6:' \
  | grep -v '^Makefile3\.am:2:' \
  | grep . && exit 1

:
