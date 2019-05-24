#include "led.h"

typedef struct led_info
{

    uint32_t RCC_APB2Periph;
    uint16_t GPIO_Pin;
    GPIO_TypeDef* GPIOx;
    uint8_t ON_Sta;
} led_info_t;

static const led_info_t g_led_info[] =
{
    {
        .RCC_APB2Periph = RCC_APB2Periph_GPIOC,
        .GPIO_Pin = GPIO_Pin_13,
        .GPIOx = GPIOC,
        .ON_Sta = Bit_RESET,
    },
};

static const uint8_t LED_NUM = sizeof (g_led_info) / sizeof (g_led_info[0]);

static void LED_GPIO_Configuration (void)
{
    int i;
    GPIO_InitTypeDef GPIO_InitStructure;

    for (i = 0; i < LED_NUM; i++)
    {
        RCC_APB2PeriphClockCmd (g_led_info[i].RCC_APB2Periph, ENABLE);
        GPIO_InitStructure.GPIO_Pin = g_led_info[i].GPIO_Pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init (g_led_info[i].GPIOx, &GPIO_InitStructure);
        GPIO_WriteBit (g_led_info[i].GPIOx, g_led_info[i].GPIO_Pin, (BitAction)!g_led_info[i].ON_Sta);
    }
}

void led_init (void)
{
    LED_GPIO_Configuration();
}

int led_set (uint8_t led_id, uint8_t state)
{
    int retval = 0;

    if (led_id < LED_NUM)
    {
        if (state)
        {
            GPIO_WriteBit (g_led_info[led_id].GPIOx,
                           g_led_info[led_id].GPIO_Pin,
                           (BitAction)g_led_info[led_id].ON_Sta);
        }
        else
        {
            GPIO_WriteBit (g_led_info[led_id].GPIOx,
                           g_led_info[led_id].GPIO_Pin,
                           (BitAction)!g_led_info[led_id].ON_Sta);
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
        GPIO_WriteBit (g_led_info[led_id].GPIOx,
                       g_led_info[led_id].GPIO_Pin,
                       (BitAction)g_led_info[led_id].ON_Sta);
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
        GPIO_WriteBit (g_led_info[led_id].GPIOx,
                       g_led_info[led_id].GPIO_Pin,
                       (BitAction)!g_led_info[led_id].ON_Sta);
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
        g_led_info[led_id].GPIOx->ODR ^= g_led_info[led_id].GPIO_Pin;
    }
    else
    {
        retval = -1;
    }

    return retval;
}

