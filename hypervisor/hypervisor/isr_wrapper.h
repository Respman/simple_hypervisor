#pragma once
#include "types.h"

QWORD get_isr_addr(void);
void isr_wrapper(void);