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

# Test on compilation and execution of Java class files.
# Also meddle with wrapper scripts, as would be probably seen in a real
# "UNIX-style" use case.
# This test uses a typical "nested" source tree setup (i.e., different
# components/aspects are separated into different subdirectories), and
# uses the parallel testsuite driver.
# The sister test 'java-compile-run-flat.sh' do similar checks with
# a "flat" setup (i.e., everything in the top-level directory), and
# forcing the use of the older 'serial-tests' driver.

required='java javac'
. test-init.sh

echo "AC_SUBST([PATH_SEPARATOR], ['$PATH_SEPARATOR'])" >> configure.ac

cat >> configure.ac <<'END'
AC_SUBST([JAVA], [java])
AC_CONFIG_SRCDIR([jprog/PkgLocation.jin])
AC_SUBST([jprogdatadir],  ['${pkgdatadir}'])
AC_SUBST([jprogclassdir], ['${jprogdatadir}/jprog'])
AC_CONFIG_FILES([jprog/PkgLocation.java:jprog/PkgLocation.jin])
AC_CONFIG_FILES([jprog/Makefile bin/Makefile tests/Makefile])
AC_OUTPUT
END

## TOP-LEVEL SETUP AND TARGETS ##

cat > Makefile.am <<'END'
SUBDIRS = bin jprog tests

test-built:
	ls -l $(srcdir)/*             ;: For debugging.
	test $(srcdir) = . || ls -l * ;: Likewise.
	test -f $(srcdir)/jprog/Main.java
	test -f $(srcdir)/jprog/HelloStream.java
	test -f $(srcdir)/jprog/PkgLocation.jin
	test -f jprog/PkgLocation.java
	test -f jprog/HelloStream.class
	test -f jprog/Main.class
	test -f jprog/PkgLocation.class
	test -f jprog/classjprogclass.stamp

test-installed:
	ls -l $(jprogclassdir) ;: For debugging.
	test -f $(jprogclassdir)/HelloStream.class
	test -f $(jprogclassdir)/Main.class
	test -f $(jprogclassdir)/PkgLocation.class
	if find $(prefix) | grep '\.stamp$$'; then exit 1; else :; fi

run-installed:
	jprog_doing_installcheck=yes $(MAKE) $(AM_MAKEFLAGS) check

check-local: test-built
installcheck-local: test-installed run-installed

.PHONY: test-built test-installed run-installed
END

## WRAPPER SCRIPT ##

mkdir bin

cat > bin/Makefile.am <<'END'
bin_SCRIPTS = jprog

edit_script = sed -e 's|[@]JAVA@|$(JAVA)|g' \
                  -e 's|[@]jprogdatadir@|$(jprogdatadir)|g' \
                  -e 's|[@]SHELL@|$(SHELL)|g' \
                  -e 's|[@]PATH_SEPARATOR@|$(PATH_SEPARATOR)|g'

jprog: jprog.sh
	rm -f $@ $@-t
	$(edit_script) `test -f '$@.sh' || echo $(srcdir)/`$@.sh >$@-t
	chmod a-w $@-t && chmod a+x $@-t && mv -f $@-t $@
	sed 's/^/ | /' $@ ;: for debugging.

EXTRA_DIST = jprog.sh
CLEANFILES = jprog
END

cat > bin/jprog.sh <<'END'
#!@SHELL@
CLASSPATH=${jprog_classpath-'@jprogdatadir@'}${CLASSPATH+"@PATH_SEPARATOR@$CLASSPATH"}
export CLASSPATH
case $# in
  0) exec @JAVA@ jprog.Main;;
  *) exec @JAVA@ jprog.Main "$@";;
esac
END

## JAVA SOURCES ##

mkdir jprog

cat > jprog/Makefile.am <<'END'
dist_jprogclass_JAVA = Main.java HelloStream.java
nodist_jprogclass_JAVA = PkgLocation.java
END

cat > jprog/PkgLocation.jin <<'END'
package jprog;
public class PkgLocation {
    public static String prefix() {
        return new String("@prefix@");
    }
}
END

cat > jprog/Main.java <<'END'
package jprog;
import jprog.PkgLocation;
import jprog.HelloStream;
public class Main {
    public static void main(String[] args) {
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("--print-prefix")) {
                System.out.println(PkgLocation.prefix());
            } else if (args[i].equals("--hello-stdout")) {
                HelloStream.to(System.out);
            } else if (args[i].equals("--hello-stderr")) {
                HelloStream.to(System.err);
            } else {
               System.err.println("jprog: invalid option '" + args[i] +
                                  "'");
               System.exit(2);
            }
        }
        System.exit(0);
    }
}
END

cat > jprog/HelloStream.java <<'END'
package jprog;
import java.io.PrintStream;
class HelloStream {
    public static void to(PrintStream stream) {
        stream.println("Hello, Stream!");
    }
}
END

## TESTS ##

mkdir tests

cat > tests/Makefile.am <<'END'
AM_TESTS_ENVIRONMENT = \
    if test x"$$jprog_doing_installcheck" != x"yes"; then \
        jprog_classpath='$(abs_top_builddir):$(abs_top_srcdir)'; \
        export jprog_classpath; \
        PATH='$(abs_top_builddir)/bin$(PATH_SEPARATOR)'$$PATH; \
        export PATH; \
    else \
        unset jprog_classpath || :; \
        PATH='$(prefix)/bin$(PATH_SEPARATOR)'$$PATH; \
        export PATH; \
    fi; \
    config_time_prefix='@prefix@'; export config_time_prefix;

TESTS = \
  simple.test \
  prefix.test \
  stdout.test \
  stderr.test \
  badarg.test

XFAIL_TESTS = badarg.test

EXTRA_DIST = $(TESTS)
END

cat > tests/simple.test <<'END'
#!/bin/sh
jprog
END

cat > tests/prefix.test <<'END'
#!/bin/sh
jprefix=`jprog --print-prefix` || exit 1
echo "$0: exp prefix: $config_time_prefix"
echo "$0: got prefix: $jprefix"
test x"$jprefix" = x"$config_time_prefix"
END

cat > tests/stdout.test <<'END'
#!/bin/sh
rc=0
jprog --hello-stdout >stdout.out 2>stdout.err || { echo \$?=$?; rc=1; }
sed 's/^/out:/' <stdout.out      # For debugging.
sed 's/^/err:/' <stdout.err >&2  # Likewise.
test -s stdout.err && rc=1
test "`cat stdout.out`" = 'Hello, Stream!' || rc=1
rm -f stdout.out stdout.err || rc=1
exit $rc
END

cat > tests/stderr.test <<'END'
#!/bin/sh
rc=0
jprog --hello-stderr >stderr.out 2>stderr.err || { echo \$?=$?; rc=1; }
sed 's/^/out:/' <stderr.out      # For debugging.
sed 's/^/err:/' <stderr.err >&2  # Likewise.
test -s stderr.out && rc=1
test "`cat stderr.err`" = 'Hello, Stream!' || rc=1
rm -f stderr.out stderr.err || rc=1
exit $rc
END

cat > tests/badarg.test <<'END'
#!/bin/sh
jprog --bad-argument
END

chmod a+x tests/*.test

## DO CHECKS ##

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

# To have the parallel testsuite more verbose.
VERBOSE=yes; export VERBOSE

./configure --prefix="$(pwd)/_inst"
cat jprog/PkgLocation.java # For debugging.
$MAKE check
$MAKE install
$MAKE test-installed
$MAKE run-installed
$MAKE distcheck

:
