/*
 * Minimal DesignWare USB3 Device Controller (DWC3 UDC) model
 * Only a subset of the register map is implemented. Unimplemented
 * registers return zero and log a warning the first time they are touched.
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/bitops.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "migration/vmstate.h"
#include "hw/usb/dwc3-udc.h"
#include "qemu/units.h"

#define DWC3_UDC_MMIO_SIZE         0x10000
#define DWC3_UDC_GLOBAL_BASE       0xC100
#define DWC3_UDC_DEVICE_BASE       0xC700
#define DWC3_UDC_ENDPOINT_BASE     0xC800
#define DWC3_UDC_ENDPOINT_STRIDE   0x20
#define DWC3_UDC_NUM_ENDPOINTS     8

#define DWC3_DEPCMD_CMD_MASK       0xf
#define DWC3_DEPCMD_SETEPCONFIG    0x01
#define DWC3_DEPCMD_SETTRANSFRESOURCE 0x02
#define DWC3_DEPCMD_STARTTRANSFER  0x06
#define DWC3_DEPCMD_CMDACT         BIT(10)
#define DWC3_DEPEVT_XFERCOMPLETE   0x01
#define DWC3_DEPEVT_XFERNOTREADY   0x03
#define DWC3_DEPEVT_EPCMDCMPLT     0x07
#define DEPEVT_STATUS_IOC          BIT(2)
#define DEPEVT_STATUS_LST          BIT(3)
#define DEPEVT_STATUS_CONTROL_DATA 1
#define DWC3_DCTL_CSFTRST          BIT(30)
#define DWC3_GCTL_CORESOFTRESET    BIT(11)
#define DWC3_GUSB_PHY_PHYSOFTRST   BIT(31)
#define DWC3_GCTL_PRTCAPDIR_MASK   (0x3 << 12)
#define DWC3_GCTL_PRTCAPDIR_HOST   (0x1 << 12)
#define DWC3_GCTL_PRTCAPDIR_DEVICE (0x2 << 12)
#define DWC3_GEVNTSIZ_INTMASK      BIT(31)

#define DWC3_FAKEHOST_PERIOD_NS    (50 * (NANOSECONDS_PER_SECOND / 1000))

static void dwc3_udc_log_unimpl(hwaddr offset, bool write)
{
    qemu_log_mask(LOG_UNIMP, "dwc3-udc: %s access to unimplemented offset 0x%08" HWADDR_PRIx "\n",
                  write ? "write" : "read", offset);
}

static void dwc3_udc_log_reg(const char *op, hwaddr offset, uint32_t val)
{
    qemu_log_mask(LOG_GUEST_ERROR, "dwc3-udc: %s offset=0x%08" HWADDR_PRIx " val=0x%08x\n",
                  op, offset, val);
}

static void dwc3_udc_update_irq(DWC3UdcState *s)
{
    bool masked = s->gevntsiz & DWC3_GEVNTSIZ_INTMASK;
    bool pending = s->event_buf_count > 0;
    bool level = pending && !masked;

    if (level != s->irq_asserted) {
        qemu_set_irq(s->irq, level);
        s->irq_asserted = level;
    }
}

bool dwc3_udc_dma_rw(DWC3UdcState *s, hwaddr addr, void *buf,
                     size_t len, bool is_write)
{
    MemTxResult res;

    res = address_space_rw(s->dma_as, addr, MEMTXATTRS_UNSPECIFIED,
                           buf, len, is_write);
    if (res != MEMTX_OK) {
        qemu_log_mask(LOG_GUEST_ERROR,
                      "dwc3-udc: DMA %s failed @0x%08" HWADDR_PRIx " len=%zu\n",
                      is_write ? "write" : "read", addr, len);
        return false;
    }

    return true;
}

void dwc3_udc_push_event(DWC3UdcState *s, uint32_t event)
{
    if (!s->event_buf_size) {
        qemu_log_mask(LOG_GUEST_ERROR,
                      "dwc3-udc: event buffer size is zero, drop event 0x%08x\n",
                      event);
        return;
    }

    if (s->event_buf_count + sizeof(event) > s->event_buf_size) {
        qemu_log_mask(LOG_GUEST_ERROR,
                      "dwc3-udc: event buffer overflow (size=%u count=%u offset=%u)\n",
                      s->event_buf_size, s->event_buf_count,
                      s->event_buf_offset);
        return;
    }

    uint64_t base = s->gevntadrlo | ((uint64_t)s->gevntadrhi << 32);
    hwaddr target = base + s->event_buf_offset;
    if (!dwc3_udc_dma_rw(s, target, &event, sizeof(event), true)) {
        return;
    }

    s->event_buf_offset += sizeof(event);
    if (s->event_buf_offset >= s->event_buf_size) {
        s->event_buf_offset = 0;
    }
    s->event_buf_count += sizeof(event);
    s->gevntcount = s->event_buf_count;
    qemu_log_mask(LOG_GUEST_ERROR,
                  "dwc3-udc: push_event=0x%08x size=%u offset=%u pending=%u base=0x%016" PRIx64 " target=0x%08" HWADDR_PRIx "\n",
                  event, s->event_buf_size, s->event_buf_offset,
                  s->event_buf_count, base, target);
    dwc3_udc_update_irq(s);
}

static void dwc3_udc_push_ep_event(DWC3UdcState *s, uint8_t ep,
                                   uint8_t evt, uint8_t status,
                                   uint16_t param)
{
    uint32_t event = 0;

    event |= (uint32_t)(ep & 0x1f) << 1;
    event |= (uint32_t)(evt & 0xf) << 6;
    event |= (uint32_t)(status & 0xf) << 12;
    event |= (uint32_t)param << 16;

    dwc3_udc_push_event(s, event);
}

static void dwc3_udc_push_cmd_complete(DWC3UdcState *s, uint8_t ep,
                                       uint8_t opcode)
{
    uint16_t param = (uint16_t)opcode << 8;

    dwc3_udc_push_ep_event(s, ep, DWC3_DEPEVT_EPCMDCMPLT, 0, param);
}

static void dwc3_udc_core_reset(DWC3UdcState *s)
{
    unsigned i;

    qemu_log_mask(LOG_GUEST_ERROR,
                  "dwc3-udc: core_reset clearing event ring size=%u offset=%u count=%u\n",
                  s->event_buf_size, s->event_buf_offset, s->event_buf_count);

    s->gctl = 0;
    /* set CURMOD to device to satisfy dwc3 driver polls */
    s->gsts = 0x2;
    s->gevntadrlo = 0;
    s->gevntadrhi = 0;
    s->gevntsiz = 0;
    s->gevntcount = 0;
    s->event_buf_size = 0;
    s->event_buf_offset = 0;
    s->event_buf_count = 0;

    s->gusb3pipectl = 0;
    s->dcfg = 0;
    s->dctl = 0;
    s->devten = 0;
    s->dsts = 0;
    s->dalepena = 0;

    s->irq_asserted = false;
    dwc3_udc_update_irq(s);

    for (i = 0; i < DWC3_UDC_NUM_ENDPOINTS; ++i) {
        s->eps[i].par0 = 0;
        s->eps[i].par1 = 0;
        s->eps[i].par2 = 0;
        s->eps[i].cmd = 0;
        s->eps[i].sts = 0;
        s->eps[i].active_trb = 0;
    }
    dwc3_udc_update_irq(s);
}

