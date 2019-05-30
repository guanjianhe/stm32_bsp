#ifndef RTC_H
#define RTC_H
#include "stm32f10x.h"

#define F_LSE   32768   /* LSE oscillator frequency */

#define _RTC_TDIF   +8.0


typedef struct
{
    uint16_t    year;   /* 1970..2106 */
    uint8_t     month;  /* 1..12 */
    uint8_t     mday;   /* 1..31 */
    uint8_t     hour;   /* 0..23 */
    uint8_t     min;    /* 0..59 */
    uint8_t     sec;    /* 0..59 */
    uint8_t     wday;   /* 0..6 (Sun..Sat) */
} rtc_t;

int rtc_init(void);               /* Initialize RTC */
int rtc_gettime(rtc_t* rtc);          /* Get time */
int rtc_settime(const rtc_t* rtc);    /* Set time */
int rtc_getutc(uint32_t* utc);          /* Get time in UTC */
int rtc_setutc(uint32_t utc);           /* Set time in UTC */




#endif

