#include "vmexit.h"
#include "hypervisor.h"
#include "types.h"
#include "msr_regs.h"
#include "inline_asm.h"
#include "serialport_log64.h"
#include "vmx.h"

static void CPUID_hndlr()
{
    QWORD *ptrGUEST_REGS = (QWORD *)GUEST_REGS_addr;
    
    DWORD eax_v, ecx_v, edx_v, ebx_v;
    DWORD cpuid_init = (DWORD)ptrGUEST_REGS[0];

    if(cpuid_init == 0x80000002){
        eax_v = 0x20736b4b;
        ebx_v = 0x64757453; 
        ecx_v = 0x6F432079;
        edx_v = 0x293A6572;
    }
    else if(cpuid_init == 0x80000003 || cpuid_init == 0x80000004){
        eax_v = 0;
        ebx_v = 0; 
        ecx_v = 0;
        edx_v = 0;
    }
    else
        cpuid(cpuid_init, &eax_v, &ecx_v, &edx_v, &ebx_v);
    
    ptrGUEST_REGS[0] = eax_v;
    ptrGUEST_REGS[1] = ecx_v;
    ptrGUEST_REGS[2] = edx_v;
    ptrGUEST_REGS[3] = ebx_v;

    QWORD vm_rd_val;
    /* add cpuid length to Guest RIP 0000681EH */
    vm_rd_val = vmread(0x681E);
    vm_rd_val += 2;
    vmwrite(0x681E, vm_rd_val);
    
    return;
}

/*---------------------------------------------------------------------------------------------------*/

static bool CR_Reg_hndlr()
{
    
    QWORD vm_rd_val = vmread(0x6400);                            /* Exit qualification */
    
    SerialPrintStr64("Access to CR\r\n");
    SerialPrintStr64("Exit qualification\r\n");
    SerialPrintDigit64(vm_rd_val);
    SerialPrintStr64("\r\n");

    /* get Guest RIP 0000681EH */
    SerialPrintStr64("RIP:\r\n");
    QWORD vm_rd_val_RIP = vmread(0x681E);
    SerialPrintDigit64(vm_rd_val_RIP);
    SerialPrintStr64("\r\n");

    QWORD *ptrGUEST_REGS = (QWORD *)GUEST_REGS_addr;
    BYTE gp_reg_n = (BYTE)((vm_rd_val >> 8) & 0xF);              /* [11:8] - Number of general-purpose register */
    
    SerialPrintStr64("val of reg_n:\r\n");
    SerialPrintDigit64(gp_reg_n);
    SerialPrintStr64("\r\n");

    QWORD gp_reg_val;
    if(gp_reg_n == 4)                                            /* RSP case, it's strange, but still possible */
        gp_reg_val = vmread(0x681C);                             /* Guest RSP */
    else
        gp_reg_val = ptrGUEST_REGS[gp_reg_n];
    
    //gp_reg_val = tmp_rax;

    SerialPrintStr64("val of reg:\r\n");
    SerialPrintDigit64(gp_reg_val);
    SerialPrintStr64("\r\n");

    switch (vm_rd_val & 0xFF)
    {
        case 0:{
            SerialPrintStr64("Access to CR0\r\n");
    
            SerialPrintStr64("previous CR0:\r\n");
            SerialPrintDigit64(vmread(0x6800));
            SerialPrintStr64("\r\n");

            QWORD msr_rd_val = rdmsr(IA32_VMX_CR0_FIXED0);
            QWORD cr0_fixed0 = msr_rd_val & 0x1FFFFFFE;            // clear PG, PE, PE, CD, NW

            msr_rd_val = rdmsr(IA32_VMX_CR0_FIXED1);
            QWORD cr0_fixed1 = msr_rd_val & 0xFFFFFFFF;
    
            gp_reg_val = gp_reg_val & 
                         cr0_fixed1 | 
                         cr0_fixed0;

            /* get Guest CR0 00006800H */
            SerialPrintStr64("CR0:\r\n");
            SerialPrintDigit64(gp_reg_val & 0x9FFFFFFF);
            SerialPrintStr64("\r\n");

            vmwrite(0x6800, gp_reg_val & 0x9FFFFFFF);                    /* clear CD and WT */

            break;
        }
        case 4:{
            SerialPrintStr64("Access to CR4\r\n");
            SerialPrintStr64("previous CR4:\r\n");
            SerialPrintDigit64(vmread(0x6804));
            SerialPrintStr64("\r\n");


            QWORD msr_rd_val = rdmsr(IA32_VMX_CR4_FIXED0);
            QWORD cr4_fixed0 = msr_rd_val & 0xFFFFFFFF;

            msr_rd_val = rdmsr(IA32_VMX_CR4_FIXED1);
            QWORD cr4_fixed1 = msr_rd_val & 0xFFFFFFFF;
    
            gp_reg_val = gp_reg_val & 
                         cr4_fixed1 | 
                         cr4_fixed0;
            
            gp_reg_val = (gp_reg_val | (1ULL<<13)); //i&0xFFFFFFDF; //disable PAE

            /* get Guest CR4 00006804H */
            SerialPrintStr64("CR4:\r\n");
            SerialPrintDigit64(gp_reg_val);
            SerialPrintStr64("\r\n");

            vmwrite(0x6804, gp_reg_val);
            
            break;
        }
        default:{
            SerialPrintStr64("Access to CR: unexpected case\r\n");
            return FALSE;
        }
    }
    /* add mov CRn length to Guest RIP 0000681EH */
    vm_rd_val = vmread(0x681E);                                    
    vm_rd_val += 3;
    vmwrite(0x681E, vm_rd_val);

    return TRUE;
   
}

/*---------------------------------------------------------------------------------------------------*/

static void XSETBV_hndlr()
{
    QWORD *ptrGUEST_REGS = (QWORD *)GUEST_REGS_addr;

    xsetbv((DWORD)ptrGUEST_REGS[0], (DWORD)ptrGUEST_REGS[1], (DWORD)ptrGUEST_REGS[2]);
    
    /* add xsetbv length to Guest RIP 0000681EH */
    QWORD vm_rd_val = vmread(0x681E);                                    
    vm_rd_val += 3;
    vmwrite(0x681E, vm_rd_val);
    
    return;
}

/*---------------------------------------------------------------------------------------------------*/

//sourse: https://revers.engineering/day-5-vmexits-interrupts-cpuid-emulation/

enum __vmexit_reason_e
{
    vmexit_nmi = 0,
    vmexit_ext_int,
    vmexit_triple_fault,
    vmexit_init_signal,
    vmexit_sipi,
    vmexit_smi,
    vmexit_other_smi,
    vmexit_interrupt_window,
    vmexit_nmi_window,
    vmexit_task_switch,
    vmexit_cpuid,
    vmexit_getsec,
    vmexit_hlt,
    vmexit_invd,
    vmexit_invlpg,
    vmexit_rdpmc,
    vmexit_rdtsc,
    vmexit_rsm,
    vmexit_vmcall,
    vmexit_vmclear,
    vmexit_vmlaunch,
    vmexit_vmptrld,
    vmexit_vmptrst,
    vmexit_vmread,
    vmexit_vmresume,
    vmexit_vmwrite,
    vmexit_vmxoff,
    vmexit_vmxon,
    vmexit_control_register_access,
    vmexit_mov_dr,
    vmexit_io_instruction,
    vmexit_rdmsr,
    vmexit_wrmsr,
    vmexit_vmentry_failure_due_to_guest_state,
    vmexit_vmentry_failure_due_to_msr_loading,
    vmexit_mwait = 36,
    vmexit_monitor_trap_flag,
    vmexit_monitor = 39,
    vmexit_pause,
    vmexit_vmentry_failure_due_to_machine_check_event,
    vmexit_tpr_below_threshold = 43,
    vmexit_apic_access,
    vmexit_virtualized_eoi,
    vmexit_access_to_gdtr_or_idtr,
    vmexit_access_to_ldtr_or_tr,
    vmexit_ept_violation,
    vmexit_ept_misconfiguration,
    vmexit_invept,
    vmexit_rdtscp,
    vmexit_vmx_preemption_timer_expired,
    vmexit_invvpid,
    vmexit_wbinvd,
    vmexit_xsetbv,
    vmexit_apic_write,
    vmexit_rdrand,
    vmexit_invpcid,
    vmexit_vmfunc,
    vmexit_encls,
    vmexit_rdseed,
    vmexit_pml_full,
    vmexit_xsaves,
    vmexit_xrstors,
};

