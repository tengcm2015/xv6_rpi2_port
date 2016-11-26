#ifndef RPI_AUX_H
#define RPI_AUX_H

#include "rpi_base.h"

#define AUX_BASE ( DEVSPACE + 0x215000 )

#define AUX_ENA_MINIUART            ( 1 << 0 )
#define AUX_ENA_SPI1                ( 1 << 1 )
#define AUX_ENA_SPI2                ( 1 << 2 )

#define AUX_IRQ_SPI2                ( 1 << 2 )
#define AUX_IRQ_SPI1                ( 1 << 1 )
#define AUX_IRQ_MU                  ( 1 << 0 )

#define AUX_MULCR_8BIT_MODE         ( 3 << 0 )  /* See errata for this value */
#define AUX_MULCR_BREAK             ( 1 << 6 )
#define AUX_MULCR_DLAB_ACCESS       ( 1 << 7 )

#define AUX_MUMCR_RTS               ( 1 << 1 )

#define AUX_MULSR_DATA_READY        ( 1 << 0 )
#define AUX_MULSR_RX_OVERRUN        ( 1 << 1 )
#define AUX_MULSR_TX_EMPTY          ( 1 << 5 )
#define AUX_MULSR_TX_IDLE           ( 1 << 6 )

#define AUX_MUMSR_CTS               ( 1 << 5 )

#define AUX_MUCNTL_RX_ENABLE        ( 1 << 0 )
#define AUX_MUCNTL_TX_ENABLE        ( 1 << 1 )
#define AUX_MUCNTL_RTS_FLOW         ( 1 << 2 )
#define AUX_MUCNTL_CTS_FLOW         ( 1 << 3 )
#define AUX_MUCNTL_RTS_FIFO         ( 3 << 4 )
#define AUX_MUCNTL_RTS_ASSERT       ( 1 << 6 )
#define AUX_MUCNTL_CTS_ASSERT       ( 1 << 7 )

#define AUX_MUSTAT_SYMBOL_AV        ( 1 << 0 )
#define AUX_MUSTAT_SPACE_AV         ( 1 << 1 )
#define AUX_MUSTAT_RX_IDLE          ( 1 << 2 )
#define AUX_MUSTAT_TX_IDLE          ( 1 << 3 )
#define AUX_MUSTAT_RX_OVERRUN       ( 1 << 4 )
#define AUX_MUSTAT_TX_FIFO_FULL     ( 1 << 5 )
#define AUX_MUSTAT_RTS              ( 1 << 6 )
#define AUX_MUSTAT_CTS              ( 1 << 7 )
#define AUX_MUSTAT_TX_EMPTY         ( 1 << 8 )
#define AUX_MUSTAT_TX_DONE          ( 1 << 9 )
#define AUX_MUSTAT_RX_FIFO_LEVEL    ( 7 << 16 )
#define AUX_MUSTAT_TX_FIFO_LEVEL    ( 7 << 24 )


#define FSEL0(x)        ( x )
#define FSEL1(x)        ( x << 3 )
#define FSEL2(x)        ( x << 6 )
#define FSEL3(x)        ( x << 9 )
#define FSEL4(x)        ( x << 12 )
#define FSEL5(x)        ( x << 15 )
#define FSEL6(x)        ( x << 18 )
#define FSEL7(x)        ( x << 21 )
#define FSEL8(x)        ( x << 24 )
#define FSEL9(x)        ( x << 27 )

#define FSEL10(x)       ( x )
#define FSEL11(x)       ( x << 3 )
#define FSEL12(x)       ( x << 6 )
#define FSEL13(x)       ( x << 9 )
#define FSEL14(x)       ( x << 12 )
#define FSEL15(x)       ( x << 15 )
#define FSEL16(x)       ( x << 18 )
#define FSEL17(x)       ( x << 21 )
#define FSEL18(x)       ( x << 24 )
#define FSEL19(x)       ( x << 27 )

#define FSEL20(x)       ( x )
#define FSEL21(x)       ( x << 3 )
#define FSEL22(x)       ( x << 6 )
#define FSEL23(x)       ( x << 9 )
#define FSEL24(x)       ( x << 12 )
#define FSEL25(x)       ( x << 15 )
#define FSEL26(x)       ( x << 18 )
#define FSEL27(x)       ( x << 21 )
#define FSEL28(x)       ( x << 24 )
#define FSEL29(x)       ( x << 27 )

#define FSEL30(x)       ( x )
#define FSEL31(x)       ( x << 3 )
#define FSEL32(x)       ( x << 6 )
#define FSEL33(x)       ( x << 9 )
#define FSEL34(x)       ( x << 12 )
#define FSEL35(x)       ( x << 15 )
#define FSEL36(x)       ( x << 18 )
#define FSEL37(x)       ( x << 21 )
#define FSEL38(x)       ( x << 24 )
#define FSEL39(x)       ( x << 27 )

#define FSEL40(x)       ( x )
#define FSEL41(x)       ( x << 3 )
#define FSEL42(x)       ( x << 6 )
#define FSEL43(x)       ( x << 9 )
#define FSEL44(x)       ( x << 12 )
#define FSEL45(x)       ( x << 15 )
#define FSEL46(x)       ( x << 18 )
#define FSEL47(x)       ( x << 21 )
#define FSEL48(x)       ( x << 24 )
#define FSEL49(x)       ( x << 27 )

#define FSEL50(x)       ( x )
#define FSEL51(x)       ( x << 3 )
#define FSEL52(x)       ( x << 6 )
#define FSEL53(x)       ( x << 9 )


typedef struct {
    volatile uint IRQ;
    volatile uint ENABLES;

    volatile uint reserved1[((0x40 - 0x04) / 4) - 1];

    volatile uint MU_IO;
    volatile uint MU_IER;
    volatile uint MU_IIR;
    volatile uint MU_LCR;
    volatile uint MU_MCR;
    volatile uint MU_LSR;
    volatile uint MU_MSR;
    volatile uint MU_SCRATCH;
    volatile uint MU_CNTL;
    volatile uint MU_STAT;
    volatile uint MU_BAUD;

    volatile uint reserved2[(0x80 - 0x68) / 4];

    volatile uint SPI0_CNTL0;
    volatile uint SPI0_CNTL1;
    volatile uint SPI0_STAT;
    volatile uint SPI0_IO;
    volatile uint SPI0_PEEK;

    volatile uint reserved3[(0xC0 - 0x94) / 4];

    volatile uint SPI1_CNTL0;
    volatile uint SPI1_CNTL1;
    volatile uint SPI1_STAT;
    volatile uint SPI1_IO;
    volatile uint SPI1_PEEK;
} aux_t;

#endif