static void dwc3_udc_handle_depcmd(DWC3UdcState *s, unsigned ep, uint32_t cmd)
{
    DWC3UdcEpState *epst = &s->eps[ep];
    /* Linux writes param0=upper 32 bits, param1=lower 32 bits */
    uint64_t trb_addr = ((uint64_t)epst->par0 << 32) | epst->par1;
    Dwc3UdcTrb trb = {};
    uint32_t opcode = cmd & DWC3_DEPCMD_CMD_MASK;

    epst->cmd = cmd;
    epst->active_trb = 0;

    /* Handle simple commands without TRB data. DEPSTARTCFG (0x09) enables eps. */
    if (opcode == 0x09) {
        if (ep == 0) {
            /* enable both control directions */
            s->dalepena |= 0x3;
        } else {
            s->dalepena |= (1u << ep);
        }
        epst->cmd = cmd & ~DWC3_DEPCMD_CMDACT;
        epst->sts = 0;
        dwc3_udc_push_cmd_complete(s, ep, opcode);
        return;
    }

    /* SEPCONFIG and SETXFERRES succeed immediately for now. */
    if (opcode == DWC3_DEPCMD_SETEPCONFIG || opcode == DWC3_DEPCMD_SETTRANSFRESOURCE) {
        epst->cmd = cmd & ~DWC3_DEPCMD_CMDACT;
        epst->sts = 0;
        dwc3_udc_push_cmd_complete(s, ep, opcode);
        return;
    }

    /* Accept UPDATETRANSFER (0x07) to move to a new TRB/segment without DMA. */
    if (opcode == 0x07) {
        epst->cmd = cmd & ~DWC3_DEPCMD_CMDACT;
        epst->sts = 0;
        epst->active_trb = trb_addr;
        dwc3_udc_push_cmd_complete(s, ep, opcode);
        return;
    }

    /* ENDETRANSFER (0x08) completes immediately for now. */
    if (opcode == 0x08) {
        epst->cmd = cmd & ~DWC3_DEPCMD_CMDACT;
        epst->sts = 0;
        dwc3_udc_push_cmd_complete(s, ep, opcode);
        return;
    }

    /* Only TRB-based commands reach here. */
    epst->active_trb = trb_addr;

    if (!trb_addr) {
        qemu_log_mask(LOG_GUEST_ERROR,
                      "dwc3-udc: TRB addr is zero ep=%u cmd=0x%08x, completing without DMA\n",
                      ep, cmd);
        epst->cmd = cmd & ~DWC3_DEPCMD_CMDACT;
        epst->sts = 0;
        dwc3_udc_push_cmd_complete(s, ep, opcode);
        return;
    }

    if (!dwc3_udc_dma_rw(s, trb_addr, &trb, sizeof(trb), false)) {
        qemu_log_mask(LOG_GUEST_ERROR,
                      "dwc3-udc: TRB read failed ep=%u addr=0x%08" HWADDR_PRIx "\n",
                      ep, trb_addr);
        uint32_t event = (ep << 16) | opcode;
        epst->cmd = cmd & ~DWC3_DEPCMD_CMDACT;
        epst->sts = 0;
        dwc3_udc_push_event(s, event);
        return;
    }

    qemu_log_mask(LOG_GUEST_ERROR,
                  "dwc3-udc: depcmd ep=%u cmd=0x%08x trb_addr=0x%08" HWADDR_PRIx
                  " buf=0x%016" PRIx64 " status=0x%08x ctrl=0x%08x\n",
                  ep, cmd, trb_addr, trb.buffer, trb.status, trb.control);

    uint32_t len = trb.status & 0x00ffffff;
    if (len) {
        size_t chunk = MIN((size_t)len, (size_t)64);
        uint8_t bounce[64];
        memset(bounce, 0, sizeof(bounce));

        if (!dwc3_udc_dma_rw(s, trb.buffer, bounce, chunk, false)) {
            uint32_t event = (ep << 16) | opcode;
            epst->cmd = cmd & ~DWC3_DEPCMD_CMDACT;
            epst->sts = 0;
            dwc3_udc_push_event(s, event);
            return;
        }
        /* Echo the same data back to prove DMA writes also work. */
        dwc3_udc_dma_rw(s, trb.buffer, bounce, chunk, true);
    }

    /* Push a simple endpoint-complete event. */
    dwc3_udc_push_ep_event(s, ep, DWC3_DEPEVT_XFERCOMPLETE,
                           DEPEVT_STATUS_IOC | DEPEVT_STATUS_LST,
                           (uint16_t)len);

    /* clear CMDACT to signal completion */
    epst->cmd = cmd & ~DWC3_DEPCMD_CMDACT;
    epst->sts = 0;
    dwc3_udc_push_cmd_complete(s, ep, opcode);
}

