// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_C_SYSTEM_MACROS_H_
#define MOJO_PUBLIC_C_SYSTEM_MACROS_H_

#include <stddef.h>

// Assert things at compile time. (|msg| should be a valid identifier name.)
// This macro is currently C++-only, but we want to use it in the C core.h.
// Use like:
//   MOJO_STATIC_ASSERT(sizeof(Foo) == 12, "Foo has invalid size");
#if defined(__cplusplus)
#define MOJO_STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#else
#define MOJO_STATIC_ASSERT(expr, msg)
#endif

// Defines a pointer-sized struct field of the given type. This ensures that the
// field has an 8-byte footprint on both 32-bit and 64-bit systems, using an
// anonymous bitfield of either 32 or 0 bits, depending on pointer size. Weird
// formatting here courtesy of clang-format.
#define MOJO_POINTER_FIELD(type, name) \
  type name;                           \
  uint32_t:                            \
  (sizeof(void*) == 4 ? 32 : 0)

// Like the C++11 |alignof| operator.
#if defined(__GNUC__)
#define MOJO_ALIGNOF(type) __alignof__(type)
#elif __cplusplus >= 201103L
#define MOJO_ALIGNOF(type) alignof(type)
#elif defined(_MSC_VER)
// The use of |sizeof| is to work around a bug in MSVC 2010 (see
// http://goo.gl/isH0C; supposedly fixed since then).
#define MOJO_ALIGNOF(type) (sizeof(type) - sizeof(type) + __alignof(type))
#else
#error "Please define MOJO_ALIGNOF() for your compiler."
#endif

// Specify the alignment of a |struct|, etc.
// Use like:
//   struct MOJO_ALIGNAS(8) Foo { ... };
// Unlike the C++11 |alignas()|, |alignment| must be an integer. It may not be a
// type, nor can it be an expression like |MOJO_ALIGNOF(type)| (due to the
// non-C++11 MSVS version).
#if __cplusplus >= 201103L
#define MOJO_ALIGNAS(alignment) alignas(alignment)
#elif defined(__GNUC__)
#define MOJO_ALIGNAS(alignment) __attribute__((aligned(alignment)))
#elif defined(_MSC_VER)
#define MOJO_ALIGNAS(alignment) __declspec(align(alignment))
#else
#error "Please define MOJO_ALIGNAS() for your compiler."
#endif

#endif  // MOJO_PUBLIC_C_SYSTEM_MACROS_H_
