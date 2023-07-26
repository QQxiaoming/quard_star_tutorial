
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
#include "ext4_data.h"
#include <string.h>
#include <stdlib.h>

static void data_dealloc(pobject o) {
    struct data * d = (struct data *) o;
    if(d) {
        if(d->m) {
            free(d->m);
        }
        free(d);
    }
}

struct data * data_create(void * mem, uint64_t length) {
    if(mem && length) {
        struct data * d = (struct data *) ObjCreate(sizeof(struct data));
        if(d) {
            d->o_.dealloc = data_dealloc;
            d->len = length;
            d->m = (uint8_t *) malloc(length);
            if(d->m) {
                memcpy(d->m, mem, length);
                return d;
            }
            free(d);
        }
    }
    return NULL;
}

