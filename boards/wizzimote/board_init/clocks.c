void cc430_clocks_init(void)
{
    volatile uint16_t delay = 100;

    uint16_t selref_32k;
    uint16_t sela_32k;

    #ifdef MSP430_HAS_EXTERNAL_CRYSTAL
    {
        // Board not populated with external oscillator, use internal
        selref_32k = SELREF__REFOCLK;
        sela_32k = SELA__REFOCLK;

        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | sela_32k;

        // Disable XT1 - XT2 and SMCLK enabled
        // XT2 : XT2OFF set, XT2 active upon radio request
        // XT1 Drive 0 :  XT1DRIVE_0 lowest power consumption
        // XTS not set : XT1 in low frequency mode
        // XT1BYPASS : not set, no clock source on XT1IN
        // XCAP : XT1 capacitor selection
        // SMCLKOFF not set
        // XT1OFF set to disable XT1 oscillator
        UCSCTL6 = XT2OFF | XT1OFF | XT1DRIVE_0;

        // Clear all oscillator error flag
        UCSCTL7 = 0;
        // Clear OFIFG fault flag
        SFRIFG1 &= ~OFIFG;
    }
    #else
    #warning "Using XT1 as 32K generator"
    {
        // Board populated with external oscillator, use external
        selref_32k = SELREF__XT1CLK;
        sela_32k = SELA__XT1CLK;
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | sela_32k;

        UCSCTL6 = XT2OFF | XT1DRIVE_0;

        // Clear all oscillator error flag
        UCSCTL7 = 0;
        // Clear OFIFG fault flag
        SFRIFG1 &= ~OFIFG;
    }
    #endif

    // ---------------------------------------------------------------------
    // Configure CPU clock for 19.92MHz
    // Disable the FLL control loop
    _BIS_SR(SCG0);

    // Reset DCO tap and modulation to a medium value
    // This is managed by FLL when enabled
    UCSCTL0 = 0x1080;
    // Set DCO operating frequency - modulation enabled
    UCSCTL1 = DCORSEL_6;
    // Select FLL input. 32k CLK divided by 1
    UCSCTL3 = FLLREFDIV__1 | selref_32k;
    // Set multiplier and divider of FLL
    // 610 = (304+1) x 2
    UCSCTL2 = 304 | FLLD__2 ;

    // Delay required here
    for (delay=0;delay<10;delay++)
        __delay_cycles(32*32);

    // Enable the FLL control loop
    _BIC_SR(SCG0);

    // Delay to allow for clock stabilization
    // Wait for maximum delay (32 x 32 x Cycle @ FrefclockFLL (32k))
    for (delay=0;delay<20000/32;delay++)
        __delay_cycles(32*32);

    // Loop until XT1 & DCO stabilizes, use do-while to insure that
    // body is executed at least once
    while (SFRIFG1 & OFIFG)
    {
        // Clear all oscillator fault Flags
        UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT1HFOFFG+XT2OFFG);
        // Clear OFIFG fault flag
        SFRIFG1 &= ~OFIFG;
    }

    // Select Sources for UCS generated clocks
    // Set ACLK to 32768 Hz from 32K CLK
    // Set SMCLK to 2.49625 from DCO/8
    // Set MCLK to 19.97 MHz from DCOCLK
    // Divider selection for ACLK, SMCLK, MCLK
    UCSCTL5 = DIVA__1 | DIVS__8 | DIVM__1;
    // Source selection
    UCSCTL4 = sela_32k | SELS__DCOCLK | SELM__DCOCLK;

    // Enable all request control for all clock sources
    UCSCTL8 = ACLKREQEN | MCLKREQEN | SMCLKREQEN | MODOSCREQEN;
}