
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ext4_fs.h"

#include "ext4.h"

#include "ext4_super_block.h"
#include "ext4_block_group_desc.h"
#include "ext4_fs_entity.h"
#include "ext4_inode.h"
#include "ext4_list.h"
#include "ext4_map.h"
#include "ext4_data.h"

#if defined(_WIN32)
char *strsep(char **stringp, const char *delim) {
    char *rv = *stringp;
    if (rv) {
        *stringp += strcspn(*stringp, delim);
        if (**stringp)
            *(*stringp)++ = '\0';
        else
            *stringp = 0; }
    return rv;
}
#endif

struct FS {
    struct SuperBlock * super_block;
    uint64_t inode_table;
    pmap inodes;
    pmap entities;
};

struct FSContext {
    struct SuperBlock * sb;
    pmap inodes;
    pmap entities;
    uint64_t inode_table_start;
};

struct DataRef {
    uint64_t start;
    uint64_t length;
};

void fs_destroy(struct FS * fs) {
    if(fs) {
        RELEASE(fs->super_block);
        RELEASE(fs->inodes);
        RELEASE(fs->entities);
        free(fs);
    }
}

typedef  void (*extractor)(const uint8_t * data, uint64_t data_size, struct fs_entity * parent_entity, struct inode * inode, uint64_t offset, uint64_t length, struct FSContext * ctx);
static void extract_data(const uint8_t * data, uint64_t data_size, struct fs_entity * start_entity, struct inode * start_inode, uint64_t offset, extractor extr, struct FSContext * ctx);

char entity_name[EXT4_NAME_LEN + 1];


static void read_symlink(const uint8_t * data, uint64_t data_size, struct fs_entity * parent_entity, struct inode * inode, uint64_t offset, uint64_t length, struct FSContext * ctx) {
    if((inode->mode & 0xf000) == S_EIFLNK) {
        struct fs_entity * entity = (struct fs_entity *) map_get(ctx->entities, inode->ino);
        if(entity) {
            memcpy(entity_name, data + offset, inode->size);
            entity_name[inode->size] = 0;
            assert(entity->links_to == NULL);
            entity->links_to = string_create(entity_name);
        } else {
            assert(0);
        }
    }
    (void)(data_size);
    (void)(parent_entity);
    (void)(length);
}

static void entity_processor(const uint8_t * data, uint64_t data_size, struct fs_entity * parent_entity, struct inode * inode, uint64_t offset, uint64_t length, struct FSContext * ctx) {
    //assert(!(inode->flags & EXT4_INDEX_FL));
    if(inode->flags & EXT4_INDEX_FL) return;
    const uint8_t * ptr = data + offset;
    uint64_t consumed = 0;
    uint16_t what = inode->mode & 0xf000;
    switch(what) {
        case S_EIFDIR:
            while(consumed < length) {
                struct ext4_dir_entry_2 * de = (struct ext4_dir_entry_2 *) ptr;
                if((de->inode == 0) && (de->rec_len == 0xc) && (de->name_len == 0) && (de->file_type == 0xde)) {
                    break;
                }
                if((de->rec_len < EXT4_DIR_REC_LEN(1)) || (de->rec_len & 3)) {
                    break;
                }
                consumed += de->rec_len;
                ptr += de->rec_len;
                memcpy(entity_name, de->name, de->name_len);
                entity_name[de->name_len] = 0;
                if(!strcmp(entity_name, ".") || !strcmp(entity_name, "..")) {
                    continue;
                }
                if(de->inode > 0) {
                    struct fs_entity * entity = create_entity((uint64_t) de->inode, entity_name, (fs_entity_type)de->file_type, parent_entity);
                    if(entity) {
                        map_insert(ctx->entities, entity->ino, OBJ(entity));
                        list_append(parent_entity->children, OBJ(entity));
                        RELEASE(entity);
                        if(!map_get(ctx->inodes, entity->ino)) {
                            uint64_t n_inodes = (uint64_t)(de->inode - 1) * ctx->sb->inode_size;
                            uint64_t n_blocks = n_inodes / ctx->sb->block_size;
                            uint64_t offset_in_block = n_inodes % ctx->sb->block_size;
                            uint64_t inode_pos = (ctx->inode_table_start + n_blocks) * ctx->sb->block_size + offset_in_block;
                            struct inode * new_inode = inode_create(data, inode_pos, (uint64_t)de->inode);
                            if(new_inode) {
                                map_insert(ctx->inodes, (uint64_t)new_inode->ino, OBJ(new_inode));
                                RELEASE(new_inode);
                            } else {
                                assert(0);
                            }
                            struct fs_entity * ent = parent_entity;
                            if(entity->type == DIR) {
                                ent = entity;
                            }
                            if(new_inode->flags & EXT4_EXTENTS_FL) {
                                extract_data(data, data_size, ent, new_inode, new_inode->block_start, entity_processor, ctx);
                            } else if((new_inode->flags & 0xf000) == S_EIFLNK) {
                                char * n = (char *)(data + new_inode->block_start);
                                assert(entity->links_to == NULL);
                                entity->links_to = string_create(n);
                            }
                        }
                    }
                }
            }
            break;
        case S_EIFREG:
        {
            struct fs_entity * ent = (struct fs_entity *) map_get(ctx->entities, inode->ino);
            if(ent) {
                ent->data_length = inode->size;
                struct DataRef dr = { offset, inode->size };
                struct data * d = data_create(&dr, sizeof(dr));
                if(d) {
                    list_append(ent->data_refs, OBJ(d));
                    RELEASE(d);
                }
            } else {
                assert(0);
            }
        }
            break;
        case S_EIFLNK:
            if(inode->flags & EXT4_EXTENTS_FL) {
                extract_data(data, data_size, parent_entity, inode, inode->block_start, read_symlink, ctx);
            }
            break;
        default:
            assert(0);
            break;
    }
}

static void extract_data(const uint8_t * data, uint64_t data_size, struct fs_entity * start_entity, struct inode * start_inode, uint64_t offset, extractor extr, struct FSContext * ctx) {
    struct ext4_extent_header * ehdr = (struct ext4_extent_header *) (data + offset);
    if(ehdr->eh_magic == EXT4_EXT_MAGIC) {
        uint16_t i;
        uint64_t new_offset;
        uint8_t * ptr = (((uint8_t *) ehdr) + sizeof(struct ext4_extent_header));
        for(i = 0; i < ehdr->eh_entries; i ++) {
            if(ehdr->eh_depth == 0) {
                struct ext4_extent * xt = (struct ext4_extent *) ptr;
                uint64_t ee_start = ((uint64_t) xt->ee_start_hi << 32) | xt->ee_start_lo;
                new_offset = ee_start * ctx->sb->block_size;
                struct fs_entity * current = (struct fs_entity *) map_get(ctx->entities, start_inode->ino);
                uint64_t len = xt->ee_len * ctx->sb->block_size;
                if(current && (current->type == REG_FILE) && (current->data_length > 0)) {
                    struct DataRef dr = { new_offset, len };
                    struct data * d = data_create(&dr, sizeof(dr));
                    if(d) {
                        list_append(current->data_refs, OBJ(d));
                        RELEASE(d);
                    }
                    ptr += sizeof(*xt);
                    continue;
                }
                extr(data, data_size, start_entity, start_inode, new_offset, len, ctx);
                ptr += sizeof(*xt);
            } else {
                struct ext4_extent_idx * ix = (struct ext4_extent_idx *) ptr;
                ptr += sizeof(*ix);
                uint64_t ei_leaf = ((uint64_t) ix->ei_leaf_hi << 32) | ix->ei_leaf_lo;
                new_offset = ei_leaf * ctx->sb->block_size;
                extract_data(data, data_size, start_entity, start_inode, new_offset, extr, ctx);
            }
        }
    }
}

#define ROOT_INODE 2

struct FS * fs_create(const uint8_t * data, uint64_t data_size) {
    const uint8_t * ptr;
    uint64_t remaining_size = data_size;
    struct FS * self = (struct FS *) malloc(sizeof(struct FS));
    if(self) {
        ptr = data + 0x400;
        remaining_size = data_size - 0x400;
        struct SuperBlock * sb = super_block_create(ptr, remaining_size);
        self->super_block = sb;
        uint64_t num_block_desc = 1 + sb->blocks_count / sb->blocks_per_group;
        plist group_block_descirptors = list_create();
        struct FSContext ctx;
        ctx.sb = sb;
        ctx.entities = map_create();
        ctx.inodes = map_create();
        
        if(group_block_descirptors) {
            ptr = data + 0x1000;
            remaining_size = data_size - 0x1000;
            uint64_t c = 0;
            while(c < num_block_desc) {
                struct BlockGroupDescriptor * bldesc = block_group_descriptor_create(ptr, remaining_size, sb->desc_size);
                ptr += bldesc->my_size;
                remaining_size -= sizeof(struct ext4_group_desc);
                list_append(group_block_descirptors, OBJ(bldesc));
                RELEASE(bldesc);
                ++ c;
            }
            struct BlockGroupDescriptor * start_bd = (struct BlockGroupDescriptor *) list_at(group_block_descirptors, sb->block_group_nr);
            ctx.inode_table_start = start_bd->bg_inode_table;
            RELEASE(group_block_descirptors);
        } else {
            RELEASE(ctx.entities);
            RELEASE(ctx.inodes);
            RELEASE(ctx.sb);
            RELEASE(self);
            return NULL;
        }
        uint64_t ino = ROOT_INODE;
        uint64_t n_inodes = (ino - 1) * sb->inode_size;
        uint64_t n_blocks = n_inodes / sb->block_size;
        uint64_t offset_in_block = n_inodes % sb->block_size;
        uint64_t root_inode_pos = (ctx.inode_table_start + n_blocks) * sb->block_size + offset_in_block;
        struct inode * inode = inode_create(data, root_inode_pos, ino);
        map_insert(ctx.inodes, ino, OBJ(inode));
        RELEASE(inode);
        struct fs_entity * root = create_entity(ino, "/", DIR, NULL);
        map_insert(ctx.entities, ino, (pobject) root);
        ObjRelease((pobject) root);
        
        if(inode->flags & EXT4_EXTENTS_FL) {
            extract_data(data, data_size, root, inode, inode->block_start, entity_processor, &ctx);
        }
        self->inodes = ctx.inodes;
        self->entities = ctx.entities;
        self->inode_table = ctx.inode_table_start;
        
    }
    return self;
}

uint64_t fs_list_contents(struct FS * fs, const char * path, uint8_t * outdata) {
    struct fs_entity * node = NULL;
    uint64_t size = 0;
    if(fs) {
        node = (struct fs_entity *) map_get(fs->entities, ROOT_INODE);
        if(!node) {
            goto l_abort;
        }
        pstring spath = string_create(path);
        if(spath) {
            char * ptr = (char *) string_get_string(spath);
            char * t;
            plist components = list_create();
            if(components) {
                uint16_t c = 0;
                while(((void)(t = strsep(&ptr, "/")), t)) {
                    if(strlen(t)) {
                        pstring ts = string_create(t);
                        if(ts) {
                            list_append(components, OBJ(ts));
                            ++ c;
                            RELEASE(ts);
                        }
                    }
                }
                void * it, * it2;
                uint8_t done = 0;
                uint8_t fail = 0;
                if(!c) {
                    goto l_iterate;
                }
                while(!done && !fail) {
                    for(it = list_begin(components); it != list_end(components); it = list_next(it)) {
                        const char * cname = string_get_string((pstring) list_access(it));
                        struct fs_entity * the_child = NULL;
                        for(it2 = list_begin(node->children); it2 != list_end(node->children); it2 = list_next(it2)) {
                            struct fs_entity * child = (struct fs_entity *) list_access(it2);
                            const char * chname = string_get_string(child->name);
                            if(!strcmp(chname, cname)) {
                                the_child = child;
                                break;
                            }
                        }
                        if(!the_child) {
                            fail = 1;
                        } else {
                            node = the_child;
                        }
                    }
                    done = 1;
                }
l_iterate:
                if(node && !fail) {
                    void * it;
                    uint8_t * optr = outdata;
                    for(it = list_begin(node->children); it != list_end(node->children); it = list_next(it)) {
                        struct fs_entity * en = (struct fs_entity *) list_access(it);
                        uint8_t s = (uint8_t) string_get_size(en->name);
                        size += sizeof(uint64_t) + 2 + s;
                        if(optr) {
                            *(uint64_t *) optr = en->ino;
                            optr[sizeof(uint64_t)] = en->type;
                            optr[sizeof(uint64_t)+1] = s;
                            memcpy(optr + sizeof(uint64_t) + 2, string_get_string(en->name), s);
                            optr += sizeof(uint64_t) + 2 + s;
                        }
                    }
                }
                RELEASE(components);
            }
            RELEASE(spath);
        }
    }
l_abort:
    return size;
}

uint64_t fs_get_contents(struct FS * fs, uint64_t ino, uint8_t * outdata) {
    uint64_t size = 0;
    if(fs) {
        struct fs_entity * entity = (struct fs_entity *) map_get(fs->entities, ino);
        if(entity) {
            void * it;
            uint8_t * ptr = outdata;
            for(it = list_begin(entity->data_refs); it != list_end(entity->data_refs); it = list_next(it)) {
                struct data * d = (struct data *) list_access(it);
                struct DataRef * dr = (struct DataRef *) d->m;
                size += d->len;
                if(ptr) {
                    *(struct DataRef *) ptr = *dr;
                    ptr += sizeof(struct DataRef);
                }
            }
        }
    }
    return size;
}
