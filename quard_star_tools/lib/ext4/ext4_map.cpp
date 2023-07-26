
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

#include "ext4_map.h"
#include "ext4_khash.h"



KHASH_MAP_INIT_INT64(int64_pobject_map, pobject)

struct map {
    struct object o_;
    khash_t(int64_pobject_map) * map;
};

static void map_dealloc(pobject o) {
    struct map * m = (struct map *) o;
    if(m) {
        pobject obj;
        uint64_t key;
        kh_foreach(m->map, key, obj, ObjRelease(obj); kh_val(m->map, __i) = NULL;)
        kh_destroy(int64_pobject_map, m->map);
        free(m);
        (void)(key);
    }
}


struct map * map_create(void) {
    struct map * m = (struct map *) ObjCreate(sizeof(struct map));
    if(m) {
        m->o_.dealloc = map_dealloc;
        m->map = kh_init(int64_pobject_map);
    }
    return m;
}

void map_insert(struct map * map, uint64_t key, pobject value) {
    if(map) {
        int r;
        int k = kh_put(int64_pobject_map, map->map, key, &r);
        if(!r) {
            pobject obj = kh_value(map->map, k);
            ObjRelease(obj);
        }
        kh_value(map->map, k) = value;
        ObjRetain(value);
    }
}

pobject map_get(struct map * map, uint64_t key) {
    if(map) {
        int k = kh_get(int64_pobject_map, map->map, key);
        if(k == (int)kh_end(map->map)) {
            return NULL;
        }
        return kh_value(map->map, k);
    }
    return NULL;
}

void map_remove_key(struct map * map, uint64_t key) {
    if(map) {
        int k = kh_get(int64_pobject_map, map->map, key);
        pobject obj = kh_value(map->map, k);
        ObjRelease(obj);
        kh_del(int64_pobject_map, map->map, k);
    }
}

void map_iterate(struct map * map, iteration_cb cb, void * ctx) {
    if(map) {
        uint64_t key;
        pobject obj;
        kh_foreach(map->map, key, obj, if(cb(ctx, key, obj)) break; );
    }
}
