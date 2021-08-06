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
# This test uses a "flat" setup for the source tree (i.e., everything in
# the top-level directory), and forces the use of the old, non-parallel
# testsuite driver.  The sister test 'java-compile-run-nested.sh' do
# similar checks with a more usual, "nested" setup, and using the older
# 'serial-tests' driver.

required='java javac'
am_serial_tests=yes
. test-init.sh

echo "AC_SUBST([PATH_SEPARATOR], ['$PATH_SEPARATOR'])" >> configure.ac

cat >> configure.ac <<'END'
AC_SUBST([JAVA], [java])
AC_CONFIG_SRCDIR([PkgLocation.jin])
AC_CONFIG_FILES([PkgLocation.java:PkgLocation.jin])
AC_OUTPUT
END

## TOP-LEVEL SETUP AND TARGETS ##

cat > Makefile.am <<'END'
EXTRA_DIST = # Will be updated later.

test-built:
	ls -l $(srcdir)               ;: For debugging.
	test $(srcdir) = . || ls -l . ;: Likewise.
	test -f $(srcdir)/Main.java
	test -f $(srcdir)/HelloStream.java
	test -f $(srcdir)/PkgLocation.jin
	test -f PkgLocation.java
	test -f HelloStream.class
	test -f PkgLocation.class
	test -f Main.class
	test -f classjava.stamp

test-installed:
	ls -l $(javadir) ;: For debugging.
	test -f $(javadir)/HelloStream.class
	test -f $(javadir)/PkgLocation.class
	test -f $(javadir)/Main.class
	if find $(prefix) | grep '\.stamp$$'; then exit 1; else :; fi

run-installed:
	jprog_doing_installcheck=yes $(MAKE) $(AM_MAKEFLAGS) check

check-local: test-built
installcheck-local: test-installed run-installed

.PHONY: test-built test-installed run-installed
END

## WRAPPER SCRIPT ##

cat >> Makefile.am <<'END'
bin_SCRIPTS = jprog

edit_script = sed -e 's|[@]JAVA@|$(JAVA)|g' \
                  -e 's|[@]javadir@|$(javadir)|g' \
                  -e 's|[@]SHELL@|$(SHELL)|g' \
                  -e 's|[@]PATH_SEPARATOR@|$(PATH_SEPARATOR)|g'

jprog: jprog.sh
	rm -f $@ $@-t
	$(edit_script) `test -f '$@.sh' || echo $(srcdir)/`$@.sh >$@-t
	chmod a-w $@-t && chmod a+x $@-t && mv -f $@-t $@
	sed 's/^/ | /' $@ ;: for debugging.

EXTRA_DIST += jprog.sh
CLEANFILES = jprog
END

cat > jprog.sh <<'END'
#!@SHELL@
CLASSPATH=${jprog_classpath-'@javadir@'}${CLASSPATH+"@PATH_SEPARATOR@$CLASSPATH"}
export CLASSPATH
case $# in
  0) exec @JAVA@ Main;;
  *) exec @JAVA@ Main "$@";;
esac
END

## JAVA SOURCES ##

cat >> Makefile.am <<'END'
javadir = $(pkgdatadir)/java

dist_java_JAVA = Main.java HelloStream.java
nodist_java_JAVA = PkgLocation.java
END

cat > PkgLocation.jin <<'END'
public class PkgLocation {
    public static String prefix() {
        return new String("@prefix@");
    }
}
END

cat > Main.java <<'END'
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

cat > HelloStream.java <<'END'
import java.io.PrintStream;
class HelloStream {
    public static void to(PrintStream stream) {
        stream.println("Hello, Stream!");
    }
}
END

## TESTS ##

cat >> Makefile.am <<'END'
## We must use 'TESTS_ENVIRONMENT', not 'AM_TESTS_ENVIRONMENT',
## because the latter is not hnoured by the old serial test
## harness.
TESTS_ENVIRONMENT = \
    if test x"$$jprog_doing_installcheck" != x"yes"; then \
        jprog_classpath='$(abs_top_builddir):$(abs_top_srcdir)'; \
        export jprog_classpath; \
        PATH='$(abs_top_builddir)$(PATH_SEPARATOR)'$$PATH; \
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

EXTRA_DIST += $(TESTS)
END

cat > simple.test <<'END'
#!/bin/sh
jprog
END

cat > prefix.test <<'END'
#!/bin/sh
jprefix=`jprog --print-prefix` || exit 1
echo "$0: exp prefix: $config_time_prefix"
echo "$0: got prefix: $jprefix"
test x"$jprefix" = x"$config_time_prefix"
END

cat > stdout.test <<'END'
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

cat > stderr.test <<'END'
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

cat > badarg.test <<'END'
#!/bin/sh
jprog --bad-argument
END

chmod a+x *.test

## DO CHECKS ##

$ACLOCAL
$AUTOCONF
$AUTOMAKE

# To have the parallel testsuite more verbose.
VERBOSE=yes; export VERBOSE

./configure --prefix="$(pwd)/_inst"
cat PkgLocation.java # For debugging.
$MAKE check
$MAKE install
$MAKE test-installed
$MAKE run-installed
$MAKE distcheck

:
