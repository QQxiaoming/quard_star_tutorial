/*
   The MIT License

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
#include "ext4_block_group_desc.h"

#include "ext4.h"



static void block_group_descriptor_dealloc(pobject o) {
    struct BlockGroupDescriptor * me = (struct BlockGroupDescriptor *) o;
    if(me) {
        free(me);
    }
}


struct BlockGroupDescriptor * block_group_descriptor_create(const uint8_t * data, uint64_t size, uint16_t desc_size) {
	// if the size is not matching the ext4_group_desc size then something is wrong
    if(size < sizeof(struct ext4_group_desc)) {
        return NULL;
    }

    struct BlockGroupDescriptor * self = (struct BlockGroupDescriptor *) ObjCreate(sizeof(struct BlockGroupDescriptor));
    if(self) {
        self->o_.dealloc = block_group_descriptor_dealloc;
        self->my_size = 32; // the default size

        struct ext4_group_desc * gd = (struct ext4_group_desc *) data;

        self->bg_block_bitmap = (uint64_t) gd->bg_block_bitmap_lo;
        self->bg_inode_bitmap = (uint64_t) gd->bg_inode_bitmap_lo;
        self->bg_inode_table = (uint64_t) gd->bg_inode_table_lo;
        self->bg_itable_unused = (uint64_t) gd->bg_itable_unused_lo;
        self->bg_free_blocks_count = (uint64_t) gd->bg_free_blocks_count_lo;
        self->bg_used_dirs_count = (uint64_t) gd->bg_used_dirs_count_lo;
        self->bg_exclude_bitmap = (uint64_t) gd->bg_exclude_bitmap_lo;
        self->bg_block_bitmap_csum = (uint64_t) gd->bg_block_bitmap_csum_lo;
        self->bg_inode_bitmap_csum = (uint64_t) gd->bg_inode_bitmap_csum_lo;
        self->bg_free_inodes_count = (uint64_t) gd->bg_free_inodes_count_lo;
        
        if(desc_size) { // perhaps we have large group block descriptor?
            self->my_size = 64;
            self->bg_block_bitmap |= ((uint64_t) gd->bg_block_bitmap_hi << 32);
            self->bg_inode_bitmap |= ((uint64_t) gd->bg_inode_bitmap_hi << 32);
            self->bg_inode_table |= ((uint64_t) gd->bg_inode_table_hi << 32);
            self->bg_itable_unused |= ((uint64_t)gd->bg_itable_unused_hi << 16);
            self->bg_free_blocks_count |= ((uint64_t)gd->bg_free_blocks_count_hi << 16);
            self->bg_used_dirs_count |= ((uint64_t)gd->bg_used_dirs_count_hi << 16);
            self->bg_exclude_bitmap |= ((uint64_t) gd->bg_exclude_bitmap_hi << 32);
            self->bg_block_bitmap_csum |= ((uint64_t)gd->bg_block_bitmap_csum_hi << 16);
            self->bg_inode_bitmap_csum |= ((uint64_t)gd->bg_inode_bitmap_csum_hi << 16);
            self->bg_free_inodes_count |= ((uint64_t)gd->bg_free_inodes_count_hi << 16);
        }
    }
    return self;
}


