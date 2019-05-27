#include "uart.h"

#define UART_TXB 128

static volatile struct
{
    uint16_t    tri, twi, tct;
    uint8_t     tbuf[UART_TXB];
} Fifo1;


void xputc(uint8_t d)
{
    int i;

    while (Fifo1.tct >= UART_TXB) ;

    i = Fifo1.twi;
    Fifo1.tbuf[i] = d;
    Fifo1.twi = ++i % UART_TXB;
    __disable_irq();
    Fifo1.tct++;
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    __enable_irq();
}

void xputs(const char* str)
{
    while (*str)
    {
        xputc(*str++);
    }
}

static void uart1_init(uint32_t baudrate)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
    USART_ClearFlag(USART1, USART_FLAG_TC);
}

void uart_init(void)
{
    uart1_init(9600);
    Fifo1.tri = 0;
    Fifo1.twi = 0;
    Fifo1.tct = 0;
}

void USART1_IRQHandler(void)
{
    uint8_t d;
    int i;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        d = USART_ReceiveData(USART1);
        xputc(d);
    }

    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        i = Fifo1.tct;

        if (i--)
        {
            Fifo1.tct = (uint16_t)i;
            i = Fifo1.tri;
            USART_SendData(USART1, Fifo1.tbuf[i]);
            Fifo1.tri = ++i % UART_TXB;
        }
        else
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }
}

