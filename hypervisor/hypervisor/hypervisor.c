#include "types.h"
#include "long_mode.h"
#include "serialport_log64.h"
#include "isr_wrapper.h"
#include "inline_asm.h"
#include "vmx.h"
#include "vmexit.h"
#include "hypervisor.h"

//defines-----------------

QWORD VPID=1;

QWORD HYPERVISOR_base=1;

QWORD HYPERVISOR_size=1;

QWORD HYPERVISOR_RAM_MAX=1;


QWORD TABLE_SZ=1;
QWORD PAGE_SZ=1;

// entry counts
QWORD PDE_CNT=1;
QWORD PDPTE_CNT=1;
QWORD PML4E_CNT=1;

// tables counts
QWORD PD_CNT=1;
QWORD PDPT_CNT=1;
QWORD PML4_CNT=1;

QWORD EPT_RAM_MAX=1;

// entry counts
QWORD EPT_PTE_CNT=1;
QWORD EPT_PDE_CNT=1;
QWORD EPT_PDPTE_CNT=1;
QWORD EPT_PML4E_CNT=1;

// tables counts
QWORD EPT_PT_CNT=1;
QWORD EPT_PD_CNT=1;
QWORD EPT_PDPT_CNT=1;
QWORD EPT_PML4_CNT=1;

QWORD EPT_STRUCT_SIZEx8=1;

QWORD CACHE_TP_UC=1;
QWORD CACHE_TP_WC=1;
QWORD CACHE_TP_WT=1;
QWORD CACHE_TP_WP=1;
QWORD CACHE_TP_WB=1;

QWORD DEV_RAM_addr=1;
QWORD DEV_RAM_size=1;

QWORD VIDEO_RAM0_addr=1;
QWORD VIDEO_RAM0_size=1;

QWORD VIDEO_RAM1_addr=1;
QWORD VIDEO_RAM1_size=1;

QWORD VIDEO_RAM2_addr=1;
QWORD VIDEO_RAM2_size=1;

QWORD VIDEO_RAM3_addr=1;
QWORD VIDEO_RAM3_size=1;

QWORD TIMER_val=1;

QWORD CONFIG_ADDRESS=1;
QWORD CONFIG_DATA=1;
//---------------------------------------------------------


//memory_map-----------------------------------------------

QWORD STACK64_size=1;
QWORD STACK64_addr=1;  

QWORD STACK64i_size=1;
QWORD STACK64i_addr=1;              

QWORD GDT64_size=1;
QWORD GDT64_addr=1;                                

QWORD IDT64_size=1;
QWORD IDT64_addr=1;                    

QWORD PML4_size=1;
QWORD PML4_addr=1;

QWORD PDPT_size=1;
QWORD PDPT_addr=1;

QWORD PD_size=1;
QWORD PD_addr=1;

QWORD TSS_size=1;
QWORD TSS_addr=1;

QWORD VMXON_size=1;
QWORD VMXON_addr=1;

QWORD VMCS_size=1;
QWORD VMCS_addr=1;

QWORD MSR_BMP_size=1;
QWORD MSR_BMP_addr=1;

QWORD IO_BMPA_size=1;
QWORD IO_BMPA_addr=1;

QWORD IO_BMPB_size=1;
QWORD IO_BMPB_addr=1;

QWORD MSR_EX_LDR_size=1;
QWORD MSR_EX_LDR_addr=1;       /* VM-exit MSR-load address */

QWORD MSR_EX_STR_size=1;
QWORD MSR_EX_STR_addr=1; /* VM-exit MSR-store address */

QWORD MSR_EN_LDR_size=1;
QWORD MSR_EN_LDR_addr=1;                     /* VM-entry MSR-load address */

QWORD EPT_PML4_size=1;
QWORD EPT_PML4_addr=1;

QWORD EPT_PDPT_size=1;
QWORD EPT_PDPT_addr=1;

QWORD EPT_PD_size=1;
QWORD EPT_PD_addr=1;

QWORD EPT_PT_size=1;
QWORD EPT_PT_addr=1;

QWORD VAR_size=1;
QWORD VAR_addr=1;

QWORD GUEST_REGS_size=1;                                /* rax, rcx, rdx, rbx, rsp, rbp, rsi , rdi */
QWORD GUEST_REGS_addr=1;

//-------------------------------------------------------------

QWORD MCR_NUMBER = 1;

QWORD MemChkBuf = 1;

QWORD HyperSpaceUsed = 1;

void main()
{

//defines-----------------

	 VPID               = 1;

	asm volatile("mov %0, rax"
	:"=a"(HYPERVISOR_base));

	asm volatile("mov %0, rbx"
	:"=b"(MCR_NUMBER));
	
	asm volatile("mov %0, rcx"
	:"=c"(MemChkBuf));

	asm volatile("mov %0, rdx"
	:"=d"(HyperSpaceUsed));


	 HYPERVISOR_size    = 0x100000;

	 HYPERVISOR_RAM_MAX = HYPERVISOR_base + HyperSpaceUsed;

	 TABLE_SZ           = 0x1000;
	 PAGE_SZ            = 0x1000;

// entry counts
	 PDE_CNT            = (HYPERVISOR_RAM_MAX / 0x200000);
	 PDPTE_CNT          = (PDE_CNT / 512);
	 PML4E_CNT          = 1;

// tables counts
	 PD_CNT             = (PDE_CNT / 512);
	 PDPT_CNT           = 1;
	 PML4_CNT           = 1;

	 EPT_RAM_MAX        = HYPERVISOR_base;
	 //EPT_RAM_MAX = HYPERVISOR_RAM_MAX;

// entry counts
	 EPT_PTE_CNT        = (EPT_RAM_MAX / 4096);
	 EPT_PDE_CNT        = (EPT_PTE_CNT / 512);
	 EPT_PDPTE_CNT      = 4;
	 EPT_PML4E_CNT      = 1;

// tables counts
	 EPT_PT_CNT         = (EPT_PTE_CNT / 512);
	 EPT_PD_CNT         = (EPT_PT_CNT  / 512);
	 EPT_PDPT_CNT       = 1;
	 EPT_PML4_CNT       = 1;

	 EPT_STRUCT_SIZEx8  = ((EPT_PML4_CNT * 512) + (EPT_PDPT_CNT * 512) + (EPT_PD_CNT * 512) + (EPT_PT_CNT * 512));

	 CACHE_TP_UC        = 0;
	 CACHE_TP_WC        = 1;
	 CACHE_TP_WT        = 4;
	 CACHE_TP_WP        = 5;
	 CACHE_TP_WB        = 6;

	 DEV_RAM_addr       = 0xBA647000;
	 DEV_RAM_size       = (0x100000000 - 0xBA647000);

	 VIDEO_RAM0_addr    = 0xA0000;
	 VIDEO_RAM0_size    = (0xC0000 - 0xA0000);

	 VIDEO_RAM1_addr    = 0xD0000000;
	 VIDEO_RAM1_size    = (0xD2000000 - 0xD0000000);

	 VIDEO_RAM2_addr    = 0xC0000000;
	 VIDEO_RAM2_size    = (0xD0000000 - 0xC0000000);

	 VIDEO_RAM3_addr    = 0xFA000000;
	 VIDEO_RAM3_size    = (0xFB000000 - 0xFA000000);

	 TIMER_val          = 0x0FFFFFFF;

	 CONFIG_ADDRESS     = 0xCF8;
	 CONFIG_DATA        = 0xCFC;
//---------------------------------------------------------


//memory_map-----------------------------------------------

	 STACK64_size       = 0x1000;
	 STACK64_addr       = (HYPERVISOR_base + HYPERVISOR_size + STACK64_size);  

	 STACK64i_size      = 0x1000;
	 STACK64i_addr      = (STACK64_addr + STACK64i_size);              

	 GDT64_size         = 0x1000;
	 GDT64_addr         = STACK64i_addr;                                

	 IDT64_size         = 0x1000;
	 IDT64_addr         = (GDT64_addr + GDT64_size);                    

	 PML4_size          = (PML4_CNT * 512 *8);
	 PML4_addr          = (IDT64_addr + IDT64_size);

	 PDPT_size          = (PDPT_CNT * 512 * 8);
	 PDPT_addr          = (PML4_addr + PML4_size);

	 PD_size            = (PD_CNT * 512 * 8);
	 PD_addr            = (PDPT_addr + PDPT_size);

	 TSS_size           = 0x1000;
	 TSS_addr           = (PD_addr + PD_size);

	 VMXON_size         = 0x1000;
	 VMXON_addr         = (TSS_addr + TSS_size);

	 VMCS_size          = 0x1000;
	 VMCS_addr          = (VMXON_addr + VMXON_size);

	 MSR_BMP_size       = 0x1000;
	 MSR_BMP_addr       = (VMCS_addr + VMCS_size);

	 IO_BMPA_size       = 0x1000;
	 IO_BMPA_addr       = (MSR_BMP_addr + MSR_BMP_size);

	 IO_BMPB_size       = 0x1000;
	 IO_BMPB_addr       = (IO_BMPA_addr + IO_BMPA_size);

	 MSR_EX_LDR_size    = 0x1000;
	 MSR_EX_LDR_addr    = (IO_BMPB_addr + IO_BMPB_size);       /* VM-exit MSR-load address */

	 MSR_EX_STR_size    = 0x1000;
	 MSR_EX_STR_addr    = (MSR_EX_LDR_addr + MSR_EX_LDR_size); /* VM-exit MSR-store address */

	 MSR_EN_LDR_size    = 0x1000;
	 MSR_EN_LDR_addr    = MSR_EX_STR_addr;                     /* VM-entry MSR-load address */

	 EPT_PML4_size      = (EPT_PML4_CNT * 512 * 8);
	 EPT_PML4_addr      = (MSR_EN_LDR_addr + MSR_EN_LDR_size);

	 EPT_PDPT_size      = (EPT_PDPT_CNT * 512 * 8);
	 EPT_PDPT_addr      = (EPT_PML4_addr + EPT_PML4_size);

	 EPT_PD_size        = (EPT_PD_CNT * 512 * 8);
	 EPT_PD_addr        = (EPT_PDPT_addr + EPT_PDPT_size);

	 EPT_PT_size        = (EPT_PT_CNT * 512 * 8);
	 EPT_PT_addr        = (EPT_PD_addr + EPT_PD_size);

	 VAR_size           = 0x1000;
	 VAR_addr           = (EPT_PT_addr + EPT_PT_size);

	 GUEST_REGS_size    = 8 * 8;                                /* rax, rcx, rdx, rbx, rsp, rbp, rsi , rdi */
	 GUEST_REGS_addr    = VAR_addr;


//-------------------------------------------------------------
	
	SerialPrintStr64("hypervisor base is:\r\n");
	SerialPrintDigit64(HYPERVISOR_base);
	SerialPrintStr64("\r\n");
	

	// setup stack pointer
    asm volatile("mov rsp, %0"::"r"(STACK64_addr));

    InitLongModeGdt();
    
	InitLongModeIdt(get_isr_addr());

    InitLongModeTSS();

    InitLongModePages();

    InitControlAndSegmenRegs();

    if(!CheckVMXConditions())
        while(1);
    
    InitVMX();
    
    InitGuestRegisterState();
    
    InitGuestNonRegisterState();

    InitHostStateArea();  

    InitEPT();
    
    if(!InitExecutionControlFields())
        while(1);
    
    if(!InitVMExitControl())
        while(1);
    
    if(!InitVMEntryControl())
        while(1);
    
    // enable PIC
    outb(0xA1, 0);
    outb(0x21, 0);

    VMLaunch();

    
    VMEnter_error();    
}
