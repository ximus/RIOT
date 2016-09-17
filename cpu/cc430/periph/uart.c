/*
 * uart.c - Implementation for the Zolertia Z1 UART
 * Copyright (C) 2014 INRIA
 *
 * Author : Kevin Roussel <kevin.roussel@inria.fr>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     board_z1
 * @{
 *
 * @file        uart.c
 * @brief       Board specific UART/USB driver HAL for the Zolertia Z1
 *
 * @author      Kévin Roussel <Kevin.Roussel@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include "cpu.h"
#include "board.h"
#include "kernel.h"
#include "irq.h"
#include "board_uart0.h"
#include "msp430.h"



typedef enum {
    BLOCK,
    CHAR
} uart_mode_t;

static struct {
    uart_mode_t tx_mode;
    uart_mode_t rx_mode;
    uint8_t     rx_size;
} uart = {CHAR,CHAR,0};


static void (*block_receive_callback)(void);
static void (*block_send_callback)(void);

static void (*char_receive_callback)(uint8_t c);


void set_rx_mode(uart_mode_t mode);

// Runs in char mode when no block transfer is active
// defaults to 112500 bps
void uart_init(void)
{
    // SMCLK, parity off, LSB-first, 8N1, a uart
    /*
     * NOTE : MCU pin (GPIO port) initialisasynction is done
     * in board.c.
     */
    UCA0CTL1  = UCSWRST;         /* hold UART module in reset state
                                    while we configure it */

    UCA0CTL1 |= UCSSEL__SMCLK;   /* source UART's BRCLK from SMCLK  */

// Safeguard. Helps guarantee changes in SMCLK are reflected here
#if MSP430_INITIAL_SMCLK_SPEED != 4997120uL
#warning "UART baudrate calculations are invalid"
#endif

    // BaudRate is 112500
    // BR1:0 = Floor(BitCLK/BaudRate)
    // MCTL  = Round(((BitCLK/BaudRate) - Floor(BitCLK/BaudRate)) * 8) << 1
    UCA0BR0   = 0x02;
    UCA0BR1   = 0x00;
    UCA0MCTL  = UCBRS_6 | UCBRF_11 | UCOS16;

    // clear interupt before enable
    UCA0IE &= ~UCRXIFG;

    /* configuration done, release reset bit => start UART */
    UCA0CTL1 &= ~UCSWRST;

    set_rx_mode(CHAR);
}

void uart_on_char_receive(void (*rx_callback)(uint8_t))
{
    char_receive_callback = rx_callback;
}

void set_rx_mode(uart_mode_t mode)
{
    if (mode == CHAR) {
        DMA0CTL &= ~(DMAIE | DMAEN);   /* disable dma interrupt */
        UCA0IE  |= UCRXIE;             /* enable UART RX interrupts */
    }
    else { /* BLOCK */
        UCA0IE  &= ~UCRXIE;         /* disbale UART RX interrupts */
        DMA0CTL |= DMAIE | DMAEN;   /* enable dma counter interrupt */
    }
    uart.rx_mode = mode;
}

void set_tx_mode(uart_mode_t mode)
{
    if (mode == CHAR) {
        DMA2CTL &= ~(DMAIE | DMAEN);   /* disable dma interrupt */
    }
    else { /* BLOCK */
        DMA2CTL |= DMAIE | DMAEN;      /* enable dma counter interrupt */
    }
    uart.tx_mode = mode;
}

void uart_block_send(uint8_t *src, uint8_t len, void (*callback)(void))
{
    DMACTL1 = DMA2TSEL_17;
    DMACTL4 = ENNMI | ROUNDROBIN;
    DMA2SA  = (uint16_t) src;
    DMA2DA  = (uint16_t) UCA0TXBUF_;
    DMA2SZ  = (len);
    DMA2CTL = DMADT_0 | DMASRCINCR_3 | DMASBDB | DMALEVEL;

    block_send_callback = callback;

    set_tx_mode(BLOCK);
}

void uart_block_send_end(void)
{
    set_tx_mode(CHAR);
}


// uses single transfer mode
// cuts uart0 out of the loop during transfer
void uart_block_receive(uint16_t dest, uint8_t size, void (*callback)(void)) {
    DMACTL0 = DMA0TSEL_16;
    DMACTL4 = ENNMI | ROUNDROBIN;
    DMA0SA  = (uint16_t) UCA0RXBUF_;
    DMA0DA  = (uint16_t) dest;
    DMA0SZ  = size;
    DMA0CTL = DMADT_0 | DMADSTINCR_3 | DMASBDB | DMALEVEL;

    uart.rx_size = size;
    block_receive_callback = callback;

    set_rx_mode(BLOCK);
}

void uart_block_receive_end(void)
{
    uart.rx_size = 0;
    set_rx_mode(CHAR);
}

int uart_block_received_count(void) {
    if (uart.rx_mode == BLOCK)
        return uart.rx_size - DMA0SZ;
    return -1;
}

#ifdef BUFFER_PUTCHAR
char putchar_buffer[1000] = {0};
uint16_t putchar_buffer_pos = 0;
#endif
/**
 * TODO(max): big issue here when stdlib putchar is used
 * and uart.tx_mode is disregarded
 */
#ifdef MSPGCC
int putchar(int c)
{
    #ifdef BUFFER_PUTCHAR
    putchar_buffer[putchar_buffer_pos++] = c;
    if (putchar_buffer_pos == sizeof(putchar_buffer))
        putchar_buffer_pos = 0;
    #else
    /**
     * TODO(max): this needs thought
     * if we are in BLOCK mode, then we shouldn't interveave the BLOCK
     * transfer with calls to putchar(). Initially I had calls to putchar wait:
     * `while (uart.tx_mode != CHAR);`
     * but it seems like too great of a risk for deadlocks.
     * So for now, just drop calls to putchar() and put the responsibility
     * in developer's hand to not call putchar() in a meaningful way during
     * block transfers.
     */
    /* load TX byte buffer */

    UCA0TXBUF = (uint8_t) c;
    /* wait for transmission to end */
    while (!(UCA0IFG & UCTXIFG));
    #endif
    return c;
}
#endif /* MSPGCC */

/**
 * \brief the interrupt handler for UART reception
 */
interrupt(USCI_A0_VECTOR) __attribute__ ((naked)) uart0irq(void)
{
    __enter_isr();

    int c;

    /* Check status register for receive errors. */
    if (UCA0STAT & UCRXERR) {
        if (UCA0STAT & UCFE) {
            puts("UART RX framing error");
        }
        if (UCA0STAT & UCOE) {
            puts("UART RX overrun error");
        }
        if (UCA0STAT & UCPE) {
            puts("UART RX parity error");
        }
        if (UCA0STAT & UCBRK) {
            puts("UART RX break condition -> error");
        }
        /* Clear error flags by forcing a dummy read. */
        c = UCA0RXBUF;
    } else {
        /* All went well -> let's signal the reception to adequate callbacks */
        /* ideally should not read UCA0RXBUF if neither uart0 and char_receive_callback defined*/
        c = UCA0RXBUF;
#ifdef MODULE_UART0
        if (uart0_handler_pid != KERNEL_PID_UNDEF) {
            uart0_handle_incoming(c);
            uart0_notify_thread();
        }
#endif
        if (char_receive_callback != NULL)
        {
            char_receive_callback((uint8_t) c);
        }
    }

    __exit_isr();
}

/**
 * DMA interrupt hogging, but RIOT doesn't currently have
 * a centrialized facility for DMA
 */
interrupt(DMA_VECTOR) __attribute__ ((naked)) dma_irq(void)
{
    __enter_isr();

    uint16_t cause = DMAIV;

    if (cause == 2) // DMA0
    {
        uart_block_receive_end();
        if (block_receive_callback != NULL)
        {
            block_receive_callback();
            block_receive_callback = NULL;
        }
    }
    else if (cause == 6) { // DMA2
        uart_block_send_end();
        if (block_send_callback != NULL)
        {
            block_send_callback();
            block_send_callback = NULL;
        }
    }

    __exit_isr();
}