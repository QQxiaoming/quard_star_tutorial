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

#include <stdint.h>

#define MAX_PATH_ELEMENTS 10
#define PATH_TYPE_SHIFT 4
#define PATH_ELEMENTS_MASK (0x0F)

/*
 * Sourced from hostboot: src/include/usr/hwas/common/hwasCallout.H
 */
enum gard_reason {
	GARD_NO_REASON = 0x0,
	GARD_MANUAL = 0xD2,
	GARD_UNRECOVERABLE = 0xE2,
	GARD_FATAL = 0xE3,
	GARD_PREDICTIVE = 0xE6,
	GARD_POWER = 0xE9,
	GARD_HYP = 0xEA,
	GARD_RECONFIG = 0xEB,

	/*
	 * This should only occur if the GUARD partition isn't correctly
	 * programmed with ECC bits.
	 */
	GARD_VOID = 0xFF,
};

/* see src/include/usr/targeting/common/entitypath.H */
enum path_type {
	PATH_NA			= 0x00,
	PATH_AFFINITY		= 0x01,
	PATH_PHYSICAL		= 0x02,
	PATH_DEVICE		= 0x03,
	PATH_POWER		= 0x04,
};

struct path_element {
	uint8_t		target_type;
	uint8_t		instance;
} __attribute__((packed));

struct entity_path {
	/* First 4 bits are a path_type enum */
	/* Second 4 bits are the amount of path_elements */
	uint8_t			type_size;
	struct path_element	path_elements[MAX_PATH_ELEMENTS];

} __attribute__((packed));

/* From hostboot: src/include/usr/hwas/common/deconfigGard.H:GardRecord */
struct gard_record {
	uint32_t		record_id;
	struct entity_path	target_id;
	uint8_t			pad0[3]; /* compiler dependent padding */
	uint32_t		errlog_eid;
	uint8_t			error_type;
	uint8_t			resource_recovery;
	uint8_t			pad1[6];
} __attribute__((packed));

#define MAX_PATH_SIZE 420

struct chip_unit_desc {
	int type;
	const char *desc;
};

extern const struct chip_unit_desc *chip_units;
extern const struct chip_unit_desc p8_chip_units[];
extern const struct chip_unit_desc p9_chip_units[];
