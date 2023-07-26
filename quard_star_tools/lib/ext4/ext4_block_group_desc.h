
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

#ifndef BLOCK_GROUP_DESC_H
#define BLOCK_GROUP_DESC_H

#include <stdint.h>
#include "ext4_object.h"

// A reference counted representation of a block group descriptor

struct BlockGroupDescriptor {
    struct object o_;
    uint64_t bg_block_bitmap;
    uint64_t bg_inode_bitmap;
    uint64_t bg_inode_table;
    uint64_t bg_itable_unused;
    uint64_t bg_free_blocks_count;
    uint64_t bg_used_dirs_count;
    uint64_t bg_exclude_bitmap;
    uint64_t bg_block_bitmap_csum;
    uint64_t bg_inode_bitmap_csum;
    uint64_t bg_free_inodes_count;
    uint8_t my_size;
};

// Creates an in-memory representation of a block group descriptor, based on the block group descriptor from the disk image.
// Returns NULL on failure.

struct BlockGroupDescriptor * block_group_descriptor_create(const uint8_t * data, uint64_t size, uint16_t desc_size);

#endif // BLOCK_GROUP_DESC_H
