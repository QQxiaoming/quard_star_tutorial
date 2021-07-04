/* Copyright 2013-2016 IBM Corp.
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

#include <skiboot.h>
#include "container.h"

bool stb_is_container(const void *buf, size_t size)
{
	ROM_container_raw *c;

	c = (ROM_container_raw*) buf;
	if (!buf || size < SECURE_BOOT_HEADERS_SIZE)
		return false;
	if (be32_to_cpu(c->magic_number) != ROM_MAGIC_NUMBER )
		return false;
	return true;
}

uint32_t stb_payload_magic(const void *buf, size_t size)
{
	uint8_t *p;
	if (!stb_is_container(buf, size))
		return 0;
	p = (uint8_t*) buf;
	return be32_to_cpu(*(uint32_t*)(p+SECURE_BOOT_HEADERS_SIZE));
}

uint64_t stb_sw_payload_size(const void *buf, size_t size)
{
	struct parsed_stb_container c;
	if (!stb_is_container(buf, size))
		return 0;
	if (parse_stb_container(buf, size, &c) != 0)
		return 0;
	return be64_to_cpu(c.sh->payload_size);
}

int parse_stb_container(const void* data, size_t len, struct parsed_stb_container *c)
{
	const size_t prefix_data_min_size = 3 * (EC_COORDBYTES * 2);
	c->buf = data;
	c->bufsz = len;
	c->c = data;
	c->ph = data += sizeof(ROM_container_raw);
	c->pd = data += sizeof(ROM_prefix_header_raw) + (c->ph->ecid_count * ECID_SIZE);
	c->sh = data += prefix_data_min_size + c->ph->sw_key_count * (EC_COORDBYTES * 2);
	c->ssig = data += sizeof(ROM_sw_header_raw) +
		c->sh->ecid_count * ECID_SIZE;

	return 0;
}

const uint8_t* stb_sw_payload_hash(const void *buf, size_t size)
{
	struct parsed_stb_container c;

	if (!stb_is_container(buf, size))
		return NULL;
	if (parse_stb_container(buf, size, &c) != 0)
		return NULL;

	return c.sh->payload_hash;
}


void stb_print_data(const void* data, size_t len)
{
	char hash[1+SHA512_DIGEST_LENGTH*2];
	char *h = hash;
	char *d = (char*)data;

	assert(len <= SHA512_DIGEST_LENGTH);

	while(len) {
		snprintf(h, 3, "%02x", *d);
		h+=2;
		d++;
		len--;
	}
	*h='\0';
	prlog(PR_NOTICE, "%s\n", hash);
}
