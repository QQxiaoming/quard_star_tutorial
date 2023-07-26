
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

#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include "ext4_object.h"


// A reference counted wrapper of the 'klist'.
// 'klist' is a very simple list implementation, see klist.h for more details.

struct list;
typedef struct list * plist;

// Creates a list object.
// Return value: the list object or NULL if error.
struct list * list_create(void);

// Appends an object to the list.
// Parameters: lst - the list itself.
//             obj - a reference counted object to be added to the list.
void list_append(struct list * lst, pobject obj);

// Returns an iterator to the list's head.
// Parameters: lst - the list object itself.
// Return value: an iterator to the begin of the list (the head).
void * list_begin(struct list *  lst);

// Returns an iterator to the list's tail.
// Parameters: lst - the list object itself.
// Return value: an iterator to the end of the list (the tail).
void * list_end(struct list * lst);

// Moves forward the iterator, returns the next iterator or NULL if end is reached.
// Parameters: it - the iterator itself.
// Return value: the next iterator or NULL if the end is reached.
void * list_next(void * it);

// Returns the underlying object from the iterator or NULL if error.
// Parameters: it - the iterator itself.
// Return value: pointer to the underlying object. The object is of type 'object' so a cast is necessary to access the actual data.
pobject list_access(void * it);

// Returns the underlying object from a list, by given index or NULL if error.
// Parameters: lst - the list itself.
//             index - the index inside the list (as if the list is an array)
// Return value: pointer to the underlying object. The object is of type 'object' so a cast is necessary to access the actual data.
pobject list_at(struct list * lst, int index);

// Returns the list size in elements or 0 if error.
// Parameters: lst - the list itself.
// Return value: the size of the list in elements. Returns 0 if the list is empty OR if the 'lst' points to a NULL pointer.
uint64_t list_size(struct list * lst);

#endif // LIST_H

