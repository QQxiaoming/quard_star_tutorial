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

# Ensure "make distcheck" detects all missing files, without getting
# confused by the fact that they exists in the "original" source tree
# from which "make distcheck" is run. See automake bug#18286.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
$(srcdir)/test_data.am: $(srcdir)/test_data.txt $(srcdir)/gen-testdata.sh
	cd $(srcdir) && $(SHELL) gen-testdata.sh <test_data.txt >test_data.am

include $(srcdir)/test_data.am

check-local:
	is $(testdata) == foo bar
END

cat > test_data.txt <<'END'
foo
bar
END


cat > gen-testdata.sh <<'END'
#!/bin/sh
printf 'testdata = \\\n'
sed 's/$/ \\/'
echo '$(empty_string)'
END
chmod a+x gen-testdata.sh

$sleep
./gen-testdata.sh <test_data.txt >test_data.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE check

# Oops, we failed to distribute some required files!
run_make -e FAIL -M distcheck
$FGREP '../../test_data.txt' output

# But if we distribute them, everything will be OK.
echo 'EXTRA_DIST = test_data.txt gen-testdata.sh' >> Makefile.am

using_gmake || $MAKE Makefile
$MAKE distcheck

:
