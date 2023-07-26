
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

#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

#include <stdint.h>
#include "ext4_object.h"

// A reference counted representation of a super block.

struct SuperBlock {
    struct object o_;
    uint64_t blocks_count;
    uint64_t block_size;
    uint64_t blocks_per_group;
    uint64_t block_group_size;
    uint64_t inodes_per_group;
    uint64_t cluster_size;
    uint8_t has_dir_indices;
    uint8_t sparse_super_block;
    uint8_t flexible_block_groups;
    uint64_t flex_block_size;
    uint8_t  huge_file;
    uint64_t inode_size;
    uint64_t desc_size;
    uint64_t free_inodex_count;
    uint64_t free_blocks_count;
    uint64_t feature_compat;
    uint64_t feature_incompat;
    uint64_t feature_ro_compat;
    uint16_t block_group_nr;
};

// Creates an in-memory representation of a super block, based on the super block from the disk image.
// Returns NULL on failure.
struct SuperBlock * super_block_create(const uint8_t * data, uint64_t size);



#endif // SUPER_BLOCK_H

