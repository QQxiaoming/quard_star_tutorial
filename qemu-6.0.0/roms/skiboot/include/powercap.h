/* Copyright 2017 IBM Corp.
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

#ifndef __POWERCAP_H
#define __POWERCAP_H

#include <opal.h>

enum powercap_class {
	POWERCAP_CLASS_OCC,
};

/*
 * Powercap handle is defined as u32. The first and last bytes are
 * used to indicate the class and attribute.
 *
 *	| Class |    Reserved   | Attribute |
 *	|-------|---------------|-----------|
 */

#define powercap_make_handle(class, attr) (((class & 0xF) << 24) | (attr & 0xF))

#define powercap_get_class(handle)	((handle >> 24) & 0xF)
#define powercap_get_attr(handle)	(handle & 0xF)

/* Powercap OCC interface */
int occ_get_powercap(u32 handle, u32 *pcap);
int occ_set_powercap(u32 handle, int token, u32 pcap);

#endif /* __POWERCAP_H */
