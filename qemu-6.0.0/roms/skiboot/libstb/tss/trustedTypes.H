/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/secureboot/trusted/trustedTypes.H $                   */
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
 * @file trustedTypes.H
 *
 * @brief Trustedboot TPM Types
 *
 */

/////////////////////////////////////////////////////////////////
// NOTE: This file is exportable as TSS-Lite for skiboot/PHYP  //
/////////////////////////////////////////////////////////////////

#ifndef __TRUSTEDTYPES_H
#define __TRUSTEDTYPES_H
// -----------------------------------------------
// Includes
// -----------------------------------------------
#include <stdint.h>
#ifdef __HOSTBOOT_MODULE
#include <builtins.h>
#include <secureboot/trustedbootif.H>
#else
#include "trustedboot.H"
#define PACKED __attribute__((__packed__))
#endif

#ifdef __cplusplus
namespace TRUSTEDBOOT
{
#endif

    /// TPM Algorithm defines
    typedef enum
    {
        TPM_ALG_SHA1    = 0x0004,   ///< SHA1 Id
        TPM_ALG_SHA256  = 0x000B,   ///< SHA256 Id
        TPM_ALG_INVALID_ID          ///< Used for error checking
    } TPM_Alg_Id;

    typedef enum
    {
        TPM_ALG_NUM_SUPPORTED = 1,       ///< Number of algorithms supported
        TPM_ALG_SHA1_SIZE     = 20,      ///< SHA1 digest byte size
        TPM_ALG_SHA256_SIZE   = 32,      ///< SHA256 digest byte size
        TPM_ALG_INVALID_SIZE             ///< Used for error checking
    } TPM_Alg_Sizes;

    /// Common static values
    enum
    {
        MAX_TPM_LOG_MSG = 128,    ///< Maximum log message size

        HASH_COUNT            = 2,       ///< Maximum # of digests

        PCR_SELECT_MAX = (IMPLEMENTATION_PCR+7)/8, ///< PCR selection octet max
    };

    typedef enum
    {
        EV_PREBOOT_CERT            = 0x00,
        EV_POST_CODE               = 0x01,
        EV_UNUSED                  = 0x02,
        EV_NO_ACTION               = 0x03,
        EV_SEPARATOR               = 0x04,
        EV_ACTION                  = 0x05,
        EV_EVENT_TAG               = 0x06,
        EV_S_CRTM_CONTENTS         = 0x07,
        EV_S_CRTM_VERSION          = 0x08,
        EV_CPU_MICROCODE           = 0x09,
        EV_PLATFORM_CONFIG_FLAGS   = 0x0A,
        EV_TABLE_OF_DEVICES        = 0x0B,
        EV_COMPACT_HASH            = 0x0C,
        EV_IPL                     = 0x0D,   ///< Deprecated
        EV_IPL_PARTITION_DATA      = 0x0E,   ///< Deprecated
        EV_NONHOST_CODE            = 0x0F,
        EV_NONHOST_CONFIG          = 0x10,
        EV_NONHOST_INFO            = 0x11,
        EV_OMIT_BOOT_DEVICE_EVENTS = 0x12,
        EV_INVALID                 ///< Used for error checking
    } EventTypes;

    /**
     * @brief Get the digest size of the selected hash algorithm
     * @param[in] i_algId Algorith ID to query
     * @returns digest length in bytes, 0 on invalid algorithm
     */
    uint32_t getDigestSize(const TPM_Alg_Id i_algId)  __attribute__ ((const));


    /// Various static values
    enum
    {
        // TPM Spec supported
        TPM_SPEC_MAJOR        = 2,
        TPM_SPEC_MINOR        = 0,
        TPM_SPEC_ERRATA       = 0,
        TPM_PLATFORM_SERVER   = 1,

        // Command structure tags
        TPM_ST_NO_SESSIONS        = 0x8001,    ///< A command with no sess/auth
        TPM_ST_SESSIONS           = 0x8002,    ///< A command has sessions

        // Command Codes
        TPM_CC_Startup            = 0x00000144,
        TPM_CC_GetCapability      = 0x0000017A,
        TPM_CC_PCR_Read           = 0x0000017E,
        TPM_CC_PCR_Extend         = 0x00000182,


