#ifndef RPI_BASE_H
#define RPI_BASE_H

#include "types.h"
#include "memlayout.h"
#include "rpi_aux.h"
#include "rpi_gpio.h"

#define PERIPHERAL_BASE DEVSPACE

typedef volatile u32 rpi_reg_rw_t;
typedef volatile const u32 rpi_reg_ro_t;
typedef volatile u32 rpi_reg_wo_t;

typedef volatile u64 rpi_wreg_rw_t;
typedef volatile const u64 rpi_wreg_ro_t;

#endif
