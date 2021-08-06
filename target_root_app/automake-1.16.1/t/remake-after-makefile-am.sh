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

# Test remake rules when Makefile.am or its prerequisites change.
# Keep in sync with the other sister tests 'remake-after-*.sh'.

. test-init.sh

if using_gmake; then
  remake_() { $MAKE nil; }
else
  remake_() { $MAKE Makefile && $MAKE foo.sh; }
fi

magic1=::MagicStringOne::
magic2=__MagicStringTwo__

cat >> configure.ac <<END
AC_OUTPUT
END

cat > Makefile.am <<'END'
FINGERPRINT = BadBadBad

all-local: nil
nil: foo.sh
.PHONY: nil

$(srcdir)/Makefile.am: $(srcdir)/tweak-makefile-am
	$(SHELL) $(srcdir)/tweak-makefile-am <$@ >$@-t
	mv -f $@-t $@
EXTRA_DIST = $(srcdir)/tweak-makefile-am

foo.sh: Makefile
	rm -f $@ $@-t
	echo '#!/bin/sh' > $@-t
	echo "echo '$(FINGERPRINT)'" >> $@-t
	chmod a+x $@-t && mv -f $@-t $@
CLEANFILES = foo.sh

# Used by "make distcheck" later.
check-local:
	test x'$(FINGERPRINT)' = x'DummyValue'
	test x"`./foo.sh`" = x"DummyValue"
END

echo cat > tweak-makefile-am # It is a no-op by default.

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
  sed "s/^\\(FINGERPRINT\\) *=.*/\\1 = $magic1/" $srcdir/Makefile.am >t
  mv -f t $srcdir/Makefile.am
  remake_
  $FGREP FINGERPRINT Makefile # For debugging.
  $FGREP $magic1 Makefile
  test x"$(./foo.sh)" = x"$magic1"

  $sleep
  echo 'sed "s/^\\(FINGERPRINT\\) *=.*/\\1 = '$magic2'/"' \
    > $srcdir/tweak-makefile-am
  remake_
  $FGREP FINGERPRINT Makefile # For debugging.
  $FGREP $magic1 Makefile && exit 1
  $FGREP $magic2 Makefile
  test x"$(./foo.sh)" = x"$magic2"

  $sleep
  echo cat > $srcdir/tweak-makefile-am # Make it a no-op again.
  sed "s/^\\(FINGERPRINT\\) *=.*/\\1 = DummyValue/" $srcdir/Makefile.am >t
  mv -f t $srcdir/Makefile.am
  using_gmake || remake_
  $MAKE distcheck
  $FGREP $magic1 Makefile && exit 1 # Sanity check.
  $FGREP $magic2 Makefile && exit 1 # Likewise.

  $MAKE distclean

  cd $srcdir

done

:
