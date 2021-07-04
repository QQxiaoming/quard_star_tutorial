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

#include "trustedbootUtils.H"
#include "trustedboot.H"
#include <skiboot.h>
#include <stdlib.h>

errlHndl_t tpmTransmit(TpmTarget * io_target, uint8_t* io_buffer,
		       size_t i_cmdSize, size_t i_bufsize )
{
	errlHndl_t err = 0;
	err = io_target->driver->transmit(io_target->dev,
                                          io_buffer,
                                          i_cmdSize,
                                          &i_bufsize);
	return err;
}

errlHndl_t tpmCreateErrorLog(const uint8_t i_modId, const uint16_t i_reasonCode,
			     const uint64_t i_user1, const uint64_t i_user2)
{
	prlog(PR_ERR,"TSS: Error Log %d %d %d %d\n",
	      i_modId, i_reasonCode, (int)i_user1, (int)i_user2);
	return (i_modId << 16) | i_reasonCode;
}

void tpmMarkFailed(TpmTarget *io_target)
{
	prlog(PR_ERR, "TSS: %s called for %d\n", __func__, io_target->id);
}
