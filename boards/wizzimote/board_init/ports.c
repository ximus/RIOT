/****************************************************************************************************
 * @fn          cc430_io_init
 * @brief       configure platform IOs - uses port_definitions.h mapping
 * @param       none
 * @return      none
 ***************************************************************************************************/
void cc430_io_init(void)
{
    // Unlock IO mapping registers
    PMAPKEYID = PMAPKEY;
    PMAPCTL = PMAPRECFG;

    // Set function to IO for all pins
    // Select port 1 to 3
#ifdef __MSP430_HAS_PORT1_R__
    P1MAP0  = P1MAP0_CFG;
    P1MAP1  = P1MAP1_CFG;
    P1MAP2  = P1MAP2_CFG;
    P1MAP3  = P1MAP3_CFG;
    P1MAP4  = P1MAP4_CFG;
    P1MAP5  = P1MAP5_CFG;
    P1MAP6  = P1MAP6_CFG;
    P1MAP7  = P1MAP7_CFG;
#endif

#ifdef __MSP430_HAS_PORT2_R__
    P2MAP0  = P2MAP0_CFG;
    P2MAP1  = P2MAP1_CFG;
    P2MAP2  = P2MAP2_CFG;
    P2MAP3  = P2MAP3_CFG;
    P2MAP4  = P2MAP4_CFG;
    P2MAP5  = P2MAP5_CFG;
    P2MAP6  = P2MAP6_CFG;
    P2MAP7  = P2MAP7_CFG;
#endif

#ifdef __MSP430_HAS_PORT3_R__
    P3MAP0  = P3MAP0_CFG;
    P3MAP1  = P3MAP1_CFG;
    P3MAP2  = P3MAP2_CFG;
    P3MAP3  = P3MAP3_CFG;
    P3MAP4  = P3MAP4_CFG;
    P3MAP5  = P3MAP5_CFG;
    P3MAP6  = P3MAP6_CFG;
    P3MAP7  = P3MAP7_CFG;
#endif

    // Lock IO mapping register
    PMAPPWD = 0;

#ifdef __MSP430_HAS_PORT1_R__
    //-------------------
    // Set PORT1 IOs
    //-------------------
    P1OUT = P1_GPIO_OUT_INIT ;
    // Pull Resistors Enable
    P1OUT |= (P1_GPIO_PULL_UP & P1_GPIO_IN);
    P1REN = (P1_GPIO_PULL_UP | P1_GPIO_PULL_DOWN) & P1_GPIO_IN;
    // Drive strength
    P1DS = P1_IO_STRONG_DRIVE;
    // Select secondary function for non GPIO or non unused pins
    P1SEL = P1_SEL_ALTERNATE;
    // Select direction
    P1DIR = P1_UNCONNECT | P1_GPIO_OUT | P1_DIR_ALTERNATE;
    // Disable and clear all pin IRQs
    P1IE = 0;
    P1IFG = 0;
#endif

#ifdef __MSP430_HAS_PORT2_R__
    //-------------------
    // Set PORT2 IOs
    //-------------------
    P2OUT = P2_GPIO_OUT_INIT;
    // Pull Resistors Enable
    P2OUT |= (P2_GPIO_PULL_UP & P2_GPIO_IN);
    P2REN = (P2_GPIO_PULL_UP | P2_GPIO_PULL_DOWN) & P2_GPIO_IN;
    // Drive strength
    P2DS = P2_IO_STRONG_DRIVE;
    // Select secondary function for non GPIO or non unused pins
    P2SEL = P2_SEL_ALTERNATE;
    // Select direction
    P2DIR = P2_UNCONNECT | P2_GPIO_OUT | P2_DIR_ALTERNATE;
    // Disable and clear all pin IRQs
    P2IE = 0;
    P2IFG = 0;
#endif

#ifdef __MSP430_HAS_PORT3_R__
    //-------------------
    // Set PORT3 IOs
    //-------------------
    P3OUT = P3_GPIO_OUT_INIT;
    // Pull Resistors Enable
    P3OUT |= (P3_GPIO_PULL_UP & P3_GPIO_IN);
    P3REN = (P3_GPIO_PULL_UP | P3_GPIO_PULL_DOWN) & P3_GPIO_IN;
    // Drive strength
    P3DS = P3_IO_STRONG_DRIVE;
    // Select secondary function for non GPIO or non unused pins
    P3SEL = P3_SEL_ALTERNATE;
    // Select direction
    P3DIR = P3_UNCONNECT | P3_GPIO_OUT | P3_DIR_ALTERNATE;
#endif

#ifndef NO_PORT4
#ifdef __MSP430_HAS_PORT4_R__
    //-------------------
    // Set PORT4 IOs
    //-------------------
    P4OUT = P4_GPIO_OUT_INIT;
    // Pull Resistors Enable
    P4OUT |= (P4_GPIO_PULL_UP & P4_GPIO_IN);
    P4REN = (P4_GPIO_PULL_UP | P4_GPIO_PULL_DOWN) & P4_GPIO_IN;
    // Drive strength
    P4DS = P4_IO_STRONG_DRIVE;
    // Select secondary function for non GPIO or non unused pins
    P4SEL = P4_SEL_ALTERNATE;
    // Select direction
    P4DIR = P4_UNCONNECT | P4_GPIO_OUT | P4_DIR_ALTERNATE;
#endif
#endif

#ifdef __MSP430_HAS_PORT5_R__
    //-------------------
    // Set PORT5 IOs
    //-------------------
    P5OUT = P5_GPIO_OUT_INIT;
    // Pull Resistors Enable
    P5OUT |= (P5_GPIO_PULL_UP & P5_GPIO_IN);
    P5REN = (P5_GPIO_PULL_UP | P5_GPIO_PULL_DOWN) & P5_GPIO_IN;
    // Drive strength
    P5DS = P5_IO_STRONG_DRIVE;
    // Select secondary function for non GPIO or non unused pins
    P5SEL = P5_SEL_ALTERNATE;
    // Select direction
    P5DIR = P5_UNCONNECT | P5_GPIO_OUT | P5_DIR_ALTERNATE;
#endif

#ifdef __MSP430_HAS_PORTJ_R__
    //-------------------
    // Set PORTJ IOs
    //-------------------
    PJOUT = PJ_GPIO_OUT_INIT;
    // Pull Resistors Enable
    PJOUT |= (PJ_GPIO_PULL_UP & PJ_GPIO_IN);
    PJREN = (PJ_GPIO_PULL_UP | PJ_GPIO_PULL_DOWN) & PJ_GPIO_IN;
    // Drive strength
    PJDS = PJ_IO_STRONG_DRIVE;
    // Select direction
    PJDIR = PJ_UNCONNECT | PJ_GPIO_OUT;
#endif
}
