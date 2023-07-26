
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

#ifndef FS_ENTITY_H
#define FS_ENTITY_H

#include <stdint.h>

#include "ext4_string.h"
#include "ext4_list.h"

enum fs_entity_type {
    UNKNOWN = 0,
    REG_FILE,
    DIR,
    CHARDEV,
    BLOCKDEV,
    FIFO,
    SOCKET,
    SYMLINK,
    LAST
};

// A simple in-memory reference counted representation of the filesystem object

struct fs_entity {
    struct object _o;
    uint64_t ino;
    pstring  name; // the name of the object
    enum fs_entity_type type; // the type of the object
    plist children; // strong pointers to the children
    pstring links_to; // if it is a symbolic link
    plist data_refs; // strong pointers to all data chunks for that object
    uint64_t data_length; // total data length
    struct fs_entity * parent; // that is weak pointer to the parent
};


// Creates an entity.
// Parameters: ino - the ino of the entity, as it is in the disk image.
//             name - the name of the object, as it is in the disk image.
//             type - the type of the object, as it is in the disk image.
//             parent- the object's parent (a directory usually).
// Return value: the fs_entity object, representing the disk image object or NULL if error.
struct fs_entity * create_entity(uint64_t ino, const char * name, enum fs_entity_type type, struct fs_entity * parent);

// Gets the string representation of the type of the object.
// Parameters: o - the fs_entity object itself
// Return value: a human-readable string, according to the 'fs_entity_type' defined above or NULL if error.
const char * fs_entity_get_type(pobject o);

#endif // FS_ENTITY_H

