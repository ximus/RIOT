#ifndef LED_H__
#define LED_H__

/**
 * Helper library to deal with concurrent access to hardware LEDs.
 *
 * Threads call `led_aquire` to take ownership of the array of LEDs and then
 * `led_release` when they are done using the LEDs. Multiple threads can call
 * `led_aquire`, the last one to call it has control of the LEDs. The state of
 * the LEDs for each thread is remembered and restored when control of the LEDs
 * is restored once later threads have released the LEDs. Threads can continue
 * to alter their LED states while they are not in control of the LEDs.
 *
 * All calls are scoped impliclityly to current thread's PID. Maybe it would be
 * better to provide an explicit token to represent scope (TODO).
 *
 * LEDs state is represented by an integer, currently 8-bit. Integer masks are
 * passed to the led control functions (currently just `led_on` and `led_off`).
 * Those masks toggle the bits of the state integer. When the state changes, it
 * is passed to a board specific function `led_arch_apply_state` responsible
 * for making sense of the state and controlling the hardware leds.
 */

#include "stdint.h"

#ifndef LED_CLIENTS_MAX
#define LED_CLIENTS_MAX (3)
#endif

void led_init(void);

int led_aquire(void);
int led_release(void);

int led_on(uint8_t);
int led_off(uint8_t);
int led_toggle(uint8_t);

#endif /* end of include guard: LED_H__ */