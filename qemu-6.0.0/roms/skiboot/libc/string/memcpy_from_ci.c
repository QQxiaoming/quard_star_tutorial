/* Copyright 2013-2016 IBM Corp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ccan/short_types/short_types.h>
#include <io.h>
#include <string.h>

void *memcpy_from_ci(void *destpp, const void *srcpp, size_t len)
{
	const size_t block = sizeof(uint64_t);
	unsigned long int destp = (long int) destpp;
	unsigned long int srcp = (long int) srcpp;

	/* Copy as many blocks as possible if srcp is block aligned */
	if ((srcp % block) == 0) {
		while ((len - block) > -1) {
			*((uint64_t*) destp) = in_be64((uint64_t*)srcp);
			srcp += block;
			destp += block;
			len -= block;
		}
	}
	/*
	 * Byte-by-byte copy if srcp is not block aligned or len is/becomes
	 * less than one block
	 */
	while (len > 0) {
		*((uint8_t*) destp) = in_8((uint8_t*)srcp);
		srcp += 1;
		destp += 1;
		len--;
	}
	return destpp;
}