static uint64_t dwc3_udc_read(void *opaque, hwaddr offset, unsigned size)
{
    DWC3UdcState *s = opaque;

    if (offset >= DWC3_UDC_GLOBAL_BASE && offset < DWC3_UDC_DEVICE_BASE) {
        switch (offset - DWC3_UDC_GLOBAL_BASE) {
        case 0x10: return s->gctl;
        case 0x18: return s->gsts;
        case 0x20: return s->gsnpsid;
        case 0x40: return 0x40204048; /* GHWPARAMS0: DRD capable, 16 IN/OUT eps */
        case 0x44: return 0x0222493b; /* GHWPARAMS1: borrowed from host model */
        case 0x48: return 0x12345678; /* GHWPARAMS2 */
        case 0x4c: return 0x0618c088; /* GHWPARAMS3 */
        case 0x50: return 0x04782204; /* GHWPARAMS4 */
        case 0x54: return 0x04202088; /* GHWPARAMS5 */
        case 0x58: return 0x07850c20; /* GHWPARAMS6 */
        case 0x5c: return 0x00000000; /* GHWPARAMS7 */
        case 0x500: return 0x00000478; /* GHWPARAMS8 */
        case 0x100: return s->gusb2phycfg;
        case 0x1c0: return s->gusb3pipectl;
        case 0x300: return s->gevntadrlo;
        case 0x304: return s->gevntadrhi;
        case 0x308: return s->gevntsiz;
        case 0x30c: return s->gevntcount;
        default:
            dwc3_udc_log_unimpl(offset, false);
            return 0;
        }
    }

    if (offset >= DWC3_UDC_DEVICE_BASE && offset < DWC3_UDC_ENDPOINT_BASE) {
        switch (offset - DWC3_UDC_DEVICE_BASE) {
        case 0x00: return s->dcfg;
        case 0x04:
            /* log every DCTL read and clear CSFTRST when observed */
            dwc3_udc_log_reg("dctl_read", offset, s->dctl);
            if (s->dctl & DWC3_DCTL_CSFTRST) {
                dwc3_udc_log_reg("dctl_read_clear", offset, s->dctl);
                s->dctl &= ~DWC3_DCTL_CSFTRST;
            }
            return s->dctl;
        case 0x08: return s->devten;
        case 0x0c: return s->dsts;
        case 0x10: return s->dalepena;
        default:
            dwc3_udc_log_unimpl(offset, false);
            return 0;
        }
    }

    if (offset >= DWC3_UDC_ENDPOINT_BASE &&
        offset < DWC3_UDC_MMIO_SIZE) {
        unsigned ep = (offset - DWC3_UDC_ENDPOINT_BASE) /
                      DWC3_UDC_ENDPOINT_STRIDE;
        unsigned ep_off = (offset - DWC3_UDC_ENDPOINT_BASE) %
                          DWC3_UDC_ENDPOINT_STRIDE;

        if (ep >= DWC3_UDC_NUM_ENDPOINTS) {
            dwc3_udc_log_unimpl(offset, false);
            return 0;
        }

        DWC3UdcEpState *epst = &s->eps[ep];
        switch (ep_off) {
        case 0x00: return epst->par0;
        case 0x04: return epst->par1;
        case 0x08: return epst->par2;
        case 0x0c: return epst->cmd;
        case 0x10: return epst->sts;
        default:
            dwc3_udc_log_unimpl(offset, false);
            return 0;
        }
    }

    dwc3_udc_log_unimpl(offset, false);
    return 0;
}

static void dwc3_udc_write(void *opaque, hwaddr offset, uint64_t value,
                           unsigned size)
{
    DWC3UdcState *s = opaque;
    uint32_t val = (uint32_t)value;
    dwc3_udc_log_reg("write", offset, val);

    if (offset >= DWC3_UDC_GLOBAL_BASE && offset < DWC3_UDC_DEVICE_BASE) {
        switch (offset - DWC3_UDC_GLOBAL_BASE) {
        case 0x10:
            /* auto-complete core soft reset */
            if (val & DWC3_GCTL_CORESOFTRESET) {
                dwc3_udc_core_reset(s);
                s->gctl = val & ~DWC3_GCTL_CORESOFTRESET;
            } else {
                s->gctl = val;
            }
            dwc3_udc_log_reg("gctl_write", offset, s->gctl);
            /* drive GSTS.CURMOD to match device/host selection */
            if ((s->gctl & DWC3_GCTL_PRTCAPDIR_MASK) == DWC3_GCTL_PRTCAPDIR_DEVICE) {
                s->gsts = 0x2; /* device mode */
            } else if ((s->gctl & DWC3_GCTL_PRTCAPDIR_MASK) == DWC3_GCTL_PRTCAPDIR_HOST) {
                s->gsts = 0x1; /* host mode */
            }
            return;
        case 0x18:
            s->gsts = val;
            return;
        case 0x20:
            s->gsnpsid = val;
            return;
        case 0x100:
            s->gusb2phycfg = val;
            dwc3_udc_log_reg("gusb2phycfg_write", offset, val);
            return;
        case 0x1c0:
            s->gusb3pipectl = val;
            dwc3_udc_log_reg("gusb3pipectl_write", offset, val);
            return;
        case 0x300:
            s->gevntadrlo = val;
            return;
        case 0x304:
            s->gevntadrhi = val;
            return;
        case 0x308:
            /* Update size/mask without discarding pending events unless size changes. */
            {
                uint32_t new_size = val & 0xffff;
                bool size_changed = new_size && new_size != s->event_buf_size;

                s->gevntsiz = val;

                if (size_changed) {
                    s->event_buf_size = new_size;
                    s->event_buf_offset = 0;
                    s->event_buf_count = 0;
                    s->gevntcount = 0;
                }

                dwc3_udc_update_irq(s);
                return;
            }
        case 0x30c: {
            /* Writing count subtracts consumed bytes from the event queue. */
            uint32_t consumed = val;
            if (consumed > s->event_buf_count) {
                consumed = s->event_buf_count;
            }
            s->event_buf_count -= consumed;
            s->gevntcount = s->event_buf_count;
            qemu_log_mask(LOG_GUEST_ERROR,
                          "dwc3-udc: gevntcount_write consumed=%u new_count=%u offset=%u\n",
                          consumed, s->event_buf_count, s->event_buf_offset);
            dwc3_udc_update_irq(s);
            return;
        }
        default:
            dwc3_udc_log_unimpl(offset, true);
            return;
        }
    }

    if (offset >= DWC3_UDC_DEVICE_BASE && offset < DWC3_UDC_ENDPOINT_BASE) {
        switch (offset - DWC3_UDC_DEVICE_BASE) {
        case 0x00: s->dcfg = val; return;
        case 0x04:
            dwc3_udc_log_reg("dctl_write", offset, val);
            if (val & DWC3_DCTL_CSFTRST) {
                /* emulate core soft reset completion */
                dwc3_udc_core_reset(s);
                s->dctl = val & ~DWC3_DCTL_CSFTRST;
            } else {
                s->dctl = val;
            }
            return;
        case 0x08: s->devten = val; return;
        case 0x0c: s->dsts = val; return;
        case 0x10: s->dalepena = val; return;
        default:
            dwc3_udc_log_unimpl(offset, true);
            return;
        }
    }

    if (offset >= DWC3_UDC_ENDPOINT_BASE &&
        offset < DWC3_UDC_MMIO_SIZE) {
        unsigned ep = (offset - DWC3_UDC_ENDPOINT_BASE) /
                      DWC3_UDC_ENDPOINT_STRIDE;
        unsigned ep_off = (offset - DWC3_UDC_ENDPOINT_BASE) %
                          DWC3_UDC_ENDPOINT_STRIDE;

        if (ep >= DWC3_UDC_NUM_ENDPOINTS) {
            dwc3_udc_log_unimpl(offset, true);
            return;
        }

        DWC3UdcEpState *epst = &s->eps[ep];
        switch (ep_off) {
        case 0x00: epst->par0 = val; return;
        case 0x04: epst->par1 = val; return;
        case 0x08: epst->par2 = val; return;
        case 0x0c: dwc3_udc_handle_depcmd(s, ep, val); return;
        case 0x10: epst->sts = val; return;
        default:
            dwc3_udc_log_unimpl(offset, true);
            return;
        }
    }

    dwc3_udc_log_unimpl(offset, true);
}

static const MemoryRegionOps dwc3_udc_ops = {
    .read = dwc3_udc_read,
    .write = dwc3_udc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid.min_access_size = 4,
    .valid.max_access_size = 4,
};

static void dwc3_udc_reset(DeviceState *dev)
{
    DWC3UdcState *s = DWC3_UDC(dev);
    s->gsnpsid = 0x5533330a;
    s->gusb2phycfg = 0x40102410;
    dwc3_udc_core_reset(s);
}

static void dwc3_udc_realize(DeviceState *dev, Error **errp)
{
    DWC3UdcState *s = DWC3_UDC(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    s->dma_as = &address_space_memory;

    memory_region_init_io(&s->mmio, OBJECT(dev), &dwc3_udc_ops, s,
                          TYPE_DWC3_UDC, DWC3_UDC_MMIO_SIZE);
    sysbus_init_mmio(sbd, &s->mmio);
    sysbus_init_irq(sbd, &s->irq);
}

static const VMStateDescription vmstate_dwc3_udc_ep = {
    .name = TYPE_DWC3_UDC ".ep",
    .version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(par0, DWC3UdcEpState),
        VMSTATE_UINT32(par1, DWC3UdcEpState),
        VMSTATE_UINT32(par2, DWC3UdcEpState),
        VMSTATE_UINT32(cmd, DWC3UdcEpState),
        VMSTATE_UINT32(sts, DWC3UdcEpState),
        VMSTATE_UINT64(active_trb, DWC3UdcEpState),
        VMSTATE_END_OF_LIST()
    }
};

