/* Copyright 2017 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SKIBOOT_VALGRIND_H
#define __SKIBOOT_VALGRIND_H

#ifdef USE_VALGRIND
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>
#else

#define RUNNING_ON_VALGRIND    0

#define VALGRIND_MAKE_MEM_UNDEFINED(p, len)	\
	do { 					\
		(void)(p);			\
		(void)(len);			\
	} while (0)

#endif

#endif /* __SKIBOOT_VALGRIND_H */
