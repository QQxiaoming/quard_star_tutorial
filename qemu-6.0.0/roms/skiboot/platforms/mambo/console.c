/* Copyright 2016 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <skiboot.h>
#include <console.h>

#include "mambo.h"

/*
 * The SIM_READ_CONSOLE callout will return -1 if there is no character to read.
 * There's no explicit poll callout so we "poll" by doing a read and stashing
 * the result until we do an actual read.
 */
static int mambo_char = -1;

static bool mambo_console_poll(void)
{
	if (mambo_char < 0)
		mambo_char = callthru0(SIM_READ_CONSOLE_CODE);

	return mambo_char >= 0;
}

static size_t mambo_console_read(char *buf, size_t len)
{
	size_t count = 0;

	while (count < len) {
		if (!mambo_console_poll())
			break;

		buf[count++] = mambo_char;
		mambo_char = -1;
	}

	return count;
}

size_t mambo_console_write(const char *buf, size_t len)
{
	callthru2(SIM_WRITE_CONSOLE_CODE, (unsigned long)buf, len);
	return len;
}

static struct con_ops mambo_con_driver = {
	.poll_read = mambo_console_poll,
	.read = mambo_console_read,
	.write = mambo_console_write,
};

void enable_mambo_console(void)
{
	prlog(PR_NOTICE, "Enabling Mambo console\n");
	set_console(&mambo_con_driver);
}

/*
 * mambo console based printf(), this is useful for debugging the console
 * since mambo_console_write() can be safely called from anywhere.
 *
 * This is a debug hack and you shouldn't use it in real code.
 */
void mprintf(const char *fmt, ...)
{
	char buf[320];
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	mambo_console_write(buf, i);
}