        // TPM Startup types
        TPM_SU_CLEAR              = 0x0000,///< TPM perform reset,restart
        TPM_SU_STATE              = 0x0001,///< TPM perform restore saved state

        // Capability
        MAX_TPM_PROPERTIES        = 2,
        TPM_CAP_TPM_PROPERTIES    = 0x00000006, ///< Pull TPM Properties

        // TPM Properties
        TPM_PT_MANUFACTURER       = 0x00000105,
        TPM_PT_FIRMWARE_VERSION_1 = 0x0000010B,
        TPM_PT_FIRMWARE_VERSION_2 = 0x0000010C,


        // TPM Return Codes
        TPM_SUCCESS               = 0x000,

        TPM_RC_INITIALIZE         = 0x100,


        // TPM Authorization types
        TPM_RS_PW                 = 0x40000009,

    };


    // Command structures taken from Trusted Platform Module Library Part 3:
    //    Commands Family "2.0"

    /// TPM capability response structure
    struct _TPMS_TAGGED_PROPERTY
    {
        uint32_t property;  ///< TPM_PT_xx identifier
        uint32_t value;     ///< value of the property
    } PACKED;
    typedef struct _TPMS_TAGGED_PROPERTY TPMS_TAGGED_PROPERTY;

    struct _TPML_TAGGED_TPM_PROPERTY
    {
        uint32_t count;   ///< Number of properties
        TPMS_TAGGED_PROPERTY tpmProperty[MAX_TPM_PROPERTIES];
    } PACKED;
    typedef struct _TPML_TAGGED_TPM_PROPERTY TPML_TAGGED_TPM_PROPERTY;
    const uint8_t* TPML_TAGGED_TPM_PROPERTY_unmarshal(
                                      TPML_TAGGED_TPM_PROPERTY* val,
                                      const uint8_t* i_tpmBuf,
                                      size_t* io_tpmBufSize);

    union _TPMU_CAPABILITIES
    {
        // Currently only TPM properties supported
        TPML_TAGGED_TPM_PROPERTY tpmProperties;
    } PACKED;
    typedef union _TPMU_CAPABILITIES TPMU_CAPABILITIES;

    struct _TPMS_CAPABILITY_DATA
    {
        uint32_t capability;    ///< The capability type
        TPMU_CAPABILITIES data; ///< The capability data
    } PACKED;
    typedef struct _TPMS_CAPABILITY_DATA TPMS_CAPABILITY_DATA;
    const uint8_t* TPMS_CAPABILITY_DATA_unmarshal(TPMS_CAPABILITY_DATA* val,
                                            const uint8_t* i_tpmBuf,
                                            size_t * io_tpmBufSize);


    /// SHA1 Event log entry format
    struct _TCG_PCR_EVENT
    {
        uint32_t   pcrIndex;    ///< PCRIndex event extended to
        uint32_t   eventType;   ///< Type of event
        uint8_t    digest[20];  ///< Value extended into PCR index
        uint32_t   eventSize;   ///< Size of event data
        uint8_t    event[MAX_TPM_LOG_MSG]; ///< The event data
    } PACKED;
    typedef struct _TCG_PCR_EVENT TCG_PCR_EVENT;
    size_t TCG_PCR_EVENT_marshalSize(const TCG_PCR_EVENT* val);
    const uint8_t* TCG_PCR_EVENT_logUnmarshal(TCG_PCR_EVENT* val,
                                              const uint8_t* i_tpmBuf,
                                              size_t i_bufSize, bool* o_err);
    uint8_t* TCG_PCR_EVENT_logMarshal(const TCG_PCR_EVENT* val,
                                      uint8_t* i_logBuf);

    /// Digest union
    union _TPMU_HA
    {
        uint8_t sha1[TPM_ALG_SHA1_SIZE];
        uint8_t sha256[TPM_ALG_SHA256_SIZE];
    } PACKED;
    typedef union _TPMU_HA TPMU_HA;

