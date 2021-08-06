#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure rebuild rules work even when AC_CONFIG_FILES uses colons.
# Report from Alexander Turbov.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([bar/Makefile:bar/Makefile.in:Makefile.bot])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = bar
EXTRA_DIST = Makefile.bot
END

mkdir bar
cat > bar/Makefile.am << 'END'
top-rule:
	@echo 'top rule'
END

cat > Makefile.bot << 'END'
bot-rule:
	@echo 'bot rule'
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE

cd bar
run_make -O top-rule
grep 'top rule' stdout
run_make -O bot-rule
grep 'bot rule' stdout
cd ..

$sleep

cat > bar/Makefile.am << 'END'
top-rule:
	@echo 'top2 rule'
END
$MAKE

cd bar
run_make -O top-rule
grep 'top2 rule' stdout
run_make -O bot-rule
grep 'bot rule' stdout
cd ..

$sleep

cat > Makefile.bot << 'END'
bot-rule:
	@echo 'bot2 rule'
END
$MAKE

cd bar
run_make -O top-rule
grep 'top2 rule' stdout
run_make -O bot-rule
grep 'bot2 rule' stdout
cd ..

$MAKE distcheck

:
