/**
 * @ingroup wizzimote
 * @{
 */

/**
 * @file
 * @brief       Wizzimote board initialization
 *
 */

#include <stdint.h>

#include "board.h"
#include "port_conf.h"
#include "cpu.h"
#include "board_leds.h"
#include "irq.h"
#include "debug.h"
#include "uart.h"
#include "cc430f5137.h"

#include "board_init/clocks.c"
#include "board_init/ports.c"
#include "board_init/voltage.c"

void board_init(void)
{
    /* disable watchdog */
    WDTCTL = WDTPW + WDTHOLD;

    msp430_cpu_init();

    /* Set Vcore to 2.2V to allow running at 20MHz */
    cc430_pmm_set_vcore(PMMCOREV_2);
    cc430_clocks_init();
    cc430_io_init();

    leds_init();
    uart_init();

    /* enable interrupts */
    _BIS_SR(GIE);
}
