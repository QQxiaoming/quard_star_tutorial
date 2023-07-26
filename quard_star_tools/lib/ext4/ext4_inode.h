
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

#ifndef INODE_H
#define INODE_H

#include <stdint.h>
#include "ext4_object.h"

// A simple in-memory reference counted representation of the inode

struct inode {
    struct object o_;
    uint64_t ino;
    uint16_t mode;
    uint32_t flags;
    uint64_t size;
    uint64_t block_start;
};

typedef struct inode * pinode;

// Creates an inode
// Parameters: data - the disk image data
//             offset - the offset in the disk image, where the inode is
//             ino - the inode index
// Return value: the inode representation or NULL if error.
struct inode * inode_create(const uint8_t * data, uint64_t offset, uint64_t ino);


#endif // INODE_H

