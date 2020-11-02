#include "rpi.h"
#include "assert.h"
#include "helper_macros.h"
#include <stdint.h>

// symbol created by libpi/memmap, placed at the end
// of all the code/data in a pi binary file.
extern unsigned int __heap_start__;