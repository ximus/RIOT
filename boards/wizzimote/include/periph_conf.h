#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Real Time Clock configuration
 * @{
 */
#define RTC_NUMOF           (1)
/** @} */


/**
 * @brief Timer configuration
 * @{
 */
#define TIMER_NUMOF         (2U)
#define TIMER_0_EN          1
#define TIMER_1_EN          1

/* Timer 0 configuration */
#define TIMER_0_CHANNELS    5
#define TIMER_0_MAX_VALUE   (0xffff)
/* TODO(ximus): Finalize this, current setup is not provide 1Mhz */
#define TIMER_0_CLK_TASSEL  TASSEL_2 /* SMLCK @ Mhz */
#define TIMER_0_DIV_ID      ID__1     /* /1 */
#define TIMER_0_DIV_TAIDEX  TAIDEX_4  /* /5 */

/* Timer 1 configuration */
#define TIMER_1_CHANNELS    3
#define TIMER_1_MAX_VALUE   (0xffff)
#define TIMER_1_CLK_TASSEL  TASSEL_2 /* SMLCK @ Mhz */
#define TIMER_1_DIV_ID      ID__1     /* /1 */
#define TIMER_1_DIV_TAIDEX  TAIDEX_4  /* /5 */
/** @} */


/**
 * @brief wtimer configuration
 * @{
 */
/* NOTE(ximus): all msp430's are 16-bit, should be defined in cpu */
#define WTIMER_MASK 0xffff0000 /* 16-bit timers */
/* TODO(ximus): set these correctly. default values for now */
#define WTIMER_USLEEP_UNTIL_OVERHEAD 3
#define WTIMER_OVERHEAD 24
#define WTIMER_BACKOFF 30
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */