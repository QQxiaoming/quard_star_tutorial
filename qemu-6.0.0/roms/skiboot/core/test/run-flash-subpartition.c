/* Copyright 2013-2016 IBM Corp.
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

#include <skiboot.h>
#include <opal-api.h>
#include <stdlib.h>

#include "../flash-subpartition.c"
#include <assert.h>

/* This is a straight dump of the CAPP ucode partition header */
char capp[4096] = {0x43, 0x41, 0x50, 0x50, 0x00, 0x00, 0x00, 0x01,
		   0x00, 0x01, 0x00, 0xea, 0x00, 0x00, 0x10, 0x00,
		   0x00, 0x00, 0x8e, 0x50, 0x00, 0x02, 0x00, 0xea,
		   0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x8e, 0x50,
		   0x00, 0x02, 0x00, 0xef, 0x00, 0x00, 0x10, 0x00,
		   0x00, 0x00, 0x8e, 0x50, 0x00, 0x02, 0x01, 0xef,
		   0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x8e, 0x50,
		   0x00, 0x01, 0x00, 0xd3, 0x00, 0x00, 0x10, 0x00,
		   0x00, 0x00, 0x8e, 0x50, 0x00, 0x00, 0x00, 0x00 };

int main(void)
{
	int rc;
	uint32_t part_actual;
	uint32_t offset;
	uint32_t size;
	uint32_t subids[] = { 0x100ea, 0x200ea, 0x200ef, 0x201ef, 0x100d3 };

	for (int i = 0; i < sizeof(subids)/sizeof(uint32_t); i++) {
		offset = 0;
		rc = flash_subpart_info(capp, sizeof(capp), 0x24000,
					&part_actual, subids[i],
					&offset, &size);
		printf("\nsubid %x\n", subids[i]);
		printf("part_actual %u\n", part_actual);
		printf("offset %u\n", offset);
		printf("size %u\n", size);
		assert (rc == 0);
		assert (size == 36432);
		assert (offset == 4096);
		assert (part_actual == 40960);
	}

	return 0;
}
