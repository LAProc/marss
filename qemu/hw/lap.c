
#include "hw.h"
#include "isa.h"
#include "lap.h"

#ifdef MARSS_QEMU
#include <ptl-qemu.h>
#endif

qemu_irq irq_lap;

typedef struct LAPState {
    ISADevice dev;
} LAPState;

void lap_send_irq(int level)
{
    qemu_set_irq(irq_lap, level);
}

void lap_finish()
{
    lap_mmio_reg = 0;
    printf("Sending interrupt!\n");
    lap_send_irq(1);
    lap_send_irq(0);
}

#define lap_mem_base 0x200000000L
#define LAP_MEM_SIZE 24
#define LAP_IRQ_NO 13

static uint32_t lap_mem_readb(void *opaque, target_phys_addr_t addr)
{
    if (addr >= 0 && addr < 8)
        return lap_mmio_reg;
    else if (addr >= 8 && addr < 12)
        return lap_buf_addr & 0xffffffffL;
    else
        return lap_buf_addr >> 32;
}

static uint32_t lap_mem_readw(void *opaque, target_phys_addr_t addr)
{
    if (addr >= 0 && addr < 8)
        return lap_mmio_reg;
    else if (addr >= 8 && addr < 12)
        return lap_buf_addr & 0xffffffffL;
    else
        return lap_buf_addr >> 32;
}

static uint32_t lap_mem_readl(void *opaque, target_phys_addr_t addr)
{
    if (addr >= 0 && addr < 8)
        return lap_mmio_reg;
    else if (addr >= 8 && addr < 12)
        return lap_buf_addr & 0xffffffffL;
    else
        return lap_buf_addr >> 32;
}

static void lap_mem_writeb(void *opaque, target_phys_addr_t addr, uint32_t val)
{
    uint64_t hugeval = ((uint64_t) val) << 32;
    if (addr >= 0 && addr < 8)
        lap_mmio_reg = val;
    else if (addr >= 8 && addr < 12)
        lap_buf_addr = (lap_buf_addr & 0xffffffff00000000L) | (val);
    else
        lap_buf_addr = (lap_buf_addr & 0xffffffff) | (hugeval);
    printf("lap_mem_writeb: target_phys_addr: %p, val: %d!\n", addr, val);
}

static void lap_mem_writew(void *opaque, target_phys_addr_t addr, uint32_t val)
{
    uint64_t hugeval = ((uint64_t) val) << 32;
    if (addr >= 0 && addr < 8)
        lap_mmio_reg = val;
    else if (addr >= 8 && addr < 12)
        lap_buf_addr = (lap_buf_addr & 0xffffffff00000000L) | (val);
    else
        lap_buf_addr = (lap_buf_addr & 0xffffffff) | (hugeval);
    printf("lap_mem_writew: target_phys_addr: %p, val: %d!\n", addr, val);
}

static void lap_mem_writel(void *opaque, target_phys_addr_t addr, uint32_t val)
{
    uint64_t hugeval = ((uint64_t) val) << 32;
    if (addr >= 0 && addr < 8) {
        lap_mmio_reg = val;

        // HAO: XXX Ad-hoc code to test correctness of interrupt handling, should be
        // removed when interrupt code on Marss side is finished.
        //lap_finish();
    }
    else if (addr >= 8 && addr < 12)
        lap_buf_addr = (lap_buf_addr & 0xffffffff00000000L) | (val);
    else
        lap_buf_addr = (lap_buf_addr & 0xffffffff) | (hugeval);
    printf("lap_mem_writel: target_phys_addr: %p, val: %d!\n", addr, val);

}


CPUReadMemoryFunc * const lap_mem_read[3] = {
    lap_mem_readb,
    lap_mem_readw,
    lap_mem_readl,
};

CPUWriteMemoryFunc * const lap_mem_write[3] = {
    lap_mem_writeb,
    lap_mem_writew,
    lap_mem_writel,
};

static int lap_init_fn(ISADevice *dev)
{
    int lap_io_memory;

    isa_init_irq(dev, &irq_lap, LAP_IRQ_NO);

    lap_io_memory = cpu_register_io_memory(lap_mem_read, lap_mem_write, NULL,
                                           DEVICE_LITTLE_ENDIAN);
    //cpu_register_physical_memory(lap_mem_base, 4, lap_io_memory);
    //ram_addr_t ram_addr = qemu_ram_alloc(NULL, "lap.ram", 4);
    cpu_register_physical_memory(lap_mem_base, LAP_MEM_SIZE, lap_io_memory);
    qemu_register_coalesced_mmio(lap_mem_base, LAP_MEM_SIZE);
    printf("LAP_INIT: lap_mem_base=%ld, lap_io_memory=%d\n", lap_mem_base, lap_io_memory);

    return 0;
}

static ISADeviceInfo lap_isa_info = {
    .qdev.name     = "lap",
    .qdev.size     = sizeof(LAPState),
    //.qdev.no_user  = 1,
    .init          = lap_init_fn,
};

static void lap_register(void)
{
    isa_qdev_register(&lap_isa_info);
}
device_init(lap_register)
