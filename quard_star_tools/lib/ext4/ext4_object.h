
/* The MIT License

   Copyright (c) 2020 by ggetchev

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#ifndef OBJECT_H
#define OBJECT_H

// A reference counted object representation.
// Note: The object is not thread safe!

struct object {
    long rc;
    long (*retain)(struct object *);
    long (*release)(struct object *);
    void (*dealloc)(struct object *);
};

typedef struct object * pobject;

pobject ObjCreate(unsigned long size);

long ObjRetain(struct object * o);
long ObjRelease(struct object * o);

#define OBJ(x) ((struct object *) (x))

// Macros for convenience

#define RETAIN(x) ObjRetain(OBJ(x))
#define RELEASE(x) ObjRelease(OBJ(x))

#endif // OBJECT_H

