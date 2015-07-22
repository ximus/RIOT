#include "cpu.h"

#include "periph/timer.h"
#include "periph_conf.h"


#if TIMER_NUMOF

typedef struct {
    void (*cb)(int);
} timer_conf_t;

timer_conf_t config[TIMER_NUMOF];


/* Helpers forward declarations */
static void init_channels(tim_t dev);
static volatile unsigned int *channel_ccr(tim_t dev, int channel);
static volatile unsigned int *channel_ctl(tim_t dev, int channel);
static unsigned int channel_count(tim_t dev);
static void channel_clear(tim_t dev, int channel);
static int channel_exists(tim_t dev, int channel);


int timer_init(tim_t dev, unsigned int us_per_tick, void (*callback)(int))
{
    /* at the moment, the timer can only run at 1MHz */
    if (us_per_tick != 1) {
        return -1;
    }

    switch (dev) {
#if TIMER_0_EN
        case TIMER_0:
            TA0CTL = TACLR;                 /* Clear the timer counter */
            TA0CTL |= TIMER_0_CLK_TASSEL;   /* Select clock */
            TA0CTL |= TIMER_0_DIV_ID;       /* Select clock divider `ID` */
            TA0EX0 = TIMER_0_DIV_TAIDEX;    /* Select clock divider `TAIDEX` */
            TA0CTL &= ~TAIFG;               /* Clear the IFG */
            TA0CTL &= ~TAIE;                /* Disable TAIE (overflow IRQ) */

            init_channels(dev);
            break;
#endif
#if TIMER_1_EN
        case TIMER_1:
            TA1CTL = TACLR;                 /* Clear the timer counter */
            TA1CTL |= TIMER_1_CLK_TASSEL;   /* Select clock */
            TA1CTL |= TIMER_1_DIV_ID;       /* Select clock divider `ID` */
            TA1EX0 = TIMER_1_DIV_TAIDEX;    /* Select clock divider `TAIDEX` */
            TA1CTL &= ~TAIFG;               /* Clear the IFG */
            TA1CTL &= ~TAIE;                /* Disable TAIE (overflow IRQ) */

            init_channels(dev);
            break;
#endif
        case TIMER_UNDEFINED:
        default:
            return -1;
    }

    timer_start(dev);

    config[dev].cb = callback;

    return 0;
}

int timer_set(tim_t dev, int channel, unsigned int timeout)
{
    return timer_set_absolute(dev, channel, timer_read(dev) + timeout);
}

int timer_set_absolute(tim_t dev, int channel, unsigned int value)
{
    if (dev == TIMER_UNDEFINED) {
        return -1;
    }

    /* make sure channel is available */
    if (!channel_exists(dev, channel)) {
        return -1;
    }

    /* get timer base register address */
    volatile unsigned int *ccr = channel_ccr(dev, channel);
    volatile unsigned int *ctl = channel_ctl(dev, channel);

    *ccr = value;     /* set compare value */
    *ctl |= CCIE;     /* enable channel iterrupt */
    *ctl &= ~(CCIFG); /* clear channel interrupt */

    return 1;
}

int timer_clear(tim_t dev, int channel)
{
    if (dev == TIMER_UNDEFINED) {
        return -1;
    }

    channel_clear(dev, channel);

    return 1;
}

unsigned int timer_read(tim_t dev)
{
    switch (dev) {
#if TIMER_0_EN
        case TIMER_0:
            return TA0R;
#endif
#if TIMER_1_EN
        case TIMER_1:
            return TA1R;
#endif
        case TIMER_UNDEFINED:
        default:
            return -1;
    }
}

void timer_start(tim_t dev)
{
#if TIMER_0_EN
    if (dev == TIMER_0) {
        TA0CTL |= MC_2;
    }
#endif
#if TIMER_1_EN
    if (dev == TIMER_1) {
        TA1CTL |= MC_2;
    }
#endif
}

void timer_stop(tim_t dev)
{
#if TIMER_0_EN
    if (dev == TIMER_0) {
        TA0CTL |= MC_0;
    }
#endif
#if TIMER_1_EN
    if (dev == TIMER_1) {
        TA1CTL |= MC_0;
    }
#endif
}

void timer_irq_enable(tim_t dev)
{
    unsigned int count = channel_count(dev);

    /* for each channel */
    for (int i = 0; i < count; i++) {
        volatile unsigned int *ctl = channel_ctl(dev, i);

        *ctl |= CCIE;     /* enable channel iterrupt */
        *ctl &= ~(CCIFG); /* clear channel interrupt */
    }
}

void timer_irq_disable(tim_t dev)
{
    unsigned int count = channel_count(dev);

    /* for each channel */
    for (int i = 0; i < count; i++) {
        volatile unsigned int *ctl = channel_ctl(dev, i);

        *ctl &= ~(CCIE);   /* disable channel interrupt */
    }
}

void timer_reset(tim_t dev)
{
#if TIMER_0_EN
    if (dev == TIMER_0) {
        TA0R = 0;
    }
#endif
#if TIMER_1_EN
    if (dev == TIMER_1) {
        TA1R = 0;
    }
#endif
}


/**
 * Helpers
 */

static void init_channels(tim_t dev)
{
    unsigned int count = channel_count(dev);

    /* for each channel */
    for (int i = 0; i < count; i++) {
        channel_clear(dev, i);
    }
}

static void channel_clear(tim_t dev, int channel)
{
    volatile unsigned int *ccr = channel_ccr(dev, channel);
    volatile unsigned int *ctl = channel_ctl(dev, channel);

    *ctl &= ~(CCIFG);  /* clear channel interrupt flag */
    *ctl &= ~(CCIE);   /* disable channel interrupt */
    *ccr = 0;
}

static volatile unsigned int *channel_ccr(tim_t dev, int channel)
{
#if TIMER_0_EN
    if (dev == TIMER_0) {
        return &TA0CCR0 + channel;
    }
#endif
#if TIMER_1_EN
    if (dev == TIMER_1) {
        return &TA1CCR0 + channel;
    }
#endif
    return NULL;
}

static volatile unsigned int *channel_ctl(tim_t dev, int channel)
{
#if TIMER_0_EN
    if (dev == TIMER_0) {
        return &TA0CCTL0 + channel;
    }
#endif
#if TIMER_1_EN
    if (dev == TIMER_1) {
        return &TA1CCTL0 + channel;
    }
#endif
    return NULL;
}

static int channel_exists(tim_t dev, int channel)
{
    if (channel >= channel_count(dev)) {
        return 0; /* does not exist */
    }
    return 1; /* exists */
}

static unsigned int channel_count(tim_t dev)
{
#if TIMER_0_EN
    if (dev == TIMER_0) {
        return TIMER_0_CHANNELS;
    }
#endif
#if TIMER_1_EN
    if (dev == TIMER_1) {
        return TIMER_1_CHANNELS;
    }
#endif
    return 0;
}


/**
 * Interupts
 */

#if TIMER_0_EN
/* interrupt dedicated to CCR0 */
interrupt(TIMER0_A0_VECTOR) __attribute__((naked)) timer0_a0_isr(void)
{
    __enter_isr();

    channel_clear(TIMER_0, 0);  /* remove the timer */
    config[TIMER_0].cb(0);      /* fire callback */

    __exit_isr();
}

/* interrupt for CCR1-5 */
interrupt(TIMER0_A1_VECTOR) __attribute__((naked)) timer0_a1_isr(void)
{
    __enter_isr();

    short channel = TA0IV >> 1;

    channel_clear(TIMER_0, channel);  /* remove the timer */
    config[TIMER_0].cb(channel);      /* fire callback */

    __exit_isr();
}
#endif /* TIMER_0_EN interrupts */

#if TIMER_1_EN
/* interrupt dedicated to CCR0 */
interrupt(TIMER1_A0_VECTOR) __attribute__((naked)) timer1_a0_isr(void)
{
    __enter_isr();

    channel_clear(TIMER_1, 0);  /* remove the timer */
    config[TIMER_1].cb(0);      /* fire callback */

    __exit_isr();
}

/* interrupt for CCR1-3 */
interrupt(TIMER1_A1_VECTOR) __attribute__((naked)) timer1_a1_isr(void)
{
    __enter_isr();

    short channel = TA1IV >> 1;

    channel_clear(TIMER_1, channel);  /* remove the timer */
    config[TIMER_1].cb(channel);      /* fire callback */

    __exit_isr();
}
#endif /* TIMER_1_EN interrupts */

#endif /* TIMER_NUMOF */