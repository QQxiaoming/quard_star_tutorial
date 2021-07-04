/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/usr/secureboot/trustedboot_reasoncodes.H $        */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/////////////////////////////////////////////////////////////////
// NOTE: This file is exportable as TSS-Lite for skiboot/PHYP  //
/////////////////////////////////////////////////////////////////

#ifndef __TRUSTEDBOOT_REASONCODES_H
#define __TRUSTEDBOOT_REASONCODES_H

#ifdef __HOSTBOOT_MODULE
#include <hbotcompid.H>
#else
#define SECURE_COMP_ID 0x1E00
#endif

#ifdef __cplusplus
namespace TRUSTEDBOOT
{
#endif

    enum TRUSTEDModuleId
    {
        MOD_HOST_UPDATE_MASTER_TPM      = 0x00,
        MOD_TPM_INITIALIZE              = 0x01,
        MOD_TPM_CMD_STARTUP             = 0x02,
        MOD_TPM_CMD_GETCAPFWVERSION     = 0x03,
        MOD_TPM_MARSHALCMDDATA          = 0x04,
        MOD_TPM_UNMARSHALRESPDATA       = 0x05,
        MOD_TPM_VERIFYFUNCTIONAL        = 0x06,
        MOD_TPM_CMD_PCREXTEND           = 0x07,
        MOD_TPM_CMD_PCRREAD             = 0x08,
        MOD_TPM_REPLAY_LOG              = 0x09,
        MOD_TPM_PCREXTEND               = 0x0A,
        MOD_TPM_TPMDAEMON               = 0x0B,
        MOD_TPM_SYNCRESPONSE            = 0x0C,

        MOD_TPMLOGMGR_INITIALIZE        = 0x10,
        MOD_TPMLOGMGR_ADDEVENT          = 0x11,
        MOD_TPMLOGMGR_INITIALIZEEXISTLOG = 0x012,
        MOD_TPMLOGMGR_GETDEVTREEINFO    = 0x13,
   };

    enum TRUSTEDReasonCode
    {
        // Reason codes 0x00 - 0x9F reserved for secure_reasoncodes.H

        RC_TPM_START_FAIL               = SECURE_COMP_ID | 0xA0,
        RC_TPM_EXISTENCE_FAIL           = SECURE_COMP_ID | 0xA1,
        RC_TPM_GETCAP_FAIL              = SECURE_COMP_ID | 0xA2,
        RC_TPM_GETCAP_FW_INVALID_RESP   = SECURE_COMP_ID | 0xA3,
        RC_TPM_GETCAP2_FAIL             = SECURE_COMP_ID | 0xA4,
        RC_TPM_GETCAP2_FW_INVALID_RESP  = SECURE_COMP_ID | 0xA5,
        RC_TPM_MARSHAL_INVALID_CMD      = SECURE_COMP_ID | 0xA6,
        RC_TPM_MARSHALING_FAIL          = SECURE_COMP_ID | 0xA7,
        RC_TPM_UNMARSHAL_INVALID_CMD    = SECURE_COMP_ID | 0xA8,
        RC_TPM_UNMARSHALING_FAIL        = SECURE_COMP_ID | 0xA9,
        RC_TPMLOGMGR_ADDEVENT_FAIL      = SECURE_COMP_ID | 0xAA,
        RC_TPMLOGMGR_ADDEVENTMARSH_FAIL = SECURE_COMP_ID | 0xAB,
        RC_TPMLOGMGR_INIT_FAIL          = SECURE_COMP_ID | 0xAC,
        RC_TPM_NOFUNCTIONALTPM_FAIL     = SECURE_COMP_ID | 0xAD,
        RC_TPM_COMMAND_FAIL             = SECURE_COMP_ID | 0xAE,
        RC_TPM_INVALID_ARGS             = SECURE_COMP_ID | 0xAF,
        RC_TPMLOGMGR_LOGWALKFAIL        = SECURE_COMP_ID | 0xB0,
        RC_PCREXTEND_SENDRECV_FAIL      = SECURE_COMP_ID | 0xB1,
        RC_PCREXTEND_SEND_FAIL          = SECURE_COMP_ID | 0xB2,
        RC_MSGRESPOND_FAIL              = SECURE_COMP_ID | 0xB3,
    };
#ifdef __cplusplus
}
#endif

#endif
