/* Copyright 2013-2018 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __SBE_P8_H
#define __SBE_P8_H

/* P8 SBE update timer function */
extern void p8_sbe_update_timer_expiry(uint64_t new_target);

/* Is SBE timer available ? */
extern bool p8_sbe_timer_ok(void);

/* Initialize SBE timer */
extern void p8_sbe_init_timer(void);

#endif /* __SBE_P8_H */
