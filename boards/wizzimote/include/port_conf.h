#ifndef __PORT_CONF_H
#define __PORT_CONF_H

// WIZZIMOTE is based on CC430F5137
#include "cc430f5137.h"

// Port 1
// P1.0 GPIO4
// P1.1 GPIO5
// P1.2 GPIO6
// P1.3 GPIO7
// P1.4 SW3
// P1.5 SW2
// P1.6 SW1
// P1.7 LED1
#define P1_UNCONNECT      0X00
#define P1_GPIO_IN        0X7B
#define P1_GPIO_OUT       0X80
#define P1_GPIO_OUT_INIT    0X04
#define P1_IO_STRONG_DRIVE    0X00
#define P1_GPIO_PULL_DOWN   0X0B
#define P1_GPIO_PULL_UP     0X70
#define P1_SEL_ALTERNATE    0x00
#define P1_DIR_ALTERNATE    0x00

#define P1MAP0_CFG  PM_NONE
#define P1MAP1_CFG  PM_NONE
#define P1MAP2_CFG  PM_NONE
#define P1MAP3_CFG  PM_NONE
#define P1MAP4_CFG  PM_NONE
#define P1MAP5_CFG  PM_NONE
#define P1MAP6_CFG  PM_NONE
#define P1MAP7_CFG  PM_NONE

// Port 2
// P2.0 DIN
// P2.1 DOUT
// P2.2 ADIO3
// P2.3 ADIO2
// P2.4 ADIO1
// P2.5 ADIO0
// P2.6 ADIO4
// P2.7 ADIO5
#define P2_UNCONNECT      0X00
#define P2_GPIO_IN        0XFC
#define P2_GPIO_OUT       0X80
#define P2_GPIO_OUT_INIT    0X82
#define P2_IO_STRONG_DRIVE    0X00
#define P2_GPIO_PULL_DOWN   0X7C
#define P2_GPIO_PULL_UP     0X00
#define P2_SEL_ALTERNATE    0x03
#define P2_DIR_ALTERNATE    0x02

#define P2MAP0_CFG  PM_UCA0RXD
#define P2MAP1_CFG  PM_UCA0TXD
#define P2MAP2_CFG  PM_NONE
#define P2MAP3_CFG  PM_NONE
#define P2MAP4_CFG  PM_NONE
#define P2MAP5_CFG  PM_NONE
#define P2MAP6_CFG  PM_NONE
#define P2MAP7_CFG  PM_NONE

// Port 3 - Accessible on CON3
// P3.0 unconnect
// P3.1 unconnect
// P3.2 unconnect
// P3.3 unconnect
// P3.4 unconnect
// P3.5 unconnect
// P3.6 LED3
// P3.7 LED2
#define P3_UNCONNECT      0X3f
#define P3_GPIO_IN        0X00
#define P3_GPIO_OUT       0Xc0
#define P3_GPIO_OUT_INIT    0X00
#define P3_IO_STRONG_DRIVE    0X00
#define P3_GPIO_PULL_DOWN   0X00
#define P3_GPIO_PULL_UP     0X00
#define P3_SEL_ALTERNATE    0x00
#define P3_DIR_ALTERNATE    0x00

#define P3MAP0_CFG  PM_NONE
#define P3MAP1_CFG  PM_NONE
#define P3MAP2_CFG  PM_NONE
#define P3MAP3_CFG  PM_NONE
#define P3MAP4_CFG  PM_NONE
#define P3MAP5_CFG  PM_NONE
#define P3MAP6_CFG  PM_NONE
#define P3MAP7_CFG  PM_NONE

// Port 4 - Does not exist
// P4.0 unconnect
// P4.1 unconnect
// P4.2 unconnect
// P4.3 unconnect
// P4.4 unconnect
// P4.5 unconnect
// P4.6 unconnect
// P4.7 unconnect
#define NO_PORT4
#define P4_UNCONNECT      0Xff
#define P4_GPIO_IN        0X00
#define P4_GPIO_OUT       0X00
#define P4_GPIO_OUT_INIT    0X00
#define P4_IO_STRONG_DRIVE    0X00
#define P4_GPIO_PULL_DOWN   0X00
#define P4_GPIO_PULL_UP     0X00
#define P4_SEL_ALTERNATE    0x00
#define P4_DIR_ALTERNATE    0x00

// Port 5
// P5.0 XIN
// P5.1 XOUT
// P5.2 unconnect
// P5.3 unconnect
// P5.4 unconnect
// P5.5 unconnect
// P5.6 unconnect
// P5.7 unconnect
#if MSP430_HAS_EXTERNAL_CRYSTAL
#define P5_UNCONNECT      0Xfc
#define P5_GPIO_IN        0X00
#define P5_GPIO_OUT       0X00
#define P5_GPIO_OUT_INIT    0X00
#define P5_IO_STRONG_DRIVE    0X00
#define P5_GPIO_PULL_DOWN   0X00
#define P5_GPIO_PULL_UP     0X00
#define P5_SEL_ALTERNATE    0x03
#define P5_DIR_ALTERNATE    0x00
#else
#define P5_UNCONNECT      0Xff
#define P5_GPIO_IN        0X00
#define P5_GPIO_OUT       0X00
#define P5_GPIO_OUT_INIT    0X00
#define P5_IO_STRONG_DRIVE    0X00
#define P5_GPIO_PULL_DOWN   0X00
#define P5_GPIO_PULL_UP     0X00
#define P5_SEL_ALTERNATE    0x00
#define P5_DIR_ALTERNATE    0x00
#endif

// Port J
// PJ.0 JTAG TDO / GPIO3
// PJ.1 JTAG TDI / GPIO2
// PJ.2 JTAG TMS / GPIO1
// PJ.3 JTAG TCK / GPIO0
#define PJ_UNCONNECT      0X00
#define PJ_GPIO_IN        0X00
#define PJ_GPIO_OUT       0X00
#define PJ_GPIO_OUT_INIT    0X00
#define PJ_IO_STRONG_DRIVE    0X00
#define PJ_GPIO_PULL_DOWN   0X00
#define PJ_GPIO_PULL_UP     0X00

//======================================================================

#endif
