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

#ifndef __LIBFLASH_MBOX_FLASH_H
#define __LIBFLASH_MBOX_FLASH_H

int mbox_flash_lock(struct blocklevel_device *bl, uint64_t pos, uint64_t len);
int mbox_flash_init(struct blocklevel_device **bl);
void mbox_flash_exit(struct blocklevel_device *bl);
#endif /* __LIBFLASH_MBOX_FLASH_H */