void VMEXIT_handler()
{
    QWORD *ptr;
    QWORD vm_rd_val;
    QWORD *ptrGUEST_REGS;
    
    asm volatile(
                 "mov %0, rax"
	:"=a"(ptr));
    
    ptrGUEST_REGS = (QWORD *)GUEST_REGS_addr;

    for (int i=0; i < 8; i++){
        ptrGUEST_REGS[i] = ptr[7-i];
    }

    /* determine exit reason 00004402H */
    vm_rd_val = vmread(0x4402);
    
    switch(vm_rd_val & 0xFFFF)
    {
        case vmexit_nmi:{
            SerialPrintStr64("VMEXIT: nmi\r\n");
            while(1);
        }

        case vmexit_ext_int:{
            SerialPrintStr64("VMEXIT: ext_int\r\n");
            while(1);
        }
    
        case vmexit_triple_fault:{
            SerialPrintStr64("VMEXIT: triple_fault\r\n");
            
            /* get Guest RIP 0000681EH */
            SerialPrintStr64("RIP:\r\n");
            QWORD vm_rd_val_RIP = vmread(0x681E);
            SerialPrintDigit64(vm_rd_val_RIP);
            SerialPrintStr64("\r\n");

            /* get Guest CR0 0x6800 */
            SerialPrintStr64("CR0:\r\n");
            QWORD vm_rd_val_CR0 = vmread(0x6800);
            SerialPrintDigit64(vm_rd_val_CR0);
            SerialPrintStr64("\r\n");

            
            /* get Guest CR3 00006802H */
            SerialPrintStr64("CR3:\r\n");
            QWORD vm_rd_val_CR3 = vmread(0x6802);
            SerialPrintDigit64(vm_rd_val_CR3);
            SerialPrintStr64("\r\n");

            
            /* get Guest CR4 00006804H */
            SerialPrintStr64("CR4:\r\n");
            QWORD vm_rd_val_CR4 = vmread(0x6804);
            SerialPrintDigit64(vm_rd_val_CR4);
            SerialPrintStr64("\r\n");

            
            /* get Guest RSP 0000681CH */
            SerialPrintStr64("RSP:\r\n");
            QWORD vm_rd_val_RSP = vmread(0x681c);
            SerialPrintDigit64(vm_rd_val_RSP);
            SerialPrintStr64("\r\n");

            while(1);
        }
    
        case vmexit_init_signal:{
            SerialPrintStr64("VMEXIT: init_signal\r\n");
            while(1);
        }
    
        case vmexit_sipi:{
            SerialPrintStr64("VMEXIT: sipi\r\n");
            while(1);
        }
    
        case vmexit_smi:{
            SerialPrintStr64("VMEXIT: smi\r\n");
            while(1);
        }
    
        case vmexit_other_smi:{
            SerialPrintStr64("VMEXIT: smi\r\n");
            while(1);
        }
    
        case vmexit_interrupt_window:{
            SerialPrintStr64("VMEXIT: interrupt_window\r\n");
            while(1);
        }
    
        case vmexit_nmi_window:{
            SerialPrintStr64("VMEXIT: nmi_window\r\n");
            while(1);
        }
    
       case vmexit_task_switch:{
            SerialPrintStr64("VMEXIT: task_switch\r\n");
            while(1);
        }

    /* CPUID. Guest software attempted to execute CPUID. */
        case vmexit_cpuid:{
            SerialPrintStr64("[*] VMEXIT: CPUID\r\n");
            CPUID_hndlr();
            break;
        }
    
       case vmexit_getsec:{
            SerialPrintStr64("VMEXIT: getsec\r\n");
            while(1);
        }
    
       case vmexit_hlt:{
            SerialPrintStr64("VMEXIT: hlt\r\n");
            while(1);
        }
    
       case vmexit_invd:{
            SerialPrintStr64("VMEXIT: invd\r\n");
            while(1);
        }
    
       case vmexit_invlpg:{
            SerialPrintStr64("VMEXIT: invlpg\r\n");
            while(1);
        }
    
       case vmexit_rdpmc:{
            SerialPrintStr64("VMEXIT: rdpmc\r\n");
            while(1);
        }
    
       case vmexit_rdtsc:{
            SerialPrintStr64("VMEXIT: rdtsc\r\n");
            while(1);
        }
    
       case vmexit_rsm:{
            SerialPrintStr64("VMEXIT: rsm\r\n");
            while(1);
        }
    
       case vmexit_vmcall:{
            SerialPrintStr64("VMEXIT: vmcall\r\n");
            while(1);
        }
    
       case vmexit_vmclear:{
            SerialPrintStr64("VMEXIT: vmclear\r\n");
            while(1);
        }
    
       case vmexit_vmlaunch:{
            SerialPrintStr64("VMEXIT: vmlaunch\r\n");
            while(1);
        }
    
       case vmexit_vmptrld:{
            SerialPrintStr64("VMEXIT: vmptrld\r\n");
            while(1);
        }
    
       case vmexit_vmptrst:{
            SerialPrintStr64("VMEXIT: vmptrst\r\n");
            while(1);
        }
    
       case vmexit_vmread:{
            SerialPrintStr64("VMEXIT: vmread\r\n");
            while(1);
        }
    
       case vmexit_vmresume:{
            SerialPrintStr64("VMEXIT: vmresume\r\n");
            while(1);
        }
    
       case vmexit_vmwrite:{
            SerialPrintStr64("VMEXIT: vmwrite\r\n");
            while(1);
        }
    
       case vmexit_vmxoff:{
            SerialPrintStr64("VMEXIT: vmxoff\r\n");
            while(1);
        }
    
       case vmexit_vmxon:{
            SerialPrintStr64("VMEXIT: vmxon\r\n");
            while(1);
        }
    
    /* Control-register accesses. */
        case vmexit_control_register_access:{
            SerialPrintStr64("VMEXIT: control_register_access\r\n");
            if(!CR_Reg_hndlr()){
                while(1);
            }
            else
                break;
        }   
    
       case vmexit_mov_dr:{
            SerialPrintStr64("VMEXIT: mov_dr\r\n");
            while(1);
        }
    
       case vmexit_io_instruction:{
            SerialPrintStr64("VMEXIT: io_instruction\r\n");
            while(1);
        }
    
       case vmexit_rdmsr:{
            SerialPrintStr64("VMEXIT: rdmsr\r\n");
            while(1);
        }
    
       case vmexit_wrmsr:{
            SerialPrintStr64("VMEXIT: wrmsr\r\n");
            while(1);
        }
    
       case vmexit_vmentry_failure_due_to_guest_state:{
            SerialPrintStr64("VMEXIT: vmentry_failure_due_to_guest_state\r\n");
            
            /* get Guest RIP 0000681EH */
            SerialPrintStr64("RIP:\r\n");
            QWORD vm_rd_val_RIP = vmread(0x681E);
            SerialPrintDigit64(vm_rd_val_RIP);
            SerialPrintStr64("\r\n");
            
            /* get Guest CR0 0x6800 */
            SerialPrintStr64("CR0:\r\n");
            QWORD vm_rd_val_CR0 = vmread(0x6800);
            SerialPrintDigit64(vm_rd_val_CR0);
            SerialPrintStr64("\r\n");

            
            /* get Guest CR3 00006802H */
            SerialPrintStr64("CR3:\r\n");
            QWORD vm_rd_val_CR3 = vmread(0x6802);
            SerialPrintDigit64(vm_rd_val_CR3);
            SerialPrintStr64("\r\n");

            
            /* get Guest CR4 00006804H */
            SerialPrintStr64("CR4:\r\n");
            QWORD vm_rd_val_CR4 = vmread(0x6804);
            SerialPrintDigit64(vm_rd_val_CR4);
            SerialPrintStr64("\r\n");

            
            /* get Guest RSP 0000681CH */
            SerialPrintStr64("RSP:\r\n");
            QWORD vm_rd_val_RSP = vmread(0x681c);
            SerialPrintDigit64(vm_rd_val_RSP);
            SerialPrintStr64("\r\n");

            while(1);
        }
    
       case vmexit_vmentry_failure_due_to_msr_loading:{
            SerialPrintStr64("VMEXIT: vmentry_failure_due_to_msr_loading\r\n");
            while(1);
        }
    
       case vmexit_mwait:{
            SerialPrintStr64("VMEXIT: mwait\r\n");
            while(1);
        }
    
       case vmexit_monitor_trap_flag:{
            SerialPrintStr64("VMEXIT: monitor_trap_flag\r\n");
            while(1);
        }
    
       case vmexit_monitor:{
            SerialPrintStr64("VMEXIT: monitor\r\n");
            while(1);
        }
    
       case vmexit_pause:{
            SerialPrintStr64("VMEXIT: pause\r\n");
            while(1);
        }
    
       case vmexit_vmentry_failure_due_to_machine_check_event:{
            SerialPrintStr64("VMEXIT: vmentry_failure_due_to_machine_check_event\r\n");
            while(1);
        }
    
       case vmexit_tpr_below_threshold:{
            SerialPrintStr64("VMEXIT: tpr_below_threshold\r\n");
            while(1);
        }
    
       case vmexit_apic_access:{
            SerialPrintStr64("VMEXIT: apic_access\r\n");
            while(1);
        }
    
       case vmexit_virtualized_eoi:{
            SerialPrintStr64("VMEXIT: virtualized_eoi\r\n");
            while(1);
        }
    
       case vmexit_access_to_gdtr_or_idtr:{
            SerialPrintStr64("VMEXIT: access_to_gdtr_or_idtr\r\n");
            while(1);
        }
    
       case vmexit_access_to_ldtr_or_tr:{
            SerialPrintStr64("VMEXIT: access_to_ldtr_or_tr\r\n");
            while(1);
        }
    
       case vmexit_ept_violation:{
            SerialPrintStr64("VMEXIT: ept_violation\r\n");
            
            /*SerialPrintStr64("IA32_VMX_EPT_VPID_CAP MSR:\r\n");
            QWORD VPID_CAP = rdmsr(0x48c)&(1ull<<);
            SerialPrintDigit64(VPID_CAP);
            SerialPrintStr64("\r\n");*/

    /* get Guest RIP 0000681EH */
            SerialPrintStr64("RIP:\r\n");
            QWORD vm_rd_val_RIP = vmread(0x681E);
            SerialPrintDigit64(vm_rd_val_RIP);
            SerialPrintStr64("\r\n");
            
            /* get Guest CR0 0x6800 */
            SerialPrintStr64("CR0:\r\n");
            QWORD vm_rd_val_CR0 = vmread(0x6800);
            SerialPrintDigit64(vm_rd_val_CR0);
            SerialPrintStr64("\r\n");

            
            /* get Guest CR3 00006802H */
            SerialPrintStr64("CR3:\r\n");
            QWORD vm_rd_val_CR3 = vmread(0x6802);
            SerialPrintDigit64(vm_rd_val_CR3);
            SerialPrintStr64("\r\n");

            
            /* get Guest CR4 00006804H */
            SerialPrintStr64("CR4:\r\n");
            QWORD vm_rd_val_CR4 = vmread(0x6804);
            SerialPrintDigit64(vm_rd_val_CR4);
            SerialPrintStr64("\r\n");

            
            /* get Guest RSP 0000681CH */
            SerialPrintStr64("RSP:\r\n");
            QWORD vm_rd_val_RSP = vmread(0x681c);
            SerialPrintDigit64(vm_rd_val_RSP);
            SerialPrintStr64("\r\n");

            while(1);
        }
    
       case vmexit_ept_misconfiguration:{
            SerialPrintStr64("VMEXIT: ept_misconfiguration\r\n");
            while(1);
        }
    
       case vmexit_invept:{
            SerialPrintStr64("VMEXIT: invept\r\n");
            while(1);
        }
    
       case vmexit_rdtscp:{
            SerialPrintStr64("VMEXIT: rdtscp\r\n");
            while(1);
        }
    
       case vmexit_vmx_preemption_timer_expired:{
            SerialPrintStr64("VMEXIT: vmx_preemption_timer_expired\r\n");
            while(1);
        }
    
       case vmexit_invvpid:{
            SerialPrintStr64("VMEXIT: invvpid\r\n");
            while(1);
        }
    
       case vmexit_wbinvd:{
            SerialPrintStr64("VMEXIT: wbinvd\r\n");
            while(1);
        }

    /* XSETBV. Guest software attempted to execute XSETBV */
        case vmexit_xsetbv:{
            SerialPrintStr64("VMEXIT: xsetbv\r\n");
            XSETBV_hndlr();
            break;
        }    
    
       case vmexit_apic_write:{
            SerialPrintStr64("VMEXIT: apic_write\r\n");
            while(1);
        }
    
       case vmexit_rdrand:{
            SerialPrintStr64("VMEXIT: rdrand\r\n");
            while(1);
        }
    
       case vmexit_invpcid:{
            SerialPrintStr64("VMEXIT: invpcid\r\n");
            while(1);
        }
    
       case vmexit_vmfunc:{
            SerialPrintStr64("VMEXIT: vmfunc\r\n");
            while(1);
        }
    
       case vmexit_encls:{
            SerialPrintStr64("VMEXIT: encls\r\n");
            while(1);
        }
    
       case vmexit_rdseed:{
            SerialPrintStr64("VMEXIT: rdseed\r\n");
            while(1);
        }
    
       case vmexit_pml_full:{
            SerialPrintStr64("VMEXIT: pml_full\r\n");
            while(1);
        }
    
       case vmexit_xsaves:{
            SerialPrintStr64("VMEXIT: xsaves\r\n");
            while(1);
        }
    
       case vmexit_xrstors:{
            SerialPrintStr64("VMEXIT: xrstors\r\n");
            while(1);
        }

        default:{
            SerialPrintStr64("VMEXIT: unexpected case\r\n");
            SerialPrintDigit64(vm_rd_val);
            SerialPrintStr64("\r\n");
            while(1);
        }
    }

    for (int i=0; i < 8; i++){
        ptr[7-i] = ptrGUEST_REGS[i];
    }

    return;
}

/*---------------------------------------------------------------------------------------------------*/

void VMEnter_error()
{

    SerialPrintStr64("VMEnter error\r\n");    
    QWORD vm_rd_val = vmread(0x6400);                            /* Exit qualification */
    SerialPrintDigit64(vm_rd_val);
    
    vm_rd_val = vmread(0x4402);                                  /* Exit reason */
    SerialPrintDigit64(vm_rd_val);
    
    while(1);
}
