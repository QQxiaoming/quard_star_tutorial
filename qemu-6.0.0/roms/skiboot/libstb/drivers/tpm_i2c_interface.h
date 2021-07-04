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

#ifndef __TPM_I2C_H
#define __TPM_I2C_H

#include <i2c.h>
#include <stdlib.h>

#include "../tpm_chip.h"

extern int tpm_i2c_request_send(struct tpm_dev *tpm_device, int read_write,
				uint32_t offset, uint32_t offset_bytes, void* buf,
				size_t buflen);
#endif /* __TPM_I2C_H */
