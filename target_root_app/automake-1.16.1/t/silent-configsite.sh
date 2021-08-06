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

# Check that the user can control default mode of silent-rules
# from config.site, and that this default can be overridden from
# either the ./configure or make command line.

. test-init.sh

cat >> configure.ac <<'EOF'
# This line will be edited later to force silent-rules default.
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
.PHONY: test-silent test-nosilent
test-silent:
	test x'$(AM_DEFAULT_VERBOSITY)' = x'0'
test-nosilent:
	test x'$(AM_DEFAULT_VERBOSITY)' = x'1'
EOF

unset enable_silent_rules

: 'No explicit default in configure.ac, enable by default in config.site'

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing
echo "enable_silent_rules=\${enable_silent_rules-yes}" > config.site
CONFIG_SITE=./config.site ./configure
$MAKE test-silent
$MAKE distclean
# Command line should win over default values in config.site.
CONFIG_SITE=./config.site ./configure --disable-silent-rules
$MAKE test-nosilent
$MAKE distclean

: 'Disable by default in configure.ac, enable by default in config.site'

sed 's/.*silent-rules default.*/AM_SILENT_RULES([no])/' configure.ac > t
diff t configure.ac && fatal_ "editing configure.ac"
mv -f t configure.ac
$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing
echo "enable_silent_rules=\${enable_silent_rules-yes}" > config.site
CONFIG_SITE=./config.site ./configure
$MAKE test-silent
# Command line should win over default values in config.site.
$MAKE distclean
CONFIG_SITE=./config.site ./configure --disable-silent-rules
$MAKE test-nosilent
$MAKE distclean

: 'Enable by default in configure.ac, disable by default in config.site'

sed 's/.*AM_SILENT_RULES.*/AM_SILENT_RULES([yes])/' configure.ac > t
diff t configure.ac && fatal_ "editing configure.ac"
mv -f t configure.ac
$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing
echo "enable_silent_rules=\${enable_silent_rules-no}" > config.site
CONFIG_SITE=./config.site ./configure
$MAKE test-nosilent
$MAKE distclean
# Command line should win over default values in config.site.
CONFIG_SITE=./config.site ./configure --enable-silent-rules
$MAKE test-silent
$MAKE distclean

:
