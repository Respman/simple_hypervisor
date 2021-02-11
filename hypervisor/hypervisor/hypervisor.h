#pragma once

#include "types.h"


//defines-----------------

extern QWORD VPID;

extern QWORD HYPERVISOR_base;

extern QWORD HYPERVISOR_size;

extern QWORD HYPERVISOR_RAM_MAX;


extern QWORD TABLE_SZ;
extern QWORD PAGE_SZ;

// entry counts
extern QWORD PDE_CNT;
extern QWORD PDPTE_CNT;
extern QWORD PML4E_CNT;

// tables counts
extern QWORD PD_CNT;
extern QWORD PDPT_CNT;
extern QWORD PML4_CNT;

extern QWORD EPT_RAM_MAX;

// entry counts
extern QWORD EPT_PTE_CNT;
extern QWORD EPT_PDE_CNT;
extern QWORD EPT_PDPTE_CNT;
extern QWORD EPT_PML4E_CNT;

// tables counts
extern QWORD EPT_PT_CNT;
extern QWORD EPT_PD_CNT;
extern QWORD EPT_PDPT_CNT;
extern QWORD EPT_PML4_CNT;

extern QWORD EPT_STRUCT_SIZEx8;

extern QWORD CACHE_TP_UC;
extern QWORD CACHE_TP_WC;
extern QWORD CACHE_TP_WT;
extern QWORD CACHE_TP_WP;
extern QWORD CACHE_TP_WB;

extern QWORD DEV_RAM_addr;
extern QWORD DEV_RAM_size;

extern QWORD VIDEO_RAM0_addr;
extern QWORD VIDEO_RAM0_size;

extern QWORD VIDEO_RAM1_addr;
extern QWORD VIDEO_RAM1_size;

extern QWORD VIDEO_RAM2_addr;
extern QWORD VIDEO_RAM2_size;

extern QWORD VIDEO_RAM3_addr;
extern QWORD VIDEO_RAM3_size;

extern QWORD TIMER_val;

extern QWORD CONFIG_ADDRESS;
extern QWORD CONFIG_DATA;
//---------------------------------------------------------


//memory_map-----------------------------------------------

extern QWORD STACK64_size;
extern QWORD STACK64_addr;  

extern QWORD STACK64i_size;
extern QWORD STACK64i_addr;              

extern QWORD GDT64_size;
extern QWORD GDT64_addr;                                

extern QWORD IDT64_size;
extern QWORD IDT64_addr;                    

extern QWORD PML4_size;
extern QWORD PML4_addr;

extern QWORD PDPT_size;
extern QWORD PDPT_addr;

extern QWORD PD_size;
extern QWORD PD_addr;

extern QWORD TSS_size;
extern QWORD TSS_addr;

extern QWORD VMXON_size;
extern QWORD VMXON_addr;

extern QWORD VMCS_size;
extern QWORD VMCS_addr;

extern QWORD MSR_BMP_size;
extern QWORD MSR_BMP_addr;

extern QWORD IO_BMPA_size;
extern QWORD IO_BMPA_addr;

extern QWORD IO_BMPB_size;
extern QWORD IO_BMPB_addr;

extern QWORD MSR_EX_LDR_size;
extern QWORD MSR_EX_LDR_addr;       /* VM-exit MSR-load address */

extern QWORD MSR_EX_STR_size;
extern QWORD MSR_EX_STR_addr; /* VM-exit MSR-store address */

extern QWORD MSR_EN_LDR_size;
extern QWORD MSR_EN_LDR_addr;                     /* VM-entry MSR-load address */

extern QWORD EPT_PML4_size;
extern QWORD EPT_PML4_addr;

extern QWORD EPT_PDPT_size;
extern QWORD EPT_PDPT_addr;

extern QWORD EPT_PD_size;
extern QWORD EPT_PD_addr;

extern QWORD EPT_PT_size;
extern QWORD EPT_PT_addr;

extern QWORD VAR_size;
extern QWORD VAR_addr;

extern QWORD GUEST_REGS_size;                                /* rax, rcx, rdx, rbx, rsp, rbp, rsi , rdi */
extern QWORD GUEST_REGS_addr;
extern QWORD HyperSpaceUsed;

//-------------------------------------------------------------
extern QWORD MCR_NUMBER;
extern QWORD MemChkBuf;
extern QWORD HyperSpaceUsed;
//void main(void);
