#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Ensure the right condition is listed after 'else' and 'endif'.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([USE_A], [test x = y])
AM_CONDITIONAL([USE_B], [test x = z])
AC_OUTPUT
END

notcompat="incompatible with current conditional"

$ACLOCAL

cat > Makefile.am << 'END'
if USE_A
endif !USE_A
END

AUTOMAKE_fails
grep "^Makefile\\.am:2:.*endif.*!USE_A.*$notcompat.*[^!]USE_A" stderr

cat > Makefile.am << 'END'
if USE_A
endif USE_B
END

AUTOMAKE_fails
grep "^Makefile\\.am:2:.*endif.*[^!]USE_B.*$notcompat.*[^!]USE_A" stderr

cat > Makefile.am << 'END'
if USE_A
else
endif USE_A
END

AUTOMAKE_fails
grep "^Makefile\\.am:3:.*endif.*[^!]USE_A.*$notcompat.*USE_A" stderr

cat > Makefile.am << 'END'
if USE_A
else
endif !USE_A
END

$AUTOMAKE

cat > Makefile.am << 'END'
if USE_A
if USE_B
else
endif !USE_A
endif
END

AUTOMAKE_fails
grep "^Makefile\\.am:4:.*endif.*!USE_A.*$notcompat.*USE_B" stderr

cat > Makefile.am << 'END'
if USE_A
else USE_A
endif
END

AUTOMAKE_fails
grep "^Makefile\\.am:2:.*else.*[^!]USE_A.*$notcompat.*[^!]USE_A" stderr

cat > Makefile.am << 'END'
if USE_A
else !USE_A
endif
END

$AUTOMAKE

cat > Makefile.am << 'END'
if USE_A
else USE_B
endif
END

AUTOMAKE_fails
grep "^Makefile\\.am:2:.*else.*[^!]USE_B.*$notcompat.*[^!]USE_A" stderr

cat > Makefile.am << 'END'
if USE_A
if USE_B
else USE_A
endif
endif
END

AUTOMAKE_fails
grep "^Makefile\\.am:3:.*else.*[^!]USE_A.*$notcompat.*[^!]USE_B" stderr

:
