
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

#include "ext4_module.h"

#include "ext4_fs.h"

static fsref fs = (fsref) 0;

void ext4_init(const uint8_t * data, uint64_t data_size) {
    if(!fs) {
        fs = fs_create(data, data_size);
    }
}

void ext4_close(void) {
    if(fs) {
        fs_destroy(fs);
        fs = 0;
    }
}

uint64_t ext4_list_contents(const char * path, uint8_t * outdata) {
    if(fs) {
        return fs_list_contents(fs, path, outdata);
    }
    return 0;
}

uint64_t ext4_get_contents(uint64_t ino, uint8_t * outdata) {
    if(fs) {
        return fs_get_contents(fs, ino, outdata);
    }
    return 0;
}

