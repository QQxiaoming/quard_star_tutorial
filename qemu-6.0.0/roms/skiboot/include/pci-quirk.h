/* Copyright 2016 IBM Corp.
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

#ifndef __PCI_QUIRK_H
#define __PCI_QUIRK_H

#include <pci.h>

#define PCI_ANY_ID 0xFFFF

struct pci_quirk {
	void (*fixup)(struct phb *, struct pci_device *);
	uint16_t vendor_id;
	uint16_t device_id;
};

void pci_handle_quirk(struct phb *phb, struct pci_device *pd);

#endif /* __PCI_QUIRK_H */
