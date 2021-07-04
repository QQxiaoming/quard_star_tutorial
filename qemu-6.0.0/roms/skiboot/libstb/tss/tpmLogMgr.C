/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/secureboot/trusted/tpmLogMgr.C $                      */
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
 * @file TpmLogMgr.C
 *
 * @brief TPM Event log manager
 */

/////////////////////////////////////////////////////////////////
// NOTE: This file is exportable as TSS-Lite for skiboot/PHYP  //
/////////////////////////////////////////////////////////////////

// ----------------------------------------------
// Includes
// ----------------------------------------------
#include <string.h>
#include "tpmLogMgr.H"
#ifdef __HOSTBOOT_MODULE
#include <sys/mm.h>
#include <util/align.H>
#include <secureboot/trustedboot_reasoncodes.H>
#else
#include "trustedboot_reasoncodes.H"
#endif
#include "trustedbootUtils.H"
#include "trustedboot.H"

#ifdef __cplusplus
namespace TRUSTEDBOOT
{
#endif

    uint32_t TCG_EfiSpecIdEventStruct_size(TCG_EfiSpecIdEventStruct* val)
    {
        return (sizeof(TCG_EfiSpecIdEventStruct) + val->vendorInfoSize);
    }

#ifdef __HOSTBOOT_MODULE
    errlHndl_t TpmLogMgr_initialize(TpmLogMgr* val)
    {
        errlHndl_t err = TB_SUCCESS;
        const char vendorInfo[] = "IBM";
        const char eventSignature[] = "Spec ID Event03";
        TCG_EfiSpecIdEventStruct* eventData = NULL;

        TCG_PCR_EVENT eventLogEntry;

        TRACUCOMP( g_trac_trustedboot, ">>initialize()");

        if (NULL == val)
        {
            TRACFCOMP( g_trac_trustedboot,
                       "TPM LOG INIT FAIL");

                /*@
                 * @errortype
                 * @reasoncode     RC_TPMLOGMGR_INIT_FAIL
                 * @severity       ERRL_SEV_UNRECOVERABLE
                 * @moduleid       MOD_TPMLOGMGR_INITIALIZE
                 * @userdata1      0
                 * @userdata2      0
                 * @devdesc        TPM log buffer init failure.
                 * @custdesc       TPM log buffer init failure.
                 */
                err = tpmCreateErrorLog( MOD_TPMLOGMGR_INITIALIZE,
                                         RC_TPMLOGMGR_INIT_FAIL, 0, 0);

        }
        else
        {

            memset(val, 0, sizeof(TpmLogMgr));
            val->logMaxSize = TPMLOG_BUFFER_SIZE;

            mutex_init( &val->logMutex );
            mutex_lock( &val->logMutex );

            // Assign our new event pointer to the start
            val->newEventPtr = val->eventLog;
            memset(val->eventLog, 0, TPMLOG_BUFFER_SIZE);

            eventData = (TCG_EfiSpecIdEventStruct*) eventLogEntry.event;

            // Add the header event log
            // Values here come from the PC ClientSpecificPlatformProfile spec
            eventLogEntry.eventType = EV_NO_ACTION;
            eventLogEntry.pcrIndex = 0;
            eventLogEntry.eventSize = sizeof(TCG_EfiSpecIdEventStruct) +
                sizeof(vendorInfo);

            memcpy(eventData->signature, eventSignature,
                   sizeof(eventSignature));
            eventData->platformClass = htole32(TPM_PLATFORM_SERVER);
            eventData->specVersionMinor = TPM_SPEC_MINOR;
            eventData->specVersionMajor = TPM_SPEC_MAJOR;
            eventData->specErrata = TPM_SPEC_ERRATA;
            eventData->uintnSize = 1;
            eventData->numberOfAlgorithms = htole32(HASH_COUNT);
            eventData->digestSizes[0].algorithmId = htole16(TPM_ALG_SHA256);
            eventData->digestSizes[0].digestSize = htole16(TPM_ALG_SHA256_SIZE);
            eventData->digestSizes[1].algorithmId = htole16(TPM_ALG_SHA1);
            eventData->digestSizes[1].digestSize = htole16(TPM_ALG_SHA1_SIZE);
            eventData->vendorInfoSize = sizeof(vendorInfo);
            memcpy(eventData->vendorInfo, vendorInfo, sizeof(vendorInfo));
            val->newEventPtr = TCG_PCR_EVENT_logMarshal(&eventLogEntry,
                                                        val->newEventPtr);

            // Done, move our pointers
            val->logSize += TCG_PCR_EVENT_marshalSize(&eventLogEntry);

            mutex_unlock( &val->logMutex );

            // Debug display of raw data
            TRACUBIN(g_trac_trustedboot, "tpmInitialize: Header Entry",
                     val->eventLog, val->logSize);

            TRACUCOMP( g_trac_trustedboot,
                       "<<initialize() LS:%d - %s",
                       val->logSize,
                       ((TB_SUCCESS == err) ? "No Error" : "With Error") );
        }
        return err;
    }
#endif

    errlHndl_t TpmLogMgr_initializeUsingExistingLog(TpmLogMgr* val,
                                                    uint8_t* eventLogPtr,
                                                    uint32_t eventLogSize)
    {
        errlHndl_t err = TB_SUCCESS;
        TRACUCOMP( g_trac_trustedboot,
                   ">>initializeUsingExistingLog()");

        do
        {

            mutex_init( &val->logMutex );
            mutex_lock( &val->logMutex );

            val->logMaxSize = eventLogSize;
            val->eventLogInMem = eventLogPtr;

            // Ok, walk the log to figure out how big this is
            val->logSize = TpmLogMgr_calcLogSize(val);

            if (0 == val->logSize)
            {
                TRACFCOMP( g_trac_trustedboot,
                       "TPM LOG INIT WALK FAIL");
                /*@
                 * @errortype
                 * @reasoncode     RC_TPMLOGMGR_LOGWALKFAIL
                 * @severity       ERRL_SEV_UNRECOVERABLE
                 * @moduleid       MOD_TPMLOGMGR_INITIALIZEEXISTLOG
                 * @userdata1      0
                 * @userdata2      0
                 * @devdesc        TPM log header entry is missing.
                 * @custdesc       TPM log invalid format
                 */
                err = tpmCreateErrorLog(MOD_TPMLOGMGR_INITIALIZEEXISTLOG,
                                        RC_TPMLOGMGR_LOGWALKFAIL,
                                        0,
                                        0);
                break;
            }
            // We are good, let's move the newEventLogPtr
            val->newEventPtr = val->eventLogInMem + val->logSize;

        }
        while(0);

        if (TB_SUCCESS != err)
        {
            val->eventLogInMem = NULL;
            val->newEventPtr = NULL;
            val->logMaxSize = 0;
            val->logSize = 0;
        }

        mutex_unlock( &val->logMutex );

        return err;
    }

    errlHndl_t TpmLogMgr_addEvent(TpmLogMgr* val,
                                  TCG_PCR_EVENT2* logEvent)
    {
        errlHndl_t err = TB_SUCCESS;
        size_t newLogSize = TCG_PCR_EVENT2_marshalSize(logEvent);

        TRACUCOMP( g_trac_trustedboot,
                   ">>tpmAddEvent() PCR:%d Type:%d Size:%d",
                   logEvent->pcrIndex, logEvent->eventType, (int)newLogSize);

        mutex_lock( &val->logMutex );

        do
        {

            // Need to ensure we have room for the new event
            // We have to leave room for the log full event as well
            if (NULL == val->newEventPtr ||
                val->logSize + newLogSize > val->logMaxSize)
            {
                TRACFCOMP( g_trac_trustedboot,
                           "TPM LOG ADD FAIL PNULL(%d) LS(%d) New LS(%d)"
                           " Max LS(%d)",
                           (NULL == val->newEventPtr ? 0 : 1),
                           (int)val->logSize, (int)newLogSize,
                           (int)val->logMaxSize);

                /*@
                 * @errortype
                 * @reasoncode     RC_TPMLOGMGR_ADDEVENT_FAIL
                 * @severity       ERRL_SEV_UNRECOVERABLE
                 * @moduleid       MOD_TPMLOGMGR_ADDEVENT
                 * @userdata1[0:31]  Max log size
                 * @userdata1[32:63] Log buffer NULL
                 * @userdata2[0:31]  Current Log Size
                 * @userdata2[32:63] New entry size
                 * @devdesc        TPM log buffer add failure.
                 * @custdesc       TPM log overflow
                 */
                err = tpmCreateErrorLog( MOD_TPMLOGMGR_ADDEVENT,
                                         RC_TPMLOGMGR_ADDEVENT_FAIL,
                                         (uint64_t)val->logMaxSize << 32 |
                                         (NULL == val->newEventPtr ? 0 : 1),
                                         (uint64_t)val->logSize << 32 |
                                         newLogSize);

                break;
            }

            val->newEventPtr = TCG_PCR_EVENT2_logMarshal(logEvent,
                                                         val->newEventPtr);

            if (NULL == val->newEventPtr)
            {
                TRACFCOMP( g_trac_trustedboot,
                           "TPM LOG MARSHAL Fail");

                /*@
                 * @errortype
                 * @reasoncode     RC_TPMLOGMGR_ADDEVENTMARSH_FAIL
                 * @severity       ERRL_SEV_UNRECOVERABLE
                 * @moduleid       MOD_TPMLOGMGR_ADDEVENT
                 * @userdata1      0
                 * @userdata2      0
                 * @devdesc        log buffer marshal failure.
                 * @custdesc       TPM log operation failure
                 */
                err = tpmCreateErrorLog( MOD_TPMLOGMGR_ADDEVENT,
                                         RC_TPMLOGMGR_ADDEVENTMARSH_FAIL,
                                         0,
                                         0);
                break;
            }

            val->logSize += newLogSize;


        } while ( 0 );

        TRACUCOMP( g_trac_trustedboot,
                   "<<tpmAddEvent() LS:%d - %s",
                   (int)val->logSize,
                   ((TB_SUCCESS == err) ? "No Error" : "With Error") );

        mutex_unlock( &val->logMutex );
        return err;
    }

    uint32_t TpmLogMgr_getLogSize(TpmLogMgr* val)
    {
        return val->logSize;
    }

#ifdef __HOSTBOOT_MODULE
    void TpmLogMgr_dumpLog(TpmLogMgr* val)
    {

        // Debug display of raw data
        TRACUCOMP(g_trac_trustedboot, "tpmDumpLog Size : %d",
                  (int)val->logSize);

        // Debug display of raw data
        if (NULL == val->eventLogInMem)
        {
            TRACUBIN(g_trac_trustedboot, "tpmDumpLog",
                     val->eventLog, val->logSize);
        }
        else
        {
            TRACUBIN(g_trac_trustedboot, "tpmDumpLog From Memory",
                     val->eventLogInMem, val->logSize);
        }
    }
#endif

    uint32_t TpmLogMgr_calcLogSize(TpmLogMgr* val)
    {
        uint32_t logSize = 0;
        TCG_PCR_EVENT event;
        TCG_PCR_EVENT2 event2;
        bool errorFound = false;
        const uint8_t* prevLogHandle = NULL;
        const uint8_t* nextLogHandle = NULL;

        TRACUCOMP( g_trac_trustedboot, ">>calcLogSize");

        // Start walking events
        prevLogHandle = TpmLogMgr_getLogStartPtr(val);
        do
        {

            // First need to deal with the header entry
            nextLogHandle = TCG_PCR_EVENT_logUnmarshal(&event,
                                                       prevLogHandle,
                                                       sizeof(TCG_PCR_EVENT),
                                                       &errorFound);

            if (NULL == nextLogHandle || errorFound ||
                EV_NO_ACTION != event.eventType ||
                0 == event.eventSize)
            {
                TRACFCOMP( g_trac_trustedboot, "Header Marshal Failure");
                prevLogHandle = NULL;
                break;
            }

            if (( nextLogHandle - TpmLogMgr_getLogStartPtr(val)) >
                val->logMaxSize)
            {
                TRACFCOMP( g_trac_trustedboot, "calcLogSize overflow");
                prevLogHandle = NULL;
                break;
            }
            prevLogHandle = nextLogHandle;

            // Now iterate through all the other events
            while (NULL != prevLogHandle)
            {
                nextLogHandle = TCG_PCR_EVENT2_logUnmarshal(
                                               &event2,
                                               prevLogHandle,
                                               sizeof(TCG_PCR_EVENT2),
                                               &errorFound);
                if (NULL == nextLogHandle || errorFound)
                {
                    // Failed parsing so we must have hit the end of log
                    break;
                }
                if (( nextLogHandle - TpmLogMgr_getLogStartPtr(val)) >
                    val->logMaxSize)
                {
                    TRACFCOMP( g_trac_trustedboot, "calcLogSize overflow");
                    prevLogHandle = NULL;
                    break;
                }
                prevLogHandle = nextLogHandle;
            }
        }
        while (0);

        if (NULL == prevLogHandle)
        {
            logSize = 0;
        }
        else
        {
            logSize = (prevLogHandle - TpmLogMgr_getLogStartPtr(val));
        }
        TRACUCOMP( g_trac_trustedboot, "<<calcLogSize : %d", logSize);

        return logSize;
    }

    const uint8_t* TpmLogMgr_getFirstEvent(TpmLogMgr* val)
    {
        TCG_PCR_EVENT event;
        bool err = false;
        const uint8_t* result = NULL;

        // Header event in the log is always first, we skip over that
        const uint8_t* firstEvent = TpmLogMgr_getLogStartPtr(val);
        memset(&event, 0, sizeof(TCG_PCR_EVENT));

        firstEvent = TCG_PCR_EVENT_logUnmarshal(&event, firstEvent,
                                                sizeof(TCG_PCR_EVENT),
                                                &err);
        if (NULL != firstEvent && !err &&
            firstEvent < val->newEventPtr)
        {
            result = firstEvent;
        }

        return result;
    }

    const uint8_t* TpmLogMgr_getNextEvent(TpmLogMgr* val,
                                          const uint8_t* i_handle,
                                          TCG_PCR_EVENT2* i_eventLog,
                                          bool* o_err)
    {
        const uint8_t* l_resultPtr = NULL;
        if (NULL == i_handle)
        {
            *o_err = true;
        }
        else
        {
            memset(i_eventLog, 0, sizeof(TCG_PCR_EVENT2));
            TRACUCOMP( g_trac_trustedboot, "TPM getNextEvent 0x%p", i_handle);
            l_resultPtr = TCG_PCR_EVENT2_logUnmarshal(i_eventLog, i_handle,
                                                      sizeof(TCG_PCR_EVENT2),
                                                      o_err);
            if (NULL == l_resultPtr)
            {
                // An error was detected, ensure o_err is set
                *o_err = true;
            }
            else if (l_resultPtr >= val->newEventPtr)
            {
                l_resultPtr = NULL;
            }
        }

        return l_resultPtr;
    }