    /// Crypto agile digest
    struct _TPMT_HA
    {
        uint16_t   algorithmId; ///< ID of hashing algorithm
        TPMU_HA    digest;      ///< Digest, depends on algorithmid
#ifdef __cplusplus
        bool operator==(const _TPMT_HA& i_rhs) const;
#endif
    } PACKED;
    typedef struct _TPMT_HA TPMT_HA;
    size_t TPMT_HA_marshalSize(const TPMT_HA* val);
    uint8_t* TPMT_HA_logMarshal(const TPMT_HA* val, uint8_t* i_logBuf);
    const uint8_t* TPMT_HA_logUnmarshal(TPMT_HA* val,
                                        const uint8_t* i_tpmBuf, bool* o_err);
    uint8_t* TPMT_HA_marshal(const TPMT_HA* val, uint8_t* o_tpmBuf,
                             size_t i_tpmBufSize, size_t * io_cmdSize);


    /// Crypto agile digests list
    struct _TPML_DIGEST_VALUES
    {
        uint32_t   count;                ///< Number of digests
        TPMT_HA    digests[HASH_COUNT];  ///< Digests
#ifdef __cplusplus
        bool operator==(const _TPML_DIGEST_VALUES& i_rhs) const;
#endif
    } PACKED;
    typedef struct _TPML_DIGEST_VALUES TPML_DIGEST_VALUES;
    size_t TPML_DIGEST_VALUES_marshalSize(const TPML_DIGEST_VALUES* val);
    uint8_t* TPML_DIGEST_VALUES_logMarshal(const TPML_DIGEST_VALUES* val,
                                           uint8_t* i_logBuf);
    const uint8_t* TPML_DIGEST_VALUES_logUnmarshal(TPML_DIGEST_VALUES* val,
                                                   const uint8_t* i_tpmBuf,
                                                   bool* o_err);
    uint8_t* TPML_DIGEST_VALUES_marshal(const TPML_DIGEST_VALUES* val,
                                        uint8_t* o_tpmBuf, size_t i_tpmBufSize,
                                        size_t * io_cmdSize);

    /// Event field structure
    struct _TPM_EVENT_FIELD
    {
        uint32_t   eventSize;               ///< Size of event data
        uint8_t    event[MAX_TPM_LOG_MSG];  ///< The event data
#ifdef __cplusplus
        bool operator==(const _TPM_EVENT_FIELD& i_rhs) const;
#endif
    } PACKED;
    typedef struct _TPM_EVENT_FIELD TPM_EVENT_FIELD;
    size_t TPM_EVENT_FIELD_marshalSize(const TPM_EVENT_FIELD* val);
    uint8_t* TPM_EVENT_FIELD_logMarshal(const TPM_EVENT_FIELD* val,
                                        uint8_t* i_logBuf);
    const uint8_t* TPM_EVENT_FIELD_logUnmarshal(TPM_EVENT_FIELD* val,
                                                const uint8_t* i_tpmBuf,
                                                bool* o_err);

    /// Crypto agile log entry format
    struct _TCG_PCR_EVENT2
    {
        uint32_t   pcrIndex;    ///< PCRIndex event extended to
        uint32_t   eventType;   ///< Type of event
        TPML_DIGEST_VALUES digests; ///< List of digests extended to PCRIndex
        TPM_EVENT_FIELD event;      ///< Event information
#ifdef __cplusplus
        bool operator==(const _TCG_PCR_EVENT2& i_rhs) const;
#endif
    } PACKED;
    typedef struct _TCG_PCR_EVENT2 TCG_PCR_EVENT2;
    uint8_t* TCG_PCR_EVENT2_logMarshal(const TCG_PCR_EVENT2* val,
                                       uint8_t* i_logBuf);
    const uint8_t* TCG_PCR_EVENT2_logUnmarshal(TCG_PCR_EVENT2* val,
                                               const uint8_t* i_tpmBuf,
                                               size_t i_bufSize, bool* o_err);
    size_t TCG_PCR_EVENT2_marshalSize(const TCG_PCR_EVENT2* val);

    struct _TPM2_BaseIn
    {
        uint16_t      tag;         ///< Type TPM_ST_xx
        uint32_t      commandSize; ///< Total # output bytes incl cmdSize & tag
        uint32_t      commandCode; ///< Type TPM_CC_xx
    } PACKED;
    typedef struct _TPM2_BaseIn TPM2_BaseIn;
    uint8_t* TPM2_BaseIn_marshal(const TPM2_BaseIn* val, uint8_t* o_tpmBuf,
                                 size_t i_tpmBufSize, size_t* io_cmdSize);

