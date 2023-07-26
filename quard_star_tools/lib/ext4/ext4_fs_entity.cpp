
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

#include "ext4_fs_entity.h"

static const char * fs_entity_str_types[LAST] = {
  "unknown", "file", "directory", "character device", "block device", "fifo", "socket", "symbolic link"
};

static void fs_entity_dealloc(pobject o) {
    struct fs_entity * me = (struct fs_entity *) o;
    if(me) {
        RELEASE(me->name);
        RELEASE(me->children);
        RELEASE(me->links_to);
        RELEASE(me->data_refs);
        free(me);
    }
}

struct fs_entity * create_entity(uint64_t ino, const char * name, enum fs_entity_type type, struct fs_entity * parent) {
    struct fs_entity * me = (struct fs_entity *) ObjCreate(sizeof(struct fs_entity));
    if(me) {
        me->_o.dealloc = fs_entity_dealloc;
        me->ino = ino;
        me->name = string_create(name);
        me->type = type;
        me->parent = parent;
        me->children = list_create();
        me->data_refs = list_create();
        me->links_to = NULL;
        me->data_length = 0;
    }
    return me;
}

const char * fs_entity_get_type(pobject o) {
    struct fs_entity * fe = (struct fs_entity *) o;
    if(fe->type < LAST) {
    	return fs_entity_str_types[fe->type];
    }
    return fs_entity_str_types[UNKNOWN];
}

