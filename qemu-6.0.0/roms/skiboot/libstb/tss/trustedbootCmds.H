/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/secureboot/trusted/trustedbootCmds.H $                */
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
/**
 * @file trustedbootCmds.H
 *
 * @brief Trustedboot TPM command interfaces
 */

/////////////////////////////////////////////////////////////////
// NOTE: This file is exportable as TSS-Lite for skiboot/PHYP  //
/////////////////////////////////////////////////////////////////

#ifndef __TRUSTEDBOOTCMDS_H
#define __TRUSTEDBOOTCMDS_H
// -----------------------------------------------
// Includes
// -----------------------------------------------
#ifdef __HOSTBOOT_MODULE
#include <secureboot/trustedbootif.H>
#else
#include "trustedbootUtils.H"
#include "../tpm_chip.h"
#endif
#include "trustedboot.H"
#include "trustedTypes.H"

#ifdef __cplusplus
namespace TRUSTEDBOOT
{
#endif

enum
{
    BUFSIZE         = 256,
    MAX_TRANSMIT_SIZE = 1024, ///< Maximum send/receive transmit size
};

/**
 * @brief Transmit the command to the TPM and perform marshaling
 * @param[in/out] io_target Current TPM target structure
 * @param[in/out] io_buffer Input the command buffer to send, response on exit
 * @param[in] i_bufsize Size of io_buffer in bytes
 * @return errlHndl_t NULL if successful, otherwise a pointer to the
 *       error log.
 */
errlHndl_t tpmTransmitCommand(TpmTarget* io_target,
                              uint8_t* io_buffer,
                              size_t i_bufsize );

/**
 * @brief Take structure pointed to by cmd and format for input into TPM
 * @param[in] i_cmd Prefilled command input structure
 * @param[out] o_outbuf Buffer to place marshalled data
 * @param[in] i_bufsize Size of o_outbuf in bytes
 * @param[out] o_cmdSize Byte size of io_outbuf data after marshal
 * @return errlHndl_t NULL if successful, otherwise a pointer to the
 *       error log.
 */
errlHndl_t tpmMarshalCommandData(TPM2_BaseIn* i_cmd,
                                 uint8_t* o_outbuf,
                                 size_t i_bufsize,
                                 size_t* o_cmdSize);

/**
 * @brief Take structure pointed to by cmd and format for input into TPM
 * @param[in] i_commandCode Command code that was executed on the TPM
 * @param[in] i_respBuf Buffer with response data from TPM
 * @param[in] i_respBufSize Byte size of respBuf buffer from TPM
 * @param[out] o_outBuf Buffer to place formatted response data
 * @param[in] i_outBufSize Byte size of o_outBuf buffer
 * @return errlHndl_t NULL if successful, otherwise a pointer to the
 *       error log.
 */
errlHndl_t tpmUnmarshalResponseData(uint32_t i_commandCode,
                                    uint8_t* i_respBuf,
                                    size_t i_respBufSize,
                                    TPM2_BaseOut* o_outBuf,
                                    size_t i_outBufSize);
/**
 * @brief Send the TPM_STARTUP command to the targetted TPM
 * @param[in/out] io_target Current TPM target structure
 * @return errlHndl_t NULL if successful, otherwise a pointer to the
 *       error log.
*/
errlHndl_t tpmCmdStartup(TpmTarget* io_target);

/**
 * @brief Send the TPM_GETCAPABILITY command to read FW version from TPM
 * @param[in/out] io_target Current TPM target structure
 * @return errlHndl_t NULL if successful, otherwise a pointer to the
 *       error log.
*/

errlHndl_t tpmCmdGetCapFwVersion(TpmTarget* io_target);

/**
 * @brief Send the TPM_Extend command to the targeted TPM and log
 * @param[in/out] io_target Current TPM target structure
 * @param[in] i_pcr PCR to write to
 * @param[in] i_algId Algorithm to use
 * @param[in] i_digest Digest value to write to PCR, zeros appended as needed
 * @param[in] i_digestSize Byte size of i_digest array
 * @return errlHndl_t NULL if successful, otherwise a pointer to the
 *       error log.
 */
errlHndl_t tpmCmdPcrExtend(TpmTarget * io_target,
                           TPM_Pcr i_pcr,
                           TPM_Alg_Id i_algId,
                           const uint8_t* i_digest,
                           size_t  i_digestSize);

/**
 * @brief Send the TPM_Extend command to the targeted TPM and log
 * @param[in/out] io_target Current TPM target structure
 * @param[in] i_pcr PCR to write to
 * @param[in] i_algId_1 Algorithm to use
 * @param[in] i_digest_1 Digest value to write to PCR, zeros appended as needed
 * @param[in] i_digestSize_1 Byte size of i_digest_1 array
 * @param[in] i_algId_2 Algorithm to use
 * @param[in] i_digest_2 Digest value to write to PCR, zeros appended as needed
 *                         NULL if second digest not used
 * @param[in] i_digestSize_2 Byte size of i_digest_2 array
 * @return errlHndl_t NULL if successful, otherwise a pointer to the
 *       error log.
 */
errlHndl_t tpmCmdPcrExtend2Hash(TpmTarget * io_target,
                                TPM_Pcr i_pcr,
                                TPM_Alg_Id i_algId_1,
                                const uint8_t* i_digest_1,
                                size_t  i_digestSize_1,
                                TPM_Alg_Id i_algId_2,
                                const uint8_t* i_digest_2,
                                size_t  i_digestSize_2);

/**
 * @brief Send the TPM_Read command to the targeted TPM and log
 * @param[in/out] io_target Current TPM target structure
 * @param[in] i_pcr PCR to read from
 * @param[in] i_algId Algorithm back to read from
 * @param[out] o_digest Array to store PCR contents
 * @param[in] i_digestSize Byte size of i_digest array
 * @return errlHndl_t NULL if successful, otherwise a pointer to the
 *       error log.
 */
errlHndl_t tpmCmdPcrRead(TpmTarget* io_target,
                         TPM_Pcr i_pcr,
                         TPM_Alg_Id i_algId,
                         uint8_t* o_digest,
                         size_t  i_digestSize);

#ifdef __cplusplus
} // end TRUSTEDBOOT namespace
#endif

#endif
