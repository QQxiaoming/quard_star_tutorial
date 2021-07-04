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

#ifndef __TPM_H
#define __TPM_H

#include <device.h>

#include "tss/tpmLogMgr.H"
#include "tss/trustedTypes.H"

struct tpm_dev {

	/* TPM bus id */
	int bus_id;

	/* TPM address in the bus */
	int i2c_addr;
};

struct tpm_driver {

	/* Driver name */
	const char* name;

	/* Transmit the TPM command stored in buf to the tpm device */
	int (*transmit)(struct tpm_dev *dev, uint8_t* buf, size_t cmdlen,
			size_t *buflen);
};

struct tpm_chip {

	/* TPM chip id */
	int id;

	/* Indicates whether or not the device and log are functional */
	bool enabled;

	/* TPM device tree node */
	struct dt_node *node;

	/* Event log handler */
	struct _TpmLogMgr logmgr;

	/* TPM device handler */
	struct tpm_dev    *dev;

	/* TPM driver handler */
	struct tpm_driver *driver;

	struct list_node link;
};

/* TSS tweak */
typedef struct tpm_chip TpmTarget;

/*
 * Register a tpm chip by binding the driver to dev.
 * Event log is also registered by this function.
 */
extern int tpm_register_chip(struct dt_node *node, struct tpm_dev *dev,
			     struct tpm_driver *driver);

/*
 * tpm_extendl - For each TPM device, this extends the sha1 and sha 256 digests
 * to the indicated PCR and also records an event for the same PCR
 * in the event log
 * This calls a TSS extend function that supports multibank. Both sha1 and
 * sha256 digests are extended in a single operation sent to the TPM device.
 *
 * @pcr: PCR number to be extended and recorded in the event log. The same PCR
 * number is extende for both sha1 and sha256 banks.
 * @alg1: SHA algorithm of digest1. Either TPM_ALG_SHA1 or TPM_ALG_SHA256
 * @digest1: digest1 buffer
 * @size1: size of digest1. Either TPM_ALG_SHA1_SIZE or TPM_ALG_SHA256_SIZE
 * @alg2: SHA algorithm of digest2. Either TPM_ALG_SHA1 or TPM_ALG_SHA256
 * @digest2: digest2 buffer
 * @size2: size of digest2. Either TPM_ALG_SHA1_SIZE or TPM_ALG_SHA256_SIZE
 * @event_type: event type log. In skiboot, either EV_ACTION or EV_SEPARATOR.
 * @event_msg: event log message that describes the event
 *
 * Returns O for success or a negative number if it fails.
 */
extern int tpm_extendl(TPM_Pcr pcr,
		       TPM_Alg_Id alg1, uint8_t* digest1, size_t size1,
		       TPM_Alg_Id alg2, uint8_t* digest2, size_t size2,
		       uint32_t event_type, const char* event_msg);

/* Add status property to the TPM devices */
extern void tpm_add_status_property(void);

extern int tpm_init(void);
extern void tpm_cleanup(void);

#endif /* __TPM_H */
