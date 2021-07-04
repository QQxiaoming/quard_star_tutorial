/* Copyright 2013-2019 IBM Corp.
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
#include <cpu.h>
#include <console.h>
#include <fsp.h>
#include <psi.h>
#include <opal.h>
#include <mem_region.h>
#include <xscom.h>
#include <interrupts.h>
#include <cec.h>
#include <timebase.h>
#include <pci.h>
#include <xive.h>
#include <chip.h>
#include <chiptod.h>
#include <ipmi.h>
#include <direct-controls.h>
#include <nvram.h>

/* Flag tested by the OPAL entry code */
static volatile bool fast_boot_release;

static bool cpu_state_wait_all_others(enum cpu_thread_state state,
					unsigned long timeout_tb)
{
	struct cpu_thread *cpu;
	unsigned long end = mftb() + timeout_tb;

	sync();
	for_each_ungarded_cpu(cpu) {
		if (cpu == this_cpu())
			continue;

		if (cpu->state != state) {
			smt_lowest();
			while (cpu->state != state) {
				barrier();

				if (timeout_tb && (tb_compare(mftb(), end) == TB_AAFTERB)) {
					smt_medium();
					return false;
				}
			}
			smt_medium();
		}
	}
	sync();

	return true;
}

static const char *fast_reboot_disabled = NULL;

void disable_fast_reboot(const char *reason)
{
	if (fast_reboot_disabled)
		return;

	prlog(PR_NOTICE, "RESET: Fast reboot disabled: %s\n", reason);
	fast_reboot_disabled = reason;
}

void add_fast_reboot_dt_entries(void)
{
	dt_check_del_prop(opal_node, "fast-reboot");

	if (fast_reboot_disabled) {
		dt_add_property_string(opal_node, "fast-reboot", fast_reboot_disabled);
	} else {
		dt_add_property_string(opal_node, "fast-reboot", "okay");
	}
}

/*
 * This is called by the reboot CPU after all other CPUs have been
 * quiesced and stopped, to perform various sanity checks on firmware
 * data (and potentially hardware), to determine whether the fast
 * reboot should go ahead.
 */
static bool fast_reboot_sanity_check(void)
{
	if (!mem_check_all()) {
		disable_fast_reboot("Inconsistent firmware data");
		return false;
	}

	if (!verify_romem()) {
		disable_fast_reboot("Inconsistent firmware romem checksum");
		return false;
	}

	return true;
}

void fast_reboot(void)
{
	static int fast_reboot_count = 0;

	if (!chip_quirk(QUIRK_MAMBO_CALLOUTS) &&
			(proc_gen != proc_gen_p8 && proc_gen != proc_gen_p9)) {
		prlog(PR_DEBUG,
		      "RESET: Fast reboot not available on this CPU\n");
		return;
	}
	if (chip_quirk(QUIRK_NO_DIRECT_CTL)) {
		prlog(PR_DEBUG,
		      "RESET: Fast reboot disabled by quirk\n");
		return;
	}

	/*
	 * Ensure all other CPUs have left OPAL calls.
	 */
	if (!opal_quiesce(QUIESCE_HOLD, -1)) {
		disable_fast_reboot("OPAL quiesce timeout");
		return;
	}

	if (fast_reboot_disabled &&
	    nvram_query_eq_dangerous("fast-reset", "im-feeling-lucky")) {
		/* Do fast reboot even if it's been disabled */
		prlog(PR_NOTICE, "RESET: Ignoring fast reboot disabled: %s\n",
				fast_reboot_disabled);
	} else if (fast_reboot_disabled) {
		prlog(PR_NOTICE, "RESET: Fast reboot disabled: %s\n",
		      fast_reboot_disabled);
		opal_quiesce(QUIESCE_RESUME, -1);
		return;
	}

	prlog(PR_NOTICE, "RESET: Initiating fast reboot %d...\n", ++fast_reboot_count);
	fast_boot_release = false;
	sync();

	/* Put everybody in stop except myself */
	if (sreset_all_prepare()) {
		prlog(PR_NOTICE, "RESET: Fast reboot failed to prepare "
				"secondaries for system reset\n");
		opal_quiesce(QUIESCE_RESUME, -1);
		return;
	}

	if (!fast_reboot_sanity_check()) {
		opal_quiesce(QUIESCE_RESUME, -1);
		return;
	}

	cpu_set_sreset_enable(false);
	cpu_set_ipi_enable(false);

	/*
	 * There is no point clearing special wakeup or un-quiesce due to
	 * failure after this point, because we will be going to full IPL.
	 * Less cleanup work means less opportunity to fail.
	 */

	/*
	 * Move SPRs and exception vectors back to OPAL-mode while all
	 * others are quiesced. MSR[ME] is disabled while these are switched,
	 * but system reset can not be blocked -- in theory an sreset coming
	 * from the BMC or SPE could crash here.
	 */
	disable_machine_check();

	/*
	 * Primarily we want to fix up the HID bits here to allow the OPAL
	 * exception handlers to work. Machine check would be the important
	 * one.
	 *
	 * This is one case where a thread other than thread0 of the core
	 * may update the shared SPRs. All other threads are stopped, so
	 * there should be no races.
	 */
	init_shared_sprs();
	init_replicated_sprs();

	/* Restore skiboot vectors  */
	copy_exception_vectors();

	/*
	 * Secondaries may still have an issue with machine checks if they have
	 * HILE set because the machine check exception does not FIXUP_ENDIAN.
	 * Adding that would trash CFAR however. So we have a window where
	 * if a secondary takes an interrupt before the HILE is fixed, it will
	 * crash.
	 */
	enable_machine_check();
	mtmsrd(MSR_RI, 1);

	/*
	 * sreset vector has a FIXUP_ENDIAN sequence at the start, so
	 * secondaries can cope.
	 */
	copy_sreset_vector_fast_reboot();

	/* Send everyone else to 0x100 */
	if (sreset_all_others() != OPAL_SUCCESS) {
		prlog(PR_NOTICE, "RESET: Fast reboot failed to system reset "
				"secondaries\n");
		return;
	}

	/* Ensure all the sresets get through */
	if (!cpu_state_wait_all_others(cpu_state_present, msecs_to_tb(1000))) {
		prlog(PR_NOTICE, "RESET: Fast reboot timed out waiting for "
				"secondaries to call in\n");
		return;
	}

	prlog(PR_DEBUG, "RESET: Releasing special wakeups...\n");
	sreset_all_finish();

	/* This resets our quiesce state ready to enter the new kernel. */
	opal_quiesce(QUIESCE_RESUME_FAST_REBOOT, -1);

	console_complete_flush();

	asm volatile("ba	0x100\n\t" : : : "memory");
	for (;;)
		;
}

static void cleanup_cpu_state(void)
{
	struct cpu_thread *cpu = this_cpu();

	/* Per core cleanup */
	if (cpu_is_thread0(cpu)) {
		/* Shared SPRs whacked back to normal */

		/* XXX Update the SLW copies ! Also dbl check HIDs etc... */
		init_shared_sprs();

		if (proc_gen == proc_gen_p8) {
			/* If somebody was in fast_sleep, we may have a
			 * workaround to undo
			 */
			if (cpu->in_fast_sleep) {
				prlog(PR_DEBUG, "RESET: CPU 0x%04x in fast sleep"
				      " undoing workarounds...\n", cpu->pir);
				fast_sleep_exit();
			}

			/* The TLB surely contains garbage.
			 * P9 clears TLBs in cpu_fast_reboot_complete
			 */
			cleanup_local_tlb();
		}

		/* And we might have lost TB sync */
		chiptod_wakeup_resync();
	}

	/* Per-thread additional cleanup */
	init_replicated_sprs();

	// XXX Cleanup SLW, check HIDs ...
}

void __noreturn enter_nap(void);

static void check_split_core(void)
{
	struct cpu_thread *cpu;
	u64 mask, hid0;

        hid0 = mfspr(SPR_HID0);
	mask = SPR_HID0_POWER8_4LPARMODE | SPR_HID0_POWER8_2LPARMODE;

	if ((hid0 & mask) == 0)
		return;

	prlog(PR_INFO, "RESET: CPU 0x%04x is split !\n", this_cpu()->pir);

	/* If it's a secondary thread, just send it to nap */
	if (this_cpu()->pir & 7) {
		/* Prepare to be woken up */
		icp_prep_for_pm();
		/* Setup LPCR to wakeup on external interrupts only */
		mtspr(SPR_LPCR, ((mfspr(SPR_LPCR) & ~SPR_LPCR_P8_PECE) |
				 SPR_LPCR_P8_PECE2));
		isync();
		/* Go to nap (doesn't return) */
		enter_nap();
	}

	prlog(PR_INFO, "RESET: Primary, unsplitting... \n");

	/* Trigger unsplit operation and update SLW image */
	hid0 &= ~SPR_HID0_POWER8_DYNLPARDIS;
	set_hid0(hid0);
	opal_slw_set_reg(this_cpu()->pir, SPR_HID0, hid0);

	/* Wait for unsplit */
	while (mfspr(SPR_HID0) & mask)
		cpu_relax();

	/* Now the guys are sleeping, wake'em up. They will come back
	 * via reset and continue the fast reboot process normally.
	 * No need to wait.
	 */
	prlog(PR_INFO, "RESET: Waking unsplit secondaries... \n");

	for_each_cpu(cpu) {
		if (!cpu_is_sibling(cpu, this_cpu()) || (cpu == this_cpu()))
			continue;
		icp_kick_cpu(cpu);
	}
}


/* Entry from asm after a fast reset */
void __noreturn fast_reboot_entry(void);

void __noreturn fast_reboot_entry(void)
{
	prlog(PR_DEBUG, "RESET: CPU 0x%04x reset in\n", this_cpu()->pir);

	if (proc_gen == proc_gen_p9) {
		reset_cpu_xive();
	} else if (proc_gen == proc_gen_p8) {
		/* We reset our ICP first ! Otherwise we might get stray
		 * interrupts when unsplitting
		 */
		reset_cpu_icp();

		/* If we are split, we need to unsplit. Since that can send us
		 * to NAP, which will come back via reset, we do it now
		 */
		check_split_core();
	}

	sync();
	this_cpu()->state = cpu_state_present;
	sync();

	/* Are we the original boot CPU ? If not, we spin waiting
	 * for a relase signal from CPU 1, then we clean ourselves
	 * up and go processing jobs.
	 */
	if (this_cpu() != boot_cpu) {
		if (!fast_boot_release) {
			smt_lowest();
			while (!fast_boot_release)
				barrier();
			smt_medium();
		}
		sync();
		cleanup_cpu_state();
		enable_machine_check();
		mtmsrd(MSR_RI, 1);

		__secondary_cpu_entry();
	}

	prlog(PR_INFO, "RESET: Boot CPU waiting for everybody...\n");

	/* We are the original boot CPU, wait for secondaries to
	 * be captured.
	 */
	cpu_state_wait_all_others(cpu_state_present, 0);

	if (proc_gen == proc_gen_p9)
		xive_reset();

	prlog(PR_INFO, "RESET: Releasing secondaries...\n");

	/* Release everybody */
	sync();
	fast_boot_release = true;

	/* Cleanup ourselves */
	cleanup_cpu_state();

	/* Set our state to active */
	sync();
	this_cpu()->state = cpu_state_active;
	sync();

	/* Wait for them to respond */
	cpu_state_wait_all_others(cpu_state_active, 0);

	sync();

	prlog(PR_INFO, "RESET: All done, cleaning up...\n");

	/* Clear release flag for next time */
	fast_boot_release = false;

	/* Let the CPU layer do some last minute global cleanups */
	cpu_fast_reboot_complete();

	/* Restore OPAL's sreset vector now that all CPUs have HILE clear */
	copy_sreset_vector();

	/* We can now do NAP mode */
	cpu_set_sreset_enable(true);
	cpu_set_ipi_enable(true);

	if (!chip_quirk(QUIRK_MAMBO_CALLOUTS)) {
		/*
		 * mem_region_clear_unused avoids these preload regions
		 * so it can run along side image preloading. Clear these
		 * regions now to catch anything not overwritten by
		 * preload.
		 *
		 * Mambo may have embedded payload here, so don't clear
		 * it at all.
		 */
		memset(KERNEL_LOAD_BASE, 0, KERNEL_LOAD_SIZE);
		memset(INITRAMFS_LOAD_BASE, 0, INITRAMFS_LOAD_SIZE);
	}

	/* Start preloading kernel and ramdisk */
	start_preload_kernel();

	/* Start clearing memory */
	start_mem_region_clear_unused();

	if (platform.fast_reboot_init)
		platform.fast_reboot_init();

	if (proc_gen == proc_gen_p8) {
		/* XXX */
		/* Reset/EOI the PSI interrupt */
		psi_irq_reset();
	}

	/* Remove all PCI devices */
	if (pci_reset()) {
		prlog(PR_NOTICE, "RESET: Fast reboot failed to reset PCI\n");

		/*
		 * Can't return to caller here because we're past no-return.
		 * Attempt an IPL here which is what the caller would do.
		 */
		if (platform.cec_reboot)
			platform.cec_reboot();
		for (;;)
			;
	}

	ipmi_set_fw_progress_sensor(IPMI_FW_PCI_INIT);

	wait_mem_region_clear_unused();

	/* Load and boot payload */
	load_and_boot_kernel(true);
}
