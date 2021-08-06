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

# Test AM_MISSING_PROG.

. test-init.sh

cat >> configure.ac <<'END'
AM_MISSING_PROG([NO_SUCH_COMMAND],    [am-none-none])
AM_MISSING_PROG([MISMATCHED_COMMAND], [am-exit-63])
AM_MISSING_PROG([OVERRIDDEN_COMMAND], [am-none-none])
AM_MISSING_PROG([COMMAND_FOUND],      [my-command])
AC_OUTPUT
END

mkdir bin
cat > bin/am-exit-63 <<'END'
#!/bin/sh
echo "Oops, I'm too old"
exit 63
END
cat > bin/am-overridden <<'END'
#!/bin/sh
echo "Hey, I'm OK!"
exit 0
END
cat > bin/my-command <<'END'
#!/bin/sh
echo SNAFU
exit 0
END
chmod a+x bin/*
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH; export PATH

cat > Makefile.am <<'END'
# Different for different targets, for the sake of parallel make.
o = $@-stdout
e = $@-stderr

debug_info = grep . $@-stdout $@-stderr
status_is = $(debug_info); echo $@: st=$$st; test $$st -eq

w_mis = 'am-none-none' is needed, and is missing on your system
w_old = 'am-exit-63' is needed, and is probably too old

test1:
	st=0; $(NO_SUCH_COMMAND) >$o 2>$e || st=$$?; $(status_is) 127
        grep "^WARNING: $(w_mis)" $e
	test ! -s $o
test2:
	st=0; $(MISMATCHED_COMMAND) >$o 2>$e || st=$$?; $(status_is) 63
        grep "^WARNING: $(w_old)" $e
	test "`cat $o`" = "Oops, I'm too old"
test3:
	st=0; $(OVERRIDDEN_COMMAND) >$o 2>$e || st=$$?; $(status_is) 0
	st=0; $(OVERRIDDEN_COMMAND) >$o 2>$e || st=$$?; \
	test ! -s $e
	test "`cat $o`" = "Hey, I'm OK!"
test4:
	st=0; $(COMMAND_FOUND) >$o 2>$e || st=$$?; $(status_is) 0
	test ! -s $e
	test "`cat $o`" = SNAFU
check-local: test1 test2 test2 test4
.PHONY: test1 test2 test2 test4
CLEANFILES = test[1234]-stdout test[1234]-stderr
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure OVERRIDDEN_COMMAND=am-overridden

$FGREP COMMAND Makefile.in Makefile # For debugging.

grep "^NO_SUCH_COMMAND = \${SHELL} .*/missing .*am-none-none" Makefile
grep "^MISMATCHED_COMMAND = \${SHELL} .*/missing .*am-exit-63" Makefile
grep "^COMMAND_FOUND = \${SHELL} .*/missing .*my-command" Makefile
grep '^OVERRIDDEN_COMMAND = am-overridden *$' Makefile

$MAKE test1 test2 test3 test4
$MAKE distcheck DISTCHECK_CONFIGURE_FLAGS='OVERRIDDEN_COMMAND=am-overridden'

:
