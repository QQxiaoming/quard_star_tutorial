
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

#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include "ext4_object.h"

// A simple reference counted wrapper around the 'khash'.
// 'Khash' is a simple hash implementation.

struct map;
typedef struct map * pmap;


// Creates a map.
// Return value: the map object or NULL if error.
struct map * map_create(void);

// Inserts a reference counted object with key into the map.
// Parameters: map - the map itself.
//             key - the key.
//             value - the object to be inserted.
void map_insert(struct map * map, uint64_t key, pobject value);

// Returns the value, corresponding to a key from the map.
// Parameters: map - the map itself.
//             key - the key, whose value has to be returned.
// Return value: the object, stored with the key. The object has to be casted to the appropriate structure to get the actual data.
// Returns NULL on missing key or on error.
pobject map_get(struct map * map, uint64_t key);

// Removes the key-value pair associated with the given key.
// Parameters: map - the map object
//             key - the key whose value has to be removed.
void map_remove_key(struct map * map, uint64_t key);

// A prototype for the map iteration callback
// Parameters: void * - a pointer to an user defined data structure
//             uint64_t - the key from the key-value pair
//             pobject - a pointer to an 'object' type of object. Must be cast to get the appropriate data structure.
// Return value: uint8_t, treated as a boolean; if the callback returns TRUE the iteration stops.
typedef uint8_t (*iteration_cb)(void *, uint64_t, pobject);

// Iterates over the map, invoking a callback for every key-value pair.
// Parameters: map - the map itself.
//             cb - the callback - see the 'iteration_cb' prototype.
//             ctx - user supplied pointer to work with 'iteration_cb' callback
void map_iterate(struct map * map, iteration_cb cb, void * ctx);

#endif // MAP_H

