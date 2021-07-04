/* Copyright 2013-2014 IBM Corp.
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

#include <config.h>

#include "../container.c"

#include <assert.h>

int main(void)
{
	ROM_container_raw *c = malloc(SECURE_BOOT_HEADERS_SIZE);
	assert(stb_is_container(NULL, 0) == false);
	assert(stb_is_container(NULL, SECURE_BOOT_HEADERS_SIZE) == false);
	c->magic_number = cpu_to_be32(ROM_MAGIC_NUMBER + 1);
	assert(stb_is_container(c, SECURE_BOOT_HEADERS_SIZE) == false);
	c->magic_number = cpu_to_be32(ROM_MAGIC_NUMBER);
	assert(stb_is_container(c, SECURE_BOOT_HEADERS_SIZE) == true);

	return 0;
}