    /// Base of all outgoing messages
    struct _TPM2_BaseOut
    {
        uint16_t      tag;          ///< Type TPM_ST_xx
        uint32_t      responseSize; ///< Total # out bytes incl paramSize & tag
        uint32_t      responseCode; ///< The return code of the operation
    } PACKED;
    typedef struct _TPM2_BaseOut TPM2_BaseOut;
    const uint8_t* TPM2_BaseOut_unmarshal(TPM2_BaseOut* val,
                                    const uint8_t* i_tpmBuf,
                                    size_t* io_tpmBufSize, size_t i_outBufSize);

    /// Generic TPM Input Command structure with a 2 byte param
    struct _TPM2_2ByteIn
    {
        TPM2_BaseIn  base;
        uint16_t     param;
    } PACKED;
    typedef struct _TPM2_2ByteIn TPM2_2ByteIn;
    uint8_t* TPM2_2ByteIn_marshal(const TPM2_2ByteIn* val, uint8_t* o_tpmBuf,
                                  size_t i_tpmBufSize, size_t* io_cmdSize);

    /// Generic TPM Input Command structure with a 4 byte param
    struct _TPM2_4ByteIn
    {
        TPM2_BaseIn  base;
        uint32_t     param;
    } PACKED;
    typedef struct _TPM2_4ByteIn TPM2_4ByteIn;
    uint8_t* TPM2_4ByteIn_marshal(const TPM2_4ByteIn* val, uint8_t* o_tpmBuf,
                                  size_t i_tpmBufSize, size_t* io_cmdSize);


    /// Generic TPM Output Command structure with a 4 byte return data
    struct _TPM2_4ByteOut
    {
        TPM2_BaseOut base;
        uint32_t     resp;
    } PACKED;
    typedef struct _TPM2_4ByteOut TPM2_4ByteOut;

    /// Incoming TPM_GetCapability structure
    struct _TPM2_GetCapabilityIn
    {
        TPM2_BaseIn  base;
        uint32_t    capability;      ///< group selection
        uint32_t    property;        ///< Further definition
        uint32_t    propertyCount;   ///< Number of properties to return
    } PACKED;
    typedef struct _TPM2_GetCapabilityIn TPM2_GetCapabilityIn;
    uint8_t* TPM2_GetCapabilityIn_marshal(const TPM2_GetCapabilityIn* val,
                                          uint8_t* o_tpmBuf,
                                          size_t i_tpmBufSize,
                                          size_t* io_cmdSize);

    /// Outgoing TPM_GetCapability structure
    struct _TPM2_GetCapabilityOut
    {
        TPM2_BaseOut base;
        uint8_t     moreData;    ///< Flag to indicate if more values available
        TPMS_CAPABILITY_DATA capData; ///< The capability response
    } PACKED;
    typedef struct _TPM2_GetCapabilityOut TPM2_GetCapabilityOut;
    const uint8_t* TPM2_GetCapabilityOut_unmarshal(TPM2_GetCapabilityOut* val,
                                             const uint8_t* i_tpmBuf,
                                             size_t* io_tpmBufSize,
                                             size_t i_outBufSize);

    /// Incoming TPM_EXTEND structure
    struct _TPM2_ExtendIn
    {
        TPM2_BaseIn  base;
        uint32_t     pcrHandle;     ///< PCR number to extend
        TPML_DIGEST_VALUES digests; ///< Values to be extended
    } PACKED;
    typedef struct _TPM2_ExtendIn TPM2_ExtendIn;
    uint8_t* TPM2_ExtendIn_marshalHandle(const TPM2_ExtendIn* val,
                                         uint8_t* o_tpmBuf, size_t i_tpmBufSize,
                                         size_t* io_cmdSize);
    uint8_t* TPM2_ExtendIn_marshalParms(const TPM2_ExtendIn* val,
                                        uint8_t* o_tpmBuf, size_t i_tpmBufSize,
                                        size_t* io_cmdSize);

