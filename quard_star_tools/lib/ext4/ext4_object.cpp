
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

#include "ext4_object.h"

#include <stdlib.h>

static void object_deallocate(struct object * o) {
    free(o);
}

static long object_retain(struct object * o) {
    o->rc ++;
    return o->rc;
}

static long object_release(struct object * o) {
    -- o->rc;
    long r = o->rc;
    if(!r) {
        o->dealloc(o);
    }
    return r;
}

long ObjRetain(struct object * o) {
    if(!o) {
        return 0;
    }
    return o->retain(o);
}

long ObjRelease(struct object * o) {
    if(!o) {
        return 0;
    }
    return o->release(o);
}


pobject ObjCreate(unsigned long size) {
    struct object * o = (struct object *) malloc(size);
    if(o) {
        o->rc = 1;
        o->dealloc = object_deallocate;
        o->retain = object_retain;
        o->release = object_release;
    }
    return o;
}
