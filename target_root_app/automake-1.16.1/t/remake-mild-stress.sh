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

# Test basic remake rules for Makefiles with non-default names
# and/or with multiple sources.

. test-init.sh

magic1=::MagicStringOne::
magic2=__MagicStringTwo__
magic3=%%MagicStringThree%%

# See if the user's make implementation support an include directive.
# We need it in this test.
echo 'all:; @echo include is supported' > incl.mk
echo 'include incl.mk' > gnu-style.mk
echo '.include "incl.mk"' > bsd-style.mk
if $MAKE -f gnu-style.mk | grep 'include is supported'; then
  echo "$me: GNU make include style supported"
  include_zardoz='include zardoz'
elif $MAKE -f bsd-style.mk | grep 'include is supported'; then
  echo "$me: BSD make include style supported"
  include_zardoz='.include "zardoz"'
else
  skip_ "make doesn't support any \"include\" directive"
fi

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([zardoz])
AC_CONFIG_LINKS([Makefile:Makefile])
AC_OUTPUT
END

cat > zardoz.am <<END
EXTRA_DIST = Makefile
#H: $magic1
END

cat > Makefile <<END
$include_zardoz
nil:
.PHONY: nil
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

if using_gmake; then
  $MAKE nil
else
  $MAKE Makefile
fi
grep '^#H:' zardoz.in # For debugging.
$FGREP $magic1 zardoz
$FGREP $magic1 zardoz.in
$MAKE distcheck
$MAKE distclean # This shouldn't remove Makefile.
ls -l
test -f Makefile

./configure

$sleep
sed "s/%MAGIC3%/$magic3/" >> Makefile <<'END'
my-check:
	ls -l . $(srcdir) ;: For debugging.
	test -f $(srcdir)/quux.am
	test -f $(srcdir)/quux.in
	test -f $(srcdir)/bot.in
	test -f $(srcdir)/top.in
	test ! -r $(srcdir)/zardoz.am
	test ! -r $(srcdir)/zardoz.in
	grep FOO zardoz ;: For debugging.
	test x'$(FOO)' = x'%MAGIC3%'
test:
	ls -l ;: For debugging.
	test x'$(FOO)' = x'dummy'
.PHONY: test my-check
END
sed "s/^#H:.*/#H: $magic2/" zardoz.am > t
cat >> t <<'END'
# Used by "make distcheck" below.
check-local: my-check
END
mv -f t zardoz.am
cat zardoz.am # For debugging.
if using_gmake; then
  $MAKE nil
else
  $MAKE zardoz
fi
$FGREP my-check zardoz # Sanity check.
$FGREP $magic1 zardoz zardoz.in && exit 1
$FGREP $magic2 zardoz
$FGREP $magic2 zardoz.in

./configure

$sleep
sed 's/^\(AC_CONFIG_FILES\)(.*/\1([zardoz:top.in:quux.in:bot.in])/' \
  <configure.ac >t
mv -f t configure.ac
cat configure.ac # For debugging.
sed '/^#H:/d' zardoz.am > quux.am
echo 'FOO = dummy' >> quux.am
echo 'BAR = $(BAZ)' > top.in
echo "BAZ = $magic3" > bot.in
is_gmake || $MAKE zardoz
$MAKE test
$FGREP my-check zardoz # Sanity check.
$FGREP $magic3 quux.in && exit 1
$FGREP $magic3 zardoz
$FGREP $magic1 zardoz && exit 1
$FGREP $magic2 zardoz && exit 1
# After the remake above, the files 'zardoz.am' and 'zardoz.in'
# should be no more needed.
echo 'endif' > zardoz.am # Put in a syntax error.
is_gmake || $MAKE zardoz
$MAKE test
rm -f zardoz.in zardoz.am # Get rid of them.
is_gmake || $MAKE zardoz
$MAKE test

echo 'FOO = $(BAR)' >> quux.am
is_gmake || $MAKE zardoz
$MAKE distcheck

:
