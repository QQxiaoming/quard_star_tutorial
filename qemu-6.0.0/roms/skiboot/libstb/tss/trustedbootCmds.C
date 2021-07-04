/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/secureboot/trusted/trustedbootCmds.C $                */
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
 * @file trustedbootCmds.C
 *
 * @brief Trusted boot TPM command interfaces
 */

/////////////////////////////////////////////////////////////////
// NOTE: This file is exportable as TSS-Lite for skiboot/PHYP  //
/////////////////////////////////////////////////////////////////

// ----------------------------------------------
// Includes
// ----------------------------------------------
#include <string.h>
#include <stdlib.h>
#ifdef __HOSTBOOT_MODULE
#include <secureboot/trustedboot_reasoncodes.H>
#else
#include "trustedboot_reasoncodes.H"
#endif
#include "trustedbootCmds.H"
#include "trustedbootUtils.H"
#include "trustedboot.H"
#include "trustedTypes.H"

#ifdef __cplusplus
namespace TRUSTEDBOOT
{
#endif

errlHndl_t tpmTransmitCommand(TpmTarget * io_target,
                              uint8_t* io_buffer,
                              size_t i_bufsize )
{
    errlHndl_t err = TB_SUCCESS;
    uint8_t* transmitBuf = NULL;
    size_t cmdSize = 0;
    size_t dataSize = 0;
    TPM2_BaseIn* cmd = (TPM2_BaseIn*)io_buffer;
    TPM2_BaseOut* resp = (TPM2_BaseOut*)io_buffer;

    TRACUCOMP( g_trac_trustedboot,
               ">>TPM TRANSMIT CMD START : BufLen %d : %016llx",
               (int)i_bufsize,
               *((uint64_t*)io_buffer)  );

    do
    {
        transmitBuf = (uint8_t*)malloc(MAX_TRANSMIT_SIZE);

        // Marshal the data into a byte array for transfer to the TPM
        err = tpmMarshalCommandData(cmd,
                                    transmitBuf,
                                    MAX_TRANSMIT_SIZE,
                                    &cmdSize);
        if (TB_SUCCESS != err)
        {
            break;
        }

        // Send to the TPM
        dataSize = MAX_TRANSMIT_SIZE;
        err = tpmTransmit(io_target,
                          transmitBuf,
                          cmdSize,
                          dataSize);

        if (TB_SUCCESS != err)
        {
            break;
        }

        // Unmarshal the response
        err = tpmUnmarshalResponseData(cmd->commandCode,
                                       transmitBuf,
                                       dataSize,
                                       resp,
                                       i_bufsize);


    } while ( 0 );


    free(transmitBuf);

    TRACUCOMP( g_trac_trustedboot,
               "<<tpmTransmitCommand() - %s",
               ((TB_SUCCESS == err) ? "No Error" : "With Error") );
    return err;
}

errlHndl_t tpmMarshalCommandData(TPM2_BaseIn* i_cmd,
                                 uint8_t* o_outbuf,
                                 size_t i_bufsize,
                                 size_t* o_cmdSize)
{
    errlHndl_t err = TB_SUCCESS;
    uint8_t* sBuf = o_outbuf;
    uint32_t* sSizePtr = NULL;
    size_t curSize = 0;
    int stage = 0;
    TPM2_BaseIn* baseCmd =
        (TPM2_BaseIn*)o_outbuf;
    TPMS_AUTH_COMMAND cmdAuth;

    *o_cmdSize = 0;

    TRACDCOMP( g_trac_trustedboot,
               ">>tpmMarshalCommandData()" );
    do
    {

        TRACUCOMP( g_trac_trustedboot,
                   "TPM MARSHAL START : BufLen %d : %016llx",
                   (int)i_bufsize,
                   *((uint64_t*)i_cmd)  );

        // Start with the command header
        sBuf = TPM2_BaseIn_marshal(i_cmd, sBuf, i_bufsize, o_cmdSize);
        if (NULL == sBuf)
        {
            break;
        }

        // Marshal the handles
        stage = 1;
        if (TPM_CC_PCR_Extend == i_cmd->commandCode)
        {
            TPM2_ExtendIn* cmdPtr = (TPM2_ExtendIn*)i_cmd;
            sBuf = TPM2_ExtendIn_marshalHandle(cmdPtr,
                                               sBuf,
                                               i_bufsize,
                                               o_cmdSize);
            if (NULL == sBuf)
            {
                break;
            }
        }

        // Marshal the authorizations
        stage = 2;
        if (TPM_CC_PCR_Extend == i_cmd->commandCode)
        {
            // Insert a password authorization with a null pw
            // Make room for the 4 byte size field at the beginning
            sSizePtr = (uint32_t*)sBuf;
            sBuf += sizeof(uint32_t);
            *o_cmdSize += sizeof(uint32_t);
            i_bufsize -= sizeof(uint32_t);
            curSize = *o_cmdSize;

            cmdAuth.sessionHandle = TPM_RS_PW;
            cmdAuth.nonceSize = 0;
            cmdAuth.sessionAttributes = 0;
            cmdAuth.hmacSize = 0;

            sBuf = TPMS_AUTH_COMMAND_marshal(&cmdAuth, sBuf, i_bufsize,
                                             o_cmdSize);

            if (NULL == sBuf)
            {
                break;
            }
            // Put in the size of the auth area
            *sSizePtr = (*o_cmdSize - curSize);

        }

        // Marshal the command parameters
        stage = 3;
        switch (i_cmd->commandCode)
        {
          // Two byte parm fields
          case TPM_CC_Startup:
              {
                  TPM2_2ByteIn* cmdPtr =
                      (TPM2_2ByteIn*)i_cmd;
                  sBuf = TPM2_2ByteIn_marshal(cmdPtr, sBuf,
                                              i_bufsize, o_cmdSize);
              }
              break;

          case TPM_CC_GetCapability:
              {
                  TPM2_GetCapabilityIn* cmdPtr =
                      (TPM2_GetCapabilityIn*)i_cmd;
                  sBuf = TPM2_GetCapabilityIn_marshal(cmdPtr,sBuf,
                                                      i_bufsize, o_cmdSize);
              }
              break;
          case TPM_CC_PCR_Read:
              {
                  TPM2_PcrReadIn* cmdPtr = (TPM2_PcrReadIn*)i_cmd;
                  sBuf = TPM2_PcrReadIn_marshal(cmdPtr, sBuf,
                                                i_bufsize - (sBuf - o_outbuf),
                                                o_cmdSize);
              }
              break;

          case TPM_CC_PCR_Extend:
              {
                  TPM2_ExtendIn* cmdPtr = (TPM2_ExtendIn*)i_cmd;
                  sBuf = TPM2_ExtendIn_marshalParms(cmdPtr, sBuf,
                                                    i_bufsize, o_cmdSize);
              }
              break;

          default:
              {
                  // Command code not supported
                  TRACFCOMP( g_trac_trustedboot,
                             "TPM MARSHAL INVALID COMMAND : %X",
                             i_cmd->commandCode );
                  sBuf = NULL;
                  /*@
                   * @errortype
                   * @reasoncode     RC_TPM_MARSHAL_INVALID_CMD
                   * @severity       ERRL_SEV_UNRECOVERABLE
                   * @moduleid       MOD_TPM_MARSHALCMDDATA
                   * @userdata1      Command Code
                   * @userdata2      0
                   * @devdesc        Unsupported command code during marshal
                   */
                  err = tpmCreateErrorLog(MOD_TPM_MARSHALCMDDATA,
                                          RC_TPM_MARSHAL_INVALID_CMD,
                                          i_cmd->commandCode,
                                          0);
              }
              break;
        };

        if (TB_SUCCESS != err || NULL == sBuf)
        {
            break;
        }

        // Do a verification that the cmdSize equals what we used
        if (((size_t)(sBuf - o_outbuf)) != *o_cmdSize)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM MARSHAL MARSHAL SIZE MISMATCH : %d %d",
                       (int)(sBuf - o_outbuf), (int)(*o_cmdSize) );
            sBuf = NULL;
        }

        // Lastly now that we know the size update the byte stream
        baseCmd->commandSize = *o_cmdSize;

    } while ( 0 );

    if (NULL == sBuf && TB_SUCCESS == err)
    {
        TRACFCOMP( g_trac_trustedboot,
                   "TPM MARSHAL FAILURE : Stage %d", stage);
        /*@
         * @errortype
         * @reasoncode     RC_TPM_MARSHALING_FAIL
         * @severity       ERRL_SEV_UNRECOVERABLE
         * @moduleid       MOD_TPM_MARSHALCMDDATA
         * @userdata1      stage
         * @userdata2      0
         * @devdesc        Marshaling error detected
         */
        err = tpmCreateErrorLog(MOD_TPM_MARSHALCMDDATA,
                                RC_TPM_MARSHALING_FAIL,
                                stage,
                                0 );

    }

    TRACUBIN(g_trac_trustedboot, "Marshal Out",
             o_outbuf, *o_cmdSize);

    TRACUCOMP( g_trac_trustedboot,
               "TPM MARSHAL END   : CmdSize: %d : %016llx ",
               (int)(*o_cmdSize),
               *((uint64_t*)o_outbuf)  );

    TRACDCOMP( g_trac_trustedboot,
               "<<tpmMarshalCommandData()" );

    return err;
}

errlHndl_t tpmUnmarshalResponseData(uint32_t i_commandCode,
                                    uint8_t* i_respBuf,
                                    size_t i_respBufSize,
                                    TPM2_BaseOut* o_outBuf,
                                    size_t i_outBufSize)
{
    errlHndl_t err = TB_SUCCESS;
    const uint8_t* sBuf = i_respBuf;
    int stage = 0;

    TRACDCOMP( g_trac_trustedboot,
               ">>tpmUnmarshalResponseData()" );

    do {

        TRACUCOMP( g_trac_trustedboot,
                   "TPM UNMARSHAL START : RespBufLen %d : OutBufLen %d",
                   (int)i_respBufSize, (int)i_outBufSize);
        TRACUBIN(g_trac_trustedboot,"Unmarshal In",
                 i_respBuf, i_respBufSize);


        // Start with the response header
        stage = 1;
        sBuf = TPM2_BaseOut_unmarshal(o_outBuf, sBuf,
                                      &i_respBufSize, i_outBufSize);
        if (NULL == sBuf)
        {
            break;
        }

        // If the TPM returned a failure it will not send the rest
        // Let the caller deal with the RC
        if (TPM_SUCCESS != o_outBuf->responseCode)
        {
            break;
        }


        // Unmarshal the parameters
        stage = 2;
        switch (i_commandCode)
        {
          // Empty response commands
          case TPM_CC_Startup:
          case TPM_CC_PCR_Extend:
            // Nothing to do
            break;

          case TPM_CC_GetCapability:
              {
                  TPM2_GetCapabilityOut* respPtr =
                      (TPM2_GetCapabilityOut*)o_outBuf;
                  sBuf = TPM2_GetCapabilityOut_unmarshal(respPtr, sBuf,
                                                         &i_respBufSize,
                                                         i_outBufSize);

              }
              break;

          case TPM_CC_PCR_Read:
              {
                  TPM2_PcrReadOut* respPtr = (TPM2_PcrReadOut*)o_outBuf;
                  sBuf = TPM2_PcrReadOut_unmarshal(respPtr, sBuf,
                                                   &i_respBufSize,
                                                   i_outBufSize);
              }
              break;

          default:
              {
                  // Command code not supported
                  TRACFCOMP( g_trac_trustedboot,
                             "TPM UNMARSHAL INVALID COMMAND : %X",
                             i_commandCode );
                  sBuf = NULL;

                  /*@
                   * @errortype
                   * @reasoncode     RC_TPM_UNMARSHAL_INVALID_CMD
                   * @severity       ERRL_SEV_UNRECOVERABLE
                   * @moduleid       MOD_TPM_UNMARSHALRESPDATA
                   * @userdata1      commandcode
                   * @userdata2      stage
                   * @devdesc        Unsupported command code during unmarshal
                   */
                  err = tpmCreateErrorLog(MOD_TPM_UNMARSHALRESPDATA,
                                          RC_TPM_UNMARSHAL_INVALID_CMD,
                                          i_commandCode,
                                          stage);
              }
              break;
        }


    } while ( 0 );

    if (NULL == sBuf && TB_SUCCESS == err)
    {
        TRACFCOMP( g_trac_trustedboot,
                   "TPM UNMARSHAL FAILURE : Stage %d", stage);
        /*@
         * @errortype
         * @reasoncode     RC_TPM_UNMARSHALING_FAIL
         * @severity       ERRL_SEV_UNRECOVERABLE
         * @moduleid       MOD_TPM_UNMARSHALRESPDATA
         * @userdata1      Stage
         * @userdata2      Remaining response buffer size
         * @devdesc        Unmarshaling error detected
         */
        err = tpmCreateErrorLog(MOD_TPM_UNMARSHALRESPDATA,
                                RC_TPM_UNMARSHALING_FAIL,
                                stage,
                                i_respBufSize);



    }

    TRACUCOMP( g_trac_trustedboot,
               "TPM UNMARSHAL END   : %016llx ",
               *((uint64_t*)o_outBuf)  );

    TRACDCOMP( g_trac_trustedboot,
               "<<tpmUnmarshalResponseData()" );

    return err;
}

errlHndl_t tpmCmdStartup(TpmTarget* io_target)
{
    errlHndl_t err = TB_SUCCESS;
    uint8_t dataBuf[BUFSIZE];

    TPM2_BaseOut* resp =
        (TPM2_BaseOut*)(dataBuf);

    TPM2_2ByteIn* cmd =
        (TPM2_2ByteIn*)(dataBuf);

    TRACUCOMP( g_trac_trustedboot,
               ">>tpmCmdStartup()" );

    do
    {
        // Send the TPM startup command
        // Build our command block for a startup
        memset(dataBuf, 0, sizeof(dataBuf));


        cmd->base.tag = TPM_ST_NO_SESSIONS;
        cmd->base.commandCode = TPM_CC_Startup;
        cmd->param = TPM_SU_CLEAR;

        err = tpmTransmitCommand(io_target,
                                 dataBuf,
                                 sizeof(dataBuf));

        if (TB_SUCCESS != err)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM STARTUP transmit Fail");
            break;

        }
        else if (TPM_SUCCESS != resp->responseCode)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM STARTUP OP Fail %X : ",
                       resp->responseCode);

            /*@
             * @errortype
             * @reasoncode     RC_TPM_START_FAIL
             * @severity       ERRL_SEV_UNRECOVERABLE
             * @moduleid       MOD_TPM_CMD_STARTUP
             * @userdata1      responseCode
             * @userdata2      0
             * @devdesc        Invalid operation type.
             */
            err = tpmCreateErrorLog(MOD_TPM_CMD_STARTUP,
                                    RC_TPM_START_FAIL,
                                    resp->responseCode,
                                    0);

            break;
        }


    } while ( 0 );


    TRACUCOMP( g_trac_trustedboot,
               "<<tpmCmdStartup() - %s",
               ((TB_SUCCESS == err) ? "No Error" : "With Error") );
    return err;
}

errlHndl_t tpmCmdGetCapFwVersion(TpmTarget* io_target)
{
    errlHndl_t err = TB_SUCCESS;
    uint8_t dataBuf[BUFSIZE];
    size_t dataSize = BUFSIZE;
    uint16_t fwVersion[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    TPM2_GetCapabilityOut* resp =
        (TPM2_GetCapabilityOut*)dataBuf;
    TPM2_GetCapabilityIn* cmd =
        (TPM2_GetCapabilityIn*)dataBuf;


    TRACUCOMP( g_trac_trustedboot,
               ">>tpmCmdGetCapFwVersion()" );

    do
    {

        // Build our command block for a get capability of the FW version
        memset(dataBuf, 0, dataSize);

        cmd->base.tag = TPM_ST_NO_SESSIONS;
        cmd->base.commandCode = TPM_CC_GetCapability;
        cmd->capability = TPM_CAP_TPM_PROPERTIES;
        cmd->property = TPM_PT_FIRMWARE_VERSION_1;
        cmd->propertyCount = 1;

        err = tpmTransmitCommand(io_target,
                                 dataBuf,
                                 sizeof(dataBuf));

        if (TB_SUCCESS != err)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM GETCAP Transmit Fail");
            break;

        }

        if (TPM_SUCCESS != resp->base.responseCode)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM GETCAP OP Fail %X Size(%d) ",
                       resp->base.responseCode,
                       (int)dataSize);

            /*@
             * @errortype
             * @reasoncode     RC_TPM_GETCAP_FAIL
             * @severity       ERRL_SEV_UNRECOVERABLE
             * @moduleid       MOD_TPM_CMD_GETCAPFWVERSION
             * @userdata1      responseCode
             * @userdata2      0
             * @devdesc        Command failure reading TPM FW version.
             */
            err = tpmCreateErrorLog(MOD_TPM_CMD_GETCAPFWVERSION,
                                    RC_TPM_GETCAP_FAIL,
                                    resp->base.responseCode,
                                    0);

            break;
        }
        else
        {
            // Walk the reponse data to pull the high order bytes out

            if (resp->capData.capability != TPM_CAP_TPM_PROPERTIES ||
                resp->capData.data.tpmProperties.count != 1 ||
                resp->capData.data.tpmProperties.tpmProperty[0].property !=
                TPM_PT_FIRMWARE_VERSION_1) {

                TRACFCOMP( g_trac_trustedboot,
                           "TPM GETCAP FW INVALID DATA "
                           "Cap(%X) Cnt(%X) Prop(%X)",
                           resp->capData.capability,
                           resp->capData.data.tpmProperties.count,
                           resp->capData.data.tpmProperties.
                           tpmProperty[0].property);

                /*@
                 * @errortype
                 * @reasoncode     RC_TPM_GETCAP_FW_INVALID_RESP
                 * @severity       ERRL_SEV_UNRECOVERABLE
                 * @moduleid       MOD_TPM_CMD_GETCAPFWVERSION
                 * @userdata1      capability
                 * @userdata2      property
                 * @devdesc        Command failure reading TPM FW version.
                 */
                err = tpmCreateErrorLog(MOD_TPM_CMD_GETCAPFWVERSION,
                                        RC_TPM_GETCAP_FW_INVALID_RESP,
                                        resp->capData.capability,
                                        resp->capData.data.tpmProperties.
                                        tpmProperty[0].property);

                break;
            }
            else
            {
                fwVersion[0] =
                    (resp->capData.data.
                     tpmProperties.tpmProperty[0].value >> 16);
                fwVersion[1] =
                    (resp->capData.data.
                     tpmProperties.tpmProperty[0].value & 0xFFFF);
            }

        }

        // Read part 2 of the version
        dataSize = BUFSIZE;
        memset(dataBuf, 0, dataSize);

        cmd->base.tag = TPM_ST_NO_SESSIONS;
        cmd->base.commandCode = TPM_CC_GetCapability;
        cmd->capability = TPM_CAP_TPM_PROPERTIES;
        cmd->property = TPM_PT_FIRMWARE_VERSION_2;
        cmd->propertyCount = 1;


        err = tpmTransmitCommand(io_target,
                                 dataBuf,
                                 sizeof(dataBuf));

        if (TB_SUCCESS != err)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM GETCAP2 Transmit Fail");
            break;

        }

        if ((sizeof(TPM2_GetCapabilityOut) > dataSize) ||
            (TPM_SUCCESS != resp->base.responseCode))
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM GETCAP2 OP Fail %X Size(%d) ",
                       resp->base.responseCode,
                       (int)dataSize);

            /*@
             * @errortype
             * @reasoncode     RC_TPM_GETCAP2_FAIL
             * @severity       ERRL_SEV_UNRECOVERABLE
             * @moduleid       MOD_TPM_CMD_GETCAPFWVERSION
             * @userdata1      responseCode
             * @userdata2      0
             * @devdesc        Command failure reading TPM FW version.
             */
            err = tpmCreateErrorLog(MOD_TPM_CMD_GETCAPFWVERSION,
                                    RC_TPM_GETCAP2_FAIL,
                                    resp->base.responseCode,
                                    0);

            break;
        }
        else
        {
            // Walk the reponse data to pull the high order bytes out

            if (resp->capData.capability != TPM_CAP_TPM_PROPERTIES ||
                resp->capData.data.tpmProperties.count != 1 ||
                resp->capData.data.tpmProperties.tpmProperty[0].property !=
                TPM_PT_FIRMWARE_VERSION_2) {

                TRACFCOMP( g_trac_trustedboot,
                           "TPM GETCAP2 FW INVALID DATA "
                           "Cap(%X) Cnt(%X) Prop(%X)",
                           resp->capData.capability,
                           resp->capData.data.tpmProperties.count,
                           resp->capData.data.tpmProperties.
                             tpmProperty[0].property);

                /*@
                 * @errortype
                 * @reasoncode     RC_TPM_GETCAP2_FW_INVALID_RESP
                 * @severity       ERRL_SEV_UNRECOVERABLE
                 * @moduleid       MOD_TPM_CMD_GETCAPFWVERSION
                 * @userdata1      capability
                 * @userdata2      property
                 * @devdesc        Command failure reading TPM FW version.
                 */
                err = tpmCreateErrorLog(MOD_TPM_CMD_GETCAPFWVERSION,
                                        RC_TPM_GETCAP2_FW_INVALID_RESP,
                                        resp->capData.capability,
                                        resp->capData.data.tpmProperties.
                                        tpmProperty[0].property);
                break;
            }
            else
            {
                fwVersion[2] =
                    (resp->capData.data.tpmProperties.
                     tpmProperty[0].value >> 16);
                fwVersion[3] =
                    (resp->capData.data.tpmProperties.
                     tpmProperty[0].value & 0xFFFF);
            }
            // Trace the response
            TRACFCOMP( g_trac_trustedboot,
                       "TPM GETCAP FW Level %d.%d.%d.%d",
                       fwVersion[0],fwVersion[1],fwVersion[2],fwVersion[3]
                       );
        }


    } while ( 0 );


    TRACDCOMP( g_trac_trustedboot,
               "<<tpmCmdGetCapFwVersion() - %s",
               ((TB_SUCCESS == err) ? "No Error" : "With Error") );
    return err;
}


