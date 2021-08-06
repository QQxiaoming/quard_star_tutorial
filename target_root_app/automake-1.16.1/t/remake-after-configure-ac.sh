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

# Test remake rules when configure.ac or its prerequisites change.
# Keep in sync with the other sister tests 'remake-after-*.sh'.

. test-init.sh

magic1=::MagicStringOne::
magic2=__MagicStringTwo__

if using_gmake; then
  remake_() { $MAKE nil; }
else
  remake_() { $MAKE Makefile && $MAKE foo.sh; }
fi

cat >> configure.ac <<END
FINGERPRINT=BadBadBad
AC_SUBST([FINGERPRINT])
AC_CONFIG_FILES([foo.sh:foo.in], [chmod a+x foo.sh])
AC_OUTPUT
END

cat > Makefile.am <<'END'
nil:
.PHONY: nil

$(srcdir)/configure.ac: $(srcdir)/tweak-configure-in
	$(SHELL) $(srcdir)/tweak-configure-in <$@ >$@-t
	mv -f $@-t $@
EXTRA_DIST = $(srcdir)/tweak-configure-in

# Used by "make distcheck" later.
check-local:
	test x'$(FINGERPRINT)' = x'DummyValue'
	test x"`./foo.sh`" = x"DummyValue"
END

cat > foo.in <<END
#!/bin/sh
echo '@FINGERPRINT@'
END

echo cat > tweak-configure-in # It is a no-op by default.

$ACLOCAL
$AUTOCONF
$AUTOMAKE

for vpath in : false; do

  if $vpath; then
    mkdir build
    cd build
    srcdir=..
  else
    srcdir=.
  fi

  $srcdir/configure
  $MAKE # Should be a no-op.

  $sleep
  sed "s/^\\(FINGERPRINT\\)=.*/\\1=$magic1/" $srcdir/configure.ac >t
  mv -f t $srcdir/configure.ac
  remake_
  $FGREP FINGERPRINT Makefile # For debugging.
  $FGREP $magic1 Makefile
  test x"$(./foo.sh)" = x"$magic1"

  $sleep
  echo 'sed "s/^\\(FINGERPRINT\\)=.*/\\1='$magic2'/"' \
    > $srcdir/tweak-configure-in
  remake_
  $FGREP FINGERPRINT Makefile # For debugging.
  $FGREP $magic1 Makefile && exit 1
  $FGREP $magic2 Makefile
  test x"$(./foo.sh)" = x"$magic2"

  $sleep
  echo cat > $srcdir/tweak-configure-in # Make it a no-op again.
  sed "s/^\\(FINGERPRINT\\)=.*/\\1=DummyValue/" $srcdir/configure.ac >t
  mv -f t $srcdir/configure.ac
  using_gmake || remake_
  $MAKE distcheck
  $FGREP $magic1 Makefile && exit 1 # Sanity check.
  $FGREP $magic2 Makefile && exit 1 # Likewise.

  $MAKE distclean

  cd $srcdir

done

:
