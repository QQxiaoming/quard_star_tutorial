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

#include <powercap.h>

static int opal_get_powercap(u32 handle, int token __unused, u32 *pcap)
{
	if (!pcap || !opal_addr_valid(pcap))
		return OPAL_PARAMETER;

	if (powercap_get_class(handle) == POWERCAP_CLASS_OCC)
		return occ_get_powercap(handle, pcap);

	return OPAL_UNSUPPORTED;
};

opal_call(OPAL_GET_POWERCAP, opal_get_powercap, 3);

static int opal_set_powercap(u32 handle, int token, u32 pcap)
{
	if (powercap_get_class(handle) == POWERCAP_CLASS_OCC)
		return occ_set_powercap(handle, token, pcap);

	return OPAL_UNSUPPORTED;
};

opal_call(OPAL_SET_POWERCAP, opal_set_powercap, 3);