    struct _TPMS_PCR_SELECTION
    {
        uint16_t   algorithmId; ///< ID of hashing algorithm
        uint8_t    sizeOfSelect; ///< Byte size of pcrSelect array
        uint8_t    pcrSelect[PCR_SELECT_MAX];
    } PACKED;
    typedef struct _TPMS_PCR_SELECTION TPMS_PCR_SELECTION;
    uint8_t* TPMS_PCR_SELECTION_marshal(const TPMS_PCR_SELECTION* val,
                                        uint8_t* o_tpmBuf, size_t i_tpmBufSize,
                                        size_t* io_cmdSize);
    const uint8_t* TPMS_PCR_SELECTION_unmarshal(TPMS_PCR_SELECTION* val,
                                          const uint8_t* i_tpmBuf,
                                          size_t* io_tpmBufSize);


    struct _TPM2B_DIGEST
    {
        uint16_t size;
        uint8_t  buffer[sizeof(TPMU_HA)];
    } PACKED;
    typedef struct _TPM2B_DIGEST TPM2B_DIGEST;
    const uint8_t* TPM2B_DIGEST_unmarshal(TPM2B_DIGEST* val,
                                    const uint8_t* i_tpmBuf,
                                    size_t* io_tpmBufSize);

    struct _TPML_DIGEST
    {
        uint32_t  count;
        TPM2B_DIGEST digests[HASH_COUNT];
    } PACKED;
    typedef struct _TPML_DIGEST TPML_DIGEST;
    const uint8_t* TPML_DIGEST_unmarshal(TPML_DIGEST* val,
                                   const uint8_t* i_tpmBuf,
                                   size_t* io_tpmBufSize);

    struct _TPML_PCR_SELECTION
    {
        uint32_t     count;
        TPMS_PCR_SELECTION pcrSelections[HASH_COUNT];
    } PACKED;
    typedef struct _TPML_PCR_SELECTION TPML_PCR_SELECTION;
    uint8_t* TPML_PCR_SELECTION_marshal(const TPML_PCR_SELECTION* val,
                                        uint8_t* o_tpmBuf, size_t i_tpmBufSize,
                                        size_t* io_cmdSize);
    const uint8_t* TPML_PCR_SELECTION_unmarshal(TPML_PCR_SELECTION* val,
                                          const uint8_t* i_tpmBuf,
                                          size_t* io_tpmBufSize);

    /// Incoming PCR_Read structure
    struct _TPM2_PcrReadIn
    {
        TPM2_BaseIn  base;
        TPML_PCR_SELECTION pcrSelectionIn;
    } PACKED;
    typedef struct _TPM2_PcrReadIn TPM2_PcrReadIn;
    uint8_t* TPM2_PcrReadIn_marshal(const TPM2_PcrReadIn* val,
                                    uint8_t* o_tpmBuf, size_t i_tpmBufSize,
                                    size_t* io_cmdSize);

    /// Outgoing Pcr_Read structure
    struct _TPM2_PcrReadOut
    {
        TPM2_BaseOut base;
        uint32_t     pcrUpdateCounter;
        TPML_PCR_SELECTION pcrSelectionOut;
        TPML_DIGEST  pcrValues;
    } PACKED;
    typedef struct _TPM2_PcrReadOut TPM2_PcrReadOut;
    const uint8_t* TPM2_PcrReadOut_unmarshal(TPM2_PcrReadOut* val,
                                       const uint8_t* i_tpmBuf,
                                       size_t* io_tpmBufSize,
                                       size_t i_outBufSize);

    /// TPM Authorization structure
    ///  This is not the full structure and only works for PW auth with NULL PW
    struct _TPMS_AUTH_COMMAND
    {
        uint32_t    sessionHandle;
        uint16_t    nonceSize;        ///< Size of nonce structure, currently 0
        uint8_t     sessionAttributes; ///< Session attributes
        uint16_t    hmacSize;         ///< Size of hmac structure, currently 0
    } PACKED;
    typedef struct _TPMS_AUTH_COMMAND TPMS_AUTH_COMMAND;
    uint8_t* TPMS_AUTH_COMMAND_marshal(const TPMS_AUTH_COMMAND* val,
                                       uint8_t* o_tpmBuf, size_t i_tpmBufSize,
                                       size_t* io_cmdSize);

#ifdef __cplusplus
} // end TRUSTEDBOOT namespace
#endif

#endif

