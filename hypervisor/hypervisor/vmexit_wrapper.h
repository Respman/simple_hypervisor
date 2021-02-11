#pragma once
#include "types.h"

QWORD get_vmexit_addr(void);
void vmexit_wrapper(void);