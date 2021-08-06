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

# Sanity check on the function used by the automake testsuite to
# analyze output of "configure --help".

. test-init.sh

# FIXME: this test is a good candidate for a conversion to TAP.

# Prefer real-world data.  This is from the help screen of the
# configure script from GNU coreutils, with small adaptations.
cat > help <<'END'
'configure' configures GNU coreutils 8.14a to adapt to many kinds of systems.

Usage: ./configure [OPTION]... [VAR=VALUE]...

To assign environment variables (e.g., CC, CFLAGS...), specify them as
VAR=VALUE.  See below for descriptions of some of the useful variables.

Defaults for the options are specified in brackets.

Configuration:
  -h, --help              display this help and exit
      --help=short        display options specific to this package
      --help=recursive    display the short help of all the included packages
  -V, --version           display version information and exit
  -q, --quiet, --silent   do not print 'checking ...' messages
      --cache-file=FILE   cache test results in FILE [disabled]
  -C, --config-cache      alias for '--cache-file=config.cache'
  -n, --no-create         do not create output files
      --srcdir=DIR        find the sources in DIR [configure dir or '..']

Installation directories:
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [/usr/local]
  --exec-prefix=EPREFIX   install architecture-dependent files in EPREFIX
                          [PREFIX]

By default, 'make install' will install all the files in
'/usr/local/bin', '/usr/local/lib' etc.  You can specify
an installation prefix other than '/usr/local' using '--prefix',
for instance '--prefix=$HOME'.

For better control, use the options below.

Fine tuning of the installation directories:
  --bindir=DIR            user executables [EPREFIX/bin]
  --sbindir=DIR           system admin executables [EPREFIX/sbin]
  --libexecdir=DIR        program executables [EPREFIX/libexec]
  --sysconfdir=DIR        read-only single-machine data [PREFIX/etc]
  --sharedstatedir=DIR    modifiable architecture-independent data [PREFIX/com]
  --localstatedir=DIR     modifiable single-machine data [PREFIX/var]
  --libdir=DIR            object code libraries [EPREFIX/lib]
  --includedir=DIR        C header files [PREFIX/include]
  --oldincludedir=DIR     C header files for non-gcc [/usr/include]
  --datarootdir=DIR       read-only arch.-independent data root [PREFIX/share]
  --datadir=DIR           read-only architecture-independent data [DATAROOTDIR]
  --infodir=DIR           info documentation [DATAROOTDIR/info]
  --localedir=DIR         locale-dependent data [DATAROOTDIR/locale]
  --mandir=DIR            man documentation [DATAROOTDIR/man]
  --docdir=DIR            documentation root [DATAROOTDIR/doc/coreutils]
  --htmldir=DIR           html documentation [DOCDIR]
  --dvidir=DIR            dvi documentation [DOCDIR]
  --pdfdir=DIR            pdf documentation [DOCDIR]
  --psdir=DIR             ps documentation [DOCDIR]

Program names:
  --program-prefix=PREFIX            prepend PREFIX to installed program names
  --program-suffix=SUFFIX            append SUFFIX to installed program names
  --program-transform-name=PROGRAM   run sed PROGRAM on installed program names

System types:
  --build=BUILD     configure for building on BUILD [guessed]
  --host=HOST       cross-compile to build programs to run on HOST [BUILD]

Optional Features:
  --disable-option-checking  ignore unrecognized --enable/--with options
  --disable-FEATURE       do not include FEATURE (same as --enable-FEATURE=no)
  --enable-FEATURE[=ARG]  include FEATURE [ARG=yes]
  --enable-silent-rules          less verbose build output (undo: 'make V=1')
  --disable-silent-rules         verbose build output (undo: 'make V=0')
  --disable-dependency-tracking  speeds up one-time build
  --enable-dependency-tracking   do not reject slow dependency extractors
  --disable-largefile     omit support for large files
  --enable-threads={posix|solaris|pth|win32}
                          specify multithreading API
  --disable-threads       build without multithread safety
  --disable-acl           do not support ACLs
  --disable-assert        turn off assertions
  --disable-rpath         do not hardcode runtime library paths
  --disable-xattr         do not support extended attributes
  --disable-libcap        disable libcap support
  --enable-gcc-warnings   turn on lots of GCC warnings (for developers)
  --enable-install-program=PROG_LIST
                          install the programs in PROG_LIST (comma-separated,
                          default: none)
  --enable-no-install-program=PROG_LIST
                          do NOT install the programs in PROG_LIST
                          (comma-separated, default: arch,hostname,su)
  --disable-nls           do not use Native Language Support

