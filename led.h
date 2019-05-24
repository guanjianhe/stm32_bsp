#ifndef LED_H
#define LED_H

#include "stm32f10x.h"

extern void led_init (void);
extern int led_set(uint8_t led_id,uint8_t state);
extern int led_on(uint8_t led_id);
extern int led_off(uint8_t led_id);
extern int led_toggle(uint8_t led_id);

#endif