errlHndl_t tpmCmdPcrExtend(TpmTarget * io_target,
                           TPM_Pcr i_pcr,
                           TPM_Alg_Id i_algId,
                           const uint8_t* i_digest,
                           size_t  i_digestSize)
{
    return tpmCmdPcrExtend2Hash(io_target, i_pcr,
                                i_algId, i_digest, i_digestSize,
                                TPM_ALG_INVALID_ID, NULL, 0);
}

errlHndl_t tpmCmdPcrExtend2Hash(TpmTarget * io_target,
                                TPM_Pcr i_pcr,
                                TPM_Alg_Id i_algId_1,
                                const uint8_t* i_digest_1,
                                size_t  i_digestSize_1,
                                TPM_Alg_Id i_algId_2,
                                const uint8_t* i_digest_2,
                                size_t  i_digestSize_2)
{
    errlHndl_t err = 0;
    uint8_t dataBuf[sizeof(TPM2_ExtendIn)];
    size_t dataSize = sizeof(dataBuf);
    size_t fullDigestSize_1 = 0;
    size_t fullDigestSize_2 = 0;
    TPM2_BaseOut* resp = (TPM2_BaseOut*)dataBuf;
    TPM2_ExtendIn* cmd = (TPM2_ExtendIn*)dataBuf;


    TRACDCOMP( g_trac_trustedboot,
               ">>tpmCmdPcrExtend2Hash()" );
    if (NULL == i_digest_2)
    {
        TRACUCOMP( g_trac_trustedboot,
                   ">>tpmCmdPcrExtend2Hash() Pcr(%d) Alg(%X) DS(%d)",
                   i_pcr, i_algId_1, (int)i_digestSize_1);
    }
    else
    {
        TRACUCOMP( g_trac_trustedboot,
                   ">>tpmCmdPcrExtend2Hash() Pcr(%d) Alg(%X:%X) DS(%d:%d)",
                   i_pcr, i_algId_1, i_algId_2,
                   (int)i_digestSize_1, (int)i_digestSize_2);
    }

    do
    {

        fullDigestSize_1 = getDigestSize(i_algId_1);
        if (NULL != i_digest_2)
        {
            fullDigestSize_2 = getDigestSize(i_algId_2);
        }

        // Build our command block
        memset(dataBuf, 0, sizeof(dataBuf));

        // Argument verification
        if (fullDigestSize_1 == 0 ||
            NULL == i_digest_1 ||
            IMPLEMENTATION_PCR < i_pcr ||
            (NULL != i_digest_2 && fullDigestSize_2 == 0)
            )
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM PCR EXTEND ARG FAILURE FDS(%d:%d) DS(%d:%d) "
                       "PCR(%d)",
                       (int)fullDigestSize_1, (int)fullDigestSize_2,
                       (int)i_digestSize_1, (int)i_digestSize_2, i_pcr);
            /*@
             * @errortype
             * @reasoncode     RC_TPM_INVALID_ARGS
             * @severity       ERRL_SEV_UNRECOVERABLE
             * @moduleid       MOD_TPM_CMD_PCREXTEND
             * @userdata1      Digest Ptr
             * @userdata2[0:15] Full Digest Size 1
             * @userdata2[16:31] Full Digest Size 2
             * @userdata2[32:63] PCR
             * @devdesc        Unmarshaling error detected
             */
            err = tpmCreateErrorLog(MOD_TPM_CMD_PCREXTEND,
                                    RC_TPM_INVALID_ARGS,
                                    (uint64_t)i_digest_1,
                                    (fullDigestSize_1 << 48) |
                                    (fullDigestSize_2 << 32) |
                                    i_pcr);
            break;
        }

        // Log the input PCR value
        TRACUBIN(g_trac_trustedboot, "PCR In",
                 i_digest_1, fullDigestSize_1);

        cmd->base.tag = TPM_ST_SESSIONS;
        cmd->base.commandCode = TPM_CC_PCR_Extend;
        cmd->pcrHandle = i_pcr;
        cmd->digests.count = 1;
        cmd->digests.digests[0].algorithmId = i_algId_1;
        memcpy(&(cmd->digests.digests[0].digest), i_digest_1,
               (i_digestSize_1 < fullDigestSize_1 ?
                i_digestSize_1 : fullDigestSize_1) );
        if (NULL != i_digest_2)
        {
            cmd->digests.count = 2;
            cmd->digests.digests[1].algorithmId = i_algId_2;
            memcpy(&(cmd->digests.digests[1].digest), i_digest_2,
                   (i_digestSize_2 < fullDigestSize_2 ?
                    i_digestSize_2 : fullDigestSize_2));
        }

        err = tpmTransmitCommand(io_target,
                                 dataBuf,
                                 sizeof(dataBuf));

        if (TB_SUCCESS != err)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM PCRExtend Transmit Fail");
            break;

        }
        else if ((sizeof(TPM2_BaseOut) > dataSize)
                 || (TPM_SUCCESS != resp->responseCode))
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM PCRExtend OP Fail Ret(%X) ExSize(%d) Size(%d) ",
                       resp->responseCode,
                       (int)sizeof(TPM2_BaseOut),
                       (int)dataSize);

            /*@
             * @errortype
             * @reasoncode     RC_TPM_COMMAND_FAIL
             * @severity       ERRL_SEV_UNRECOVERABLE
             * @moduleid       MOD_TPM_CMD_PCREXTEND
             * @userdata1      responseCode
             * @userdata2      dataSize
             * @devdesc        Command failure reading TPM FW version.
             */
            err = tpmCreateErrorLog(MOD_TPM_CMD_PCREXTEND,
                                    RC_TPM_COMMAND_FAIL,
                                    resp->responseCode,
                                    dataSize);
            break;

        }

    } while ( 0 );


    TRACUCOMP( g_trac_trustedboot,
               "<<tpmCmdPcrExtend() - %s",
               ((TB_SUCCESS == err) ? "No Error" : "With Error") );
    return err;

}

errlHndl_t tpmCmdPcrRead(TpmTarget* io_target,
                         TPM_Pcr i_pcr,
                         TPM_Alg_Id i_algId,
                         uint8_t* o_digest,
                         size_t  i_digestSize)
{
    errlHndl_t err = 0;
    uint8_t dataBuf[sizeof(TPM2_PcrReadOut)];
    size_t dataSize = sizeof(dataBuf);
    size_t fullDigestSize = 0;
    TPM2_PcrReadOut* resp = (TPM2_PcrReadOut*)dataBuf;
    TPM2_PcrReadIn* cmd = (TPM2_PcrReadIn*)dataBuf;


    TRACDCOMP( g_trac_trustedboot,
               ">>tpmCmdPcrRead()" );
    TRACUCOMP( g_trac_trustedboot,
               ">>tpmCmdPcrRead() Pcr(%d) DS(%d)",
               i_pcr, (int)i_digestSize);

    do
    {

        fullDigestSize = getDigestSize(i_algId);

        // Build our command block
        memset(dataBuf, 0, sizeof(dataBuf));

        // Argument verification
        if (fullDigestSize > i_digestSize ||
            NULL == o_digest ||
            IMPLEMENTATION_PCR < i_pcr
            )
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM PCR READ ARG FAILURE FDS(%d) DS(%d) PCR(%d)",
                       (int)fullDigestSize, (int)i_digestSize, i_pcr);
            /*@
             * @errortype
             * @reasoncode     RC_TPM_INVALID_ARGS
             * @severity       ERRL_SEV_UNRECOVERABLE
             * @moduleid       MOD_TPM_CMD_PCRREAD
             * @userdata1      Digest Ptr
             * @userdata2[0:31] Full Digest Size
             * @userdata2[32:63] PCR
             * @devdesc        Unmarshaling error detected
             */
            err = tpmCreateErrorLog(MOD_TPM_CMD_PCRREAD,
                                    RC_TPM_INVALID_ARGS,
                                    (uint64_t)o_digest,
                                    (fullDigestSize << 32) |
                                    i_pcr);

            break;
        }

        cmd->base.tag = TPM_ST_NO_SESSIONS;
        cmd->base.commandCode = TPM_CC_PCR_Read;
        cmd->pcrSelectionIn.count = 1; // One algorithm
        cmd->pcrSelectionIn.pcrSelections[0].algorithmId = i_algId;
        cmd->pcrSelectionIn.pcrSelections[0].sizeOfSelect = PCR_SELECT_MAX;
        memset(cmd->pcrSelectionIn.pcrSelections[0].pcrSelect, 0,
               sizeof(cmd->pcrSelectionIn.pcrSelections[0].pcrSelect));
        cmd->pcrSelectionIn.pcrSelections[0].pcrSelect[i_pcr / 8] =
            0x01 << (i_pcr % 8);

        err = tpmTransmitCommand(io_target,
                                 dataBuf,
                                 sizeof(dataBuf));

        if (TB_SUCCESS != err)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM PCRRead Transmit Fail ");
            break;

        }
        else if ((sizeof(TPM2_BaseOut) > dataSize) ||
                 (TPM_SUCCESS != resp->base.responseCode) ||
                 (resp->pcrValues.count != 1) ||
                 (resp->pcrValues.digests[0].size != fullDigestSize))
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM PCRRead OP Fail Ret(%X) ExSize(%d) "
                       "Size(%d) Cnt(%d) DSize(%d)",
                       resp->base.responseCode,
                       (int)sizeof(TPM2_BaseOut),
                       (int)dataSize,
                       resp->pcrValues.count,
                       resp->pcrValues.digests[0].size);

            /*@
             * @errortype
             * @reasoncode     RC_TPM_COMMAND_FAIL
             * @severity       ERRL_SEV_UNRECOVERABLE
             * @moduleid       MOD_TPM_CMD_PCRREAD
             * @userdata1      responseCode
             * @userdata2      dataSize
             * @devdesc        Command failure reading TPM FW version.
             */
            err = tpmCreateErrorLog(MOD_TPM_CMD_PCRREAD,
                                    RC_TPM_COMMAND_FAIL,
                                    resp->base.responseCode,
                                    dataSize);
            break;
        }
        else
        {

            memcpy(o_digest, resp->pcrValues.digests[0].buffer, fullDigestSize);

            // Log the PCR value
            TRACUBIN(g_trac_trustedboot, "PCR Out",
                     o_digest, fullDigestSize);

        }

    } while ( 0 );


    TRACUCOMP( g_trac_trustedboot,
               "<<tpmCmdPcrRead() - %s",
               ((TB_SUCCESS == err) ? "No Error" : "With Error") );
    return err;

}


#ifdef __cplusplus
} // end TRUSTEDBOOT
#endif
