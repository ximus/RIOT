#ifndef LEDS_H__
#define LEDS_H__

#define LED_GREEN_ON  (P3OUT |= BIT6)
#define LED_GREEN_OFF (P3OUT &= ~BIT6)

#define LED_RED_ON  (P1OUT |= BIT7)
#define LED_RED_OFF (P1OUT &= ~BIT7)

#define LED_YELLOW_ON  (P3OUT |= BIT7)
#define LED_YELLOW_OFF (P3OUT &= ~BIT7)

static inline leds_init(void)
{
    // green led
    P3DIR |= BIT6;
    LED_GREEN_OFF;
    // red led
    P1DIR |= BIT7;
    LED_RED_OFF;
    // yellow led
    P3DIR |= BIT7;
    LED_YELLOW_OFF;
}

#endif /* end of include guard: LEDS_H__ */