#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Basic tests on the Objective C support that don't actually
# require an Objective-C compiler.
# See also sister test 'objcxx-basic.sh'.

. test-init.sh

cat > Makefile.am <<'END'
bin_PROGRAMS = hello
hello_SOURCES = hello.m
END

$ACLOCAL
AUTOMAKE_fails
grep "'OBJC'.* undefined" stderr
grep "add .*'AC_PROG_OBJC'" stderr

rm -rf autom4te*.cache

echo AC_PROG_OBJC >> configure.ac

$ACLOCAL
$AUTOMAKE
$EGREP '^\.SUFFIXES:.* \.m( |$)' Makefile.in

:
