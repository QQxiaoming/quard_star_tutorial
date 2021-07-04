/* Copyright 2018 IBM Corp.
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

#ifndef __LIBFLASH_IPMI_HIOMAP_H
#define __LIBFLASH_IPMI_HIOMAP_H

#include <lock.h>
#include <stdbool.h>
#include <stdint.h>

#include "blocklevel.h"

enum lpc_window_state { closed_window, read_window, write_window };

struct lpc_window {
	uint32_t lpc_addr; /* Offset into LPC space */
	uint32_t cur_pos;  /* Current position of the window in the flash */
	uint32_t size;     /* Size of the window into the flash */
};

struct ipmi_hiomap {
	/* Members protected by the blocklevel lock */
	uint8_t seq;
	uint8_t version;
	uint8_t block_size_shift;
	uint16_t timeout;
	struct blocklevel_device bl;
	uint32_t total_size;
	uint32_t erase_granule;
	struct lpc_window current;

	/*
	 * update, bmc_state and window_state can be accessed by both calls
	 * through read/write/erase functions and the IPMI SEL handler. All
	 * three variables are protected by lock to avoid conflict.
	 */
	struct lock lock;
	uint8_t bmc_state;
	enum lpc_window_state window_state;
};

int ipmi_hiomap_init(struct blocklevel_device **bl);
void ipmi_hiomap_exit(struct blocklevel_device *bl);

#endif /* __LIBFLASH_IPMI_HIOMAP_H */
