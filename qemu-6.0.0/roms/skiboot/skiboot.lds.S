/* Copyright 2013-2018 IBM Corp.
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

#include <config.h>
#include <mem-map.h>

/* Debug sections from binutils `ld --verbose` */

#define DEBUG_SECTIONS							\
  /* Stabs debugging sections.  */					\
  .stab          0 : { *(.stab) }					\
  .stabstr       0 : { *(.stabstr) }					\
  .stab.excl     0 : { *(.stab.excl) }					\
  .stab.exclstr  0 : { *(.stab.exclstr) }				\
  .stab.index    0 : { *(.stab.index) }					\
  .stab.indexstr 0 : { *(.stab.indexstr) }				\
  .comment       0 : { *(.comment) }					\
  /* DWARF debug sections.						\
     Symbols in the DWARF debugging sections are relative to the beginning \
     of the section so we begin them at 0.  */				\
  /* DWARF 1 */								\
  .debug          0 : { *(.debug) }					\
  .line           0 : { *(.line) }					\
  /* GNU DWARF 1 extensions */						\
  .debug_srcinfo  0 : { *(.debug_srcinfo) }				\
  .debug_sfnames  0 : { *(.debug_sfnames) }				\
  /* DWARF 1.1 and DWARF 2 */						\
  .debug_aranges  0 : { *(.debug_aranges) }				\
  .debug_pubnames 0 : { *(.debug_pubnames) }				\
  /* DWARF 2 */								\
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }		\
  .debug_abbrev   0 : { *(.debug_abbrev) }				\
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }	\
  .debug_frame    0 : { *(.debug_frame) }				\
  .debug_str      0 : { *(.debug_str) }					\
  .debug_loc      0 : { *(.debug_loc) }					\
  .debug_macinfo  0 : { *(.debug_macinfo) }				\
  /* SGI/MIPS DWARF 2 extensions */					\
  .debug_weaknames 0 : { *(.debug_weaknames) }				\
  .debug_funcnames 0 : { *(.debug_funcnames) }				\
  .debug_typenames 0 : { *(.debug_typenames) }				\
  .debug_varnames  0 : { *(.debug_varnames) }				\
  /* DWARF 3 */								\
  .debug_pubtypes 0 : { *(.debug_pubtypes) }				\
  .debug_ranges   0 : { *(.debug_ranges) }				\
  /* DWARF Extension.  */						\
  .debug_macro    0 : { *(.debug_macro) }				\
  .debug_addr     0 : { *(.debug_addr) }

ENTRY(boot_entry);
SECTIONS
{
	_start = .;
	. = 0;

	.head : {
		KEEP(*(.head))
	}

	. = SPIRA_OFF;
	.spira : {
		KEEP(*(.spira.data))
	}

	. = SPIRAH_OFF;
	.spirah : {
		KEEP(*(.spirah.data))
	}

	. = PROCIN_OFF;
	.procin.data : {
		KEEP(*(.procin.data))
	}

	. = MDST_TABLE_OFF;
	.mdst : {
		KEEP(*(.mdst.data))
	}

	. = CPU_CTL_OFF;
	.cpuctrl : {
		KEEP(*(.cpuctrl.data))
	}

	. = ALIGN(0x10);
	_stext = .;
 	.text : {
		*(.text*)
		*(.sfpr .glink)
	}
	_etext = .;

	.rodata : {
		__rodata_start = .;
		*(.rodata .rodata.*)
		__rodata_end = .;
	}

	. = ALIGN(0x10);
	.init : {
		__ctors_start = .;
		KEEP(*(.ctors*))
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		__ctors_end = .;
	}

	. = ALIGN(0x10);
	.opd : {
		*(.opd)
	}
  
	. = ALIGN(0x100);
	.got : {
		__toc_start = . + 0x8000;
		*(.got)
		*(.toc)
	}

	. = ALIGN(0x10);
	.opal_table : {
		__opal_table_start = .;
		KEEP(*(.opal_table))
		__opal_table_end = .;
	}

	.platforms : {
		__platforms_start = .;
		KEEP(*(.platforms))
		__platforms_end = .;
	}

	/* Do I need to keep these ? */
	.dynsym : { *(.dynsym)	}
	.dynstr : { *(.dynstr)	}

	/* Relocations */
	. = ALIGN(0x10);
	.dynamic : {
		__dynamic_start = .;
		*(.dynamic)
		__dynamic_end = .;
	}

	. = ALIGN(0x10);
	.rela.dyn : {
		__rela_dyn_start = .;
		*(.rela*)
		__rela_dyn_end = .;
	}
	.plt    : { *(.plt) *(.iplt) }

	.hash          : { *(.hash)   }
	.gnu.hash      : { *(.gnu.hash) }
	.dynsym        : { *(.dynsym) }
	.dynstr        : { *(.dynstr) }
	.gnu.version   : { *(.gnu.version) }
	.gnu.version_d : { *(.gnu.version_d) }
	.gnu.version_r : { *(.gnu.version_r) }

	. = ALIGN(0x10);
	.sym_map : {
		__sym_map_start = . ;
		KEEP(*(.sym_map))
		__sym_map_end = . ;
	}

	/*
	 * Memory above this point should be un-changing after the OS
	 * boots, and is verified with a checksum upon fast reboot.
	 */
	_romem_end = .;

	.data : {
		/*
		 * A couple of things that need to be 4K aligned and
		 * to reside in their own pages for the sake of TCE
		 * mappings
		 */
		. = ALIGN(0x1000);
		*(.data.memcons);
		. = ALIGN(0x10000);
		*(.data.boot_trace);
		. = ALIGN(0x10000);
		*(.data*)
		*(.force.data)
		*(.toc1)
		*(.branch_lt)
	}

	/* We locate the BSS at 4M to leave room for the symbol map */
	. = 0x400000;

	_sbss = .;
	.bss : {
		*(.dynbss)
		*(.bss*)
	}
	. = ALIGN(0x10000);
	_ebss = .;
	_end = .;

	DEBUG_SECTIONS

	/* Optional kernel image */
       . = ALIGN(0x10000);
       .builtin_kernel : {
		__builtin_kernel_start = .;
		KEEP(*(.builtin_kernel))
		__builtin_kernel_end = .;
	}

	/* Discards */
	/DISCARD/ : {
		*(.note.GNU-stack)
		*(.comment)
		*(.eh_frame)
		*(.interp)
		*(.fini_array.*)
	}
}