Optional Packages:
  --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]
  --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)
  --with-gnu-ld           assume the C compiler uses GNU ld default=no
  --with-libiconv-prefix[=DIR]  search for libiconv in DIR/include and DIR/lib
  --without-libiconv-prefix     don't search for libiconv in includedir and libdir
  --with-libpth-prefix[=DIR]  search for libpth in DIR/include and DIR/lib
  --without-libpth-prefix     don't search for libpth in includedir and libdir
  --without-included-regex
                          don't compile regex; this is the default on systems
                          with recent-enough versions of the GNU C Library
                          (use with caution on other systems).
  --without-selinux       do not use SELinux, even on systems with SELinux
  --with-packager         String identifying the packager of this software
  --with-packager-version Packager-specific version information
  --with-packager-bug-reports
                          Packager info for bug reports (URL/e-mail/...)
  --with-tty-group[=NAME]
                          group used by system for TTYs, "tty" when not
                          specified (default: do not rely on any group used
                          for TTYs)
  --without-gmp           do not use the GNU MP library for arbitrary
                          precision calculation (default: use it if available)
  --with-libintl-prefix[=DIR]  search for libintl in DIR/include and DIR/lib
  --without-libintl-prefix     don't search for libintl in includedir and libdir

Some influential environment variables:
  CC          C compiler command
  CFLAGS      C compiler flags
  LDFLAGS     linker flags, e.g. -L<lib dir> if you have libraries in a
              nonstandard directory <lib dir>
  LIBS        libraries to pass to the linker, e.g. -l<library>
  CPPFLAGS    (Objective) C/C++ preprocessor flags, e.g. -I<include dir> if
              you have headers in a nonstandard directory <include dir>
  CPP         C preprocessor
  YACC        The "Yet Another C Compiler" implementation to use. Defaults to
              'bison -y'. Values other than 'bison -y' will most likely break
              on most systems.
  YFLAGS      YFLAGS contains the list arguments that will be passed by
              default to Bison. This script will default YFLAGS to the empty
              string to avoid a default value of '-d' given by some make
              applications.
  DEFAULT_POSIX2_VERSION
              POSIX version to default to; see 'config.hin'.

Use these variables to override the choices made by 'configure' or to help
it to find libraries and programs with nonstandard names/locations.

Report bugs to <bug-automake@gnu.org>.
General help using GNU software: <https://www.gnu.org/gethelp/>.
END

compare_extracted_help ()
{
  cat > exp || exit 99
  extract_configure_help "$1" help > got || exit 1
  cat exp
  cat got
  diff exp got || exit 1
}

compare_extracted_help --disable-libcap <<'END'
  --disable-libcap        disable libcap support
END

compare_extracted_help --disable-nls <<'END'
  --disable-nls           do not use Native Language Support
END

compare_extracted_help --disable-option-checking <<'END'
  --disable-option-checking  ignore unrecognized --enable/--with options
END

compare_extracted_help --build= <<'END'
  --build=BUILD     configure for building on BUILD [guessed]
END

compare_extracted_help --host= <<'END'
  --host=HOST       cross-compile to build programs to run on HOST [BUILD]
END

compare_extracted_help '--with-libintl-prefix[=]' <<'END'
  --with-libintl-prefix[=DIR]  search for libintl in DIR/include and DIR/lib
END

compare_extracted_help --without-included-regex <<'END'
  --without-included-regex
                          don't compile regex; this is the default on systems
                          with recent-enough versions of the GNU C Library
                          (use with caution on other systems).
END

compare_extracted_help --enable-threads= <<'END'
  --enable-threads={posix|solaris|pth|win32}
                          specify multithreading API
END

compare_extracted_help '--with-tty-group[=]' <<'END'
  --with-tty-group[=NAME]
                          group used by system for TTYs, "tty" when not
                          specified (default: do not rely on any group used
                          for TTYs)
END

compare_extracted_help '--prefix=' <<'END'
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [/usr/local]
END

compare_extracted_help '--exec-prefix=' <<'END'
  --exec-prefix=EPREFIX   install architecture-dependent files in EPREFIX
                          [PREFIX]
END

compare_extracted_help CC <<'END'
  CC          C compiler command
END

compare_extracted_help CPP <<'END'
  CPP         C preprocessor
END

compare_extracted_help CPPFLAGS <<'END'
  CPPFLAGS    (Objective) C/C++ preprocessor flags, e.g. -I<include dir> if
              you have headers in a nonstandard directory <include dir>
END

compare_extracted_help YACC <<'END'
  YACC        The "Yet Another C Compiler" implementation to use. Defaults to
              'bison -y'. Values other than 'bison -y' will most likely break
              on most systems.
END

compare_extracted_help DEFAULT_POSIX2_VERSION <<'END'
  DEFAULT_POSIX2_VERSION
              POSIX version to default to; see 'config.hin'.
END

for o in \
  '--nonesuch' \
  '--nonesuch=' \
  '--nonesuch[=]' \
  '--disable-nls=' \
  '--disable-nls[=]' \
  '--without-included-regex=' \
  '--without-included-regex[=]' \
  '--prefix' \
  '--prefix[=]' \
  '--build' \
  '--build[=]' \
  '--with-tty-group' \
  '--with-tty-group=' \
  'NO_SUCH_VAR' \
  'NOSUCHVAR' \
; do
  compare_extracted_help "$o" </dev/null
done

:
