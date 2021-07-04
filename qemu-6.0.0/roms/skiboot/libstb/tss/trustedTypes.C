/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/secureboot/trusted/trustedTypes.C $                   */
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
 * @file trustedTypes.C
 *
 * @brief Trusted boot type inline functions
 */

/////////////////////////////////////////////////////////////////
// NOTE: This file is exportable as TSS-Lite for skiboot/PHYP  //
/////////////////////////////////////////////////////////////////

// ----------------------------------------------
// Includes
// ----------------------------------------------
#include <string.h>
#include "trustedboot.H"
#include "trustedTypes.H"

#ifdef __cplusplus
namespace TRUSTEDBOOT
{
#endif

    const uint8_t* unmarshalChunk(const uint8_t* i_tpmBuf,
                            size_t * io_tpmBufSize,
                            void* o_chunkPtr,
                            size_t i_chunkSize);

    uint8_t* marshalChunk(uint8_t* o_tpmBuf,
                          size_t i_tpmBufSize,
                          size_t * io_cmdSize,
                          const void* i_chunkPtr,
                          size_t i_chunkSize);

    const uint8_t* unmarshalChunk(const uint8_t* i_tpmBuf,
                            size_t * io_tpmBufSize,
                            void* o_chunkPtr,
                            size_t i_chunkSize)
    {
        if (NULL != i_tpmBuf)
        {
            if (i_chunkSize > *io_tpmBufSize)
            {
                return NULL;
            }
            memcpy(o_chunkPtr, i_tpmBuf, i_chunkSize);
            i_tpmBuf += i_chunkSize;
            *io_tpmBufSize -= i_chunkSize;
        }
        return i_tpmBuf;
    }

    uint8_t* marshalChunk(uint8_t* o_tpmBuf,
                          size_t i_tpmBufSize,
                          size_t * io_cmdSize,
                          const void* i_chunkPtr,
                          size_t i_chunkSize)
    {
        if (NULL != o_tpmBuf)
        {
            if ((*io_cmdSize + i_chunkSize) > i_tpmBufSize)
            {
                return NULL;
            }
            memcpy(o_tpmBuf, i_chunkPtr, i_chunkSize);
            o_tpmBuf += i_chunkSize;
            *io_cmdSize += i_chunkSize;
        }
        return o_tpmBuf;
    }

    uint32_t getDigestSize(const TPM_Alg_Id i_algId)
    {
        uint32_t ret = 0;
        switch (i_algId)
        {
          case TPM_ALG_SHA1:
            ret = TPM_ALG_SHA1_SIZE;
            break;
          case TPM_ALG_SHA256:
            ret = TPM_ALG_SHA256_SIZE;
            break;
          default:
            ret = 0;
            break;
        };
        return ret;
    }

    const uint8_t* TPML_TAGGED_TPM_PROPERTY_unmarshal(
                           TPML_TAGGED_TPM_PROPERTY* val,
                           const uint8_t* i_tpmBuf,
                           size_t* io_tpmBufSize)
    {

        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->count), sizeof(val->count));

        // Now we know the count as well
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->tpmProperty[0]),
                                  sizeof(TPMS_TAGGED_PROPERTY) * val->count);

        return i_tpmBuf;
    }

    const uint8_t* TPMS_CAPABILITY_DATA_unmarshal(TPMS_CAPABILITY_DATA* val,
                                                  const uint8_t* i_tpmBuf,
                                                  size_t * io_tpmBufSize)
    {
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->capability),
                                  sizeof(val->capability));

        switch (val->capability)
        {
          case TPM_CAP_TPM_PROPERTIES:
              {
                  return TPML_TAGGED_TPM_PROPERTY_unmarshal(
                                      &(val->data.tpmProperties), i_tpmBuf,
                                      io_tpmBufSize);
              }
              break;
          default:
              {
                  TRACFCOMP( g_trac_trustedboot,
                       "TPMS_CAPABILITY_DATA::unmarshal Unknown capability");
                  return NULL;
              }
              break;
        }
        return NULL;
    }

    uint8_t* TPMT_HA_marshal(const TPMT_HA* val,
                             uint8_t* o_tpmBuf,
                             size_t i_tpmBufSize,
                             size_t * io_cmdSize)
    {
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                &(val->algorithmId), sizeof(val->algorithmId));
        if (getDigestSize((TPM_Alg_Id)val->algorithmId) == 0)
        {
            return NULL;
        }
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                &(val->digest),
                                getDigestSize((TPM_Alg_Id)val->algorithmId));
        return o_tpmBuf;
    }

    uint8_t* TPML_DIGEST_VALUES_marshal(const TPML_DIGEST_VALUES* val,
                                        uint8_t* o_tpmBuf,
                                        size_t i_tpmBufSize,
                                        size_t * io_cmdSize)
    {
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                &(val->count), sizeof(val->count));
        if (NULL != o_tpmBuf && HASH_COUNT < val->count)
        {
            o_tpmBuf = NULL;
        }
        else
        {
            for (size_t idx = 0; idx < val->count; idx++)
            {
                o_tpmBuf = TPMT_HA_marshal(&(val->digests[idx]),
                                           o_tpmBuf,
                                           i_tpmBufSize,
                                           io_cmdSize);
                if (NULL == o_tpmBuf)
                {
                    break;
                }
            }
        }
        return o_tpmBuf;
    }

    uint8_t* TPM2_BaseIn_marshal(const TPM2_BaseIn* val, uint8_t* o_tpmBuf,
                                 size_t i_tpmBufSize, size_t* io_cmdSize)
    {
        return marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                            val, sizeof(TPM2_BaseIn));
    }

    const uint8_t* TPM2_BaseOut_unmarshal(TPM2_BaseOut* val,
                                          const uint8_t* i_tpmBuf,
                                          size_t* io_tpmBufSize,
                                          size_t i_outBufSize)
    {
        if (sizeof(TPM2_BaseOut) > i_outBufSize)
        {
            return NULL;
        }
        return unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                              val, sizeof(TPM2_BaseOut));
    }

    uint8_t* TPM2_2ByteIn_marshal(const TPM2_2ByteIn* val,
                                  uint8_t* o_tpmBuf,
                                  size_t i_tpmBufSize,
                                  size_t* io_cmdSize)
    {
        // Base has already been marshaled
        return marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                            &(val->param), sizeof(val->param));
    }

    uint8_t* TPM2_4ByteIn_marshal(const TPM2_4ByteIn* val,
                                  uint8_t* o_tpmBuf,
                                  size_t i_tpmBufSize,
                                  size_t* io_cmdSize)
    {
        // Base has already been marshaled
        return marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                            &(val->param), sizeof(val->param));
    }

    uint8_t* TPM2_GetCapabilityIn_marshal(const TPM2_GetCapabilityIn* val,
                                          uint8_t* o_tpmBuf,
                                          size_t i_tpmBufSize,
                                          size_t* io_cmdSize)
    {
        // Base has already been marshaled
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                &(val->capability),
                                sizeof(val->capability));
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                &(val->property),
                                sizeof(val->property));
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                &(val->propertyCount),
                                sizeof(val->propertyCount));
        return o_tpmBuf;
    }

    const uint8_t* TPM2_GetCapabilityOut_unmarshal(TPM2_GetCapabilityOut* val,
                                                   const uint8_t* i_tpmBuf,
                                                   size_t* io_tpmBufSize,
                                                   size_t i_outBufSize)
    {
        // Base has already been unmarshaled
        if (sizeof(TPM2_GetCapabilityOut) > i_outBufSize)
        {
            return NULL;
        }
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->moreData), sizeof(val->moreData));

        // Capability data block
        return TPMS_CAPABILITY_DATA_unmarshal(&(val->capData), i_tpmBuf,
                                              io_tpmBufSize);

    }

    uint8_t* TPM2_ExtendIn_marshalHandle(const TPM2_ExtendIn* val,
                                         uint8_t* o_tpmBuf,
                                         size_t i_tpmBufSize,
                                         size_t* io_cmdSize)
    {
        // Base has already been marshaled
        // only marshal the pcr handle in this stage
        return marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                            &(val->pcrHandle), sizeof(val->pcrHandle));
    }

    uint8_t* TPM2_ExtendIn_marshalParms(const TPM2_ExtendIn* val,
                                        uint8_t* o_tpmBuf,
                                        size_t i_tpmBufSize,
                                        size_t* io_cmdSize)
    {
        // Base and handle has already been marshaled
        return (TPML_DIGEST_VALUES_marshal(&(val->digests), o_tpmBuf,
                                           i_tpmBufSize, io_cmdSize));
    }

    uint8_t* TPMS_PCR_SELECTION_marshal(const TPMS_PCR_SELECTION* val,
                                        uint8_t* o_tpmBuf,
                                        size_t i_tpmBufSize,
                                        size_t* io_cmdSize)
    {
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                &(val->algorithmId), sizeof(val->algorithmId));
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                               &(val->sizeOfSelect), sizeof(val->sizeOfSelect));

        if (NULL != o_tpmBuf &&
            PCR_SELECT_MAX < val->sizeOfSelect)
        {
            return NULL;
        }

        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                val->pcrSelect, val->sizeOfSelect);
        return o_tpmBuf;
    }

    const uint8_t* TPMS_PCR_SELECTION_unmarshal(TPMS_PCR_SELECTION* val,
                                                const uint8_t* i_tpmBuf,
                                                size_t* io_tpmBufSize)
    {
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->algorithmId),
                                  sizeof(val->algorithmId));
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->sizeOfSelect),
                                  sizeof(val->sizeOfSelect));
        if (NULL != i_tpmBuf &&
            PCR_SELECT_MAX < val->sizeOfSelect)
        {
            return NULL;
        }
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  val->pcrSelect, val->sizeOfSelect);

        return i_tpmBuf;
    }

    const uint8_t* TPM2B_DIGEST_unmarshal(TPM2B_DIGEST* val,
                                    const uint8_t* i_tpmBuf,
                                    size_t* io_tpmBufSize)
    {
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &val->size, sizeof(val->size));
        if (NULL != i_tpmBuf &&
            sizeof(TPMU_HA) < val->size)
        {
            TRACUCOMP( g_trac_trustedboot,
                       "TPM2B_DIGEST::unmarshal invalid size");
            return NULL;
        }
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  val->buffer, val->size);
        return i_tpmBuf;

    }

    const uint8_t* TPML_DIGEST_unmarshal(TPML_DIGEST* val,
                                   const uint8_t* i_tpmBuf,
                                   size_t* io_tpmBufSize)
    {
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->count), sizeof(val->count));
        if (NULL != i_tpmBuf && HASH_COUNT < val->count)
        {
            TRACUCOMP( g_trac_trustedboot,
                       "TPML_DIGEST::unmarshal invalid count %d", val->count);
            i_tpmBuf = NULL;
        }
        else if (NULL != i_tpmBuf)
        {
            for (size_t idx = 0; idx < val->count; idx++)
            {
                i_tpmBuf = TPM2B_DIGEST_unmarshal(&(val->digests[idx]),
                                                  i_tpmBuf,
                                                  io_tpmBufSize);
                if (NULL == i_tpmBuf)
                {
                    break;
                }
            }
        }
        return i_tpmBuf;

    }

    uint8_t* TPML_PCR_SELECTION_marshal(const TPML_PCR_SELECTION* val,
                                        uint8_t* o_tpmBuf,
                                        size_t i_tpmBufSize,
                                        size_t* io_cmdSize)
    {
        o_tpmBuf = marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                                &(val->count), sizeof(val->count));
        if (NULL != o_tpmBuf && HASH_COUNT < val->count)
        {
            TRACUCOMP( g_trac_trustedboot,
                       "TPML_PCR_SELECTION::marshal invalid count");
            o_tpmBuf = NULL;
        }
        else if (NULL != o_tpmBuf)
        {
            for (size_t idx = 0; idx < val->count; idx++)
            {
                o_tpmBuf = TPMS_PCR_SELECTION_marshal(
                                          &(val->pcrSelections[idx]),
                                          o_tpmBuf,
                                          i_tpmBufSize,
                                          io_cmdSize);
                if (NULL == o_tpmBuf)
                {
                    break;
                }
            }
        }
        return o_tpmBuf;
    }

    const uint8_t* TPML_PCR_SELECTION_unmarshal(TPML_PCR_SELECTION* val,
                                          const uint8_t* i_tpmBuf,
                                          size_t* io_tpmBufSize)
    {
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->count), sizeof(val->count));
        if (NULL != i_tpmBuf && HASH_COUNT < val->count)
        {
            TRACUCOMP( g_trac_trustedboot,
                       "TPML_PCR_SELECTION::unmarshal invalid count");
            i_tpmBuf = NULL;
        }
        else if (NULL != i_tpmBuf)
        {
            for (size_t idx = 0; idx < val->count; idx++)
            {
                i_tpmBuf = TPMS_PCR_SELECTION_unmarshal(
                                 &(val->pcrSelections[idx]),
                                 i_tpmBuf,
                                 io_tpmBufSize);
                if (NULL == i_tpmBuf)
                {
                    break;
                }
            }
        }
        return i_tpmBuf;

    }

    uint8_t* TPM2_PcrReadIn_marshal(const TPM2_PcrReadIn* val,
                                    uint8_t* o_tpmBuf,
                                    size_t i_tpmBufSize,
                                    size_t* io_cmdSize)
    {
        // Base and handle has already been marshaled
        return (TPML_PCR_SELECTION_marshal(&(val->pcrSelectionIn), o_tpmBuf,
                                           i_tpmBufSize, io_cmdSize));
    }

    const uint8_t* TPM2_PcrReadOut_unmarshal(TPM2_PcrReadOut* val,
                                       const uint8_t* i_tpmBuf,
                                       size_t* io_tpmBufSize,
                                       size_t i_outBufSize)
    {
        // Base and handle has already been marshaled
        if (sizeof(TPM2_PcrReadOut) > i_outBufSize) return NULL;
        i_tpmBuf = unmarshalChunk(i_tpmBuf, io_tpmBufSize,
                                  &(val->pcrUpdateCounter),
                                  sizeof(val->pcrUpdateCounter));

        i_tpmBuf = TPML_PCR_SELECTION_unmarshal(&(val->pcrSelectionOut),
                                                i_tpmBuf, io_tpmBufSize);
        i_tpmBuf = TPML_DIGEST_unmarshal(&(val->pcrValues), i_tpmBuf,
                                         io_tpmBufSize);
        return i_tpmBuf;

    }

    uint8_t* TPMS_AUTH_COMMAND_marshal(const TPMS_AUTH_COMMAND* val,
                                       uint8_t* o_tpmBuf,
                                       size_t i_tpmBufSize,
                                       size_t* io_cmdSize)
    {
        return marshalChunk(o_tpmBuf, i_tpmBufSize, io_cmdSize,
                            val, sizeof(TPMS_AUTH_COMMAND));
    }


    uint8_t* TPMT_HA_logMarshal(const TPMT_HA* val, uint8_t* i_logBuf)
    {
        uint16_t* field16 = (uint16_t*)i_logBuf;
        *field16 = htole16(val->algorithmId);
        i_logBuf += sizeof(uint16_t);
        memcpy(i_logBuf, &(val->digest),
               getDigestSize((TPM_Alg_Id)val->algorithmId));
        i_logBuf += getDigestSize((TPM_Alg_Id)val->algorithmId);
        return i_logBuf;
    }

    const uint8_t* TPMT_HA_logUnmarshal(TPMT_HA* val,
                                        const uint8_t* i_tpmBuf, bool* o_err)
    {
        size_t size = 0;
        uint16_t* field16 = NULL;

        do {
            *o_err = false;

            // algorithmId
            size = sizeof(val->algorithmId);
            field16 = (uint16_t*)i_tpmBuf;
            val->algorithmId = le16toh(*field16);
            // Ensure a valid count
            if (val->algorithmId >= TPM_ALG_INVALID_ID)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                TRACFCOMP(g_trac_trustedboot,"ERROR> TPMT_HA:logUnmarshal()"
                          " invalid algorithmId %d",
                          val->algorithmId);
                break;
            }
            i_tpmBuf += size;

            // digest
            size = getDigestSize((TPM_Alg_Id)val->algorithmId);
            // Ensure a valid count
            if (size >= TPM_ALG_INVALID_SIZE)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                TRACFCOMP(g_trac_trustedboot,"ERROR> TPMT_HA:logUnmarshal() "
                          "invalid algorithm size of %d for algorithm id %d",
                          (int)size, val->algorithmId);
                break;
            }

            memcpy(&(val->digest), i_tpmBuf, size);
            i_tpmBuf += size;
        } while(0);

        return i_tpmBuf;
    }

    size_t TPMT_HA_marshalSize(const TPMT_HA* val)
    {
        return (sizeof(val->algorithmId) +
                getDigestSize((TPM_Alg_Id)(val->algorithmId)));
    }

#ifdef __cplusplus
    bool TPMT_HA::operator==(const TPMT_HA& i_rhs) const
    {
        size_t digestSize = getDigestSize((TPM_Alg_Id)algorithmId);
        return (algorithmId == i_rhs.algorithmId) &&
            (memcmp(&(digest), &(i_rhs.digest), digestSize) == 0);
    }
#endif

    size_t TPML_DIGEST_VALUES_marshalSize(const TPML_DIGEST_VALUES* val)
    {
        size_t ret = sizeof(val->count);
        for (size_t idx = 0; (idx < val->count && idx < HASH_COUNT); idx++)
        {
            ret += TPMT_HA_marshalSize(&(val->digests[idx]));
        }
        return ret;
    }

    uint8_t* TPML_DIGEST_VALUES_logMarshal(const TPML_DIGEST_VALUES* val,
                                           uint8_t* i_logBuf)
    {
        uint32_t* field32 = (uint32_t*)i_logBuf;
        if (HASH_COUNT < val->count)
        {
            i_logBuf = NULL;
        }
        else
        {
            *field32 = htole32(val->count);
            i_logBuf += sizeof(uint32_t);
            for (size_t idx = 0; idx < val->count; idx++)
            {
                i_logBuf = TPMT_HA_logMarshal(&(val->digests[idx]), i_logBuf);
                if (NULL == i_logBuf) break;
            }
        }
        return i_logBuf;
    }

    const uint8_t* TPML_DIGEST_VALUES_logUnmarshal(TPML_DIGEST_VALUES* val,
                                                   const uint8_t* i_tpmBuf,
                                                   bool* o_err)
    {
        size_t size = 0;
        uint32_t* field32 = NULL;
        do {
            *o_err = false;

            // count
            size = sizeof(val->count);
            field32 = (uint32_t*)(i_tpmBuf);
            val->count = le32toh(*field32);
            // Ensure a valid count
            if (val->count > HASH_COUNT)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                TRACFCOMP(g_trac_trustedboot,"ERROR> "
                          "TPML_DIGEST_VALUES:logUnmarshal() "
                          "invalid digest count %d",
                          val->count);
                break;
            }
            i_tpmBuf += size;

            // Iterate all digests
            for (size_t idx = 0; idx < val->count; idx++)
            {
                i_tpmBuf = TPMT_HA_logUnmarshal(&(val->digests[idx]),
                                                i_tpmBuf, o_err);
                    if (NULL == i_tpmBuf)
                    {
                        break;
                    }
            }
        } while(0);

        return i_tpmBuf;
    }

#ifdef __cplusplus
    bool TPML_DIGEST_VALUES::operator==(const TPML_DIGEST_VALUES& i_rhs) const
    {
        bool result = (count == i_rhs.count);
        // Iterate all digests
        for (size_t idx = 0; idx < count; idx++)
        {
            result = (result && (digests[idx] == i_rhs.digests[idx]));
        }

        return result;
    }
#endif

    const uint8_t* TCG_PCR_EVENT_logUnmarshal(TCG_PCR_EVENT* val,
                                              const uint8_t* i_tpmBuf,
                                              size_t i_bufSize,
                                              bool* o_err)
    {
        size_t size = 0;
        uint32_t* field32;

        *o_err = false;
        do {
            // Ensure enough space for unmarshalled data
            if (sizeof(TCG_PCR_EVENT) > i_bufSize)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                break;
            }

            // pcrIndex
            size = sizeof(val->pcrIndex);
            field32 = (uint32_t*)(i_tpmBuf);
            val->pcrIndex = le32toh(*field32);
            // Ensure a valid pcr index
            if (val->pcrIndex >= IMPLEMENTATION_PCR)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                TRACFCOMP(g_trac_trustedboot,
                  "ERROR> TCG_PCR_EVENT:logUnmarshal() invalid pcrIndex %d",
                          val->pcrIndex);
                break;
            }
            i_tpmBuf += size;

            // eventType
            size = sizeof(val->eventType);
            field32 = (uint32_t*)(i_tpmBuf);
            val->eventType = le32toh(*field32);
            // Ensure a valid event type
            if (val->eventType == 0 || val->eventType >= EV_INVALID)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                TRACFCOMP(g_trac_trustedboot,
                    "ERROR> TCG_PCR_EVENT:logUnmarshal() invalid eventType %d",
                          val->eventType);
                break;
            }
            i_tpmBuf += size;

            // digest
            size = sizeof(val->digest);
            memcpy(val->digest, i_tpmBuf, size);
            i_tpmBuf += size;

            // eventSize
            size = sizeof(val->eventSize);
            field32 = (uint32_t*)(i_tpmBuf);
            val->eventSize = le32toh(*field32);
            // Ensure a valid eventSize
            if (val->eventSize >= MAX_TPM_LOG_MSG)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                TRACFCOMP(g_trac_trustedboot,
                    "ERROR> TCG_PCR_EVENT:logUnmarshal() invalid eventSize %d",
                          val->eventSize);
                break;
            }
            i_tpmBuf += size;

            memcpy(val->event, i_tpmBuf, val->eventSize);
            i_tpmBuf += val->eventSize;

        } while(0);

        return i_tpmBuf;
    }

    uint8_t* TCG_PCR_EVENT_logMarshal(const TCG_PCR_EVENT* val,
                                      uint8_t* i_logBuf)
    {
        uint32_t* field32 = (uint32_t*)(i_logBuf);
        *field32 = htole32(val->pcrIndex);
        i_logBuf += sizeof(uint32_t);

        field32 = (uint32_t*)(i_logBuf);
        *field32 = htole32(val->eventType);
        i_logBuf += sizeof(uint32_t);

        memcpy(i_logBuf, val->digest, sizeof(val->digest));
        i_logBuf += sizeof(val->digest);

        field32 = (uint32_t*)(i_logBuf);
        *field32 = htole32(val->eventSize);
        i_logBuf += sizeof(uint32_t);

        if (val->eventSize > 0)
        {
            memcpy(i_logBuf, val->event, val->eventSize);
            i_logBuf += val->eventSize;
        }
        return i_logBuf;
    }

    size_t TCG_PCR_EVENT_marshalSize(const TCG_PCR_EVENT* val)
    {
        return (sizeof(TCG_PCR_EVENT) + val->eventSize - MAX_TPM_LOG_MSG);
    }

    uint8_t* TPM_EVENT_FIELD_logMarshal(const TPM_EVENT_FIELD* val,
                                        uint8_t* i_logBuf)
    {
        uint32_t* field32 = (uint32_t*)i_logBuf;
        if (MAX_TPM_LOG_MSG < val->eventSize)
        {
            i_logBuf = NULL;
        }
        else
        {
            *field32 = htole32(val->eventSize);
            i_logBuf += sizeof(uint32_t);

            memcpy(i_logBuf, val->event, val->eventSize);
            i_logBuf += val->eventSize;
        }
        return i_logBuf;
    }

    const uint8_t* TPM_EVENT_FIELD_logUnmarshal(TPM_EVENT_FIELD* val,
                                                const uint8_t* i_tpmBuf,
                                                bool* o_err)
    {
        size_t size = 0;
        uint32_t* field32 = NULL;
        do {
            *o_err = false;

            // Event size
            size = sizeof(val->eventSize);
            field32 = (uint32_t*)(i_tpmBuf);
            val->eventSize = le32toh(*field32);
            i_tpmBuf += size;

            // Event
            size = val->eventSize;
            if (size > MAX_TPM_LOG_MSG)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                break;
            }
            memcpy(&val->event, i_tpmBuf, size);
            i_tpmBuf += size;
        } while(0);

        return i_tpmBuf;
    }
    size_t TPM_EVENT_FIELD_marshalSize(const TPM_EVENT_FIELD* val)
    {
        return (sizeof(val->eventSize) + val->eventSize);
    }


#ifdef __cplusplus
    bool TPM_EVENT_FIELD::operator==(const TPM_EVENT_FIELD& i_rhs) const
    {
        return (eventSize == i_rhs.eventSize) &&
               (memcmp(event, i_rhs.event, eventSize) == 0);
    }
#endif


    size_t TCG_PCR_EVENT2_marshalSize(const TCG_PCR_EVENT2* val)
    {
        return (sizeof(val->pcrIndex) + sizeof(val->eventType) +
                TPML_DIGEST_VALUES_marshalSize(&(val->digests)) +
                TPM_EVENT_FIELD_marshalSize(&(val->event)));
    }

    uint8_t* TCG_PCR_EVENT2_logMarshal(const TCG_PCR_EVENT2* val,
                                       uint8_t* i_logBuf)
    {
        uint32_t* field32 = (uint32_t*)i_logBuf;
        *field32 = htole32(val->pcrIndex);
        i_logBuf += sizeof(uint32_t);
        field32 = (uint32_t*)i_logBuf;
        *field32 = htole32(val->eventType);
        i_logBuf += sizeof(uint32_t);

        i_logBuf = TPML_DIGEST_VALUES_logMarshal(&(val->digests),i_logBuf);
        if (NULL != i_logBuf)
        {
            i_logBuf = TPM_EVENT_FIELD_logMarshal(&(val->event),i_logBuf);
        }
        return i_logBuf;
    }

    const uint8_t* TCG_PCR_EVENT2_logUnmarshal(TCG_PCR_EVENT2* val,
                                               const uint8_t* i_tpmBuf,
                                               size_t i_bufSize,
                                               bool* o_err)
    {
        size_t size = 0;
        uint32_t* field32 = NULL;

        do {
            *o_err = false;

            // Ensure enough space for unmarshalled data
            if (sizeof(TCG_PCR_EVENT2) > i_bufSize)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                break;
            }

            // pcrIndex
            size = sizeof(val->pcrIndex);
            field32 = (uint32_t*)(i_tpmBuf);
            val->pcrIndex = le32toh(*field32);
            // Ensure a valid pcr index
            if (val->pcrIndex > IMPLEMENTATION_PCR)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                TRACUCOMP(g_trac_trustedboot,"ERROR> TCG_PCR_EVENT2:"
                          "logUnmarshal() invalid pcrIndex %d",
                          val->pcrIndex);
                break;
            }
            i_tpmBuf += size;

            // eventType
            size = sizeof(val->eventType);
            field32 = (uint32_t*)(i_tpmBuf);
            val->eventType = le32toh(*field32);
            // Ensure a valid event type
            if (val->eventType == 0 ||
                val->eventType >= EV_INVALID)
            {
                *o_err = true;
                i_tpmBuf = NULL;
                TRACUCOMP(g_trac_trustedboot,"ERROR> TCG_PCR_EVENT2:"
                          "logUnmarshal() invalid eventType %d",
                          val->eventType);
                break;
            }
            i_tpmBuf += size;

            // TPML_DIGEST_VALUES
            i_tpmBuf = TPML_DIGEST_VALUES_logUnmarshal(&(val->digests),
                                                       i_tpmBuf, o_err);
            if (i_tpmBuf == NULL)
            {
                break;
            }

            // TPM EVENT FIELD
            i_tpmBuf = TPM_EVENT_FIELD_logUnmarshal(&(val->event),
                                                    i_tpmBuf, o_err);
            if (i_tpmBuf == NULL)
            {
                break;
            }
        } while(0);

        return i_tpmBuf;
    }

#ifdef __cplusplus
    bool TCG_PCR_EVENT2::operator==(const TCG_PCR_EVENT2& i_rhs) const
    {
        return (pcrIndex == i_rhs.pcrIndex) &&
               (eventType == i_rhs.eventType) &&
               (digests == i_rhs.digests) &&
               (event == i_rhs.event);
    }
} // end TRUSTEDBOOT
#endif
