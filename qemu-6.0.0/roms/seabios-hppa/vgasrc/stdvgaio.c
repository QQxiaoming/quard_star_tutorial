// Standard VGA IO port access
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "farptr.h" // GET_FARVAR
#include "stdvga.h" // VGAREG_PEL_MASK
#include "vgautil.h" // stdvga_pelmask_read
#include "x86.h" // inb

#define PARISC_VGA_PORT_OFFS (parisc_vga_mmio + 0x400 - 0x3c0)

u8
vga_inb(unsigned long port)
{
#if CONFIG_PARISC
	return *(u8 *)(PARISC_VGA_PORT_OFFS + port);
#else
	return inb(port);
#endif
}

void
vga_outb(u8 value, unsigned long port)
{
#if CONFIG_PARISC
	*(u8 *)(PARISC_VGA_PORT_OFFS + port) = value;
#else
	return outb(port);
#endif
}

void
vga_outw(u16 value, unsigned long port)
{
#if CONFIG_PARISC
	vga_outb(value & 0xff, port);
	vga_outb(value >> 8, port+1);
#else
	outw(value, port);
#endif
}

u8
stdvga_pelmask_read(void)
{
    return vga_inb(VGAREG_PEL_MASK);
}

void
stdvga_pelmask_write(u8 value)
{
    vga_outb(value, VGAREG_PEL_MASK);
}


u8
stdvga_misc_read(void)
{
    return vga_inb(VGAREG_READ_MISC_OUTPUT);
}

void
stdvga_misc_write(u8 value)
{
    vga_outb(value, VGAREG_WRITE_MISC_OUTPUT);
}

void
stdvga_misc_mask(u8 off, u8 on)
{
    stdvga_misc_write((stdvga_misc_read() & ~off) | on);
}


u8
stdvga_sequ_read(u8 index)
{
    vga_outb(index, VGAREG_SEQU_ADDRESS);
    return vga_inb(VGAREG_SEQU_DATA);
}

void
stdvga_sequ_write(u8 index, u8 value)
{
    vga_outw((value<<8) | index, VGAREG_SEQU_ADDRESS);
}

void
stdvga_sequ_mask(u8 index, u8 off, u8 on)
{
    vga_outb(index, VGAREG_SEQU_ADDRESS);
    u8 v = vga_inb(VGAREG_SEQU_DATA);
    vga_outb((v & ~off) | on, VGAREG_SEQU_DATA);
}


u8
stdvga_grdc_read(u8 index)
{
    vga_outb(index, VGAREG_GRDC_ADDRESS);
    return vga_inb(VGAREG_GRDC_DATA);
}

void
stdvga_grdc_write(u8 index, u8 value)
{
    vga_outw((value<<8) | index, VGAREG_GRDC_ADDRESS);
}

void
stdvga_grdc_mask(u8 index, u8 off, u8 on)
{
    vga_outb(index, VGAREG_GRDC_ADDRESS);
    u8 v = vga_inb(VGAREG_GRDC_DATA);
    vga_outb((v & ~off) | on, VGAREG_GRDC_DATA);
}


u8
stdvga_crtc_read(u16 crtc_addr, u8 index)
{
    vga_outb(index, crtc_addr);
    return vga_inb(crtc_addr + 1);
}

void
stdvga_crtc_write(u16 crtc_addr, u8 index, u8 value)
{
    vga_outw((value<<8) | index, crtc_addr);
}

void
stdvga_crtc_mask(u16 crtc_addr, u8 index, u8 off, u8 on)
{
    vga_outb(index, crtc_addr);
    u8 v = vga_inb(crtc_addr + 1);
    vga_outb((v & ~off) | on, crtc_addr + 1);
}


u8
stdvga_attr_read(u8 index)
{
    vga_inb(VGAREG_ACTL_RESET);
    u8 orig = vga_inb(VGAREG_ACTL_ADDRESS);
    vga_outb(index, VGAREG_ACTL_ADDRESS);
    u8 v = vga_inb(VGAREG_ACTL_READ_DATA);
    vga_inb(VGAREG_ACTL_RESET);
    vga_outb(orig, VGAREG_ACTL_ADDRESS);
    return v;
}

void
stdvga_attr_write(u8 index, u8 value)
{
    vga_inb(VGAREG_ACTL_RESET);
    u8 orig = vga_inb(VGAREG_ACTL_ADDRESS);
    vga_outb(index, VGAREG_ACTL_ADDRESS);
    vga_outb(value, VGAREG_ACTL_WRITE_DATA);
    vga_outb(orig, VGAREG_ACTL_ADDRESS);
}

void
stdvga_attr_mask(u8 index, u8 off, u8 on)
{
    vga_inb(VGAREG_ACTL_RESET);
    u8 orig = vga_inb(VGAREG_ACTL_ADDRESS);
    vga_outb(index, VGAREG_ACTL_ADDRESS);
    u8 v = vga_inb(VGAREG_ACTL_READ_DATA);
    vga_outb((v & ~off) | on, VGAREG_ACTL_WRITE_DATA);
    vga_outb(orig, VGAREG_ACTL_ADDRESS);
}

u8
stdvga_attrindex_read(void)
{
    vga_inb(VGAREG_ACTL_RESET);
    return vga_inb(VGAREG_ACTL_ADDRESS);
}

void
stdvga_attrindex_write(u8 value)
{
    vga_inb(VGAREG_ACTL_RESET);
    vga_outb(value, VGAREG_ACTL_ADDRESS);
}


void
stdvga_dac_read(u16 seg, u8 *data_far, u8 start, int count)
{
    vga_outb(start, VGAREG_DAC_READ_ADDRESS);
    while (count) {
        SET_FARVAR(seg, *data_far, vga_inb(VGAREG_DAC_DATA));
        data_far++;
        SET_FARVAR(seg, *data_far, vga_inb(VGAREG_DAC_DATA));
        data_far++;
        SET_FARVAR(seg, *data_far, vga_inb(VGAREG_DAC_DATA));
        data_far++;
        count--;
    }
}

void
stdvga_dac_write(u16 seg, u8 *data_far, u8 start, int count)
{
    vga_outb(start, VGAREG_DAC_WRITE_ADDRESS);
    while (count) {
        vga_outb(GET_FARVAR(seg, *data_far), VGAREG_DAC_DATA);
        data_far++;
        vga_outb(GET_FARVAR(seg, *data_far), VGAREG_DAC_DATA);
        data_far++;
        vga_outb(GET_FARVAR(seg, *data_far), VGAREG_DAC_DATA);
        data_far++;
        count--;
    }
}
