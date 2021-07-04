
/* Copyright 2019 IBM Corp.
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

#ifndef __DIO_H
#define __DIO_H

struct proc_chip;

/* Initialize the P9 DIO */
extern void p9_dio_init(void);

/* The function typedef for dio interrupt callback */
typedef void (*dio_interrupt_callback)(struct proc_chip *chip);

/* Register dio interrupt on GPIO port.
 * This effectively enables the DIO interrupt on the GPIO port,
 * and callback will be called when the interrupt is triggered */
extern int dio_interrupt_register(struct proc_chip *chip,
		int port, dio_interrupt_callback c);

/* Deregister dio interrupt on GPIO port.
 * This effectively disables the DIO interrupt on the GPIO port. */
extern int dio_interrupt_deregister(struct proc_chip *chip,
		int port, dio_interrupt_callback c);

/* The function to be called when DIO interrupt is triggered */
extern void dio_interrupt_handler(uint32_t chip_id);


#define NUM_OF_P9_DIO_PORTS	3 /* P9 has GPIO port 0~2 for interrupts */

struct p9_dio {
	dio_interrupt_callback callbacks[NUM_OF_P9_DIO_PORTS];
};

#endif	/* __DIO_H */
