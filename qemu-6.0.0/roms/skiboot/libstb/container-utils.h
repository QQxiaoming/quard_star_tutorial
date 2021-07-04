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

#ifndef _CREATE_CONTAINER_UTILS_H
#define _CREATE_CONTAINER_UTILS_H

#include <stdio.h>
#include <unistd.h>

#define die(status, msg, ...)						\
        { fprintf(stderr, "error: %s.%s() line %d: " msg "\n", progname, \
        		__func__, __LINE__, __VA_ARGS__); exit(status); }

#define debug_msg(msg, ...) \
        if (debug) fprintf(stderr, "--> %s.%s(): " msg "\n", progname, \
        		__func__, __VA_ARGS__);

#define verbose_msg(msg, ...) \
        if (verbose) fprintf(stdout, "--> %s: " msg "\n", progname, \
        		__VA_ARGS__);

void hex_print(char *lead, unsigned char *buffer, size_t buflen);
void verbose_print(char *lead, unsigned char *buffer, size_t buflen);
void debug_print(char *lead, unsigned char *buffer, size_t buflen);
int isValidHex(char *input, int len);
int isValidAscii(char *input, int len);

#endif /* _CREATE_CONTAINER_UTILS_H */
