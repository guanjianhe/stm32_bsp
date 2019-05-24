#include "led.h"


/* 初始化LED */
void InitLed ( void )
{
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启GPIOC外设时钟*/
    RCC_APB2PeriphClockCmd ( RCC_LED, ENABLE );

    /* 选择要控制的GPIOB引脚 */
    GPIO_InitStructure.GPIO_Pin = PIN_LED;

    /*设置引脚速率为50MHz */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /*设置引脚模式为通用推挽输出*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    /*调用库函数，初始化GPIOA*/
    GPIO_Init ( PORT_LED, &GPIO_InitStructure );

    /* 关闭led灯 GPIOx->BSRR = GPIO_Pin */
    GPIO_SetBits ( PORT_LED, PIN_LED );

    /* 关闭led灯 GPIOx->BRR = GPIO_Pin; */
    // GPIO_ResetBits( PORT_LED, PIN_LED );
}

int led_set (uint8_t led_id, uint8_t state)
{
    int retval = 0;

    if (led_id < LED_NUM)
    {
        switch (led_id)
        {
            case 0:
            {
                if (state)
                {

                }
                else
                {

                }

                break;
            }

            default:
            {
                break;
            }
        }
    }
    else
    {
        retval = -1;
    }

    return retval;

}
int led_on (uint8_t led_id)
{
    int retval = 0;

    if (led_id < LED_NUM)
    {
        switch (led_id)
        {
            case 0:
            {
                PORT_LED->BSRR = PIN_LED;
                break;
            }

            default:
            {
                break;
            }
        }
    }
    else
    {
        retval = -1;
    }

    return retval;

}
int led_off (uint8_t led_id)
{
    int retval = 0;

    if (led_id < LED_NUM)
    {
        switch (led_id)
        {
            case 0:
            {
                PORT_LED->BSRR = PIN_LED;
                break;
            }

            default:
            {
                break;
            }
        }
    }
    else
    {
        retval = -1;
    }

    return retval;
}
int led_toggle (uint8_t led_id)
{
    int retval = 0;

    if (led_id < LED_NUM)
    {
        switch (led_id)
        {
            case 0:
            {
                PORT_LED->ODR ^= PIN_LED;

                break;
            }

            default:
            {
                break;
            }
        }
    }
    else
    {
        retval = -1;
    }

    return retval;
}

