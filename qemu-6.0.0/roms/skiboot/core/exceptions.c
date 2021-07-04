/* Copyright 2013-2014 IBM Corp.
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

#include <skiboot.h>
#include <stack.h>
#include <opal.h>
#include <processor.h>
#include <cpu.h>

#define REG		"%016llx"
#define REG32		"%08x"
#define REGS_PER_LINE	4

static void dump_regs(struct stack_frame *stack)
{
	unsigned int i;

	prerror("CFAR : "REG" MSR  : "REG"\n", stack->cfar, stack->msr);
	prerror("SRR0 : "REG" SRR1 : "REG"\n", stack->srr0, stack->srr1);
	prerror("HSRR0: "REG" HSRR1: "REG"\n", stack->hsrr0, stack->hsrr1);
	prerror("DSISR: "REG32"         DAR  : "REG"\n", stack->dsisr, stack->dar);
	prerror("LR   : "REG" CTR  : "REG"\n", stack->lr, stack->ctr);
	prerror("CR   : "REG32"         XER  : "REG32"\n", stack->cr, stack->xer);
	for (i = 0;  i < 16;  i++)
		prerror("GPR%02d: "REG" GPR%02d: "REG"\n",
		       i, stack->gpr[i], i + 16, stack->gpr[i + 16]);
}

#define EXCEPTION_MAX_STR 320

void exception_entry(struct stack_frame *stack)
{
	bool fatal = false;
	bool hv;
	uint64_t nip;
	uint64_t msr;
	char buf[EXCEPTION_MAX_STR];
	size_t l;

	switch (stack->type) {
	case 0x500:
	case 0x980:
	case 0xe00:
	case 0xe20:
	case 0xe40:
	case 0xe60:
	case 0xe80:
	case 0xea0:
	case 0xf80:
		hv = true;
		break;
	default:
		hv = false;
		break;
	}

	if (hv) {
		nip = stack->hsrr0;
		msr = stack->hsrr1;
	} else {
		nip = stack->srr0;
		msr = stack->srr1;
	}

	if (!(msr & MSR_RI))
		fatal = true;

	prerror("***********************************************\n");
	l = 0;
	if (stack->type == 0x100) {
		if (fatal) {
			l += snprintf(buf + l, EXCEPTION_MAX_STR - l,
				"Fatal System Reset at "REG"   ", nip);
		} else {
			l += snprintf(buf + l, EXCEPTION_MAX_STR - l,
				"System Reset at "REG"   ", nip);
		}
	} else if (stack->type == 0x200) {
		fatal = true;
		l += snprintf(buf + l, EXCEPTION_MAX_STR - l,
			"Fatal MCE at "REG"   ", nip);
	} else {
		fatal = true;
		l += snprintf(buf + l, EXCEPTION_MAX_STR - l,
			"Fatal Exception 0x%llx at "REG"  ", stack->type, nip);
	}
	l += snprintf_symbol(buf + l, EXCEPTION_MAX_STR - l, nip);
	l += snprintf(buf + l, EXCEPTION_MAX_STR - l, "  MSR "REG, msr);
	prerror("%s\n", buf);
	dump_regs(stack);

	if (fatal)
		abort();
	else
		backtrace();

	if (hv) {
		/* Set up for SRR return */
		stack->srr0 = nip;
		stack->srr1 = msr;
	}
}

void exception_entry_pm_sreset(void)
{
	char buf[EXCEPTION_MAX_STR];
	size_t l;

	prerror("***********************************************\n");
	l = 0;
	l += snprintf(buf + l, EXCEPTION_MAX_STR - l,
		"System Reset in sleep");
	prerror("%s\n", buf);
	backtrace();
}

void __noreturn exception_entry_pm_mce(void)
{
	char buf[EXCEPTION_MAX_STR];
	size_t l;

	prerror("***********************************************\n");
	l = 0;
	l += snprintf(buf + l, EXCEPTION_MAX_STR - l,
		"Fatal MCE in sleep");
	prerror("%s\n", buf);
	prerror("SRR0 : "REG" SRR1 : "REG"\n",
			(uint64_t)mfspr(SPR_SRR0), (uint64_t)mfspr(SPR_SRR1));
	prerror("DSISR: "REG32"         DAR  : "REG"\n",
			(uint32_t)mfspr(SPR_DSISR), (uint64_t)mfspr(SPR_DAR));
	abort();
}

static int64_t opal_register_exc_handler(uint64_t opal_exception __unused,
					 uint64_t handler_address __unused,
					 uint64_t glue_cache_line __unused)
{
	/* This interface is deprecated */
	return OPAL_UNSUPPORTED;
}
opal_call(OPAL_REGISTER_OPAL_EXCEPTION_HANDLER, opal_register_exc_handler, 3);

