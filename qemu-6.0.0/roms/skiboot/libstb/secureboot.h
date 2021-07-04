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

#ifndef __SECUREBOOT_H
#define __SECUREBOOT_H

#include <platform.h>
#include <device.h>
#include "container.h"
#include "cvc.h"

enum secureboot_version {
	IBM_SECUREBOOT_V1,
	IBM_SECUREBOOT_SOFTROM,
	IBM_SECUREBOOT_V2,
};

bool secureboot_is_compatible(struct dt_node *node, int *version, const char **compat);
void secureboot_init(void);

/**
 * secureboot_verify - verify a PNOR partition content
 * @id   : PNOR partition id
 * @buf  : PNOR partition content to be verified
 * @len  : @buf length
 *
 * This verifies the integrity and authenticity of @buf downloaded from PNOR if
 * secure mode is on. The verification is done by the Container Verification
 * Code (CVC) flashed in ROM.
 *
 * For more information refer to 'doc/stb.rst'
 *
 * returns: 0 otherwise the boot process is aborted
 */
int secureboot_verify(enum resource_id id, void *buf, size_t len);

#endif /* __SECUREBOOT_H */
