#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Check that Automake warns about variables containing spaces
# and other non-POSIX characters.

. test-init.sh

cat >Makefile.am <<'EOF'
L01 = $(shell echo *)
L02 = $$(not an error)
L03 = $$(this is)$${ok too}
L04 = $(nextvariableisbad)$(addsuffix .a, $(A))
L05 = "$(bad boy)"
L06 = $(this:is= ok)
L07 = ${three errors}${on this} $(long line)
L08$(o u c h): $(wildcard *.c)
	${another Error}
	echo $${ok-this is}
L11: $(thisis) $(ok)
	${here}
EOF

$ACLOCAL
# Make sure this warning is print in the 'portability' category.
$AUTOMAKE --warnings=no-error,none,portability 2>stderr \
  || { cat stderr >&2; exit 1; }
cat stderr >&2

# Lines number are printed in error message.
# Use them to make sure errors are diagnosed against the right lines.

# No error expected apart from those on these lines.
grep -v '^Makefile\.am:[145789]:' stderr | grep . && exit 1

# Now check some individual values.
grep ':1:.*shell echo' stderr
grep 'nextvariableisbad' stderr && exit 1
grep ':4:.*addsuffix' stderr
grep ':5:.*bad boy' stderr
grep ':7:.*three errors' stderr
grep ':7:.*on this' stderr
grep ':7:.*long line' stderr
grep ':8:.*o u c h' stderr
grep ':8:.*wildcard' stderr
grep ':9:.*another Error' stderr

$EGREP 'ok|thisis|here' stderr && exit 1

# None of these errors be diagnosed with '-Wno-portability'.
$AUTOMAKE -Wno-portability

# Likewise if we add this in the Makefile.am
# (although this makes some difference internally: AUTOMAKE_OPTIONS is
# processed far later).
echo 'AUTOMAKE_OPTIONS = -Wno-portability' >> Makefile.am
$AUTOMAKE

:
