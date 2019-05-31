#ifndef DELAY_H
#define DELAY_H

#include "stm32f10x.h"

extern void systick_init(void);

extern void systick_delay_us(uint32_t nus);

extern void systick_delay_ms(uint16_t nms);


#endif
