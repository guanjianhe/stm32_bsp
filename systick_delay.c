#include "delay.h"

/* us延时时钟数 */
static uint8_t  delay_us    = 0;
/* ms延时时钟数 */
static uint16_t delay_ms    = 0;


void systick_init(void)
{
    /*滴答时钟选择，SysTick_CLKSource_HCLK_Div8*/
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    /*为系统时钟的1/8  系统内核时钟=72M,9个时钟1us*/
    delay_us    = SystemCoreClock / 8000000;
    delay_ms    = (uint16_t) delay_us * 1000;
}


void systick_delay_us(uint32_t nus)
{
    uint32_t temp;

    if (nus > 1864135)
    {
        return;
    }

    SysTick->LOAD   = nus * delay_us;                       /* 时间加载 */
    SysTick->VAL    = 0x00;                                 /* 清空计数器 */
    SysTick->CTRL   |= SysTick_CTRL_ENABLE_Msk;             /* 开始倒数 */

    do  /* 等待时间到达 */
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));


    SysTick->CTRL   &= ~SysTick_CTRL_ENABLE_Msk;            /* 关闭计数器 */
    SysTick->VAL    = 0X00;                                 /* 清空计数器 */
}


void systick_delay_ms(uint16_t nms)
{
    uint32_t temp;

    if (nms > 1864)
    {
        return;
    }

    /* 时间加载(SysTick->LOAD为24bit) */
    SysTick->LOAD   = (uint32_t) nms * delay_ms;
    SysTick->VAL    = 0x00;                                 /* 清空计数器 */
    SysTick->CTRL   |= SysTick_CTRL_ENABLE_Msk;             /* 开始倒数 */

    do  /* 等待时间到达 */
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));

    SysTick->CTRL   &= ~SysTick_CTRL_ENABLE_Msk;            /* 关闭计数器 */
    SysTick->VAL    = 0x00;                                 /* 清空计数器 */
}

