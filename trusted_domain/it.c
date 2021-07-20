#include <stdint.h>
#include "debug_log.h"
#include "sbi.h"
#include "riscv_asm.h"


static const char *Exception_Name[] = 
{
    "Instruction Address Misaligned",
    "Instruction Access Fault",
    "Illegal Instruction",
    "Breakpoint",
    "Load Address Misaligned",
    "Load Access Fault",
    "Store/AMO Address Misaligned",
    "Store/AMO Access Fault",
    "Environment call from U-mode",
    "Environment call from S-mode",
    "Reserved-10",
    "Reserved-11",
    "Instruction Page Fault",
    "Load Page Fault",
    "Reserved-14",
    "Store/AMO Page Fault"
};

static const char *Interrupt_Name[] = 
{
    "User Software Interrupt",
    "Supervisor Software Interrupt",
    "Reversed-2",
    "Reversed-3",
    "User Timer Interrupt",
    "Supervisor Timer Interrupt",
    "Reversed-6",
    "Reversed-7",
    "User External Interrupt",
    "Supervisor External Interrupt",
    "Reserved-10",
    "Reserved-11",
};

struct stack_frame
{
    uint64_t sepc;
    uint64_t ra;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0_fp;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    uint64_t sstatus;
};

void dump_regs(struct stack_frame *regs)
{
    debug_log("--------------Dump Registers-----------------\n");
    debug_log("Function Registers:\n");
    debug_log("\tra(x1) = 0x%x\tuser_sp = 0x%x\n",regs -> ra);
    debug_log("Temporary Registers:\n");
    debug_log("\tt0(x5) = 0x%x\tt1(x6) = 0x%x\n",regs -> t0,regs -> t1);
    debug_log("\tt2(x7) = 0x%x\n",regs -> t2);
    debug_log("\tt3(x28) = 0x%x\tt4(x29) = 0x%x\n",regs -> t3,regs -> t4);
    debug_log("\tt5(x30) = 0x%x\tt6(x31) = 0x%x\n",regs -> t5,regs -> t6);
    debug_log("Saved Registers:\n");
    debug_log("\ts0/fp(x8) = 0x%x\ts1(x9) = 0x%x\n",regs -> s0_fp,regs -> s1);
    debug_log("\ts2(x18) = 0x%x\ts3(x19) = 0x%x\n",regs -> s2,regs -> s3);
    debug_log("\ts4(x20) = 0x%x\ts5(x21) = 0x%x\n",regs -> s4,regs -> s5);
    debug_log("\ts6(x22) = 0x%x\ts7(x23) = 0x%x\n",regs -> s6,regs -> s7);
    debug_log("\ts8(x24) = 0x%x\ts9(x25) = 0x%x\n",regs -> s8,regs -> s9);
    debug_log("\ts10(x26) = 0x%x\ts11(x27) = 0x%x\n",regs -> s10,regs -> s11);
    debug_log("Function Arguments Registers:\n");
    debug_log("\ta0(x10) = 0x%x\ta1(x11) = 0x%x\n",regs -> a0,regs -> a1);
    debug_log("\ta2(x12) = 0x%x\ta3(x13) = 0x%x\n",regs -> a2,regs -> a3);
    debug_log("\ta4(x14) = 0x%x\ta5(x15) = 0x%x\n",regs -> a4,regs -> a5);
    debug_log("\ta6(x16) = 0x%x\ta7(x17) = 0x%x\n",regs -> a6,regs -> a7);
    debug_log("sstatus = 0x%x\n",regs -> sstatus);
    debug_log("\t%s\n",(regs -> sstatus & SSTATUS_SIE) ? "Supervisor Interrupt Enabled" : "Supervisor Interrupt Disabled");
    debug_log("\t%s\n",(regs -> sstatus & SSTATUS_SPIE) ? "Last Time Supervisor Interrupt Enabled" : "Last Time Supervisor Interrupt Disabled");
    debug_log("\t%s\n",(regs -> sstatus & SSTATUS_SPP) ? "Last Privilege is Supervisor Mode" : "Last Privilege is User Mode");
    debug_log("\t%s\n",(regs -> sstatus & (1 << 19)) ? "Permit to Read Executable-only Page" : "Not Permit to Read Executable-only Page");
    debug_log("-----------------Dump OK---------------------\n");
}

void handle_trap(uint64_t scause,uint64_t sepc,uint64_t stval, struct stack_frame *sp)
{
    int id = scause&0x1ff;
    const char *msg;

    if(scause >> 63)
    {
        if(id < sizeof(Interrupt_Name) / sizeof(const char *))
        {
            msg = Interrupt_Name[id];
        }
        else
        {
            msg = "Unknown Interrupt";
        }

        debug_log("Unhandled Interrupt %d:%s\n",id,msg);
    }
    else
    {
        if(id < sizeof(Exception_Name) / sizeof(const char *))
        {
            msg = Exception_Name[id];
        }
        else
        {
            msg = "Unknown Exception";
        }

        debug_log("Unhandled Exception %d:%s\n",id,msg);
    }

    debug_log("scause:0x%x,stval:0x%x,sepc:0x%x\n",scause,stval,sepc);
    dump_regs(sp);
    while(1);
}

void handle_interrupt(void)
{
    while(1);
}