static const VMStateDescription vmstate_dwc3_udc = {
    .name = TYPE_DWC3_UDC,
    .version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(gctl, DWC3UdcState),
        VMSTATE_UINT32(gsts, DWC3UdcState),
        VMSTATE_UINT32(gsnpsid, DWC3UdcState),
        VMSTATE_UINT32(gusb2phycfg, DWC3UdcState),
        VMSTATE_UINT32(gusb3pipectl, DWC3UdcState),
        VMSTATE_UINT32(gevntadrlo, DWC3UdcState),
        VMSTATE_UINT32(gevntadrhi, DWC3UdcState),
        VMSTATE_UINT32(gevntsiz, DWC3UdcState),
        VMSTATE_UINT32(gevntcount, DWC3UdcState),
        VMSTATE_UINT32(dcfg, DWC3UdcState),
        VMSTATE_UINT32(dctl, DWC3UdcState),
        VMSTATE_UINT32(devten, DWC3UdcState),
        VMSTATE_UINT32(dsts, DWC3UdcState),
        VMSTATE_UINT32(dalepena, DWC3UdcState),
        VMSTATE_UINT32(event_buf_size, DWC3UdcState),
        VMSTATE_UINT32(event_buf_offset, DWC3UdcState),
        VMSTATE_STRUCT_ARRAY(eps, DWC3UdcState, DWC3_UDC_NUM_ENDPOINTS, 0,
                     vmstate_dwc3_udc_ep, DWC3UdcEpState),
        VMSTATE_END_OF_LIST()
    }
};

static void dwc3_udc_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->reset = dwc3_udc_reset;
    dc->realize = dwc3_udc_realize;
    dc->vmsd = &vmstate_dwc3_udc;
}

static const TypeInfo dwc3_udc_info = {
    .name          = TYPE_DWC3_UDC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(DWC3UdcState),
    .class_init    = dwc3_udc_class_init,
};

static void dwc3_udc_register_types(void)
{
    type_register_static(&dwc3_udc_info);
}

type_init(dwc3_udc_register_types)
