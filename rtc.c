#include "rtc.h"

static const uint8_t samurai[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static uint8_t rtcok;

#define _BV(bit) (1<<(bit))


/* 1:RTC is available, 0:RTC is not available */

int rtc_init(void)
{
    uint32_t n;

    /* Enable BKP and PWR module */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);


    PWR->CR |= _BV(8);   /* Enable write access to backup domain */

    RCC->BDCR = 0x00008101;  /* Enable LSE oscillator */

    /* Wait for LSE start and stable */
    for (n = 8000000; n && !(RCC->BDCR & _BV(1)); n--) ;

    if (n)
    {
        for (n = 100000; n && !(RTC->CRL & _BV(5)); n--) ;

        if (n)
        {
            /* Enter RTC configuration mode */
            RTC->CRL = _BV(4);
            RTC->PRLH = 0;
            /* Set RTC clock divider for 1 sec tick */
            RTC->PRLL = F_LSE - 1;
            /* Exit RTC configuration mode */
            RTC->CRL = 0;

            /* Wait for RTC internal process */
            for (; n && !(RTC->CRL & _BV(5)); n--) ;

            /* Wait for RTC is in sync */
            for (; n && !(RTC->CRL & _BV(3)); n--) ;
        }
    }

    if (n)
    {
        rtcok = 1;      /* RTC is available */
    }
    else
    {
        rtcok = 0;      /* RTC is not available */
        RCC->BDCR = 0;   /* Stop LSE oscillator */
    }

    /* Inhibit write access to backup domain */
    PWR->CR &= ~_BV(8);  /* Inhibit write access to backup domain */

    return rtcok;
}


int rtc_setutc(uint32_t tmr)
{
    uint32_t n = 0;

    if (rtcok)
    {
        /* Enable write access to backup domain */
        PWR->CR |= _BV(8);

        /* Wait for end of RTC internal process */
        for (n = 100000; n && !(RTC->CRL & _BV(5)); n--) ;

        if (n)
        {
            /* Enter RTC configuration mode */
            RTC->CRL = _BV(4);

            /* Set time counter */
            RTC->CNTL = tmr;
            RTC->CNTH = tmr >> 16;

            /* Set RTC clock divider for 1 sec tick */
            RTC->PRLL = F_LSE - 1;
            RTC->PRLH = 0;

            /* Exit RTC configuration mode */
            RTC->CRL = 0;

            /* Wait for end of RTC internal process */
            for (; n && !(RTC->CRL & _BV(5)); n--) ;
        }

        /* Inhibit write access to backup domain */
        PWR->CR &= ~_BV(8);
    }

    return n ? 1 : 0;
}


int rtc_getutc(uint32_t* tmr)
{
    uint32_t t1, t2;


    if (rtcok)
    {
        /* Read RTC counter */
        t1 = RTC->CNTH << 16 | RTC->CNTL;

        do
        {
            t2 = t1;
            t1 = RTC->CNTH << 16 | RTC->CNTL;
        } while (t1 != t2);

        *tmr = t1;
        return 1;
    }

    return 0;
}

int rtc_gettime(rtc_t* rtc)
{
    uint32_t utc, n, i, d;

    if (!rtc_getutc(&utc))
    {
        return 0;
    }

    utc += (int32_t)(_RTC_TDIF * 3600);

    rtc->sec = (uint8_t)(utc % 60);
    utc /= 60;
    rtc->min = (uint8_t)(utc % 60);
    utc /= 60;
    rtc->hour = (uint8_t)(utc % 24);
    utc /= 24;
    rtc->wday = (uint8_t)((utc + 4) % 7);
    rtc->year = (uint16_t)(1970 + utc / 1461 * 4);
    utc %= 1461;
    n = ((utc >= 1096) ? utc - 1 : utc) / 365;
    rtc->year += n;
    utc -= n * 365 + (n > 2 ? 1 : 0);

    for (i = 0; i < 12; i++)
    {
        d = samurai[i];

        if (i == 1 && n == 2)
        {
            d++;
        }

        if (utc < d)
        {
            break;
        }

        utc -= d;
    }

    rtc->month = (uint8_t)(1 + i);
    rtc->mday = (uint8_t)(1 + utc);

    return 1;
}

int rtc_settime(const rtc_t* rtc)
{
    uint32_t utc, i, y;


    y = rtc->year - 1970;

    if (y > 2106 || !rtc->month || !rtc->mday)
    {
        return 0;
    }

    utc = y / 4 * 1461;
    y %= 4;
    utc += y * 365 + (y > 2 ? 1 : 0);

    for (i = 0; i < 12 && i + 1 < rtc->month; i++)
    {
        utc += samurai[i];

        if (i == 1 && y == 2)
        {
            utc++;
        }
    }

    utc += rtc->mday - 1;
    utc *= 86400;
    utc += rtc->hour * 3600 + rtc->min * 60 + rtc->sec;

    utc -= (long)(_RTC_TDIF * 3600);

    return rtc_setutc(utc);
}
