
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

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "ext4_inode.h"
#include "ext4.h"



static void inode_dealloc(pobject o) {
    struct inode * me = (struct inode *) o;
    if(me) {
        free(me);
    }
}

struct inode * inode_create(const uint8_t * data, uint64_t offset, uint64_t ino) {
    struct inode * self = (struct inode *) ObjCreate(sizeof(struct inode));
    if(self) {
        self->o_.dealloc = inode_dealloc;
        struct ext4_inode * nd = (struct ext4_inode *) (data + offset);
        self->ino = ino;
        self->flags = nd->i_flags;
        self->mode = nd->i_mode;
        self->block_start = offset + offsetof(struct ext4_inode, i_block);
        self->size = ((uint64_t) nd->i_size_high << 32) | nd->i_size_lo;
    }
    return self;
}

