/*****************************************************************
*       uart.c
*       by Zhiyi Huang, hzy@cs.otago.ac.nz
*       University of Otago
*
********************************************************************/



#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "traps.h"
#include "arm.h"

#include "rpi_gpio.h"
#include "rpi_aux.h"

#define SYS_FREQ    250000000

static rpi_gpio_t* rpiGpio = (rpi_gpio_t*)RPI_GPIO_BASE;
static aux_t* auxillary = (aux_t*)AUX_BASE;

// #define GPFSEL0         0xFE200000
// #define GPFSEL1         0xFE200004
// #define GPFSEL2         0xFE200008
// #define GPFSEL3         0xFE20000C
// #define GPFSEL4         0xFE200010
// #define GPFSEL5         0xFE200014
// #define GPSET0          0xFE20001C
// #define GPSET1          0xFE200020
// #define GPCLR0          0xFE200028
// #define GPCLR1          0xFE20002C
// #define GPPUD           0xFE200094
// #define GPPUDCLK0       0xFE200098
// #define GPPUDCLK1       0xFE20009C
//
// #define AUX_IRQ             0xFE215000
// #define AUX_ENABLES         0xFE215004
// #define AUX_MU_IO_REG       0xFE215040
// #define AUX_MU_IER_REG      0xFE215044
// #define AUX_MU_IIR_REG      0xFE215048
// #define AUX_MU_LCR_REG      0xFE21504C
// #define AUX_MU_MCR_REG      0xFE215050
// #define AUX_MU_LSR_REG      0xFE215054
// #define AUX_MU_MSR_REG      0xFE215058
// #define AUX_MU_SCRATCH      0xFE21505C
// #define AUX_MU_CNTL_REG     0xFE215060
// #define AUX_MU_STAT_REG     0xFE215064
// #define AUX_MU_BAUD_REG     0xFE215068

void
setgpioval(uint func, uint val)
{
	uint sel, ssel, rsel;

	if(func > 53) return;
	sel = func >> 5;
	ssel = rpiGpio->GPSET0 + (sel << 2);
	rsel = rpiGpio->GPCLR0 + (sel << 2);
	sel = func & 0x1f;
	if(val == 0) outw(rsel, 1<<sel);
	else outw(ssel, 1<<sel);
}


void
setgpiofunc(uint func, uint alt)
{
	uint sel, data, shift;

	if(func > 53) return;
	sel = 0;
	while (func > 10) {
		func = func - 10;
		sel++;
	}
	sel = (sel << 2) + rpiGpio->GPFSEL0;
	data = inw(sel);
	shift = func + (func << 1);
	data &= ~(7 << shift);
	data |= alt << shift;
	outw(sel, data);
}


void
uartputc(uint c)
{
	if(c=='\n') {
		while(1) if(inw(auxillary->MU_LSR) & 0x20) break;
		outw(auxillary->MU_IO, 0x0d); // add CR before LF
	}
	while(1) if(inw(auxillary->MU_LSR) & 0x20) break;
	outw(auxillary->MU_IO, c);
}

static int
uartgetc(void)
{
	if(inw(auxillary->MU_LSR)&0x1) return inw(auxillary->MU_IO);
	else return -1;
}

void
enableirqminiuart(void)
{
	intctrlregs *ip;

	ip = (intctrlregs *)INT_REGS_BASE;
	ip->gpuenable[0] |= (1 << 29);   // enable the miniuart through Aux
}


void
miniuartintr(void)
{
	consoleintr(uartgetc);
}

void
uartinit(int baud)
{
	outw(auxillary->ENABLES, AUX_ENA_MINIUART);
	outw(auxillary->MU_CNTL, 0);
	outw(auxillary->MU_LCR, AUX_MULCR_8BIT_MODE);
	outw(auxillary->MU_MCR, 0);
	outw(auxillary->MU_IER, 0x1);
	outw(auxillary->MU_IIR, 0xC7);
	outw(auxillary->MU_BAUD, ( SYS_FREQ / ( 8 * baud ) ) - 1);

	setgpiofunc(RPI_GPIO14, FS_ALT5);
	setgpiofunc(RPI_GPIO15, FS_ALT5);

	outw(rpiGpio->GPPUD, 0);
	delay(10);
	outw(rpiGpio->GPPUDCLK0, (1 << 14) | (1 << 15) );
	delay(10);
	outw(rpiGpio->GPPUDCLK0, 0);

	outw(auxillary->MU_CNTL, AUX_MUCNTL_TX_ENABLE);
	enableirqminiuart();
}
