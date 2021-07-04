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

#include <skiboot.h>
#include <stdlib.h>
#include <ipmi.h>
#include <opal.h>
#include <timebase.h>

static void ipmi_chassis_control_complete(struct ipmi_msg *msg)
{
	uint8_t request = msg->data[0];
	uint8_t cc = msg->cc;

	ipmi_free_msg(msg);
	if (cc == IPMI_CC_NO_ERROR)
		return;

	prlog(PR_INFO, "IPMI: Chassis control request failed. "
	      "request=0x%02x, rc=0x%02x\n", request, cc);

	if (ipmi_chassis_control(request)) {
		prlog(PR_INFO, "IPMI: Failed to resend chassis control "
		      "request [0x%02x]\n", request);
	}
}

int ipmi_chassis_control(uint8_t request)
{
	struct ipmi_msg *msg;

	if (!ipmi_present())
		return OPAL_CLOSED;

	if (request > IPMI_CHASSIS_SOFT_SHUTDOWN)
		return OPAL_PARAMETER;

	msg = ipmi_mkmsg(IPMI_DEFAULT_INTERFACE, IPMI_CHASSIS_CONTROL,
			 ipmi_chassis_control_complete, NULL,
			 &request, sizeof(request), 0);
	if (!msg)
		return OPAL_HARDWARE;
	/* Set msg->error callback function */
	msg->error = ipmi_chassis_control_complete;

	prlog(PR_INFO, "IPMI: sending chassis control request 0x%02x\n",
			request);

	return ipmi_queue_msg(msg);
}

int ipmi_set_power_state(uint8_t system, uint8_t device)
{
	struct ipmi_msg *msg;
	struct {
		uint8_t system;
		uint8_t device;
	} power_state;

	if (!ipmi_present())
		return OPAL_CLOSED;

	power_state.system = system;
	power_state.device = device;

	if (system != IPMI_PWR_NOCHANGE)
		power_state.system |= 0x80;
	if (device != IPMI_PWR_NOCHANGE)
		power_state.device |= 0x80;

	msg = ipmi_mkmsg_simple(IPMI_SET_POWER_STATE, &power_state,
				sizeof(power_state));

	if (!msg)
		return OPAL_HARDWARE;

	prlog(PR_INFO, "IPMI: setting power state: sys %02x, dev %02x\n",
			power_state.system, power_state.device);

	return ipmi_queue_msg(msg);
}
