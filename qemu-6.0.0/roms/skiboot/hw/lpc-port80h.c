/* Copyright 2018 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define pr_fmt(fmt)	"Port80h: " fmt

#include <lpc.h>
#include <op-panel.h>
#include <chip.h>

/*
 * Convert our detailed op_display() call into 1 byte for LPC port 80h
 *
 * Our layout looks like this:
 * MSB (bit 7): 1 = Comes from OPAL
 *      bit 6 : 0 = OP_MOD_INIT (the main one), 1 = (see bit 5)
 *      bit 5432 : (if bit 6=0, low nibble of op-panel code)
 *      bit 5432 : (if bit 6=1, other OP_MOD_ values in bits 54:
 *                              00b=OP_MOD_CPU, 01b=OP_MOD_LOCK,
 *                              10b=OP_MOD_MEM, 11b=OP_MOD_CHIPTOD
 *                  bits 0,1 from code in bits 32)
 *
 *      bit 1,0: 00b=OP_LOG, 10b=OP_WARN, 01b=OP_ERROR, 11b=OP_FATAL
 *               i.e. bit 0 indicates ERROR or FATAL.
 *
 * If port 80h number has the MSB and LSB set, then you died in OPAL.
 * Any *odd* number with the MSB set (i.e. > 0x80) indicates error.
 */
static inline uint8_t op_display_to_port80(uint8_t last_value, enum op_severity s, enum op_module m, uint16_t c)
{
	uint8_t r = 0x80; /* Start with top bit set indicating in OPAL */

	switch(m) {
	case OP_MOD_INIT:
		/* bit 6 is zero */
		/* bits 5432 have low nibble of c */
		r |= (c & 0x0f) << 2;
		break;
	case OP_MOD_CPU:
		r |= 0x40 | (c & 0x03) << 2;
		break;
	case OP_MOD_LOCK:
		r |= 0x50 | (c & 0x03) << 2;
		break;
	case OP_MOD_MEM:
		r |= 0x60 | (c & 0x03) << 2;
		break;
	case OP_MOD_CHIPTOD:
		r |= 0x70 | (c & 0x03) << 2;
		break;
	case OP_MOD_CORE:
		/*
		 * Only current OP_MOD_CORE is where we're OP_FATAL,
		 * So let's go for the last value set and tweak the
		 * bits for OP_FATAL.
		 */
		r = last_value & 0xFC;
		break;
	case OP_MOD_FSP:
	case OP_MOD_FSPCON:
		/* Should never be hit, port80h only used on non-FSP! */
		break;
	}

	switch(s) {
	case OP_LOG:
		break;
	case OP_WARN:
		r |= 0x02;
		break;
	case OP_ERROR:
		r |= 0x01;
		break;
	case OP_FATAL:
		r |= 0x03;
	}

	return r;
}

/*
 * Convert our detailed op_display() call into 2 bytes for LPC port 81h and 82h
 *
 * This looks pretty similar to our port80 code.
 * Notably we now have more bits to throw progress into.
 *
 * Our layout looks like this:
 * MSB (bit 15): 1 = Comes from OPAL
 *      bit 14 : 0 = OP_MOD_INIT (the main one), 1 = (see bit 13)
 *      bits 13-2 : (if bit 6=0, low 12 bits of op-panel code)
 *      bit 13,12 : (if bit 6=1, other OP_MOD_ values in bits 13 and 12:
 *                              00b=OP_MOD_CPU, 01b=OP_MOD_LOCK,
 *                              10b=OP_MOD_MEM, 11b=OP_MOD_CHIPTOD)
 *                   and bits 11-2 are low 10 bits of op-panel code)
 *
 *      bit 1,0: 00b=OP_LOG, 10b=OP_WARN, 01b=OP_ERROR, 11b=OP_FATAL
 *               i.e. bit 0 indicates ERROR or FATAL.
 *
 * If port 80h number has the MSB and LSB set, then you died in OPAL.
 * Any *odd* number with the MSB set (i.e. > 0x80) indicates error.
 */
static inline uint16_t op_display_to_port8x(uint16_t last_value, enum op_severity s, enum op_module m, uint16_t c)
{
	uint16_t r = 0x8000; /* Start with top bit set indicating in OPAL */

	switch(m) {
	case OP_MOD_INIT:
		/* bit 6 is zero */
		/* bits 13 through 2 have low 12 bits of c */
		r |= (c & 0xFFF) << 2;
		break;
	case OP_MOD_CPU:
		r |= 0x4000 | (c & 0x03FF) << 2;
		break;
	case OP_MOD_LOCK:
		r |= 0x5000 | (c & 0x03FF) << 2;
		break;
	case OP_MOD_MEM:
		r |= 0x6000 | (c & 0x03FF) << 2;
		break;
	case OP_MOD_CHIPTOD:
		r |= 0x7000 | (c & 0x03FF) << 2;
		break;
	case OP_MOD_CORE:
		/*
		 * Only current OP_MOD_CORE is where we're OP_FATAL,
		 * So let's go for the last value set and tweak the
		 * bits for OP_FATAL.
		 */
		r = last_value & 0xFFFC;
		break;
	case OP_MOD_FSP:
	case OP_MOD_FSPCON:
		/* Should never be hit, port80h only used on non-FSP! */
		break;
	}

	switch(s) {
	case OP_LOG:
		break;
	case OP_WARN:
		r |= 0x02;
		break;
	case OP_ERROR:
		r |= 0x01;
		break;
	case OP_FATAL:
		r |= 0x03;
	}

	return r;
}


void op_display_lpc(enum op_severity s, enum op_module m, uint16_t c)
{
	static uint8_t port80_val = 0x80;
	static uint16_t port8x_val = 0x8000;

	if (chip_quirk(QUIRK_SIMICS))
		return;

	port80_val = op_display_to_port80(port80_val, s, m, c);
	lpc_outb(port80_val, 0x80);
	port8x_val = op_display_to_port8x(port8x_val, s, m, c);
	lpc_outb(port8x_val >> 8, 0x81);
	lpc_outb(port8x_val & 0xFF, 0x82);
}

