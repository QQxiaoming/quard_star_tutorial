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

#ifndef __STB_STATUS_CODES_H
#define __STB_STATUS_CODES_H

/*  general return codes */
#define STB_ERROR		-1
#define STB_ARG_ERROR		-2
#define STB_DRIVER_ERROR	-3

/* secure boot */
#define STB_SECURE_MODE_DISABLED	 100
#define STB_VERIFY_FAILED  		-100

/* trusted boot */
#define STB_TRUSTED_MODE_DISABLED	 200
#define STB_MEASURE_FAILED		-200

/* TPM */
#define STB_NO_TPM_INITIALIZED	 300
#define STB_TPM_OVERFLOW	-300
#define STB_TPM_TIMEOUT	-301

#endif /* __STB_STATUS_CODES_H */
