#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Test 'make -n' for various targets, to ensure that:
#
#  1. no files or directories are created or removed, and
#
#  2. if using GNU make or a make implementation supporting the .MAKE
#     special target, the output from make is sufficiently complete.
#
# This test exercises the GCS-mandated targets (except for dist)
# as well as tags.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

# Does $MAKE support the '.MAKE' special target?
have_dotmake=false
if using_gmake; then
  have_dotmake=: # GNU make must support it.
else
  unindent > mk.tmp << 'END'
    targ.tmp:
	: > $@
    .MAKE: targ.tmp
END
  if $MAKE -n -f mk.tmp targ.tmp && test -f targ.tmp; then
    have_dotmake=:
  fi
fi

mkdir sub sub2

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/Makefile sub2/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
TESTS = foo.test
SUBDIRS = sub sub2
##BUILT_SOURCES = foo
foo:
foo.test:
all-local:
	@: > stamp-all
install-data-local:
	@: > stamp-install
uninstall-local:
	@: > stamp-uninstall
clean-local:
	@: > stamp-clean
distclean-local:
	@: > stamp-distclean
info-local:
	@: > stamp-info
html-local:
	@: > stamp-html
dvi-local:
	@: > stamp-dvi
ps-local:
	@: > stamp-ps
pdf-local:
	@: > stamp-pdf
check-local:
	@: > stamp-check
installcheck-local:
	@: > stamp-installcheck
mostlyclean-local:
	@: > stamp-mostlyclean
maintainer-clean-local:
	@: > stamp-maintainer-clean
END
cat >sub/Makefile.am <<'END'
all-local:
	@: > stamp-all-sub
install-data-local:
	@: > stamp-install-sub
uninstall-local:
	@: > stamp-uninstall-sub
clean-local:
	@: > stamp-clean-sub
distclean-local:
	@: > stamp-distclean-sub
info-local:
	@: > stamp-info-sub
html-local:
	@: > stamp-html-sub
dvi-local:
	@: > stamp-dvi-sub
ps-local:
	@: > stamp-ps-sub
pdf-local:
	@: > stamp-pdf-sub
check-local:
	@: > stamp-check-sub
installcheck-local:
	@: > stamp-installcheck-sub
tags:
	@: > stamp-tags-sub
mostlyclean-local:
	@: > stamp-mostlyclean-sub
maintainer-clean-local:
	@: > stamp-maintainer-clean-sub
END
cat >sub2/Makefile.am <<'END'
all install uninstall clean check:
	@: > sub2-$@-should-not-be-executed
info dvi pdf ps:
	@: > sub2-$@-should-not-be-executed
html:
	@: > sub2-$@-should-not-be-executed
install-info install-html install-dvi install-pdf install-ps:
	@: > sub2-$@-should-not-be-executed
installcheck installdirs tags mostlyclean:
	@: > sub2-$@-should-not-be-executed
## These targets cannot be overridden like this:
## install-strip distclean maintainer-clean
END

$ACLOCAL
$AUTOCONF

check_targets ()
{
  for target in \
    all install install-strip uninstall clean distclean check \
    info html dvi pdf ps \
    install-info install-html install-dvi install-pdf install-ps \
    installcheck installdirs tags mostlyclean maintainer-clean
  do
    run_make -O -- -n $target
    case $target in
    install-* | installdirs | tags ) ;;
    *)
      if $have_dotmake; then
        grep "stamp-$target$" stdout || exit 1
      fi
      test ! -e "stamp-$target" || exit 1
      ;;
    esac
    case $target in
    install-* | installdirs ) ;;
    *)
      if $have_dotmake; then
        grep "stamp-$target-sub" stdout || exit 1
      fi
      test ! -e "sub/stamp-$target-sub" || exit 1
      ;;
    esac
    case $target in
    distclean | maintainer-clean ) ;;
    *)
      if $have_dotmake; then
        grep "should-not-be-executed" stdout || exit 1
      fi
      test ! -e "sub2/sub2-$target-should-not-be-executed" || exit 1
      ;;
    esac
  done
}

$AUTOMAKE -a -Wno-override
./configure
check_targets || exit 1

# Now, introduce BUILT_SOURCES into the toplevel Makefile
# TODO: add BUILT_SOURCES to sub2, fix fallout.
sed 's/##//' < Makefile.am > t
mv -f t Makefile.am
$AUTOMAKE -Wno-override Makefile
./configure
check_targets || exit 1

exit 0