    TCG_PCR_EVENT2 TpmLogMgr_genLogEventPcrExtend(TPM_Pcr i_pcr,
                                                  TPM_Alg_Id i_algId_1,
                                                  const uint8_t* i_digest_1,
                                                  size_t i_digestSize_1,
                                                  TPM_Alg_Id i_algId_2,
                                                  const uint8_t* i_digest_2,
                                                  size_t i_digestSize_2,
						  uint32_t i_logType,
                                                  const char* i_logMsg)
    {
        TCG_PCR_EVENT2 eventLog;
        size_t fullDigestSize_1 = 0;
        size_t fullDigestSize_2 = 0;

        fullDigestSize_1 = getDigestSize(i_algId_1);
        if (NULL != i_digest_2)
        {
            fullDigestSize_2 = getDigestSize(i_algId_2);
        }

        memset(&eventLog, 0, sizeof(eventLog));
        eventLog.pcrIndex = i_pcr;
        eventLog.eventType = i_logType;

        // Update digest information
        eventLog.digests.count = 1;
        eventLog.digests.digests[0].algorithmId = i_algId_1;
        memcpy(&(eventLog.digests.digests[0].digest),
               i_digest_1,
               (i_digestSize_1 < fullDigestSize_1 ?
                i_digestSize_1 : fullDigestSize_1));

        if (NULL != i_digest_2)
        {
            eventLog.digests.count = 2;
            eventLog.digests.digests[1].algorithmId = i_algId_2;
            memcpy(&(eventLog.digests.digests[1].digest),
                   i_digest_2,
                   (i_digestSize_2 < fullDigestSize_2 ?
                    i_digestSize_2 : fullDigestSize_2));
        }
        // Event field data
        eventLog.event.eventSize = strlen(i_logMsg);
        memset(eventLog.event.event, 0, sizeof(eventLog.event.event));
        memcpy(eventLog.event.event, i_logMsg,
               (strlen(i_logMsg) > MAX_TPM_LOG_MSG ?
                MAX_TPM_LOG_MSG - 1 // Leave room for NULL termination
                : strlen(i_logMsg)) );

        return eventLog;
    }


    uint8_t* TpmLogMgr_getLogStartPtr(TpmLogMgr* val)
    {
#ifdef __HOSTBOOT_MODULE
        return (val->eventLogInMem == NULL ?
           reinterpret_cast<uint8_t*>(&(val->eventLog)) : val->eventLogInMem);
#else
        return val->eventLogInMem;
#endif
    }

#ifdef __HOSTBOOT_MODULE
    errlHndl_t TpmLogMgr_getDevtreeInfo(TpmLogMgr* val,
                                        uint64_t & io_logAddr,
                                        size_t & o_allocationSize,
                                        uint64_t & o_xscomAddr,
                                        uint32_t & o_i2cMasterOffset)
    {
        errlHndl_t err = NULL;

        mutex_lock( &val->logMutex );

        assert(io_logAddr != 0, "Invalid starting log address");
        assert(val->eventLogInMem == NULL,
               "getDevtreeInfo can only be called once");

        io_logAddr -= ALIGN_PAGE(TPMLOG_DEVTREE_SIZE);
        // Align to 64KB for Opal
        io_logAddr = ALIGN_DOWN_X(io_logAddr,64*KILOBYTE);

        val->inMemlogBaseAddr = io_logAddr;
        o_allocationSize = TPMLOG_DEVTREE_SIZE;
        o_xscomAddr = val->devtreeXscomAddr;
        o_i2cMasterOffset = val->devtreeI2cMasterOffset;

        // Copy image.
        val->eventLogInMem = (uint8_t*)(mm_block_map(
                                 (void*)(io_logAddr),
                                 ALIGN_PAGE(TPMLOG_DEVTREE_SIZE)));
        // Copy log into new location
        memset(val->eventLogInMem, 0, TPMLOG_DEVTREE_SIZE);
        memcpy(val->eventLogInMem, val->eventLog, val->logSize);
        val->newEventPtr = val->eventLogInMem + val->logSize;

        mutex_unlock( &val->logMutex );

        TRACUCOMP( g_trac_trustedboot,
                   "<<getDevtreeInfo() Addr:%lX - %s",
                   io_logAddr,
                   ((TB_SUCCESS == err) ? "No Error" : "With Error") );
        return err;
    }


    void TpmLogMgr_setTpmDevtreeInfo(TpmLogMgr* val,
                                     uint64_t i_xscomAddr,
                                     uint32_t i_i2cMasterOffset)
    {
        val->devtreeXscomAddr = i_xscomAddr;
        val->devtreeI2cMasterOffset = i_i2cMasterOffset;
    }

#endif

#ifdef __cplusplus
} // end TRUSTEDBOOT
#endif
