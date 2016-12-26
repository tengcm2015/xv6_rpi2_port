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
#include "rpi_interrupts.h"

#define SYS_FREQ    250000000

static rpi_gpio_t* rpiGpio = (rpi_gpio_t*)RPI_GPIO_BASE;
static aux_t* auxillary = (aux_t*)AUX_BASE;

// flash LED with no absolute addr
void
led_flash_no_map(int interval, int recur)
{
    volatile unsigned int tim;
    volatile unsigned int* gpio = (unsigned int*)(PHYSIO + 0x200000);

    for (int i = 0; i < recur; i++)
    {
        for(tim = 0; tim < interval; tim++)
            ;
        gpio[11] = (1 << 15);

        for(tim = 0; tim < interval; tim++)
            ;
        gpio[8] = (1 << 15);

    }
}

// flash LED with interval microsec * 2 for recur times
void
led_flash(int interval, int recur)
{
    for(int i = 0; i < recur; i++)
    {
        delay(interval);
        LED_ON(rpiGpio);
        delay(interval);
        LED_OFF(rpiGpio);
    }
}

void
RPI_SetGpioHi( rpi_gpio_pin_t gpio )
{
    switch( gpio / 32 )
    {
        case 0:
            rpiGpio->GPSET0 = ( 1 << gpio );
            break;

        case 1:
            rpiGpio->GPSET1 = ( 1 << ( gpio - 32 ) );
            break;

        default:
            break;
    }
}

void
RPI_SetGpioLo( rpi_gpio_pin_t gpio )
{
    switch( gpio / 32 )
    {
        case 0:
            rpiGpio->GPCLR0 = ( 1 << gpio );
            break;

        case 1:
            rpiGpio->GPCLR1 = ( 1 << ( gpio - 32 ) );
            break;

        default:
            break;
    }
}

void
setgpioval(rpi_gpio_pin_t gpio, rpi_gpio_value_t value)
{
    if( ( value == RPI_IO_LO ) || ( value == RPI_IO_OFF ) )
        RPI_SetGpioLo( gpio );
    else if( ( value == RPI_IO_HI ) || ( value == RPI_IO_ON ) )
        RPI_SetGpioHi( gpio );
}


void
setgpiofunc(rpi_gpio_pin_t gpio, rpi_gpio_alt_function_t func)
{
    rpi_reg_rw_t* fsel_reg = &((rpi_reg_rw_t*)rpiGpio)[ gpio / 10 ];
    rpi_reg_rw_t fsel_data = *fsel_reg;
    fsel_data &= ~( FS_MASK << ( ( gpio % 10 ) * 3 ) );
    fsel_data |= (func << ( ( gpio % 10 ) * 3 ) );
    *fsel_reg = fsel_data;
}


void
uartputc(uint c)
{
    if(c == '\n') {
        while( ( auxillary->MU_LSR & AUX_MULSR_TX_EMPTY ) == 0 )
            ;
        auxillary->MU_IO = 0x0d; // add CR before LF
    }

    /* Wait until the UART has an empty space in the FIFO */
    while( ( auxillary->MU_LSR & AUX_MULSR_TX_EMPTY ) == 0 )
        ;
    /* Write the character to the FIFO for transmission */
    auxillary->MU_IO = c;
}

static int
uartgetc(void)
{
    if (auxillary->MU_LSR & AUX_MULSR_DATA_READY)
        return auxillary->MU_IO;
    else
        return -1;
}

void
miniuartintr(void)
{
    consoleintr(uartgetc);
}

void
uartinit(int baud)
{
    /* Write 1 to the LED init nibble in the Function Select GPIO
    peripheral register to enable LED pin as an output */
    // rpiGpio->LED_GPFSEL |= LED_GPFBIT;
    led_flash(500000, 5); // debug

    auxillary->ENABLES = 1;
    auxillary->MU_CNTL = 0;
    auxillary->MU_LCR  = AUX_MULCR_8BIT_MODE;
    auxillary->MU_MCR  = 0;
    auxillary->MU_IER  = 0x1;
    auxillary->MU_IIR  = 0xC7;
    auxillary->MU_BAUD = ( SYS_FREQ / ( 8 * baud ) ) - 1;

    setgpiofunc(RPI_GPIO14, FS_ALT5); // gpio 14, alt 5
    setgpiofunc(RPI_GPIO15, FS_ALT5); // gpio 15, alt 5

    rpiGpio->GPPUD = 0;
    delay(10);
    rpiGpio->GPPUDCLK0 = (1 << 14) | (1 << 15);
    delay(10);
    rpiGpio->GPPUDCLK0 = 0;

    auxillary->MU_CNTL = AUX_MUCNTL_RX_ENABLE | AUX_MUCNTL_TX_ENABLE;

    // enableirqminiuart();
    RPI_GetIrqController()->Enable_IRQs_1 |= (1 << 29);

}
