#include "pwm.h"


void InitPWM (void)
{
    GPIO_InitTypeDef    GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef   TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);   /* 开启TIM2时钟 */
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);  /* 开启GPIOA时钟 */

    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;      /* 复用推挽输出模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init (GPIOA, &GPIO_InitStructure);                 /* 初始化 */

    /*定时周期，定时周期为(arr+1)×T，T为时钟源周期*/
    TIM_TimeBaseStructure.TIM_Period = 7199;


    /*
     * 对定时器时钟TIMxCLK的预分频值，分频后作为脉冲计数器TIMx_CNT
     * 的驱动时钟，得到脉冲计数器的时钟频率为f(CK_CNT)=f(TIMxCLK)/(N+1)，
     * 其中N即为赋给本成员的时钟分频值，不分频的话是72MHz
     */

    /* 不分频，10K 频率 f = F/C = 72M/7200 */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;

    /*没任何影响*/
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    /*计数模式，向上计数*/
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    /*初始化*/
    TIM_TimeBaseInit (TIM2, &TIM_TimeBaseStructure);


    /***********************************************************/


    /*
     * 输出模式配置
     * PWM1模式：在向上或向下计数时，当TIMx_CNT<TIMx_CCRn(比较寄存器,其数值等于TIM_Pulse成员的内容)时，
     * 通道n输出为有效电平，否则为无效电平。
     * PWM2模式与PWM1模式相反。有效电平由TIM_OCPolarity成员配置。
     */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

    /* 配置有效电平 */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    /* 配置输出模式的状态，使能或关闭输出 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;


    /* 本成员的参数值即为比较寄存器TIMx_CCR的数值 */
    TIM_OCInitStructure.TIM_Pulse = 0;

    /* 初始化通道4，可以改变以上成员的值，其他通道也一样 */
    TIM_OC4Init (TIM2, &TIM_OCInitStructure);


    /*
     * 可以利用库函数TIM_SetComparex动态修改比较寄存器的值（即TIM_Mode_Config()）
     * 例：TIM_SetCompare1(TIM3,500);
     */

    TIM_SetCompare4 (TIM2, 72 * 0);


    /* 使能TIM比较寄存器CCRx重载 */
    TIM_OC4PreloadConfig (TIM2, TIM_OCPreload_Enable);
    /* 使能TIM重载寄存器ARR */
    TIM_ARRPreloadConfig (TIM2, ENABLE);
    /* 使能定时器2 */
    TIM_Cmd (TIM2, ENABLE);
}


