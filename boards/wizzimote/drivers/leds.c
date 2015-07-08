#include "leds.h"
#include "stdint.h"

void led_arch_apply_state(uint8_t state)
{
    state & YELLOW ? LED_YELLOW_ON : LED_YELLOW_OFF;
    state & GREEN ? LED_GREEN_ON : LED_GREEN_OFF;
    state & RED ? LED_RED_ON : LED_RED_OFF;
}