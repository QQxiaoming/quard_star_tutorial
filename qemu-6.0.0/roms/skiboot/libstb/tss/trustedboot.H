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

/***************************************************************
 * This file maps some routines and types from TSS and tpmLogMgr
 * codes to equivalent routines and types in skiboot.
 ***************************************************************/

#ifndef __TSS_TRUSTEDBOOT_H
#define __TSS_TRUSTEDBOOT_H

#include <skiboot.h>
#include <stdint.h>
#include <stdio.h>
#include <lock.h>

/* Failure trace routines */
#define TRACFCOMP(TB, fmt, ...) prlog(PR_ERR, "TSS: " fmt "\n", ##__VA_ARGS__);
//#define TRACFCOMP(args...)

/* Debug trace routines */
//#define TRACDCOMP(TB, fmt, ...) prlog(PR_DEBUG, "TSS: " fmt "\n", ##__VA_ARGS__)
#define TRACDCOMP(args...)

//#define TRACUCOMP(TB, fmt, ...) prlog(PR_DEBUG, "TSS: " fmt "\n", ##__VA_ARGS__);
#define TRACUCOMP(args...)

//#define TRACUBIN(TB, fmt, ...) prlog(PR_DEBUG, "TSS: " fmt "\n", ##__VA_ARGS__);
#define TRACUBIN(args...)

#define g_trac_trustedboot NULL
#define g_trac_tpmdd NULL
typedef uint32_t errlHndl_t;
#define TB_SUCCESS 0
#define htole32(val) cpu_to_le32(val)
#define le32toh(val) le32_to_cpu(val)
#define le16toh(val) le16_to_cpu(val)
#define htole16(val) cpu_to_le16(val)

#define mutex_init(mutex) init_lock(mutex)
#define mutex_lock(mutex) lock(mutex)
#define mutex_unlock(mutex) unlock(mutex)
#define mutex_t struct lock

typedef enum {
    PCR_0 = 0,
    PCR_1 = 1,
    PCR_2 = 2,
    PCR_3 = 3,
    PCR_4 = 4,
    PCR_5 = 5,
    PCR_6 = 6,
    PCR_7 = 7,
        PCR_DEBUG = 16,
        PCR_DRTM_17 = 17,
        PCR_DRTM_18 = 18,
        PCR_DRTM_19 = 19,
        PCR_DRTM_20 = 20,
        PCR_DRTM_21 = 21,
        PCR_DRTM_22 = 22,
        PLATFORM_PCR = 24, ///< The number of PCR required by the platform spec
        IMPLEMENTATION_PCR = 24, ///< The number of PCRs implemented by TPM
} TPM_Pcr;

#endif /* __TSS_TRUSTEDBOOT_H */
