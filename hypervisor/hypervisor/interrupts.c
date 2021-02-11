#include "types.h"
#include "hypervisor.h"
#include "interrupts.h"
#include "serialport_log64.h"

void common_handler()
{
    SerialPrintStr64("interrupt occurred!");
    return;
}
