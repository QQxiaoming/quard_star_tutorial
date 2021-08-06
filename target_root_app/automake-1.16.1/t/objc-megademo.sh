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
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Stress test on Objective C/C++.

required=libtoolize
am_create_testdir=empty
. test-init.sh

## Autotools Input Files.

cat > configure.ac << 'END'
AC_INIT([play], [1.3], [bug-automake@gnu.org])

AC_CONFIG_SRCDIR([play.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

AM_PROG_AR
LT_INIT

AC_PROG_CC
AC_PROG_CXX
AC_PROG_OBJC
AC_PROG_OBJCXX

AC_LANG_PUSH([Objective C])
AC_CACHE_CHECK(
  [whether the Objective C compiler really works],
  [my_cv_objc_works],
  [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#import <stdio.h>]],
                                   [[printf ("foo\n");]])],
                  [my_cv_objc_works=yes],
                  [my_cv_objc_works=no])])
AC_LANG_POP([Objective C])

AC_LANG_PUSH([Objective C++])
AC_CACHE_CHECK(
  [whether the Objective C++ compiler really works],
  [my_cv_objcxx_works],
  [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#import <iostream>]],
                                   [[std::cout << "foo" << "\n";]])],
                  [my_cv_objcxx_works=yes],
                  [my_cv_objcxx_works=no])])
AC_LANG_POP([Objective C++])

if test $my_cv_objc_works != yes; then
  AC_MSG_ERROR([couldn't find a working Objective C compiler], [77])
fi

if test $my_cv_objcxx_works != yes; then
  AC_MSG_ERROR([couldn't find a working Objective C++ compiler], [77])
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = play
play_SOURCES = play.h play.c playxx.cxx playo.m playoxx.mm
play_LDADD = libfoo.la
play_LDFLAGS = -lobjc
lib_LTLIBRARIES = libfoo.la
libfoo_la_SOURCES = foo.h foo.c fooxx.cxx fooo.m foooxx.mm
END

## Run Autotools.

libtoolize
$ACLOCAL
$AUTOHEADER
$AUTOCONF
$AUTOMAKE --add-missing

## Program Sources.

cat > play.h << 'END'
#ifndef PLAY_H
#define PLAY_H

#include "foo.h"

#ifdef __cplusplus
extern "C" {
#endif

void hello_cxx (void);
void hello_objc (void);
void hello_objcxx (void);

#ifdef __OBJC__
@interface Hello_ObjC
{ }
+ (void)display;
@end
#endif /* __OBJC__ */

#ifdef __cplusplus
}

class Hello_CXX
{
  public:
    Hello_CXX() { }
    virtual ~Hello_CXX () { }
    void hello_cxx ();
};

#ifdef __OBJC__
@interface Hello_ObjCXX
{ }
+ (void)display;
@end

class Hello_OBJCXX
{
  public:
    Hello_OBJCXX () { }
    virtual ~Hello_OBJCXX () { }
    void hello_objcxx();
};
#endif /* __OBJC__ */

#endif /* __cplusplus */

#endif /* PLAY_H */
END

cat > play.c << 'END'
#include "play.h"
int main (void)
{
  printf ("[Hello C,");
  world_c ();
  hello_cxx ();
  hello_objc ();
  hello_objcxx ();
  return 0;
}
END

cat > playxx.cxx << 'END'
#include "play.h"

void hello_cxx(void)
{
  Hello_CXX *hello = new Hello_CXX;
  hello->hello_cxx();
}

void Hello_CXX::hello_cxx()
{
  std::cout << "[Hello C++,";
  World_CXX *world = new World_CXX;
  world->world_cxx();
}
END

cat > playo.m << 'END'
#import "play.h"

void hello_objc (void)
{
  [Hello_ObjC display];
}

@implementation Hello_ObjC
+ (void)display
{
  printf ("[Hello ObjC,");
  [World_ObjC display];
}
@end
END

cat > playoxx.mm << 'END'
#import "play.h"

// Calling: C -> C++ -> ObjC

void hello_objcxx (void)
{
  Hello_OBJCXX *hello = new Hello_OBJCXX;
  hello->hello_objcxx ();
}

void Hello_OBJCXX::hello_objcxx ()
{
  [Hello_ObjCXX display];
}

@implementation Hello_ObjCXX
+ (void)display
{
  std::cout << "[Hello ObjC++,";
  [World_ObjCXX display];
}
@end
END

## Library Sources.

cat > foo.h << 'END'
#ifndef FOO_H
#define FOO_H

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

void world_c (void);

#ifdef __OBJC__
@interface World_ObjC
{ }
+ (void)display;
@end
#endif /* __OBJC__ */

#ifdef __cplusplus
}

class World_CXX
{
  public:
    World_CXX() { }
    virtual ~World_CXX () { }
    void world_cxx ();
};

#ifdef __OBJC__
class World_OBJCXX
{
  public:
    World_OBJCXX () { }
    virtual ~World_OBJCXX () { }
    void world_objcxx ();
};

@interface World_ObjCXX
{ }
+ (void)display;
@end
#endif /* __OBJC__ */

#endif /* __cplusplus */

#endif /* FOO_H */
END

cat > foo.c << 'END'
#include "foo.h"

void world_c (void)
{
  printf (" world C]\n");
}
END

cat > fooxx.cxx << 'END'
#include "foo.h"

void World_CXX::world_cxx ()
{
  std::cout << " world C++]" << "\n";
}
END

cat > fooo.m << 'END'
#import "foo.h"

@implementation World_ObjC
+ (void)display
{
  printf (" world ObjC]\n");
}
@end
END

cat > foooxx.mm << 'END'
#import "foo.h"

// Calling: ObjC -> C++

@implementation World_ObjCXX
+ (void)display
{
  World_OBJCXX *world = new World_OBJCXX;
  world->world_objcxx ();
}
@end

void World_OBJCXX::world_objcxx ()
{
  std::cout << " world ObjC++]" << "\n";
}
END

## Configure and build.

./configure
$MAKE

if ! cross_compiling; then
  unindent > exp << 'END'
    [Hello C, world C]
    [Hello C++, world C++]
    [Hello ObjC, world ObjC]
    [Hello ObjC++, world ObjC++]
END
  ./play > got || { cat got; exit 1; }
  cat exp
  cat got
  diff exp got
fi

$MAKE distcheck

:
