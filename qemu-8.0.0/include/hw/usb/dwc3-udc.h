/*
 * Minimal DesignWare USB3 Device Controller (DWC3 UDC) model
 */
#ifndef HW_USB_DWC3_UDC_H
#define HW_USB_DWC3_UDC_H

#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "qemu/timer.h"

#define TYPE_DWC3_UDC "dwc3-udc"

#define DWC3_UDC(obj) \
    OBJECT_CHECK(DWC3UdcState, (obj), TYPE_DWC3_UDC)

typedef struct Dwc3UdcTrb {
    uint64_t buffer;
    uint32_t status;
    uint32_t control;
} QEMU_PACKED Dwc3UdcTrb;

typedef struct DWC3UdcEpState {
    uint32_t par0;
    uint32_t par1;
    uint32_t par2;
    uint32_t cmd;
    uint32_t sts;
    uint64_t active_trb;
} DWC3UdcEpState;

typedef struct DWC3UdcState {
    SysBusDevice parent_obj;

    MemoryRegion mmio;
    qemu_irq irq;

    /* Selected global registers we actively model. */
    uint32_t gctl;
    uint32_t gsts;
    uint32_t gsnpsid;
    uint32_t gusb2phycfg;
    uint32_t gusb3pipectl;
    uint32_t gevntadrlo;
    uint32_t gevntadrhi;
    uint32_t gevntsiz;
    uint32_t gevntcount;

    /* Device-mode registers (subset). */
    uint32_t dcfg;
    uint32_t dctl;
    uint32_t devten;
    uint32_t dsts;
    uint32_t dalepena;

    DWC3UdcEpState eps[8];

    uint32_t event_buf_size;
    uint32_t event_buf_offset;
    uint32_t event_buf_count;
    bool irq_asserted;

    AddressSpace *dma_as;
} DWC3UdcState;

bool dwc3_udc_dma_rw(DWC3UdcState *s, hwaddr addr, void *buf,
                     size_t len, bool is_write);
void dwc3_udc_push_event(DWC3UdcState *s, uint32_t event);

#endif /* HW_USB_DWC3_UDC_H */
