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

# Check a documented limitation of the Automake's Makefile parser
# w.r.t. POSIX variable substitutions used in the name of targets.
# See Section "General Operation" in the Automake manual.
# If you cause some parts of this test to fail, chances are that you've
# improved the Automake parser ;-)
# See: <https://lists.gnu.org/archive/html/automake/2010-08/msg00074.html>
# or: <http://thread.gmane.org/gmane.comp.sysutils.automake.general/11943/focus=11962>

. test-init.sh

cat > Makefile.am <<'END'
$(FOO:=x): bar
END

$ACLOCAL
AUTOMAKE_fails

grep 'bad characters.*variable name.*\$(FOO' stderr
grep ':=.*assignments.*not portable' stderr

:
