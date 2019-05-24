#ifndef LED_H
#define LED_H

#include "stm32f10x.h"


#define RCC_LED     RCC_APB2Periph_GPIOC
#define PORT_LED    GPIOC
#define PIN_LED     GPIO_Pin_13

#define LED_NUM 1

extern void InitLed ( void );

extern int led_set (uint8_t led_id, uint8_t state);
extern int led_on (uint8_t led_id);
extern int led_off (uint8_t led_id);
extern int led_toggle (uint8_t led_id);

#endif
