/* Copyright 2014-2016 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * For now it just validates that addresses passed are sane and test the
 * wrapper that validates addresses
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <compiler.h>
#include <opal-internal.h>

#define __TEST__
unsigned long top_of_ram;	/* Fake it here */
int main(void)
{
	unsigned long addr = 0xd000000000000000;

	top_of_ram = 16ULL * 1024 * 1024 * 1024; /* 16 GB */
	assert(opal_addr_valid((void *)addr) == false);

	addr = 0xc000000000000000;
	assert(opal_addr_valid((void *)addr) == true);

	addr = 0x0;
	assert(opal_addr_valid((void *)addr) == true);

	addr = ~0;
	assert(opal_addr_valid((void *)addr) == false);

	addr = top_of_ram + 1;
	assert(opal_addr_valid((void *)addr) == false);
	return 0;
}
