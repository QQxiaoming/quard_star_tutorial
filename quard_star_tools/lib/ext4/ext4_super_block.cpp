
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
#include "ext4_super_block.h"

#include "ext4.h"

#include "ext4_object.h"


static void super_block_dealloc(pobject o) {
    struct SuperBlock * sb = (struct SuperBlock *) o;
    if(sb) {
        free(sb);
    }
}

struct SuperBlock * super_block_create(const uint8_t * data, uint64_t size) {
    if(size < sizeof(struct ext4_super_block)) {
        return NULL;
    }
    struct SuperBlock * self = (struct SuperBlock *) ObjCreate(sizeof(struct SuperBlock));
    if(self) {
        struct ext4_super_block * sb = (struct ext4_super_block *) data;
        self->o_.dealloc = super_block_dealloc;
        self->blocks_count = (uint64_t) sb->s_blocks_count_lo | ((uint64_t) sb->s_blocks_count_hi << 32);
        self->block_size = (uint64_t) 0x400UL << sb->s_log_block_size;
        self->blocks_per_group = sb->s_blocks_per_group;
        self->block_group_size = self->block_size * self->blocks_per_group;
        self->inodes_per_group = sb->s_inodes_per_group;
        self->cluster_size = (uint64_t) 0x400UL << sb->s_log_cluster_size;
        self->has_dir_indices = !!(sb->s_feature_compat & 0x20);
        self->sparse_super_block = !!(sb->s_feature_compat & 0x200);
        self->flexible_block_groups = !!(sb->s_feature_incompat & 0x200);
        self->flex_block_size = (uint64_t) 1UL << sb->s_log_groups_per_flex;
        self->huge_file = !!(sb->s_feature_ro_compat & 0x8);
        self->inode_size = sb->s_inode_size;
        self->desc_size = 0;
        if(sb->s_feature_compat & 0x80) {
            self->desc_size = sb->s_desc_size;
        }
        self->free_inodex_count = sb->s_free_inodes_count;
        self->free_blocks_count = (uint64_t) sb->s_free_blocks_count_lo | ((uint64_t) sb->s_free_blocks_count_hi << 32);
        self->feature_compat = sb->s_feature_compat;
        self->feature_incompat = sb->s_feature_incompat;
        self->feature_ro_compat = sb->s_feature_ro_compat;
        self->block_group_nr = sb->s_block_group_nr;
    }
    return self;
}

