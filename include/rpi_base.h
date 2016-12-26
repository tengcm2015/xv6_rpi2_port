#ifndef RPI_BASE_H
#define RPI_BASE_H

#ifdef RPI2
    #define PERIPHERAL_BASE     0x3F000000
#else
    #define PERIPHERAL_BASE     0x20000000
#endif

#ifndef __ASSEMBLER__

#include "memlayout.h"

typedef volatile uint32 rpi_reg_rw_t;
typedef volatile const uint32 rpi_reg_ro_t;
typedef volatile uint32 rpi_reg_wo_t;

typedef volatile uint64 rpi_wreg_rw_t;
typedef volatile const uint64 rpi_wreg_ro_t;

#endif

#endif
