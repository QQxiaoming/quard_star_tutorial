/* Copyright 2013-2017 IBM Corp.
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
#include <lock.h>
#include <stdint.h>

#include "../../include/lpc-mbox.h"

void check_timers(bool unused);
void time_wait_ms(unsigned long ms);
unsigned long mftb(void);
void _prlog(int log_level, const char* fmt, ...);
void bmc_put_u16(struct bmc_mbox_msg *msg, int offset, uint16_t data);
void bmc_put_u32(struct bmc_mbox_msg *msg, int offset, uint32_t data);
u16 bmc_get_u16(struct bmc_mbox_msg *msg, int offset);
u32 bmc_get_u32(struct bmc_mbox_msg *msg, int offset);
void *__zalloc(size_t sz);
void __free(const void *p);
void lock_caller(struct lock *l, const char *caller);
void unlock(struct lock *l);
