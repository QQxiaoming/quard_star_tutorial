
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

#include <string.h>
#include <stdlib.h>

#include "ext4_string.h"


static void string_dealloc(pobject o) {
    struct string * me = (struct string *) o;
    if(me && me->size) {
        free(me->s);
    }
    free(me);
}

pstring string_create(const char * literal) {
    struct string * res = (struct string *) ObjCreate(sizeof(struct string));
    if(res) {
        res->o_.dealloc = string_dealloc;
        res->size = strlen(literal);
        res->s = (char *) malloc(res->size + 1);
        memcpy(res->s, literal, res->size);
        res->s[res->size] = 0;
    }
    return res;
}

const char * string_get_string(pstring str) {
    if(str) {
        return str->s;
    }
    return NULL;
}

unsigned long string_get_size(pstring str) {
    if(str) {
        return str->size;
    }
    return 0;
}


